// test.cpp : Defines the entry point for the console application.
//
// if CLUCENEDLL_INCLUDED is defined, then doesn't use DLL
//
#ifdef TR_LEAKS 
#ifdef COMPILER_MSVC
#ifdef _DEBUG
	int _lucene_BlockStop;
	#define CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
#endif
#endif
#endif

#include "stdafx.h"
#include <time.h>

#include <sys/timeb.h>
#include <stdio.h>
#include <sys/timeb.h>
#include <string.h>

#include "../clucene_dll.h"
#  ifdef _UNICODE
# ifndef _T
#	define _T(x)       L ## x
# endif
#define char_t wchar_t
#define PRINT wprintf
#else
# ifndef _T
#	define _T(x)       x
# endif
#define char_t char
#define PRINT printf
# endif

#ifdef WIN32
#define long_t unsigned __int64
#else
#define long_t long long
#endif

int run_test(){
	char_t err[errlen];

	//add a document
#ifndef CLUCENE_LITE
	CL_UNLOCK(_T("index"));
#endif
	int resource = CL_OPEN(_T("index"),1);
	if ( !resource ){
		CL_ERRSTRGLOBAL(err,errlen);
		PRINT(_T("Open failed - %s\n"), err);
		return 0;
	}


	/*PRINT (_T("%d items deleted"),CL_DELETE(resource,_T("beautiful"),_T("contents") ) );
	CL_ERRSTR(resource,err,errlen);
	PRINT(_T(" - %s\n"),err);
	CL_OPTIMIZE(resource);
	CL_CLOSE(resource);
	return 0;*/

#ifndef CLUCENE_LITE
	//for ( int i=0;i<2;i++){
		CL_NEW_DOCUMENT(resource);
		CL_ADD_FIELD(resource,_T("contents"),_T("Some text value"),16,0,1,1); //not stored

        struct timeb tstruct;
		ftime( &tstruct );
		CL_ADD_DATE(resource,_T("date"), tstruct.time,1,0,0); //stored only

		CL_ADD_FIELD(resource,_T("note"),_T("This is a stored note containing some text value - ülaut."),58,1,1,1); //stored and indexed
		CL_ADD_FILE(resource,_T("file"),_T("words.txt"),0,1,1); //indexed file

		char_t docinfo[1024];
		CL_DOCUMENT_INFO(resource,docinfo,1024);
		PRINT(_T("Adding value: %s\n"),docinfo);
		//printf("Adding %s\n", _ctime64( &ltime ));

		if ( CL_INSERT_DOCUMENT(resource) )
			printf ("Document inserted...\n");
		else{
			CL_ERRSTR(resource,err,errlen);
			PRINT (_T("Document insert failed - %s\n"), err);
		}
		printf("\n");
	//}

	if ( CL_OPTIMIZE(resource) == 0 ){
		CL_ERRSTR(resource,err,errlen);
		PRINT (_T("Failed optimizing index - %s\n"), err);
	}

	CL_CLOSE(resource);
	resource = CL_OPEN(_T("index"),1);
#endif

	//do a search
	const char_t* fields[2];
	fields[0] = _T("contents");
	fields[1] = _T("note");
	if ( CL_SEARCHMULTIFIELDS(resource,_T("some text"),fields,2) ){
		if ( CL_HITCOUNT(resource) > 0 ){
			char_t qryText[2048];
			CL_SEARCHINFO(resource,qryText,2048);
			PRINT (_T("Found %d documents while searching for: %s\n"),CL_HITCOUNT(resource),qryText);

			int i=1;
			do{
				const type_long dt = CL_GETDATEFIELD(resource,_T("date"));

				char_t* note = NULL;
				size_t ln = 0;
				CL_GETFIELD(resource,_T("note"),&note,&ln);

				float score = CL_HITSCORE(resource);

				PRINT(_T("%d. note: %s, date: %d, score: "), i, note, dt);
				printf("%1.5g\n",score);

#ifdef HIGHLIGHTING
				char_t* hl = new char_t[100];
				CL_HIGHLIGHT(resource,note,false,hl,100);
				PRINT (_T("highlight: '%s'\n"),hl);
				delete[] hl;
#endif

				delete[] note;

				i++;
			}while(CL_NEXTHIT(resource) && i<10);
		}else
			printf("No documents found\n");

	}else{
		CL_ERRSTR(resource,err,errlen);
		PRINT (_T("Search failed - %s\n"),err);
	}
	CL_CLOSE(resource);

	CL_CLEANUP();
	return 0;
}
long_t currentTimeMillis(){
	struct timeb tstruct;
	ftime( &tstruct );

	long_t tm = tstruct.time;
	return (tm * 1000) + tstruct.millitm;
}
int main(int argc, char** argv)
{
	//Dumper Debug
	#ifdef TR_LEAKS 
	#ifdef COMPILER_MSVC
	#ifdef _DEBUG
		_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );//| _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_CHECK_CRT_DF );
	#endif
	#endif
	#endif
	long_t str = currentTimeMillis();
	int ret = run_test();	

	//Debuggin techniques:
	//In the Watch window, type the following expression in the Name column: 
	//_crtBreakAlloc
	//_cout << endl << endl << _T("Time taken: ") << (lucene::util::Misc::currentTimeMillis() - str) << _T("ms.") << endl << endl;

}

