#ifndef _lucene_search_highlight_simplehtmlformatter_
#define _lucene_search_highlight_simplehtmlformatter_

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
#include <CLucene\Util\StringBuffer.h>
#include "Formatter.h"

using namespace lucene::util;

namespace lucene { namespace search { namespace highlight {

/**
 * Simple {@link Formatter} implementation to highlight terms with a pre and post tag
 * @author MAHarwood
 *
 */
	class SimpleHTMLFormatter : public Formatter
{
private:
	const char_t * _preTag;
	const char_t * _postTag;

public:
	SimpleHTMLFormatter(const char_t * preTag, const char_t * postTag);

	/**
	 * Default constructor uses HTML: &lt;B&gt; tags to markup terms
	 * 
	 **/
	SimpleHTMLFormatter();

	~SimpleHTMLFormatter(); 

	char_t * highlightTerm(const char_t * originalText, const char_t * term, float_t score, int_t startOffset);

};

}}}

#endif
