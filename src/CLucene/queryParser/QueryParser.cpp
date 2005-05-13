#include "CLucene/StdHeader.h"
#include "QueryParser.h"

#include "QueryParserConstants.h"
#include "CLucene/analysis/AnalysisHeader.h"
#include "CLucene/util/Reader.h"
#include "CLucene/search/SearchHeader.h"
#include "CLucene/index/Term.h"

#include "CLucene/search/WildcardQuery.h"
#include "CLucene/search/FuzzyQuery.h"
#include "CLucene/search/PrefixQuery.h"

#include "TokenList.h"
#include "QueryToken.h"
#include "QueryParserBase.h"
#include "Lexer.h"

using namespace lucene::util;
using namespace lucene::index;
using namespace lucene::analysis;
using namespace lucene::search;

namespace lucene { namespace queryParser {

	QueryParser::QueryParser(const char_t* _field, Analyzer& _analyzer) : analyzer(_analyzer){
	//Func - Constructor.
	//       Instantiates a QueryParser for the named field _field
	//Pre  - _field != NULL
	//Post - An instance has been created

		CND_PRECONDITION(_field != NULL, "_field is NULL")

		field = stringDuplicate( _field);
		tokens = NULL;
	}

	QueryParser::~QueryParser() {
	//Func - Destructor
	//Pre  - true
	//Post - The instance has been destroyed

		_DELETE_ARRAY(field);
		_DELETE(tokens);
	}

	Query& QueryParser::Parse(const char_t* query, const char_t* field, Analyzer& analyzer){
	//Func - Static Method
	//       Returns a new instance of the Query class with a specified query, field and
	//       analyzer values.
	//Pre  - query != NULL and holds the query to parse
	//       field != NULL and holds the default field for query terms
	//       analyzer holds a valid reference to an Analyzer and is used to
	//       find terms in the query text
	//Post - query has been parsed and an instance of Query has been returned

		CND_PRECONDITION(query != NULL, "query is NULL")
		CND_PRECONDITION(field != NULL, "field is NULL")

		QueryParser parser(field, analyzer);
		return parser.Parse(query);
	}

	Query& QueryParser::Parse(const char_t* query){
	//Func - Returns a parsed Query instance
	//Pre  - query != NULL and contains the query value to be parsed
	//Post - Returns a parsed Query Instance

		CND_PRECONDITION(query != NULL, "query is NULL")

		//Instantie a Stringer that can read the query string
		Reader* r = new StringReader(query);

		//Check to see if r has been created properly
		CND_CONDITION(r != NULL, "Could not allocate memory for StringReader r")

		//Pointer for the return value
		Query* ret = NULL;

		_TRY{
			//Parse the query managed by the StringReader R and return a parsed Query instance
			//into ret
			ret = &Parse(*r);
			}
		_FINALLY (
			_DELETE(r);
		);

		//Check to if the ret points to a valid instance of Query
		CND_CONDITION(ret != NULL, "ret is NULL")

		return *ret;
	}

	Query& QueryParser::Parse(Reader& reader){
	//Func - Returns a parsed Query instance
	//Pre  - reader contains a valid reference to a Reader and manages the query string
	//Post - A parsed Query instance has been returned or

		//instantiate the TokenList tokens
		tokens = new TokenList;

		//Instantiate a lexer
		Lexer lexer(reader);

		//tokens = &lexer.Lex();
		//Lex the tokens
		lexer.Lex(tokens);

		//Peek to the first token and check if is an EOF
		if (tokens->Peek().Type == lucene::queryParser::EOF_){
			// The query string failed to yield any tokens.  We discard the
			// TokenList tokens and raise an exceptioin.
			_DELETE(tokens);
		   _THROWC("No query given.");
		}

		//Return the parsed Query instance
		return *MatchQuery(field);
	}

	int_t QueryParser::MatchConjunction(){
	//Func - matches for CONJUNCTION
	//       CONJUNCTION ::= <AND> | <OR>
	//Pre  - tokens != NULL
	//Post - if the first token is an AND or an OR then
	//       the token is extracted and deleted and CONJ_AND or CONJ_OR is returned
	//       otherwise CONJ_NONE is returned

		CND_PRECONDITION(tokens != NULL, "tokens is NULL")

		switch(tokens->Peek().Type){
			case lucene::queryParser::AND_ :{
				//Delete the first token of tokenlist
				ExtractAndDeleteToken();
				return CONJ_AND;
				}
			case lucene::queryParser::OR   :{
				//Delete the first token of tokenlist
				ExtractAndDeleteToken();
				return CONJ_OR;
				}
			default : {
				return CONJ_NONE;
				}
			}
	}

