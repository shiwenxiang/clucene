#include "stdafx.h"

using namespace std;
namespace lucene{ namespace test { namespace analysisTest{

  static void test(Reader* reader, bool verbose, long_t bytes) {
	  Analyzer* analyzer = new lucene::analysis::standard::StandardAnalyzer();
    TokenStream& stream = analyzer->tokenStream(NULL, reader);

	long_t start = lucene::util::Misc::currentTimeMillis();

    int_t count = 0;
    for (Token* t = stream.next(); t!=NULL; t = stream.next()) {
      if (verbose) {
		_cout << _T("Text=") << t->TermText() 
				<< _T(" start=") << t->StartOffset()
				<< _T(" end=") << t->EndOffset() << endl;
      }
	  delete t;
      count++;
    }

    long_t end = lucene::util::Misc::currentTimeMillis();
    long_t time = end - start;
	_cout << time << _T(" milliseconds to extract ") << count << _T(" tokens") << endl;
    _cout << (time*1000.0)/count << _T(" microseconds/token") << endl;
    _cout << (bytes * 1000.0 * 60.0 * 60.0)/(time * 1000000.0) << _T(" megabytes/hour") << endl;


	delete analyzer;
//	delete &stream;
  }

  void testFile(const char_t* fname, bool verbose) {
    struct Struct_Stat buf;	
	Cmd_Stat(fname,&buf);

	long_t bytes = buf.st_size;

    _cout << _T(" Reading test file containing ") << bytes << _T(" bytes.") << endl;
    FileReader fr (fname);
    test(&fr, verbose, fr.available());
    fr.close();
  }

  void testText(const char_t* text, bool verbose) {
    _cout << _T(" Tokenizing string: ") << text << endl ;
    test( new StringReader(text), verbose, stringLength(text));
  }
  
	}
}}
