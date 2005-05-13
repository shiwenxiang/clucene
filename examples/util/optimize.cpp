#include "stdafx.h"
#include "CLucene.h"
#include "CLucene/util/Reader.h"
#include <iostream>

using namespace lucene::index;
using namespace lucene::analysis;
using namespace lucene::util;
using namespace lucene::store;
using namespace lucene::document;

int cmd_optimize( int_t argc, char** argv ){
    char_t* target = TO_CHAR_T(argv[0]);
	argc--;argv++;

	Directory& d = FSDirectory::getDirectory( target,false );
	SimpleAnalyzer a;
	IndexWriter& writer = *new IndexWriter(d,a,false) ;
	writer.optimize();

	writer.close();
	delete &writer;
	delete[] target;
	return 0;
}
