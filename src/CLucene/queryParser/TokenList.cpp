#include "CLucene/StdHeader.h"
#include "TokenList.h"

#include "CLucene/util/VoidMap.h"
#include "CLucene/util/VoidList.h"
#include "QueryToken.h"

namespace lucene{ namespace queryParser{
  
    TokenList::TokenList(){
    //Func - Constructor
    //Pre  - true
    //Post - Instance has been created
    }

    TokenList::~TokenList(){
    //Func - Destructor
	//Pre  - true
	//Post - The tokenlist has been destroyed

        tokens.setDoDelete(DELETE_TYPE_DELETE);
        tokens.clear();
    }

	void TokenList::Add(QueryToken* token){
    //Func - Adds a QueryToken token to the TokenList
	//Pre  - token != NULL
	//Post - token has been added to the token list

       CND_PRECONDITION(token != NULL,"token != NULL")

       tokens.push_front(token);
    }

    void TokenList::Push(QueryToken* token){
    //Func - 
	//Pre  - token != NULL
	//Post - 

      CND_PRECONDITION(token != NULL,"token is NULL") 

      tokens.push_back(token);
    }

    QueryToken& TokenList::Peek() {
      /* DSR:2004.11.01: Reverted my previous (circa April 2004) fix (which
      ** raised an exception if Peek was called when there were no tokens) in
      ** favor of returning the EOF token.  This solution is much better
      ** integrated with the rest of the code in the queryParser subsystem. */
      uint_t nTokens = tokens.size();
      if (nTokens == 0) {
        Push(new QueryToken(lucene::queryParser::EOF_));
        nTokens++;
      }
      return *tokens[nTokens-1];
    }

    QueryToken* TokenList::Extract(){
    //Func - Extract token from the TokenList
	//Pre  - true
	//Post - Retracted token has been returned

        QueryToken* token = &Peek();
		//Retract the current peeked token
        tokens.pop_back();

        return token;
    }

    int_t TokenList::Count()
    {
      return tokens.size();
    }
}}
