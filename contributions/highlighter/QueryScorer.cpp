#include "QueryScorer.h"

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

	 QueryScorer::QueryScorer(Query * query)
	 {
		 _weighted_terms = QueryTermExtractor::getTerms(query);
		 initialize(_weighted_terms);
	 }


	 QueryScorer::QueryScorer(WeightedTermList * weightedTerms)
	 {
		 // Copy external weighted terms
		 _weighted_terms = new WeightedTermList();
		 _weighted_terms->setDoDelete(true, DELETE_TYPE_DELETE);
		 for (WeightedTermList::const_iterator it = weightedTerms->begin(); it != weightedTerms->end(); it++) {
			 WeightedTerm * term = new WeightedTerm(*(it->first));
			 _weighted_terms->put(term, term);
		 }

		 initialize(weightedTerms);
	}
	
	void QueryScorer::initialize(WeightedTermList * terms)
	{
		_currentTextFragment = 0;
		_totalScore = 0;
		_uniqueTermsInFragment.setDoDelete(true, DELETE_TYPE_DELETE_ARRAY);

		for (WeightedTermList::iterator it = terms->begin(); it != terms->end(); it++)
		{
			_termsToFind.put((*it).first->getTerm(),(*it).first);
		}
	}

	void QueryScorer::startFragment(TextFragment * newFragment)
	{
		_uniqueTermsInFragment.clear();
		_currentTextFragment=newFragment;
		_totalScore=0;
		
	}
	
	float_t QueryScorer::getTokenScore(Token * token)
	{
		const char_t * termText=token->TermText();
		
		WeightedTerm * queryTerm=(WeightedTerm *) _termsToFind.get(termText);
		if(queryTerm==0)
		{
			//not a query term - return
			return 0;
		}
		//found a query term - is it unique in this doc?
		if(!_uniqueTermsInFragment.exists(termText))
		{
			_totalScore+=queryTerm->getWeight();
			char_t * owned_term = stringDuplicate(termText);
			_uniqueTermsInFragment.put(owned_term, owned_term);
		}
		return queryTerm->getWeight();
	}
	
	



}}}
