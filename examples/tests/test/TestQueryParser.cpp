#include "stdafx.h"
namespace lucene{ namespace test { namespace TestQueryParser{

	class QPTestFilter: public TokenFilter {
	public:

		bool inPhrase;
		int_t savedStart, savedEnd;

		/**
		* Filter which discards the token 'stop' and which expands the
		* token 'phrase' into 'phrase1 phrase2'
		*/
		QPTestFilter(TokenStream* in):
			TokenFilter(in,true),
			inPhrase(false),
			savedStart(0),
			savedEnd(0)
		{
		}

		Token* next() {
			if (inPhrase) {
				inPhrase = false;
				return new Token( _T("phrase2"), savedStart, savedEnd);
			}
			else
			for (Token* token = input->next(); token != NULL; token = input->next())
				if ( stringCompare(token->TermText(), _T("phrase")) == 0 ) {
					inPhrase = true;
					savedStart = token->StartOffset();
					savedEnd = token->EndOffset();
					return new Token( _T("phrase1"), savedStart, savedEnd);
				}
				else if ( stringCompare(token->TermText(), _T("stop") ) !=0 )
					return token;
				return NULL;
		}
	};

	class QPTestAnalyzer: public Analyzer {
	public:
		QPTestAnalyzer() {
		}

		/** Filters LowerCaseTokenizer with StopFilter. */
		TokenStream& tokenStream(const char_t* fieldName, Reader* reader) {
			return *new QPTestFilter(new LowerCaseTokenizer(reader));
		}
	};

	Query* getQuery(const char_t* query, Analyzer* a) {
		try{
			bool del = (a==NULL);
			if (a == NULL)
				a = new SimpleAnalyzer();

			QueryParser qp(_T("field"), *a);
			Query* ret = &qp.Parse(query);
			
			if ( del )
				delete a;
			return ret;
		}catch(...){
			_cout << _T("/") << query << _T("/ threw an error.") << endl;
			return NULL;
		}
	}

	void assertQueryEquals(const char_t* query, Analyzer* a, const char_t* result)  {
		
		Query* q = getQuery(query, a);
		if ( q == NULL )
			return;

		const char_t* s = q->toString(_T("field"));
		if ( stringCompare(s,result) != 0 ) {
			_cout << _T("FAILED Query /") << query << _T("/ yielded /") << s 
				<< _T("/, expecting /") << result << _T("/") <<endl;
		}//else
		//	_cout << _T("Query /") << query << _T("/ SUCCESS")<<endl;
		delete[] s;
		delete q;
			
	}

	void assertTrue(const char_t* query, Analyzer* a, const char_t* inst, const char_t* msg){
		Query* q = getQuery(query,a);
		bool success = q->instanceOf(inst);
		delete q;
		if ( !success )
			_cout << _T("Assert FAILED: ") << msg << endl;
	}

	void testSimple() {
		assertQueryEquals(_T("term term term"), NULL, _T("term term term"));
		assertQueryEquals(_T("türm term term"), NULL, _T("türm term term"));
		
		assertQueryEquals(_T("ümlaut"), NULL, _T("ümlaut"));

		assertQueryEquals(_T("a AND b"), NULL, _T("+a +b"));
		assertQueryEquals(_T("(a AND b)"), NULL, _T("+a +b"));
		assertQueryEquals(_T("c OR (a AND b)"), NULL, _T("c (+a +b)"));
		assertQueryEquals(_T("a AND NOT b"), NULL, _T("+a -b"));
		assertQueryEquals(_T("a AND -b"), NULL, _T("+a -b"));
		assertQueryEquals(_T("a AND !b"), NULL, _T("+a -b"));
		assertQueryEquals(_T("a && b"), NULL, _T("+a +b"));
		assertQueryEquals(_T("a && ! b"), NULL, _T("+a -b"));

		assertQueryEquals(_T("a OR b"), NULL, _T("a b"));
		assertQueryEquals(_T("a || b"), NULL, _T("a b"));
		assertQueryEquals(_T("a OR !b"), NULL, _T("a -b"));
		assertQueryEquals(_T("a OR ! b"), NULL, _T("a -b"));
		assertQueryEquals(_T("a OR -b"), NULL, _T("a -b"));

		assertQueryEquals(_T("+term -term term"), NULL, _T("+term -term term"));
		assertQueryEquals(_T("foo:term AND field:anotherTerm"), NULL, 
						_T("+foo:term +anotherterm"));
		assertQueryEquals(_T("term AND \"phrase phrase\""), NULL, 
						_T("+term +\"phrase phrase\"") );
		assertQueryEquals(_T("\"hello there\""), NULL, _T("\"hello there\"") );

		assertTrue( _T("a AND b"), NULL,_T("BooleanQuery"),_T("a AND b") );
		assertTrue( _T("hello"), NULL,_T("TermQuery"), _T("hello"));
		assertTrue( _T("\"hello there\""), NULL,_T("PhraseQuery"), _T("\"hello there\""));

		assertQueryEquals(_T("germ term^2.0"), NULL, _T("germ term^2.0"));
		assertQueryEquals(_T("term^2.0"), NULL, _T("term^2.0"));
		assertQueryEquals(_T("term^2"), NULL, _T("term^2.0"));
		assertQueryEquals(_T("term^2.3"), NULL, _T("term^2.3"));
		assertQueryEquals(_T("\"germ term\"^2.0"), NULL, _T("\"germ term\"^2.0"));
		assertQueryEquals(_T("\"term germ\"^2"), NULL, _T("\"term germ\"^2.0") );

		assertQueryEquals(_T("(foo OR bar) AND (baz OR boo)"), NULL, 
						_T("+(foo bar) +(baz boo)"));
		assertQueryEquals(_T("((a OR b) AND NOT c) OR d"), NULL, 
						_T("(+(a b) -c) d"));
		assertQueryEquals(_T("+(apple \"steve jobs\") -(foo bar baz)"), NULL, 
						_T("+(apple \"steve jobs\") -(foo bar baz)") );
		assertQueryEquals(_T("+title:(dog OR cat) -author:\"bob dole\""), NULL, 
						_T("+(title:dog title:cat) -author:\"bob dole\"") );
			                
	}

