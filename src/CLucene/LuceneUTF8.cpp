#ifdef UTF8
#include "CLucene/StdHeader.h"
#include "CLucene/util/StringBuffer.h"
#include "gunichartables.h"

// This code is derived from glib 2.4 which is also covered by the LGPL

typedef uchar_t        gunichar;
typedef unsigned short guint16;
typedef          short gint16;
typedef          char  gchar;
typedef unsigned char  guchar;

/* These are the possible character classifications.
 * See http://www.unicode.org/Public/UNIDATA/UnicodeData.html
 */
typedef enum
{
  G_UNICODE_CONTROL,
  G_UNICODE_FORMAT,
  G_UNICODE_UNASSIGNED,
  G_UNICODE_PRIVATE_USE,
  G_UNICODE_SURROGATE,
  G_UNICODE_LOWERCASE_LETTER,
  G_UNICODE_MODIFIER_LETTER,
  G_UNICODE_OTHER_LETTER,
  G_UNICODE_TITLECASE_LETTER,
  G_UNICODE_UPPERCASE_LETTER,
  G_UNICODE_COMBINING_MARK,
  G_UNICODE_ENCLOSING_MARK,
  G_UNICODE_NON_SPACING_MARK,
  G_UNICODE_DECIMAL_NUMBER,
  G_UNICODE_LETTER_NUMBER,
  G_UNICODE_OTHER_NUMBER,
  G_UNICODE_CONNECT_PUNCTUATION,
  G_UNICODE_DASH_PUNCTUATION,
  G_UNICODE_CLOSE_PUNCTUATION,
  G_UNICODE_FINAL_PUNCTUATION,
  G_UNICODE_INITIAL_PUNCTUATION,
  G_UNICODE_OTHER_PUNCTUATION,
  G_UNICODE_OPEN_PUNCTUATION,
  G_UNICODE_CURRENCY_SYMBOL,
  G_UNICODE_MODIFIER_SYMBOL,
  G_UNICODE_MATH_SYMBOL,
  G_UNICODE_OTHER_SYMBOL,
  G_UNICODE_LINE_SEPARATOR,
  G_UNICODE_PARAGRAPH_SEPARATOR,
  G_UNICODE_SPACE_SEPARATOR
} GUnicodeType;


#define UTF8_COMPUTE(Char, Mask, Len)					      \
  if (Char < 128)							      \
    {									      \
      Len = 1;								      \
      Mask = 0x7f;							      \
    }									      \
  else if ((Char & 0xe0) == 0xc0)					      \
    {									      \
      Len = 2;								      \
      Mask = 0x1f;							      \
    }									      \
  else if ((Char & 0xf0) == 0xe0)					      \
    {									      \
      Len = 3;								      \
      Mask = 0x0f;							      \
    }									      \
  else if ((Char & 0xf8) == 0xf0)					      \
    {									      \
      Len = 4;								      \
      Mask = 0x07;							      \
    }									      \
  else if ((Char & 0xfc) == 0xf8)					      \
    {									      \
      Len = 5;								      \
      Mask = 0x03;							      \
    }									      \
  else if ((Char & 0xfe) == 0xfc)					      \
    {									      \
      Len = 6;								      \
      Mask = 0x01;							      \
    }									      \
  else									      \
    Len = -1;

#define UTF8_LENGTH(Char)              \
  ((Char) < 0x80 ? 1 :                 \
   ((Char) < 0x800 ? 2 :               \
    ((Char) < 0x10000 ? 3 :            \
     ((Char) < 0x200000 ? 4 :          \
      ((Char) < 0x4000000 ? 5 : 6)))))


#define UTF8_GET(Result, Chars, Count, Mask, Len)			      \
  (Result) = (Chars)[0] & (Mask);					      \
  for ((Count) = 1; (Count) < (Len); ++(Count))				      \
    {									      \
      if (((Chars)[(Count)] & 0xc0) != 0x80)				      \
  {								      \
    (Result) = -1;						      \
    break;							      \
  }								      \
      (Result) <<= 6;							      \
      (Result) |= ((Chars)[(Count)] & 0x3f);				      \
    }

static const gchar utf8_skip_data[256] = {
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,1,1
};

const gchar * const lc_utf8_skip = utf8_skip_data;

