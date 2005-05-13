class PythonTokenStream: public lucene::analysis::TokenStream {
  private:
    PyObject *pyTokenStreamIter;

    void releasePythonObjects();

  public:
    PythonTokenStream(PyObject *pyTS);
    ~PythonTokenStream();

    lucene::analysis::Token* next();
    void close();
};
