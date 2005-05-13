// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#ifndef lucene_examples_test_stdafx
#define lucene_examples_test_stdafx

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers


#include "CLucene/StdHeader.h"
#include "CLucene.h"

#include "CLucene/util/PriorityQueue.h"
#include "CLucene/util/Reader.h"
#include "CLucene/util/Misc.h"
#include "CLucene/util/VoidList.h"

#include "CLucene/store/Directory.h"
#include "CLucene/store/FSDirectory.h"
#include "CLucene/store/RAMDirectory.h"

#include "CLucene/index/IndexReader.h"
#include "CLucene/index/Term.h"
#include "CLucene/index/Terms.h"

#include "CLucene/search/IndexSearcher.h"
#include "CLucene/search/SearchHeader.h"
#include "CLucene/search/WildcardQuery.h"

#include "CLucene/queryParser/QueryParser.h"

#include "CLucene/analysis/Analyzers.h"
#include "CLucene/analysis/standard/StandardAnalyzer.h"
#include "CLucene/analysis/AnalysisHeader.h"

#include <stdlib.h>
#include <iostream>


#include <iostream>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <math.h>
#include <time.h>

using namespace std;
using namespace lucene::index;
using namespace lucene::util;
using namespace lucene::store;
using namespace lucene::search;
using namespace lucene::queryParser;
using namespace lucene::analysis;
using namespace lucene::analysis::standard;

namespace lucene{ namespace test{ 
	void StoreTest(int_t count, bool ram);
	void testAsterisk();
	void testQuestionmark();
	void TestSearches(const char_t* index);
	void TestHighFreqTerms(const char_t* index, int_t numTerms);
	void StoreTest(int_t count, bool ram);
	void SearchTest();
	void HighlightTest();


	namespace TestDateFilter{
		void testBefore();
		void testAfter();
	}

	namespace TestQueryParser {
		void testSimple();
		void testPunct();
		void testSlop();
		void testNumber();
		void testWildcard();
		void testQPA();
		void testRange();
		void testEscaped();
	}

	namespace TestDateFilter{
		void testBefore();
		void testAfter();
	}

	namespace Analysis{
		void testSimpleAnalyzer();
		void testNullAnalyzer();
		void testStopAnalyzer();
	}

	namespace analysisTest{
		void testFile(const char_t* fname, bool verbose);
		void testText(const char_t* text, bool verbose);
	}

	 namespace priorityQueue{
		void test_PriorityQueue(int_t count);
	 }

	 namespace SearchTestForDuplicates{
		 void test();
	 }
}}
#endif
