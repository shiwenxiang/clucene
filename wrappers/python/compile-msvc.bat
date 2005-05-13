@echo off

rem CLucene does not support MSVC 6 or earlier, but the only version of the
rem standard Python.org distribution of Python that's compiled with a
rem sufficiently recent version of MSVC is Python 2.4, which is compiled with
rem MSVC 7.1.  (Python 2.5 and later presumably will also use an adequate
rem version of MSVC.)
rem
rem Programmers who lack MSVC 7.1+ can either use MinGW (see compile-mingw.bat)
rem or the free command-line version of MSVC 7.1.  The free command-line
rem version does not include some files required to build pyclene, but the
rem missing files are either available with other free MS development kits or
rem can be generated from the corresponding DLLs.
rem The setup process for the free command-line version of MSVC 7.1 is arduous,
rem so unless you really need the ~12% performance advantage over MinGW-GCC
rem 3.4.x, use MinGW instead.  If you do choose to use the free MSVC 7.1, start
rem with these documents:
rem   1.  http://www.vrplumber.com/programming/mstoolkit/
rem  [To enable the successful linking of C++ STL-based code (which pyclene
rem   certainly is), you may need these instructions:]
rem   2. http://aspn.activestate.com/ASPN/Mail/Message/Python-win32/2206851
rem   3. http://aspn.activestate.com/ASPN/Mail/Message/Python-win32/2207376

python setup.py build > nodist_err.txt
