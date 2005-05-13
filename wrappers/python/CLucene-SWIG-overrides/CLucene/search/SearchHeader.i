/* Reviewed 2004.08.20: necessary */

using namespace lucene::index;
namespace lucene{ namespace search{

  class Searcher;
  class Query;
  class Hits;
  class HitDoc;

  class Hits {
  public:
    Hits(lucene::search::Searcher& s, lucene::search::Query& q, const lucene::search::Filter* f);
    ~Hits();

    int_t Length() const;
    lucene::document::Document& doc(const int_t n);
    int_t id (const int_t n);
    float_t score(const int_t n);
  };

  class Searcher {
  public:
    virtual ~Searcher(){ }

    virtual int_t docFreq(const lucene::index::Term& term)const = 0;
    virtual int_t maxDoc() const = 0;
    virtual lucene::search::TopDocs& Search(lucene::search::Query& query, const lucene::search::Filter* filter, const int_t n) = 0;

    lucene::search::Hits& search(lucene::search::Query& query);
    lucene::search::Hits& search(lucene::search::Query& query, const lucene::search::Filter* filter);

    void search(lucene::search::Query& query, lucene::search::HitCollector& results);
    virtual void Search(lucene::search::Query& query, const lucene::search::Filter* filter, lucene::search::HitCollector& results) = 0;

    virtual lucene::document::Document& doc(const int_t i) = 0;
    virtual void close() = 0;
  };

  class HitDoc {
  public:
    float_t score;
    int_t id;
    lucene::document::Document* doc;

    lucene::search::HitDoc* next;
    lucene::search::HitDoc* prev;

    HitDoc(const float_t s, const int_t i);
    ~HitDoc();
  };


  class Query {
  public:
    Query() : boost(1.0f) {}
    virtual ~Query() {}

    float_t boost;
    virtual float_t sumOfSquaredWeights(lucene::search::Searcher& searcher) = 0;
    virtual void normalize(const float_t norm) = 0;
    virtual lucene::search::Scorer* scorer(lucene::index::IndexReader& reader) = 0;
    virtual void prepare(lucene::index::IndexReader& reader) = 0;
    virtual const char_t* getQueryName() const = 0;
    bool instanceOf(const char_t* other);
    static Scorer* scorer(lucene::search::Query& query, lucene::search::Searcher& searcher, lucene::index::IndexReader& reader);
    void setBoost(float_t b);
    float_t getBoost();
    virtual const char_t* toString(const char_t* field) = 0;
  };

}}
