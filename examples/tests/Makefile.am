MAINTAINERCLEANFILES = Makefile.in

INCLUDES          = -I$(top_srcdir)/src \
	-I$(top_srcdir)/examples/tests

bin_PROGRAMS      = cl_tests

cl_tests_SOURCES     = Main.cpp test/AnalysisTest.cpp test/AnalyzersTest.cpp test/HighFreqTerms.cpp \
					test/SearchTest.cpp test/SearchTestForDuplicates.cpp test/StoreTest.cpp \
					test/TestDateFilter.cpp test/TestPriorityQueue.cpp test/TestQueryParser.cpp test/TestWildcard.cpp

cl_tests_LDADD       = $(top_builddir)/src/libclucene.la

