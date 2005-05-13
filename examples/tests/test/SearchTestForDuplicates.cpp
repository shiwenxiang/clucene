#include "stdafx.h"
namespace lucene{ namespace test { namespace SearchTestForDuplicates{

      static void print_tHits( Hits* hits ) {
        _cout << hits->Length() << _T(" total results\n")<<endl;
        
        for (int_t i = 0 ; i < hits->Length(); i++) {
          if ( i < 10 || (i > 94 && i < 105) ) {
            Document& d = hits->doc(i);
            _cout << i << _T(" ") << d.get(_T("id"))<<endl;
          }
        }
      }
    
       void test() {
        try {
          Directory& directory = *new RAMDirectory();
          Analyzer& analyzer = *new SimpleAnalyzer();
          IndexWriter& writer = *new IndexWriter(directory, analyzer, true);
    
          const int_t MAX_DOCS = 225;
    
          for (int_t j = 0; j < MAX_DOCS; j++) {
            Document& d = *new Document();
			d.add(Field::Text(_T("priority"), _T("high")));
			
			char_t buf[80];
			integerToString(j,buf,10);
			d.add(Field::Text(_T("id"), buf));
            writer.addDocument(d);
          }
          writer.close(false); //dont' close, or ramdirectory will close.
    



          // try a search without OR
          Searcher& searcher = *new IndexSearcher( IndexReader::open(directory,false) );
          Hits* hits = NULL;

          QueryParser* parser = new QueryParser(_T("priority"), analyzer);
    
          Query& query = parser->Parse(_T("high"));
          _cout << _T("Query: ") << query.toString(_T("priority")) <<endl;
    
          hits = &searcher.search(query);
          print_tHits(hits);
    
          //searcher.close();
    



          // try a new search with OR
          searcher = *new IndexSearcher( IndexReader::open(directory,false) );
          hits = NULL;
    
          parser = new QueryParser(_T("priority"), analyzer);
    
		  StringBuffer b;
		  b.append(_T("high"));
		  b.append ( _T(" OR ") );
		  b.append ( _T("medium") );
          Query& query2 = parser->Parse(b.getBuffer());

          _cout << _T("Query: ") << query2.toString(_T("priority")) <<endl;
    
          hits = &searcher.search(query2);
          print_tHits(hits);
    
          searcher.close();
		  directory.close();
    
        } catch (THROW_TYPE e) {
			_cout << _T(" exception: ") << e.what() <<endl;
        }
      }
    
   }
}}