	void testPunct() {
		Analyzer* a = new WhitespaceAnalyzer();
		assertQueryEquals(_T("a&b"), a, _T("a&b"));
		assertQueryEquals(_T("a&&b"), a, _T("a&&b"));
		assertQueryEquals(_T(".NET"), a, _T(".NET"));
	}

	void testSlop() {
		assertQueryEquals(_T("\"term germ\"~2"), NULL, _T("\"term germ\"~2") );
		assertQueryEquals(_T("\"term germ\"~2 flork"), NULL, _T("\"term germ\"~2 flork") );
		assertQueryEquals(_T("\"term\"~2"), NULL, _T("term"));
		assertQueryEquals(_T("\" \"~2 germ"), NULL, _T("germ"));
		assertQueryEquals(_T("\"term germ\"~2^2"), NULL, _T("\"term germ\"~2^2.0") );
	}

	void testNumber() {
		// The numbers go away because SimpleAnalzyer ignores them
		assertQueryEquals(_T("3"), NULL, _T(""));
		assertQueryEquals(_T("term 1.0 1 2"), NULL, _T("term"));
		assertQueryEquals(_T("term term1 term2"), NULL, _T("term term term"));

		Analyzer* a = new StandardAnalyzer();
		assertQueryEquals(_T("3"), a, _T("3"));
		assertQueryEquals(_T("term 1.0 1 2"), a, _T("term 1.0 1 2"));
		assertQueryEquals(_T("term term1 term2"), a, _T("term term1 term2"));
	}

	void testWildcard() {

		assertQueryEquals(_T("term*"), NULL, _T("term*"));
		assertQueryEquals(_T("term*^2"), NULL, _T("term*^2.0"));
		assertQueryEquals(_T("term~"), NULL, _T("term~"));
		assertQueryEquals(_T("term~^2"), NULL, _T("term^2.0~"));
		assertQueryEquals(_T("term^2~"), NULL, _T("term^2.0~"));
		assertQueryEquals(_T("term*germ"), NULL, _T("term*germ"));
		assertQueryEquals(_T("term*germ^3"), NULL, _T("term*germ^3.0"));

		assertTrue( _T("term*"), NULL,_T("PrefixQuery"), _T("term*"));
		assertTrue( _T("term*^2"), NULL,_T("PrefixQuery"), _T("term*^2"));
		assertTrue( _T("term~"), NULL,_T("FuzzyQuery"), _T("term~"));
		assertTrue( _T("term*germ"), NULL,_T("WildcardQuery"), _T("term*germ"));
	}

	void testQPA() {
		QPTestAnalyzer qpAnalyzer;
		assertQueryEquals(_T("term term term"), &qpAnalyzer, _T("term term term") );
		assertQueryEquals(_T("term +stop term"), &qpAnalyzer, _T("term term") );
		assertQueryEquals(_T("term -stop term"), &qpAnalyzer, _T("term term") );
		assertQueryEquals(_T("drop AND stop AND roll"), &qpAnalyzer, _T("+drop +roll") );
		assertQueryEquals(_T("term phrase term"), &qpAnalyzer, 
						_T("term \"phrase1 phrase2\" term") );
		assertQueryEquals(_T("term AND NOT phrase term"), &qpAnalyzer, 
						_T("+term -\"phrase1 phrase2\" term") );
		assertQueryEquals(_T("stop"), &qpAnalyzer, _T("") );
		assertTrue( _T("term term term"), &qpAnalyzer,_T("BooleanQuery"), _T("term term term"));
		assertTrue( _T("term +stop"), &qpAnalyzer,_T("TermQuery"), _T("term +stop"));
	}

	void testRange() {
		assertQueryEquals(_T("[ a z]"), NULL, _T("[a-z]"));
		assertTrue( _T("[ a z]"), NULL, _T("RangeQuery"), _T("[ a z]") );
		assertQueryEquals(_T("[ a z ]"), NULL, _T("[a-z]"));
		assertQueryEquals(_T("{ a z}"), NULL, _T("{a-z}"));
		assertQueryEquals(_T("{ a z }"), NULL, _T("{a-z}"));
		assertQueryEquals(_T("{ a z }^2.0"), NULL, _T("{a-z}^2.0"));
		assertQueryEquals(_T("[ a z] OR bar"), NULL, _T("[a-z] bar"));
		assertQueryEquals(_T("[ a z] AND bar"), NULL, _T("+[a-z] +bar"));
		assertQueryEquals(_T("( bar blar { a z}) "), NULL, _T("bar blar {a-z}"));
		assertQueryEquals(_T("gack ( bar blar { a z}) "), NULL, _T("gack (bar blar {a-z})"));
	}

	void testEscaped() {
		Analyzer* a = new WhitespaceAnalyzer();
		assertQueryEquals(_T("\\[brackets"), a, _T("\\[brackets"));
		assertQueryEquals(_T("\\[brackets"), NULL, _T("brackets"));
		assertQueryEquals(_T("\\\\"), a, _T("\\\\"));
		assertQueryEquals(_T("\\+blah"), a, _T("\\+blah"));
		assertQueryEquals(_T("\\(blah"), a, _T("\\(blah"));
	}
}}}