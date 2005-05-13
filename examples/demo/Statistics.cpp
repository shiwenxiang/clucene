#include "stdafx.h"
#ifndef _lucene_demo_Statistics_
#define _lucene_demo_Statistics_

#include "CLucene.h"
#include "CLucene/index/Term.h"
#include "CLucene/index/SegmentTermEnum.h"
#include <iostream>

using namespace std;
namespace lucene{ namespace demo {
//	using namespace lucene::index;
	using namespace lucene::analysis;
	using namespace lucene::util;
//	using namespace lucene::store;
	using namespace lucene::document;
	using namespace lucene::search;
	
	static void getStats(const char_t* directory){

		IndexReader& r = IndexReader::open(directory,true);
		_cout << _T("Statistics for ") << directory << endl;
		_cout << _T("==================================")<<endl;

		_cout << _T("Max Docs: ") << r.MaxDoc() << endl;
		_cout << _T("Num Docs:") << r.NumDocs() << endl;
		_cout << _T("Last Modified: ") << r.lastModified( directory ) << endl;
		
		TermEnum& te = r.getTerms();
		_cout << _T("Term count: ") << ((SegmentTermEnum&)te).size << endl;

		_cout <<endl;
		delete &te;

		r.close();
		delete &r;
	}
}}
#endif
