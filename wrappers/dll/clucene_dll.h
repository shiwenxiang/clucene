//////////////////
/*

The CLucene dll is an easy way of dealing with a clucene index. It is not
object oriented, so can easily be implemented in other languages, however
this is at the sacrifice of some flexibility


Configuration:

Methods Macros:
---------------

Global methods:

Cleanup all resources
CL_CLEANUP (  )

Return the last error from the specified resource
CL_ERRSTR ( resourceID, Char_t* ret, int ret_len )

Return the last non-resource specific error
CL_ERRSTRGLOBAL ( Char_t* ret, int ret_len )

Unlock the specified directory (in case an error occurred and left the index locked)
CL_UNLOCK ( char_t* directory)


Resource methods:
Open the specified directory, create it if it doesn't exist
CL_OPEN (Char_t* dir, bool create)

Close the resource
CL_CLOSE ( resourceID )

Reload the resources reader to reflect changes to the index
CL_RELOAD ( resourceID )

Optimize the index
CL_OPTIMIZE ( resourceID )

Perform a search using the specified query on the specified field.
Delete all documents that is returned from the search
CL_DELETE ( resourceID, Char_t query, Char_t field )


Adding documents:
Start a new document
CL_NEW_DOCUMENT ( resourceID )

Adds the specified value to the current document
Field specifies the name of the field within the document
Store: Store the value in the index. This makes the index bigger in size,
       so would only usually be true for the unique identifier to the document
	   (such as the url, or the database id)
Index: Index the field to make it searchable. (usually true)
Tokenize: Split the value into words using the StandardAnalyzer. If the field
	      is a reference, then this should be false, else usually true
CL_ADD_FIELD ( resourceID, Char_t* field, Char_t* value, int value_length, bool store, bool index, bool tokenize )


Adds the contents of the specified file.
CL_ADD_FILE ( resourceID, Char_t* field, Char_t* filename, bool store, bool index, bool tokenize )

Adds a date to the field.
CL_ADD_DATE ( resourceID, Char_t* field, long time, bool store, bool index, bool tokenize )

//returns an overview of the document about to be added
CL_DOCUMENT_INFO

Insert the current document
CL_INSERT_DOCUMENT ( resourceID )


Searching:

Search within the specified field using the specified query
CL_SEARCH ( resourceID, Char_t* query, Char_t* field, byte* flags )

Search within the specified fields using the specified query
Flags is an array matching each field with a value of:
    byte NORMAL FIELD     = 0;
    byte REQUIRED FIELD   = 1;
    byte PROHIBITED FIELD = 2;
CL_SEARCHMULTIFIELDSFLAGGED 

Search within the specified fields using the specified query
Each field is set as a normal field
CL_SEARCHMULTIFIELDS ( resourceID, Char_t* query, Char_t** fields, int fields_len, byte* flags )

Returns an overview of the query given to the search
CL_SEARCHINFO ( resourceID, Char_t* ret, int ret_len )

Return the hitcount of the last search
CL_HITCOUNT CL_HitCount ( resourceID )

Go to the next hit
CL_NEXTHIT CL_NextHit ( resourceID )

Go to the specified hit
CL_GOTOHIT CL_GotoHit ( resourceID, number )

Return the HitScore of the current search result Document
float CL_HITSCORE CL_HitScore ( resourceID )

Return a the specified field as a date
long CL_GETDATEFIELD ( resourceID, Char_t* field )

Return a the specified field
A Char_t array is created for return, with ret_len length
This should be cleaned up after use
CL_GETFIELD ( resourceID, Char_t* field, Char_t** ret, int* ret_len )

Frees the current search
CL_CLEARSEARCH CL_ClearSearch ( resourceID )


Unicode:
* If _UNICODE is defined, the MACROS listed above will be the unicode equivalents.
* Char_t will be defined as wchar_t. The macros listed above will point to their
  unicode equivalents. The unicode mode will be CL_U_MethodName.
* CL_METHODNAME_OTHER will point to the non-unicode function.

* If _UNICODE is NOT defined, the MACROS listed above will be the non-unicode equivalents.
* Char_t will be defined as char. The macros listed above will point to their
  non-unicode equivalents. The non-unicode method will be CL_N_MethodName.
* If Possible, CL_METHOD_OTHER will point to the unicode function


*/
//////////////////

