#include <assert.h>
#include "StandardTokenizer.h"

using namespace lucene::analysis;
using namespace lucene::util;
namespace lucene{ namespace analysis { namespace standard {

  /* A bunch of shortcut macros, many of which make assumptions about variable
  ** names.  These macros enhance readability, not just convenience! */
  #define EOS             (rd.Eos())
  #define SPACE           (isSpace((char_t)ch) != 0)
  #define ALPHA           (Misc::isLetter((int_t)ch) != 0)
  #define ALNUM           (isAlNum(ch) != 0)
  #define DIGIT           (isDigit(ch) != 0)
  #define UNDERSCORE      (ch == '_')

  #define DASH            (ch == '-')
  #define NEGATIVE_SIGN_  DASH
  #define POSITIVE_SIGN_  (ch == '+')
  #define SIGN            (NEGATIVE_SIGN_ || POSITIVE_SIGN_)

  #define DOT             (ch == '.')
  #define DECIMAL         DOT


  #define _CONSUME_AS_LONG_AS(conditionFails) \
    while (!EOS) { \
      ch = readChar(); \
      if (!(conditionFails)) { \
        break; \
      } \
      str.append(ch); \
    }

  #define CONSUME_ALPHAS _CONSUME_AS_LONG_AS(ALPHA)

  #define CONSUME_DIGITS _CONSUME_AS_LONG_AS(DIGIT)

  /* otherMatches is a condition (possibly compound) under which a character
  ** that's not an ALNUM or UNDERSCORE can be considered not to break the
  ** span.  Callers should pass false if only ALNUM/UNDERSCORE are acceptable. */
  #define CONSUME_WORD                  _CONSUME_AS_LONG_AS(ALNUM || UNDERSCORE)
  #define CONSUME_WORD_OR(otherMatches) _CONSUME_AS_LONG_AS(ALNUM || UNDERSCORE || (otherMatches))


  /* It is considered that "nothing of value" has been read if:
  ** a) The "read head" hasn't moved since specialCharPos was established.
  ** or
  ** b) The "read head" has moved by one character, but that character was
  **    either whitespace or not among the characters found in the body of
  **    a token (deliberately doesn't include the likes of '@'/'&'). */
  #define CONSUMED_NOTHING_OF_VALUE \
    (rdPos == specialCharPos || (rdPos == specialCharPos+1 && ( \
      SPACE || !(ALNUM || DOT || DASH || UNDERSCORE) \
    )))

  #define RIGHTMOST(sb) (sb.getBuffer()[sb.len-1])
  #define RIGHTMOST_IS(sb, c) (RIGHTMOST(sb) == c)
  /* To discard the last character in a StringBuffer, we decrement the buffer's
  ** length indicator and move the terminator back by one character. */
  #define SHAVE_RIGHTMOST(sb) (sb.getBuffer()[--sb.len] = '\0')

  #define REMOVE_TRAILING_CHARS(sb, charMatchesCondition) \
  { \
    char_t* sbBuf = sb.getBuffer(); \
    for (int_t i = sb.len-1; i >= 0; i--) { \
      char_t c = sbBuf[i]; \
      if (charMatchesCondition) { \
        sbBuf[--sb.len] = '\0'; \
      } else { \
        break; \
      } \
    } \
  }

  /* Does StringBuffer sb contain any of the characters in string ofThese? */
  #define CONTAINS_ANY(sb, ofThese) \
    (stringCSpn(sb.getBuffer(), _T(ofThese)) != static_cast<size_t>(sb.len))


  /** Constructs a tokenizer for this Reader. */
  StandardTokenizer::StandardTokenizer(Reader& reader):
    rd(*new FastCharStream(reader)),
    /* rdPos is zero-based.  It starts at -1, and will advance to the first
    ** position when readChar() is first called. */
    rdPos(-1),
    tokenStart(-1)
  {
  }

  StandardTokenizer::~StandardTokenizer() {
    delete &rd;
  }

  void StandardTokenizer::close(){
  }

  uchar_t StandardTokenizer::readChar() {
    uchar_t ch = rd.GetNext();
    /* Increment by 1 because we're speaking in terms of characters, not
    ** necessarily bytes: */
    rdPos++;
    return ch;
  }

  void StandardTokenizer::unReadChar() {
    rd.UnGet();
    rdPos--;
  }

  inline Token* StandardTokenizer::createToken(StringBuffer& sb, TokenTypes tokenCode) {
    return createToken(sb.getBuffer(), sb.len, tokenCode);
  }

  inline Token* StandardTokenizer::createToken(const char_t* text, int_t tokenLength, TokenTypes tokenCode) {
    assert (tokenLength > 0);
    return new Token(text, tokenStart, tokenStart+tokenLength, tokenImage[tokenCode]);
  }

