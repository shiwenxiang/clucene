#ifndef _lucene_search_highlight_querytermextractor_
#define _lucene_search_highlight_querytermextractor_

/**
 * Copyright 2002-2004 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <CLucene\StdHeader.h>
#include <CLucene\Util\VoidList.h>
#include <CLucene\Search\SearchHeader.h>
#include <CLucene\Search\BooleanQuery.h>
#include <CLucene\Search\PhraseQuery.h>
#include <CLucene\Search\TermQuery.h>



#include ".\WeightedTerm.h"



using namespace lucene::search;


namespace lucene { namespace search { namespace highlight {


/** VoidMap of WeightedTerm */
typedef lucene::util::VoidMap<WeightedTerm*, WeightedTerm *, weightedTermCompare> WeightedTermList;


/**
 * Utility class used to extract the terms used in a query, plus any weights.
 * This class will not find terms for MultiTermQuery, RangeQuery and PrefixQuery classes
 * so the caller must pass a rewritten query (see query.rewrite) to obtain a list of 
 * expanded terms. 
 * 
 */
class QueryTermExtractor
{

public:

	/**
	 * Extracts all terms texts of a given Query into an array of WeightedTerms
	 *
	 * @param query      Query to extract term texts from
	 * @return an array of the terms used in a query, plus their weights. Memory owned by the caller
	 */
	static WeightedTermList * getTerms(Query *query) 
	{
		return getTerms(query,false);
	}


	/**
	 * Extracts all terms texts of a given Query into an array of WeightedTerms
	 *
	 * @param query      Query to extract term texts from
	 * @param prohibited <code>true</code> to extract "prohibited" terms, too
   * @return an array of the terms used in a query, plus their weights.Memory owned by the caller
   */
	static WeightedTermList* getTerms(Query * query, bool prohibited);


	static void getTerms(Query * query, WeightedTermList* terms,bool prohibited);
	static void getTermsFromBooleanQuery(BooleanQuery * query, WeightedTermList* terms, bool prohibited);
	static void getTermsFromPhraseQuery(PhraseQuery * query, WeightedTermList* terms);
	static void getTermsFromTermQuery(TermQuery * query, WeightedTermList* terms);


};

}}}



#endif
