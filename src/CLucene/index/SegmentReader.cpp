#include "CLucene/StdHeader.h"
#include "SegmentHeader.h"

#include "SegmentInfo.h"
#include "FieldInfos.h"
#include "FieldsReader.h"
#include "IndexReader.h"
#include "TermInfosReader.h"
#include "CLucene/util/BitVector.h"
#include "Terms.h"
#include "CLucene/util/VoidMap.h"

using namespace lucene::util;
namespace lucene{ namespace index{

  Norm::Norm(InputStream& instrm): in(instrm){
  //Func - Constructor
  //Pre  - instrm is a valid reference to an InputStream
  //Post - A Norm instance has been created with an empty bytes array

	  bytes = NULL;
  }

  Norm::~Norm() {
  //Func - Destructor
  //Pre  - true
  //Post - The InputStream in has been deleted (and closed by its destructor) 
  //       and the array too.

      //Close and destroy the inputstream in. The inputstream will be closed
      // by its destructor. Note that the InputStream 'in' actually is a pointer!!!!!  
      delete &in;

	  //Delete the bytes array
      _DELETE_ARRAY(bytes);
  }
  
  SegmentReader::SegmentReader(SegmentInfo& si, const bool closeDir) : 
      //Init the superclass IndexReader
      IndexReader(si.dir){
  //Func - Constructor.
  //       Opens all files of a segment
  //       .fnm     -> Field Info File
  //                    Field names are stored in the field info file, with suffix .fnm.
  //       .frq     -> Frequency File
  //                   The .frq file contains the lists of documents which contain 
  //                   each term, along with the frequency of the term in that document.
  //       .prx     -> Prox File
  //                   The prox file contains the lists of positions that each term occurs
  //                   at within documents.
  //       .tis     -> Term Info File
  //                   This file is sorted by Term. Terms are ordered first lexicographically 
  //                   by the term's field name, and within that lexicographically by the term's text.
  //       .del     -> Deletion File
  //                   The .del file is optional, and only exists when a segment contains deletions
  //       .f[0-9]* -> Norm File
  //                   Contains s, for each document, a byte that encodes a value that is 
  //                   multiplied into the score for hits on that field:
  //
  //Pre  - si is a valid reference to SegmentInfo instance
  //       closeDir indicates if the directory must be closed after reading the segment
  //       identified by si
  //Post - All files of the segment have been read

	  //Initialize member variables
      closeDirectory   = closeDir;
      deletedDocs      = NULL;
	  //There are no documents yet marked as deleted
      deletedDocsDirty = false;
	  //Duplicate the name of the segment from SegmentInfo to segment
      segment          = stringDuplicate(si.name);
	  // make sure that all index files have been read or are kept open
      // so that if an index update removes them we'll still have them
      freqStream       = NULL;
      proxStream       = NULL;

	  //instantiate a buffer large enough to hold a directory path
      char_t buf[MAX_PATH];

	  //Create the name of the field info file with suffix .fnm in buf
      segmentname(buf, _T(".fnm"));
      fieldInfos = new FieldInfos(directory, buf );

      //Condition check to see if fieldInfos points to a valid instance
      CND_CONDITION(fieldInfos != NULL,"No memory could be allocated for fieldInfos")	

	  //Create the name of the frequence file with suffix .frq in buf
      segmentname(buf, _T(".frq"));

	  //Open an InputStream freqStream to the frequency file
      freqStream = &directory.openFile( buf );

      //Condition check to see if freqStream points to a valid instance and was able to open the
	  //frequency file
      CND_CONDITION(freqStream != NULL,"InputStream freqStream could not open the frequency file")

	  //Create the name of the prox file with suffix .prx in buf
      segmentname(buf, _T(".prx"));

	  //Open an InputStream proxStream to the prox file
      proxStream = &directory.openFile( buf );

	  //Condition check to see if proxStream points to a valid instance and was able to open the
	  //prox file
      CND_CONDITION(proxStream != NULL,"InputStream proxStream could not open proximity file")

	  //Instantiate a FieldsReader for reading the Field Info File
      fieldsReader = new FieldsReader(directory, segment, *fieldInfos);

      //Condition check to see if fieldsReader points to a valid instance 
      CND_CONDITION(fieldsReader != NULL,"No memory could be allocated for fieldsReader")

	  //Instantiate a TermInfosReader for reading the Term Dictionary .tis file
      tis = new TermInfosReader(directory, segment, *fieldInfos);

      //Condition check to see if tis points to a valid instance 
      CND_CONDITION(tis != NULL,"No memory could be allocated for tis")

	  //Check if the segment has deletion according to the SegmentInfo instance si
      if (hasDeletions(si)){
		  //Create a deletion file with suffix .del          
          segmentname(buf, _T(".del"));
		  //Instantiate a BitVector that manages which documents have been deleted
          deletedDocs = new lucene::util::BitVector(directory, buf );
          }

	  //Open the norm file. There's a norm file for each indexed field with a byte for each document. 
	  //The .f[0-9]* file contains, for each document, a byte that encodes a value 
	  //that is multiplied into the score for hits on that field
      openNorms();
  }

