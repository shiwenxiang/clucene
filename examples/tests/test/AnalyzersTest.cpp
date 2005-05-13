#include "stdafx.h"

namespace lucene{ namespace test { namespace Analysis {

  static void assertAnalyzersTo(Analyzer& a, const char_t* input, char_t* output){
    Reader& reader = *new StringReader(input);
	TokenStream& ts = a.tokenStream(_T("dummy"), &reader );

	int_t i = 0;
	const char_t* pos = output;
	char_t buffer[80];
	const char_t* last = output;
	while( (pos = stringFind(pos+1, _T(";"))) != NULL ) {
		int_t len = (int_t)(pos-last);
		stringNCopy(buffer,last,len);
		buffer[len]=0;

		Token* t = ts.next();
		if ( t == NULL ){
            _cout << _T("Test failed. - ") << input <<endl;
			return;
		}
		else if ( stringCompare( t->TermText(),buffer) != 0){
			_cout << _T("Test failed, term is not equal - ") << input <<endl;
			return;
		}

        last = pos+1;
        delete t;
    }
    if ( ts.next() != NULL)
        _cout << "Test failed, more fields than expected."<<endl;
    
	ts.close();
    delete &reader;
    delete &ts;
  }

  void testSimpleAnalyzer(){
    Analyzer& a = *new SimpleAnalyzer();
	assertAnalyzersTo(a, _T("foo bar FOO BAR"), _T("foo;bar;foo;bar;") );
    assertAnalyzersTo(a, _T("foo      bar .  FOO <> BAR"), _T("foo;bar;foo;bar;"));
    assertAnalyzersTo(a, _T("foo.bar.FOO.BAR"), _T("foo;bar;foo;bar;"));
    assertAnalyzersTo(a, _T("U.S.A."), _T("u;s;a;") );
    assertAnalyzersTo(a, _T("C++"), _T("c;") );
    assertAnalyzersTo(a, _T("B2B"), _T("b;b;"));
    assertAnalyzersTo(a, _T("2B"), _T("b;"));
    assertAnalyzersTo(a, _T("\"QUOTED\" word"), _T("quoted;word;"));
    
    delete &a;
  }

  void testNullAnalyzer(){
    Analyzer& a = *new WhitespaceAnalyzer();
    assertAnalyzersTo(a, _T("foo bar FOO BAR"), _T("foo;bar;FOO;BAR;"));
    assertAnalyzersTo(a, _T("foo      bar .  FOO <> BAR"), _T("foo;bar;.;FOO;<>;BAR;"));
    assertAnalyzersTo(a, _T("foo.bar.FOO.BAR"), _T("foo.bar.FOO.BAR;"));
    assertAnalyzersTo(a, _T("U.S.A."), _T("U.S.A.;"));
    assertAnalyzersTo(a, _T("C++"), _T("C++;"));
    assertAnalyzersTo(a, _T("B2B"), _T("B2B;"));
    assertAnalyzersTo(a, _T("2B"), _T("2B;"));
    assertAnalyzersTo(a, _T("\"QUOTED\" word"), _T("\"QUOTED\";word;") );
    
    delete &a;
  }

  void testStopAnalyzer(){
    Analyzer& a = *new StopAnalyzer();
    assertAnalyzersTo(a, _T("foo bar FOO BAR"), _T("foo;bar;foo;bar;"));
    assertAnalyzersTo(a, _T("foo a bar such FOO THESE BAR"), _T("foo;bar;foo;bar;"));
    
    delete &a;
  }
  
}
}}
