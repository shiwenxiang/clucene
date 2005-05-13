#include "CLucene/StdHeader.h"
#include "Similarity.h"

#include "CLucene/index/Term.h"
#include "SearchHeader.h"


using namespace std;
namespace lucene{ namespace search {
	
	
	
	static float_t _SIMILARITY_NORM_TABLE[256];
	static bool _SIMILARITY_NORM_TABLE_MADE=false;

	//static 
	float_t* Similarity::SIMILARITY_NORM_TABLE(){
		if ( !_SIMILARITY_NORM_TABLE_MADE ){
			for (int_t i = 0; i < 256; i++)
				_SIMILARITY_NORM_TABLE[i] = i / 255.0F;
			_SIMILARITY_NORM_TABLE_MADE = true;
		}
		return _SIMILARITY_NORM_TABLE;
	}

	//static 
	l_byte_t Similarity::normb(const int_t numTerms) {
		// Scales 1/sqrt(numTerms) into a byte, i.e. 256/sqrt(numTerms).
		// Math.ceil is used to ensure that even very long_t documents don't get a
		// zero norm byte, as that is reserved for zero-lengthed documents and
		// deleted documents.
		return (l_byte_t) ceil(255.0 / sqrt( (double)numTerms));
	}

	//static 
	float_t Similarity::normf(const l_byte_t normByte) {
		// Un-scales from the byte encoding of a norm into a float_t, i.e.,
		// approximately 1/sqrt(numTerms).
		return SIMILARITY_NORM_TABLE()[normByte & 0xFF];
	}

	//static 
	float_t Similarity::tf(const int_t freq) {
		return (float_t)floatSquareRoot((float_t)freq);
	}

	//static 
	float_t Similarity::tf(const float_t freq) {
		return (float_t)floatSquareRoot(freq);
	}
		
	//static 
	float_t Similarity::idf(const lucene::index::Term& term, const Searcher& searcher) {
		// Use maxDoc() instead of numDocs() because its proportional to docFreq(),
		// i.e., when one is inaccurate, so is the other, and in the same way.
		return idf(searcher.docFreq(term), searcher.maxDoc());
	}

	//static 
	float_t Similarity::idf(const int_t docFreq, const int_t numDocs){
		return (float_t)(floatLog(numDocs/(float_t)(docFreq+1)) + 1.0);
	}
	
	//static 
	float_t Similarity::coord(const int_t overlap, const int_t maxOverlap) {
		return overlap / (float_t)maxOverlap;
	}
}}
