@echo off

rem 2004.11.24:
rem Please use SWIG 1.3.22, not 1.3.23!  The latter is both more buggy than
rem 1.3.22 *and* produces slower, larger binaries.  Until further notice, DSR
rem requests that contributors refrain from checking in any SWIG-generated
rem files generated with any version other than 1.3.22.

rem Default SWIG_EXE setting assumes that swig.exe is on the system PATH:
SET SWIG_EXE=swig

rem To use a version of SWIG that's not on the system path, set the following
rem two vars:
rem SET SWIG_EXE=D:\dev\c\SWIG-1.3.24\swig.exe
rem SET SWIG_LIB=D:\dev\c\SWIG-1.3.24\Lib

%SWIG_EXE% -c++ -python -modern -I../../src -I./CLucene-SWIG-overrides -o _clucene_wrap.cpp -interface _cl_c -module _cl_py pyclene.i

rem 2005.02.05:
rem Added ad hoc post-processing step to overcome SWIG bugs:
python swig_postprocess.py