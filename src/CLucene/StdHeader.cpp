#include "CLucene/StdHeader.h"
#include <sys/stat.h>

#ifdef _UNICODE
wchar_t *lucenewcsdup(const wchar_t *v){
	int len = wcslen(v);
	wchar_t* ret = new wchar_t[len+1];
	wcsncpy(ret,v,len+1);
	return ret;
};
#endif

char * lucenestrdup(const char *v){
	int len = strlen(v);
	char* ret = new char[len+1];
	strncpy(ret,v,len+1);
	return ret;
};


#ifndef _WIN32

void lucenesleep( int_t MilliSeconds){
	time_t Seconds = (time_t) (MilliSeconds /1000);
	long   NanoSec = (long) (MilliSeconds - (Seconds * 1000)) * 1000000;

	timespec WaitTime;

	WaitTime.tv_sec  = Seconds;
	WaitTime.tv_nsec = NanoSec;

	nanosleep(&WaitTime,NULL);
}


long_t fileSize(int_t filehandle)
{
  struct Struct_Stat info;
  if (fstat(filehandle, &info) == -1)
	_THROWC("fstat error in StdHeader/fileSize");
  return info.st_size;
};


/*********************************************************************
 *                  strlwr   (NTDLL.@)
 *
 * convert a string in place to lowercase
 */
LPSTR strlwr( LPSTR str )
{
	LPSTR ret = str;
	for ( ; *str; str++) *str = tolower(*str);
	return ret;
};


/*********************************************************************
 *                  strupr   (NTDLL.@)
 */
LPSTR strupr( LPSTR str )
{
	LPSTR ret = str;
	for ( ; *str; str++) *str = toupper(*str);
	return ret;
};


/*********************************************************************
 *      _i64toa   (NTDLL.@)
 *
 * Converts a large integer to a string.
 *
 * RETURNS
 *  Always returns str.
 *
 * NOTES
 *  Converts value to a '\0' terminated string which is copied to str.
 *  The maximum length of the copied str is 65 bytes. If radix
 *  is 10 and value is negative, the value is converted with sign.
 *  Does not check if radix is in the range of 2 to 36.
 *  If str is NULL it crashes, as the native function does.
 *
 * DIFFERENCES
 * - The native DLL converts negative values (for base 10) wrong:
 *                     -1 is converted to -18446744073709551615
 *                     -2 is converted to -18446744073709551614
 *   -9223372036854775807 is converted to  -9223372036854775809
 *   -9223372036854775808 is converted to  -9223372036854775808
 *   The native msvcrt _i64toa function and our ntdll _i64toa function
 *   do not have this bug.
 */
char * _i64toa(
	LONGLONG value, /* [I] Value to be converted */
	char *str,      /* [O] Destination for the converted value */
	int radix)      /* [I] Number base for conversion */
{
	ULONGLONG val;
	int negative;
	char buffer[65];
	char *pos;
	int digit;

	if (value < 0 && radix == 10) {
	negative = 1;
		val = -value;
	} else {
	negative = 0;
		val = value;
	} /* if */

	pos = &buffer[64];
	*pos = '\0';

	do {
	digit = val % radix;
	val = val / radix;
	if (digit < 10) {
		*--pos = '0' + digit;
	} else {
		*--pos = 'a' + digit - 10;
	} /* if */
	} while (val != 0L);

	if (negative) {
	*--pos = '-';
	} /* if */

	memcpy(str, pos, &buffer[64] - pos + 1);
	return str;
};


/*********************************************************************
 *      _i64tow   (NTDLL.@)
 *
 * Converts a large integer to an unicode string.
 *
 * RETURNS
 *  Always returns str.
 *
 * NOTES
 *  Converts value to a '\0' terminated wstring which is copied to str.
 *  The maximum length of the copied str is 33 bytes. If radix
 *  is 10 and value is negative, the value is converted with sign.
 *  Does not check if radix is in the range of 2 to 36.
 *  If str is NULL it just returns NULL.
 *
 * DIFFERENCES
 * - The native DLL converts negative values (for base 10) wrong:
 *                     -1 is converted to -18446744073709551615
 *                     -2 is converted to -18446744073709551614
 *   -9223372036854775807 is converted to  -9223372036854775809
 *   -9223372036854775808 is converted to  -9223372036854775808
 *   The native msvcrt _i64tow function and our ntdll function do
 *   not have this bug.
 */
LPWSTR _i64tow(
	LONGLONG value, /* [I] Value to be converted */
	LPWSTR str,     /* [O] Destination for the converted value */
	INT radix)      /* [I] Number base for conversion */
{
	ULONGLONG val;
	int negative;
	WCHAR buffer[65];
	PWCHAR pos;
	WCHAR digit;

	if (value < 0 && radix == 10) {
		negative = 1;
		val = -value;
	} else {
		negative = 0;
		val = value;
	} /* if */

	pos = &buffer[64];
	*pos = '\0';

	do {
	digit = val % radix;
	val = val / radix;
	if (digit < 10) {
		*--pos = '0' + digit;
	} else {
		*--pos = 'a' + digit - 10;
	} /* if */
	} while (val != 0L);

	if (negative) {
	*--pos = '-';
	} /* if */

	if (str != NULL) {
	memcpy(str, pos, (&buffer[64] - pos + 1) * sizeof(WCHAR));
	} /* if */
	return str;
};
#endif
