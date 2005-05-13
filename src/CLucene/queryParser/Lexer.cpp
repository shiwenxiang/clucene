#include "CLucene/StdHeader.h"
#include "Lexer.h"

#include "QueryParserConstants.h"
#include "CLucene/util/FastCharStream.h"
#include "CLucene/util/Reader.h"
#include "CLucene/util/StringBuffer.h"
#include "TokenList.h"
#include "QueryToken.h"
#include "QueryParserBase.h"

using namespace lucene::util;

namespace lucene{ namespace queryParser{

  Lexer::Lexer(const char_t* query){
    //Func - Constructor
    //Pre  - query != NULL and contains the query string
    //Post - An instance of Lexer has been created

        CND_PRECONDITION(query != NULL, "query is NULL")

    //The InputStream of Reader must be destroyed in the destructor
        delSR = true;

    StringReader *r = new StringReader(query);

        //Check to see if r has been created properly
        CND_CONDITION(r != NULL, "Could not allocate memory for StringReader r")

    //Instantie a FastCharStream instance using r and assign it to reader
        reader = new FastCharStream(*r);

    //Check to see if reader has been created properly
    CND_CONDITION(reader != NULL, "Could not allocate memory for FastCharStream reader")

        //The InputStream of Reader must be destroyed in the destructor
        delSR = true;

    }

  Lexer::Lexer(Reader& source){
  //Func - Constructor
    //       Initializes a new instance of the Lexer class with the specified
    //       TextReader to lex.
    //Pre  - Source contains a valid reference to a Reader
    //Post - An instance of Lexer has been created using source as the reader

        //Instantie a FastCharStream instance using r and assign it to reader
        reader = new FastCharStream(source);

    //Check to see if reader has been created properly
    CND_CONDITION(reader != NULL, "Could not allocate memory for FastCharStream reader")

    //The InputStream of Reader must not be destroyed in the destructor
        delSR  = false;
    }

    Lexer::~Lexer(){
  //Func - Destructor
  //Pre  - true
  //Post - if delSR was true the InputStream input of reader has been deleted
  //       The instance of Lexer has been destroyed

    if (delSR){
            delete &reader->input;
            }

        _DELETE(reader);
    }

  void Lexer::Lex(TokenList *tokenList){
  //Func - Breaks the input stream onto the tokens list tokens
  //Pre  - tokens != NULL and contains a TokenList in which the tokens can be stored
  //Post - The tokens have been added to the TokenList tokens

        CND_PRECONDITION(tokenList != NULL, "tokens is NULL")

    //Get the next token
        QueryToken* token = NULL;

    //Get all the tokens
    while((token = GetNextToken()) != NULL){
            //Add the token to the tokens list
            tokenList->Add(token);
       }


    //The end has been reached so create an EOF_ token
    token = new QueryToken( lucene::queryParser::EOF_);

        //Check to see if token has been created properly
    CND_CONDITION(token != NULL, "Could not allocate memory for QueryToken token")

    //Add the final token to the TokenList _tokens
    tokenList->Add(token);
    }

    QueryToken* Lexer::GetNextToken()
    {
      while(!reader->Eos())
      {
        uchar_t ch = reader->GetNext();

        // skipping whitespaces
        if( isSpace(ch)!=0 )
        {
          continue;
        }
        char_t buf[250] = {ch,'\0'};
        switch(ch)
        {
          case '+':
            return new QueryToken(buf , lucene::queryParser::PLUS);
          case '-':
            return new QueryToken(buf, lucene::queryParser::MINUS);
          case '(':
            return new QueryToken(buf, lucene::queryParser::LPAREN);
          case ')':
            return new QueryToken(buf, lucene::queryParser::RPAREN);
          case ':':
            return new QueryToken(buf, lucene::queryParser::COLON);
          case '!':
            return new QueryToken(buf, lucene::queryParser::NOT);
          case '^':
            return new QueryToken(buf, lucene::queryParser::CARAT);
          case '~':
#ifndef NO_FUZZY_QUERY
          if( isDigit( reader->Peek() )!=0 )
            {
              const char_t* number = ReadIntegerNumber(ch);
              QueryToken* ret = new QueryToken(number, lucene::queryParser::SLOP);
            delete[] number;
            return ret;
            }else
            {
              return new QueryToken(buf, lucene::queryParser::FUZZY);
            }
#endif
          case '"':
            return ReadQuoted(ch);
#ifndef NO_RANGE_QUERY
          case '[':
            return ReadInclusiveRange(ch);
          case '{':
            return ReadExclusiveRange(ch);
#endif
          case ']':
          case '}':
          case '*':
            QueryParserBase::throwParserException( _T("Unrecognized char_t %d at %d::%d."), ch, reader->Column(), reader->Line() );
          default:
            return ReadTerm(ch);

        } // end of swith

      }
      return NULL;
    }

    // Reads an integer number
    const char_t* Lexer::ReadIntegerNumber(const uchar_t ch)
    {
      StringBuffer number;
      number.append(ch); //TODO: check this
      while(!reader->Eos() && isDigit(reader->Peek())!=0 )
      {
        number.append(reader->GetNext());
      }
      return number.ToString();
    }

#ifndef NO_RANGE_QUERY
    // Reads an inclusive range like [some words]
    QueryToken* Lexer::ReadInclusiveRange(const uchar_t prev)
    {
      uchar_t ch = prev;
      StringBuffer range;
      range.append(ch);

      while(!reader->Eos())
      {
        ch = reader->GetNext();
        range.append(ch);

        if(ch == ']')
          return new QueryToken(range.getBuffer(), lucene::queryParser::RANGEIN);
      }
      /* DSR:CL_BUG: The old format string contained %s where it should have
      ** contained %d, which caused sprintf (Linux, glibc 2.3.2) to crash
      ** because the char was eventually passed to strlen. */
      QueryParserBase::throwParserException(_T("Unterminated inclusive range! %d %d::%d"),' ',reader->Column(),reader->Column());
      return NULL;
    }

