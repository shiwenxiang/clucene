#include "CLucene/StdHeader.h"
#include "DateFilter.h"

using namespace lucene::index;
using namespace lucene::util;
using namespace lucene::document;
namespace lucene{ namespace search{

  DateFilter::~DateFilter(){
    delete[] field;
    delete[] start;
    delete[] end;
  }

  /** Constructs a filter for field <code>f</code> matching times between
    <code>from</code> and <code>to</code>. */
  DateFilter::DateFilter(const char_t* f, long_t from, long_t to):
    field( stringDuplicate(f) ),
    start ( DateField::timeToString(from) ),
    end ( DateField::timeToString(to) )
  {
  }

  /** Constructs a filter for field <code>f</code> matching times before
    <code>time</code>. */
  DateFilter* DateFilter::Before(const char_t* field, long_t time) {
    return new DateFilter(field, 0,time);
  }

  /** Constructs a filter for field <code>f</code> matching times after
    <code>time</code>. */
  DateFilter* DateFilter::After(const char_t* field, long_t time) {
    return new DateFilter(field,time, DateField::MAX_TIME );
  }

  /** Returns a BitSet with true for documents which should be permitted in
    search results, and false for those that should not. */
  BitSet* DateFilter::bits(IndexReader& reader) const {
    BitSet& bts = *new BitSet(reader.MaxDoc());

    Term* t = new Term(field, start);
    TermEnum& _enum = reader.getTerms(t);
    t->finalize();

    if (_enum.getTerm(false) == NULL){
      delete &_enum;
      return &bts;
    }
    /* DSR:CL_BUG_LEAK: Moved the creation of termDocs from before the above
    ** block to after it in order to avoid leak. */
    TermDocs& termDocs = reader.termDocs();

    _TRY {
      Term* stop = new Term(field, end);
      while (_enum.getTerm(false)->compareTo(*stop) <= 0) {
        termDocs.seek(_enum.getTerm(false));
/*DSR:CL_BUG(1):        _TRY {*/
        while (termDocs.next()) {
          bts.set(termDocs.Doc());
        }
/* DSR:CL_BUG(1):
** In CLucene 0.8.11, termDocs was closed by the finally block below, but the
** outer while loop can come back for another pass without ever reopening
** termDocs.  In that situation, when the termDocs.seek call above is made,
** the program crashes because termDocs's underlying input stream has been
** closed.
**      } _FINALLY ( termDocs.close() );*/

        if (!_enum.next()) {
          break;
        }
      }
      stop->finalize();
    } _FINALLY (
/* DSR:CL_BUG(1): termDocs.close call moved here from try/finally block above. */
        termDocs.close();
        delete &termDocs;
        _enum.close();
        delete &_enum;
    );
    return &bts;
  }
}}
