
PHP_ARG_ENABLE(clucene,whether to enable clucene,
[  --enable-clucene            Enable CLucene])

PHP_ARG_WITH(clucene-dir, for specified location of the CLucene library and include files,
[  --with-clucene-dir[=DIR]   CLucene: Location of the CLucene library and include files.
                            If unspecified, the default locations are searched.], no, no)

if test "$PHP_CLUCENE" != "no"; then
  if test -r $PHP_CLUCENE_DIR/src/CLucene/CLucene.h; then
    CLUCENE_DIR=$PHP_CLUCENE_DIR
  else
    AC_MSG_CHECKING(for CLucene in default path)
    for i in /usr/local/* /opt/*; do
      if test -r $i/src/CLucene/CLucene.h; then
        CLUCENE_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi

  if test -z "$CLUCENE_DIR"; then
    AC_MSG_RESULT(not found)
    AC_MSG_ERROR(Please install the CLucene library)
  fi
fi

  CLUCENE_LIB_DIR=$CLUCENE_DIR/build/gcc/src
  PHP_ADD_LIBRARY_WITH_PATH(clucene, $CLUCENE_LIB_DIR, CLUCENE_SHARED_LIBADD)
  CLUCENE_LIBS="-L$CLUCENE_LIB_DIR -lclucene"

  CPPFLAGS="-g -DCOMPILE_DL_CLUCENE"

  PHP_REQUIRE_CXX

  PHP_NEW_EXTENSION(clucene, php_clucene.c clucene.cpp, $ext_shared)
  PHP_ADD_LIBRARY(stdc++)
  PHP_ADD_INCLUDE($CLUCENE_DIR/src)

  PHP_SUBST(CLUCENE_SHARED_LIBADD)  
  PHP_SUBST_OLD(CLUCENE_LIBS)