	int_t QueryParser::MatchModifier(){
	//Func - matches for MODIFIER
	//       MODIFIER ::= <PLUS> | <MINUS> | <NOT>
	//Pre  - tokens != NULL
	//Post - if the first token is a PLUS the token is extracted and deleted and MOD_REQ is returned
	//       if the first token is a MINUS or NOT the token is extracted and deleted and MOD_NOT is returned
	//       otherwise MOD_NONE is returned

		CND_PRECONDITION(tokens != NULL, "tokens is NULL")

		switch(tokens->Peek().Type){
			case lucene::queryParser::PLUS :{
				//Delete the first token of tokenlist
				ExtractAndDeleteToken();
				return MOD_REQ;
				}

			case lucene::queryParser::MINUS :
			case lucene::queryParser::NOT   :{
				//Delete the first token of tokenlist
				ExtractAndDeleteToken();
				return MOD_NOT;
				}
			default :{
				return MOD_NONE;
				}
			}
	}

	Query* QueryParser::MatchQuery(const char_t* field){
	//Func - matches for QUERY
	//       QUERY ::= [MODIFIER] QueryParser::CLAUSE (<CONJUNCTION> [MODIFIER] CLAUSE)*
	//Pre  - field != NULL
	//Post -

		CND_PRECONDITION(tokens != NULL, "tokens is NULL")

		VoidList<BooleanClause*> clauses;

		Query* q = NULL;
		Query* firstQuery = NULL;

		int_t mods = MOD_NONE;
		int_t conj = CONJ_NONE;

		//match for MODIFIER
		mods = MatchModifier();

		//match for CLAUSE
		q = MatchClause(field);

		AddClause(clauses, CONJ_NONE, mods, q);

		if(mods == MOD_NONE){
			firstQuery = q;
			}

		// match for CLAUSE*
		while(true){
			if(tokens->Peek().Type == lucene::queryParser::EOF_){
				delete MatchQueryToken(lucene::queryParser::EOF_);
				break;
				}

			if(tokens->Peek().Type == lucene::queryParser::RPAREN){
				//MatchQueryToken(lucene::queryParser::RPAREN);
				break;
				}

			//match for a conjuction (AND OR NOT)
			conj = MatchConjunction();
			//match for a modifier
			mods = MatchModifier();

			q = MatchClause(field);
			AddClause(clauses, conj, mods, q);
			}

		// finalize query
		if(clauses.size() == 1 && firstQuery != NULL){
			BooleanClause* c = clauses[0];

			//Condition check to be sure clauses[0] is valid
			CND_CONDITION(c != NULL, "c is NULL")

			//Tell the boolean clause not to delete its query
			c->deleteQuery=false;
			//Clear the clauses list
			clauses.clear();
			_DELETE(c);
			return firstQuery;
			}
		else{
			BooleanQuery* query = new BooleanQuery();
			//Condition check to see if query has been allocated properly
			CND_CONDITION(query != NULL, "No memory could be allocated for query")

			//iterate through all the clauses
			for( uint_t i=0;i<clauses.size();i++ ){
				//Condition check to see if clauses[i] is valdid
				CND_CONDITION(clauses[i] != NULL, "clauses[i] is NULL")
				//Add it to query
				query->add(*clauses[i]);
				}
			return query;
			}
	}