  SegmentReader::~SegmentReader(){
  //Func - Destructor.
  //Pre  - doClose has been invoked!
  //Post - the instance has been destroyed

      _DELETE(fieldInfos);
	  _DELETE(fieldsReader);
      _DELETE(tis);	      
 	  _DELETE(freqStream);
	  _DELETE(proxStream);
	  _DELETE_ARRAY(segment);
	  _DELETE(deletedDocs);
  }

  void SegmentReader::doClose() {
  //Func - Closes all streams to the files of a single segment
  //Pre  - fieldsReader != NULL
  //       tis != NULL
  //Post - All streams to files have been closed

      CND_PRECONDITION(fieldsReader != NULL, "fieldsReader is NULL")
      CND_PRECONDITION(tis != NULL, "tis is NULL")

	  //Place a mutex lock
      LOCK_MUTEX(doClose_LOCK);

#ifndef CLUCENE_LITE
	  //Check if there are documents that have been marked deleted
      if (deletedDocsDirty) {
		  //Create a lock using the filename commit.lock
          LuceneLock* lock = directory.makeLock(_T("commit.lock"));

          //Condition check to see if lock points to a valid instance
          CND_CONDITION(lock != NULL,"No memory could be allocated for lock")

		  SegmentReaderLockWith with ( lock, this );

          LOCK_MUTEX(DIRECTORIES_MUTEX); // in- & inter-process sync		  
          with.run();
          UNLOCK_MUTEX(DIRECTORIES_MUTEX);

		  //Remove the lock
          delete lock;
		  //Reset deletedDocsDirty
          deletedDocsDirty = false;
          }
#endif // CLUCENE_LITE

	  //Close the fieldsReader
      fieldsReader->close();
	  //Close the TermInfosReader
      tis->close();

	  //Close the frequency stream
	  if (freqStream != NULL){
          freqStream->close();
	      }
	  //Close the prox stream
	  if (proxStream != NULL){
          proxStream->close();
	      }

	  //Close the norm file
      closeNorms();

	  //Check if the directory must be closed
	  if (closeDirectory){
		  //Close the directory
		  directory.close();
	      }
    
    UNLOCK_MUTEX(doClose_LOCK);
  }


  bool SegmentReader::hasDeletions(const SegmentInfo& si) {
  //Func - Static method
  //       Checks if a segment managed by SegmentInfo si has deletions
  //Pre  - si holds a valid reference to an SegmentInfo instance
  //Post - if the segement contains deleteions true is returned otherwise flas

	  //Create a buffer f of length MAX_PATH
      char_t f[MAX_PATH];
      //SegmentReader::segmentname(f, si.name,_T(".del"),-1 );
      //create the name of the deletion file
	  segmentname(f, si.name,_T(".del"),-1 );
	  //Check if the deletion file exists and return the result
      return si.dir.fileExists( f );
  }

  void SegmentReader::doDelete(const int_t docNum){
  //Func - Marks document docNum as deleted
  //Pre  - docNum >=0 and DocNum < MaxDoc() 
  //       docNum contains the number of the document that must be 
  //       marked deleted
  //Post - The document identified by docNum has been marked deleted

     CND_PRECONDITION(docNum >= 0, "docNum is a negative number")
     CND_PRECONDITION(docNum < MaxDoc(), "docNum is bigger than the total number of documents")

      LOCK_MUTEX(doDelete_LOCK);
	  //Check if deletedDocs exists
	  if (deletedDocs == NULL){
          deletedDocs = new lucene::util::BitVector(MaxDoc());

          //Condition check to see if deletedDocs points to a valid instance
          CND_CONDITION(deletedDocs != NULL,"No memory could be allocated for deletedDocs")
	      }
      //Flag that there are documents marked deleted
      deletedDocsDirty = true;
      //Mark document identified by docNum as deleted
      deletedDocs->set(docNum);

      UNLOCK_MUTEX(doDelete_LOCK);
  }

