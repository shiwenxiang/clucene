#include "Stdafx.h"
#ifndef _lucene_managed_search
#define _lucene_managed_search

#include "Clucene/document/Document.h"
#include "Clucene/queryParser/QueryParser.h"
#include "Search.h"
#include "Analysis.h"
#include "Managed.h"
#include "MDocument.h"
#include "CLucene/search/SearchHeader.h"
#include "CLucene/search/IndexSearcher.h"
#include "CLucene/index/IndexReader.h"
using namespace System;
using namespace lucene::util;
using namespace lucene::index;
using namespace lucene::queryParser;
using namespace lucene::document;

namespace lucene { namespace search {

	public __gc class MHits{
	public:
		Hits* __mp;
		MHits(Hits* _mp):
			__mp(_mp)
		{
		}

		MDocument* doc ( int_t n ){
			return new MDocument( &__mp->doc(n) );
		}
		
		float_t score ( int_t n ){
			return __mp->score(n);
		}

		int_t Length(){
			return __mp->Length();
		}
		~MHits(){
			//delete __mp; //TODO: deleting this causes error!!!
		}
	};




	public __gc class MQuery{
	public:
		Query* __mp;
		MQuery(Query* mp):
			__mp(mp)
		{
		}
		~MQuery(){
			delete __mp;
		}

		String* ToString(String* field){
			const char_t* fld = Managed::toString(field);
			const char_t* ret = __mp->toString(fld);

			String* str = new String(ret);
			delete[] ret;
			delete[] fld;

			return str;
		}
	};




	public __gc class MQueryParser{
	public:
		QueryParser* __mp;

		MQueryParser(String* field, lucene::analysis::MAnalyzer* analyzer){
			char_t* fld = Managed::toString(field);
			__mp = new QueryParser(fld,*analyzer->__mp);
			delete[] fld;
		}

		MQuery* Parse ( String* query ){
			char_t* qry = Managed::toString(query);
			MQuery* ret = new MQuery ( &__mp->Parse(qry) );
			delete[] qry;

			return ret;
		}


		static MQuery* Parse ( String* query, String* field, lucene::analysis::MAnalyzer* analyzer){
			MQueryParser* p = new MQueryParser(field,analyzer);
			return p->Parse(query);
		}

		static MQuery* Parse ( String* query, String* field ){
			return Parse ( query, field, new lucene::analysis::MStandardAnalyzer() );
		}

		~MQueryParser(){
			delete __mp;
		}
	};







	
	public __gc class MIndexSearcher{
	private:
		IndexReader* __rd;
	public:
		Searcher* __mp;
		MIndexSearcher(String* path)
		{
			char_t* pth = Managed::toString(path);
			__rd = &IndexReader::open(pth);
			delete[] pth;

			__mp = new IndexSearcher(*__rd);

		}

		MHits* search ( MQuery* query ){
			return new MHits( &__mp->search(*query->__mp) );
		}

		~MIndexSearcher(){
			__mp->close();
			delete __mp;
			delete __rd;
		}
	};


}}
#endif
