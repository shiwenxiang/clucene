#include "CLucene/StdHeader.h"
#include "MultiSearcher.h"

#include "SearchHeader.h"
#include "CLucene/document/Document.h"
#include "CLucene/index/Term.h"
#include "CLucene/util/PriorityQueue.h"
#include "HitQueue.h"

using namespace lucene::index;
using namespace lucene::util;
using namespace lucene::document;

namespace lucene{ namespace search{

  /** Creates a searcher which searches <i>searchers</i>. */
  MultiSearcher::MultiSearcher(Searcher** _searchers, int_t _searchersLen):
    searchers(_searchers),
    searchersLen(_searchersLen),
  _maxDoc(0)
  {
    starts = new int_t[_searchersLen + 1];	  // build starts array
    for (int_t i = 0; i < _searchersLen; i++) {
      starts[i] = _maxDoc;
      _maxDoc += searchers[i]->maxDoc();		  // compute maxDocs
    }
    starts[_searchersLen] = _maxDoc;
  }

  MultiSearcher::~MultiSearcher() {
    /* DSR:CL_BUG_LEAK_ARGUABLY: */
    delete[] searchers;
    delete[] starts;
  }

  /** Frees resources associated with this <code>Searcher</code>. */
  void MultiSearcher::close() {
    for (int_t i = 0; i < searchersLen; i++)
      searchers[i]->close();
  }

  int_t MultiSearcher::docFreq(const Term& term) const {
    int_t docFreq = 0;
    for (int_t i = 0; i < searchersLen; i++)
      docFreq += searchers[i]->docFreq(term);
    return docFreq;
  }

  /** For use by {@link HitCollector} implementations. */
  Document& MultiSearcher::doc(const int_t n) {
    int_t i = searcherIndex(n);			  // find searcher index
    return searchers[i]->doc(n - starts[i]);	  // dispatch to searcher
  }

  /** For use by {@link HitCollector} implementations to identify the
   * index of the sub-searcher that a particular hit came from. */
  int_t MultiSearcher::searcherIndex(int_t n) {	  // find searcher for doc n:
    // replace w/ call to Arrays.binarySearch in Java 1.2
    int_t lo = 0;					  // search starts array
    int_t hi = searchersLen - 1;		  // for first element less
              // than n, return its index
    while (hi >= lo) {
      int_t mid = (lo + hi) >> 1;
      int_t midValue = starts[mid];
      if (n < midValue)
      hi = mid - 1;
      else if (n > midValue)
      lo = mid + 1;
      else
      return mid;
    }
    return hi;
  }

  int_t MultiSearcher::maxDoc() const{
    return _maxDoc;
  }

  TopDocs& MultiSearcher::Search(Query& query, const Filter* filter, const int_t nDocs) {
    HitQueue* hq = new HitQueue(nDocs);
    float_t minScore = 0.0f;
    int_t totalHits = 0;

    for (int_t i = 0; i < searchersLen; i++) {  // search each searcher
      TopDocs* docs = &searchers[i]->Search(query, filter, nDocs);
      totalHits += docs->totalHits;		  // update totalHits
      ScoreDoc** scoreDocs = docs->scoreDocs;
    int_t scoreDocsLen = docs->scoreDocsLength;
      for (int_t j = 0; j <scoreDocsLen; j++) { // merge scoreDocs int_to hq
      ScoreDoc* scoreDoc = scoreDocs[j];
      if (scoreDoc->score >= minScore) {
        scoreDoc->doc += starts[i];		  // convert doc
        hq->put(scoreDoc);			  // update hit queue
        if (hq->Size() > nDocs) {		  // if hit queue overfull
          delete hq->pop();				  // remove lowest in hit queue
          minScore = ((ScoreDoc*)hq->top())->score; // reset minScore
        }
      } else
        break;				  // no more scores > minScore
      }
    docs->deleteScoreDocs = false;
    delete docs;
    }


    int_t scoreDocsLen = hq->Size();
  ScoreDoc** scoreDocs = new ScoreDoc*[scoreDocsLen];
    for (int_t i = hq->Size()-1; i >= 0; i--)	  // put docs in array
      scoreDocs[i] = (ScoreDoc*)hq->pop();

  //cleanup
  delete hq;

    return *new TopDocs(totalHits, scoreDocs,scoreDocsLen);
  }

  /** Lower-level search API.
   *
   * <p>{@link HitCollector#collect(int_t,float_t)} is called for every non-zero
   * scoring document.
   *
   * <p>Applications should only use this if they need <i>all</i> of the
   * matching documents.  The high-level search API ({@link
   * Searcher#search(Query)}) is usually more efficient, as it skips
   * non-high-scoring hits.
   *
   * @param query to match documents
   * @param filter if non-null, a bitset used to eliminate some documents
   * @param results to receive hits
   */
  void MultiSearcher::Search(Query& query, const Filter* filter, HitCollector& results){
    for (int_t i = 0; i < searchersLen; i++) {
      /* DSR:CL_BUG: Old implementation leaked and was misconceived.  We need
      ** to have the original HitCollector ($results) collect *all* hits;
      ** the MultiHitCollector instantiated below serves only to adjust
      ** (forward by starts[i]) the docNo passed to $results.
      ** Old implementation instead created a sort of linked list of
      ** MultiHitCollectors that applied the adjustments in $starts
      ** cumulatively (and was never deleted). */
      HitCollector *docNoAdjuster = new MultiHitCollector(&results, starts[i]);
      searchers[i]->Search(query, filter, *docNoAdjuster);
      delete docNoAdjuster;
    }
  }




  MultiHitCollector::MultiHitCollector(HitCollector* _results, int_t _start):
  results(_results),
  start(_start)
  {
  }
  void MultiHitCollector::collect(const int_t doc, const float_t score) {
    results->collect(doc + start, score);
  }


}}
