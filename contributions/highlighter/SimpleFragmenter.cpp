#include "SimpleFragmenter.h"

namespace lucene { namespace search { namespace highlight {

void SimpleFragmenter::start(const char_t *)
{
	_currentNumFrags=1;
}

bool SimpleFragmenter::isNewFragment(const Token * token)
{
	bool isNewFrag= token->EndOffset()>=(_fragmentSize*_currentNumFrags);
	if (isNewFrag) {
		_currentNumFrags++;
	}
	return isNewFrag;
}

int SimpleFragmenter::getFragmentSize() const
{
	return _fragmentSize;
}

void SimpleFragmenter::setFragmentSize(int size)
{
	_fragmentSize = size;
}

}}}