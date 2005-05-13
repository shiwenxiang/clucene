#ifndef _lucene_search_highlight_simplefragmenter_
#define _lucene_search_highlight_simplefragmenter_

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
#include <CLucene\Analysis\AnalysisHeader.h>
#include "Fragmenter.h"

namespace lucene { namespace search { namespace highlight {

/**
 * {@link Fragmenter} implementation which breaks text up into same-size 
 * fragments with no concerns over spotting sentence boundaries.
 * @author mark@searcharea.co.uk
 */

	class SimpleFragmenter: public Fragmenter
{
private:
	static const int_t DEFAULT_FRAGMENT_SIZE =100;
	int_t _currentNumFrags;
	int_t _fragmentSize;

public:
	/**
	 * 
	 * @param fragmentSize size in bytes of each fragment
	 */
	SimpleFragmenter(int fragmentSize = DEFAULT_FRAGMENT_SIZE)
		: _fragmentSize(fragmentSize), _currentNumFrags(0)
	{
	}

	/* (non-Javadoc)
	 * @see org.apache.lucene.search.highlight.TextFragmenter#start(java.lang.String)
	 */
	void start(const char_t *);

	/* (non-Javadoc)
	 * @see org.apache.lucene.search.highlight.TextFragmenter#isNewFragment(org.apache.lucene.analysis.Token)
	 */
	bool isNewFragment(const Token * token);

	/**
	 * @return size in bytes of each fragment
	 */
	int getFragmentSize() const;

	/**
	 * @param size size in bytes of each fragment
	 */
	void setFragmentSize(int size);

};

}}}

#endif
