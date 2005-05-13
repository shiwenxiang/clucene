#include "Stdafx.h"
#ifndef _lucene_managed_document
#define _lucene_managed_document

#include "Clucene/document/Document.h"
#include "Clucene/document/Field.h"
#include "Managed.h"
using namespace System;
using namespace lucene::util;

namespace lucene { namespace document {

	public __gc class MField{
	protected: 
		MField ( Field* pfield ):
			 __mp(pfield)
		{
		}

	public:
		Field* __mp;
	
		//MField( String name,String value,bool store, bool index,bool token ){
			//__mp = new Field(name.Chars,value.Chars, store, index,token);
		//}
		~MField(){
			delete __mp;
		}

		// Constructs a String-valued Field that is not tokenized, but is indexed
		//	and stored.  Useful for non-text fields, e.g. date or url.
		static MField* Keyword(String* Name, String* Value){
			char_t * name = Managed::toString(Name);
			char_t * value = Managed::toString(Name);
			MField* ret = new MField( &Field::Keyword(name,value) );
			delete[] name;
			delete[] value;
			return ret;
		}

		// Constructs a String-valued Field that is not tokenized nor indexed,
		//	but is stored in the index, for return with hits.
		static MField* UnIndexed(String* Name, String* Value){
			char_t * name = Managed::toString(Name);
			char_t * value = Managed::toString(Name);
			MField* ret = new MField( &Field::UnIndexed(name,value) );
			delete[] name;
			delete[] value;
			return ret;
		}

		//  Constructs a String-valued Field that is tokenized and indexed,
		//	and is stored in the index, for return with hits.  Useful for short text
		//	fields, like "title" or "subject". 
		static MField* Text(String* Name, String* Value){
			char_t * name = Managed::toString(Name);
			char_t * value = Managed::toString(Name);
			MField* ret = new MField( &Field::Text(name,value) );
			delete[] name;
			delete[] value;
			return ret;
		}

		//  Constructs a String-valued Field that is tokenized and indexed,
		//	but that is not stored in the index.
		static MField* UnStored(String* Name, String* Value){
			char_t * name = Managed::toString(Name);
			char_t * value = Managed::toString(Name);
			MField* ret = new MField( &Field::UnStored(name,value) );
			delete[] name;
			delete[] value;
			return ret;
		}
	};

	public __gc class MDocument{
	public:
		Document* __mp;
		MDocument(){
			__mp = new Document();
		}
		MDocument(Document* mp){
			__mp = mp;
		}
		String* toString(){
			const char_t* ret = __mp->toString();
			String* str = new String(ret);
			delete[] ret;
			
			return str;
		}
		void add(MField* field){
			__mp->add( *field->__mp );
		}

		String* get(String* field){
			char_t* fld = Managed::toString(field);
			String* ret = new String( __mp->get(fld) );
			delete[] fld;

			return ret;
		}
		~MDocument(){
			delete __mp;
		}
	};

}}
#endif
