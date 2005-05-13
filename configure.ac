dnl Process this file with autoconf to produce a configure script.
dnl Written by Jimmy Pritts.

dnl initialize autoconf and automake
AC_INIT([clucene], [1])
AC_PREREQ([2.54])
AC_CONFIG_SRCDIR([src/CLucene.h])
AC_CONFIG_AUX_DIR([./build/gcc/config])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE

dnl Check for existence of a C and C++ compilers.
AC_PROG_CC
AC_PROG_CXX

dnl Check for headers
AC_HEADER_DIRENT
 
dnl Configure libtool.
AC_PROG_LIBTOOL

dnl option to use UTF-8 as internal 8-bit charset to support characters in Unicode™
AC_ARG_ENABLE(utf8,
AC_HELP_STRING([--enable-utf8],[UTF-8 as internal 8-bit charset to support characters in Unicode™ (default=no)]),
 [AC_DEFINE([UTF8],[],[use UTF-8 as internal 8-bit charset to support characters in Unicode™])],enable_utf8=no)

AM_CONDITIONAL(USEUTF8, test x$enable_utf8 = xyes)

AC_CONFIG_FILES([Makefile src/Makefile examples/Makefile examples/demo/Makefile examples/tests/Makefile examples/util/Makefile wrappers/Makefile wrappers/dll/Makefile wrappers/dll/dlltest/Makefile])
AC_OUTPUT
