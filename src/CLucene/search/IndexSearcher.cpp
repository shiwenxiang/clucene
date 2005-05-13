#include "CLucene/StdHeader.h"
#include "IndexSearcher.h"

#include "SearchHeader.h"
#include "CLucene/store/Directory.h"
#include "CLucene/document/Document.h"
#include "CLucene/index/IndexReader.h"
#include "CLucene/index/Term.h"
#include "CLucene/util/BitSet.h"
#include "HitQueue.h"

using namespace lucene::index;
using namespace lucene::util;
using namespace lucene::document;

namespace lucene{ namespace search {

	SimpleTopDocsCollector::SimpleTopDocsCollector(const BitSet* bs, HitQueue& hitQueue, int_t* totalhits,const int_t ndocs, const float_t minScore):
    bits(bs),
    hq(hitQueue),
    nDocs(ndocs),
    totalHits(totalhits),
    ms(minScore)
  {
  }

  void SimpleTopDocsCollector::collect(const int_t doc, const float_t score) {
    if (score > 0.0f &&			  // ignore zeroed buckets
      (bits==NULL || bits->get(doc))) {	  // skip docs not in bits
      totalHits[0]++;
      if (score >= ms) {
        hq.put(new ScoreDoc(doc, score));	  // update hit queue
        if (hq.Size() > nDocs) {		  // if hit queue overfull
          delete hq.pop();			  // remove lowest in hit queue
          minScore = hq.top()->score; // reset minScore
        }
      }
    }
  }






  SimpleFilteredCollector::SimpleFilteredCollector(BitSet* bs, HitCollector& collector):
    bits(bs),
    results(collector)
  {
  }

  void SimpleFilteredCollector::collect(const int_t doc, const float_t score) {
    if (bits->get(doc)) {		  // skip docs not in bits
      results.collect(doc, score);
    }
  }

  IndexSearcher::IndexSearcher(const char_t* path){
  //Func - Constructor
  //       Creates a searcher searching the index in the named directory.  */
  //Pre  - path != NULL

  //Post - The instance has been created

      CND_PRECONDITION(path != NULL,"path is NULL")

      reader = &IndexReader::open(path);
      readerOwner = true;
  }

  IndexSearcher::IndexSearcher(IndexReader& r){
  //Func - Constructor
  //       Creates a searcher searching the index with the provide IndexReader
  //Pre  - path != NULL
  //Post - The instance has been created

      reader      = &r;
      readerOwner = false;
  }

  IndexSearcher::~IndexSearcher(){
  //Func - Destructor
  //Pre  - true
  //Post - The instance has been destroyed

	  close();
  }

  void IndexSearcher::close(){
  //Func - Frees resources associated with this Searcher.
  //Pre  - true
  //Post - The resources associated have been freed

      if (readerOwner && reader){
          reader->close();
          _DELETE(reader);
          }
  }

  int_t IndexSearcher::docFreq(const Term& term) const{
  //Func - 
  //Pre  - reader != NULL
  //Post -

      CND_PRECONDITION(reader != NULL,"reader is NULL")

      return reader->docFreq(term);
  }

  
  Document& IndexSearcher::doc(const int_t i) {
  //Func - Retrieves i-th document found
  //       For use by HitCollector implementations.
  //Pre  - reader != NULL
  //Post - The i-th document has been returned

      CND_PRECONDITION(reader != NULL,"reader is NULL")

      return reader->document(i);
  }

  int_t IndexSearcher::maxDoc() const {
  //Func - Return total number of documents including the ones marked deleted
  //Pre  - reader != NULL
  //Post - The total number of documents including the ones marked deleted 
  //       has been returned

      CND_PRECONDITION(reader != NULL,"reader is NULL")

      return reader->MaxDoc();
  }

  TopDocs& IndexSearcher::Search(Query& query, const Filter* filter, const int_t nDocs){
  //Func -
  //Pre  - reader != NULL
  //Post -

      CND_PRECONDITION(reader != NULL,"reader is NULL")

      Scorer* scorer = Query::scorer(query, *this, *reader);
	  if (scorer == NULL){
          return *new TopDocs(0, new ScoreDoc*[0],0);
	      }

      const BitSet* bits = filter != NULL ? filter->bits(*reader) : NULL;
      HitQueue* hq = new HitQueue(nDocs);

	  //Check hq has been allocated properly
	  CND_CONDITION(hq != NULL, "Could not allocate memory for HitQueue hq")
      
	  int_t* totalHits = new int_t[1];
      totalHits[0] = 0;

      SimpleTopDocsCollector hitCol(bits,*hq,totalHits,nDocs,0.0f);
      scorer->score( hitCol, reader->MaxDoc());

      ScoreDoc** scoreDocs = NULL;
      int_t scoreDocsLength = hq->Size();

      if (scoreDocsLength > 0){

		  scoreDocs = new ScoreDoc*[scoreDocsLength];
		
		  for (int_t i = scoreDocsLength-1; i >= 0; i--){	  // put docs in array
              scoreDocs[i] = hq->pop();
              }
          }

      int_t totalHitsInt = totalHits[0];
    
      _DELETE(hq);
      _DELETE(bits);
      _DELETE_ARRAY(totalHits);
      _DELETE(scorer);

      return *new TopDocs(totalHitsInt, scoreDocs, scoreDocsLength);
  }

  void IndexSearcher::Search(Query& query, const Filter* filter, HitCollector& results){
  //Func - Search an index and fetch the results
  //       Applications should only use this if they need all of the
  //       matching documents.  The high-level search API (search(Query)) is usually more efficient, 
  //       as it skips non-high-scoring hits.
  //Pre  - query is a valid reference to a query
  //       filter may or may not be NULL
  //       results is a valid reference to a HitCollector and used to store the results
  //Post - filter if non-NULL, a bitset used to eliminate some documents

      BitSet* bs = NULL;
      SimpleFilteredCollector* fc = NULL; 

      if (filter != NULL){
          bs = filter->bits(*reader);
          fc = new SimpleFilteredCollector(bs, results);
          }

      Scorer* scorer = Query::scorer(query, *this, *reader);
      if (scorer != NULL) {
		  if (fc == NULL){
              scorer->score(results, reader->MaxDoc());
		      }
		  else{
              scorer->score((HitCollector&)*fc, reader->MaxDoc());
		      }
          _DELETE(scorer); 
          }

    _DELETE(fc); 
    _DELETE(bs);
  }
}}