	Query* QueryParser::MatchClause(const char_t* field){
	//Func - matches for CLAUSE
	//       CLAUSE ::= [TERM <COLONQueryParser::>] ( TERM | (<LPAREN> QUERY <RPAREN>))
	//Pre  - field != NULL
	//Post -

		CND_PRECONDITION(field != NULL, "field is NULL")

		Query* q = NULL;
		const char_t* sfield = field;
		bool delField = false;

		QueryToken *DelToken = NULL;

		//match for [TERM <COLON>]
		QueryToken* term = tokens->Extract();
		if(term->Type == lucene::queryParser::TERM && tokens->Peek().Type == lucene::queryParser::COLON){
			DelToken = MatchQueryToken(lucene::queryParser::COLON);

			CND_CONDITION(DelToken != NULL,"DelToken is NULL")
			_DELETE(DelToken);

			sfield = stringDuplicate(term->Value);
			delField= true;
			_DELETE(term);
			}
		else{
			tokens->Push(term);
			}

		// match for
		// TERM | (<LPAREN> QUERY <RPAREN>)
		if(tokens->Peek().Type == lucene::queryParser::LPAREN){
			DelToken = MatchQueryToken(lucene::queryParser::LPAREN);

			CND_CONDITION(DelToken != NULL,"DelToken is NULL")
			_DELETE(DelToken);

			q = MatchQuery(sfield);
			//DSR:2004.11.01:
			//If exception is thrown while trying to match trailing parenthesis,
			//need to prevent q from leaking.

			try{
			   DelToken = MatchQueryToken(lucene::queryParser::RPAREN);

			   CND_CONDITION(DelToken != NULL,"DelToken is NULL")
			   _DELETE(DelToken);

			   }
			catch(...) {
				_DELETE(q);
				throw;
				}
			}
		else{
			q = MatchTerm(sfield);
			}

		if (delField){
			_DELETE_ARRAY(sfield);
			}

	  return q;
	}


	Query* QueryParser::MatchTerm(const char_t* field){
	//Func - matches for TERM
	//       TERM ::= TERM | PREFIXTERM | WILDTERM | NUMBER
	//                [ <FUZZY> ] [ <CARAT> <NUMBER> [<FUZZY>]]
	//			      | (<RANGEIN> | <RANGEEX>) [<CARAT> <NUMBER>]
	//			      | <QUOTED> [SLOP] [<CARAT> <NUMBER>]
	//Pre  - field != NULL
	//Post -

		CND_PRECONDITION(field != NULL,"field is NULL")

		QueryToken* term = NULL;
		QueryToken* slop = NULL;
		QueryToken* boost = NULL;

		bool prefix = false;
		bool wildcard = false;
		bool fuzzy = false;
		bool rangein = false;
		Query* q = NULL;

		term = tokens->Extract();

		QueryToken* DelToken = NULL; //Token that is about to be deleted

		switch(term->Type){
			case lucene::queryParser::TERM:

			case lucene::queryParser::NUMBER:

#ifndef NO_PREFIX_QUERY
			case lucene::queryParser::PREFIXTERM:
#endif

#ifndef NO_WILDCARD_QUERY
			case lucene::queryParser::WILDTERM:{
#endif
				//Check if type of QueryToken term is a prefix term
				if(term->Type == lucene::queryParser::PREFIXTERM){
					prefix = true;
					}

				//Check if type of QueryToken term is a wildcard term
				if(term->Type == lucene::queryParser::WILDTERM){
					wildcard = true;
					}

#ifndef NO_FUZZY_QUERY
				//Peek to see if the type of the next token is fuzzy term
				if(tokens->Peek().Type == lucene::queryParser::FUZZY){
					DelToken = MatchQueryToken(lucene::queryParser::FUZZY);

					CND_CONDITION(DelToken !=NULL, "DelToken is NULL")
					_DELETE(DelToken);

					fuzzy = true;
					}
#endif
				if(tokens->Peek().Type == lucene::queryParser::CARAT){
					DelToken = MatchQueryToken(lucene::queryParser::CARAT);

					CND_CONDITION(DelToken !=NULL, "DelToken is NULL")
					_DELETE(DelToken);

					boost = MatchQueryToken(lucene::queryParser::NUMBER);
#ifndef NO_FUZZY_QUERY
				   if(tokens->Peek().Type == lucene::queryParser::FUZZY){
					   DelToken = MatchQueryToken(lucene::queryParser::FUZZY);

					   CND_CONDITION(DelToken !=NULL, "DelToken is NULL")
					   _DELETE(DelToken);

					   fuzzy = true;
					   }
#endif
				   }

				if (false){
					}
#ifndef NO_WILDCARD_QUERY
				else
				if(wildcard){
					Term* t = new Term(field, term->Value);
					q = new WildcardQuery(t);
					t->finalize();
					break;
					}
#endif
#ifndef NO_PREFIX_QUERY
				else
				if(prefix){
					//Create a PrefixQuery
					q = GetPrefixQuery(field,term->Value);
					break;
					}
#endif
#ifndef NO_FUZZY_QUERY
				else
				if(fuzzy){
					//Create a FuzzyQuery
					q = GetFuzzyQuery(field,term->Value);
					break;
					}
#endif
				else{
					q = GetFieldQuery(field, analyzer, term->Value);
					break;
					}
			}//case lucene::queryParser::TERM:
			 //case lucene::queryParser::NUMBER:
			 //case lucene::queryParser::PREFIXTERM:
			 //case lucene::queryParser::WILDTERM:

#ifndef NO_RANGE_QUERY
			case lucene::queryParser::RANGEIN:
			case lucene::queryParser::RANGEEX:{

				if(term->Type == lucene::queryParser::RANGEIN){
					rangein = true;
					}

				if(tokens->Peek().Type == lucene::queryParser::CARAT){
					DelToken = MatchQueryToken(lucene::queryParser::CARAT);

					CND_CONDITION(DelToken !=NULL, "DelToken is NULL")
					_DELETE(DelToken);

					boost = MatchQueryToken(lucene::queryParser::NUMBER);
					}

				char_t* tmp = stringDuplicate ( term->Value +1);
				tmp[stringLength(tmp)-1] = 0;
				q = GetRangeQuery(field, analyzer,	tmp, rangein);
				_DELETE_ARRAY(tmp);
				break;
				} //case lucene::queryParser::RANGEIN:
				  //case lucene::queryParser::RANGEEX:
#endif
			case lucene::queryParser::QUOTED:{
				if(tokens->Peek().Type == lucene::queryParser::SLOP){
					slop = MatchQueryToken(lucene::queryParser::SLOP);
					}

				if(tokens->Peek().Type == lucene::queryParser::CARAT){
					DelToken = MatchQueryToken(lucene::queryParser::CARAT);

					CND_CONDITION(DelToken !=NULL, "DelToken is NULL")
					_DELETE(DelToken);

					boost = MatchQueryToken(lucene::queryParser::NUMBER);
					}

				//todo: check term->Value+1
				q = GetFieldQuery(field, analyzer, term->Value+1);
				if(slop != NULL && q != NULL && q->instanceOf(_T("PhraseQuery")) ){
					try {
						int_t s = stringToInteger(slop->Value+1);
						((PhraseQuery*)q)->setSlop( s );
						}
					catch(...){
						//ignored
						}
					_DELETE(slop);
					}
				break;
				} //case lucene::queryParser::QUOTED:
		} // end of switch

		_DELETE(term);

		if(boost != NULL){
			float_t f = 1.0F;
			try {
				//TODO: check this
				char_t* tmp;
				f = stringToFloat(boost->Value, &tmp);
				//f = Single.Parse(boost->Value, NumberFormatInfo.InvariantInfo);
				}
			catch(...){
				//ignored
				}
			_DELETE(boost);

			q->setBoost( f);
			}

		return q;
	}