  StringArrayConst& SegmentReader::files() {
  //Func - Returns all file names managed by this SegmentReader
  //Pre  - segment != NULL
  //Post - All filenames managed by this SegmentRead have been returned
 
     CND_PRECONDITION(segment != NULL, "segment is NULL")

	 StringArrayConst* files = new StringArrayConst(true,DELETE_TYPE_DELETE_ARRAY);

     //Condition check to see if files points to a valid instance
     CND_CONDITION(files != NULL,"No memory could be allocated for files")

     //Add the name of the Field Info file
     files->push_back( segmentname(_T(".fnm") ));
     //Add the name of the 
     files->push_back( segmentname(_T(".fdx") ));
     files->push_back( segmentname(_T(".fdt") ));
     files->push_back( segmentname(_T(".tii") ));
     files->push_back( segmentname(_T(".tis") ));
     files->push_back( segmentname(_T(".frq") ));
     files->push_back( segmentname(_T(".prx") ));

     //Create the name of the deletion file
     const char_t* DeletionFile = segmentname(_T(".del"));
     //check if the deletion file exists
     if (directory.fileExists(DeletionFile)){
         files->push_back( DeletionFile );
         }
     else{
         _DELETE_ARRAY(DeletionFile);
         }

      //iterate through the field infos
      for (int_t i = 0; i < fieldInfos->size(); i++) {
          //Get the field info for the i-th field   
          FieldInfo& fi = fieldInfos->fieldInfo(i);
          //Check if the field has been indexed
          if (fi.isIndexed){
              //The field has been indexed so add its norm file
              files->push_back( segmentname(_T(".f"), i) );
              }
          }

    return *files;
  }

  TermEnum& SegmentReader::getTerms() const {
  //Func - Returns an enumeration of all the Terms and TermInfos in the set. 
  //Pre  - tis != NULL
  //Post - An enumeration of all the Terms and TermInfos in the set has been returned

      CND_PRECONDITION(tis != NULL, "tis is NULL")

      return tis->getTerms();
  }

  TermEnum& SegmentReader::getTerms(const Term* t) const {
  //Func - Returns an enumeration of terms starting at or after the named term t 
  //Pre  - t != NULL
  //       tis != NULL
  //Post - An enumeration of terms starting at or after the named term t 

      CND_PRECONDITION(t   != NULL, "t is NULL")
      CND_PRECONDITION(tis != NULL, "tis is NULL")

      return tis->getTerms(*t);
  }

  lucene::document::Document& SegmentReader::document(const int_t n) {
  //Func - Returns a document identified by n
  //Pre  - n >=0 and identifies the document n
  //Post - if the document has been deleted then an exception has been thrown
  //       otherwise a reference to the found document has been returned

      CND_PRECONDITION(n >= 0, "n is a negative number")

      //Declare ret outside the scope of LOCK_MUTEX 
	  //This is required in case we are building with multithreading enabled
      lucene::document::Document* ret = NULL;

      LOCK_MUTEX(document_LOCK);

	  //Check if the n-th document has been marked deleted
       if (isDeleted(n)){
           //UNLOCK_MUTEX(document_LOCK);
          _THROWC(  "attempt to access a deleted document" );
          }

	   //Retrieve the n-th document
       ret = &fieldsReader->doc(n);

       //Condition check to see if ret points to a valid instance
       CND_CONDITION(ret != NULL,"No document could be retrieved")

       UNLOCK_MUTEX(document_LOCK);

	   //Return the document
       return *ret;
  }

  bool SegmentReader::isDeleted(const int_t n) {
  //Func - Checks if the n-th document has been marked deleted
  //Pre  - n >=0 and identifies the document n
  //Post - true has been returned if document n has been deleted otherwise fralse

      CND_PRECONDITION(n >= 0, "n is a negative number")

      bool ret;
      LOCK_MUTEX(isDeleted_LOCK);
	  //Is document n deleted
      ret = (deletedDocs != NULL && deletedDocs->get(n));

      UNLOCK_MUTEX(isDeleted_LOCK);
      return ret;
  }

  TermDocs& SegmentReader::termDocs() const {
  //Func - Returns an unpositioned TermDocs enumerator. 
  //Pre  - true
  //Post - An unpositioned TermDocs enumerator has been returned

       return *new SegmentTermDocs((void*)this);
  }

  TermPositions& SegmentReader::termPositions() const {
  //Func - Returns an unpositioned TermPositions enumerator. 
  //Pre  - true
  //Post - An unpositioned TermPositions enumerator has been returned

      return *new SegmentTermPositions((void*)this);
  }

