/*
   +----------------------------------------------------------------------+
   | PHP Version 4                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2003 Ben van Klinken                              |
   +----------------------------------------------------------------------+
   | 
   +----------------------------------------------------------------------+
   | Author: Ben van Klinken <bvk@villagechief.com>                       |
   +----------------------------------------------------------------------+
*/

/* $Id:  */

#ifndef PHP_CLUCENE_H
#define PHP_CLUCENE_H

extern zend_module_entry clucene_module_entry;
#define phpext_clucene_ptr &clucene_module_entry


PHP_MINIT_FUNCTION(clucene);
PHP_MSHUTDOWN_FUNCTION(clucene);
PHP_MINFO_FUNCTION(clucene);
PHP_FUNCTION(cl_open);
PHP_FUNCTION(cl_close);
PHP_FUNCTION(cl_reload);
PHP_FUNCTION(cl_optimize);
PHP_FUNCTION(cl_delete);
PHP_FUNCTION(cl_errno);
PHP_FUNCTION(cl_errstr);
PHP_FUNCTION(cl_error);

PHP_FUNCTION(cl_new_document);
PHP_FUNCTION(cl_add_field);
PHP_FUNCTION(cl_add_file);
PHP_FUNCTION(cl_add_date);
PHP_FUNCTION(cl_insert_document);
PHP_FUNCTION(cl_document_info);

PHP_FUNCTION(cl_search);
PHP_FUNCTION(cl_hitcount);
PHP_FUNCTION(cl_searchinfo);
PHP_FUNCTION(cl_highlight);
PHP_FUNCTION(cl_nexthit);
PHP_FUNCTION(cl_clearsearch);
PHP_FUNCTION(cl_getfield);
PHP_FUNCTION(cl_getdatefield);



#endif /* PHP_CLUCENE_H */

