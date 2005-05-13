MAINTAINERCLEANFILES = Makefile.in

INCLUDES          = -I$(top_srcdir)/src \
	-I$(top_srcdir)/wrappers/dll/dlltest

bin_PROGRAMS      = cl_dlltest

cl_dlltest_SOURCES     = stdafx.cpp dlltest.cpp ../clucene_dll.cpp

cl_dlltest_LDADD       = $(top_builddir)/src/libclucene.la