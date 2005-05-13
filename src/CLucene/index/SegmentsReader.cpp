#include "CLucene/StdHeader.h"
#include "SegmentsReader.h"

#include "SegmentInfo.h"
#include "IndexReader.h"
#include "CLucene/document/Document.h"
#include "Terms.h"
#include "SegmentMergeQueue.h"

namespace lucene{ namespace index{

  SegmentsTermDocs::SegmentsTermDocs(){
  //Func - Default constructor
  //       Initialises an empty SegmentsTermDocs.
  //       This constructor is needed to allow the constructor of SegmentsTermPositions
  //       initialise the instance by itself
  //Pre  - true
  //Post - An empty

      readers       = NULL;
      readersLength = 0;
      starts        = NULL;
      base          = 0;
      pointer       = 0;
    current       = NULL;
    term          = NULL;
      segTermDocs   = NULL;
  }

  SegmentsTermDocs::SegmentsTermDocs(SegmentReader** r, int_t rLen, const int_t* s){
  //Func - Constructor
  //Pre  - if r is NULL then rLen must be 0 else if r != NULL then rLen > 0
  //       s != NULL
  //Post - The instance has been created

      CND_PRECONDITION((r == NULL && rLen == 0)|| (r!= NULL && rLen > 0) ,"r or rLen is invalid to the precondition")
      CND_PRECONDITION(s != NULL,"s is NULL")

      readers       = r;
      readersLength = rLen;
      starts        = s;
      base          = 0;
      pointer       = 0;
    current       = NULL;
    term          = NULL;

      segTermDocs   = NULL;

    //Check if there are readers
    if(readers != NULL && rLen > 0){
          segTermDocs = new SegmentTermDocs*[rLen];

        CND_CONDITION(segTermDocs != NULL,"No memory could be allocated for segTermDocs")

          //Initialize the segTermDocs pointer array
          for ( int_t i=0;i<rLen;i++){
              segTermDocs[i]=NULL;
              }
          }
  }

  SegmentsTermDocs::~SegmentsTermDocs(){
  //Func - Destructor
  //Pre  - true
  //Post - The instance has been destroyed

      close();
  }

  int_t SegmentsTermDocs::Doc() const {
    return base + current->doc;
  }
  int_t SegmentsTermDocs::Freq() const {
    return current->freq;
  }

  void SegmentsTermDocs::seek( Term* tterm) {
  //Func - Resets the instance for a new search
  //Pre  - tterm != NULL
  //Post - The instance has been reset for a new search

    CND_PRECONDITION(tterm != NULL,"tterm is NULL")

      //Assigning tterm is done as below for a reason
      //The construction ensures that if seek is called from within
    //SegmentsTermDocs with as argument this->term (seek(this->term)) that the assignment
    //will succeed and all referencecounters represent the correct situation

    //Get a pointer from tterm and increase its reference counter
      Term *TempTerm = tterm->pointer();

      //Finialize term to ensure we decrease the reference counter of the instance which term points to
      term->finalize();

    //Assign TempTerm to term
      term = TempTerm;

      base = 0;
      pointer = 0;
      current = NULL;
  }

  bool SegmentsTermDocs::next() {
    if (current != NULL && current->next()) {
      return true;
    } else if (pointer < readersLength) {
      base = starts[pointer];
      current = termDocs(pointer++);
      return next();
    } else
      return false;
  }

  /** Optimized implementation. */
  int_t SegmentsTermDocs::read(int_t docs[], int_t freqs[]) {
    while (true) {
      while (current == NULL) {
        if (pointer < readersLength) {		  // try next segment
          base = starts[pointer];
          current = termDocs(pointer++);
        } else {
          return 0;
        }
      }
      int_t end = current->read(docs, freqs);
      if (end == 0) {				  // none left in segment
        current = NULL;
      } else {					  // got some
        int_t b = base;			  // adjust doc numbers
        for (int_t i = 0; i < end; i++)
          docs[i] += b;
        return end;
      }
    }
  }

  /** As yet unoptimized implementation. */
  bool SegmentsTermDocs::skipTo(const int_t target) {
    do {
      if (!next())
        return false;
    } while (target > Doc());
    return true;
  }

