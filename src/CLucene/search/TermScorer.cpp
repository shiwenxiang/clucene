#include "CLucene/StdHeader.h"
#include "TermScorer.h"

#include "CLucene/index/Terms.h"
#include "Scorer.h"
#include "CLucene/search/Similarity.h"

using namespace lucene::index;
namespace lucene{ namespace search {

	//TermScorer takes TermDocs and delets it when TermScorer is cleaned up
	TermScorer::TermScorer(TermDocs& td, l_byte_t* _norms, const float_t w):
		termDocs(td),
		norms(_norms),
		weight(w),
		pointer(0)
	{
		for (int_t i = 0; i < LUCENE_SCORE_CACHE_SIZE; i++)
			scoreCache[i] = Similarity::tf(i) * weight;
    
		pointerMax = termDocs.read(docs, freqs);	  // fill buffers
    
		if (pointerMax != 0)
			doc = docs[0];
		else {
			termDocs.close();				  // close stream
			doc = INT_MAX;			  // set to sentinel value
		}
	}

	TermScorer::~TermScorer(){
		delete &termDocs;
	}

	void TermScorer::score(HitCollector& c, const int_t end) {
		int_t d = doc;				  // cache doc in local
		while (d < end) {				  // for docs in window
			const int_t f = freqs[pointer];
			float_t score =				  // compute tf(f)*weight
				f < LUCENE_SCORE_CACHE_SIZE			  // check cache
				? scoreCache[f]			  // cache hit
				: Similarity::tf(f)*weight;		  // cache miss
		    
				score *= Similarity::normf(norms[d]);	  // normalize for field
	    
			c.collect(d, score);			  // collect score
	    
			if (++pointer == pointerMax) {
				pointerMax = termDocs.read(docs, freqs);  // refill buffers
				if (pointerMax != 0) {
					pointer = 0;
				} else {
					termDocs.close();			  // close stream
					doc = INT_MAX;		  // set to sentinel value
					return;
				}
			} 
			d = docs[pointer];
		}
		doc = d;					  // flush cache
	}
	
}}
