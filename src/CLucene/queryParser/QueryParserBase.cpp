#include "CLucene/StdHeader.h"
#include "QueryParserBase.h"

#include "CLucene/search/BooleanClause.h"
#include "CLucene/util/VoidList.h"
#include "CLucene/analysis/AnalysisHeader.h"
#include "CLucene/analysis/Analyzers.h"
#include "CLucene/index/Term.h"
#include "CLucene/search/TermQuery.h"
#include "CLucene/search/PhraseQuery.h"
#include "CLucene/search/RangeQuery.h"

#include "CLucene/analysis/standard/StandardFilter.h"
#include "CLucene/analysis/standard/StandardTokenizer.h"


using namespace lucene::search;
using namespace lucene::util;
using namespace lucene::analysis;
using namespace lucene::analysis::standard;
using namespace lucene::index;

namespace lucene{ namespace queryParser{

	QueryParserBase::QueryParserBase(){
	//Func - Constructor
	//Pre  - true
	//Post - instance has been created with PhraseSlop = 0

		PhraseSlop = 0;
		lowercaseExpandedTerms = true;
	}

	QueryParserBase::~QueryParserBase(){
	//Func - Destructor
	//Pre  - true
	//Post - The instance has been destroyed
	}

	void QueryParserBase::setLowercaseExpandedTerms(bool LowercaseExpandedTerms){
	//Func - Sets the the flag lowercaseExpandedTerms
	//Pre  - true
	//Post - lowercaseExpandedTerms contains the same value as LowercaseExpandedTerms

		lowercaseExpandedTerms = LowercaseExpandedTerms;
	}

	bool QueryParserBase::getLowercaseExpandedTerms() {
	//Func - Returns the value of lowercaseExpandedTerms
	//Pre  - true
	//Post - The value of lowercaseExpandedTerms has been returned

		return lowercaseExpandedTerms;
  }

	void QueryParserBase::AddClause(VoidList<BooleanClause*>& clauses, int_t conj, int_t mods, Query* q){
	//Func - Adds the next parsed clause.
	//Pre  -
	//Post -

	  bool required;
	  bool prohibited;

	  // If this term is introduced by AND, make the preceding term required,
	  // unless it's already prohibited.
	  if (conj == CONJ_AND) {
		// 2004.11.01: SF 1051984:
		// If attempting to parse a query such as "a AND b", where the first
		// clause is a stopword (and thus, is filtered out), the following code
		// assumed when it reached the second clause that the previous clause
		// had not been filtered out, so it undershot clauses and crashed.
		const uint_t nPreviousClauses = clauses.size();
		if (nPreviousClauses > 0) {
		  BooleanClause* c = clauses[nPreviousClauses-1];
		  if (!c->prohibited)
			c->required = true;
		}
	  }

	  // We might have been passed a NULL query; the term might have been
	  // filtered away by the analyzer.
	  if (q == NULL)
		return;

	  // We set REQUIRED if we're introduced by AND or +; PROHIBITED if
	  // introduced by NOT or -; make sure not to set both.
	  prohibited = (mods == MOD_NOT);
	  required = (mods == MOD_REQ);
	  if (conj == CONJ_AND && !prohibited)
		required = true;

	  clauses.push_back(new BooleanClause(*q,true, required, prohibited));
	}

	void QueryParserBase::throwParserException(const char_t* message, char_t ch, int_t col, int_t line )
	{
		char_t msg[1024];
		stringPrintF(msg,message,ch,col,line);
		_THROWX ( msg );
	}

