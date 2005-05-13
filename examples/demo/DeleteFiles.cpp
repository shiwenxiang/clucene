#include "stdafx.h"
#ifndef _lucene_demo_DeleteFiles_
#define _lucene_demo_DeleteFiles_

#include "CLucene.h"
#include <iostream>

using namespace std;
	using namespace lucene::index;
	using namespace lucene::util;
	using namespace lucene::store;
	using namespace lucene::document;
namespace lucene{ namespace demo {
	static void DeleteFiles(const char_t* dir) {
			IndexReader& reader = IndexReader::open(dir);

	//       Term term = new Term("path", "pizza");
	//       int_t deleted = reader.delete(term);

	//       System.out.println("deleted " + deleted +
	// 			 " documents containing " + term);

			int_t count = 0;
			for (int_t i = 0; i < reader.MaxDoc(); i++){
				reader.Delete (i);
				count ++;
			}
			_cout << _T("Deleted ") << count << _T(" files\n");
			reader.close();
			delete &reader;

			//OPTIMIZE
			if ( IndexReader::indexExists(dir) ){
				lucene::analysis::SimpleAnalyzer& an = *new lucene::analysis::SimpleAnalyzer();
				Directory* d = d = &FSDirectory::getDirectory( dir,false );
				if ( IndexReader::isLocked(*d) ){
					_cout << _T("Index was locked... unlocking it.")<<endl;
					IndexReader::unlock(*d);
				}

				IndexWriter* writer = new IndexWriter( *d, an, false);
				writer->optimize();
				delete writer;
			}

	}
}}
#endif
