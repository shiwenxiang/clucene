/* in: */
lucene::index::Term *supp_convertPythonTupleToTerm(PyObject *pyTerm) {
  if (!PyTuple_Check(pyTerm) || PyTuple_GET_SIZE(pyTerm) != 2) {
    PyErr_SetString(PyExc_TypeError, "Term must be 2-tuple of strings");
    return NULL;
  }

  PyObject *pyField = PyTuple_GET_ITEM(pyTerm, 0);
  PyObject *pyText = PyTuple_GET_ITEM(pyTerm, 1);

  #ifdef _UNICODE
    if (!PyUnicode_Check(pyField)) {
      PyErr_SetString(PyExc_TypeError, "Term's field name (first element) must be unicode");
      return NULL;
    }
    if (!PyUnicode_Check(pyText)) {
      PyErr_SetString(PyExc_TypeError, "Term's text (second element) must be unicode");
      return NULL;
    }
  #else
    if (!PyString_Check(pyField)) {
      PyErr_SetString(PyExc_TypeError, "Term's field name (first element) must be str");
      return NULL;
    }
    if (!PyString_Check(pyText)) {
      PyErr_SetString(PyExc_TypeError, "Term's text (second element) must be str");
      return NULL;
    }
  #endif

  char_t *field = NULL;
  CONVERT_STRING_IN(pyField, field);
  char_t *text = NULL;
  CONVERT_STRING_IN(pyText, text);

  lucene::index::Term *term = new lucene::index::Term(field, text);
  return term;
}

/* out: */
PyObject *supp_convertTermToPythonTuple(const lucene::index::Term *term) {
  if (term == NULL) {
    /* term will be NULL (e.g.) when a TermEnum has been exhausted but its
    ** next() method is called again. */
    RETURN_PY_NONE;
  }

  PyObject *pyTerm = PyTuple_New(2);
  FAIL_IF_NULL(pyTerm);

  PyObject *pyField;
    CONVERT_STRING_OUT(term->Field(), pyField);
  PyObject *pyText;
    CONVERT_STRING_OUT(term->Text(), pyText);

  PyTuple_SET_ITEM(pyTerm, 0, pyField);
  PyTuple_SET_ITEM(pyTerm, 1, pyText);

  return pyTerm;
  fail:
    if (!PyErr_Occurred()) PyErr_NoMemory();
    Py_XDECREF(pyTerm);
    return NULL;
}
