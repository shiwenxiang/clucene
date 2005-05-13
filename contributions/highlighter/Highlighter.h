#ifndef _lucene_search_highlight_highlighter_
#define _lucene_search_highlight_highlighter_

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
#include <CLucene\util\StringBuffer.h>
#include <CLucene\util\PriorityQueue.h>
#include "Formatter.h"
#include "SimpleHTMLFormatter.h"
#include "Fragmenter.h"
#include "SimpleFragmenter.h"
#include "Scorer.h"
#include "TextFragment.h"

#undef max

using lucene::util::StringBuffer;
using lucene::util::PriorityQueue;

namespace lucene { namespace search { namespace highlight {

/**
 * Class used to markup highlighted terms found in the best sections of a 
 * text, using configurable {@link Fragmenter}, {@link Scorer}, {@link Formatter} 
 * and tokenizers.
 * @author mark@searcharea.co.uk
 */
class Highlighter
{
public:
	const static int_t DEFAULT_MAX_DOC_BYTES_TO_ANALYZE=50*1024;

private:
	int_t maxDocBytesToAnalyze;
	Formatter * _formatter;
	Fragmenter * _textFragmenter;
	Scorer * _fragmentScorer;

public:
	/**
	 * Constructs a Highlighter object with the provided scorer. The scorer object is owned
	 * by the Highlighter object, and it will freed in the destructor.
	 */

	Highlighter(Scorer * fragmentScorer);

	
	/**
	 * Constructs a Highlighter object with the provided formatter and scorer. The scorer and 
	 * the formatter objects are owned by the Highlighter object, and it will free them
	 * in the destructor.
	 */
	Highlighter(Formatter * formatter, Scorer * fragmentScorer);


	Highlighter(Formatter * formatter, Scorer * fragmentScorer, Fragmenter * textFragmenter);


	/**
	 * Destructor for Highlighter. It deletes the owned scorer, formatter and textFragmenter.
	 */

	~Highlighter();

	/**
	 * Highlights chosen terms in a text, extracting the most relevant section.
	 * The document text is analysed in chunks to record hit statistics
	 * across the document. After accumulating stats, the fragment with the highest score
	 * is returned
	 *
	 * @param tokenStream   a stream of tokens identified in the text parameter, including offset information. 
	 * This is typically produced by an analyzer re-parsing a document's 
	 * text. Some work may be done on retrieving TokenStreams more efficently 
	 * by adding support for storing original text position data in the Lucene
	 * index but this support is not currently available (as of Lucene 1.4 rc2).  
	 * @param text text to highlight terms in
	 *
	 * @return highlighted text fragment or null if no terms found
	 */
	char_t * getBestFragment(TokenStream * tokenStream, const char_t * text);

	/**
	 * Highlights chosen terms in a text, extracting the most relevant sections.
	 * The document text is analysed in chunks to record hit statistics
	 * across the document. After accumulating stats, the fragments with the highest scores
	 * are returned as an array of strings in order of score (contiguous fragments are merged into 
	 * one in their original order to improve readability)
	 *
	 * @param text        	text to highlight terms in
	 * @param maxNumFragments  the maximum number of fragments.
	 *
	 * @return highlighted text fragments (between 0 and maxNumFragments number of fragments)
	 */
	StringArray * getBestFragments(
		TokenStream * tokenStream,	
		const char_t * text,
		int_t maxNumFragments);

private:
	/**
	 * Low level api to get the most relevant sections of the document
	 * @param tokenStream
	 * @param text
	 * @param maxNumFragments
	 * @return 
	 * @throws IOException
	 */
	TextFragmentList * getBestDocFragments(
		TokenStream * tokenStream,	
		const char_t * text,
		StringBuffer * newText,
		int_t maxNumFragments);


	/** Improves readability of a score-sorted list of TextFragments by merging any fragments 
	 * that were contiguous in the original text into one larger fragment with the correct order.
	 * This will leave a "null" in the array entry for the lesser scored fragment. 
	 * 
	 * @param frag An array of document fragments in descending score
	 */
	void mergeContiguousFragments(TextFragmentList * frag);
	
	public:
	/**
	 * Highlights terms in the  text , extracting the most relevant sections
	 * and concatenating the chosen fragments with a separator (typically "...").
	 * The document text is analysed in chunks to record hit statistics
	 * across the document. After accumulating stats, the fragments with the highest scores
	 * are returned in order as "separator" delimited strings.
	 *
	 * @param text        text to highlight terms in
	 * @param maxNumFragments  the maximum number of fragments.
	 * @param separator  the separator used to intersperse the document fragments (typically "...")
	 *
	 * @return highlighted text
	 */
	char_t * getBestFragments(
		TokenStream * tokenStream,	
		const char_t * text,
		int_t maxNumFragments,
		const char_t * separator);

	/**
	 * @return the maximum number of bytes to be tokenized per doc 
	 */
	int_t getMaxDocBytesToAnalyze()
	{
		return maxDocBytesToAnalyze;
	}

	/**
	 * @param byteCount the maximum number of bytes to be tokenized per doc
	 * (This can improve performance with large documents)
	 */
	void setMaxDocBytesToAnalyze(int_t byteCount)
	{
		maxDocBytesToAnalyze = byteCount;
	}

	/**
	 * @return
	 */
	Fragmenter * getTextFragmenter()
	{
		return _textFragmenter;
	}

	/**
	 * @param fragmenter
	 */
	void setTextFragmenter(Fragmenter * fragmenter)
	{
		_textFragmenter = fragmenter;
	}

	/**
	 * @return Object used to score each text fragment 
	 */
	Scorer * getFragmentScorer()
	{
		return _fragmentScorer;
	}


	/**
	 * @param scorer
	 */
	void setFragmentScorer(Scorer * scorer)
	{
		_fragmentScorer = scorer;
	}


};


}}}

#endif

