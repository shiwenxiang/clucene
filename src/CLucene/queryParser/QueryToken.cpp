#include "CLucene/StdHeader.h"
#include "QueryToken.h"

#include "QueryParserConstants.h"

namespace lucene{ namespace queryParser{

    QueryToken::QueryToken(const char_t* value, const int_t start, const int_t end, const QueryTokenTypes type):
    	Value( stringDuplicate(value) ),
    	Start(start),
    	End(end),
    	Type(type)
    {
    }
    
    QueryToken::~QueryToken(){
    //Func - Destructor
	//Pre  - true
	//Post - Instance has been destroyed

    	_DELETE_ARRAY(Value);
    }
    
    // Initializes a new instance of the Token class LUCENE_EXPORT.
    //
    QueryToken::QueryToken(const char_t* value, const QueryTokenTypes type):
    	Value(stringDuplicate(value)),
    	Start(0),
    	End(0),
    	Type(type)
    {
    	// use this
    }
    
    // Initializes a new instance of the Token class LUCENE_EXPORT.
    //
    QueryToken::QueryToken(const QueryTokenTypes type):
    	Value(NULL),
    	Start(0),
    	End(0),
    	Type(type)
    {
    	// use this
    }
    
    // Returns a string representation of the Token.
    //
    //QueryToken::ToString()
    //{
    //	return "<" + Type + "> " + Value;	
    //}
}}
