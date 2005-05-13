/* Reviewed 2004.08.20: necessary */

namespace lucene{ namespace index {
  class IndexReader {
  public:
    lucene::store::Directory& directory;
    virtual ~IndexReader();
    static IndexReader& open(const char_t* path, const bool closeDir=true);
    static IndexReader& open( lucene::store::Directory& directory, const bool closeDir=true);
    static long_t lastModified(const char_t* directory);
    static long_t lastModified(const lucene::store::Directory& directory);
    static bool indexExists(const char_t* directory);
    static bool indexExists(const lucene::store::Directory& directory);
    virtual int_t NumDocs() = 0;
    virtual int_t MaxDoc() const = 0;
    virtual lucene::document::Document& document(const int_t n) =0;
    virtual bool isDeleted(const int_t n) = 0;
    virtual l_byte_t* getNorms(const char_t* field) = 0;//array
    virtual lucene::index::TermEnum& getTerms() const =0;
    virtual lucene::index::TermEnum& getTerms(const lucene::index::Term* t) const = 0;
    virtual int_t docFreq(const lucene::index::Term& t) const = 0;
    lucene::index::TermDocs& termDocs(lucene::index::Term* term) const;
    lucene::index::TermPositions& termPositions(lucene::index::Term* term);
    virtual lucene::index::TermPositions& termPositions() const = 0;
    virtual lucene::index::TermDocs& termDocs() const = 0;
    void Delete(const int_t docNum);
    int_t Delete(lucene::index::Term* term);
    void close();
    static bool isLocked(const lucene::store::Directory& directory);
    static bool isLocked(const char_t* directory);
    static void unlock(lucene::store::Directory& directory);
  };

  class IndexReaderLockWith : public LuceneLockWith {
  public:
    lucene::store::Directory* directory;
    bool closeDir;
    void* doBody();
    IndexReaderLockWith(lucene::store::LuceneLock* lock, lucene::store::Directory* dir, bool cd);
  };

}}
