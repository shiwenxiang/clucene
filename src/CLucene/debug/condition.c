#include "condition.h"
#ifdef _CND_DEBUG

#define __CND_STR_PRECONDITION    "PRECONDITION"
#define __CND_STR_CONDITION       "CONDITION"
#define __CND_STR_WARNING         "WARNING"
#define __CND_STR_MESSAGE         "MESSAGE"
#define __CND_STR_DEBUGMESSAGE    "DEBUG MESSAGE"
#define __CND_STR_EXIT            "EXIT"

#ifdef _WIN32
  /* Windows version */

  #ifndef __WINDOWS_H
  #include <windows.h>
  #endif

  void OutDebugWindows( const char *File, int Line, int Title, const char *Mes2, int StopIcon ) {
    char M[512];
    char *StrTitle = NULL;
    if( Mes2 )
      wsprintf((LPSTR)M,(LPSTR)"file:%s line:%d\n%s",(LPSTR)File,Line,(LPSTR)Mes2);
    else
      wsprintf((LPSTR)M,(LPSTR)"file:%s line:%d",(LPSTR)File,Line);
        /*Determine which title to use*/
        switch( Title ) {
            case CND_STR_PRECONDITION: {
              StrTitle = __CND_STR_PRECONDITION;
              break;
              }
            case CND_STR_CONDITION: {
              StrTitle = __CND_STR_CONDITION;
              break;
              }
            case CND_STR_WARNING: {
              StrTitle = __CND_STR_WARNING;
              break;
              }
            case CND_STR_MESSAGE: {
              StrTitle = __CND_STR_MESSAGE;
              break;
              }
            case CND_STR_DEBUGMESSAGE: {
              StrTitle = __CND_STR_DEBUGMESSAGE;
              break;
              }
            case CND_STR_EXIT: {
              StrTitle = __CND_STR_EXIT;
              break;
              }
            default:
              break;
          }/*switch*/

        /*Display a standard messagebox*/
        MessageBox(NULL, (LPSTR)M, (LPSTR)StrTitle, (StopIcon==1 ? MB_ICONSTOP:MB_ICONEXCLAMATION) | MB_OK | MB_TASKMODAL);

        #if defined(_CND_DEBUG_WARN_DEBUGGER) /*attempt to signal windows debugger*/
        OutputDebugString((LPCSTR)M);
        DebugBreak(); /*Position debugger just before exit program*/

        #endif
  }
#else
  /*Unix version*/
  #ifdef __GNUC__
  void OutUnix( const char* File, int Line, int Title, const char *Mes2 ) {
    /*Determine which title to use */
    char *StrTitle = NULL;

    switch( Title ) {
            case CND_STR_PRECONDITION: {
              StrTitle = __CND_STR_PRECONDITION;
              break;
              }
            case CND_STR_CONDITION: {
              StrTitle = __CND_STR_CONDITION;
              break;
              }
            case CND_STR_WARNING: {
              StrTitle = __CND_STR_WARNING;
              break;
              }
            case CND_STR_MESSAGE: {
              StrTitle = __CND_STR_MESSAGE;
              break;
              }
            case CND_STR_DEBUGMESSAGE: {
              StrTitle = __CND_STR_DEBUGMESSAGE;
              break;
              }
            case CND_STR_EXIT: {
              StrTitle = __CND_STR_EXIT;
              break;
              }
            default:
              break;
          }/*switch*/

    if( StrTitle )
      printf("%s file:%s line:%d",StrTitle,File,Line);
    else
      printf("file:%s line:%d",File,Line);
    if( Mes2 )
      printf("\n\t%s",Mes2);
    printf("\n");
  }
  #endif
#endif

#endif /* _CND_DEBUG */
