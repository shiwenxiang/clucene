/***** GENERAL/SUPPORTING FACILITIES *****/

/* LAIDBACK_PY2C_ALTERNATIVE first tries the standard SWIG input conversion
** mechanism, and if that fails, invokes an alternative constructor.  This is
** intended to facilitate that automatic conversion of Python objects into
** manually written C++ proxies, as is done for the conversion from
**   Python iterable -> lucene::analysis::TokenStream
*/
/*
%define LAIDBACK_PY2C_ALTERNATIVE(pyPtr, cPtr, cType, alternativeConstructor,
    acceptNone
  )
  #if !acceptNone
    if (pyPtr == Py_None) {
      PyErr_SetString(PyExc_TypeError, "None is not an acceptable return value.");
      throw PythonExceptionAlreadySet();
    }
  #endif
  if (-1 ==
      SWIG_ConvertPtr(
        pyPtr,
        (void **)(&cPtr),
        $descriptor(cType),
        SWIG_POINTER_EXCEPTION | 0
      )
    )
  {
    PyErr_Clear();
    // The constructor is expected to raise a C++ exception if necessary.
    $result = alternativeConstructor(pyPtr);
    // Warning: If this is used in a director method, the arguments converted
    // to Python by the SWIG directory infrastructure will not be collected if
    // the constructor above raises an exception.
    assert (!PyErr_Occurred());
  }
%enddef
*/

/* GENERATE_DUCK_TYPE_MAPS isn't currently in use because I've given up on my
** attempt to purge the Python wrapper of SWIG director classes. */
/*
%define GENERATE_DUCK_TYPE_MAPS(cType, duckAttrName, alternativeConstructor)
  %typecheck(SWIG_TYPECHECK_POINTER) cType*, cType& {
    $1 = PyObject_HasAttrString($input, duckAttrName);
  }

  %typemap(in) cType*, cType& {
    LAIDBACK_PY2C_ALTERNATIVE($input, $result, cType*,
        alternativeConstructor, false
      )
  }

  %typemap(out) cType*, cType& {
    $result = SWIG_NewPointerObj((void*)($1), $descriptor(cType*), 1)
  }
%enddef
*/


/***** STRINGS *****/
#ifndef _UNICODE
  %{
    #define CONVERT_STRING_IN(pyIn, cTarget) SWIG_AsCharPtr(pyIn, &cTarget)
    #define CONVERT_STRING_OUT(cIn, pyOut) (pyOut) = SWIG_FromCharPtr(cIn)
  %}
#else
  %{
    #define CONVERT_STRING_IN(pyIn, cTarget) SWIG_AsWCharPtr(pyIn, &cTarget)
    #define CONVERT_STRING_OUT(cIn, pyOut) (pyOut) = SWIG_FromWCharPtr(cIn)
  %}
#endif /* _UNICODE */


/***** BOOLEANS *****/
/* Booleans are not ideally supported by SWIG (1.3.21, at least). */
%typemap(python, in) bool {
  $1 = PyObject_IsTrue($input) ? true : false;
}
%typemap(python, out) bool {
  $result = PyBool_FromLong((long)$1);
}
%typemap(typecheck) bool {
   $1 = PyBool_Check($input) ? 1 : 0;
}


/***** 64-bit ints *****/
%typemap(python, in) long_t {
  if (PyInt_Check($input)) {
    $1 = (long_t) PyInt_AS_LONG($input);
  } else if (PyLong_Check($input)) {
    $1 = PyLong_AsLongLong($input);
    if (PyErr_Occurred()) { /* E.g., an OverflowError in the conversion above. */
      SWIG_fail;
    }
  } else {
    PyErr_SetString(PyExc_TypeError, "int or long required");
    SWIG_fail;
  }
}

%typemap(python, out) long_t {
  if ($1 <= LONG_MAX) {
    $result = PyInt_FromLong((long)$1);
  } else {
    $result = PyLong_FromLongLong($1);
  }
}


/***** lucene::search::ScoreDoc** *****/
%typemap(python, out) lucene::search::ScoreDoc** {
  lucene::search::TopDocs *self = arg1;
  $result = supp_convertScoreDocsArrayToTupleOfScoreDocsObjects(
    self->scoreDocs, self->scoreDocsLength
  );
}