  void SegmentsTermDocs::close() {
  //Func - Closes all SegmentsTermDocs managed by this instance
  //Pre  - true
  //Post - All the SegmentsTermDocs have been closed

      SegmentTermDocs* curSeg = NULL;

      //Check if segTermDocs is valid
    if (segTermDocs){
          //iterate through the segTermDocs array
          for (int_t i = 0; i < readersLength; i++) {
          //Retrieve the i-th SegmentTermDocs instance
              curSeg = segTermDocs[i];

          //Check if it is a valid pointer
          if (curSeg != NULL) {
            //Close it
                  curSeg->close();
                  //Upon deletion, no need to set segTermDocs[i] = NULL; the entire
                  //segTermDocs array is deleted immediately below, so there's no chance
                  //that a member will be referenced.
                 _DELETE(curSeg);
                 }
              }

      _DELETE_ARRAY(segTermDocs);
        }

      //current previously pointed to a member of segTermDocs; ensure that
      //it doesn't now point to invalid memory.
      current = NULL;
      base          = 0;
      pointer       = 0;

    term->finalize();
    term          = NULL;
  }

  SegmentTermDocs* SegmentsTermDocs::termDocs(const SegmentReader& reader) const {
    SegmentTermDocs* ret = dynamic_cast<SegmentTermDocs *>(&reader.termDocs());
    CND_CONDITION(ret != NULL,
        "Dynamic downcast in SegmentsTermDocs::termDocs from TermDocs to"
        " SegmentTermDocs failed."
      )
    return ret;
  }

  SegmentTermDocs* SegmentsTermDocs::termDocs(const int_t i) const {
    if (term == NULL)
      return NULL;
    SegmentTermDocs* result = segTermDocs[i];
    if (result == NULL){
      segTermDocs[i] = termDocs(*readers[i]);
      result = segTermDocs[i];
    }
    result->seek(term);

    return result;
  }

  SegmentsTermEnum::SegmentsTermEnum(
    SegmentReader** readers, const int_t readersLength, const int_t *starts, const Term* t){
  //Func - Constructor
  //       Opens all enumerations of all readers
  //Pre  - readers != NULL and contains an array of SegmentReader instances each responsible for
  //       reading a single segment
  //       readersLength >= 0 and represents the number of readers in the readers array
  //       starts is an array of
  //Post - An instance of has been created

  //Pre  - if readers is NULL then readersLength must be 0 else if readers != NULL then readersLength > 0
  //       s != NULL
  //Post - The instance has been created

      CND_PRECONDITION((readers == NULL && readersLength == 0)|| (readers != NULL && readersLength > 0) ,
      "readers or readersLength  is invalid to the precondition")
      CND_PRECONDITION(starts != NULL,"starts is NULL")

    //Temporary variables
      SegmentReader*   reader    = NULL;
    SegmentTermEnum* termEnum  = NULL;
    SegmentMergeInfo* smi      = NULL;
      queue                      = new SegmentMergeQueue(readersLength);

    CND_CONDITION (queue != NULL,"Could not allocate memory for queue")

    //iterate through all the readers
    for (int_t i = 0; i < readersLength; i++) {
        //Get the i-th reader
        reader = readers[i];

        //Check if the enumeration must start from term t
        if (t != NULL) {
        //termEnum is an enumeration of terms starting at or after the named term t
            termEnum = (SegmentTermEnum*)&reader->getTerms(t);
            }
        else{
            //termEnum is an enumeration of all the Terms and TermInfos in the set.
            termEnum = (SegmentTermEnum*)&reader->getTerms();
            }

          //Instantiate an new SegmentMerginfo
          smi = new SegmentMergeInfo(starts[i], *termEnum, *reader);

          // Note that in the call termEnum->getTerm(false) below false is required because
      // otherwise a reference is leaked. By passing false getTerm is
          // ordered to return an unowned reference instead. (Credits for DSR)

      if (t == NULL ? smi->next() : termEnum->getTerm(false) != NULL){
       // initialize queue
              queue->put(smi);
              }
          else{
        //Close the SegmentMergeInfo
          smi->close();
        //And have it deleted
              _DELETE(smi);
            }
          }

    //Check if the queue has elements
    if (queue->Size() > 0) {
        //Get the top element of the queue
        SegmentMergeInfo* top = queue->top();
        //Get the current term of enumeration of the top SegmentMergeInfo (
        //The reference counter will be increased)
        term = top->termEnum.getTerm();
        //Get the document Frequence of the  current Term of the enumeration of the top SegmentMergeInfo
        docFreq = top->termEnum.DocFreq();
        }
    else{
    //No current term
        term = NULL;
    //So no document frequency
        docFreq = 0;
        }
  }

