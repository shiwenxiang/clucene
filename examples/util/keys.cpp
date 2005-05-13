#include "stdafx.h"
#include "CLucene.h"
#include "CLucene/util/Reader.h"
#include <iostream>

using namespace lucene::index;
using namespace lucene::analysis;
using namespace lucene::util;
using namespace lucene::store;
using namespace lucene::document;

int cmd_keys( int_t argc, char** argv ){
    char_t* target = TO_CHAR_T(argv[0]);
    argc--;argv++;
    int ret = 1;
	if ( argc == 2 ){
	    char_t* key = TO_CHAR_T(argv[0]);
        char_t* val = TO_CHAR_T(argv[1]);
        Term* t = new Term( key, val, false,true);
		IndexReader* reader = &IndexReader::open( target );

		reader->close();
		delete reader;
		delete t;
		ret = 0;
	}else{
		usage();
	}
	delete[] target;

	return ret;
}