//#define g_utf8_next_char(p) (char *)((p) + g_utf8_skip[*(guchar *)(p)])

/**
 * g_utf8_get_char:
 * @p: a pointer to Unicode character encoded as UTF-8
 *
 * Converts a sequence of bytes encoded as UTF-8 to a Unicode character.
 * If @p does not point to a valid UTF-8 encoded character, results are
 * undefined. If you are not sure that the bytes are complete
 * valid Unicode characters, you should use g_utf8_get_char_validated()
 * instead.
 *
 * Return value: the resulting character
 **/
uchar_t lc_utf8_get_char (const char *p, int *len)
{
  int i, mask = 0;
  gunichar result;
  unsigned char c = (unsigned char) *p;

  UTF8_COMPUTE (c, mask, *len);
  if (*len == -1)
    return (gunichar)-1;
  UTF8_GET (result, p, i, mask, *len);

  return result;
}


#define ATTR_TABLE(Page) (((Page) <= G_UNICODE_LAST_PAGE_PART1) \
                          ? attr_table_part1[Page] \
                          : attr_table_part2[(Page) - 0xe00])

#define ATTTABLE(Page, Char) \
  ((ATTR_TABLE(Page) == G_UNICODE_MAX_TABLE_INDEX) ? 0 : (attr_data[ATTR_TABLE(Page)][Char]))


#define TTYPE_PART1(Page, Char) \
  ((type_table_part1[Page] >= G_UNICODE_MAX_TABLE_INDEX) \
   ? (type_table_part1[Page] - G_UNICODE_MAX_TABLE_INDEX) \
   : (type_data[type_table_part1[Page]][Char]))

#define TTYPE_PART2(Page, Char) \
  ((type_table_part2[Page] >= G_UNICODE_MAX_TABLE_INDEX) \
   ? (type_table_part2[Page] - G_UNICODE_MAX_TABLE_INDEX) \
   : (type_data[type_table_part2[Page]][Char]))

#define TYPE(Char) \
  (((Char) <= G_UNICODE_LAST_CHAR_PART1) \
   ? TTYPE_PART1 ((Char) >> 8, (Char) & 0xff) \
   : (((Char) >= 0xe0000 && (Char) <= G_UNICODE_LAST_CHAR) \
      ? TTYPE_PART2 (((Char) - 0xe0000) >> 8, (Char) & 0xff) \
      : G_UNICODE_UNASSIGNED))

/* Count the number of elements in an array. The array must be defined
 * as such; using this with a dynamically allocated array will give
 * incorrect results.
 */
#define G_N_ELEMENTS(arr)		(sizeof (arr) / sizeof ((arr)[0]))

bool lc_utf32_isletter(uchar_t c)
{
    int t = TYPE (c);
    switch(t)
    {
      case G_UNICODE_LOWERCASE_LETTER: return true;
      case G_UNICODE_TITLECASE_LETTER: return true;
      case G_UNICODE_UPPERCASE_LETTER: return true;
      case G_UNICODE_MODIFIER_LETTER: return true;
      case G_UNICODE_OTHER_LETTER: return true;
      default: return false;
    }
}

bool lc_utf32_isalnum(uchar_t c)
{
    int t = TYPE (c);
    switch(t)
    {
      case G_UNICODE_LOWERCASE_LETTER: return true;
      case G_UNICODE_TITLECASE_LETTER: return true;
      case G_UNICODE_UPPERCASE_LETTER: return true;
      case G_UNICODE_MODIFIER_LETTER: return true;
      case G_UNICODE_OTHER_LETTER: return true;
      case G_UNICODE_DECIMAL_NUMBER: return true;
      case G_UNICODE_LETTER_NUMBER: return true;
      case G_UNICODE_OTHER_NUMBER: return true;
      default: return false;
    }
}

bool lc_utf32_isdigit(uchar_t c)
{
    int t = TYPE (c);
    switch(t)
    {
      case G_UNICODE_DECIMAL_NUMBER: return true;
      case G_UNICODE_LETTER_NUMBER: return true;
      case G_UNICODE_OTHER_NUMBER: return true;
      default: return false;
    }
}