  SegmentsTermEnum::~SegmentsTermEnum(){
  //Func - Destructor
  //Pre  - true
  //Post - All the resource have been freed and the instance has been deleted

    //Close the enumeration
    close();

    //Delete the queue
    _DELETE(queue);
  }

  bool SegmentsTermEnum::next(){
  //Func - Move the current term to the next in the set of enumerations
  //Pre  - true
  //Post - Returns true if term has been moved to the next in the set of enumerations
  //       Returns false if this was not possible

      SegmentMergeInfo* top = queue->top();
      if (top == NULL) {
          term->finalize(); 
          term = NULL;
          return false;
          }

      //The getTerm method requires the client programmer to indicate whether he
      // owns the returned reference, so we can discard ours
      // right away.
      term->finalize(); 

	  //Assign term the term of top and make sure the reference counter is increased
      term = top->term->pointer();
      docFreq = 0;

	  //Find the next term
      while (top != NULL && term->compareTo(*top->term) == 0) {
		  //don't delete, this is the top
          queue->pop(); 
		  // increment freq
          docFreq += top->termEnum.DocFreq();	  
		  if (top->next()){
			  // restore queue
              queue->put(top);				  
		      }
          else{
			  // done with a segment
              top->close();				  
		      _DELETE(top);
              }
          top = queue->top();
          }

      return true;
  }



  Term* SegmentsTermEnum::getTerm(const bool pointer) {
  //Func - Returns the current term of the set of enumerations
  //Pre  - pointer is true or false and indicates if the reference counter
  //       of term must be increased or not
  //       next() must have been called once!
  //Post - pointer = true -> term has been returned with an increased reference counter
  //       pointer = false -> term has been returned

    if (pointer && term!=NULL){
          return term->pointer();
        }
    else{
          return term;
          }
  }

  int_t SegmentsTermEnum::DocFreq() const {
  //Func - Returns the document frequency of the current term in the set
  //Pre  - termInfo != NULL
  //       next() must have been called once
  //Post  - The document frequency of the current enumerated term has been returned

      return docFreq;
  }


  void SegmentsTermEnum::close() {
  //Func - Closes the set of enumerations in the queue
  //Pre  - queue holds a valid reference to a SegmentMergeQueue
  //Post - The queue has been closed all SegmentMergeInfo instance have been deleted by
  //       the closing of the queue
  //       term has been finalized and reset to NULL

      // Needed when this enumeration hasn't actually been exhausted yet
      term->finalize();
    term = NULL;

    //Close the queue This will destroy all SegmentMergeInfo instances!
      queue->close();

  }

  SegmentsTermPositions::SegmentsTermPositions(SegmentReader** r, const int_t rLen, const int_t* s){
  //Func - Constructor
  //Pre  - if r is NULL then rLen must be 0 else if r != NULL then rLen > 0
  //       s != NULL
  //Post - The instance has been created

      CND_PRECONDITION((r == NULL && rLen == 0)|| (r!= NULL && rLen > 0) ,"r or rLen is invalid to the precondition")
      CND_PRECONDITION(s != NULL,"s is NULL")

      readers       = r;
      readersLength = rLen;
      starts        = s;
      base          = 0;
      pointer       = 0;
    current       = NULL;
    term          = NULL;

      segTermDocs   = NULL;

    //Check if there are readers
    if(readers != NULL && rLen > 0){
          segTermDocs = (SegmentTermDocs**) new SegmentTermPositions*[rLen];

        CND_CONDITION(segTermDocs != NULL,"No memory could be allocated for segTermDocs")

          //Initialize the segTermDocs pointer array
          for ( int_t i=0;i<rLen;i++){
              segTermDocs[i]=NULL;
              }
          }
  }

