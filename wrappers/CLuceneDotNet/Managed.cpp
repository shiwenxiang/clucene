#include "Stdafx.h"
#ifndef _lucene_managed_Managed
#define _lucene_managed_Managed

#include "CLucene/StdHeader.h"
using namespace System;

namespace lucene { namespace util {

	class Managed{
	public:
		static char_t* toString ( String* value ){
			char_t* ret = new char_t[value->Length+1];
			for ( int_t i=0;i<value->Length;i++ ){
				if ( value->get_Chars(i) != 0 )
					ret[i] = value->get_Chars(i);
				else
					ret[i] = '?'; //TODO better way???
			}
			ret[value->Length] = 0;

			return ret;
		}
	};

}}
#endif
