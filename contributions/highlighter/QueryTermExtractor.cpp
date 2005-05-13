//#include "stdafx.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include "QueryTermExtractor.h"

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


namespace lucene { namespace search { namespace highlight {

	WeightedTermList * QueryTermExtractor::getTerms(Query * query, bool prohibited) 
	{
		WeightedTermList * terms=new WeightedTermList;


		terms->setDoDelete(true, DELETE_TYPE_DELETE);
		getTerms(query,terms,prohibited);

		// Return extracted terms
		return terms;
	}

	void QueryTermExtractor::getTerms(Query * query, WeightedTermList * terms,bool prohibited) 
	{
		if (query->instanceOf(_T("BooleanQuery")))
			getTermsFromBooleanQuery((BooleanQuery *) query, terms, prohibited);
		else
			if (query->instanceOf(_T("PhraseQuery")))
				getTermsFromPhraseQuery((PhraseQuery *) query, terms);
			else
				if (query->instanceOf(_T("TermQuery")))
					getTermsFromTermQuery((TermQuery *) query, terms);
	}

	void QueryTermExtractor::getTermsFromBooleanQuery(BooleanQuery * query, WeightedTermList * terms, bool prohibited)
	{
		// TODO: change Query to get the queryclauses and their number in one function call
		BooleanClause** queryClauses = query->getClauses();
		uint_t numClauses = query->getClauseCount();

		for (uint_t i = 0; i < numClauses; i++)
		{
			if (prohibited || !queryClauses[i]->prohibited)
				getTerms(&(queryClauses[i]->query), terms, prohibited);
		}

		delete[] queryClauses;
	}

	void QueryTermExtractor::getTermsFromPhraseQuery(PhraseQuery * query, WeightedTermList * terms)
	{
		Term** queryTerms;
		int_t  numQueryTerms;
		
		query->getTerms(queryTerms, numQueryTerms);
#if 1
		for (int_t i = 0; i < numQueryTerms; i++) {
			WeightedTerm * pWT = new WeightedTerm(query->getBoost(),queryTerms[i]->Text());
			if (!terms->exists(pWT)) // possible memory leak if key already present
				terms->put(pWT, pWT);
			else
				delete pWT;
		}
#endif
		delete[] queryTerms;
	}

	void QueryTermExtractor::getTermsFromTermQuery(TermQuery * query, WeightedTermList * terms)
	{
#if 1
		Term * term = query->getTerm();
		WeightedTerm * pWT = new WeightedTerm(query->getBoost(),term->Text());
		term->finalize();
		if (!terms->exists(pWT)) // possible memory leak if key already present
			terms->put(pWT, pWT);
		else
			delete pWT;
#endif
	}

}}}