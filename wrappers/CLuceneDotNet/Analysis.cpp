#include "Stdafx.h"
#ifndef _lucene_managed_analysis
#define _lucene_managed_analysis

#include "Clucene/analysis/standard/StandardAnalyzer.h"
#include "Clucene/analysis/AnalysisHeader.h"
#include "Clucene/analysis/Analyzers.h"
#include "Managed.h"

using namespace System;
using namespace lucene::util;
namespace lucene { namespace analysis {

	public __gc class MAnalyzer{
	public:
		Analyzer* __mp;

		MAnalyzer(Analyzer* _mp):
			__mp(_mp)
		{
		}
		~MAnalyzer(){
			delete __mp;
		}
	};

	public __gc class MStandardAnalyzer:public MAnalyzer{
	public:
		MStandardAnalyzer():
			MAnalyzer( new standard::StandardAnalyzer() )
		{
		}
		~MStandardAnalyzer(){
		}
	};

}}
#endif
