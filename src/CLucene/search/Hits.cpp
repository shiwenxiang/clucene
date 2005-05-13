#include "SearchHeader.h"

#include "CLucene/StdHeader.h"
#include "CLucene/document/Document.h"
#include "CLucene/index/IndexReader.h"
#include "Filter.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "CLucene/search/SearchHeader.h"

using namespace lucene::document;
using namespace lucene::util;
using namespace lucene::index;

namespace lucene{ namespace search {

  HitDoc::HitDoc(const float_t s, const int_t i){
	//Func - Constructor
	//Pre  - true
	//Post - The instance has been created

		next  = NULL;
		prev  = NULL;
		doc   = NULL;
		score = s;
		id    = i;
	}

	HitDoc::~HitDoc(){
	//Func - Destructor
	//Pre  - true
	//Post - The instance has been destroyed

		_DELETE(doc);
	}


	Hits::Hits(Searcher& s, Query& q, const Filter* f):
		query(q), searcher(s){
	//Func - Constructor
	//Pre  - s contains a valid reference to a searcher s
	//       q contains a valid reference to a Query
	//       f is NULL or contains a pointer to a filter
	//Post - The instance has been created

		filter  = f;
		length  = 0;
		first   = NULL;
		last    = NULL;
		numDocs = 0;
		maxDocs = 200;

		hitDocs.setDoDelete(DELETE_TYPE_DELETE);
		//retrieve 100 initially
		getMoreDocs(50);
	}

	Hits::~Hits(){

	}
	int_t Hits::Length() const {
		return length;
	}

	Document& Hits::doc(const int_t n){
		HitDoc& hitDoc = getHitDoc(n);

		// Update LRU cache of documents
		remove(hitDoc);				  // remove from list, if there
		addToFront(hitDoc);				  // add to front of list
		if (numDocs > maxDocs) {			  // if cache is full
			HitDoc* oldLast = last;
			remove(*last);				  // flush last

			delete oldLast->doc;
			oldLast->doc = NULL;
		}

		if (hitDoc.doc == NULL)
			hitDoc.doc = &searcher.doc(hitDoc.id);	  // cache miss: read document

		return *hitDoc.doc;
	}

	int_t Hits::id (const int_t n){
		return getHitDoc(n).id;
	}

	float_t Hits::score(const int_t n){
		return getHitDoc(n).score;
	}

	void Hits::getMoreDocs(const int_t Min){
		int_t min = Min;
		{
			// It's inconceivable that the number of hits would exceed the
			// capacity of int_t, so the following cast is safe.
			int_t nHits = static_cast<int_t>(hitDocs.size());
			if (nHits > min)
				min = nHits;
		}

		int_t n = min * 2;				  // double # retrieved
		TopDocs& topDocs = searcher.Search(query, filter, n);
		length = topDocs.totalHits;
		ScoreDoc** scoreDocs = topDocs.scoreDocs;
		int_t scoreDocsLength = topDocs.scoreDocsLength;

		float_t scoreNorm = 1.0f;

		//Check that scoreDocs is a valid pointer before using it
		if (scoreDocs && scoreDocsLength >= 0){
			if (length > 0 && scoreDocs[0]->score > 1.0f){
				scoreNorm = 1.0f / scoreDocs[0]->score;
				}

			int_t end = scoreDocsLength < length ? scoreDocsLength : length;
			for (int_t i = hitDocs.size(); i < end; i++){
				hitDocs.push_back(new HitDoc(scoreDocs[i]->score*scoreNorm, scoreDocs[i]->doc));
				}
			}

		delete &topDocs;
	}

	HitDoc& Hits::getHitDoc(const int_t n){
		CND_PRECONDITION(n >= 0, "n must not be negative")
		if (n >= length){
			char_t buf[100];
			stringPrintF(buf, _T("Not a valid hit number: %d"),n);
			_THROWX( buf );
		}
		if (static_cast<uint_t>(n) >= hitDocs.size())
			getMoreDocs(n);

		return *((HitDoc*)(hitDocs.at(n)));
	}

	void Hits::addToFront(HitDoc& hitDoc) {  // insert at front of cache
		if (first == NULL)
			last = &hitDoc;
		else
			first->prev = &hitDoc;

		hitDoc.next = first;
		first = &hitDoc;
		hitDoc.prev = NULL;

		numDocs++;
	}

	void Hits::remove(HitDoc& hitDoc) {	  // remove from cache
		if (hitDoc.doc == NULL)			  // it's not in the list
			return;					  // abort

		if (hitDoc.next == NULL)
			last = hitDoc.prev;
		else
			hitDoc.next->prev = hitDoc.prev;

		if (hitDoc.prev == NULL)
			first = hitDoc.next;
		else
			hitDoc.prev->next = hitDoc.next;

		numDocs--;
	}
}}