  SegmentTermDocs* SegmentsTermPositions::termDocs(const SegmentReader& reader) const {
    // Here in the SegmentsTermPositions class, we want this->current to always
    // be a SegmentTermPositions rather than merely a SegmentTermDocs.
    // To that end, we override the termDocs(SegmentReader&) method to produce
    // a SegmentTermPositions via the underlying reader's termPositions method
    // rather merely producing a SegmentTermDocs via the reader's termDocs
    // method.
    SegmentTermDocs* ret = dynamic_cast<SegmentTermPositions *>(&reader.termPositions());
    CND_CONDITION(ret != NULL,
        "Dynamic downcast in SegmentsTermPositions::termDocs from"
        " SegmentTermDocs to SegmentTermPositions failed."
      )
    return ret;
  }

  int_t SegmentsTermPositions::nextPosition() {
  //Func -
  //Pre  - current != NULL
  //Post -
    CND_PRECONDITION(current != NULL,"current is NULL")

    SegmentTermPositions *curAsTP = dynamic_cast<SegmentTermPositions *>(current);
    CND_CONDITION(curAsTP != NULL,
        "Dynamic downcast in SegmentsTermPositions::nextPosition from"
        " SegmentTermDocs to SegmentTermPositions failed."
      )
    return curAsTP->nextPosition();
  }

  void SegmentsTermPositions::seek(Term* term){
    SegmentsTermDocs::seek(term);
  };
  int_t SegmentsTermPositions::Doc() const{
    return SegmentsTermDocs::Doc();
  };
  int_t SegmentsTermPositions::Freq() const{
    return SegmentsTermDocs::Freq();
  };

  int_t SegmentsTermPositions::read(int_t docs[], int_t freqs[]){
    return SegmentsTermDocs::read(docs,freqs);
  };
  bool SegmentsTermPositions::skipTo(const int_t target){
    return SegmentsTermDocs::skipTo(target);
  };
  bool SegmentsTermPositions::next(){
    return SegmentsTermDocs::next();
  }
  void SegmentsTermPositions::close(){
    SegmentsTermDocs::close();
  }

  SegmentsReader::SegmentsReader(Directory& directory, SegmentReader** r, int_t rLen):
    IndexReader(directory), normsCache(true, DELETE_TYPE_DELETE_ARRAY, true, DELETE_TYPE_DELETE_ARRAY){
  //Func - Constructor
  //Pre  - directory contains a valid reference to a Directory
  //       if r is NULL then rLen must be 0 else if r != NULL then rLen > 0
  //       s != NULL
  //Post - The instance has been created

      CND_PRECONDITION((r == NULL && rLen == 0)|| (r!= NULL && rLen > 0) ,"r or rLen is invalid to the precondition")

      //If the the number of segments is 0 (rLen is 0) then the SegmentsReader becomes responsible
      //for closing the directory
      closeDir = (rLen == 0 ? true:false);

      readers       = r;
      readersLength = rLen;
      maxDoc        = 0;
      numDocs       = -1;

      //build starts array
      starts = new int_t[readersLength + 1];
      for (int_t i = 0; i < readersLength; i++) {
          starts[i] = maxDoc;
          //compute maxDocs
          maxDoc += readers[i]->MaxDoc();
          }

      //Store the computed maxDocs in the last element of starts
      starts[readersLength] = maxDoc;
  }

  SegmentsReader::~SegmentsReader() {
  //Func - Destructor
  //Pre  - true
  //Post - The instance has been destroyed all SegmentReader instances
  //       this instance managed have been destroyed to

    _DELETE_ARRAY(starts);

    //Iterate through the readers and destroy each reader
      if (readers && readersLength > 0) {
          for (int i = 0; i < readersLength; i++) {
              _DELETE(readers[i]);
              }
          }

    //Destroy the readers array
     _DELETE_ARRAY(readers);
  }