/**
 * g_unichar_isspace:
 * @c: a Unicode character
 *
 * Determines whether a character is a space, tab, or line separator
 * (newline, carriage return, etc.).  Given some UTF-8 text, obtain a
 * character value with g_utf8_get_char().
 *
 * (Note: don't use this to do word breaking; you have to use
 * Pango or equivalent to get word breaking right, the algorithm
 * is fairly complex.)
 *
 * Return value: %TRUE if @c is a punctuation character
 **/
bool lc_utf32_isspace (uchar_t c)
{
  switch (c)
    {
      /* special-case these since Unicode thinks they are not spaces */
    case '\t':
    case '\n':
    case '\r':
    case '\f':
      return true;
      break;

    default:
      {
        int t = TYPE (c);
        return (t == G_UNICODE_SPACE_SEPARATOR || t == G_UNICODE_LINE_SEPARATOR
                || t == G_UNICODE_PARAGRAPH_SEPARATOR);
      }
      break;
    }
}



/**
 * g_unichar_tolower:
 * @c: a Unicode character.
 *
 * Converts a character to lower case.
 *
 * Return value: the result of converting @c to lower case.
 *               If @c is not an upperlower or titlecase character,
 *               or has no lowercase equivalent @c is returned unchanged.
 **/
uchar_t lc_unichar_tolower (uchar_t c)
{
  int t = TYPE (c);
  if (t == G_UNICODE_UPPERCASE_LETTER)
    {
      gunichar val = ATTTABLE (c >> 8, c & 0xff);
      if (val >= 0x1000000)
  {
    const gchar *p = special_case_table + val - 0x1000000;
    int len=0;
    return lc_utf8_get_char (p, &len);
  }
      else
  return val ? val : c;
    }
  else if (t == G_UNICODE_TITLECASE_LETTER)
    {
      unsigned int i;
      for (i = 0; i < G_N_ELEMENTS (title_table); ++i)
  {
    if (title_table[i][0] == c)
      return title_table[i][2];
  }
    }
  return c;
}



/**
 * lc_unichar_to_utf8:
 * @c: a ISO10646 character code
 * @outbuf: output buffer, must have at least 6 bytes of space.
 *       If %NULL, the length will be computed and returned
 *       and nothing will be written to @outbuf.
 *
 * Converts a single character to UTF-8.
 *
 * Return value: number of bytes written
 **/
int lc_unichar_to_utf8 (gunichar c,
       gchar   *outbuf)
{
  guchar len = 0;
  int first;
  int i;

  if (c < 0x80)
    {
      first = 0;
      len = 1;
    }
  else if (c < 0x800)
    {
      first = 0xc0;
      len = 2;
    }
  else if (c < 0x10000)
    {
      first = 0xe0;
      len = 3;
    }
   else if (c < 0x200000)
    {
      first = 0xf0;
      len = 4;
    }
  else if (c < 0x4000000)
    {
      first = 0xf8;
      len = 5;
    }
  else
    {
      first = 0xfc;
      len = 6;
    }

  if (outbuf)
    {
      for (i = len - 1; i > 0; --i)
  {
    outbuf[i] = (c & 0x3f) | 0x80;
    c >>= 6;
  }
      outbuf[0] = c | first;
    }

  return len;
}

