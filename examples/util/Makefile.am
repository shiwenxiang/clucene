MAINTAINERCLEANFILES = Makefile.in

INCLUDES          = -I$(top_srcdir)/src \
	-I$(top_srcdir)/examples/util

bin_PROGRAMS      = cl_util

cl_util_SOURCES     = Main.cpp add.cpp delete.cpp keys.cpp merge.cpp optimize.cpp search.cpp stats.cpp stdafx.cpp

cl_util_LDADD       = $(top_builddir)/src/libclucene.la