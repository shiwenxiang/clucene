PyObject *supp_convertScoreDocsArrayToTupleOfScoreDocsObjects(
    lucene::search::ScoreDoc **scoreDocs, int_t scoreDocsLength
  )
{
  PyObject *pySDClass = PyObject_GetAttrString(global_pythonLuceneModule, "ScoreDoc");
  if (pySDClass == NULL) {
    PyErr_SetString(PyExc_NameError, "Can't find class pyclene.lucene.ScoreDoc");
    return NULL;
  }

  PyObject *docs = PyTuple_New(scoreDocsLength);
  FAIL_IF_NULL(docs);
  for (int_t i = 0; i < scoreDocsLength; i++) {
    lucene::search::ScoreDoc *sd = scoreDocs[i];
    PyObject *d = PyObject_CallFunction(pySDClass, "(ld)", sd->doc, sd->score);
    if (d == NULL) {
      goto fail;
    }
    PyTuple_SET_ITEM(docs, i, d);
  }

  return docs;
  fail:
    if (!PyErr_Occurred()) PyErr_NoMemory();
    Py_XDECREF(pySDClass);
    Py_XDECREF(docs);
    return NULL;
}
