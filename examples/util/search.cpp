#include "stdafx.h"
#include "CLucene.h"
#include "CLucene/util/Reader.h"
#include <iostream>

using namespace lucene::index;
using namespace lucene::analysis;
using namespace lucene::util;
using namespace lucene::store;
using namespace lucene::document;
using namespace lucene::search;
using namespace lucene::queryParser;


//"Usage: search index srch_field ret_fields 'query string'";

int cmd_search( int_t argc, char** argv ){
	char_t* target = TO_CHAR_T(argv[0]);
	argc--;argv++;

	if ( argc != 3 && argc != 1 ){
		usage();
	}else{
		char_t* fields = NULL;
		char_t* retfields = NULL;
		char_t* query = NULL;
		if ( argc == 3 ){
			fields = TO_CHAR_T(argv[0]);
			retfields = TO_CHAR_T(argv[1]);
			query = TO_CHAR_T(argv[2]);
		}else{
			fields = stringDuplicate(_T("contents"));
			retfields = stringDuplicate(_T("path"));
			query = TO_CHAR_T(argv[0]);
		}
		standard::StandardAnalyzer analyzer;
		Query& q = QueryParser::Parse(query,fields,analyzer);

		IndexReader& r = IndexReader::open(target);
		IndexSearcher s(r);

		Hits& h = s.search(q);
		const char_t toks[] = {','};
		for ( int_t i=0;i<h.Length();i++ ){
			Document& doc = h.doc(i);
			if ( &doc != NULL ){
				char_t* token = stringToken(retfields, toks);
				while ( token != NULL ){
					const char_t* f = doc.get(token);
					_cout << token << _T(": ") << f << endl;
					token = stringToken( NULL, toks );
				}
			}
		}

		delete &h;
		delete &q;
		s.close();
		r.close();
		delete &r;

		delete[] fields;
		delete[] retfields;
		delete[] query;
		delete[] target;

		return 0;
	}
	return 1;
}
