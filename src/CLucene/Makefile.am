## Makefile.am -- Process this file with automake to produce Makefile.in
stdhdir = $(includedir)/CLucene

if USEUTF8
utf8src = $(lsrcdir)/LuceneUTF8.cpp
else
utf8src =
endif
libclucene_la_SOURCES    += $(utf8src) 
libclucene_la_SOURCES    += $(lsrcdir)/StdHeader.cpp
stdh_HEADERS =  $(lsrcdir)/*.h
stdh_HEADERS += $(top_builddir)/config.h