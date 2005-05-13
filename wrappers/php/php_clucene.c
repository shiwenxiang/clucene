
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "clucene_dll.h" //the dll wrapper
#include "php.h"
#include "php_clucene.h"

/* PHP Includes */
#include "ext/standard/info.h"

function_entry clucene_functions[] = {
	ZEND_FE(cl_open,       NULL)
	ZEND_FE(cl_close,       NULL)
	ZEND_FE(cl_reload,       NULL)
	ZEND_FE(cl_optimize, NULL)
	ZEND_FE(cl_delete, NULL)
	ZEND_FE(cl_errno,      NULL)
	ZEND_FE(cl_errstr,      NULL)
	ZEND_FE(cl_error,      NULL)

	ZEND_FE(cl_new_document, NULL)
	ZEND_FE(cl_add_field, NULL)
	ZEND_FE(cl_add_file, NULL)
	ZEND_FE(cl_add_date, NULL)
	ZEND_FE(cl_insert_document, NULL)
	ZEND_FE(cl_document_info, NULL)

	
	ZEND_FE(cl_search, NULL)
	ZEND_FE(cl_searchinfo,NULL)
	ZEND_FE(cl_highlight,NULL)
	ZEND_FE(cl_hitcount, NULL)
	ZEND_FE(cl_nexthit, NULL)
	ZEND_FE(cl_clearsearch, NULL)
	ZEND_FE(cl_getfield, NULL)
	ZEND_FE(cl_getdatefield, NULL)

	{NULL, NULL, NULL}
};

zend_module_entry clucene_module_entry = {
	STANDARD_MODULE_HEADER,
	"clucene",
	clucene_functions,
	PHP_MINIT(clucene),
	PHP_MSHUTDOWN(clucene),
	NULL,
	NULL,
	PHP_MINFO(clucene),
	NO_VERSION_YET,
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_CLUCENE
ZEND_GET_MODULE(clucene)
#endif

PHP_MINIT_FUNCTION(clucene)
{
	//CL_Init();
	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(clucene)
{
	CL_Cleanup();
	return SUCCESS;
}

PHP_MINFO_FUNCTION(clucene)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "CLucene Support", "Enabled");
	php_info_print_table_row(2, "CLucene Version", "0.9.(x)");
	php_info_print_table_end();
}

/* Returns the error number */
PHP_FUNCTION(cl_errno)
{
	RETURN_LONG(0);
}
/* Returns the error string */
PHP_FUNCTION(cl_errstr)
{
	long resource;
	char err[errlen];
	if ( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                          "l", &resource) == FAILURE) {
		return;
	}
	CL_ERRSTR(resource,err,errlen);
	RETURN_STRING(err, 1);
}
/* Returns the error number and error string in an associative array */
PHP_FUNCTION(cl_error)
{
	array_init(return_value);
		
	add_assoc_long  (return_value, "errno",  0);
	add_assoc_string(return_value, "errstr", "TEST",1);
}


/* Opens a new CLucene directory */
//proto string clopen(string dir, bool create=false)
PHP_FUNCTION(cl_open)
{
	int cr;
	char *s;
	int s_len;
	long ret = 0;

	if( ZEND_NUM_ARGS() > 2 || ZEND_NUM_ARGS() < 1 )
		WRONG_PARAM_COUNT;

	if ( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                          "s|b", &s, &s_len, &cr) == FAILURE) {
		return;
	}

	ret = CL_OPEN(s,cr);

	if ( ret != 0 )
		RETURN_LONG(ret);

	RETURN_NULL();
}


/* Closes a CLucene directory */
PHP_FUNCTION(cl_close)
{
	long resource;

	if ( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                          "l", &resource) == FAILURE) {
		return;
	}

    if ( CL_CLOSE(resource) != 1 )
		RETURN_FALSE;
	
	RETURN_TRUE;
}