/***** lucene::document::Document *****/
%typemap(in) lucene::document::Document*, lucene::document::Document& {
  try {
    $1 = supp_convertPythonDocumentToDocument($input);
    if ($1 == NULL) SWIG_fail;
  } catch (THROW_TYPE &e) {
    /* THROW_TYPE is defined in CL's StdHeader.h. */
    PyErr_SetString(PyExc_Exception, e.what());
    SWIG_fail;
  }
}

%typemap(freearg) lucene::document::Document*, lucene::document::Document& {
  delete $1;
}

%typemap(out) lucene::document::Document*, lucene::document::Document& {
  $result = supp_convertDocumentToPythonDocument($1);
}

%typemap(newfree) lucene::document::Document*, lucene::document::Document& {
  delete $1;
}


/***** Byte array and length *****/
/* An example of the following is OutputStream::writeBytes. */
%typemap(in) (const l_byte_t* b, const int_t length) {
  if (!PyString_Check($input)) {
    PyErr_SetString(PyExc_ValueError, "Expecting a string");
    return NULL;
  }
  PyString_AsStringAndSize($input, (char **) &$1, (int *) &$2);
}


/***** lucene::util::BitSet *****/
%typemap(python, directorout) lucene::util::BitSet* {
  $result = supp_convertPythonSequenceToBitSet($input);
}

%typemap(python, out) lucene::util::BitSet* {
  $result = supp_convertBitSetToPythonList($1, $1->getSize());
  delete $1;
}

/* lucene::index::Term (converted to/from Python list) */
%typemap(in) lucene::index::Term*, lucene::index::Term& {
  $1 = supp_convertPythonTupleToTerm($input);
  if ($1 == NULL) SWIG_fail;
}

%typemap(freearg) lucene::index::Term*, lucene::index::Term& {
  $1->finalize();
}

%typemap(out) lucene::index::Term*, lucene::index::Term& {
  $result = supp_convertTermToPythonTuple($1);
  /* Release our IGCollectable reference to $1: */
  $1->finalize();
}

/***** lucene::analysis::TokenStream *****/
%typemap(directorout) lucene::analysis::TokenStream*, lucene::analysis::TokenStream& {
  /* YYY: It's safe to decref these specific variable names because I know
  ** that this typemap is currently used in only one context, but this
  ** could cause a problem in the future. */
  %#define _FREE_THE_SWIGCREATED_DIRECTOR_ARGS()       \
      Py_XDECREF(obj0);                                \
      Py_XDECREF(obj1);                                \
      Py_XDECREF(result);

  if ($input == Py_None) {
    PyErr_SetString(PyExc_TypeError, "The token stream must not be None.");
    _FREE_THE_SWIGCREATED_DIRECTOR_ARGS();
    throw PythonExceptionAlreadySet();
  }
  if (-1 ==
      SWIG_ConvertPtr(
        $input,
        (void **)(&$result),
        $descriptor(lucene::analysis::TokenStream*),
        SWIG_POINTER_EXCEPTION | 0
      )
    )
  {
    PyErr_Clear();
    try {
      $result = new PythonTokenStream($input);
    } catch(...) {
      _FREE_THE_SWIGCREATED_DIRECTOR_ARGS();
      throw;
    }
    /* If there was an error, the alternative constructor should've raised a
    ** C++ exception rather than just setting a Python exception and
    ** returning. */
    assert (!PyErr_Occurred());
  }
}

%typemap(directorin) const char_t* {
  /* YYY:
  ** $input is the Python object, but I could find no special SWIG variable
  ** name for the C char_t pointer that was bound in the context of a
  ** Director-typemap. */
  CONVERT_STRING_OUT(fieldName, $input);
}


%typemap(directorin) lucene::util::Reader* {
  /* YYY:
  ** $input is the Python object, but I could find no special SWIG variable
  ** name for the C Reader pointer that was bound in the context of a
  ** Director-typemap. */
  const long_t nCharsAvailable_long = reader->available();
  if (nCharsAvailable_long > INT_MAX) {
    /* Python string can have length no longer than the max of C int. */
    _THROWC("Reader value too long to fit in Python string.");
  }
  const int nCharsAvailable = static_cast<int>(nCharsAvailable_long);
  $input = PyString_FromStringAndSize(NULL, nCharsAvailable);
  if ($input == NULL) {
    PyErr_NoMemory();
    throw PythonExceptionAlreadySet();
  }

  /* UNICODE: Will need to be retrofitted. */
  char* internalBuf = PyString_AS_STRING($input);
  reader->read(internalBuf, 0, nCharsAvailable);
}