  int_t SegmentReader::docFreq(const Term& t) const {
  //Func - Returns the number of documents which contain the term t
  //Pre  - t holds a valid reference to a Term
  //Post - The number of documents which contain term t has been returned

      //Get the TermInfo ti for Term  t in the set
      TermInfo* ti = tis->get(t);
      //Check if an TermInfo has been returned
      if (ti){
		  //Get the frequency of the term
          int_t ret = ti->docFreq;
		  //TermInfo ti is not needed anymore so delete it
          delete ti;
		  //return the number of documents which containt term t
          return ret;
          }
	  else
		  //No TermInfo returned so return 0
          return 0;
  }

  int_t SegmentReader::NumDocs() {
  //Func - Returns the actual number of documents in the segment
  //Pre  - true
  //Post - The actual number of documents in the segments

	  //Get the number of all the documents in the segment including the ones that have 
	  //been marked deleted
      int_t n = MaxDoc();

	  //Check if there any deleted docs
      if (deletedDocs != NULL)
		  //Substract the number of deleted docs from the number returned by MaxDoc
          n -= deletedDocs->Count();

	  //return the actual number of documents in the segment
      return n;
  }

  int_t SegmentReader::MaxDoc() const {
  //Func - Returns the number of  all the documents in the segment including
  //       the ones that have been marked deleted
  //Pre  - true
  //Post - The total number of documents in the segment has been returned

      return fieldsReader->Size();
  }

  l_byte_t* SegmentReader::getNorms(const char_t* field) {
  //Func - Returns the bytes array that holds the norms of a named field
  //Pre  - field != NULL and contains the name of the field for which the norms 
  //       must be retrieved
  //Post - If there was norm for the named field then a bytes array has been allocated 
  //       and returned containing the norms for that field. If the named field is unknown NULL is returned.

      CND_PRECONDITION(field != NULL, "field is NULL")

      //Try to retrieve the norms for field
      Norm* norm = (Norm*)norms.get(field);
      //Check if a norm instance was found
      if (norm == NULL){
         //return NULL as there are no norms to be returned
         return NULL;
		 }

      //Check if the bytes array exist which holds the norms
      if (norm->bytes == NULL) {
          //allocate a new bytes array to hold the norms
          l_byte_t* bytes = new l_byte_t[MaxDoc()]; 

          //Condition check to see if bytes points to a valid array
          CND_CONDITION(bytes != NULL,"bytes is NULL")

          //Read the norms from disk straight into the new bytes array
          getNorms(field, bytes, 0);
          //assign the new allocated bytes array
          norm->bytes = bytes;
          }

	//Return the norms
    return norm->bytes;
  }

  void SegmentReader::getNorms(const char_t* field, l_byte_t* bytes, const int_t offset) {
  //Func - Reads the Norms for field from disk starting at offset in the inputstream
  //Pre  - field != NULL
  //       bytes != NULL is an array of bytes which is to be used to read the norms into.
  //       it is advisable to have bytes initalized by zeroes!
  //       offset >= 0
  //Post - The if an inputstream to the norm file could be retrieved the bytes have been read
  //       You are never sure whether or not the norms have been read into bytes properly!!!!!!!!!!!!!!!!!

      CND_PRECONDITION(field != NULL, "field is NULL")
      CND_PRECONDITION(bytes != NULL, "field is NULL")
      CND_PRECONDITION(offset >= 0, "offset is a negative number")

      //Get a stream to the norm file
      InputStream* _normStream = normStream(field);

      //If there is no stream just exit
	  if (_normStream == NULL){
          //Strange code because the invoker does not know if getNorms succeeds or not!!!!!!!
          // use zeros in array
          return;			
	      }

      //Try to read the bytes from the _normStream
      _TRY{
          _normStream->readBytes(bytes, offset, MaxDoc());
          } 
      _FINALLY(
          //Have the normstream closed
          _normStream->close();
          //Destroy the normstream
          delete _normStream;
          );
  }

  InputStream* SegmentReader::normStream(const char_t* field) {
  //Func - Returns an inputstream to the norms file for field
  //Pre  - field != NULL
  //Post - An input stream to the norms file for field has been cloned and reset and 
  //       returned

      CND_PRECONDITION(field != NULL, "field is NULL")

      //Try to retrieve the norms for field
      Norm* norm = norms.get(field);
      //Check if a norm instance was found
      if (norm == NULL)
          return NULL;
      
      //Clone the inputStream of the norm
      InputStream* result = &norm->in.clone();

      //Condition check to see if result points to a valid instance
      CND_CONDITION(result != NULL,"result is NULL")

	  //Reset the inputstream at the from
      result->seek(0);

      return result;
  }

