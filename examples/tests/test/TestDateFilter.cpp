#include "stdafx.h"

namespace lucene { namespace test { namespace TestDateFilter {
	static void assertEquals(bool v){
		if ( !v )
			cout << "An item failed.\n";
	}

	void testBefore() {
	// create an index
		FSDirectory indexStore( _T("index1"),true);
		IndexWriter& writer = *new IndexWriter(indexStore, *new SimpleAnalyzer(), true);
     	long_t now = Misc::currentTimeMillis();

     	Document doc;
     	// add time that is in the past
     	doc.add(Field::Keyword( _T("datefield"), DateField::timeToString(now - 1000)));
     	doc.add(Field::Text(_T("body"), _T("Today is a very sunny day in New York City")));
      	writer.addDocument(doc);
     	writer.optimize();
    	writer.close(false);
	
		IndexReader& reader = IndexReader::open(indexStore,true);
    	IndexSearcher& searcher = *new IndexSearcher(reader);
	
    	// filter that should preserve matches
		DateFilter* df1 = DateFilter::Before(_T("datefield"), now);
	
    	// filter that should discard matches
    	DateFilter* df2 = DateFilter::Before(_T("datefield"), now - 999999);
	
    	// search something that doesn't exist with DateFilter
    	Query& query1 = *new TermQuery(*new Term(_T("body"), _T("NoMatchForThis")));
	
    	// search for something that does exists
    	Query& query2 = *new TermQuery(*new Term(_T("body"), _T("sunny")));
	
    	Hits* result = NULL;
	
    	// ensure that queries return expected results without DateFilter first
    	result = &searcher.search(query1);
    	assertEquals(0 == result->Length());
	
    	result = &searcher.search(query2);
    	assertEquals(1 == result->Length());
	
	
    	// run queries with DateFilter
    	result = &searcher.search(query1, df1);
    	assertEquals(0 == result->Length());
	
    	result = &searcher.search(query1, df2);
    	assertEquals(0 == result->Length());
	
     	result = &searcher.search(query2, df1);
     	assertEquals(1 == result->Length());
	
    	result = &searcher.search(query2, df2);
    	assertEquals(0 == result->Length());
	}

	void testAfter() {
		// create an index
		RAMDirectory indexStore;
		IndexWriter& writer = *new IndexWriter(indexStore, *new SimpleAnalyzer(), true);

		long_t now = Misc::currentTimeMillis();
	
     	Document doc;
     	// add time that is in the future
     	doc.add(Field::Keyword(_T("datefield"), DateField::timeToString(now + 888888)));
     	doc.add(Field::Text(_T("body"), _T("Today is a very sunny day in New York City")));
      	writer.addDocument(doc);
     	writer.optimize();
    	writer.close(false);
	
		
		IndexReader& reader = IndexReader::open(indexStore,true);
    	IndexSearcher& searcher = *new IndexSearcher(reader);
	
    	// filter that should preserve matches
    	DateFilter* df1 = DateFilter::After(_T("datefield"), now);
	
    	// filter that should discard matches
    	DateFilter* df2 = DateFilter::After(_T("datefield"), now + 999999);
	
    	// search something that doesn't exist with DateFilter
    	Query& query1 = *new TermQuery(*new Term(_T("body"), _T("NoMatchForThis")));
	
    	// search for something that does exists
    	Query& query2 = *new TermQuery(*new Term(_T("body"), _T("sunny")));
	
    	Hits* result = NULL;
	
    	// ensure that queries return expected results without DateFilter first
    	/*result = &searcher.search(query1);
    	assertEquals(0 == result->Length());
	
    	result = &searcher.search(query2);
    	assertEquals(1 == result->Length());
	*/
	
    	// run queries with DateFilter
    	result = &searcher.search(query1, df1);
    	assertEquals(0 == result->Length());
	
    	result = &searcher.search(query1, df2);
    	assertEquals(0 == result->Length());
	
     	result = &searcher.search(query2, df1);
     	assertEquals(1 == result->Length());
	
    	result = &searcher.search(query2, df2);
    	assertEquals(0 == result->Length());
	}
}}}
