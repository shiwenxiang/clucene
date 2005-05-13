#include "CLucene/StdHeader.h"
#ifndef NO_PREFIX_QUERY
#include "PrefixQuery.h"
/*
- reader set to NULL in constructor
- delete query protected in destructor
- delete query in prepare


*/
using namespace lucene::index;
namespace lucene{ namespace search{

  PrefixQuery::PrefixQuery(Term* Prefix){
  //Func - Constructor.
  //       Constructs a query for terms starting with prefix
  //Pre  - Prefix != NULL 
  //Post - The instance has been created

      //Get a pointer to Prefix
      prefix = Prefix->pointer();

      //Initialize query to NULL so that we can safely delete query in the destructor.
      query  = NULL;
      reader = NULL;
  }

  PrefixQuery::~PrefixQuery(){
  //Func - Destructor
  //Pre  - true
  //Post - The instance has been destroyed.
    
      //Delete prefix by finalizing it
      prefix->finalize();

	  //delete the query
	  _DELETE(query);
  }

  const char_t* PrefixQuery::getQueryName()const{
  //Func - Returns the name "PrefixQuery" 
  //Pre  - true
  //Post - The string "PrefixQuery" has been returned

      return _T("PrefixQuery");
  }

  void PrefixQuery::prepare(IndexReader& reader) {
  //Func - Resets the PrefixQuery.
  //Pre  - read holds a valid reference to an IndexReader
  //Post - if pre(query) != NULL then it has been deleted 
  //       query = null and this->reader points to reader

      //Check if query is NULL if this is not the case then have query deleted, 
	  // otherwise the memory currently allocated by query will be leaked!
	  _DELETE(query);
      this->reader = &reader;
  }

  float_t PrefixQuery::sumOfSquaredWeights(Searcher& searcher){
  //Func - Sums the squard weights of the rewritten Prefix Query
  //Pre  - searcher holds a valid reference to a Searcher
  //Post - The sum of squard weights of the rewritten PrefixQuery is returned

      return getQuery()->sumOfSquaredWeights( searcher );
  }

  void PrefixQuery::normalize(const float_t norm) {
  //Func - Normalizes the prefix query
  //Pre  - norm con
  //Post - The rewritten Prefix query has been normalized
      getQuery()->normalize(norm);
  }

  Scorer* PrefixQuery::scorer(IndexReader& reader) {
  //Func -
  //Pre  -
  //Post -
      
	  return getQuery()->scorer(reader);
  }

  BooleanQuery* PrefixQuery::getQuery() {
  //Func - Rewrites the PrefixQuery as an alternate, more primitive query.
  //Pre  - query = NULL or query != NULL
  //       reader != NULL
  //Post - if pre(query) != NULL then it has been returned
  //       if pre(query) == NULL then the

      //Check if query equals NULL
      if (query == NULL) {
		  //Instantiate a Boolean query so Prefix can rewritten as a Boolean query
          BooleanQuery* q = new BooleanQuery();

          //Retrieve an enumeration of terms from the reader using the prefix 
          TermEnum& _enum = reader->getTerms(prefix);
          _TRY {
			  //Get the text of the prefix term
              const char_t* prefixText = prefix->Text();
			  //Get the field name of the prefix term
              const char_t* prefixField = prefix->Field();

			  //Iterate through the enumerate terms (_enum) 
              do {
                  //Retrieve a term but order getTerm not to return reference ownership. This prevents a memory leak
                  Term* term = _enum.getTerm(false);

                  if (term != NULL && 
					  //Does text of term start with prefixText?
					  stringFind(term->Text(),prefixText)==term->Text() &&
					  //Are the field names of term and prefixField equeal?
                      stringCompare(term->Field(), prefixField)==0 ){

                          //Found a match

						  //Instantiate a TermQuery based on the found term
                          TermQuery* tq = new TermQuery(*term);	  
						  //Set the boost
                          tq->setBoost(boost);			          
						  //Add the new TermQuery to the boolean query q
                          q->add(*tq,true, false, false);		  
                          } 
                   else{
                      break;
                      }
                  } 
			  while (_enum.next());
              }
		  _FINALLY (
			  //close the enumeration and delete teh enumeration
              _enum.close();
              delete &_enum;
              );

		//store query q into query
        query = q;
    }
    return query;
  }

  const char_t* PrefixQuery::toString(const char_t* field) {
  //Func - Creates a user-readable version of this query and returns it as as string
  //Pre  - field != NULL
  //Post - a user-readable version of this query has been returned as as string

	  //Instantiate a stringbuffer buffer to store the readable version temporarily
      lucene::util::StringBuffer buffer;
	  //check if field equal to the field of prefix
      if( stringCompare(prefix->Field(),field) != 0 ) {
		  //Append the field of prefix to the buffer
          buffer.append(prefix->Field());
		  //Append a colon
          buffer.append(_T(":") );
    }
    //Append the text of the prefix
    buffer.append(prefix->Text());
	//Append a wildchar character
    buffer.append(_T("*"));
	//if the boost factor is not eaqual to 1
    if (boost != 1.0f) {
		//Append ^
        buffer.append(_T("^"));
		//Append the boost factor
        buffer.append( boost,10);
    }
	//Convert StringBuffer buffer to char_t block and return it
    return buffer.ToString();
  }

}}
#endif