  /** Returns the next token in the stream, or NULL at end-of-stream.
  * The returned token's type is set to an element of
  * StandardTokenizerConstants::tokenImage. */
  Token* StandardTokenizer::next() {
    while (!EOS) {
      uchar_t ch = readChar();

      if (SPACE) {
        continue;
      } else if (ALPHA || UNDERSCORE) {
        tokenStart = rdPos;
        return ReadAlphaNum(ch);
      } else if (DIGIT || NEGATIVE_SIGN_ || DECIMAL) {
        tokenStart = rdPos;
        Token* t = ReadNumber(NULL, ch);
        /* ReadNumber returns NULL if it fails to extract a valid number; in
        ** that case, we just continue. */
        if (t != NULL) {
          return t;
        }
      }
    }
    return NULL;
  }

  Token* StandardTokenizer::ReadNumber(const char_t* previousNumber, const uchar_t prev) {
    /* previousNumber is only non-NULL if this function already read a complete
    ** number in a previous recursion, yet has been asked to read additional
    ** numeric segments.  For example, in the HOST "192.168.1.3", "192.168" is
    ** a complete number, but this function will recurse to read the "1.3",
    ** generating a single HOST token "192.168.1.3". */
    StringBuffer str;
    TokenTypes tokenType;
    bool decExhausted;
    if (previousNumber != NULL) {
      str.prepend(previousNumber);
      tokenType = lucene::analysis::standard::HOST;
      decExhausted = false;
    } else {
      tokenType = lucene::analysis::standard::NUM;
      decExhausted = (prev == '.');
    }
    str.append(prev);
    const bool signExhausted = (prev == '-');
    uchar_t ch = prev;

    CONSUME_DIGITS;

    if (str.len < 2 /* CONSUME_DIGITS didn't find any digits. */
        && (
                (signExhausted && !DECIMAL)
             || (decExhausted /* && !DIGIT is implied, since CONSUME_DIGITS stopped on a non-digit. */)
           )
       )
    {
      /* We have either:
      **   a) a negative sign that's not followed by either digit(s) or a decimal
      **   b) a decimal that's not followed by digit(s)
      ** so this is not a valid number. */
      if (!EOS) {
        /* Unread the character that stopped CONSUME_DIGITS: */
        unReadChar();
      }
      return NULL;
    }

    /* We just read a group of digits.  Is it followed by a decimal symbol,
    ** implying that there might be another group of digits available? */
    if (!EOS) {
      if (DECIMAL) {
        str.append(ch);
      } else {
        unReadChar();
        goto SUCCESSFULLY_EXTRACTED_NUMBER;
      }

      CONSUME_DIGITS;
      if (!DIGIT && !DECIMAL) {
        unReadChar();
      } else if (!EOS && DECIMAL && isDigit(rd.Peek())) {
        /* We just read the fractional digit group, but it's also followed by
        ** a decimal symbol and at least one more digit, so this must be a
        ** HOST rather than a real number. */
        return ReadNumber(str.getBuffer(), '.');
      }
    }

    SUCCESSFULLY_EXTRACTED_NUMBER:
    uchar_t rightmost = RIGHTMOST(str);
    /* Don't including a trailing decimal point. */
    if (rightmost == '.') {
      SHAVE_RIGHTMOST(str);
      unReadChar();
      rightmost = RIGHTMOST(str);
    }
    /* If all we have left is a negative sign, it's not a valid number. */
    if (rightmost == '-') {
      assert (str.len == 1);
      return NULL;
    }

    return createToken(str, tokenType);
  }

  Token* StandardTokenizer::ReadAlphaNum(const uchar_t prev) {
    StringBuffer str;
    str.append(prev);
    uchar_t ch = prev;

    CONSUME_WORD;
    if (!EOS) {
      switch(ch) { /* What follows the first alphanum segment? */
        case '.':
          str.append('.');
          return ReadDotted(str, lucene::analysis::standard::UNKNOWN);
        case '\'':
          str.append('\'');
          return ReadApostrophe(str);
        case '@':
          str.append('@');
          return ReadAt(str);
        case '&':
          str.append('&');
          return ReadCompany(str);
        /* default: fall through to end of this function. */
      }
    }

    return createToken(str, lucene::analysis::standard::ALPHANUM);
  }

