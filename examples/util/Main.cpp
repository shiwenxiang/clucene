#include "stdafx.h"

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

#if defined(MSVC6)
#pragma comment(lib, "clucene.lib")
#endif

#include <iostream>
#include "CLucene.h"

int cmd_add( int_t argc, char** argv );
int cmd_delete( int_t argc, char** argv );
int cmd_search( int_t argc, char** argv );
int cmd_stats( int_t argc, char** argv );
int cmd_merge( int_t argc, char** argv );
int cmd_optimize( int_t argc, char** argv );

using namespace std;
int main( int_t argc, char** argv ){
	//Dumper Debug
	#ifdef TR_LEAKS 
	#ifdef COMPILER_MSVC
	#ifdef _DEBUG
		_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );//| _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_CHECK_CRT_DF );
	#endif
	#endif
	#endif

	long_t str = lucene::util::Misc::currentTimeMillis();
	try{
		argc--;argv++; //move past exe name
		if ( argc < 2 ) 
			usage();
		else  if ( strcmp( argv[0], "add" )== 0 ){
			return cmd_add(--argc,++argv);
		}else if (strcmp(argv[0], "delete" )== 0 ){
			return cmd_delete(--argc,++argv);
		}else if (strcmp(argv[0], "search" )== 0 ){
			return cmd_search(--argc,++argv);
		}else if (strcmp(argv[0], "stats" )== 0 ){
			return cmd_stats(--argc,++argv);
		}else if (strcmp(argv[0], "merge" )== 0 ){
			return cmd_merge(--argc,++argv);
		}else if (strcmp(argv[0], "optimize" )== 0 ){
			return cmd_optimize(--argc,++argv);
		}else{
			usage();
		}

	}catch (THROW_TYPE e){
		cerr << "Exception: " << e.what() << endl;
		throw;
	}catch ( char_t* err ){
		_cout << _T("Error: ") << err;
		throw;
	}
	catch ( ... ){
		cerr << "UNKNOWN ERROR" << endl;
		throw; 
	}

	return 1;

	//Debuggin techniques:
	//In the Watch window, type the following expression in the Name column: 
	//_crtBreakAlloc
	//_cout << endl << endl << _T("Time taken: ") << (lucene::util::Misc::currentTimeMillis() - str) << _T("ms.") << endl << endl;
}
