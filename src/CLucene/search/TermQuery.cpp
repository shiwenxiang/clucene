#include "CLucene/StdHeader.h"
#include "TermQuery.h"

#include "SearchHeader.h"
#include "Scorer.h"
#include "CLucene/index/Term.h"
#include "TermScorer.h"
#include "CLucene/index/IndexReader.h"
#include "CLucene/util/StringBuffer.h"
#include "CLucene/index/Terms.h"

using namespace lucene::index;
namespace lucene{ namespace search {


	/** Constructs a query for the term <code>t</code>. */
	TermQuery::TermQuery(Term& t):
		term( t.pointer() ),
		idf ( 0.0f ),
		weight ( 0.0f )
	{
	}
	TermQuery::~TermQuery(){
	    term->finalize();
	}

	const char_t* TermQuery::getQueryName() const{
		return _T("TermQuery");
	}
    
	float_t TermQuery::sumOfSquaredWeights(Searcher& searcher){
		idf = Similarity::idf(*term, searcher);
		weight = idf * boost;
		return weight * weight;			  // square term weights
	}
    
	void TermQuery::normalize(const float_t norm) {
		weight *= norm;				  // normalize for query
		weight *= idf;				  // factor from document
	}
      
	//added by search highlighter
	Term* TermQuery::getTerm()
	{
		return term->pointer();
	}
    
	Scorer* TermQuery::scorer(IndexReader& reader){
		TermDocs* termDocs = &reader.termDocs(term);
		if (termDocs == NULL)
			return NULL;
        
		//Termscorer will delete termDocs when finished
		return new TermScorer(*termDocs, reader.getNorms(term->Field()), weight);
	}
    
	/** Prints a user-readable version of this query. */
	const char_t* TermQuery::toString(const char_t* field) {
		lucene::util::StringBuffer buffer;
		if ( stringCompare(term->Field(),field)!= 0 ) {
			buffer.append(term->Field());
			buffer.append(_T(":"));
		}
		buffer.append(term->Text());
		if (boost != 1.0f) {
			buffer.append(_T("^"));
			buffer.append( boost,10 );
		}
		return buffer.ToString();
	}
}}

