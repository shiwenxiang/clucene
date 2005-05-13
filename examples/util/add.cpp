#include "stdafx.h"
#include "CLucene.h"
#include "CLucene/util/Reader.h"
#include <iostream>

using namespace lucene::index;
using namespace lucene::analysis;
using namespace lucene::util;
using namespace lucene::store;
using namespace lucene::document;

static Document& FileDocument(const char_t* f, bool store){	// make a new, empty document
	Document& doc = *new Document();

	// Add the path of the file as a field named "path".  Use a Tex t field, so
	// that the index stores the path, and so that the path is searchable
	doc.add( Field::Keyword(_T("path"), f ) );

	// Add the last modified date of the file a field named "modified".  Use a
	// Keyword field, so that it's searchable, but so that no attempt is made
	// to tokenize the field into words.
	//doc.add(Field.Keyword("modified",
	//		  DateField.timeToString(f.lastModified())));

	// Add the contents of the file a field named "contents".  Use a Text
	// field, specifying a Reader, so that the text of the file is tokenized.
	// ?? why doesn't FileReader work here ??
	
	FileReader* reader = new FileReader(f);
	doc.add( Field::Text(_T("contents"),reader) );

	// return the document
	return doc;
}

static void indexDocs(IndexWriter* writer, char_t* path, bool dontRecurse, bool store) {
	DIR* dir = NULL;
	struct dirent* fl;
    struct Struct_Stat buf;
    
	dir = opendir(path);
	fl = readdir(dir);
	while ( fl != NULL ){
		if ( (stringCompare(fl->d_name, _T("."))) && (stringCompare(fl->d_name, _T(".."))) ) {
			char_t tmp[CL_MAX_DIR];
    		stringCopy(tmp,path);
    		stringCat(tmp,PATH_DELIMITER);
    		stringCat(tmp, fl->d_name);
			int_t ret = Cmd_Stat(tmp,&buf);
			if ( ret == 0 ){
				if ( buf.st_mode & S_IFDIR ) {
					indexDocs(writer, tmp, dontRecurse, store );
				}else{
    				printFormatted( _T("%s\n"), fl->d_name );
    				Document& doc = FileDocument( tmp, store );
    				writer->addDocument( doc );
    				delete &doc;
				}
			}
		}
        fl = readdir(dir);
	}
	closedir(dir);
}

int cmd_add( int_t argc, char** argv ){
	if ( argc < 2 )
		usage();

	bool dontRecurse = false;
	bool optimize = false;
	bool store = false;
	char_t* path = NULL;
	
	for ( int i=0;i< argc-2;i++ ){
		if ( strcmp(argv[i], "-dontrecurse")==0 )
			dontRecurse = true;
		else if ( strcmp(argv[i],"-optimize")==0 )
			optimize = true;
		else if ( strcmp(argv[i],"-store")==0 )
			store = true;
	}
	
	char_t* target = TO_CHAR_T(argv[argc-2]);
	path = TO_CHAR_T(argv[argc-1]);

	IndexWriter* writer = NULL;
	Directory* d = NULL;
	lucene::analysis::standard::StandardAnalyzer an;
	
	if ( IndexReader::indexExists(target) ){
		d = &FSDirectory::getDirectory( target,false );
		writer = new IndexWriter( *d, an, false);
	}else{
		d = &FSDirectory::getDirectory(target,true);
		writer = new IndexWriter( *d ,an, true);
	}
	
	struct Struct_Stat buf;
	if ( Cmd_Stat(path,&buf) == 0 ){
		if ( buf.st_mode & S_IFREG ){
			Document& doc = FileDocument( path, store );
			writer->addDocument( doc );
			delete &doc;
		}else if ( buf.st_mode & S_IFDIR ){
			indexDocs(writer, path, dontRecurse, store);
		}else{
			cerr << "File/directory is not valid.";
		}
	}else{
		printf( "File/directory does not exist." );
	}
		if ( optimize )
			writer->optimize();

		writer->close();
		delete[] target;
		delete[] path;

	return 1;
}
