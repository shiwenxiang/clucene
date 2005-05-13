
// clucene includes
#include "CLucene.h"
#include "CLucene/util/Reader.h"
#include "CLucene/util/Misc.h"
#include "CLucene/queryParser/MultiFieldQueryParser.h"

#ifdef HIGHLIGHTING
# include "..\contributions\highlighter\QueryTermExtractor.h"
# include "..\contributions\highlighter\QueryScorer.h"
# include "..\contributions\highlighter\Highlighter.h"
# include "..\contributions\highlighter\SimpleHTMLFormatter.h"

#define HIGHLIGHT_HTML_START _T("<b>")
#define HIGHLIGHT_HTML_END _T("</b>")
#define HIGHLIGHT_FRAGMENT_SIZE 100
#define HIGHLIGHT_MAX_NUM_FRAGMENTS 3
#define HIGHLIGHT_SEPARATOR _T("...")

using namespace lucene::search::highlight;
#endif

#include <iostream>

#include "clucene_dll.h"

using namespace std;

using namespace lucene::index;
using namespace lucene::analysis;
using namespace lucene::analysis::standard;
using namespace lucene::util;
using namespace lucene::store;
using namespace lucene::document;
using namespace lucene::search;
using namespace lucene::queryParser;



#ifdef _UNICODE
//# ifdef USE_NARROW
#  define o_char_t char
#  define _CONV_AWAY(x) Misc::wideToChar(x)
#  define _CONV_BACK(x) Misc::charToWide(x)
#  define _COPY_AWAY(src,dest,len) Misc::wideToChar(src,dest,len)
#  define _COPY_BACK(src,dest,len) Misc::charToWide(src,dest,len)
//# else
//# endif
#else
//# ifdef USE_WIDE
#  define o_char_t wchar_t
#  define _CONV_AWAY(x) Misc::charToWide(x)
#  define _CONV_BACK(x) Misc::wideToChar(x)
#  define _COPY_AWAY(src,dest,len) Misc::charToWide(src,dest,len)
#  define _COPY_BACK(src,dest,len) Misc::wideToChar(src,dest,len)
//# else
//# endif
#endif


//private================================
struct Resource{
public:
    //searcher
    Document* doc;
    IndexSearcher* searcher;
    Query* query;
    Hits* hits;
    int hitPos;
    int count;
    char_t directory[CL_MAX_DIR];
    
    //general
    char_t errstr[errlen];
    //Directory* dir;
	IndexReader* reader;
    
    Resource(const char_t* dir){
        doc = NULL;
        searcher = NULL;
        query = NULL;
        hits = NULL;
        hitPos = 0;
        errstr[0]=0;
	    reader = NULL;
	    count=1;
		stringNCopy(directory,dir,CL_MAX_DIR);
    }
    ~Resource(){
    	delete doc;
    	doc = NULL;

		delete reader;
		reader = NULL;
    }
};


extern "C" {
	//
	Resource* resources[MAX_CLUCENE_DLL_RESOURCES];
	int resourcesInitialised = 0;
	bool resource_mutex = false;
	char_t globalerrstr[errlen];
	StandardAnalyzer analyzer;
	int isValidResource(const int resource, const int reportFail = 1);
}
#ifdef USE_UNICODE
void reportGlobalError(const wchar_t* error){
	unsigned int i;
	if ( error == NULL )
		return;

	for ( i=0;i<errlen-1&&i<wcslen(error);i++ )
		globalerrstr[i]=error[i];
	globalerrstr[i]=0;
	//CL_CallBack_Error ( errstr );
}
void reportError(const wchar_t* error, const int resource){
	unsigned int i;
	if ( error == NULL )
		return;

    if ( isValidResource(resource) ){
    	for ( i=0;i<errlen-1&&i<wcslen(error);i++ )
    		resources[resource]->errstr[i]=error[i];
    	resources[resource]->errstr[i]=0;
    	//CL_CallBack_Error ( errstr );
	}
	
}
#endif

