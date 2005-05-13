#include "highlighter.h"

namespace lucene { namespace search { namespace highlight {

	class FragmentQueue : public PriorityQueue<TextFragment *>
	{
	public:
		FragmentQueue(int_t size)
		{
			initialize(size, true);
		}

	protected:
		bool lessThan(TextFragment * fragA, TextFragment * fragB)
		{
			if (fragA->getScore() == fragB->getScore())
				return fragA->getFragNum() > fragB->getFragNum();
			else
				return fragA->getScore() < fragB->getScore();
		}
	};


	Highlighter::Highlighter(Scorer * fragmentScorer)
		: maxDocBytesToAnalyze(DEFAULT_MAX_DOC_BYTES_TO_ANALYZE)
	{
		_fragmentScorer = fragmentScorer;
		_formatter = new SimpleHTMLFormatter();
		_textFragmenter = new SimpleFragmenter();
	}


	Highlighter::Highlighter(Formatter * formatter, Scorer * fragmentScorer)
		: maxDocBytesToAnalyze(DEFAULT_MAX_DOC_BYTES_TO_ANALYZE)
	{
		_fragmentScorer = fragmentScorer;
		_formatter = formatter;
		_textFragmenter = new SimpleFragmenter();
	}

	Highlighter::Highlighter(Formatter * formatter, Scorer * fragmentScorer, Fragmenter * textFragmenter)
		: maxDocBytesToAnalyze(DEFAULT_MAX_DOC_BYTES_TO_ANALYZE)
	{
		_fragmentScorer = fragmentScorer;
		_formatter = formatter;
		_textFragmenter = textFragmenter;
	}

	Highlighter::~Highlighter()
	{
		delete _fragmentScorer;
		delete _formatter;
		delete _textFragmenter;
	}

	char_t * Highlighter::getBestFragment(TokenStream * tokenStream, const char_t * text)
	{
		StringArray * results = getBestFragments(tokenStream,text, 1);
		char_t * result = 0;

		if (results->size() > 0)
			result = stringDuplicate(results->at(0));

		delete results;

		return result;
	}

	StringArray * Highlighter::getBestFragments(
		TokenStream * tokenStream,
		const char_t * text,
		int_t maxNumFragments)
	{
		maxNumFragments = std::max((int_t)1, maxNumFragments); //sanity check
		StringBuffer newText;

		TextFragmentList * frag = getBestDocFragments(tokenStream,text, &newText, maxNumFragments);

		mergeContiguousFragments(frag);
		frag->setDoDelete(DELETE_TYPE_DELETE);

		//Get text
		StringArray * fragTexts = new StringArray();
		fragTexts->setDoDelete(DELETE_TYPE_DELETE);

		for (uint_t i = 0; i < frag->size(); i++)
		{
			if ((frag->at(i)) && (frag->at(i)->getScore() > 0))
			{
				int_t chars_to_copy = frag->at(i)->getTextEndPos() - frag->at(i)->getTextStartPos();
				char_t * substring = new char_t[chars_to_copy+1];
				stringNCopy(substring, newText.getBuffer()+frag->at(i)->getTextStartPos(), chars_to_copy);
				substring[chars_to_copy]=0;
				fragTexts->put(substring);
			}
		}

		delete frag;

		return fragTexts;
	}

