#include "CLucene/StdHeader.h"
#ifndef NO_RANGE_QUERY
#include "RangeQuery.h"

#include "SearchHeader.h"
#include "Scorer.h"
#include "BooleanQuery.h"
#include "TermQuery.h"

#include "CLucene/index/Term.h"
#include "CLucene/index/Terms.h"
#include "CLucene/index/IndexReader.h"
#include "CLucene/util/StringBuffer.h"


using namespace lucene::index;
using namespace lucene::util;
namespace lucene{ namespace search{

	RangeQuery::RangeQuery(Term* LowerTerm, Term* UpperTerm, const bool Inclusive){
	//Func - Constructor
	//Pre  - (LowerTerm != NULL OR UpperTerm != NULL) AND
	//       if LowerTerm and UpperTerm are valid pointer then the fieldnames must be the same
	//Post - The instance has been created

		CND_PRECONDITION(LowerTerm != NULL || UpperTerm != NULL,"At least one term must be non-NULL")
		CND_PRECONDITION(LowerTerm != NULL && UpperTerm != NULL && stringCompare(lowerTerm->Field(), upperTerm->Field()) != 0,
		    "Both terms must be for the same field")

        lowerTerm = (LowerTerm != NULL ? LowerTerm->pointer() : NULL);
        upperTerm = (UpperTerm != NULL ? UpperTerm->pointer() : NULL);


        inclusive = Inclusive;
        query     = NULL;
        reader    = NULL;
    }

    RangeQuery::~RangeQuery() {
    //Func - Destructor
    //Pre  - true
    //Post - The instance has been destroyed

        lowerTerm->finalize();
        upperTerm->finalize();
        _DELETE(query); 
    }

    const char_t* RangeQuery::getQueryName() const{
    
      return _T("RangeQuery");
    }

    void RangeQuery::prepare(IndexReader& reader){
	//Func - Prepares a range query with an IndexReader
	//Pre  - reader contains a valid reference to an indexReader
	//Post - The RangeQuery has been prepared by resetting query and setting the reader

		_DELETE(query);
        this->reader = &reader;
    }

    float_t RangeQuery::sumOfSquaredWeights(Searcher& searcher)
    {
        return getQuery()->sumOfSquaredWeights(searcher);
    }

    void RangeQuery::normalize(const float_t norm)
    {
            getQuery()->normalize(norm);
    }

    Scorer* RangeQuery::scorer(IndexReader& reader)
    {
        return getQuery()->scorer(reader);
    }

    BooleanQuery* RangeQuery::getQuery(){
    //Func - Returns a BooleanQuery representing the RangeQuery
	//Pre  - true
	//Post - The BooleanQuery representing the RangeQuery

        if (query == NULL){
            BooleanQuery* q = new BooleanQuery();
            //if we have a lowerTerm, start there. Otherwise, start at beginning
			if (lowerTerm == NULL){
                lowerTerm = new Term(getField(), _T(""));
			    }

            CND_CONDITION(lowerTerm != NULL,"lowerTerm is NULL")

            TermEnum* _enum = &reader->getTerms(lowerTerm);
            _TRY {
                const char_t* lowerText = NULL;
                //char_t* field;
                bool checkLower = false;

                if (!inclusive){
                    // make adjustments to set to exclusive
                    if (lowerTerm != NULL){
                        lowerText = lowerTerm->Text();
                        checkLower = true;
                        }
                    if (upperTerm != NULL){
                        // set upperTerm to an actual term in the index
                        TermEnum *uppEnum = &reader->getTerms(upperTerm);
                        upperTerm = uppEnum->getTerm();
                        uppEnum->close();
                        _DELETE(uppEnum);
                        }
                    }

                const char_t* testField = getField();
                do{
                    //Ordered getTerm not to return reference/
                    Term* term = _enum->getTerm(false);
                    if (term != NULL && stringCompare(term->Field(), testField)==0 ){
                        if (!checkLower || stringCompare(term->Text(),lowerText) > 0){
                            checkLower = false;
                            if (upperTerm != NULL){
                                int_t compare = upperTerm->compareTo( *term );
                                //if beyond the upper term, or is exclusive and
                                //this is equal to the upper term, break out
								if ((compare < 0) || (!inclusive && compare == 0)){
									break;
								    }
                                }
							//found a match
                            TermQuery *tq = new TermQuery(*term);
							//set the boost
                            tq->setBoost(boost);
							//add to q
                            q->add(*tq,true, false, false);		  
                            }
                        }
                    else{
                        break;
                        }
                    }
                while (_enum->next());
                }
			_FINALLY(
                _enum->close();
                delete _enum;
                );

            query = q;
            }
        return query;
    }


    /** Prints a user-readable version of this query. */
  const char_t* RangeQuery::toString(const char_t* field)
    {
        StringBuffer buffer;
        if ( stringCompare(getField(),field)!=0 )
        {
            buffer.append( getField() );
            buffer.append( _T(":"));
        }
        buffer.append(inclusive ? _T("[") : _T("{"));
        buffer.append(lowerTerm != NULL ? lowerTerm->Text() : _T("NULL"));
        buffer.append(_T("-"));
        buffer.append(upperTerm != NULL ? upperTerm->Text() : _T("NULL"));
        buffer.append(inclusive ? _T("]") : _T("}"));
        if (boost != 1.0f)
        {
            buffer.append( _T("^"));
            buffer.append( boost,4 );
        }
        return buffer.ToString();
    }


  const char_t* RangeQuery::getField()
    {
        return (lowerTerm != NULL ? lowerTerm->Field() : upperTerm->Field());
    }

}}
#endif
