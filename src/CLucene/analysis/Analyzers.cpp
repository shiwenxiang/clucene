#include "CLucene/StdHeader.h"
#include "Analyzers.h"

namespace lucene{ namespace analysis {
		
CharTokenizer::CharTokenizer(Reader* in) :
			//input(in), ; input is in tokenizer base class (bug fix thanks to Andy Osipienko)
			offset(0),
			bufferIndex(0),
			dataLen(0)

{
	input = in; //; input is in tokenizer base class (bug fix thanks to Andy Osipienko)
}

uchar_t CharTokenizer::normalize(const uchar_t c) 
{ 
	return c; 
}

Token* CharTokenizer::next() {
	int_t length = 0;
#ifdef UTF8
	int_t start = input->position();
	while (true) {
		uchar_t c;
		offset++;
		c = input->readChar();
		if (!c)
		{
			if (length > 0)
				break;
			else
				return NULL;
		}
		if (isTokenChar(c)) {                       // if it's a token char_t
			buffer[length++] = normalize(c);          // buffer it, normalized

			if (length == MAX_WORD_LEN)		  // buffer overflow!
				throw "token too large for the buffer";

		} 
		else 
		{
			if (length > 0)			  // at non-Letter w/ chars
				break;					  // return 'em
		}
	}
	buffer[length]=0;
	char *tokbuf = lc_ucs4_to_utf8(buffer, length);
	offset = input->position();
	Token *retval = new Token( tokbuf, start, offset);
	delete[] tokbuf;
	return retval;
#else
	int_t start = offset;
	while (true) {
		uchar_t c;
		offset++;
		if (bufferIndex >= dataLen) {
			dataLen = input->read(ioBuffer,0,IO_BUFFER_SIZE);
			bufferIndex = 0;
		};
		if (dataLen <= 0 ) {
			if (length > 0)
				break;
			else
				return NULL;
			}
		else
			c = ioBuffer[bufferIndex++];
		if (isTokenChar(c)) {                       // if it's a token char_t

			if (length == 0)			  // start of token
				start = offset-1;

			buffer[length++] = normalize(c);          // buffer it, normalized

			if (length == MAX_WORD_LEN)		  // buffer overflow!
				break;

		} else if (length > 0)			  // at non-Letter w/ chars
			break;					  // return 'em

	}
	buffer[length]=0;
	return new Token( buffer, start, start+length);
#endif
}

// Collects only characters which satisfy
// {@link Character#isLetter(char_t)}.
bool LetterTokenizer::isTokenChar(const uchar_t c) {
	//printf("checking if %d is a letter(%d)\n", c, Misc::isLetter(c));fflush(NULL);
	return Misc::isLetter(c)!=0;
}


// Collects only characters which satisfy
// {@link Character#isLetter(char_t)}.
uchar_t LowerCaseTokenizer::normalize(const uchar_t chr) {
	//printf("normalizing lowering %d\n", chr);fflush(NULL);
	return toLower(chr);
}

		
// Collects only characters which do not satisfy
// {@link Character#isWhitespace(char_t)}.
bool WhitespaceTokenizer::isTokenChar(const uchar_t c) {
	return isSpace(c)==0; //(return true if NOT a space)
}


TokenStream& WhitespaceAnalyzer::tokenStream(const char_t* fieldName, Reader* reader) {
	return *new WhitespaceTokenizer(reader);
}


TokenStream& SimpleAnalyzer::tokenStream(const char_t* fieldName, Reader* reader) {
	return *new LowerCaseTokenizer(reader);
}


Token* LowerCaseFilter::next(){
	Token* t = input->next();
	if (t == NULL)
		return NULL;
	#ifdef UTF8
	 char *tmpstr = lc_utf8_strcasefold( t->termText );
	 delete [] t->termText;
	 t->termText = tmpstr;
	#else
 	 stringLower( t->termText );
	#endif
	return t;
}


StopFilter::StopFilter(TokenStream* in, bool deleteTokenStream, char_t* stopWords[], int_t stopWordsLength):
	TokenFilter(in, deleteTokenStream),
	table(*new VoidMap< char_t*, char_t*>)
{
	fillStopTable( table,stopWords,stopWordsLength );
}

// Builds a Hashtable from an array of stop words, appropriate for passing
//	into the StopFilter constructor.  This permits this table construction to
//	be cached once when an Analyzer is constructed. 
// Note: the stopWords list must be a static list
void StopFilter::fillStopTable(VoidMap< char_t*, char_t*>& stopTable,
								  char_t* stopWords[],
								  int_t length) {
	for (int_t i = 0; i < length; i++)
		stopTable.put(stopWords[i],stopWords[i]);
}

// Returns the next input Token whose termText() is not a stop word. 
Token* StopFilter::next() {
	// return the first non-stop word found
	//printf("StopFilter::next\n");fflush(NULL);
	for (Token* token = input->next(); token != NULL; token = input->next()){
		if (!table.exists(token->termText)){
			//printf("StopFilter::next returning %s\n", token->termText);fflush(NULL);
			return token;
		}else
		{
			//printf("StopFilter::next deleting stopped %s\n", token->termText);fflush(NULL);
			delete token;
		}
	}

	// reached EOS -- return null
	return NULL;
}

StopAnalyzer::StopAnalyzer()
{
	StopFilter::fillStopTable(stopTable,ENGLISH_STOP_WORDS,ENGLISH_STOP_WORDS_LENGTH);
}
StopAnalyzer::StopAnalyzer( char_t* stopWords[], int_t length) {
	StopFilter::fillStopTable(stopTable,stopWords,length);
}
// Filters LowerCaseTokenizer with StopFilter. 
TokenStream& StopAnalyzer::tokenStream(const char_t* fieldName, Reader* reader) {
	return *new StopFilter(new LowerCaseTokenizer(reader),true, stopTable);
}

char_t* StopAnalyzer::ENGLISH_STOP_WORDS [] = 
{
	_T("a"), _T("and"), _T("are"), _T("as"), _T("at"), _T("be"), _T("but"), _T("by"),
	_T("for"), _T("if"), _T("in"), _T("into"), _T("is"), _T("it"),
	_T("no"), _T("not"), _T("of"), _T("on"), _T("or"), _T("s"), _T("such"),
	_T("t"), _T("that"), _T("the"), _T("their"), _T("then"), _T("there"), _T("these"),
	_T("they"), _T("this"), _T("to"), _T("was"), _T("will"), _T("with")
};

}}


