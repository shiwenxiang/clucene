#include "stdafx.h"
#include "CLucene.h"
#include "CLucene/util/Reader.h"
#include <iostream>

using namespace lucene::index;
using namespace lucene::analysis;
using namespace lucene::util;
using namespace lucene::store;
using namespace lucene::document;
using namespace lucene::queryParser;

int cmd_delete( int_t argc, char** argv ){
    char_t* target = TO_CHAR_T(argv[0]);
	argc--;argv++;

	int ret = 0;
	if ( argc == 1 ){
        char_t* query = TO_CHAR_T(argv[0]);
        standard::StandardAnalyzer analyzer;
		Query& q = QueryParser::Parse(query,_T("contents"),analyzer);
		
		Directory* d = &FSDirectory::getDirectory( target,false );
		
		IndexReader& r = IndexReader::open(*d,false);
		IndexSearcher s(r);
		Hits& h = s.search(q);
		ret = h.Length();
		for ( int_t i=0;i<h.Length();i++ ){
			r.Delete(h.id(i));
		}

		delete &h;
		delete &q;
		s.close();
//		r.close();

		delete[] query;
	}else{
		usage();
	}

	delete[] target;
	return ret;
}
