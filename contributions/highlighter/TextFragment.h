#ifndef _lucene_search_highlight_textfragment_
#define _lucene_search_highlight_textfragment_

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


/**
 * Low-level class used to record information about a section of a document 
 * with a score.
 * @author MAHarwood
 *
 * 
 */

#include <CLucene\StdHeader.h>

namespace lucene { namespace search { namespace highlight {

class TextFragment
{
private:
	int_t _fragNum;
	int_t _textStartPos;
	int_t _textEndPos;
	float_t _score;

public:
	TextFragment(int_t textStartPos, int_t fragNum)
	{
		_textStartPos = textStartPos;
		_fragNum = fragNum;
	}

	void setScore(float_t score)
	{
		_score=score;
	}

	float_t getScore() const
	{
		return _score;
	}
	/**
	 * @param frag2 Fragment to be merged into this one
	 */
	void merge(const TextFragment * frag2)
	{
		_textEndPos = frag2->getTextEndPos();
	}
	/**
	 * @param fragment 
	 * @return true if this fragment follows the one passed
	 */
	bool follows(const TextFragment * fragment) const
	{
		return _textStartPos == fragment->getTextEndPos();
	}

	/**
	 * @return the fragment sequence number
	 */
	int_t getFragNum() const
	{
		return _fragNum;
	}

	/**
	 * @return the fragment end position
	 */
	int_t getTextEndPos() const
	{
		return _textEndPos;
	}

	/**
	 * @return the fragment end position
	 */
	int_t getTextStartPos() const
	{
		return _textStartPos;
	}

	/**
	 * Modifies the fragment start position
	 */
	void setTextStartPos(int_t textStartPos)
	{
		_textStartPos = textStartPos;
	}

	/**
	 * Modifies the fragment end position
	 */
	void setTextEndPos(int_t textEndPos)
	{
		_textEndPos = textEndPos;
	}
};

/**
 * Text fragment list.
 */

typedef lucene::util::VoidList<TextFragment *> TextFragmentList;

}}}

#endif
