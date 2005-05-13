#ifndef _lucene_search_highlight_weightedterm_
#define _lucene_search_highlight_weightedterm_


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


#include "CLucene\StdHeader.h"

namespace lucene { namespace search { namespace highlight {

/** Lightweight class to hold term and a weight value used for scoring this term 
 * @author Mark Harwood
 */
class WeightedTerm
{
private:
	float_t _weight; // multiplier
	char_t * _term; //stemmed form
public:
	WeightedTerm (float_t weight,const char_t * term)
	{
		_weight=weight;
		_term = stringDuplicate(term);
	}
	
	~WeightedTerm()
	{
		delete[] _term;
	}

	WeightedTerm(const WeightedTerm & other) 
	{
		_weight = other.getWeight();
		_term = stringDuplicate(other.getTerm());
	}

	/**
	 * @return the term value (stemmed)
	 */
	char_t * getTerm() const
	{
		return _term;
	}

	/**
	 * @return the weight associated with this term
	 */
	float_t getWeight() const 
	{
		return _weight;
	}

	/**
	 * @param term the term value (stemmed)
	 */
	void setTerm(char_t *term)
	{
		delete[] _term;
		_term = stringDuplicate(term);
	}

	/**
	 * @param weight the weight associated with this term
	 */
	void setWeight(float_t weight) {
		_weight = weight;
	}

};

	/**
	 * Compare weighted terms, according to the term text.
	 * @returns true if elem1 is greater then elem2
	 * @todo Do we have to take boost factors into account
	 */
	class weightedTermCompare:public binary_function<const WeightedTerm*,const WeightedTerm*,bool>
	{
	public:
		bool operator()( const WeightedTerm* elem1, const WeightedTerm* elem2 ) const{
			bool ret = (stringCompare(elem1->getTerm(), elem2->getTerm()) < 0);
			return ret;
		}
	};



}}}


#endif

