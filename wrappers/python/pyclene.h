#ifndef LONG_LONG_MIN
  #ifdef LLONG_MIN
    #define LONG_LONG_MIN LLONG_MIN
    #define LONG_LONG_MAX LLONG_MAX
  #else
    #ifdef _I64_MIN
      #define LONG_LONG_MIN _I64_MIN
      #define LONG_LONG_MAX _I64_MAX
    #endif
  #endif
#endif

#define MAX_POSSIBLE_CHARS_IN_FLOAT_REPR 80

#define RETURN_PY_NONE \
  Py_INCREF(Py_None); \
  return Py_None;

#define FAIL_IF_NULL(x) if ((x) == NULL) { goto fail; }

static PyObject *global_pythonLuceneModule = NULL;
static PyObject *global_DocumentConstructor = NULL;
static PyObject *global_FieldConstructor = NULL;

/* Cached Python string objects, typically for use as attribute names in
** PyObject_[Get|Set]Attr and the like.
** These pointers are never expected to change during the life of the process
** (except perhaps during the final garbage collection at shutdown), but they
** aren't marked 'const' because the Python C API doesn't expect const
** pointers. */
static PyObject *global_string__fields = PyString_FromString("_fields");
static PyObject *global_string__value = PyString_FromString("_value");
static PyObject *global_string__isStored = PyString_FromString("_isStored");
static PyObject *global_string__isIndexed = PyString_FromString("_isIndexed");
static PyObject *global_string__isTokenized = PyString_FromString("_isTokenized");

static PyObject *global_string_this = PyString_FromString("this");
static PyObject *global_string_thisown = PyString_FromString("thisown");


void setGlobals(PyObject *luceneModule,
    PyObject *documentConstructor, PyObject *fieldConstructor
  )
{
  Py_INCREF(luceneModule);
  global_pythonLuceneModule = luceneModule;

  Py_INCREF(documentConstructor);
  global_DocumentConstructor = documentConstructor;

  Py_INCREF(fieldConstructor);
  global_FieldConstructor = fieldConstructor;
}
