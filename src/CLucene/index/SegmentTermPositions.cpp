#include "CLucene/StdHeader.h"
#include "SegmentHeader.h"

#include "Terms.h"

using namespace lucene::util;
namespace lucene{ namespace index{

  SegmentTermPositions::SegmentTermPositions(void* Parent):
      SegmentTermDocs(Parent){
  //Func - Constructor
  //Pre  - Parent != NULL
  //Post - The instance has been created

       CND_PRECONDITION(Parent != NULL,"Parent is NULL")

       proxStream = &((SegmentReader*)parent)->proxStream->clone();

       CND_CONDITION(proxStream != NULL,"proxStream is NULL")

       position  = 0;
       proxCount = 0;
  }

  SegmentTermPositions::~SegmentTermPositions() {
  //Func - Destructor
  //Pre  - true
  //Post - The intance has been closed

      close();
  }

  void SegmentTermPositions::seek(TermInfo* ti) {
    SegmentTermDocs::seek(ti);
    if (ti != NULL)
      proxStream->seek(ti->proxPointer);
    else
      proxCount = 0;
  }

  void SegmentTermPositions::close() {
  //Func - Frees the resources
  //Pre  - true
  //Post - The resources  have been freed

      SegmentTermDocs::close();
	  //Check if proxStream still exists
	  if(proxStream){
          proxStream->close();         
          _DELETE( proxStream );
	      }
  }

  int_t SegmentTermPositions::nextPosition() {
    /* DSR:CL_BUG: Should raise exception if proxCount == 0 at the
    ** beginning of this method, as in
    **   if (--proxCount == 0) throw ...;
    ** The JavaDocs for TermPositions.nextPosition declare this constraint,
    ** but CLucene doesn't enforce it. */
    proxCount--;
    return position += proxStream->readVInt();
  }

  bool SegmentTermPositions::next() {
    for (int_t f = proxCount; f > 0; f--)		  // skip unread positions
      proxStream->readVInt();

    if (SegmentTermDocs::next()) {				  // run super
      proxCount = freq;				  // note frequency
      position = 0;				  // reset position
      return true;
    }
    return false;
  }

  int_t SegmentTermPositions::read(const int_t docs[], const int_t freqs[]) {
    _THROWC( "Runtime WException");
  }

  void SegmentTermPositions::seek(Term* term){
    //todo: hack!!!
    //supposed to call base class which calls seek(terminfo) of this class
    //but base class is not calling this class (calls it's own implementation) - grrr!!!
    //SegmentTermDocs::seek(term); (used to be just this line)

    TermInfo* ti = ((SegmentReader*)parent)->tis->get(*term);
    seek(ti);
    _DELETE(ti);
  };
  int_t SegmentTermPositions::Doc() const{
    return SegmentTermDocs::Doc();
  };
  int_t SegmentTermPositions::Freq() const{
    return SegmentTermDocs::Freq();
  };

  int_t SegmentTermPositions::read(int_t docs[], int_t freqs[]){
    return SegmentTermDocs::read(docs,freqs);
  };
  bool SegmentTermPositions::skipTo(const int_t target){
    return SegmentTermDocs::skipTo(target);
  };

  void SegmentTermPositions::skippingDoc() {
    for (int_t f = freq; f > 0; f--)		  // skip all positions
      proxStream->readVInt();
  }
}}
