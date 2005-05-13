#ifndef _lucene_search_highlighter_queryscorer_
#define _lucene_search_highlighter_queryscorer_

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

#include <CLucene\Analysis\AnalysisHeader.h>
#include <CLucene\Search\SearchHeader.h>
#include "Scorer.h"
#include "WeightedTerm.h"
#include "QueryTermExtractor.h"

namespace lucene { namespace search { namespace highlight {

/**
 * {@link Scorer} implementation which scores text fragments by the number of unique query terms found.
 * This class uses the {@link QueryTermExtractor} class to process determine the query terms and 
 * their boosts to be used. 
 * @author mark@searcharea.co.uk
 */
//TODO: provide option to roll idf into the scoring equation by passing a IndexReader.
//TODO: provide option to boost score of fragments near beginning of document 
// based on fragment.getFragNum()
	class QueryScorer : public Scorer
	{
	private:
		TextFragment * _currentTextFragment;
		lucene::util::VoidMap<const char_t *, const char_t *> _uniqueTermsInFragment;
		float_t _totalScore;
		lucene::util::VoidMap<const char_t *, const WeightedTerm *> _termsToFind;
		WeightedTermList *_weighted_terms;
	
	public:
	/**
	 * 
	 * @param query a Lucene query (ideally rewritten using query.rewrite 
	 * before being passed to this class and the searcher)
	 */
	QueryScorer(Query * query);


	QueryScorer(WeightedTermList *weightedTerms);
	

	~QueryScorer()
	{
		delete _weighted_terms;
	}

	private:
	void initialize(WeightedTermList * weightedTerms);

	public:
	/* (non-Javadoc)
	 * @see org.apache.lucene.search.highlight.FragmentScorer#startFragment(org.apache.lucene.search.highlight.TextFragment)
	 */
	void startFragment(TextFragment * newFragment);
	
	/* (non-Javadoc)
	 * @see org.apache.lucene.search.highlight.FragmentScorer#scoreToken(org.apache.lucene.analysis.Token)
	 */
	float_t getTokenScore(Token * token);
	
	/* (non-Javadoc)
	 * @see org.apache.lucene.search.highlight.FragmentScorer#endFragment(org.apache.lucene.search.highlight.TextFragment)
	 */
	float_t getFragmentScore()
	{
		return _totalScore;		
	}


	/* (non-Javadoc)
	 * @see org.apache.lucene.search.highlight.FragmentScorer#allFragmentsProcessed()
	 */
	void allFragmentsProcessed()
	{
		//this class has no special operations to perform at end of processing
	}

};

}}}

#endif