#ifndef _DLL_CLUCENE
#define _DLL_CLUCENE
#include "CLucene/CLConfig.h"
#include <wchar.h>
#ifdef WIN32
	#define type_long __int64

	#ifdef CLUCENEDLL_EXPORTS
		#define CLUCENEDLL_API __declspec(dllexport)
	#elif CLUCENEDLL_INCLUDED
		#define CLUCENEDLL_API
	#else
		#define CLUCENEDLL_API __declspec(dllimport)
	#endif
#else
	#define CLUCENEDLL_API
	typedef long long type_long;
#endif

//defining NO_NARROW when in unicode mode will disable creation of narrow interface
//defining USE_UNICODE when in narrow mode will create a unicode interface also
#ifdef _UNICODE
# define USE_UNICODE
# ifndef NO_NARROW
#  define USE_NARROW
#  define USE_OTHER
# endif
#else
# define USE_NARROW
# ifdef USE_UNICODE
#  define USE_OTHER
# endif
#endif

//function defines
//these can be used instead of the char specific functions	
#ifdef _UNICODE
//unicode methods
# define CL_UNLOCK CL_U_Unlock
# define CL_OPEN CL_U_Open
# define CL_DOCUMENT_INFO CL_U_Document_Info
# define CL_ADD_FILE CL_U_Add_File
# define CL_ADD_FIELD CL_U_Add_Field
# define CL_ADD_DATE CL_U_Add_Date
# define CL_SEARCHMULTIFIELDSFLAGGED CL_U_SearchMultiFieldsFlagged
# define CL_SEARCHMULTIFIELDS CL_U_SearchMultiFields
# define CL_SEARCH CL_U_Search
# define CL_SEARCHINFO CL_U_SearchInfo
# define CL_GETDATEFIELD CL_U_GetDateField
# define CL_GETFIELD CL_U_GetField
# define CL_DELETE CL_U_Delete
# define CL_ERRSTR CL_U_ErrStr
# define CL_ERRSTRGLOBAL CL_U_ErrStrGlobal
# define CL_HIGHLIGHT CL_U_HighLight
# define CL_HIGHLIGHT_X CL_U_HighLight_X

//other narrow methods
# define CL_UNLOCK_OTHER CL_N_Unlock
# define CL_OPEN_OTHER CL_N_Open
# define CL_DOCUMENT_INFO_OTHER CL_N_Document_Info
# define CL_ADD_FILE_OTHER CL_N_Add_File
# define CL_ADD_FIELD_OTHER CL_N_Add_Field
# define CL_ADD_DATE_OTHER CL_N_Add_Date
# define CL_SEARCHMULTIFIELDSFLAGGED_OTHER CL_N_SearchMultiFieldsFlagged
# define CL_SEARCHMULTIFIELDS_OTHER CL_N_SearchMultiFields
# define CL_SEARCH_OTHER CL_N_Search
# define CL_SEARCHINFO_OTHER CL_N_SearchInfo
# define CL_GETDATEFIELD_OTHER CL_N_GetDateField
# define CL_GETFIELD_OTHER CL_N_GetField
# define CL_DELETE_OTHER CL_N_Delete
# define CL_ERRSTR_OTHER CL_N_ErrStr
# define CL_ERRSTRGLOBAL_OTHER CL_N_ErrStrGlobal
# define CL_HIGHLIGHT_OTHER CL_N_HighLight
# define CL_HIGHLIGHT_X_OTHER CL_N_HighLight_X

#else
//narrow methods
# define CL_UNLOCK CL_N_Unlock
# define CL_OPEN CL_N_Open
# define CL_UNLOCK CL_N_Unlock
# define CL_OPEN CL_N_Open
# define CL_DOCUMENT_INFO CL_N_Document_Info
# define CL_ADD_FILE CL_N_Add_File
# define CL_ADD_FIELD CL_N_Add_Field
# define CL_ADD_DATE CL_N_Add_Date
# define CL_SEARCHMULTIFIELDSFLAGGED CL_N_SearchMultiFieldsFlagged
# define CL_SEARCHMULTIFIELDS CL_N_SearchMultiFields
# define CL_SEARCH CL_N_Search
# define CL_SEARCHINFO CL_N_SearchInfo
# define CL_GETDATEFIELD CL_N_GetDateField
# define CL_GETFIELD CL_N_GetField
# define CL_DELETE CL_N_Delete
# define CL_ERRSTR CL_N_ErrStr
# define CL_ERRSTRGLOBAL CL_N_ErrStrGlobal
# define CL_HIGHLIGHT CL_N_HighLight
# define CL_HIGHLIGHT_X CL_N_HighLight_X