void reportGlobalError(const char* error){
	unsigned int i;
	if ( error == NULL )
		return;

	for ( i=0;i<errlen-1&&i<strlen(error);i++ )
		globalerrstr[i]=error[i];
	globalerrstr[i]=0;
	//CL_CallBack_Error ( errstr );
}
void reportError(const char* error, const int resource){
	unsigned int i;
	if ( error == NULL )
		return;

    if ( isValidResource(resource) ){
    	for ( i=0;i<errlen-1&&i<strlen(error);i++ )
    		resources[resource]->errstr[i]=error[i];
    	resources[resource]->errstr[i]=0;
    	//CL_CallBack_Error ( errstr );
	}
	
}

int getResource(const char_t* dir){
	//multithreading mutex lock...
	while ( resource_mutex ){
		processSleep(5);	
	}
	
	resource_mutex = true;
	_TRY{
		if ( !resourcesInitialised ){
			for ( int i=1;i<MAX_CLUCENE_DLL_RESOURCES;i++ )
				resources[i] = NULL;
			resourcesInitialised = 1;
		}
		int pos = 0;
	    for ( int i=1;i<MAX_CLUCENE_DLL_RESOURCES;i++ ){
	        if ( pos == 0 && resources[i] == NULL ){
	            pos = i;
	        }
	        if ( resources[i] != NULL && stringCompare(resources[i]->directory,dir)==0 ){
	            //directory already found
	            resources[i]->count++;
	            resource_mutex = false;
	            return i;
	        }
	    }
	    if ( pos > 0 ){
	        //no existing directory found
	        resources[pos] = new Resource(dir);
	        resource_mutex = false;
	        return pos;
	    }
	    reportGlobalError(_T("Max resource limit exceeded"));
	}_FINALLY(
		resource_mutex = false;
	);
    return 0;
}