/* Reloads a CLucene index */
PHP_FUNCTION(cl_reload)
{
	long resource;

	if ( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                          "l", &resource) == FAILURE) {
		return;
	}

    if ( CL_RELOAD(resource) != 1 )
		RETURN_FALSE;
	
	RETURN_TRUE;
}

/* Optimizes a Clucene directory */
PHP_FUNCTION(cl_optimize)
{
	long resource;

	if ( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                          "l", &resource) == FAILURE) {
		return;
	}
    if ( CL_OPTIMIZE(resource) != 1 )
		RETURN_FALSE;

	RETURN_TRUE;

}


/* Creates a new document*/
PHP_FUNCTION(cl_new_document)
{
	long resource;
	if ( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                          "l", &resource) == FAILURE) {
		return;
	}
	if ( CL_NEW_DOCUMENT(resource) != 1 )
		RETURN_FALSE;
	
	RETURN_TRUE;
}

/* Adds a field to the current document*/
PHP_FUNCTION(cl_add_field)
{
	long resource;
	char* fld;
	char* val;
	int fld_len;
	int val_len;
	int store = 0;
	int index = 1;
	int token = 1;

	if ( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                          "lss|bbb", &resource, &fld, &fld_len, &val, &val_len, &store, &index, &token) == FAILURE) {
		return;
	}
	if ( CL_ADD_FIELD(resource,fld,val,val_len,store,index,token) != 1 )
		RETURN_FALSE;
	
	RETURN_TRUE;
}

PHP_FUNCTION(cl_add_file)
{
	long resource;
	char* fld;
	char* file;
	int fld_len;
	int file_len;
	int store = 0;
	int index = 1;
	int token = 1;

	if ( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                          "lss|bbb", &resource, &fld, &fld_len, &file, &file_len, &store, &index, &token) == FAILURE) {
		return;
	}
	if ( CL_ADD_FILE(resource,fld,file,store,index,token) != 1 )
		RETURN_FALSE;
	
	RETURN_TRUE;
}

/* Adds a date field to the current document*/
PHP_FUNCTION(cl_add_date)
{
	long resource;
	char* fld;
	char* val;
	int fld_len;
	int val_len;
	int store = 0;
	int index = 1;
	int token = 1;

	if ( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                          "ls|bbb", &resource, &fld, &fld_len, &val, &val_len, &store, &index, &token) == FAILURE) {
		return;
	}
	if ( CL_ADD_DATE(resource,fld,0,store,index,token) != 1 )
		RETURN_FALSE;
	
	RETURN_TRUE;
}

/* Inserts the current document*/
PHP_FUNCTION(cl_insert_document)
{
	long resource;

	if ( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                          "l", &resource) == FAILURE) {
		return;
	}

    if ( CL_INSERT_DOCUMENT(resource) != 1 )
		RETURN_FALSE;
	
	RETURN_TRUE;
}

/* Returns text about document info */
PHP_FUNCTION(cl_document_info)
{
	long resource;
	char ret[2048];
	if ( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                          "l", &resource) == FAILURE) {
		return;
	}
	CL_DOCUMENT_INFO(resource,ret,2048);
	RETURN_STRING(ret,1);
}

/* Performs a search*/
PHP_FUNCTION(cl_search)
{
	long resource;
	zval *field;
	char* qry;
	char* fld;

	HashTable *fldHash;
	zval **hash_entry;
	char* flds[100];
	int fldLen;
	int i=0;

	int qry_len;

	if ( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                          "lsz", &resource, &qry, &qry_len, &field) == FAILURE) {
		return;
	}

	if ( field->type == IS_ARRAY ){
		convert_to_array_ex(&field);
		fldHash = Z_ARRVAL_P(field);
		fldLen = zend_hash_num_elements(fldHash);

		for ( i=0;i<fldLen;i++ ){
			if (zend_hash_get_current_data(fldHash, (void **) &hash_entry) == FAILURE) {
				break;
			}
			convert_to_string_ex(hash_entry);
			flds[i]=Z_STRVAL_PP(hash_entry);

			zend_hash_move_forward(fldHash);
		}

		if ( CL_SEARCHMULTIFIELDS(resource,qry,flds,fldLen) != 1 )
			RETURN_FALSE;
		RETURN_TRUE;
	}else{
		convert_to_string_ex(&field);
		fld = Z_STRVAL_PP(&field);

		if ( CL_SEARCH(resource,qry,fld) != 1 )
			RETURN_FALSE;
        RETURN_TRUE;
	}
}