	QueryToken* QueryParser::MatchQueryToken(QueryTokenTypes expectedType){
	//Func - matches for QueryToken of the specified type and returns it
	//       otherwise Exception throws
	//Pre  - tokens != NULL
	//Post -

		CND_PRECONDITION(tokens != NULL,"tokens is NULL")

		if(tokens->Count() == 0){
			QueryParserBase::throwParserException(_T("Error: Unexpected end of program"),' ',0,0);
			}

	  //Extract a token form the TokenList tokens
	  QueryToken* t = tokens->Extract();
	  //Check if the type of the token t matches the expectedType
	  if (expectedType != t->Type){
		  char_t buf[200];
		  stringPrintF(buf,_T("Error: Unexpected QueryToken: %d, expected: %d"),t->Type,expectedType);
		  _DELETE(t);
		  QueryParserBase::throwParserException(buf,' ',0,0);
		  }

	  //Return the matched token
	  return t;
	}

	void QueryParser::ExtractAndDeleteToken(void){
	//Func - Extracts the first token from the Tokenlist tokenlist
	//       and destroys it
	//Pre  - true
	//Post - The first token has been extracted and destroyed

		CND_PRECONDITION(tokens != NULL, "tokens is NULL")

		QueryToken* Token = NULL;

		//Extract the token from the TokenList tokens
		Token = tokens->Extract();
		//Condition Check Token may not be NULL
		CND_CONDITION(Token != NULL, "Token is NULL")
		//Delete Token
		_DELETE(Token);
	}

}}
