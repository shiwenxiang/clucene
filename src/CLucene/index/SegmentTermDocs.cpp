#include "CLucene/StdHeader.h"
#include "SegmentHeader.h"

#include "CLucene/util/BitVector.h"
#include "Term.h"

namespace lucene{ namespace index {

  SegmentTermDocs::SegmentTermDocs(void* Parent){
  //Func - Constructor
  //Pre  - Paren != NULL
  //Post - The instance has been created

      CND_PRECONDITION(Parent != NULL,"Parent is NULL")

      parent      = Parent;
      deletedDocs =  ((SegmentReader*)parent)->deletedDocs;

      doc         = 0;
      freq        = 0;
      freqCount   = 0;
      freqStream  = &((SegmentReader*)parent)->freqStream->clone();
  }

  SegmentTermDocs::~SegmentTermDocs() {
  //Func - Destructor
  //Pre  - true
  //Post - The instance has been destroyed

      close();
  }


  void SegmentTermDocs::seek(Term* term) {
    TermInfo* ti = ((SegmentReader*)parent)->tis->get(*term);
    seek(ti);
    if (ti != NULL) { // 2004.11.01: SF 1057242
      delete ti;
    }
  }

  void SegmentTermDocs::seek(const TermInfo* ti) {
    if (ti == NULL) {
      freqCount = 0;
    } else {
      freqCount = ti->docFreq;
      doc = 0;
      freqStream->seek(ti->freqPointer);
    }
  }

  void SegmentTermDocs::close() {

      //Check if freqStream still exists
	  if (freqStream){
          freqStream->close();
	      }
    _DELETE( freqStream );
  }

  int_t SegmentTermDocs::Doc()const { return doc; }
  int_t SegmentTermDocs::Freq()const { return freq; }

  bool SegmentTermDocs::next() {
    while (true) {
      if (freqCount == 0)
        return false;

      int_t sdocCode = freqStream->readVInt();
      uint_t docCode = sdocCode;// shift off low bit TODO: check this... was >>>
      doc += docCode >> 1;
      if ((docCode & 1) != 0)			  // if low bit is set
        freq = 1;				  // freq is one
      else
        freq = freqStream->readVInt();		  // else read freq

      freqCount--;

      if ( (deletedDocs == NULL) || (deletedDocs->get(doc) == false ) )
        break;
      skippingDoc();
    }
    return true;
  }

  /** Optimized implementation. */
  int_t SegmentTermDocs::read(int_t docs[], int_t freqs[]) {
    int_t end = sizeof(docs)/sizeof(int_t);
    int_t i = 0;
    while (i < end && freqCount > 0) {

      // manually inlined call to next() for speed
      int_t sdocCode = freqStream->readVInt();
      uint_t docCode = sdocCode;
      doc += docCode >> 1;			  // shift off low bit TODO: check this... was >>>
      if ((docCode & 1) != 0)			  // if low bit is set
        freq = 1;				  // freq is one
      else
        freq = freqStream->readVInt();		  // else read freq
      freqCount--;

      if (deletedDocs == NULL || !deletedDocs->get(doc)) {
        docs[i] = doc;
        freqs[i] = freq;
        ++i;
      }
    }
    return i;
  }

  /** As yet unoptimized implementation. */
  bool SegmentTermDocs::skipTo(const int_t target){
    do {
      if (!next())
        return false;
    } while (target > doc);
    return true;
  }

  void SegmentTermDocs::skippingDoc() {
  }

}}