	Query* QueryParserBase::GetFieldQuery(const char_t* field, Analyzer& analyzer, const char_t* queryText){
	//Func - Returns a query for the specified field.
	//       Use the analyzer to get all the tokens, and then build a TermQuery,
	//       PhraseQuery, or nothing based on the term count
	//Pre  - field != NULL
	//       analyzer contains a valid reference to an Analyzer
	//       queryText != NULL and contains the query
	//Post - A query instance has been returned for the specified field

		CND_PRECONDITION(field != NULL,"field is NULL")
		CND_PRECONDITION(queryText != NULL,"queryText is NULL")

		//Instantiate a stringReader for queryText
		StringReader reader(queryText);

		TokenStream* source = &analyzer.tokenStream(field, &reader);

		CND_CONDITION(source != NULL,"source is NULL")

		StringArrayConst v;

		v.setDoDelete(DELETE_TYPE_DELETE_ARRAY);

		Token* t = NULL;
		//Get the tokens from the source
		while (true){
		   try{
			   t = source->next();
			   }
		   catch(...){
			   t = NULL;
			   }

		   if (t == NULL)
			  break;

			v.push_back(stringDuplicate(t->TermText()));
			_DELETE(t);
			}

		//Check if there are any tokens retrieved
		if (v.size() == 0){
			_DELETE(source);
			return NULL;
			}
		else{
			if (v.size() == 1){
				Term* t = new Term(field, v[0]);
				Query* ret = new TermQuery( *t );
				t->finalize();

				_DELETE(source);
				return ret;
				}
			else{
				PhraseQuery* q = new PhraseQuery;
				q->setSlop(PhraseSlop);

				StringArrayConstIterator itr = v.begin();
				while ( itr != v.end() ){
					const char_t* data = *itr;
					Term* t = new Term(field, data);
					q->add(t);
					t->finalize();
					itr++;
					}
				_DELETE(source);
				return q;
				}
			}
	}

#ifndef NO_RANGE_QUERY
	// Returns a range query.
	Query* QueryParserBase::GetRangeQuery(const char_t* field, Analyzer& analyzer, const char_t* queryText, bool inclusive)
	{
	  // Use the analyzer to get all the tokens.  There should be 1 or 2.
	Reader* reader = new StringReader(queryText);
	  TokenStream& source = analyzer.tokenStream(field, reader);

	  Term* terms[2];
	  Token* t;

	  for (int_t i = 0; i < 2; i++)
	  {
		try
		{
		  t = source.next();
		}
		catch (...)
		{
		  t = NULL;
		}
		if (t != NULL)
		{
		  const char_t* text = t->TermText();
		  //if ( text.ToUpper() != "NULL")
		  //{
			terms[i] = new Term(field, text);
		  //}
		  delete t;
		}
		/* DSR:CL_BUG Unless terms[i] is set NULL, an uninitialized access can
		** be performed below. */
		else {
		  terms[i] = NULL;
		}
	  }

	  Query* ret = new RangeQuery(terms[0], terms[1], inclusive);
	  /* DSR:CL_BUG: Don't try to call finalize method if the Term* is NULL. */
	  if (terms[0] != NULL) terms[0]->finalize();
	  if (terms[1] != NULL) terms[1]->finalize();

	  delete &source;
	  delete reader;

	  return ret;
	}