//other Unicode types
# define CL_UNLOCK_OTHER CL_U_Unlock
# define CL_OPEN_OTHER CL_U_Open
# define CL_DOCUMENT_INFO_OTHER CL_U_Document_Info
# define CL_ADD_FILE_OTHER CL_U_Add_File
# define CL_ADD_FIELD_OTHER CL_U_Add_Field
# define CL_ADD_DATE_OTHER CL_U_Add_Date
# define CL_SEARCHMULTIFIELDSFLAGGED_OTHER CL_U_SearchMultiFieldsFlagged
# define CL_SEARCHMULTIFIELDS_OTHER CL_U_SearchMultiFields
# define CL_SEARCH_OTHER CL_U_Search
# define CL_SEARCHINFO_OTHER CL_U_SearchInfo
# define CL_GETDATEFIELD_OTHER CL_U_GetDateField
# define CL_GETFIELD_OTHER CL_U_GetField
# define CL_DELETE_OTHER CL_U_Delete
# define CL_ERRSTR_OTHER CL_U_ErrStr
# define CL_ERRSTRGLOBAL_OTHER CL_U_ErrStrGlobal
# define CL_HIGHLIGHT_OTHER CL_U_HighLight
# define CL_HIGHLIGHT_X_OTHER CL_U_HighLight_X
#endif

	
//just for ease of use, macros in upper case
#define CL_CLOSE CL_Close
#define CL_RELOAD CL_Reload
#define CL_CLEANUP CL_Cleanup
#define CL_OPTIMIZE CL_Optimize
#define CL_NEW_DOCUMENT CL_New_Document
#define CL_INSERT_DOCUMENT CL_Insert_Document
#define CL_HITCOUNT CL_HitCount
#define CL_NEXTHIT CL_NextHit
#define CL_GOTOHIT CL_GotoHit
#define CL_HITSCORE CL_HitScore
#define CL_CLEARSEARCH CL_ClearSearch
//and the other macro, just in case :)
#define CL_CLOSE_OTHER CL_Close
#define CL_RELOAD_OTHER CL_Reload
#define CL_CLEANUP_OTHER CL_Cleanup
#define CL_OPTIMIZE_OTHER CL_Optimize
#define CL_NEW_DOCUMENT_OTHER CL_New_Document
#define CL_INSERT_DOCUMENT_OTHER CL_Insert_Document
#define CL_HITCOUNT_OTHER CL_HitCount
#define CL_NEXTHIT_OTHER CL_NextHit
#define CL_GOTOHIT_OTHER CL_GotoHit
#define CL_HITSCORE_OTHER CL_HitScore
#define CL_CLEARSEARCH_OTHER CL_ClearSearch

