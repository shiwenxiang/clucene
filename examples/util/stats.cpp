#include "stdafx.h"
#include "CLucene.h"
#include "CLucene/util/Reader.h"
#include <iostream>

using namespace lucene::index;
using namespace lucene::analysis;
using namespace lucene::util;
using namespace lucene::store;
using namespace lucene::document;


int cmd_stats( int_t argc, char** argv ){
	char_t* target = TO_CHAR_T(argv[0]);
	argc--;argv++;

	
	IndexReader& r = IndexReader::open(target,true);
	_cout << _T("Statistics for ") << target << endl;
	_cout << _T("==================================")<<endl;

	_cout << _T("Max Docs: ") << r.MaxDoc() << endl;
	_cout << _T("Num Docs:") << r.NumDocs() << endl;
	_cout << _T("Last Modified: ") << (unsigned long)r.lastModified( target ) << endl;
	
	TermEnum& te = r.getTerms();
	_cout << _T("Term count: ") << ((SegmentTermEnum&)te).size << endl;

	_cout <<endl;
	delete &te;

	r.close();
	delete &r;
	delete[] target;

	return 0;
}
