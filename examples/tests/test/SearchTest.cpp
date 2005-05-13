#include "stdafx.h"
#ifndef _lucene_test_SearchTest_
#define _lucene_test_SearchTest_

#include "CLucene.h"
#include "CLucene/analysis/Analyzers.h"
#include "CLucene/util/Reader.h"
#include "CLucene/util/VoidList.h"
#include "CLucene/analysis/AnalysisHeader.h"
#include "CLucene/index/Term.h"
#include "CLucene/index/Terms.h"
#include "CLucene/store/RAMDirectory.h"
#include <iostream>
#include "CLucene/search/SearchHeader.h"

using namespace std;
using namespace lucene::util;
using namespace lucene::analysis;
using namespace lucene::analysis::standard;
using namespace lucene::search;
using namespace lucene::queryParser;
using namespace lucene::search;
namespace lucene{ namespace test {

  void SearchTest() {
	  long_t start = lucene::util::Misc::currentTimeMillis();

	try {
		SimpleAnalyzer* analyzer = new SimpleAnalyzer();
		IndexWriter writer( _T("SearchTestIndex"), *analyzer, true);  

		char_t* docs[] = { _T("a b c d e"),
			_T("a b c d e a b c d e"),	
			_T("a b c d e f g h i j"),
			_T("a c e"),
			_T("e c a"),
			_T("a c e a c e"),
			_T("a c e a b c")
		};

		for (int j = 0; j < 7; j++) {		
			Document* d = new Document();
			Field& f = Field::Text(_T("contents"),docs[j]);
			d->add(f);
			
			writer.addDocument(*d);
			//no need to delete fields... document takes ownership
			delete d;
		}     
		writer.close();

		IndexReader* reader = &IndexReader::open(_T("SearchTestIndex"));
		IndexSearcher searcher(*reader);
		char_t* queries[] = {
			_T("a b"),	
			_T("\"a b\""),	 	
			_T("\"a b c\""),			
			_T("a c"),		 	
			_T("\"a c\""),			
			_T("\"a c e\""),     
		};	
		Hits* hits = NULL;
		QueryParser parser(_T("contents"), *analyzer);      
		parser.PhraseSlop = 4; 
		
		for (int j = 0; j < 6; j++) {		
			Query* query = &parser.Parse(queries[j]);
			const char_t* qryInfo = query->toString(_T("contents"));
			_cout << _T("Query: ") << qryInfo << endl;
			delete qryInfo;
			
			hits = &searcher.search(*query);
			_cout << hits->Length() << _T(" total results") << endl;
			for (int i=0; i<hits->Length() && i<10; i++) {	 
				Document* d = &hits->doc(i);	  
				_cout << i << _T(" ") << hits->score(i) << _T(" ") << d->get(_T("contents")) << endl;
			}

			delete hits;
			delete query;
		}

		searcher.close(); 
		reader->close();
		delete reader;
		if ( analyzer )
			delete analyzer;

	} catch (THROW_TYPE e) {
		_cout << _T(" caught a exception: ");
		printf( e.what() );
		_cout << endl;
	} catch (...){
		_cout << _T(" caught an unknown exception\n");   
	}

	_cout << (lucene::util::Misc::currentTimeMillis()-start) << _T(" milliseconds") << endl;
  }

//  static long_t Time(int_t_t year, int_t_t month, int_t_t day) {
//    GregorianCalendar calendar = new GregorianCalendar();
  //  calendar.set(year, month, day);
    //return calendar.getTime().getTime();
  //}
}}
#endif
