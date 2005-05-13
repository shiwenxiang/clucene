/* Reviewed 2004.08.20: necessary */

namespace lucene{ namespace index {
  class IndexWriter {
  public:
    lucene::index::SegmentInfos& segmentInfos;
    lucene::store::Directory& directory;
    ~IndexWriter();

    ostream* infoStream;

/* SWIG choked on these defs:
    const static int_t DEFAULT_MAX_FIELD_LENGTH = 10000;
    const static int_t FIELD_LENGTH_TRUNC_POLICY__WARN = -1;
*/
    int_t maxFieldLength;
    int_t mergeFactor;
    int_t maxMergeDocs;

    IndexWriter(const char_t* path, lucene::analysis::Analyzer& a, const bool create);
    IndexWriter(lucene::store::Directory& d, lucene::analysis::Analyzer& a, const bool create);

    void close(const bool closeDir=true);
    int_t docCount();
    void addDocument(lucene::document::Document& doc);
    void optimize();
    void addIndexes(lucene::store::Directory** dirs, const int_t dirsLength);
    void deleteSegments(lucene::util::VoidList<lucene::index::SegmentReader*> &segments);
    void deleteFiles(const StringArrayConst& files, lucene::store::Directory& directory);
    void deleteFiles(const StringArrayConst& files, StringArrayConst& deletable);
  };

  class IndexWriterLockWith : public LuceneLockWith {
  public:
    lucene::index::IndexWriter* writer;
    bool create;
    void* doBody();
    IndexWriterLockWith(lucene::store::LuceneLock* lock, lucene::index::IndexWriter* wr, bool cr);
  };

  class IndexWriterLockWith2 : public LuceneLockWith {
  public:
    lucene::util::VoidList<lucene::index::SegmentReader*>* segmentsToDelete;
    lucene::index::IndexWriter* writer;
    void* doBody();
    IndexWriterLockWith2(lucene::store::LuceneLock* lock, lucene::index::IndexWriter* wr, lucene::util::VoidList<SegmentReader*>* std);
  };

}}