  int_t SegmentsReader::NumDocs() {
      LOCK_MUTEX(NumDocs_LOCK);
    if (numDocs == -1) {			  // check cache
      int_t n = 0;				  // cache miss--recompute
      for (int_t i = 0; i < readersLength; i++)
        n += readers[i]->NumDocs();		  // sum from readers
      numDocs = n;
    }
    UNLOCK_MUTEX(NumDocs_LOCK);
    return numDocs;
  }

  int_t SegmentsReader::MaxDoc() const {
    return maxDoc;
  }

  lucene::document::Document& SegmentsReader::document(const int_t n) {
    int_t i = readerIndex(n);			  // find segment num
    return readers[i]->document(n - starts[i]);	  // dispatch to segment reader
  }

  bool SegmentsReader::isDeleted(const int_t n) {
    int_t i = readerIndex(n);			  // find segment num
    return readers[i]->isDeleted(n - starts[i]);	  // dispatch to segment reader
  }

  l_byte_t* SegmentsReader::getNorms(const char_t* field){
    l_byte_t* bytes;
  LOCK_MUTEX(getNorms_LOCK);
    bytes = normsCache.get(field);
    if (bytes != NULL){
      //UNLOCK_MUTEX(getNorms_LOCK);
      return bytes;				  // cache hit
    }

    bytes = new l_byte_t[MaxDoc()];
    for (int_t i = 0; i < readersLength; i++)
      readers[i]->getNorms(field, bytes, starts[i]);

    //Unfortunately the data in the normCache can get corrupted, since it's being loaded with string
    //keys that may be deleted while still in use by the map. To prevent this field is duplicated
  //and then stored in the normCache
  char_t* key = stringDuplicate(field);
    //update cache
    normsCache.put(key, bytes);

    UNLOCK_MUTEX(getNorms_LOCK);
    return bytes;
  }

  TermEnum& SegmentsReader::getTerms() const {
    return *new SegmentsTermEnum(readers, readersLength, starts, NULL);
  }

  TermEnum& SegmentsReader::getTerms(const Term* term) const {
    return *new SegmentsTermEnum(readers, readersLength, starts, term);
  }

  int_t SegmentsReader::docFreq(const Term& t) const {
    int_t total = 0;				  // sum freqs in segments
    for (int_t i = 0; i < readersLength; i++)
      total += readers[i]->docFreq(t);
    return total;
  }

  TermDocs& SegmentsReader::termDocs() const {
    TermDocs* ret =  (TermDocs*)new SegmentsTermDocs(readers,readersLength, starts);
    return *ret;
  }

  TermPositions& SegmentsReader::termPositions() const {
    TermPositions* ret = (TermPositions*)new SegmentsTermPositions(readers,readersLength, starts);
    return *ret;
  }

  void SegmentsReader::doDelete(const int_t n) {
    LOCK_MUTEX(doDelete_LOCK);
    numDocs = -1;				  // invalidate cache
    int_t i = readerIndex(n);			  // find segment num
    readers[i]->doDelete(n - starts[i]);		  // dispatch to segment reader

    UNLOCK_MUTEX(doDelete_LOCK);
  }

  int_t SegmentsReader::readerIndex(const int_t n) const {	  // find reader for doc n:
    int_t lo = 0;					  // search starts array
    int_t hi = readersLength - 1;		  // for first element less
              // than n, return its index
    while (hi >= lo) {
      int_t mid = (lo + hi) >> 1;
      int_t midValue = starts[mid];
      if (n < midValue)
        hi = mid - 1;
      else if (n > midValue)
        lo = mid + 1;
      else
        return mid;
    }
    return hi;
  }

  void SegmentsReader::doClose() {
    LOCK_MUTEX(doClose_LOCK);
    for (int_t i = 0; i < readersLength; i++){
      readers[i]->close();
    }

    //if the segmentsreader was opened with zero segments, the segmentsreader
  //becomes responsible for closing the directory
  if ( this->closeDir ){
    this->directory.close();
  }
    UNLOCK_MUTEX(doClose_LOCK);
  }

}}
