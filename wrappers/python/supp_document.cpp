/* in: */

#define DECREF_LOOP_VARS() \
  /* Don't need to decref pyName (it's a borrowed reference). */ \
  Py_XDECREF(pyValue); \
  Py_XDECREF(pyIsStored); \
  Py_XDECREF(pyIsIndexed); \
  Py_XDECREF(pyIsTokenized);

lucene::document::Document *supp_convertPythonDocumentToDocument(PyObject *pyDoc) {
  /* We allocate doc and don't worry about deleting it here; the surrounding
  ** typemap handler will free it even in case of error. */
  lucene::document::Document *doc = new lucene::document::Document();

  PyObject *pyFields = PyObject_GetAttr(pyDoc, global_string__fields);
  if (pyFields == NULL || !PyDict_Check(pyFields)) {
    PyErr_SetString(PyExc_TypeError,
        "Incoming object was not instance of pyclene.lucene.Document"
      );
    goto fail;
  } else {
    int dictIterPos = 0;
    PyObject *pyName, *pyField;
    while (PyDict_Next(pyFields, &dictIterPos, &pyName, &pyField)) {
      char_t *name = NULL;
      CONVERT_STRING_IN(pyName, name);

      PyObject *pyValue = PyObject_GetAttr(pyField, global_string__value);
      char_t *value = NULL;
      if (pyValue != NULL) {
        CONVERT_STRING_IN(pyValue, value);
      }
      PyObject *pyIsStored = PyObject_GetAttr(pyField, global_string__isStored);
      int isStored = PyObject_IsTrue(pyIsStored);
      PyObject *pyIsIndexed = PyObject_GetAttr(pyField, global_string__isIndexed);
      int isIndexed = PyObject_IsTrue(pyIsIndexed);
      PyObject *pyIsTokenized = PyObject_GetAttr(pyField, global_string__isTokenized);
      int isTokenized = PyObject_IsTrue(pyIsTokenized);
      if (
          /* If the string conversion attempt failed, then pyName is not
          ** actually a Python string (same for value). */
             name == NULL
          || pyValue == NULL || value == NULL
          || pyIsStored == NULL || isStored == -1 /* PyObject_IsTrue returns -1 for error. */
          || pyIsIndexed == NULL || isIndexed == -1
          || pyIsTokenized == NULL || isTokenized == -1
        )
      {
        DECREF_LOOP_VARS();
        goto fail;
      }

      lucene::document::Field& f = *new lucene::document::Field(
          /* Added the '!= 0' to the end of the int-bool vars to silence an
          ** MSVC "performance warning" that's well intentioned but doesn't
          ** apply in this case, since we need the vars to be able to hold -1,
          ** the error return value from PyObject_IsTrue. */
          name, value, (bool)(isStored != 0), (bool)(isIndexed != 0), (bool)(isTokenized != 0)
        );

      /* We pass pointer ownership over f to doc. */
      doc->add(f);
      DECREF_LOOP_VARS();
    }
  }

  Py_DECREF(pyFields);
  return doc;
  fail:
    Py_XDECREF(pyFields);
    return NULL;
}

/* out: */
PyObject *supp_convertDocumentToPythonDocument(lucene::document::Document *doc) {
  lucene::document::DocumentFieldEnumeration *fieldEnum = NULL;
  PyObject *docFields = NULL;
  PyObject *pyDoc = PyObject_CallFunctionObjArgs(global_DocumentConstructor, Py_False, NULL);
  FAIL_IF_NULL(pyDoc);
  docFields = PyDict_New();
  FAIL_IF_NULL(docFields);

  fieldEnum = doc->fields();
  while (fieldEnum->hasMoreElements()) {
    lucene::document::Field *field = fieldEnum->nextElement();

    PyObject *pyName;
    CONVERT_STRING_OUT(field->Name(), pyName);
    if (pyName == NULL) goto fail;
    PyObject *pyValue;
    CONVERT_STRING_OUT(field->StringValue(), pyValue);
    if (pyValue == NULL) {
      Py_DECREF(pyName);
      goto fail;
    }
    PyObject *pyIsStored = PyBool_FromLong( (long) field->IsStored() );
    PyObject *pyIsIndexed = PyBool_FromLong( (long) field->IsIndexed() );
    PyObject *pyIsTokenized = PyBool_FromLong( (long) field->IsTokenized() );

    PyObject *pyField = PyObject_CallFunctionObjArgs(global_FieldConstructor,
        pyName, pyValue, pyIsStored, pyIsIndexed, pyIsTokenized,
        NULL
      );
    Py_DECREF(pyValue);
    Py_DECREF(pyIsStored);
    Py_DECREF(pyIsIndexed);
    Py_DECREF(pyIsTokenized);
    if (pyField == NULL) {
      Py_DECREF(pyName);
      goto fail;
    }

    if ( PyDict_SetItem(docFields, pyName, pyField) == -1) goto fail;
    /* PyDict_SetItem established its own references to pyName and pyField. */
    Py_DECREF(pyName);
    Py_DECREF(pyField);
  }
  /* Now we have a dict containing the fields.  Attach it to pyDoc. */
  if ( PyObject_SetAttr(pyDoc, global_string__fields, docFields) == -1) goto fail;
  Py_DECREF(docFields);

  delete fieldEnum;
  return pyDoc;
  fail:
    if (!PyErr_Occurred()) PyErr_NoMemory();
    Py_XDECREF(pyDoc);
    Py_XDECREF(docFields);
    delete fieldEnum;
    return NULL;
}
