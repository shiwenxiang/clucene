# Makefile for CLucene perl extension and wrapper module
# Works on RedHat 9/Enterprise Linux and cygwin
#
# The default target creates
#		clucene_dllh.h
#		clucene_dllp.h
#		clucene_dll.o
# ready for export along with the shareable library libclucene.so as part of an RPM.
#
# Users can then build the CPAN module FulltextSearch::CLucene using the following
# commands: (a copy of the archive is supplied here as FulltextSearch-CLucene-1.00.tar.gz) 
#     tar xfz FulltextSearch-CLucene-1.00.tar.gz
#		cd FulltextSearch-CLucene-1.00
#		perl Makefile.PL
#		make test
#		make install
# Note you need to install swig version 1.3 or later before compiling the CPAN module.
#
# You can also compile and test the perl wrapper directly by doing
#		make test
#
# Copyright(c) 2005 Peter Edwards peterdragon@users.sourceforge.net
# All rights reserved. This package is free software; you can redistribute
# it and/or modify it under the same terms as Perl itself.
# 

OS := $(shell perl ./check.pl -os)
PERLCORE := $(shell perl check.pl -perl)
#PERLCORE=/usr/lib/perl5/5.8.0/i386-linux-thread-multi/CORE

ifeq ("cygwin","$(OS)") # cygwin
	LINKOPTS=
	LIBDIR=$(HOME)/clucene.output/lib
else
	# this sets the directories to search for libclucene.so and avoids needing
	# to set LD_LIBRARY_PATH or LD_RUN_PATH environment variables at run time
	LINKOPTS=-Xlinker -rpath /usr/lib -Xlinker -rpath /usr/local/lib
	LIBDIR=/usr/lib
endif

INCDIR=-I../../src -I.
INCCL=$(INCDIR)
CFLAGS=$(INCCL)
LDFLAGS=-lstdc++
LIBCL=-L$(LIBDIR) -lclucene
LIBCLUCENE=-L$(LIBDIR) -lclucene
SWIGCFLAGS=-D_REENTRANT -D_GNU_SOURCE -DTHREADS_HAVE_PIDS -DDEBUGGING -fno-strict-aliasing -I/usr/local/include -I/usr/include/gdbm -fpic

all: dll

# build C interface DLL and headers ready for export in RPM archive
dll: config.h clucene_dll.o clucene_dllh.h clucene_dllp.h

# non-object oriented DLL interface library
clucene_dll.o: ../dll/clucene_dll.cpp
	c++ -c -o clucene_dll.o ../dll/clucene_dll.cpp $(CFLAGS)

# run CLucene #include header line through preprocessor so we can export a header file
# that doesn't need the CLucene source available when using it
clucene_dllh1.h:
	echo '#include "CLucene/CLConfig.h"' > clucene_dllh1.h

config.h:
	echo '#define HAVE_DIRENT_H' > config.h

clucene_dllh.h: clucene_dllh1.h
	c++ -E -o clucene_dllh.h clucene_dllh1.h $(CFLAGS)

# base our perl clucene header on the dll one but use our preprocessed include header instead
clucene_dllp.h: ../dll/clucene_dll.h
	sed -e 's/"CLucene\/CLConfig.h"/"clucene_dllh.h"/' < ../dll/clucene_dll.h > clucene_dllp.h

## manually build perl interface via DLL to CLucene shared library
## normally you would use the CPAN module in FulltextSearch-CLucene-1.00.tar.gz instead

# shareable perl/C library wrapper for DLL interface
# for linux
CLuceneWrap.so: dll clucene_wrap.o
	${CC} -shared $(LIBCLUCENE) clucene_dll.o clucene_wrap.o -o CLuceneWrap.so $(LDFLAGS) $(LINKOPTS)

# or cygwin
cygwin: dll clucene_wrap.o
	${CC} ${LIBCLUCENE} $(LDFLAGS) /usr/bin/cygperl5_8_2.dll /usr/bin/cygcrypt-0.dll clucene_dll.o clucene_wrap.o -o CLuceneWrap.so

# swig perl/C wrapper to DLL interface
clucene_wrap.o: clucene.i clucene_perl.h
	@perl check.pl
	swig -perl clucene.i
	mv CLuceneWrap.pm FulltextSearch/
	${CC} -c clucene_wrap.c $(CFLAGS) -I${PERLCORE} -Dbool=char ${SWIGCFLAGS}

clean:
	rm -f t *.o config.h clucene_dllh1.h clucene_dllh.h clucene_dllp.h clucene_wrap.* CLuceneWrap.* clucene_dll.so swig.ver FulltextSearch/CLuceneWrap.pm

# run test perl program against perl wrapper
test: CLuceneWrap.so
	LD_LIBRARY_PATH=$(LIBDIR) perl clucene_test.pl

# test C wrapper to DLL interface, equivalent to perl test program but easier to debug against
t: t.o clucene_dll.o
	c++ -o t t.o clucene_dll.o $(LIBCL) $(LDFLAGS)

