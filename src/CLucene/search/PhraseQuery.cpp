#include "CLucene/StdHeader.h"
#include "PhraseQuery.h"

#include "SearchHeader.h"
#include "Scorer.h"
#include "BooleanQuery.h"
#include "TermQuery.h"

#include "CLucene/index/Term.h"
#include "CLucene/index/Terms.h"
#include "CLucene/index/IndexReader.h"

#include "CLucene/util/StringBuffer.h"
#include "CLucene/util/VoidList.h"

#include "ExactPhraseScorer.h"
#include "SloppyPhraseScorer.h"

using namespace lucene::index;
using namespace lucene::util;
namespace lucene{ namespace search{

  PhraseQuery::PhraseQuery(){
  //Func - Constructor
  //Pre  - true
  //Post - An empty PhraseQuery has been created
  
      idf    = 0.0f;
      weight = 0.0f;
      slop   = 0;

	  field = NULL;
  }

  PhraseQuery::~PhraseQuery(){
  //Func - Destructor
  //Pre  - true
  //Post 0 The instance has been destroyed
      
	  //Iterate through all the terms
	  for (uint_t i = 0; i < terms.size(); i++){
        terms[i]->finalize();
        }
  }

  const char_t* PhraseQuery::getQueryName() const{
  //Func - Returns the string "PhraseQuery"
  //Pre  - true
  //Post - The string "PhraseQuery" has been returned

      return _T("PhraseQuery");
  }

  bool PhraseQuery::add(Term* term) {
  //Func - Adds a term to the end of the query phrase. 
  //Pre  - term != NULL 
  //Post - The term has been added if its field matches the field of the PhraseQuery
  //       and true is returned otherwise false is returned
    
      CND_PRECONDITION(term != NULL,"term is NULL")

      if (terms.size() == 0){
          field = term->Field();
          }
	  else{
		  //Check if the field of the new term matches the field of the PhraseQuery
		  if (stringCompare(term->Field(), field) != 0){
			  return false;
		      }
          }
    //Store the new term
    terms.push_back(term->pointer());

	return true;
  }

  float_t PhraseQuery::sumOfSquaredWeights(Searcher& searcher) {
  //Func - Returns the sum of squared weights 
  //Pre  - searcher is reference to a valid Searcher
  //Post - The sum of squared weights times the boost factor has been returned
     
	  Term *T = NULL;

      //Iiterate through all terms and sum the IDFs
      for (uint_t i = 0; i < terms.size(); i++){
		  //Get the i-th term
		  T = terms.at(i);
          
		  //Check it is a valid term
		  CND_CONDITION(T != NULL,"T is NULL");
		  
		  //Sum the Idf of term T
          idf += Similarity::idf(*T, searcher);
	      }

	  //Calculate the new weight  
      weight = idf * boost;
      //square term weights
      return weight * weight;			  
  }

  void PhraseQuery::normalize(const float_t norm) {
  //Func - Normalizes the Weight
  //Pre  - norm contains a value
  //Post - The PhraseQuery has been normalized

      //normalize for query
      weight *= norm;			
      //factor from document
      weight *= idf;				  
  }

  Scorer* PhraseQuery::scorer(IndexReader& reader)  {
  //Func -
  //Pre  -
  //Post -

	  //Get the length of terms
      int_t tpsLength = terms.size();

	  //optimize zero-term case
      if (tpsLength == 0)			  
          return NULL;
    
      // optimize one-term case
	  if (tpsLength == 1) {			  
          Term* term = terms.at(0);
          TermDocs* docs = &reader.termDocs(term);
          if (docs == NULL)
              return NULL;
          return new TermScorer(*docs, reader.getNorms(term->Field()), weight);
          }

    TermPositions** tps = new TermPositions*[tpsLength];

	//Check if tps has been allocated properly
    CND_CONDITION(tps != NULL,"Could not allocate memory for tps");

    TermPositions* p = NULL;

	//Iterate through all terms
    for (uint_t i = 0; i < terms.size(); i++) {
        //Get the termPostitions for the i-th term
        p = &reader.termPositions(terms.at(i));
      
		//Check if p is valid
		if (p == NULL) {
			//Delete previous retrieved termPositions
			while (--i >= 0){
				_DELETE(tps[i]); 
			    }
            _DELETE_ARRAY(tps); 
            return NULL;
            }

        //Store p at i in tps
        tps[i] = p;
        }

    Scorer* ret = NULL;

#ifndef NO_FUZZY_QUERY
	if (slop != 0){
		 // optimize exact case
         ret = new SloppyPhraseScorer(tps,tpsLength, slop, reader.getNorms(field), weight);
         }
     else
#endif
         ret = new ExactPhraseScorer(tps, tpsLength,reader.getNorms(field), weight);

    CND_CONDITION(ret != NULL,"Could not allocate memory for ret")

	//tps can be deleted savely. SloppyPhraseScorer or ExactPhraseScorer will take care
	//of its values

    _DELETE_ARRAY(tps);
    return ret;
  }

  void PhraseQuery::getTerms(Term**& ret, int_t& size){
  //Func - added by search highlighter
  //Pre  -
  //Post -

	  //Let size contain the number of terms
      size = terms.size();
      ret = new Term*[size];
       
	  CND_CONDITION(ret != NULL,"Could not allocated memory for ret")

	  //Iterate through terms and copy each pointer to ret
	  for ( int_t i=0;i<size;i++ ){
          ret[i] = terms[i];
          }
  }
  const char_t* PhraseQuery::toString(const char_t* f) {
  //Func - Prints a user-readable version of this query. 
  //Pre  - f != NULL
  //Post - The query string has been returned

      CND_PRECONDITION(f != NULL,"f is NULL")

      StringBuffer buffer;
      if ( stringCompare(field,f)!= 0) {
          buffer.append(field);
          buffer.append( _T(":"));
          }

      buffer.append( _T("\"") );

      Term *T = NULL;

	  //iterate through all terms
      for (uint_t i = 0; i < terms.size(); i++) {
		  //Get the i-th term
		  T = terms.at(i);

		  //Ensure T is a valid Term
          CND_CONDITION(T !=NULL,"T is NULL")

          buffer.append( T->Text() );
		  //Check if i is at the end of terms
		  if (i != terms.size()-1){
              buffer.append(_T(" "));
              }
          }

      buffer.append( _T("\"") );

#ifndef NO_FUZZY_QUERY
      if (slop != 0) {
          buffer.append(_T("~"));
          buffer.append(slop);
          }
#endif

	  //Check if there is an other boost factor than 1.0
      if (boost != 1.0f) {
          buffer.append(_T("^"));
          buffer.append( boost,10 );
          }

	  //return the query string
	  return buffer.ToString();
  }

}}
