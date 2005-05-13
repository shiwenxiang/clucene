#include "CLucene/analysis/AnalysisHeader.h"
#include "CLucene/search/SearchHeader.h"
#include "CLucene/analysis/SimpleAnalyzer.h"
#include "CLucene/queryParser/QueryParser.h"
#include <iostream>

Query& getQuery(const char_t_t* query, Analyzer* a){
	if (a == NULL)
		a = new SimpleAnalyzer();
	lucene::queryParser::QueryParser qp("field", *a);

	return qp.parse(query);
}
void assertQueryEquals(const char_t_t* query, Analyzer* a, const char_t* result) {
	Query& q = getQuery(query, a);
	const char_t* s = q.toString("field");
	//if (!s.equals(result)) {
		cout << "Query /" << query << "/ yielded /" << s << "/, expecting /" << result << "/" << endl;
	//}
}

void testSimple() {
    assertQueryEquals("term term term", NULL, "term term term");
    assertQueryEquals("türm term term", NULL, "türm term term");
}