extern "C" {

int isValidResource(const int resource, const int reportFail){
	if ( resource > -1 && resource < MAX_CLUCENE_DLL_RESOURCES && resources[resource] != NULL )
		return true;
	else if ( reportFail ){
		reportGlobalError(_T("Not a valid resource"));
		return false;
	}else
		return false;
}
	
void resetError(const int resource){
	globalerrstr[0]=0;
	if ( resource > 0 && resource < MAX_CLUCENE_DLL_RESOURCES && resources[resource] != NULL )
	    resources[resource]->errstr[0]=0;
}

#ifndef CLUCENE_LITE
int _addField(const int resource, const char_t* wfield, Reader& reader, const int store, const int index, const int token){
	resetError(resource);
	int ret = 0;
	if ( isValidResource(resource) ){
		if ( resources[resource]->doc == NULL ){
			CL_New_Document(resource);
		}

		try{
			lucene::document::Field* fld = new lucene::document::Field(wfield,&reader,store!=0,index!=0,token!=0);
			resources[resource]->doc->add(*fld);
			ret = 1;
		}catch(exception e){
			reportError( e.what(), resource);
		}catch(...){
			reportError( _T("Unknown error"), resource);
		}
	}
	return ret;
}
#endif

int _CL_Search(const int resource, Query* q){
	try{
		CL_ClearSearch(resource);

		resources[resource]->query = q;
		resources[resource]->searcher = new IndexSearcher(*resources[resource]->reader);
		resources[resource]->hits = &resources[resource]->searcher->search(*resources[resource]->query);

		return 1;
	}catch(exception e){
		reportError( e.what(), resource);
	}catch(...){
		reportError( _T("Unknown error"), resource);
	}
	return 0;
}


//
//end of private ================================

#ifdef USE_OTHER
CLUCENEDLL_API void CL_ERRSTR_OTHER(const int resource, o_char_t* pl, size_t len){
	char_t* buffer = new char_t[len];
	CL_ERRSTR(resource,buffer,len);
	_COPY_AWAY(buffer,pl,len);
	delete[] buffer;
}
CLUCENEDLL_API int CL_OPEN_OTHER(const o_char_t* dir, int create){
	char_t odir[CL_MAX_DIR];
	_COPY_BACK(dir,odir,CL_MAX_DIR);
	return CL_OPEN(odir,create);
}
CLUCENEDLL_API void CL_DOCUMENT_INFO_OTHER(const int resource, o_char_t* info, const int len){
	char_t* ret = new char_t[len];
	CL_DOCUMENT_INFO(resource,ret,len);
	_COPY_AWAY(ret,info,len);
	delete[] ret;
}

CLUCENEDLL_API int CL_SEARCHMULTIFIELDSFLAGGED_OTHER(const int resource, const o_char_t* query, const o_char_t** fields, const int fieldsLen, const l_byte_t* flags){
	char_t* oquery = _CONV_BACK(query);
	char_t** ofields = new char_t*[fieldsLen];
	for ( int i=0;i<fieldsLen;i++ )
		ofields[i] = _CONV_BACK(fields[i]);
	
	int ret = CL_SEARCHMULTIFIELDSFLAGGED(resource,oquery,(const char_t**)ofields,fieldsLen,flags);
	
	for ( int i=0;i<fieldsLen;i++ )
		delete[] ofields[i];
	delete[] ofields;
	delete[] query;

	return ret;
}
CLUCENEDLL_API int CL_SEARCHMULTIFIELDS_OTHER(const int resource, const o_char_t* query, const o_char_t** fields, const int fieldsLen){
	char_t* oquery = _CONV_BACK(query);
	char_t** ofields = new char_t*[fieldsLen];
	for ( int i=0;i<fieldsLen;i++ )
		ofields[i] = _CONV_BACK(fields[i]);
	
	int ret = CL_SEARCHMULTIFIELDS(resource,oquery,(const char_t**)ofields,fieldsLen);
	
	for ( int i=0;i<fieldsLen;i++ )
		delete[] ofields[i];
	delete[] ofields;
	delete[] oquery;

	return ret;
}
CLUCENEDLL_API int CL_SEARCH_OTHER(const int resource, const o_char_t* query, const o_char_t* field){
	char_t* oquery = _CONV_BACK(query);
	char_t* ofield = _CONV_BACK(field);
	int ret = CL_SEARCH(resource,oquery,ofield);

	delete[] oquery;
	delete[] ofield;

	return ret;
}
CLUCENEDLL_API void CL_SEARCHINFO_OTHER(const int resource, o_char_t* pl, size_t len){
	char_t* buf = new char_t[len];
	CL_SEARCHINFO(resource,buf,len);
	_COPY_AWAY(buf,pl,len);
	delete[] buf;
}
CLUCENEDLL_API type_long CL_GETDATEFIELD_OTHER(const int resource, const o_char_t* field){
	char_t* ofield = _CONV_BACK(field);
	type_long ret = CL_GETDATEFIELD(resource,ofield);
	delete[] ofield;
	return ret;
}
CLUCENEDLL_API int CL_GETFIELD_OTHER(const int resource, const o_char_t* field, o_char_t** value, size_t* value_len){
	char_t* ofield = _CONV_BACK(field);
	char_t* buffer = NULL;
	int ret = CL_GETFIELD(resource,ofield,&buffer,value_len);
	*value = new o_char_t[*value_len];
	_COPY_AWAY(buffer,*value,*value_len);
	delete[] buffer;
	delete[] ofield;
	return ret;
}

CLUCENEDLL_API void CL_ERRSTRGLOBAL_OTHER(o_char_t* pl, size_t len){
	char_t* buffer = new char_t[len];
	CL_ERRSTRGLOBAL(buffer,len);
	_COPY_AWAY(buffer,pl,len);
	delete[] buffer;
}

#ifndef CLUCENE_LITE
CLUCENEDLL_API int CL_UNLOCK_OTHER(const o_char_t* directory){
	char_t odirectory[CL_MAX_DIR];
	_COPY_BACK(directory,odirectory,CL_MAX_DIR);
	return CL_UNLOCK(odirectory);
}
CLUCENEDLL_API int CL_ADD_FILE_OTHER(const int resource, const o_char_t* field, const o_char_t* filename, const int store, const int index, const int token){
	char_t* ofield = _CONV_BACK(field);
	char_t* ofilename = _CONV_BACK(filename);
	int ret = CL_ADD_FILE(resource,ofield,ofilename,store,index,token);
	delete[] ofield;
	delete[] ofilename;
	return ret;
}
CLUCENEDLL_API int CL_ADD_FIELD_OTHER(const int resource, const o_char_t* field, const o_char_t* value, const int value_length, const int store, const int index, const int token){
	char_t* ofield = _CONV_BACK(field);
	char_t* ovalue = new char_t[value_length];
	_COPY_BACK(value,ovalue,value_length);
	int ret = CL_ADD_FIELD(resource,ofield,ovalue,value_length,store,index,token);
	
	delete[] ofield;
	delete[] ovalue;
	return ret;
}
CLUCENEDLL_API int CL_ADD_DATE_OTHER(const int resource, const o_char_t* field, const type_long time, const int store, const int index, const int token){
	char_t* ofield = _CONV_BACK(field);
	int ret = CL_ADD_DATE(resource,ofield,time,store,index,token);
	delete[] ofield;

	return ret;
}
CLUCENEDLL_API int CL_DELETE_OTHER(const int resource, const o_char_t* query,const o_char_t* field){
	char_t* oquery = _CONV_BACK(query);
	char_t* ofield = _CONV_BACK(field);
	int ret = CL_DELETE(resource,oquery,ofield);
	delete[] oquery;
	delete[] ofield;
	return ret;
}

#ifdef HIGHLIGHTING
CLUCENEDLL_API void CL_HIGHLIGHT_OTHER(const int resource, const char* text, const int text_is_filename, char* ret, const size_t ret_len){
	char_t* otext = _CONV_BACK(text);
	char_t* oret = new char_t[ret_len];

	CL_HIGHLIGHT_X(resource,otext,text_is_filename,oret,ret_len,HIGHLIGHT_SEPARATOR,HIGHLIGHT_MAX_NUM_FRAGMENTS,HIGHLIGHT_FRAGMENT_SIZE,1,HIGHLIGHT_HTML_START,HIGHLIGHT_HTML_END);
	_COPY_BACK(ret,oret,ret_len);
	delete[] otext;
}

CLUCENEDLL_API void CL_HIGHLIGHT_X_OTHER(const int resource, const char* text, const int text_is_filename, char* ret, const size_t ret_len, const char* separator, const int max_fragments, const int fragment_size,  const int type, const char* d1, const char* d2){
	char_t* otext = _CONV_BACK(text);
	char_t* oseparator = _CONV_BACK(separator);
	char_t* od1 = _CONV_BACK(d1);
	char_t* od2 = _CONV_BACK(d2);
	char_t* oret = new char_t[ret_len];

	CL_HIGHLIGHT_X(resource,otext,text_is_filename,oret,ret_len,oseparator,max_fragments,fragment_size,type,od1,od2);
	_COPY_BACK(ret,oret,ret_len);
	delete[] otext;
	delete[] oseparator;
	delete[] od1;
	delete[] od2;
}
#endif
#endif
#endif

//returns resource number
CLUCENEDLL_API int CL_OPEN(const char_t* dir, int create){
	resetError(-1);
	const int resource = getResource(dir);
	if ( isValidResource(resource,0) ){
    	try{
    		if ( IndexReader::indexExists(dir) ){
				resources[resource]->reader = &IndexReader::open(dir);
    			return resource;
    		}else if ( create ){
#ifndef CLUCENE_LITE
				Directory* d = &FSDirectory::getDirectory( dir,true );
				IndexWriter w(*d,analyzer,true);
    			w.close(false);

				resources[resource]->reader = &IndexReader::open(*d);
    			return resource;
#else
				reportGlobalError("Can't create directory in Lite Mode");
#endif
    		}
    	}catch(exception e){
    		reportGlobalError( e.what());
    	}catch(...){
    		reportGlobalError( _T("Unknown error") );
    	}
    }
	return 0;
}



//info must be an inistialised char array, a maximum len will be copied
CLUCENEDLL_API void CL_DOCUMENT_INFO(const int resource, char_t* info, const int len){
	if ( isValidResource(resource) ){
		if ( resources[resource]->doc != NULL ){
			const char_t* inf = resources[resource]->doc->toString();
			stringNCopy(info,inf,len);
			delete[] inf;
		}else{
			info[0]=0;
			reportError(_T("No document available"), resource);
		}
	}
}



CLUCENEDLL_API int CL_SEARCHMULTIFIELDSFLAGGED(const int resource, const char_t* query, const char_t** fields, const int fieldsLen, const l_byte_t* flags){
	int ret = 0;
	if ( isValidResource(resource) ){
		Query* q = &MultiFieldQueryParser::Parse(query,fields,fieldsLen,flags,analyzer);
		ret = _CL_Search(resource,q);
	}
	return ret;
}

CLUCENEDLL_API int CL_SEARCHMULTIFIELDS(const int resource, const char_t* qry, const char_t** fields, const int fieldsLen){
	int ret = 0;
	if ( isValidResource(resource) ){
		l_byte_t* flags = new l_byte_t[fieldsLen];
		for ( int i=0;i<fieldsLen;i++ )
			flags[i] = lucene::queryParser::MultiFieldQueryParser::NORMAL_FIELD;

		ret = CL_SEARCHMULTIFIELDSFLAGGED(resource,qry,fields,fieldsLen,flags);
		delete[] flags;
	}
	return ret;
}

CLUCENEDLL_API int CL_SEARCH(const int resource, const char_t* query, const char_t* field){
	int ret = 0;
	if ( isValidResource(resource) ){
		Query* q = &QueryParser::Parse(query,field,analyzer);
		ret = _CL_Search(resource,q);
	}
	return ret;
}
CLUCENEDLL_API void CL_SEARCHINFO(const int resource, char_t* pl, size_t len){
	if ( isValidResource(resource) ){
		Query* q = resources[resource]->query;
		if ( q != NULL ){
			const char_t* buf = q->toString(_T(""));
			stringNCopy(pl,buf,len);
			delete[] buf;
		}
	}
}

#ifdef HIGHLIGHTING
CLUCENEDLL_API void CL_HIGHLIGHT_X(const int resource, const char_t* text, const int text_is_filename, char_t* ret, const size_t ret_len, const char_t* separator, const int max_fragments, const int fragment_size, const int type, const char_t* d1, const char_t* d2){
	if ( isValidResource(resource) ){
		Query* q = resources[resource]->query;
		if ( q != NULL ){
			SimpleAnalyzer simpleAnalyzer;
			if ( type == 1 ){
				Highlighter highlighter(new SimpleHTMLFormatter( d1, d2), new QueryScorer(q), new SimpleFragmenter(fragment_size));

				Reader* r = NULL;
				//if ( text_is_filename ){
				//todo: make getBestFragments use an filereader object
				//	r = new FileReader(text);
				//}else{
					r = new StringReader(text);
				//}
				TokenStream& ts = simpleAnalyzer.tokenStream(_T("dummy"), r);
				// Compare two best fragments
				char_t * highlighted_text = highlighter.getBestFragments(&ts, text, max_fragments, separator);
				stringNCopy(ret,highlighted_text,ret_len);

				delete[] highlighted_text;
				ts.close();
				delete &ts;
				delete r;
			}
		}
	}
}
CLUCENEDLL_API void CL_HIGHLIGHT(const int resource, const char_t* text, const int text_is_filename, char_t* ret, const size_t ret_len){
	CL_HIGHLIGHT_X(resource,text,text_is_filename,ret,ret_len,HIGHLIGHT_SEPARATOR,HIGHLIGHT_MAX_NUM_FRAGMENTS,HIGHLIGHT_FRAGMENT_SIZE, 1,HIGHLIGHT_HTML_START,HIGHLIGHT_HTML_END);
}
#endif

CLUCENEDLL_API type_long CL_GETDATEFIELD(const int resource, const char_t* field){
	type_long ret = 0;
	if ( isValidResource(resource) ){
		
		if ( resources[resource]->hits == NULL || resources[resource]->searcher == NULL ){
			reportError (_T("No hits or searcher available"), resource);
			return 0;
		}

		try{
			Document& doc = resources[resource]->hits->doc(resources[resource]->hitPos);
			if ( &doc != NULL ){
				const char_t* val = doc.get(field);
				
				if ( val != NULL ){
					try{
						ret = DateField::stringToTime(val);
					}catch(...){
						reportError(_T("Field is not a date"), resource);
					}
				}else{
					reportError(_T("Field does not exist"), resource);
				}
			}
		}catch(exception e){
			reportError( e.what(), resource);
		}catch(...){
			reportError( _T("Unknown error"), resource);
		}
	}
	return ret;
}

//value will be initialised as a char array with a length of value_len
CLUCENEDLL_API int CL_GETFIELD(const int resource, const char_t* field, char_t** value, size_t* value_len){
	int ret = 0;
	if ( isValidResource(resource) ){
		*value_len = 0;

		if ( resources[resource]->hits == NULL || resources[resource]->searcher == NULL ){
			reportError ( _T("No hits or searcher available"), resource);
			return 0;
		}

		try{
			Document& doc = resources[resource]->hits->doc(resources[resource]->hitPos);
			if ( &doc != NULL ){
				//todo: should be a way of getting value length.. not binary safe
				const char_t* val = doc.get(field); //a reference to that field
				
				if ( val != NULL ){
					*value_len = stringLength (val);
					*value = new char_t[*value_len+1];
					stringNCopy(*value,val,*value_len+1);
					ret = 1;
				}else{
					reportError(_T("Field does not exist"), resource);
				}
			}
		}catch(exception e){
			reportError( e.what(), resource);
		}catch(...){
			reportError( _T("Unknown error"), resource);
		}
	}
	return ret;
}


//copy error string. 'pl' will be a maximum length of 'len'
CLUCENEDLL_API void CL_ERRSTR(const int resource, char_t* pl, size_t len){
    //resetError(resource);
    if ( isValidResource(resource,false) ){
    	size_t i=0;
    	for ( i=0; i<len && i<errlen && i<stringLength(resources[resource]->errstr); i++ )
    		pl[i]=resources[resource]->errstr[i];
    	pl[i]=0;
	}else
		CL_ERRSTRGLOBAL(pl,len);
}

//copy global error string. 'pl' will be a maximum length of 'len'
CLUCENEDLL_API void CL_ERRSTRGLOBAL(char_t* pl, size_t len){
    size_t i=0;
	for ( i=0; i<len && i<errlen && i<stringLength(globalerrstr); i++ )
		pl[i]=globalerrstr[i];
	pl[i]=0;
}




#ifndef CLUCENE_LITE
CLUCENEDLL_API int CL_UNLOCK(const char_t* directory){
	resetError(-1);
	int ret = 0;
	try{
		FSDirectory dir(directory,false);
		IndexReader::unlock(dir);
		ret = 1;
	}catch(exception e){
    	reportGlobalError( e.what());
    }catch(...){
    	reportGlobalError( _T("Unknown error") );
    }
	return ret;
}

//adds a field to the current document
CLUCENEDLL_API int CL_ADD_FILE(const int resource, const char_t* field, const char_t* filename, const int store, const int index, const int token){
	resetError(resource);
	int ret = 0;
	if ( isValidResource(resource) ){
		try{
			FileReader& reader = *new FileReader(filename);
			ret = _addField(resource,field,reader,store,index,token);
		}catch(exception e){
			reportError( e.what(), resource);
		}catch(...){
			reportError( _T("Unknown error"), resource);
		}
	}
	return ret;
}


//adds a field to the current document
CLUCENEDLL_API int CL_ADD_FIELD(const int resource, const char_t* field, const char_t* value, const int value_length, const int store, const int index, const int token){
	char_t* cvalue = new char_t[value_length+1];
	stringNCopy(cvalue,value,value_length+1);

	//wvalue is taken by the reader, no need to delete it... gets deleted by field
	StringReader& reader = *new StringReader(cvalue,value_length,true);
	return _addField(resource,field,reader,store,index,token);
}

//adds a date field to current document
CLUCENEDLL_API int CL_ADD_DATE(const int resource, const char_t* field, const type_long time, const int store, const int index, const int token){
	resetError(resource);
	int ret = 0;
	if ( isValidResource(resource) ){
		if ( resources[resource]->doc == NULL ){
			CL_New_Document(resource);
		}

		try{
			char_t* value = lucene::document::DateField::timeToString(time);

			lucene::document::Field* fld = new lucene::document::Field(field,value,store!=0,index!=0,token!=0);
			resources[resource]->doc->add(*fld);

			delete[] value;
			ret = 1;
		}catch(exception e){
			reportError( e.what(), resource);
		}catch(...){
			reportError( _T("Unknown error"), resource);
		}
	}
	return ret;
}


//deletes documents returned from specified query.
//returns number of documents deleted.
//returns -1 if an error occurs
CLUCENEDLL_API int CL_DELETE(const int resource, const char_t* query,const char_t* field){
	int ret = -1;

	if ( isValidResource(resource) ){
		Query* q = NULL;
		IndexSearcher *s = NULL;
		Hits* h = NULL;

		try{
			q = &QueryParser::Parse(query,field,analyzer);
			s = new IndexSearcher(*resources[resource]->reader);

			h = &s->search(*q);
			ret = 0;
			for ( int i=0;i<h->Length();i++ ){
				resources[resource]->reader->Delete(h->id(i));
				ret++;
			}
		}catch(exception e){
			reportError( e.what(), resource);
			ret = -1;
		}catch(...){
			reportError( _T("Unknown error"), resource);
			ret = -1;
		}
		if ( s != NULL )
			s->close();

		delete s;
		delete h;
		delete q;
	}
	return ret;
}
#endif



//NON-CHAR SPECIFIC CODE//////////////////////////////
//
//

CLUCENEDLL_API void CL_Cleanup(){
    for ( int i=1;i<MAX_CLUCENE_DLL_RESOURCES;i++ ){
        while(CL_Close(i)>1){
        }
    }
}

//reopen the reader to take note of segment changes
CLUCENEDLL_API int CL_Reload(const int resource){
	resetError(resource);
	if ( isValidResource(resource) ){
        try{
			resources[resource]->reader->close();
			delete resources[resource]->reader;
			resources[resource]->reader = &IndexReader::open(resources[resource]->directory);
    
        	return 1;
        }catch(exception e){
        	reportError( e.what(), resource);
        }catch(...){
        	reportError( "Unknown error", resource);
        }
	}
	return 0;
}

CLUCENEDLL_API int CL_Close(const int resource){
	resetError(resource);
	if ( isValidResource(resource) ){
    	resources[resource]->count--;
    	if ( resources[resource]->count < 1 ){
        	try{
        		resources[resource]->reader->close();
    
    		    CL_ClearSearch(resource);
    		    delete resources[resource];
    		    resources[resource] = NULL;
     
        		return 1;
        	}catch(exception e){
        		reportError( e.what(), resource);
        	}catch(...){
        		reportError( "Unknown error", resource);
        	}
        }else
            return resources[resource]->count +1;
	}
	return 0;
}


CLUCENEDLL_API void CL_ClearSearch(const int resource){
	if ( isValidResource(resource) ){
		try{
			if ( resources[resource]->searcher != NULL ){
				resources[resource]->searcher->close();
				_DELETE(resources[resource]->searcher);
			}
		}catch(...){
		}

		try{
			if ( resources[resource]->query != NULL ){
				_DELETE(resources[resource]->query);
			}
		}catch(...){
		}

		_DELETE (resources[resource]->hits);
		resources[resource]->hitPos = 0;
	}
}
CLUCENEDLL_API int CL_HitCount(const int resource){
	if ( isValidResource(resource) ){
		if ( resources[resource]->hits == NULL ){
			reportError("No hits available", resource);
			return 0;
		}else
			return resources[resource]->hits->Length();
	}else
		return 0;
}

CLUCENEDLL_API int CL_NextHit(const int resource){
	int ret = 0;
	if ( isValidResource(resource) ){
		if ( resources[resource]->hits == NULL || resources[resource]->searcher == NULL ){
			reportError ("No hits or searcher available", resource);
		}else{
			resources[resource]->hitPos++;
			if ( resources[resource]->hitPos < resources[resource]->hits->Length() )
				ret = 1;
		}
	}
	return ret;
}

CLUCENEDLL_API int CL_GotoHit(const int resource, const int number){
	int ret = 0;
	if ( isValidResource(resource) ){
		if ( resources[resource]->hits == NULL || resources[resource]->searcher == NULL ){
			reportError ("No hits or searcher available", resource);
		}else{
		    if ( (number < 0) || (number > resources[resource]->hits->Length()) )
		        ret = 1;
		    else
		        resources[resource]->hitPos = number;
		}
	}
	return ret;
}

CLUCENEDLL_API float CL_HitScore(const int resource){
    float ret = 0;
    if ( isValidResource(resource) ){
		if ( resources[resource]->hits == NULL || resources[resource]->searcher == NULL ){
			reportError ( _T("No hits or searcher available"), resource);
			return 0;
		}

		try{
			ret = resources[resource]->hits->score(resources[resource]->hitPos);
		}catch(exception e){
			reportError( e.what(), resource);
		}catch(...){
			reportError( _T("Unknown error"), resource);
		}
	}
	return ret;
}

#ifndef CLUCENE_LITE
//resets the current document
CLUCENEDLL_API int CL_New_Document(const int resource){
	resetError(resource);
	if ( isValidResource(resource) ){
		try{
			delete resources[resource]->doc;
			resources[resource]->doc = new Document();
			return 1;
		}catch(exception e){
			reportError( e.what(), resource);
		}catch(...){
			reportError( "Unknown error", resource);
		}
	}
	return 0;
}

CLUCENEDLL_API int CL_Optimize(const int resource){
	resetError(resource);
	if ( isValidResource(resource) ){
		try{
			//temporarily close the reader
			resources[resource]->reader->close();
			delete resources[resource]->reader;
			resources[resource]->reader = NULL;

			IndexWriter w(resources[resource]->directory,analyzer,false);
			w.optimize();
			w.close(false);

			//reopen the reader.
			resources[resource]->reader = &IndexReader::open(resources[resource]->directory);
			return 1;
		}catch(exception e){
			reportError( e.what(), resource);
		}catch(...){
			reportError( "Unknown error", resource);
		}
	}
	return 0;
}



//adds the current document
CLUCENEDLL_API int CL_Insert_Document(const int resource){
	resetError(resource);
	int ret = 0;
	if ( isValidResource(resource) ){
		if ( resources[resource]->doc == NULL ){
			reportError("No document available", resource);
			return 0;
		}

		IndexWriter* w = NULL;
		try{
			w = new IndexWriter(resources[resource]->reader->directory,analyzer,false);
			w->addDocument(*resources[resource]->doc);

			ret = 1;
		}catch(exception e){
			reportError( e.what(), resource);
		}catch(...){
			reportError( "Unknown error", resource);
		}
		if ( w != NULL )
			w->close(false);
		_DELETE(w);

		//clear current document
		delete resources[resource]->doc;
		resources[resource]->doc = new Document();
	}
	return ret;
}
#endif

//
//
//////////////////////////////////////////////////////

}//extern "C" 
