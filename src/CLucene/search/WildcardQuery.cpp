#include "CLucene/StdHeader.h"
#ifndef NO_WILDCARD_QUERY
#include "WildcardQuery.h"

using namespace lucene::index;
namespace lucene{ namespace search {

  WildcardQuery::WildcardQuery(Term* term): MultiTermQuery( term ){
  //Func - Constructor
  //Pre  - term != NULL
  //Post - Instance has been created

      CND_PRECONDITION(term != NULL,"term is NULL")
      //Get the pointer of term and increase the reference counter of term
      wildcardTerm = term->pointer();
  }

  WildcardQuery::~WildcardQuery(){
  //Func - Destructor
  //Pre  - true
  //Post - true

      wildcardTerm->finalize();
  }

  const char_t* WildcardQuery::getQueryName() const{
  //Func - Returns the string "WildcardQuery"
  //Pre  - true
  //Post - The string "WildcardQuery" has been returned

      return _T("WildcardQuery");
  }

  void WildcardQuery::prepare(IndexReader& reader) {
  //Func - Pepares the WildcardQuery with reader
  //Pre  - reader is a valid reference to an IndexReader
  //Post - The WildcardQuery has been prepared
      
	  //Create a WildcardTermEnum
      FilteredTermEnum* w = new WildcardTermEnum(reader, wildcardTerm);

      CND_CONDITION(w != NULL,"w is NULL")
      //Set the enumeration
      MultiTermQuery::setEnum( w );
  }

  const char_t* WildcardQuery::toString(const char_t* field){
  //Func - Returns the query as a human readable string
  //Pre  - field != NULL
  //Post - The query has been returned as human readable string

	  CND_PRECONDITION(field != NULL,"field is NULL")

      lucene::util::StringBuffer buffer;
      if ( stringCompare(wildcardTerm->Field(),field)!= 0 ) {
          buffer.append(wildcardTerm->Field());
          buffer.append(_T(":"));
          }
      buffer.append(wildcardTerm->Text());
      if (boost != 1.0f) {
          buffer.append(_T("^"));
          buffer.append( boost,10 );
          }
      return buffer.ToString();
  }
}}
#endif
