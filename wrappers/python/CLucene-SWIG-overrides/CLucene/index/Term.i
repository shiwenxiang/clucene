/* Reviewed 2004.08.20: necessary */

namespace lucene{ namespace index{
/* Life is much easier if SWIG doesn't know about IGCollectable: */
/*class Term: public lucene::util::IGCollectable<Term> {*/
  class Term {

  public:
    Term(const char_t* fld, const char_t* txt);
    Term(char_t* fld, char_t* txt, const bool intern, const bool canDelete=true);
    ~Term();

    const char_t* lucene::document::Field() const;
    const char_t* Text() const;
    bool equals(const lucene::index::Term* o) const;
    void set(char_t* fld, char_t* txt, const bool intern);
    int_t hashCode();
    int_t compareTo(const lucene::index::Term& other) const;
    char_t* toString() const;
  };
}}
