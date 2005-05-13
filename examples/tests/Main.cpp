#include "stdafx.h"


#ifdef _DEBUG
#ifdef TR_LEAKS
#ifdef COMPILER_MSVC
	int _lucene_BlockStop;
	#define CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
#endif
#endif
#endif


int main( int_t argc, char** argv ){

	//Dumper Debug
	#ifdef TR_LEAKS
	#ifdef COMPILER_MSVC
	#ifdef _DEBUG
		_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );//| _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_CHECK_CRT_DF );
	#endif
	#endif	
	#endif

	long_t str = lucene::util::Misc::currentTimeMillis();
	try{
		_cout << _T("\n CLucene tests - make sure you have an index created called 'index'...")<<endl;

		_cout << _T("\n===== Test Store with RAM =====")<<endl;
		lucene::test::StoreTest(1000,true);

		_cout << _T("\n===== Test Store with FSDirectory =====")<<endl;
		lucene::test::StoreTest(1000,false);

		_cout << _T("\n===== Test Datefilter (may have to delete index1 directory) =====")<<endl;
		lucene::test::TestDateFilter::testAfter();
		lucene::test::TestDateFilter::testBefore();

		_cout << _T("\n===== Test Wildcards & Asterisk =====")<<endl;
		lucene::test::testAsterisk();
		lucene::test::testQuestionmark();
	
		_cout << _T("\n===== Test Query Parse =====")<<endl;
		_cout << _T("\nNote that some may have errors due to floating point problems\n non-unicode compiles will also have unexpected results.")<<endl;
		lucene::test::TestQueryParser::testRange();
		lucene::test::TestQueryParser::testQPA();
		lucene::test::TestQueryParser::testSimple();
		lucene::test::TestQueryParser::testEscaped();
		lucene::test::TestQueryParser::testNumber(); 
		lucene::test::TestQueryParser::testPunct();
		lucene::test::TestQueryParser::testWildcard();
		lucene::test::TestQueryParser::testSlop();

		_cout << _T("\n===== Test Analyzers =====")<<endl;
		lucene::test::Analysis::testSimpleAnalyzer();
		lucene::test::Analysis::testNullAnalyzer();
		lucene::test::Analysis::testStopAnalyzer();

		_cout << _T("\n===== Test Search =====")<<endl;
		lucene::test::SearchTest();

		_cout << _T("\n===== Test Search - for duplicates =====")<<endl;
		lucene::test::SearchTestForDuplicates::test();

		_cout << _T("\n===== Test Priority Queue =====")<<endl;
		lucene::test::priorityQueue::test_PriorityQueue(100000);

		_cout << _T("\n===== Test Analysis with text =====")<<endl;
		lucene::test::analysisTest::testText(_T("This is a test"),true);

		_cout << _T("\n===== Test Analysis with file (using 'words.txt') =====")<<endl;
		lucene::test::analysisTest::testFile(_T("words.txt"),false);

		_cout << _T("\n===== Searches (using index 'index' - will fail if index not present) =====")<<endl;
		lucene::test::TestSearches(_T("index"));

#ifdef _WIN32
// TODO: create a make file for the contributions section
// this test needs the highlighter in order to compile
		_cout << _T("\n===== Test Highlight =====") << endl;
		lucene::test::HighlightTest();
#endif
		_cout << _T("\n===== Test High Freq Terms =====")<<endl;
		lucene::test::TestHighFreqTerms(_T("index"),100);


	}catch (THROW_TYPE e){
		cout << "Exception: " << e.what() << endl;
		//throw;
	}catch ( char_t* err ){
		_cout << _T("Error: ") << err;
		//throw;
	}catch ( ... ){
		cout << "UNKNOWN ERROR" << endl;
		//throw; 
	}


	//Debuggin techniques:
	//In the Watch window, type the following expression in the Name column: 
	//_crtBreakAlloc

	_cout << endl << endl << _T("Total time taken: ") << (lucene::util::Misc::currentTimeMillis() - str) << _T("ms.") << endl << endl;

}

