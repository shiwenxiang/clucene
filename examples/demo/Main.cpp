#include "stdafx.h"
//#define TR_LEAKS

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

#include "SearchFiles.cpp"
#include "IndexFiles.cpp"
#include "DeleteFiles.cpp"
#include "Statistics.cpp"
#include <iostream>


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

	cout << "Enter index location: ";
	char_t ndx[250];
	_cin.getline(ndx,250);
	cout << "Files to index location: ";
	char_t files[250];
	_cin.getline(files,250);

	lucene::demo::IndexFiles(files,ndx,true);
	lucene::demo::getStats(ndx);
	lucene::demo::SearchFiles(ndx);
	lucene::demo::DeleteFiles(ndx);

	}catch (THROW_TYPE e){
		cout << "Exception: " << e.what() << endl;
		throw;
	}catch ( char_t* err ){
		_cout << _T("Error: ") << err;
		throw;
	}
	catch ( ... ){
		cout << "UNKNOWN ERROR" << endl;
		throw; 
	}


	//Debuggin techniques:
	//In the Watch window, type the following expression in the Name column: 
	//_crtBreakAlloc

	_cout << endl << endl << _T("Time taken: ") << (lucene::util::Misc::currentTimeMillis() - str) << _T("ms.") << endl << endl;
}