	char_t *QueryParserBase::AnalyzeExpandedTerm(const char_t* field,const char_t* termStr){
	//Func - Analyzes the expanded term termStr with the StandardFilter and the
	//       LowerCaseFilter.
	//Pre  - field != NULL
	//       termStr != NULL
	//Post - The analyzed Expanded term is returned.

		CND_PRECONDITION(field != NULL,"field is NULL")
		CND_PRECONDITION(termStr != NULL,"termStr is NULL")

		//Instantiate a stringReader initialized with termStr
		StringReader *reader = new StringReader(termStr);

		CND_CONDITION(reader != NULL,"Could not allocate memory for StringReader reader")

		TokenStream* source = NULL;

		source = new StandardTokenizer(*reader);
		source = new StandardFilter(source,true);
		source = new LowerCaseFilter(source,true);

		CND_CONDITION(source != NULL,"source is NULL")

		Token* T = NULL;

		try{
		   T = source->next();
		   }
		catch (...){
			T = NULL;
			}

		_DELETE(source);
		_DELETE(reader);

		//Check if a token has been returned
		if (T == NULL){
			//No tokens so nothing
			return NULL;
			}

		//Get the text to return
		char_t* text = stringDuplicate(T->TermText());
		//Delete the token
		_DELETE(T);

		return text;
	}


#ifndef NO_PREFIX_QUERY
	Query* QueryParserBase::GetPrefixQuery(const char_t* field,const char_t* termStr){
	//Func - Factory method for generating a query. Called when parser parses
	//       an input term token that uses prefix notation; that is,
	//       contains a single '*' wildcard character as its last character.
	//       Since this is a special case of generic wildcard term,
	//       and such a query can be optimized easily, this usually results in a different
	//       query object.
	//
	//       Depending on settings, a prefix term may be lower-cased automatically.
	//       It will not go through the default Analyzer, however, since normal Analyzers are
	//       unlikely to work properly with wildcard templates. Can be overridden by extending
	//       classes, to provide custom handling for wild card queries, which may be necessary
	//       due to missing analyzer calls.
	//Pre  - field != NULL and field contains the name of the field that the query will use
	//       termStr != NULL and is the  token to use for building term for the query
	//       (WITH or WITHOUT a trailing '*' character!)
	//Post - A PrefixQuery instance has been returned

		CND_PRECONDITION(field != NULL,"field is NULL")
		CND_PRECONDITION(termStr != NULL,"termStr is NULL")

		char_t* queryText = stringDuplicate(termStr);

		if (lowercaseExpandedTerms){
			//Check if the last char is a *
			if(queryText[stringLength(queryText)-1] == '*'){
				//remove the *
				queryText[stringLength(queryText)-1] = '\0';
				}

			//LowerText the complete string
			const char_t* TempQueryText = (const char_t*) queryText;
			queryText = AnalyzeExpandedTerm(field,TempQueryText);
			_DELETE_ARRAY(TempQueryText);
			}

		Term* t = new Term(field, queryText);

		CND_CONDITION(t != NULL,"Could not allocate memory for term T")

		Query *q = new PrefixQuery(t);

		CND_CONDITION(q != NULL,"Could not allocate memory for PrefixQuery q")

		t->finalize();
		_DELETE_ARRAY(queryText);

		return q;
	}
#endif
#ifndef NO_FUZZY_QUERY
	Query* QueryParserBase::GetFuzzyQuery(const char_t* field,const char_t* termStr){
	//Func - Factory method for generating a query (similar to getPrefixQuery}). Called when parser parses
	//       an input term token that has the fuzzy suffix (~) appended.
	//Pre  - field != NULL and field contains the name of the field that the query will use
	//       termStr != NULL and is the  token to use for building term for the query
	//       (WITH or WITHOUT a trailing '*' character!)
	//Post - A FuzzyQuery instance has been returned

		CND_PRECONDITION(field != NULL,"field is NULL")
		CND_PRECONDITION(field != NULL,"field is NULL")

		char_t* queryText = stringDuplicate(termStr);

		if (lowercaseExpandedTerms){
			//Check if the last char is a ~
			if(queryText[stringLength(queryText)-1] == '~'){
				//remove the ~
				queryText[stringLength(queryText)-1] = '\0';
				}

			//LowerText the complete string
			const char_t* TempQueryText = (const char_t*) queryText;
			queryText = AnalyzeExpandedTerm(field,TempQueryText);
			_DELETE_ARRAY(TempQueryText);
			}

		Term* t = new Term(field, queryText);

		CND_CONDITION(t != NULL,"Could not allocate memory for term T")

		Query *q = new FuzzyQuery(t);

		CND_CONDITION(q != NULL,"Could not allocate memory for FuzzyQuery q")

		t->finalize();
		_DELETE_ARRAY(queryText);

		return q;
	}
#endif



#endif

}}
