#include "stdafx.h"
#include "CLucene.h"
#include "CLucene/util/Reader.h"
#include <iostream>

using namespace lucene::index;
using namespace lucene::analysis;
using namespace lucene::util;
using namespace lucene::store;
using namespace lucene::document;

int cmd_merge( int_t argc, char** argv ){
	if ( argc != 2 )
		usage();

	char_t* index = TO_CHAR_T(argv[0]);
	char_t* target = TO_CHAR_T(argv[1]);
	IndexReader& reader = IndexReader::open(index);

	Directory* d = NULL;
	IndexWriter* writer = NULL;
	SimpleAnalyzer an;
	if ( IndexReader::indexExists(target) ){
		d = &FSDirectory::getDirectory( target,false );
		writer = new IndexWriter( *d, an, false);
	}else{
		d = &FSDirectory::getDirectory(target,true);
		writer = new IndexWriter( *d ,an, true);
	}
	for ( int i=0;i<reader.MaxDoc();i++ ){
		if ( !reader.isDeleted(i) ){
			Document& doc = reader.document(i);
			if ( &doc != NULL ){
				writer->addDocument(doc);
			}
		}
	}

	writer->close();
	reader.close();
	delete &reader;
	delete writer;
	//delete &d;


	delete[] index;
	delete[] target;
	return 0;
}