LPSTR lc_utf8_strcasefold( const char * str )
{
/**
 * g_utf8_casefold:
 * @str: a UTF-8 encoded string
 * @len: length of @str, in bytes, or -1 if @str is nul-terminated.
 *
 * Converts a string into a form that is independent of case. The
 * result will not correspond to any particular case, but can be
 * compared for equality or ordered with the results of calling
 * g_utf8_casefold() on other strings.
 *
 * Note that calling g_utf8_casefold() followed by g_utf8_collate() is
 * only an approximation to the correct linguistic case insensitive
 * ordering, though it is a fairly good one. Getting this exactly
 * right would require a more sophisticated collation function that
 * takes case sensitivity into account. GLib does not currently
 * provide such a function.
 *
 * Return value: a newly allocated string, that is a
 *   case independent form of @str.
 **/
//gchar *
//g_utf8_casefold (const gchar *str,
//		 gssize       len)
//{
  //!!!GString *result = g_string_new (NULL);
  const char *p;
  //lucene::util::StringBuffer result;
  //gchar buf[7];
  int charlen;
  int stringlen = 0;
  int buflen = strlen(str)+64;
  char *result = new char[buflen];

  //g_return_val_if_fail (str != NULL, NULL);

  p = str;
  //while ((len < 0 || p < str + len) && *p) Assume string is NULL terminated
  while (*p)
    {
      gunichar ch = lc_utf8_get_char (p, &charlen);

      int start = 0;
      int end = G_N_ELEMENTS (casefold_table);

      if (ch >= casefold_table[start].ch &&
          ch <= casefold_table[end - 1].ch)
  {
    while (1)
      {
        int half = (start + end) / 2;
        if (ch == casefold_table[half].ch)
    {
      //!!!g_string_append (result, casefold_table[half].data);
      {
      int_t sl = stringLength(casefold_table[half].data);
      int newlen = stringlen+sl+1;
      if ( newlen > buflen )
      {
        buflen += LUCENE_STREAM_BUFFER_SIZE;
        if ( buflen<newlen )
          buflen = newlen;

        char_t* tmp = new char_t[buflen];
        stringNCopy(tmp,result,stringlen+1);
        delete[] result;
        result = tmp;
        //growBuffer ( *len+sl+1 );
      }
      for ( int_t i=0;i<sl;i++ ){
        result[stringlen+i] = casefold_table[half].data[i];
      }
      stringlen += sl;
      }
      goto next;
    }
        else if (half == start)
    break;
        else if (ch > casefold_table[half].ch)
    start = half;
        else
    end = half;
      }
  }

      //!!!g_string_append_unichar (result, g_unichar_tolower (ch));
    {
    int newlen = stringlen + 7;
    if ( newlen > buflen )
    {
      buflen += LUCENE_STREAM_BUFFER_SIZE;
      if ( buflen<newlen )
        buflen = newlen;

      char_t* tmp = new char_t[buflen];
      stringNCopy(tmp,result,stringlen+1);
      delete[] result;
      result = tmp;
      //growBuffer(*len + charlen);
    }
    int charlen = lc_unichar_to_utf8 (lc_unichar_tolower(ch), result+stringlen);
    stringlen+=charlen;
      }
    next:
      p = g_utf8_next_char (p);
    }
  result[stringlen] = 0;
  return result;
}

int lc_utf8_stricmp( const char * str1, const char * str2 )
{
  char *cmp1 = g_utf8_strcasefold(str1);
  char *cmp2 = g_utf8_strcasefold(str2);
  int retval = strcmp(cmp1, cmp2);
  delete [] cmp1;
  delete [] cmp2;
  return retval;
}

/**
 * g_ucs4_to_utf8:
 * @str: a UCS-4 encoded string
 * @len: the maximum length of @str to use. If @len < 0, then
 *       the string is terminated with a 0 character.
 * @items_read: location to store number of characters read read, or %NULL.
 * @items_written: location to store number of bytes written or %NULL.
 *                 The value here stored does not include the trailing 0
 *                 byte.
 * @error: location to store the error occuring, or %NULL to ignore
 *         errors. Any of the errors in #GConvertError other than
 *         %G_CONVERT_ERROR_NO_CONVERSION may occur.
 *
 * Convert a string from a 32-bit fixed width representation as UCS-4.
 * to UTF-8. The result will be terminated with a 0 byte.
 *
 * Return value: a pointer to a newly allocated UTF-8 string.
 *               This value must be freed with g_free(). If an
 *               error occurs, %NULL will be returned and
 *               @error set.
 *
gchar *g_ucs4_to_utf8 (const gunichar *str,
    glong           len,
    glong          *items_read,
    glong          *items_written,
    GError        **error)
*/
char_t *lc_ucs4_to_utf8 (const uchar_t *str, int len)
{
  int result_length;
  char *result = NULL;
  char *p;
  int i;

  result_length = 0;
  for (i = 0; len < 0 || i < len ; i++)
    {
      if (!str[i])
  break;

      if (str[i] >= 0x80000000)
  {
    goto err_out;
  }

      result_length += UTF8_LENGTH (str[i]);
    }

  result = new char[result_length + 1];
  p = result;

  i = 0;
  while (p < result + result_length)
    p += lc_unichar_to_utf8 (str[i++], p);

  *p = '\0';

 err_out:
  return result;
}


#endif

