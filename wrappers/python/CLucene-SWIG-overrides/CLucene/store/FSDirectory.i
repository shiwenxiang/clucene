/* Reviewed 2004.08.20: necessary */

using namespace std;
namespace lucene{ namespace store{

  class FSInputStream : public InputStream {
  public:
    bool isClone;

    FSInputStream(const char_t* path);
    FSInputStream(lucene::store::FSInputStream& clone);
    ~FSInputStream();

    lucene::store::InputStream& clone();
    void close();
  };

  class FSOutputStream: public OutputStream {
  public:
    FSOutputStream(const char_t* path);
    ~FSOutputStream();

    void flushBuffer(const l_byte_t* b, const int_t size);
    void close();
    void seek(const long_t pos);
    long_t Length();
  };

  class FSDirectory : public Directory {
  public:
    FSDirectory(const char_t* path, const bool createDir);
    ~FSDirectory();

    void list(char_t**& list, int_t& size);
    bool fileExists(const char_t* name) const;
    const char_t* getDirName() const;
    static lucene::store::FSDirectory& getDirectory(const char_t* file, const bool create);
    long_t fileModified(const char_t* name) const;
    static long_t fileModified(const char_t* dir, const char_t* name);
    long_t fileLength(const char_t* name) const;
    void deleteFile(const char_t* name, const bool throwError=true);
    void renameFile(const char_t* from, const char_t* to);
    lucene::store::OutputStream& createFile(const char_t* name);
    lucene::store::InputStream& openFile(const char_t* name);
    lucene::store::LuceneLock* makeLock(const char_t* name);
    void refInc();
    void close();
  };

  class FSLock : public virtual LuceneLock {
  public:
    char_t* fname;

    FSLock(const char_t* name);
    ~FSLock();

    bool obtain();
    void release();
  };

}}
