#include "CLucene/StdHeader.h"
#include "MultiTermQuery.h"

using namespace lucene::index;
using namespace lucene::util;
namespace lucene{ namespace search{

/** Constructs a query for terms matching <code>term</code>. */

  MultiTermQuery::MultiTermQuery(Term* t){
  //Func - Constructor
  //Pre  - t != NULL
  //Post - The instance has been created

      CND_PRECONDITION(t != NULL,"t is NULL")

      term  = t;
      query = NULL;
      _enum = NULL;

	  LUCENE_STYLE_TOSTRING = false;

  }

  MultiTermQuery::~MultiTermQuery(){
  //Func - Destructor
  //Pre  - true
  //Post - The instance has been destroyed

      //Typically, MultiTermQuery shares a single reference to this->term
      //with its subclass, and the subclass's destructor decrefs the Term, so
      //we don't need to here.
      _DELETE(_enum);
      _DELETE(query);
    }

    
    void MultiTermQuery::setEnum(FilteredTermEnum* Enum){
	//Func - Sets the TermEnum to be used
    //Pre  - Enum != NULL
	//Post - The TermEnum has been set

        CND_PRECONDITION(Enum != NULL,"ENUM is NULL")

		//Destroy the current enumeration
        _DELETE(_enum);

        _enum = Enum;
    }


    float_t MultiTermQuery::sumOfSquaredWeights(Searcher& searcher){

        return getQuery()->sumOfSquaredWeights(searcher);
    }

    void MultiTermQuery::normalize(const float_t norm) {
            getQuery()->normalize(norm);
    }

    Scorer* MultiTermQuery::scorer(IndexReader& reader) {
        return getQuery()->scorer(reader);
    }

    //marked public by search highlighter
	BooleanQuery* MultiTermQuery::getQuery(){
        if (query == NULL) {
            BooleanQuery* q = new BooleanQuery();
            _TRY {
                do {
                    Term* t = _enum->getTerm();
                    if (t != NULL) {
                        TermQuery* tq = new TermQuery(*t);	// found a match
                        tq->setBoost(boost * _enum->difference()); // set the boost
                        q->add(*tq,true, false, false);		// add to q
                    }
                    t->finalize();
                } while (_enum->next());
            } _FINALLY ( _enum->close() );
            query = q;
        }
        return query;
    }

    /** Prints a user-readable version of this query. */
    const char_t* MultiTermQuery::toString(const char_t* field){
        StringBuffer buffer;

        if (!LUCENE_STYLE_TOSTRING) {
            Query* q = NULL;
            try {
                q = getQuery();
            } catch (...) {}
            if (q != NULL) {
                buffer.append(_T("("));
                buffer.append ( q->toString(field) );
                buffer.append(_T(")") );
                return buffer.ToString();
            }
        }

        if ( stringCompare(term->Field(),field)!=0 ) {
            buffer.append(term->Field());
            buffer.append( _T(":"));
        }
        buffer.append(term->Text());
        if (boost != 1.0f) {
            buffer.append( _T("^") );
            buffer.append( boost,1);
        }
        return buffer.ToString();
    }

}}
