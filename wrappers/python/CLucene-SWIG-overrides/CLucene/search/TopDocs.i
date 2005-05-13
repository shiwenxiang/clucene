/* Reviewed 2004.08.20: necessary */

namespace lucene{ namespace search {
  class TopDocs {
  public:
    const int_t totalHits;
    lucene::search::ScoreDoc** scoreDocs;
    const int_t scoreDocsLength;
    bool deleteScoreDocs;

    TopDocs(const int_t th, lucene::search::ScoreDoc **sds, const int_t sdLength);
    ~TopDocs();
  };
}}
