#include <assert.h>
#include "CLucene/StdHeader.h"

#include "DateField.h"
#include "CLucene/util/Misc.h"
using namespace lucene::util;
using namespace std;
namespace lucene{ namespace document{

  /** Converts a millisecond time to a string suitable for indexing. */
  char_t* DateField::timeToString(const long_t time) {
    if (time < 0)
      _THROWC ("time too early");
    /* DSR:2004.10.28: */
    /* Check for too-late time here rather than after the integerToString call
    ** so as to avoid buffer overflow potential. */
    if (time > DateField::MAX_TIME)
      _THROWC ("time too late (past DateField::MAX_TIME");

    char_t* buf = new char_t[DATE_LEN + 1];
    integerToString(time, buf, CHAR_RADIX);
    int_t bufLen = stringLength(buf);

    assert (bufLen <= DATE_LEN);

    /* Supply leading zeroes if necessary. */
    if (bufLen < DATE_LEN) {
      const int_t nMissingZeroes = DATE_LEN - bufLen;
      /* Move buffer contents forward to make room for leading zeroes. */
      for (int_t i = DATE_LEN - 1; i >= nMissingZeroes; i--)
        buf[i] = buf[i - nMissingZeroes];
      /* Insert leading zeroes. */
      for (int_t i = 0; i < nMissingZeroes; i++)
        buf[i] = '0';

      buf[DATE_LEN] = 0;
    }

    assert (stringLength(buf) == DATE_LEN);

    return buf;
  }

  /** Converts a string-encoded date into a millisecond time. */
  long_t DateField::stringToTime(const char_t* time) {
/* DSR:2004.10.28: I saw no need to duplicate the buffer. */
/*
    char_t* s = stringDuplicate(time);
    long_t ret = 0;

    try {
      char_t* end = s + stringLength(s) - 1;
      ret = stringToIntegerBase(s, &end, CHAR_RADIX);
    } _FINALLY (
      delete[] s;
    );

    return ret;
*/
    char_t* timeNonConst = const_cast<char_t*>(time);
    char_t* end = timeNonConst + stringLength(timeNonConst) - 1;
    return stringToIntegerBase(time, &end, CHAR_RADIX);
  }
}}
