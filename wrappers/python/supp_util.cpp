/* in: */
lucene::util::BitSet *supp_convertPythonSequenceToBitSet(PyObject *seq) {
  PyObject *pyFastSeq = PySequence_Fast(seq, "sequence required"); /* new ref */
  FAIL_IF_NULL(pyFastSeq);

  {
  const int_t nBits = PySequence_Fast_GET_SIZE(pyFastSeq);
  lucene::util::BitSet *bs = new lucene::util::BitSet(nBits);

  for (int i = 0; i < nBits; i++) {
    int isTrue = PyObject_IsTrue(PySequence_Fast_GET_ITEM(pyFastSeq, i));
    if (isTrue == 1) {
      bs->set(i);
    } else if (isTrue == -1) {
      goto fail;
    }
    /* else, isTrue == 0 and we need not act, for BitSet elements are false
    ** by default. */
  }

  Py_DECREF(pyFastSeq);
  return bs;
  }
  fail:
    if (!PyErr_Occurred()) PyErr_NoMemory();
    Py_XDECREF(pyFastSeq);
    throw PythonExceptionAlreadySet();
}

/* out: */
PyObject *supp_convertBitSetToPythonList(
    const lucene::util::BitSet *bs, const int_t bsSize
  )
{
  PyObject *pyBits = PyList_New(bsSize);
  FAIL_IF_NULL(pyBits);

  for (int i = 0; i < bsSize; i++) {
    PyList_SET_ITEM(pyBits, i, PyBool_FromLong(bs->get(i)));
  }

  return pyBits;
  fail:
    if (!PyErr_Occurred()) PyErr_NoMemory();
    Py_XDECREF(pyBits);
    return NULL;
}