#define MAX_CLUCENE_DLL_RESOURCES 10
#ifdef  __cplusplus
extern "C" {
#endif
	#define errlen 500
	
	//CLUCENEDLL_API void CL_CallBack_Error ( char* error );

	//code for both unicode and narrow
#ifndef CLUCENE_LITE
	CLUCENEDLL_API int CL_Optimize(const int resource);
	CLUCENEDLL_API int CL_New_Document(const int resource);
	CLUCENEDLL_API int CL_Insert_Document(const int resource);
#endif
	CLUCENEDLL_API int CL_Close(const int resource);
	CLUCENEDLL_API int CL_Reload(const int resource);
	CLUCENEDLL_API void CL_Cleanup();

	CLUCENEDLL_API int CL_HitCount(const int resource);
	CLUCENEDLL_API int CL_NextHit(const int resource);
	CLUCENEDLL_API int CL_GotoHit(const int resource, const int number);
	CLUCENEDLL_API float CL_HitScore(const int resource);
	CLUCENEDLL_API void CL_ClearSearch(const int resource);

#ifdef USE_UNICODE
	CLUCENEDLL_API int CL_U_Open(const wchar_t* dir, int create);
	CLUCENEDLL_API void CL_U_Document_Info(const int resource, wchar_t* info,  const int len);

	CLUCENEDLL_API int CL_U_Search(const int resource, const wchar_t* query, const wchar_t* field);
	CLUCENEDLL_API int CL_U_SearchMultiFields(const int resource, const wchar_t* qry, const wchar_t** fields, const int fieldsLen);
	CLUCENEDLL_API int CL_U_SearchMultiFieldsFlagged(const int resource, const wchar_t* qry, const wchar_t** fields, const int fieldsLen, const unsigned char * flags);
	CLUCENEDLL_API void CL_U_SearchInfo(const int resource, wchar_t* pl, size_t len);
	CLUCENEDLL_API int CL_U_GetField(const int resource, const wchar_t* field, wchar_t** value, size_t* value_len);
	CLUCENEDLL_API type_long CL_U_GetDateField(const int resource, const wchar_t* field);

	CLUCENEDLL_API void CL_U_ErrStr(const int resource, wchar_t* pl, size_t len);
	CLUCENEDLL_API void CL_U_ErrStrGlobal(wchar_t* pl, size_t len);
#ifdef HIGHLIGHTING
	CLUCENEDLL_API void CL_U_HighLight(const int resource, const wchar_t* text, const int text_is_filename, wchar_t* ret, const size_t ret_len);
	CLUCENEDLL_API void CL_U_HighLight_X(const int resource, const wchar_t* text, const int text_is_filename, wchar_t* ret, const size_t ret_len, const wchar_t* seperator, const int max_fragments, const int fragment_size, const int type, const wchar_t* d1, const wchar_t* d2);
#endif

#ifndef CLUCENE_LITE
	CLUCENEDLL_API int CL_U_Unlock(const wchar_t* dir);
	CLUCENEDLL_API int CL_U_Delete(const int resource, const wchar_t* query, const wchar_t* field);

	CLUCENEDLL_API int CL_U_Add_Field(const int resource, const wchar_t* field, const wchar_t* value, const int value_length, const int store, const int index, const int token);
	CLUCENEDLL_API int CL_U_Add_File(const int resource, const wchar_t* field, const wchar_t* filename, const int store, const int index, const int token);
	CLUCENEDLL_API int CL_U_Add_Date(const int resource, const wchar_t* field, const type_long value, const int store, const int index, const int token);
#endif
#endif

#ifdef USE_NARROW
	//returns resource number
	CLUCENEDLL_API int CL_N_Open(const char* dir, int create);
	CLUCENEDLL_API void CL_N_Document_Info(const int resource, char* info,  const int len);

	CLUCENEDLL_API int CL_N_Search(const int resource, const char* query, const char* field);
	CLUCENEDLL_API int CL_N_SearchMultiFields(const int resource, const char* qry, const char** fields, const int fieldsLen);
	CLUCENEDLL_API int CL_N_SearchMultiFieldsFlagged(const int resource, const char* qry, const char** fields, const int fieldsLen, const unsigned char * flags);
	CLUCENEDLL_API void CL_N_SearchInfo(const int resource, char* pl, size_t len);
	CLUCENEDLL_API int CL_N_GetField(const int resource, const char* field, char** value, size_t* value_len);
	CLUCENEDLL_API type_long CL_N_GetDateField(const int resource, const char* field);

	CLUCENEDLL_API void CL_N_ErrStrGlobal(char* pl, size_t len);
	CLUCENEDLL_API void CL_N_ErrStr(const int resource, char* pl, size_t len);
#ifdef HIGHLIGHTING
	CLUCENEDLL_API void CL_N_HighLight(const int resource, const char* text, const int text_is_filename, char* ret, const size_t ret_len);
	CLUCENEDLL_API void CL_N_HighLight_X(const int resource, const char* text, const int text_is_filename, char* ret, const size_t ret_len, const char* seperator, const int max_fragments, const int fragment_size,  const int type, const char* d1, const char* d2);
#endif

#ifndef CLUCENE_LITE
	CLUCENEDLL_API int CL_N_Unlock(const char* dir);
	CLUCENEDLL_API int CL_N_Delete(const int resource, const char* query, const char* field);

	CLUCENEDLL_API int CL_N_Add_Field(const int resource, const char* field, const char* value, const int value_length, const int store, const int index, const int token);
	CLUCENEDLL_API int CL_N_Add_File(const int resource, const char* field, const char* filename, const int store, const int index, const int token);
	CLUCENEDLL_API int CL_N_Add_Date(const int resource, const char* field, const type_long value, const int store, const int index, const int token);
#endif
#endif

#ifdef  __cplusplus
}
#endif

#endif