/* Creates a new document*/
PHP_FUNCTION(cl_searchinfo)
{
	long resource;
	char query[2048];
	if ( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                          "l", &resource) == FAILURE) {
		return;
	}
	
	CL_SEARCHINFO (resource,query,2048);
	RETURN_STRING(query,1);
}

/* Retrieves highlighted text for current query*/
PHP_FUNCTION(cl_highlight)
{
	long resource;
	char* text;
	char* separator;
	char* d1;
	char* d2;

	int text_len;
	int separator_len;
	int d1_len;
	int d2_len;
	int text_is_filename;
	int max_fragments;
	int fragment_size;
	int type;
	
	char ret[2048];

	if ( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                          "lsb|slllss", &resource, &text, &text_len, &text_is_filename,
						  &separator,&separator_len,
						  &max_fragments,
						  &fragment_size,
						  &type,
						  &d1,&d1_len,
						  &d2, &d2_len) == FAILURE) {
		return;
	}
	if ( separator_len == 0 )
		separator = "...";
	if ( max_fragments == 0 )
		max_fragments = 3;
	if ( fragment_size == 0 )
		fragment_size == 100;
	if ( type == 0 )
		type = 1;
	if ( d1_len == 0 )
		d1 = "<b>";
	if ( d2_len == 0 )
		d2 = "</b>";
	CL_HIGHLIGHT_X (resource,text,text_is_filename,ret,2048,separator,max_fragments,fragment_size,type,d1,d2);
	RETURN_STRING(ret,1);
}

PHP_FUNCTION(cl_hitcount)
{
	long resource;
	if ( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                          "l", &resource) == FAILURE) {
		return;
	}
	RETURN_LONG(CL_HITCOUNT(resource));
}

PHP_FUNCTION(cl_nexthit)
{
	long resource;
	if ( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                          "l", &resource) == FAILURE) {
		return;
	}
	if ( CL_NEXTHIT(resource) != 1 )
		RETURN_FALSE;
	RETURN_TRUE;
}

PHP_FUNCTION(cl_clearsearch)
{
	long resource;
	if ( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                          "l", &resource) == FAILURE) {
		return;
	}
	CL_CLEARSEARCH(resource);
}

PHP_FUNCTION(cl_getfield)
{
	long resource;
	char* fld;
	int fld_len;
	char* val = NULL;
	size_t val_len = 0;

	if ( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                          "ls", &resource, &fld, &fld_len) == FAILURE) {
		return;
	}
	
	if ( CL_GETFIELD(resource,fld,&val,&val_len) != 1 || val == NULL )
		RETURN_NULL();

	RETURN_STRINGL(val,val_len,1);
}

PHP_FUNCTION(cl_getdatefield)
{
	long resource;
	char* fld;
	int fld_len;
	type_long ret = 0;

	if ( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                          "ls", &resource, &fld, &fld_len) == FAILURE) {
		return;
	}
	
	ret = CL_GETDATEFIELD(resource,fld);
	RETURN_LONG(ret);
}

/* deletes the documents returned from the specified query*/
PHP_FUNCTION(cl_delete)
{
	long resource;
//	char* dir;
	char* qry;
	char* fld;
//	int dir_len;
	int qry_len;
	int fld_len;
	int ret = -1;

	if ( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                          "lss", &resource, &qry, &qry_len, &fld, &fld_len) == FAILURE) {
		return;
	}

	RETURN_LONG ( CL_DELETE(resource,qry,fld) );
}

