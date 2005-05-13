#include "CLucene/StdHeader.h"
#include "StandardAnalyzer.h"

#include "CLucene/util/VoidMap.h"
#include "CLucene/util/Reader.h"
#include "../AnalysisHeader.h"
#include "../Analyzers.h"
#include "StandardFilter.h"
#include "StandardTokenizer.h"

using namespace lucene::util;
using namespace lucene::analysis;

namespace lucene{ namespace analysis { namespace standard {

	// <summary> Builds an analyzer. </summary>
	StandardAnalyzer::StandardAnalyzer()
	{
		StopFilter::fillStopTable( stopTable,STOP_WORDS,STOP_WORDS_LENGTH );
	}

	//<summary> Builds an analyzer with the given stop words. </summary>
	StandardAnalyzer::StandardAnalyzer( char_t* stopWords[], int_t stopWordsLength)
	{
		StopFilter::fillStopTable( stopTable,stopWords,stopWordsLength );
	}

	StandardAnalyzer::~StandardAnalyzer(){
	}


	// <summary>
	// Constructs a StandardTokenizer filtered by a 
	// StandardFilter, a LowerCaseFilter and a StopFilter.
	// </summary>
	TokenStream& StandardAnalyzer::tokenStream(const char_t* fieldName, Reader* reader) 
	{
		TokenStream* ret = new StandardTokenizer(*reader);
		ret = new StandardFilter(ret,true);
		ret = new LowerCaseFilter(ret,true);
		ret = new StopFilter(ret,true, stopTable);
		return *ret;
	}
}}}
