/* I originally used directors to expose the C++ TokenStream class and its
** subclasses in a Python-customizable manner, but became so discouraged by
** SWIG bugs that I reverted to writing a 'has-a' TokenStream C++ subclass that
** simply holds a reference to a Python object and delegates method calls to
** it. */

PythonTokenStream::PythonTokenStream(PyObject *pyTS) {
  /* The iterator object returned by PyObject_GetIter will retain a reference
  ** to pyTS, so we don't need to. */
  pyTokenStreamIter = PyObject_GetIter(pyTS);
  if (pyTokenStreamIter == NULL) {
    /* pyTS wasn't iterable. */
    throw PythonExceptionAlreadySet();
  }
}

PythonTokenStream::~PythonTokenStream() {
  releasePythonObjects();
}

void PythonTokenStream::releasePythonObjects() {
  if (pyTokenStreamIter != NULL) {
    Py_DECREF(pyTokenStreamIter);
    pyTokenStreamIter = NULL;
  }
}

lucene::analysis::Token* PythonTokenStream::next() {
  lucene::analysis::Token* cppToken = NULL;
  PyObject *pyToken = PyIter_Next(pyTokenStreamIter);
  _TRY {
    if (pyToken == NULL) {
      /* The NULL return value signals an error if no Python exception is set;
      ** otherwise, it signals the end of the iterator. */
      if (!PyErr_Occurred()) {
        return NULL;
      } else {
        throw PythonExceptionAlreadySet();
      }
    }

    /* Extract the C++ Token companion object from the Python Token proxy. */
    if (   pyToken == Py_None
        || -1 == SWIG_ConvertPtr(
          pyToken,
          (void **)(&cppToken),
          SWIGTYPE_p_lucene__analysis__Token,
          SWIG_POINTER_EXCEPTION | 0
        )
      )
    {
      /* If the conversion failed because the C++ Token companion object was
      ** already divorced from this same Python Token proxy object in a past
      ** call to this method, raise an informative exception rather than
      ** leaving the client programmer scratching his head.
      ** We can tell that this is the case by checking for:
      **       (not hasattr(pyToken, 'this'))
      **   and (getattr(pyToken, 'thisown') == False)
      */
      if (   pyToken != Py_None
          && (   !PyObject_HasAttr(pyToken, global_string_this)
              &&  PyObject_HasAttr(pyToken, global_string_thisown)
             )
        )
      {
        PyObject *thisown = PyObject_GetAttr(pyToken, global_string_thisown);
        if (thisown == NULL) {
          /* Don't let an AttributeError trickle down; it's not the original
          ** problem. */
          PyErr_Clear();
        } else {
          if (thisown == Py_False) {
            PyErr_SetString(PyExc_RuntimeError, "Due to the gory details of"
                " CLucene/Python cooperative memory management, this Python"
                " Token proxy's C++ companion has already been deleted.  You"
                " cannot return the same lucene.Token object multiple times"
                " from a TokenStream.  If you need to continue to use a Token"
                " object after it has been returned from a TokenStream, make a"
                " copy and return the copy instead."
              );
          }
          Py_DECREF(thisown);
        }
      }

      if (!PyErr_Occurred()) {
        /* If the conversion didn't fail for a specific reason detectable
        ** above, then it failed because of a TypeError, but SWIG's error
        ** message is uninformative, so we create our own. */
        PyObject *type = PyObject_Type(pyToken);
        if (type != NULL) {
          PyObject *typeStr = PyObject_Str(type);
          Py_DECREF(type);
          if (typeStr != NULL) {
            PyObject *errMsg = PyString_FromFormat("Value extracted from token"
                " stream must be of type lucene.Token, not %s.",
                PyString_AS_STRING(typeStr)
              );
            Py_DECREF(typeStr);
            if (errMsg != NULL) {
              PyErr_SetObject(PyExc_TypeError, errMsg);
              Py_DECREF(errMsg);
            }
          }
        }
      }

      throw PythonExceptionAlreadySet();
    }

    /* The C++ method TokenStream.next is documented as returning a new object,
    ** so any properly written C++ consumer of Tokens within CLucene's
    ** internals will delete the C++ Token object explicitly.
    ** Since the C++ Token companion object extracted from its Python proxy
    ** *will* be deleted, we have two choices as to how to prevent memory
    ** corruption if the Python proxy object later tries to access its C++
    ** companion:
    **   a) Copy the C++ Token object here, return the copy to CLucene's
    **      internals, and leave the Python proxy in charge of the original C++
    **      Token object.
    **   b) Forcibly sever the relationship between the Python proxy and its
    **      C++ companion so that the Python proxy can no longer access the
    **      companion.
    ** Option b was chosen for the sake of efficiency.  Most custom Python
    ** TokenStreams won't need to access the C++ companion Token after
    ** returning it from .next; if Python does need access, the custom
    ** TokenStream class can simply make a copy of the Token before returning
    ** it from .next. */

    /* Next call is equivalent to Python code 'pyToken.thisown = False': */
    const int disownmentStatus = PyObject_SetAttr(pyToken,
        global_string_thisown, Py_False
      );
    if (disownmentStatus == -1) {
      throw PythonExceptionAlreadySet();
    }
    /* Next call is equivalent to Python code 'del pyToken.this': */
    const int severStatus = PyObject_DelAttr(pyToken, global_string_this);
    if (severStatus == -1) {
      throw PythonExceptionAlreadySet();
    }
  } _FINALLY (
    Py_XDECREF(pyToken);
  );

  return cppToken;
}

void PythonTokenStream::close() {
  releasePythonObjects();
}