    // Reads an exclusive range like {some words}
    QueryToken* Lexer::ReadExclusiveRange(const uchar_t prev)
    {
      uchar_t ch = prev;
      StringBuffer range;
      range.append(ch);

      while(!reader->Eos())
      {
        ch = reader->GetNext();
        range.append(ch);

        if(ch == '}')
          return new QueryToken(range.getBuffer(), lucene::queryParser::RANGEEX);
      }
      /* DSR:CL_BUG: The old format string contained %s where it should have
      ** contained %d, which caused sprintf (Linux, glibc 2.3.2) to crash
      ** because the char was eventually passed to strlen. */
      QueryParserBase::throwParserException(_T("Unterminated exclusive range! %d %d::%d"),' ',reader->Column(),reader->Column() );
      return NULL;
    }
#endif

    // Reads quoted string like "something else"
    QueryToken* Lexer::ReadQuoted(const uchar_t prev)
    {
      uchar_t ch = prev;
      StringBuffer quoted;
      quoted.append(ch);

      while(!reader->Eos())
      {
        ch = reader->GetNext();
        quoted.append(ch);

        if(ch == '"')
          return new QueryToken(quoted.getBuffer(), lucene::queryParser::QUOTED);
      }
      /* DSR:CL_BUG: The old format string contained %s where it should have
      ** contained %d, which caused sprintf (Linux, glibc 2.3.2) to crash
      ** because the char was eventually passed to strlen. */
      QueryParserBase::throwParserException(_T("Unterminated string! %d %d::%d"),' ',reader->Column(),reader->Column());
      return NULL;
    }

    QueryToken* Lexer::ReadTerm(const uchar_t prev)
    {
      uchar_t ch = prev;
      bool completed = false;
      int_t asteriskCount = 0;
      bool hasQuestion = false;

      StringBuffer val;

      while(true)
      {
        switch(ch)
        {
        case '\\':
          {
            const char_t* re = ReadEscape(ch);
              val.append( re );
            delete[] re;
          }
            break;
#ifndef NO_WILDCARD_QUERY
          case '*':
            asteriskCount++;
            val.append(ch);
            break;
          case '?':
            hasQuestion = true;
            val.append(ch);
            break;
#endif
          case '\n':
          case '\t':
          case ' ':
          case '+':
          case '-':
          case '!':
          case '(':
          case ')':
          case ':':
          case '^':
#ifndef NO_RANGE_QUERY
          case '[':
          case ']':
          case '{':
          case '}':
#endif
          case '"':
#ifndef NO_FUZZY_QUERY
          case '~':
            // create new QueryToken
            reader->UnGet();
            completed = true;
            break;
#endif
          default:
            val.append(ch);
            break;
        } // end of switch

        if(completed || reader->Eos())
          break;
        else
          ch = reader->GetNext();
      }

      // create new QueryToken
    if(false)
      return NULL;
#ifndef NO_WILDCARD_QUERY
    else if(hasQuestion)
        return new QueryToken(val.getBuffer(), lucene::queryParser::WILDTERM);
#endif
#ifndef NO_PREFIX_QUERY
      else if(asteriskCount == 1 && val.getBuffer()[val.length() - 1] == '*')
        return new QueryToken(val.getBuffer(), lucene::queryParser::PREFIXTERM);
#endif
#ifndef NO_WILDCARD_QUERY
      else if(asteriskCount > 0)
        return new QueryToken(val.getBuffer(), lucene::queryParser::WILDTERM);
#endif
      else if( stringICompare(val.getBuffer(), _T("AND"))==0 || stringCompare(val.getBuffer(), _T("&&"))==0 )
        return new QueryToken(val.getBuffer(), lucene::queryParser::AND_);

      else if( stringICompare(val.getBuffer(), _T("OR"))==0 || stringCompare(val.getBuffer(), _T("||"))==0)
        return new QueryToken(val.getBuffer(), lucene::queryParser::OR);

      else if( stringICompare(val.getBuffer(), _T("NOT"))==0 )
        return new QueryToken(val.getBuffer(), lucene::queryParser::NOT);

      else{

        char_t* lwr = stringLower( val.ToString() );
        char_t* upr = stringUpper( val.ToString() );
        bool n = (stringCompare(lwr,upr) == 0);
        delete[] lwr; delete[] upr;

        if ( n )
          return new QueryToken(val.getBuffer(), lucene::queryParser::NUMBER);
        else
          return new QueryToken(val.getBuffer(), lucene::queryParser::TERM);
      }
    }


    const char_t* Lexer::ReadEscape(const uchar_t prev)
    {
      uchar_t ch = prev;
      StringBuffer val;
      val.append(ch);

      ch = reader->GetNext();
      int_t idx = stringCSpn( val.getBuffer(), _T("\\+-!():^[]{}\"~*") );
      if(idx == 0)
      {
        val.append( ch );
        return val.ToString();
      }
      /* DSR:CL_BUG: The old format string contained %s where it should have
      ** contained %d, which caused sprintf (Linux, glibc 2.3.2) to crash
      ** because the char was eventually passed to strlen. */
      QueryParserBase::throwParserException(_T("Unrecognized escape sequence at %d %d::%d"), ' ',reader->Column(),reader->Line());
      return 0;
    }
}}