  Token* StandardTokenizer::ReadDotted(StringBuffer& str, TokenTypes forcedType) {
    const int_t specialCharPos = rdPos;

    /* A segment of a "dotted" is not allowed to begin with another dot or a dash.
    ** Even though hosts, e-mail addresses, etc., could have a dotted-segment
    ** that begins with a dot or a dash, it's far more common in source text
    ** for a pattern like "abc.--def" to be intended as two tokens. */
    uchar_t ch = rd.Peek();
    if (!(DOT || DASH)) {
      bool prevWasDot;
      bool prevWasDash;
      if (str.len == 0) {
        prevWasDot = false;
        prevWasDash = false;
      } else {
        prevWasDot = RIGHTMOST(str) == '.';
        prevWasDash = RIGHTMOST(str) == '-';
      }
      while (!EOS) {
        ch = readChar();
        const bool dot = ch == '.';
        const bool dash = ch == '-';

        if (!(ALNUM || UNDERSCORE || dot || dash)) {
          break;
        }
        /* Multiple dots or dashes in succession end the token.
        ** Consider the following inputs:
        **   "Visit windowsupdate.microsoft.com--update today!"
        **   "In the U.S.A.--yes, even there!"                 */
        if ((dot || dash) && (prevWasDot || prevWasDash)) {
          /* We're not going to append the character we just read, in any case.
          ** As to the character before it (which is currently RIGHTMOST(str)):
          ** Unless RIGHTMOST(str) is a dot, in which we need to save it so the
          ** acronym-versus-host detection can work, we want to get rid of it. */
          if (!prevWasDot) {
            SHAVE_RIGHTMOST(str);
          }
          break;
        }

        str.append(ch);

        prevWasDot = dot;
        prevWasDash = dash;
      }
    }

    /* There's a potential StringBuffer.append call in the code above, which
    ** could cause str to reallocate its internal buffer.  We must wait to
    ** obtain the optimization-oriented strBuf pointer until after the initial
    ** potentially realloc-triggering operations on str.
    ** Because there can be other such ops much later in this function, strBuf
    ** is guarded within a block to prevent its use during or after the calls
    ** that would potentially invalidate it. */
    { /* Begin block-guard of strBuf */
    char_t* strBuf = str.getBuffer();

    bool rightmostIsDot = RIGHTMOST_IS(str, '.');
    if (CONSUMED_NOTHING_OF_VALUE) {
      /* No more alphanums available for this token; shave trailing dot, if any. */
      if (rightmostIsDot) {
        SHAVE_RIGHTMOST(str);
      }
      /* If there are no dots remaining, this is a generic ALPHANUM. */
      if (stringFindChar(strBuf, '.') == NULL) {
        forcedType = lucene::analysis::standard::ALPHANUM;
      }

    /* Check the token to see if it's an acronym.  An acronym must have a
    ** letter in every even slot and a dot in every odd slot, including the
    ** last slot (for example, "U.S.A."). */
    } else if (rightmostIsDot) {
      bool isAcronym = true;
      const int_t upperCheckLimit = str.len - 1; /* -1 b/c we already checked the last slot. */

      for (int_t i = 0; i < upperCheckLimit; i++) {
        const bool even = (i % 2 == 0);
        ch = strBuf[i];
        if ( (even && !ALPHA) || (!even && !DOT) ) {
          isAcronym = false;
          break;
        }
      }
      if (isAcronym) {
        forcedType = lucene::analysis::standard::ACRONYM;
      } else {
        /* If it's not an acronym, we don't want the trailing dot. */
        SHAVE_RIGHTMOST(str);
        /* If there are no dots remaining, this is a generic ALPHANUM. */
        if (stringFindChar(strBuf, '.') == NULL) {
          forcedType = lucene::analysis::standard::ALPHANUM;
        }
      }
    }
    } /* End block-guard of strBuf */

    if (!EOS) {
      if (ch == '@') {
        str.append('@');
        return ReadAt(str);
      } else {
        unReadChar();
      }
    }

    return createToken(str,
        /* If the token type was never specified, assume it's a HOST: */
        forcedType != lucene::analysis::standard::UNKNOWN
          ? forcedType : lucene::analysis::standard::HOST
      );
  }

  Token* StandardTokenizer::ReadApostrophe(StringBuffer& str) {
    TokenTypes tokenType = lucene::analysis::standard::APOSTROPHE;
    const int_t specialCharPos = rdPos;
    uchar_t ch;

    CONSUME_ALPHAS;
    if (RIGHTMOST_IS(str, '\'') || CONSUMED_NOTHING_OF_VALUE) {
      /* After the apostrophe, no more alphanums were available within this
      ** token; shave trailing apostrophe and revert to generic ALPHANUM. */
      SHAVE_RIGHTMOST(str);
      tokenType = lucene::analysis::standard::ALPHANUM;
    }
    if (!EOS) {
      unReadChar();
    }

    return createToken(str, tokenType);
  }

  Token* StandardTokenizer::ReadAt(StringBuffer& str) {
    Token* t = ReadDotted(str, lucene::analysis::standard::EMAIL);
    /* JLucene grammar indicates dots/digits not allowed in company name: */
    if (!CONTAINS_ANY(str, ".0123456789")) {
      Token *newT = createToken(str, lucene::analysis::standard::COMPANY);
      delete t;
      t = newT;
    }
    return t;
  }

  Token* StandardTokenizer::ReadCompany(lucene::util::StringBuffer& str) {
    const int_t specialCharPos = rdPos;
    uchar_t ch;

    CONSUME_WORD;
    if (CONSUMED_NOTHING_OF_VALUE) {
      /* After the ampersand, no more alphanums were available within this
      ** token; shave trailing ampersand and revert to ALPHANUM. */
      assert(RIGHTMOST_IS(str, '&'));
      SHAVE_RIGHTMOST(str);

      return createToken(str, lucene::analysis::standard::ALPHANUM);
    }
    if (!EOS) {
      unReadChar();
    }

    return createToken(str, lucene::analysis::standard::COMPANY);
  }

}}}
