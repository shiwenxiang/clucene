#include "stdafx.h"
#ifndef _lucene_demo_IndexFiles_
#define _lucene_demo_IndexFiles_

#include "CLucene.h"
#include "CLucene/util/Reader.h"
#include <iostream>

using namespace std;
namespace lucene{ namespace demo {
	using namespace lucene::index;
	using namespace lucene::analysis;
	using namespace lucene::util;
	using namespace lucene::store;
	using namespace lucene::document;

	static Document& FileDocument(const char_t* f){
		// make a new, empty document
		Document& doc = *new Document();

		// Add the path of the file as a field named "path".  Use a Tex t field, so
		// that the index stores the path, and so that the path is searchable
		doc.add( Field::Text(_T("path"), f ) );

		// Add the last modified date of the file a field named "modified".  Use a
		// Keyword field, so that it's searchable, but so that no attempt is made
		// to tokenize the field into words.
		//doc.add(Field.Keyword("modified",
		//		  DateField.timeToString(f.lastModified())));

		// Add the contents of the file a field named "contents".  Use a Text
		// field, specifying a Reader, so that the text of the file is tokenized.
		FileReader* reader = new FileReader(f);
		doc.add( Field::Text(_T("contents"),reader) );

		// return the document
		return doc;
	}

	static void indexDocs(IndexWriter* writer, char_t* directory) {
		DIR* dir = opendir(directory);
		struct dirent* fl;
		
		struct Struct_Stat buf;

		char_t path[CL_MAX_DIR];
		stringCopy(path,directory);
		stringCat(path,PATH_DELIMITER);
		char_t* pathP = path + stringLength(path);

		fl = readdir(dir);
		while ( fl != NULL ){
		  if ( (stringCompare(fl->d_name, _T("."))) && (stringCompare(fl->d_name, _T(".."))) ) {
		    pathP[0]=0;
		    stringCat(pathP,fl->d_name);
		    int_t ret = Cmd_Stat(path,&buf);
		    if ( buf.st_mode & S_IFDIR ) {
		      indexDocs(writer, path );
		    }else{
		      printFormatted( _T("adding: %s\n"), fl->d_name );
		      Document& doc = FileDocument( path );
		      writer->addDocument( doc );
		      delete &doc;
		    }
		  }
		  fl = readdir(dir);

		}
		closedir(dir);
	}
	static void IndexFiles(char_t* path, char_t* target, const bool clearIndex){
		long_t str = lucene::util::Misc::currentTimeMillis();
		IndexWriter* writer = NULL;
		Directory* d = NULL;
		//lucene::analysis::SimpleAnalyzer& an = *new lucene::analysis::SimpleAnalyzer();
		lucene::analysis::standard::StandardAnalyzer& an = *new lucene::analysis::standard::StandardAnalyzer();
		
		if ( !clearIndex && IndexReader::indexExists(target) ){
			d = &FSDirectory::getDirectory( target,false );
			if ( IndexReader::isLocked(*d) ){
				_cout << _T("Index was locked... unlocking it.")<<endl;
				IndexReader::unlock(*d);
			}

			writer = new IndexWriter( *d, an, false);
		}else{
			d = &FSDirectory::getDirectory(target,true);
			writer = new IndexWriter( *d ,an, true);
		}
		//writer->infoStream = &cout; //TODO: infoStream - unicode
		indexDocs(writer, path);

		

		writer->optimize();
		writer->close();
		delete writer;
		delete &an;

		_cout << _T("Indexing took: ") << (lucene::util::Misc::currentTimeMillis() - str) << _T("ms.") << endl << endl;
	}

		
}}
#endif
