#include "CLucene/StdHeader.h"
#include "StandardFilter.h"

#include "../AnalysisHeader.h"
#include "../Analyzers.h"
#include "StandardTokenizerConstants.h"
#include "CLucene/util/StringBuffer.h"

using namespace std;
using namespace lucene::analysis;
namespace lucene{ namespace analysis { namespace standard {

  StandardFilter::StandardFilter(TokenStream* in, bool deleteTokenStream):
    TokenFilter(in, deleteTokenStream)
  {
  }

  StandardFilter::~StandardFilter(){
  }

  /** Returns the next token in the stream, or NULL at EOS.
  * <p>Removes <tt>'s</tt> from the end of words.
  * <p>Removes dots from acronyms.
  */
  Token* StandardFilter::next() {
    Token* t = input->next();

    if (t == NULL)
      return NULL;

    const char_t* text = t->TermText();
    const int_t textLength = stringLength(text);
    const char_t* type = t->Type();

    if (     stringCompare(type, tokenImage[APOSTROPHE])==0
        && ( textLength >= 2 && stringICompare(text+textLength-2, _T("'s"))==0  )
       )
    {
      // remove 's
      char_t* buf = stringDuplicate(text);
      buf[textLength-2]=0;
      Token* ret = new Token(buf, t->StartOffset(), t->EndOffset(), type);
      delete[] buf;
      /* DSR:CL_BUG_LEAK: t was not deleted in this case, so it leaked. */
      delete t;
      return ret;

    } else if ( stringCompare(type, tokenImage[ACRONYM])==0 ) {		  // remove dots
      StringBuffer trimmed;
      #ifdef UTF8
      const char_t *p = text;
      int len = 0;
      while (*p) {
        uchar_t c = lc_utf8_get_char (p, &len);
        if (c != '.')
          trimmed.append((char_t)c);
        p = lc_utf8_next_char (p);
      }
      #else
      for (int_t i = 0; i < textLength; i++) {
        char_t c = text[i];
        if (c != '.')
        trimmed.append(c);
      }
      #endif
      Token *ret = new Token(trimmed.getBuffer(), t->StartOffset(), t->EndOffset(), type);
      /* DSR:CL_BUG_LEAK: t was not deleted in this case, so it leaked. */
      delete t;
      return ret;

    } else {
      return t;
    }

  }
}}}
