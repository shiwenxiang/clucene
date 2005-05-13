/* Reviewed 2004.08.20: necessary */

namespace lucene{ namespace search {

  class SimpleTopDocsCollector : public HitCollector {
    SimpleTopDocsCollector(const lucene::util::BitSet* bs, lucene::search::HitQueue& hitQueue, int_t* totalhits, const int_t ndocs, const float_t minScore);

    void collect(const int_t doc, const float_t score);
  };

  class SimpleFilteredCollector : public HitCollector {
  public:
    SimpleFilteredCollector(lucene::util::BitSet* bs, lucene::search::HitCollector& collector);
  protected:
    void collect(const int_t doc, const float_t score);
  };

  class IndexSearcher : public Searcher{
  public:
    /* %name is deprecated in recent SWIG releases, but SWIG 1.3.22 seems the
    ** most stable, and if we use %rename (rather the %name) with 1.3.22, it
    ** doesn't work. */
    %name (IndexSearcher_FromString) IndexSearcher(const char_t* path);
    %name (IndexSearcher_FromIndexReader) IndexSearcher(IndexReader& r);
    ~IndexSearcher();

    void close();
    int_t docFreq(const lucene::index::Term& term) const;

    /* DSR:SWIG fails to detect that the doc method is virtual and hook it up
    ** properly to the director system unless it is textually declared "virtual"
    ** right here (not only in the superclass Searcher).
    //Document& doc(const int_t i); */
    virtual lucene::document::Document& doc(const int_t i);

    int_t maxDoc() const;

    lucene::search::TopDocs& Search(lucene::search::Query& query, const lucene::search::Filter* filter, const int_t nDocs);
    void Search(lucene::search::Query& query, const lucene::search::Filter* filter, lucene::search::HitCollector& results);
  };
}}
