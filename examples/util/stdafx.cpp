// stdafx.cpp : source file that includes just the standard includes
// demo.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

#include <iostream>
void usage(){
	cerr << "Usage: add [-dontrecurse -optimize -store] index directory/file\n";
	cerr << "       delete index \"query string\"(returns deleted count)\n";
	cerr << "       search index [search_field return_fields] \"query string\"\n";
	cerr << "       stats index\n";
	cerr << "       merge index targetindex (only stored fields)\n";
	cerr << "       optimize index\n";
	exit(1);
}
