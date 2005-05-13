@echo off

rem Use a recent version of MinGW, including the mingw-utils and binutils
rem packages.
rem
rem As of Nov 2004, pyclene has only been tested with MinGW-GCC 3.4.2.

python setup.py build --compiler=mingw32 2> nodist_err.txt
