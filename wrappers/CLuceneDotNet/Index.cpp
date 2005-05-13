#include "Stdafx.h"
#ifndef _lucene_managed_index
#define _lucene_managed_index

#include "Clucene/index/IndexWriter.h"
#include "Clucene/index/IndexReader.h"
#include "Managed.h"
using namespace System;
using namespace lucene::util;

namespace lucene { namespace index {


	public __gc class MIndexReader{
	public:
		IndexReader* __mp;

		MIndexReader( String* path ){
			char_t* p = Managed::toString(path);
			__mp = &IndexReader::open( p );
			delete[] p;

		}
		~MIndexReader(){
			delete __mp;
		}
	};

	/*
	public __gc class MIndexWriter{
	public:
		IndexWriter* __mp;

		MIndexWriter(){
			__mp = new IndexWriter(
			
		}
		~MIndexWriter(){
			delete __mp;
		}
	};*/



}}
#endif
