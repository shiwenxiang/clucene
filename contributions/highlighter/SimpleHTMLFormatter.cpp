//#include "stdafx.h"
#include "SimpleHTMLFormatter.h"


namespace lucene { namespace search { namespace highlight {

/**
*	Constructor.
*/

SimpleHTMLFormatter::SimpleHTMLFormatter(const char_t * preTag, const char_t * postTag):
	_preTag(stringDuplicate(preTag)),
	_postTag(stringDuplicate(postTag))
{
}

/**
* Default constructor uses HTML: &lt;B&gt; tags to markup terms
* 
**/

SimpleHTMLFormatter::SimpleHTMLFormatter()
{
	_preTag = stringDuplicate(_T("<B>"));
	_postTag = stringDuplicate(_T("</B>"));
}

/**
 * Destructor.
 */

SimpleHTMLFormatter::~SimpleHTMLFormatter() 
{
	delete[] _preTag;
	delete[] _postTag;
}

/**
 * Returns the original text enclosed in _preTag and _postTag, if the score is greater 
 * than 0. Otherwise, it returns the original text.
 * It doesn't use the stemmed text nor the startOffset. 
 * It allocates memory for the returned text, and it has to be freed by the caller.
 */

char_t * SimpleHTMLFormatter::highlightTerm(const char_t * originalText, const char_t * term, float_t score, int_t startOffset)
{
	if(score<=0) {
		return stringDuplicate(originalText);
	}
	StringBuffer sb;
	sb.append(_preTag);
	sb.append(originalText);
	sb.append(_postTag);
	return sb.ToString();
}

}}}