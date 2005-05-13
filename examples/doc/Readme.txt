CLucene
-------

CLucene is a port of Lucene: the high-performance,
full-featured text search engine written in Java.
See http://jakarta.apache.org/lucene for more info on Lucene.
*** Also see the todo.txt file for more information ***

The latest version
------------------
Details of the latest version can be found on the CLucene sourceforge project web site
http://www.sourceforge.net/projects/clucene


Documentation
-------------
See http://jakarta.apache.org/lucene for additional information/links on Lucene.


Build Procedure (win32)
-----------------------
Note: for linux build procedure see 'INSTALL'
1. Build the library (from the build directory)
	Make sure that compiler type is specified (COMPILER_MSVC,COMPILER_GCC, etc)
	If Unicode is required specify _UNICODE (recommended)
2. If you want to run the .NET search demo, then compile the .Net wrapper (wrapper/CluceneDotNet)
3. Compile one of the demos. Or write your own code.
Notes:
	* Make sure you use the same char type as the library you compiled (_UNICODE or not)
	* If you're using MSVC make sure that you use the same type of Runtime Library for the library as your program


Troubleshooting
---------------
* It is recommended that developers use unicode support. Do this by compiling using the _UNICODE directive. Most problems arise when developers don't compile with _UNICODE.


Examples
--------
There are several examples in the examples folder. This also includes a demo
(index/search/delete) and the tests that come with lucene(incomplete). There is
also an example of how to use the .NET wrapper.


CROSSPLATFORM NOTES
-------------------
CLucene started off in MSVC, but now works on Linux too! See the /docs/INSTALL file for
instructions about how to compile using make scripts.


Thanks and Acknowledgements
---------------------------
* The Lucene team on the Jakarta Project (http://jakarta.apache.org/lucene)
  for their original Lucene engine.
* JavaCC is not used in CLucene, but I used the nlucene code as a reference
  for translating this (http://sourceforge.net/projects/nlucene
* Jimmy Pritts <jpritts@sdf.lonestar.org> for the make scripts.
