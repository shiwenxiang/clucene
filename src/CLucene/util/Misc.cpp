#include "CLucene/StdHeader.h"
#include "Misc.h"

#ifdef _WIN32
#include <time.h>

#include <stddef.h> //file constants
#include <sys/stat.h>
#include <sys/timeb.h>
#else
#include <sys/time.h>
#include <sys/timeb.h>
#endif

#include "StringBuffer.h"
#include "VoidList.h"

using namespace std;
namespace lucene{ namespace util{

#ifdef _UNICODE
  //static
  char* Misc::wideToChar(const wchar_t* s){
    int len = wcslen(s);
    char* msg=new char[len+1];
    wideToChar( s,msg,len+1 );
    return msg;
  }
  wchar_t* Misc::charToWide(const char* s){
      int len = strlen(s);
    wchar_t* msg = new wchar_t[len+1];
    charToWide(s,msg,len+1);
    return msg;
  }

  void Misc::wideToChar(const wchar_t* s, char* d, size_t len){
      for ( int i=0;i<len&&i<wcslen(s)+1;i++ )
          d[i] = ((unsigned int)s[i]>0x80?'?':(char)s[i]);
  }
  void Misc::charToWide(const char* s, wchar_t* d, size_t len){
      for ( int i=0;i<len&&i<strlen(s)+1;i++ )
      d[i] = (char_t)s[i];
  }
#endif

int_t ranges[8][2] = {
    {65,90},
    {97,122},
    {170,0},
    {181,0},
    {186,0},
    {192,214},
    {216,246},
    {248,255}
};
  //static
  bool Misc::isLetter(int_t c){
#ifdef UTF8
    return lc_utf32_isletter(c);
#else

    for ( int_t i=0;i<8;i++ ){
      if ( ranges[i][1]== 0 && ranges[i][0] == c )
        return true;
      else if ( c >= ranges[i][0] && c <= ranges[i][1] )
        return true;
    }
    return false;
#endif
  }


    //static
  ulong_t Misc::currentTimeMillis() {
    /* DSR:2004.06.11: Use gettimeofday instead of ftime, which is
    ** a) deprecated in general and b) not present at all in BSD's libc (only
    ** in libcompat). */
    #ifndef _WIN32
      struct timeval tstruct;
      if (gettimeofday(&tstruct, NULL) < 0) {
        _THROWC("Error in gettimeofday call.");
      }

      return (((ulong_t) tstruct.tv_sec) * 1000) + tstruct.tv_usec / 1000;
    #else
      struct _timeb tstruct;
      _ftime(&tstruct); /* MS docs say _ftime doesn't return error code */

      return (((ulong_t) tstruct.time) * 1000) + tstruct.millitm;
    #endif
  }

    //static
  const char_t* Misc::replace_all( const char_t* val, const char_t* srch, const char_t* repl )
  {
    int_t cnt = 0;
    int_t repLen = stringLength(repl);
    int_t srchLen = stringLength(srch);
    int_t srcLen = stringLength(val);

    const char_t* pos = val;
    while( (pos = stringFind(pos+1, srch)) != NULL ) {
      ++cnt;
    }

    int_t lenNew = (srcLen - (srchLen * cnt)) + (repLen * cnt);
    char_t* ret = new char_t[lenNew+1];
    ret[lenNew] = 0;
    if ( cnt == 0 ){
      stringCopy(ret,val);
      return ret;
    }

    char_t* cur = ret; //position of return buffer
    const char_t* lst = val; //position of value buffer
    pos = val; //searched position of value buffer
    while( (pos = stringFind(pos+1,srch)) != NULL ) {
      stringNCopy(cur,lst,pos-lst); //copy till current
      cur += (pos-lst);
      lst = pos; //move val position

      stringCopy( cur,repl); //copy replace
      cur += repLen; //move return buffer position
      lst += srchLen; //move last value buffer position
    }
    stringCopy(cur, lst ); //copy rest of buffer

    return ret;
  }

  //static
  bool Misc::dir_Exists(const char_t* path){
    struct Struct_Stat buf;
    int_t ret = Cmd_Stat(path,&buf);
    return ( ret == 0);
  }

  //static
  /* DSR:CL_BUG: (See comment for join method in Misc.h): */
  char_t* Misc::join ( const char_t* a, const char_t* b, const char_t* c, const char_t* d,const char_t* e,const char_t* f ) {
    #define LEN(x) (x == NULL ? 0 : stringLength(x))
    const int_t totalLen =
        LEN(a) + LEN(b) + LEN(c) + LEN(d) + LEN(e) + LEN(f)
      + sizeof(char_t); /* Space for terminator. */

    char_t *buf = new char_t[totalLen];
    stringPrintF(buf,_T("%s%s%s%s%s%s"), a==NULL?_T(""):a, b==NULL?_T(""):b, c==NULL?_T(""):c, d==NULL?_T(""):d, e==NULL?_T(""):e, f==NULL?_T(""):f );
    return buf;
  }

  //static
  bool Misc::priv_isDotDir( const char_t* name )
  {
    if( name[0] == '\0' ) {
      return (false);
    }
    if( name[0] == '.' && name[1] == '\0' ) {
      return (true);
    }
    if( name[1] == '\0' ) {
      return (false);
    }
    if( name[0] == '.' && name[1] == '.' && name[2] == '\0' ) {
      return (true);
    }

    return (false);
    }

}}
