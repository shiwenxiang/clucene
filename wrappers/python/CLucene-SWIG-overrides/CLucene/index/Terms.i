/* Reviewed 2004.08.20: necessary */

namespace lucene{ namespace index {
  class TermDocs {
  public:
    virtual ~TermDocs(){ };
    virtual void seek(lucene::index::Term* term)=0;
    virtual int_t Doc() const=0;
    virtual int_t Freq() const=0;
    virtual bool next() =0;
    virtual int_t read(int_t docs[], int_t freqs[])=0;
    virtual bool skipTo(const int_t target)=0;
    virtual void close() = 0;
  };

  class TermEnum {
  public:
    virtual bool next()=0;
    virtual lucene::index::Term* getTerm(const bool pointer=true)=0;
    virtual int_t DocFreq() const=0;
    virtual void close() =0;
    virtual ~TermEnum(){ }
  };

  class TermPositions: public virtual TermDocs {
  public:
    virtual int_t nextPosition() = 0;
    virtual ~TermPositions(){ }
  };
}}