	TextFragmentList * Highlighter::getBestDocFragments(
		TokenStream * tokenStream,
		const char_t * text,
		StringBuffer * newText,
		int_t maxNumFragments)
	{
		TextFragmentList * docFrags = new TextFragmentList();


		TextFragment * currentFrag = new TextFragment(newText->length(), docFrags->size());
		_fragmentScorer->startFragment(currentFrag);
		docFrags->put(currentFrag);

		FragmentQueue fragQueue(maxNumFragments);

		_TRY
		{

			lucene::analysis::Token * token;
			char_t * tokenText;
			int_t startOffset;
			int_t endOffset;
			int_t lastEndOffset = 0;
			_textFragmenter->start(text);

			while ((token = tokenStream->next()) != 0)
			{

				startOffset = token->StartOffset();
				endOffset = token->EndOffset();
				//FIXME an issue was reported with CJKTokenizer that I couldnt reproduce
				// where the analyzer was producing overlapping tokens.
				// I suspect the fix is to make startOffset=Math.max(startOffset,lastEndOffset+1)
				// but cant be sure so I'll just leave this comment in for now
				tokenText = new char_t[endOffset-startOffset+1];
				stringNCopy(tokenText, text+startOffset, endOffset-startOffset);
				tokenText[endOffset-startOffset]=0;

				// append text between end of last token (or beginning of text) and start of current token
				if (startOffset > lastEndOffset) {
					newText->append(text+lastEndOffset, startOffset-lastEndOffset);
				}

				// Is fragment finished?
				if (_textFragmenter->isNewFragment(token))
				{
					currentFrag->setScore(_fragmentScorer->getFragmentScore());
					//record stats for a new fragment
					currentFrag->setTextEndPos(newText->length());
					currentFrag =new TextFragment(newText->length(), docFrags->size());
					_fragmentScorer->startFragment(currentFrag);
					docFrags->put(currentFrag);
				}

				// does query contain current token?
				float_t score=_fragmentScorer->getTokenScore(token);
				char_t * highlightedTerm = _formatter->highlightTerm(tokenText, token->TermText(), score, startOffset);
				newText->append(highlightedTerm);
				delete[] highlightedTerm;

				delete token;
				delete[] tokenText;

				lastEndOffset = endOffset;
				if(lastEndOffset>maxDocBytesToAnalyze)
				{
					break;
				}
			}
			currentFrag->setScore(_fragmentScorer->getFragmentScore());


			// append text after end of last token
			if (lastEndOffset < (int_t)stringLength(text))
				newText->append(text+lastEndOffset);

			currentFrag->setTextEndPos(newText->length());

			//sort the most relevant sections of the text
			float_t minScore = 0.0;
			while (docFrags->size() > 0) {
			//for (TextFragmentList::iterator i = docFrags->begin(); i != docFrags->end(); i++)
			//{
				currentFrag = (TextFragment*) docFrags->at(0);
				docFrags->remove(0);

				//If you are running with a version of Lucene before 11th Sept 03
				// you do not have PriorityQueue.insert() - so uncomment the code below

				if (currentFrag->getScore() >= minScore)
				{
					fragQueue.put(currentFrag);
					if (fragQueue.Size() > maxNumFragments)
					{ // if hit queue overfull
						delete fragQueue.pop(); // remove lowest in hit queue
						minScore = ((TextFragment *) fragQueue.top())->getScore(); // reset minScore
					}


				} else {
					delete currentFrag;
				}

				//The above code caused a problem as a result of Christoph Goller's 11th Sept 03
				//fix to PriorityQueue. The correct method to use here is the new "insert" method
				// USE ABOVE CODE IF THIS DOES NOT COMPILE!
				//fragQueue.insert(currentFrag);
			}

			delete docFrags;

			//return the most relevant fragments
			TextFragmentList * frag = new TextFragmentList;
			while (fragQueue.Size() > 0)
			{
				frag->push_front((TextFragment *) fragQueue.pop());
			}
			return frag;

		}
		_FINALLY(

			if (tokenStream)
			{
				try
				{
					tokenStream->close();
				}
				catch (...)
				{
				}
			}
			)
	}


	void Highlighter::mergeContiguousFragments(TextFragmentList * frag)
	{
		bool mergingStillBeingDone;
		if (frag->size() > 1)
			do
			{
				mergingStillBeingDone = false; //initialise loop control flag
				//for each fragment, scan other frags looking for contiguous blocks
				for (int_t i = 0; i < (int_t)frag->size(); i++)
				{
					if (frag->at(i) == 0)
					{
						continue;
					}
					//merge any contiguous blocks
					for (int_t x = 0; x < (int_t)frag->size(); x++)
					{
						if (frag->at(x) == 0)
						{
							continue;
						}
						if (frag->at(i) == 0)
						{
							break;
						}
						// Don't allow a fragment to merge with itself, because
						// it would then be deleted, resulting in invalid
						// memory:
						if (x == i)
						{
							continue;
						}
						TextFragment * frag1 = 0;
						TextFragment * frag2 = 0;
						int_t frag1Num = 0;
						int_t frag2Num = 0;
						int_t bestScoringFragNum;
						int_t worstScoringFragNum;
						//if blocks are contiguous....
						if (frag->at(i)->follows(frag->at(x)))
						{
							frag1 = frag->at(x);
							frag1Num = x;
							frag2 = frag->at(i);
							frag2Num = i;
						}
						else if (frag->at(x)->follows(frag->at(i)))
						{
							frag1 = frag->at(i);
							frag1Num = i;
							frag2 = frag->at(x);
							frag2Num = x;
						}
						//merging required..
						if (frag1 != 0)
						{
							if (frag1->getScore() > frag2->getScore())
							{
								bestScoringFragNum = frag1Num;
								worstScoringFragNum = frag2Num;
							}
							else
							{
								bestScoringFragNum = frag2Num;
								worstScoringFragNum = frag1Num;
							}
							frag1->merge(frag2);
							frag->set(worstScoringFragNum, 0);
							mergingStillBeingDone = true;
							frag->set(bestScoringFragNum, frag1);
							delete frag2;
						}
					}
				}
			}
			while (mergingStillBeingDone);
	}

	char_t * Highlighter::getBestFragments(
		TokenStream * tokenStream,
		const char_t * text,
		int_t maxNumFragments,
		const char_t * separator)
	{
		StringArray * sections = getBestFragments(tokenStream,text, maxNumFragments);
		StringBuffer result;

		for (int_t i = 0; i < (int_t)sections->size(); i++)
		{
			if (i > 0)
			{
				result.append(separator);
			}
			result.append((const char_t *)(sections->at(i)));
		}

		delete sections;
		return result.ToString();
	}



}}}