#include "CLucene/StdHeader.h"
#include "CLucene/util/Reader.h"
#include "Field.h"
#include "CLucene/util/Misc.h"

namespace lucene{ namespace document{ 

  Field::Field(const char_t* Name, const char_t* String, bool store, bool index, bool token){
  //Func - Constructor
  //Pre  - Name != NULL and contains the name of the field
  //       String != NULL and contains the value of the field
  //       store indicates if the field must be stored
  //       index indicates if the field must be indexed
  //       token indicates if the field must be tokenized
  //Post - The instance has been created

      CND_PRECONDITION(Name != NULL,"Name is NULL")
      CND_PRECONDITION(String != NULL,"String is NULL")
		
      name        = stringDuplicate( Name );
      stringValue = stringDuplicate( String );
      readerValue = NULL;
          
      isStored    = store;
      isIndexed   = index;
      isTokenized = token;
  }

  Field::Field(const char_t* Name, lucene::util::Reader* reader){
  //Func - Constructor
  //Pre  - Name != NULL and contains the name of the field
  //       reader != NULL and contains a Reader
  //Post - The instance has been created with:
  //       isStored    = false
  //       isIndexed   = true
  //       isTokenized = true     
  //       meaning that the field only is indexed and tokenized

      CND_PRECONDITION(Name != NULL,"Name is NULL")
      CND_PRECONDITION(reader != NULL,"reader is NULL")
		
      name        = stringDuplicate( Name );
      stringValue = NULL;
      readerValue = reader;
        
      isStored    = false;
      isIndexed   = true;
      isTokenized = true;
  }

  Field::Field(const char_t* Name, lucene::util::Reader* reader, bool store, bool index, bool token){
 //Func - Constructor
  //Pre  - Name != NULL and contains the name of the field
  //       reader != NULL and contains a Reader
  //       store indicates if the field must be stored
  //       index indicates if the field must be indexed
  //       token indicates if the field must be tokenized
  //Post - The instance has been created

	  CND_PRECONDITION(Name != NULL,"Name is NULL")
      CND_PRECONDITION(reader != NULL,"reader is NULL")
		
      name        = stringDuplicate( Name );
      stringValue = NULL;
      readerValue = reader;
        
      isStored = store;
      isIndexed = index;
      isTokenized = token;
  }

  Field::~Field(){
  //Func - Destructor
  //Pre  - true
  //Post - Instance has been destroyed

      _DELETE_ARRAY(name);
      _DELETE_ARRAY(stringValue);
	  _DELETE(readerValue);
  }

  /*static*/
  Field& Field::Keyword(char_t* Name, const char_t* Value) {
    return *new Field(Name, Value, true, true, false);
  }

  /*static*/
  Field& Field::UnIndexed(char_t* Name, const char_t* Value) {
    return *new Field(Name, Value, true, false, false);
  }

  /*static*/
  Field& Field::Text(char_t* Name, const char_t* Value) {
    return *new Field(Name, Value, true, true, true);
  }

  /*static*/
  Field& Field::UnStored(char_t* Name, const char_t* Value) {
      return *new Field(Name, Value, false, true, true);
  }

  /*static*/
  Field& Field::Text(char_t* Name, lucene::util::Reader* Value) {
      return *new Field(Name, Value);
  }



	/*===============FIELDS=======================*/
	char_t* Field::Name() 		{ return name; }
	const char_t* Field::StringValue()		{ return stringValue; }
	lucene::util::Reader* Field::ReaderValue()	{ return readerValue; }

	bool	Field::IsStored() 	{ return isStored; }
	bool 	Field::IsIndexed() 	{ return isIndexed; }
	bool 	Field::IsTokenized() 	{ return isTokenized; }

	char_t* Field::toString() {
		if (isStored && isIndexed && !isTokenized)
			return lucene::util::Misc::join(_T("Keyword{"), name, _T(":"), (stringValue==NULL?_T("Reader"):stringValue), _T("}"));
		else if (isStored && !isIndexed && !isTokenized)
			return lucene::util::Misc::join(_T("Unindexed{"), name, _T(":"), (stringValue==NULL?_T("Reader"):stringValue), _T("}"));
		else if (isStored && isIndexed && isTokenized )
			return lucene::util::Misc::join(_T("Text{"), name, _T(":"), (stringValue==NULL?_T("Reader"):stringValue), _T("}"));
		else
			return lucene::util::Misc::join(_T("Field Object ("), name, _T(")") );
	}

}}
