#include "stdafx.h"

namespace lucene { namespace test {

		void assertEquals(bool v){
			if ( !v )
				cout << "An item failed.\n";
		}
		void testAsterisk(){
			RAMDirectory indexStore;
			IndexWriter& writer = *new IndexWriter(indexStore, *new SimpleAnalyzer(), true);
			Document doc1;
			Document doc2;
			doc1.add(Field::Text(_T("body"), _T("metal")));
			doc2.add(Field::Text(_T("body"), _T("metals")));
	        
			writer.addDocument(doc1);
			writer.addDocument(doc2);
			writer.optimize();
		    
			IndexReader& reader = IndexReader::open(indexStore);
			IndexSearcher& searcher = *new IndexSearcher(reader);
			Query* query1 = new TermQuery(*new Term(_T("body"), _T("metal")));
			Query* query2 = new WildcardQuery(new Term(_T("body"), _T("metal*")));
			Query* query3 = new WildcardQuery(new Term(_T("body"), _T("m*tal")));
			Query* query4 = new WildcardQuery(new Term(_T("body"), _T("m*tal*")));

    		Hits* result = NULL;
	    
    		result = &searcher.search(*query1);
    		assertEquals(1 == result->Length());
	    
    		result = &searcher.search(*query2);
    		assertEquals(2 == result->Length());
	    
    		result = &searcher.search(*query3);
    		assertEquals(1 == result->Length());
	    
    		result = &searcher.search(*query4);
    		assertEquals(2 == result->Length());
	    
    		writer.close();
		}

		void testQuestionmark(){
			RAMDirectory indexStore;
			IndexWriter& writer = *new IndexWriter(indexStore, *new SimpleAnalyzer(), true);
			Document doc1;
			Document doc2;
			Document doc3;
			Document doc4;
			doc1.add(Field::Text(_T("body"), _T("metal")));
			doc2.add(Field::Text(_T("body"), _T("metals")));
			doc3.add(Field::Text(_T("body"), _T("mXtals")));
			doc4.add(Field::Text(_T("body"), _T("mXtXls")));
			writer.addDocument(doc1);
			writer.addDocument(doc2);
			writer.addDocument(doc3);
			writer.addDocument(doc4);
    		writer.optimize();
    		
			IndexReader& reader = IndexReader::open(indexStore);
			IndexSearcher& searcher = *new IndexSearcher(reader);
			Query* query1 = new WildcardQuery(new Term(_T("body"), _T("m?tal"))); 
			Query* query2 = new WildcardQuery(new Term(_T("body"), _T("metal?")));
			Query* query3 = new WildcardQuery(new Term(_T("body"), _T("metals?")));
			Query* query4 = new WildcardQuery(new Term(_T("body"), _T("m?t?ls")));

    		Hits* result = NULL;
	    
    		result = &searcher.search(*query1);
    		assertEquals(1 == result->Length());
	    
    		result = &searcher.search(*query2);
    		assertEquals(2 == result->Length());
	    
    		result = &searcher.search(*query3);
    		assertEquals(1 == result->Length());
	    
    		result = &searcher.search(*query4);
    		assertEquals(3== result->Length());
	    
    		writer.close();
		}
}}
