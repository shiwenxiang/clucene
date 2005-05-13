/* Reviewed 2004.08.20: necessary */

namespace lucene{ namespace store{

  class RAMDirectory;

  class RAMLock : public virtual LuceneLock {
  public:
    RAMLock(const char_t* name, RAMDirectory* dir);
    ~RAMLock();

    bool obtain();
    void release();
  };

  class RAMDirectory : public Directory {
  public:
    void list(char_t**& list, int_t& size);

    RAMDirectory();
    ~RAMDirectory();

    bool fileExists(const char_t* name) const;
    long_t fileModified(const char_t* name) const;
    long_t fileLength(const char_t* name) const;
    void deleteFile(const char_t* name, const bool throwError = true);
    void renameFile(const char_t* from, const char_t* to);
    lucene::store::OutputStream& createFile(const char_t* name);
    lucene::store::InputStream& openFile(const char_t* name);
    void close();
    lucene::store::LuceneLock* makeLock(const char_t* name);
  };

}}
