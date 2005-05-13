#ifndef _lucene_demo_SearchFiles_
#define _lucene_demo_SearchFiles_

#include <iostream>
#include "CLucene/analysis/AnalysisHeader.h"
#include "CLucene/analysis/Analyzers.h"
#include "CLucene/analysis/standard/StandardAnalyzer.h"
#include "CLucene/document/Document.h"
#include "CLucene/search/IndexSearcher.h"
#include "CLucene/queryParser/QueryParser.h"
#include "CLucene/index/Term.h"
#include "CLucene/index/Terms.h"
#include "CLucene/search/SearchHeader.h"

using namespace std;
namespace lucene{ namespace demo {

//	using namespace lucene::index;
	using namespace lucene::analysis;
	using namespace lucene::util;
	using namespace lucene::queryParser;
	using namespace lucene::document;
	using namespace lucene::search;
	
void SearchFiles(const char_t* index){
        //Searcher searcher(index);
		standard::StandardAnalyzer analyzer;
		char_t line[80];
		const char_t* buf;

        while (true) {
			_cout << _T("Enter query string: ");
			_cin.getline(line,80);

			if ( stringLength(line) == 0 )
				break;
			Query& q = QueryParser::Parse(line,_T("contents"),analyzer);

			buf = q.toString(_T("contents"));
			_cout << _T("Searching for: ") << buf << endl << endl;
			delete buf;

			if ( !IndexReader::indexExists(index) ){
			   _cout << index << _T(" does not exist!") << endl;
			   return;
			}
			IndexReader& r = IndexReader::open(index);
			IndexSearcher s(r);

			long_t str = lucene::util::Misc::currentTimeMillis();
			Hits& h = s.search(q);
			long_t srch = lucene::util::Misc::currentTimeMillis() - str;

			for ( int_t i=0;i<h.Length();i++){
				Document& doc = h.doc(i);
				//delete &doc;
			}
			long_t qry = lucene::util::Misc::currentTimeMillis() - str;
			str = lucene::util::Misc::currentTimeMillis();

			for ( int_t i=0;i<h.Length();i++ ){
				Document& doc = h.doc(i);
				//const char_t* buf = doc.get(_T("contents"));
				_cout << i << _T(". ") << doc.get(_T("path")) << _T(" - ") <<  h.score(i) << endl;
				//delete &doc;
			}

			_cout << endl << endl << _T("Search took: ") << srch << _T("ms.") << endl;
			_cout << endl << endl << _T("Get took: ") << qry << _T("ms.") << endl;
			_cout << _T("Screen dump took: ") << (lucene::util::Misc::currentTimeMillis() - str) << _T("ms.") << endl << endl;

			delete &h;
			delete &q;
			s.close();
			delete &r;
			
	    }
		//delete line;
    }	
	
	
}}



int main (int argc, char *argv[])
{ 
    lucene::demo::SearchFiles("c:/index");
}

#endif