  void SegmentReader::segmentname(char_t* buffer,const char_t* Segment, const char_t* ext, const int_t x){
  //Func - Static Method
  //       Creates a filename in buffer by concatenating Segment with ext and x
  //Pre  - buffer  != NULL
  //       Segment != NULL and holds the name of the segment
  //       ext     != NULL and holds the extension
  //       x contains a number
  //Post - When x = -1 buffer contains the concatenation of Segment and ext otherwise
  //       buffer contains the contentation of Segment, ext and x

      CND_PRECONDITION(buffer  != NULL, "buffer is NULL")
      CND_PRECONDITION(Segment != NULL, "Segment is NULL")
	  CND_PRECONDITION(ext     != NULL, "ext is NULL")

      if ( x== -1 )
          stringPrintF(buffer,_T("%s%s"), Segment,ext );
      else
          stringPrintF(buffer,_T("%s%s%d"), Segment,ext,x );
  }

  char_t* SegmentReader::segmentname(const char_t* ext, const int_t x){
  //Func - Returns an allocated buffer in which it creates a filename by 
  //       concatenating segment with ext and x
  //Pre    ext != NULL and holds the extension
  //       x contains a number
  //Post - A buffer has been instantiated an when x = -1 buffer contains the concatenation of 
  //       segment and ext otherwise buffer contains the contentation of segment, ext and x
      
	  CND_PRECONDITION(ext     != NULL, "ext is NULL")

	  //Create a buffer of length MAX_PATH
	  char_t* buf = new char_t[MAX_PATH];
	  //Create the filename
      segmentname(buf,ext,x);
	  
      return buf ;
  }

  void SegmentReader::segmentname(char_t* buffer,const char_t* ext, const int_t x ){
  //Func - Creates a filename in buffer by concatenating segment with ext and x
  //Pre  - buffer != NULL
  //       ext    != NULL
  //       x contains a number
  //Post - When x = -1 buffer contains the concatenation of segment and ext otherwise
  //       buffer contains the contentation of segment, ext and x

      CND_PRECONDITION(buffer  != NULL, "buffer is NULL")
      CND_PRECONDITION(segment != NULL, "Segment is NULL")

      SegmentReader::segmentname(buffer,segment,ext,x);
  }

  void SegmentReader::openNorms() {
  //Func - Open all norms files for all fields
  //       Creates for each field a norm Instance with an open inputstream to 
  //       a corresponding norm file ready to be read
  //Pre  - true
  //Post - For each field a norm instance has been created with an open inputstream to
  //       a corresponding norm file ready to be read

      //Iterate through all the fields
      for (int_t i = 0; i < fieldInfos->size(); i++) {
		  //Get the FieldInfo for the i-th field
          FieldInfo& fi = fieldInfos->fieldInfo(i);
          //Check if the field is indexed
          if (fi.isIndexed) {
		      //Allocate a buffer
              char_t f[MAX_PATH];
			  //Create a filename for the norm file
              segmentname(f, _T(".f"), fi.number);
              //TODO, should fi.name be copied?
			  //Create a new Norm with an open inputstream to f and store
			  //it at fi.name in norms
              norms.put(fi.name, new Norm( directory.openFile( f ) ));
              }
      }
  }

  void SegmentReader::closeNorms() {
  //Func - Close all the norms stored in norms
  //Pre  - true
  //Post - All the norms have been destroyed

    LOCK_MUTEX(norms_mutex);
	//Create an interator initialized at the beginning of norms
    map<const char_t*,Norm*,lucene::util::charCompare>::iterator itr = norms.begin();
	//Iterate through all the norms
    while (itr != norms.end()) {
        //Get the norm
        Norm* n = itr->second;
        //delete the norm n
        _DELETE(n);
        //Move the interator to the next norm in the norms collection.
	    //Note ++ is an overloaded operator
        itr++;
        }
    UNLOCK_MUTEX(norms_mutex);
  }

#ifndef CLUCENE_LITE
  void* SegmentReaderLockWith::doBody() {
  //Func - Flushes all deleted docs of reader to a new deletions file overwriting the current
  //Pre  - reader != NULL
  //Post - The deletions file has been overwritten by the new one

      CND_PRECONDITION(reader  != NULL, "reader is NULL")

      char_t tmpName[MAX_PATH];
      char_t delName[MAX_PATH];
      //Create a filename for storing the new deletes docs
      reader->segmentname(tmpName, _T(".tmp"));
	  //Create a filename for the current deletion file
      reader->segmentname(delName, _T(".del"));
      //Write the deleted docs to the new deletion file
      reader->deletedDocs->write(reader->directory,  tmpName );
    
	  //Rename the old deletions file into the new one
      reader->directory.renameFile( tmpName, delName );
      return NULL;
  }
#endif //CLUCENE_LITE
}}
