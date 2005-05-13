#include "stdafx.h"

namespace lucene{ namespace test{ 

    class TermInfo {
	public:
      int_t docFreq;
      Term* term;
      
      TermInfo(Term* t, int_t df) {
        term = t;
        docFreq = df;
      }
    };

    class TermInfoQueue: public lucene::util::PriorityQueue<TermInfo*> {
	public:
        TermInfoQueue(int_t size) {
            initialize(size,true);
        }
        bool lessThan(TermInfo* A, TermInfo* B) {
            return A->docFreq < B->docFreq;
          
        }
    };
   
	void _TestSearchesRun( Analyzer& analyzer, IndexSearcher& search, char_t* qry){
		try{
			Query& q = QueryParser::Parse(qry , _T("contents"), analyzer);
			Hits& h = search.search( q );
			delete &h;
			delete &q;
		}catch(THROW_TYPE e){
			_cout << _T("Failed: ") << qry << " - " << e.what() << endl;
		}catch(...){
			_cout << _T("FAILED: ") << qry << endl;
		}
	}
	void TestSearches(const char_t* index){
		long_t start = lucene::util::Misc::currentTimeMillis();
		IndexSearcher s(index);
		SimpleAnalyzer a;
		StandardAnalyzer aStd;
		WhitespaceAnalyzer aWS;

		//simple tests
		cout << "Simple tests\n";
		_TestSearchesRun(a,s, _T("a AND b") );
		_TestSearchesRun(a,s, _T("term term term") );
		_TestSearchesRun(a,s, _T("türm term term") );
		
		_TestSearchesRun(a,s, _T("ümlaut") );

		_TestSearchesRun(a,s, _T("(a AND b)") );
		_TestSearchesRun(a,s, _T("c OR (a AND b)") );
		_TestSearchesRun(a,s, _T("a AND NOT b") );
		_TestSearchesRun(a,s, _T("a AND -b") );
		_TestSearchesRun(a,s, _T("a AND !b") );
		_TestSearchesRun(a,s, _T("a && b") );
		_TestSearchesRun(a,s, _T("a && ! b") );

		_TestSearchesRun(a,s, _T("a OR b") );
		_TestSearchesRun(a,s, _T("a || b") );
		_TestSearchesRun(a,s, _T("a OR !b") );
		_TestSearchesRun(a,s, _T("a OR ! b") );
		_TestSearchesRun(a,s, _T("a OR -b") );

		_TestSearchesRun(a,s, _T("+term -term term") );
		_TestSearchesRun(a,s, _T("foo:term AND field:anotherTerm") );
		_TestSearchesRun(a,s, _T("term AND \"phrase phrase\"") );
		_TestSearchesRun(a,s, _T("\"hello there\"") );

		_TestSearchesRun(a,s,  _T("a AND b") );
		_TestSearchesRun(a,s,  _T("hello") );
		_TestSearchesRun(a,s,  _T("\"hello there\"") );

		_TestSearchesRun(a,s, _T("germ term^2.0") );
		_TestSearchesRun(a,s, _T("term^2.0") );
		_TestSearchesRun(a,s, _T("term^2") );
		_TestSearchesRun(a,s, _T("term^2.3") );
		_TestSearchesRun(a,s, _T("\"germ term\"^2.0") );
		_TestSearchesRun(a,s, _T("\"term germ\"^2") );

		_TestSearchesRun(a,s, _T("(foo OR bar) AND (baz OR boo)") );
		_TestSearchesRun(a,s, _T("((a OR b) AND NOT c) OR d") );
		_TestSearchesRun(a,s, _T("+(apple \"steve jobs\") -(foo bar baz)") );
		_TestSearchesRun(a,s, _T("+title:(dog OR cat) -author:\"bob dole\"") );

		//test punctuation
		cout << "punctuation tests\n";
		_TestSearchesRun(a,s, _T("a&b") );
		_TestSearchesRun(a,s, _T("a&&b") );
		_TestSearchesRun(a,s, _T(".NET") );

		//test slop
		cout << "test slop\n";
		_TestSearchesRun(a,s, _T("\"term germ\"~2") );
		_TestSearchesRun(a,s, _T("\"term germ\"~2 flork") );
		_TestSearchesRun(a,s, _T("\"term\"~2") );
		_TestSearchesRun(a,s, _T("\" \"~2 germ") );
		_TestSearchesRun(a,s, _T("\"term germ\"~2^2") );

		// The numbers go away because SimpleAnalzyer ignores them
		cout << "Numbers test w/ SimpleAnalzyer&StandardAnalyzer\n";
		_TestSearchesRun(a,s, _T("3") );
		_TestSearchesRun(a,s, _T("term 1.0 1 2") );
		_TestSearchesRun(a,s, _T("term term1 term2") );

		_TestSearchesRun(aStd,s, _T("3") );
		_TestSearchesRun(aStd,s, _T("term 1.0 1 2") );
		_TestSearchesRun(aStd,s, _T("term term1 term2") );

		//testWildcard
		cout << "testWildcard\n";
		_TestSearchesRun(a,s, _T("term*") );
		_TestSearchesRun(a,s, _T("term*^2") );
		_TestSearchesRun(a,s, _T("term~") );
		_TestSearchesRun(a,s, _T("term^2~") );
		_TestSearchesRun(a,s, _T("term~^2") );
		_TestSearchesRun(a,s, _T("term*germ") );
		_TestSearchesRun(a,s, _T("term*germ^3") );


		//testEscaped
		cout << "testEscaped\n";
		_TestSearchesRun(aWS,s, _T("\\[brackets") );
		_TestSearchesRun(a,s, _T("\\[brackets") );
		_TestSearchesRun(aWS,s, _T("\\\\") );
		_TestSearchesRun(aWS,s, _T("\\+blah") );
		_TestSearchesRun(aWS,s, _T("\\(blah") );


		//testRange
		cout << "testRange (this can take a long time!)\n";
		_TestSearchesRun(a,s, _T("[ a z]") );
		_TestSearchesRun(a,s, _T("[ a z ]") );
		_TestSearchesRun(a,s, _T("{ a z}") );
		_TestSearchesRun(a,s, _T("{ a z }") );
		_TestSearchesRun(a,s, _T("{ a z }^2.0") );
		_TestSearchesRun(a,s, _T("[ a z] OR bar") );
		_TestSearchesRun(a,s, _T("[ a z] AND bar") );
		_TestSearchesRun(a,s, _T("( bar blar { a z}) ") );
		_TestSearchesRun(a,s, _T("gack ( bar blar { a z}) ") );

		s.close();
		_cout << (lucene::util::Misc::currentTimeMillis()-start) << _T(" milliseconds") << endl;
  }

  void TestHighFreqTerms(const char_t* index, int_t numTerms) {
	  long_t start = lucene::util::Misc::currentTimeMillis();

		IndexReader& reader = IndexReader::open(index);
    
        TermInfoQueue* tiq = new TermInfoQueue(numTerms);
		TermEnum* terms = &reader.getTerms();
    
        int_t minFreq = 0;
        while (terms->next()) {
          if (terms->DocFreq() > minFreq) {
			tiq->put(new TermInfo(terms->getTerm(), terms->DocFreq()));
            if (tiq->Size() > numTerms) {		  // if tiq overfull
              tiq->pop();				  // remove lowest in tiq
              minFreq = ((TermInfo*)tiq->top())->docFreq; // reset minFreq
            }
          }
        }
    
        while (tiq->Size() != 0) {
          TermInfo* termInfo = (TermInfo*)tiq->pop();
          //_cout << termInfo->term->toString() << _T(" ") << termInfo->docFreq << endl;
        }
    
        reader.close();
        delete &reader;

		_cout << (lucene::util::Misc::currentTimeMillis()-start) << _T(" milliseconds/") << numTerms << _T("Terms") << endl;
  }

    
}}