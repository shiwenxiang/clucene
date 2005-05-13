#ifndef _lucene_search_highlight_formatter_
#define _lucene_search_highlight_formatter_

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

namespace lucene { namespace search { namespace highlight {

/**
 * Processes terms found in the original text, typically by applying some form 
 * of mark-up to highlight terms in HTML search results pages.
 *
 */
class Formatter
{
public:

	/** Virtual destructor */
	virtual ~Formatter() {}

  /**
   * Highlights a search term. For example, an HTML Formatter could simply do:
   *
   * <p><dl><dt></dt><dd><code>return "&lt;b&gt;" + term + "&lt;/b&gt;";</code></dd></dl>
   *
   * @param originalTermText (unstemmed) term text to highlight
   * @param stemmedTerm the stemmed form of the originalTermText
   * @param startOffset the position of the originalTermText in the text being highlighted  
   *
   * @return highlighted term text (memory owned by the caller)
   */
  virtual char_t * highlightTerm(const char_t * originalTermText, const char_t * stemmedTerm, float_t score, int_t startOffset) = 0;

};

}}}

#endif



