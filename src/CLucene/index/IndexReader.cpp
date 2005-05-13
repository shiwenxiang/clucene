#include "CLucene/StdHeader.h"
#include "IndexReader.h"

#include "CLucene/store/Directory.h"
#include "CLucene/store/FSDirectory.h"
#include "CLucene/store/Lock.h"
#include "CLucene/document/Document.h"
#include "SegmentInfos.h"
#include "SegmentsReader.h"
#include <sys/stat.h>
#include "Terms.h"

using namespace lucene::util;
namespace lucene{ namespace index {

  IndexReader::IndexReader(Directory& dir):directory(dir) {
  //Constructor.
  //Func - Creates an instance of IndexReader
  //Pre  - true
  //Post - An instance has been created with writeLock = NULL

      writeLock = NULL;
  }

  IndexReader::~IndexReader(){
  //Func - Destructor
  //       Destroys the instance and releases the writeLock if needed
  //Pre  - true
  //Post - The instance has been destroyed if pre(writeLock) exists is has been released

      if (writeLock != NULL) {
		  //release writeLock
          writeLock->release();
		  _DELETE(writeLock);
          }
  }
  
  IndexReader& IndexReader::open(const char_t* path, const bool closeDir){
  //Func - Static method.
  //       Returns an IndexReader reading the index in an FSDirectory in the named path. 
  //Pre  - path != NULL and contains the path of the index for which an IndexReader must be 
  //       instantiated
  //       closeDir indicates if the directory needs to be closed
  //Post - An IndexReader has been returned that reads tnhe index located at path

	  CND_PRECONDITION(path != NULL, "path is NULL");

      return open(FSDirectory::getDirectory(path,false), closeDir);
  }

  IndexReader& IndexReader::open( Directory& directory, const bool closeDir){
  //Func - Static method.
  //       Returns an IndexReader reading the index in an FSDirectory in the named path. 
  //Pre  - directory represents a directory 
  //       closeDir indicates if the directory needs to be closed
  //Post - An IndexReader has been returned that reads the index located at directory

      IndexReader* ret = NULL;     
	  LuceneLock* lock = NULL;

	  // in- & inter-process sync
      LOCK_MUTEX(DIRECTORIES_MUTEX); 

#ifndef CLUCENE_LITE
      lock = directory.makeLock(_T("commit.lock"));
#endif
	  //Instantiate an IndexReaderLockWith which can produce an IndexReader
      IndexReaderLockWith with(lock,&directory,closeDir );

	  //Create an IndexReader reading the index
      ret = (IndexReader*)with.run();

#ifndef CLUCENE_LITE
      delete lock;
#endif
       // in- & inter-process sync
       UNLOCK_MUTEX(DIRECTORIES_MUTEX);

	   CND_CONDITION(ret != NULL,"ret is NULL");
	   //return reference 
       return *ret;
  }

  void* IndexReaderLockWith::doBody() {
  //Func - Reads the segmentinfo file and depending on the number of segments found
  //       it returns a SegmentsReader or a SegmentReader
  //Pre  - directory != NULL
  //Post - Depending on the number of Segments present in directory this method
  //       returns an empty SegmentsReader when there are no segments, a SegmentReader when
  //       directory contains 1 segment and a nonempty SegmentsReader when directory
  //       contains multiple segements

	   CND_PRECONDITION(directory != NULL, "directory is NULL");

	   //Instantiate SegmentInfos
       SegmentInfos infos;
       //Have SegmentInfos read the segments file in directory
       infos.read(*directory);

       // If there is at least one segment (if infos.size() >= 1), the last
       // SegmentReader object will close the directory when the SegmentReader
       // object itself is closed (see SegmentReader::doClose).
       // If there are no segments, there will be no "last SegmentReader object"
       // to fulfill this responsibility, so we need to explicitly close the
       // directory in the segmentsreader.close
       
	   //Count the number segments in the directory
	   const uint_t nSegs = infos.size();

       if (nSegs == 1 ) {
			// index is optimized 
            return new SegmentReader(infos.info(0), closeDir);
	   }else{
			//Instantiate an array of pointers to SegmentReaders of size nSegs (The number of segments in the index)
			SegmentReader** readers = NULL;

			if (nSegs > 0){
				readers = new SegmentReader*[nSegs];
                bool isResponsibleForClosingDir;

			    for (uint_t i = 0; i < nSegs; i++) {
				    //if i is the last segment then it will be reponsible for closing the directory
				    isResponsibleForClosingDir = i == nSegs-1;
				    //Instantiate a SegementReader responsible for reading the i-th segment and store it in
				    //the readers array
				    readers[i] = new SegmentReader(infos.info(i), isResponsibleForClosingDir);
				}
			}

			//return an instance of SegmentsReader which is a reader that manages all Segments
            SegmentsReader* ret = new SegmentsReader(*directory, readers, nSegs);		   
			return ret;
        }// end if
  }

  long_t IndexReader::lastModified(const char_t* directory) {
  //Func - Static method
  //       Returns the time the index in the named directory was last modified. 
  //Pre  - directory != NULL and contains the path name of the directory to check
  //Post - The last modified time of the index has been returned

      CND_PRECONDITION(directory != NULL, "directory is NULL");

      struct Struct_Stat buf;
      Cmd_Stat(directory, &buf);
      return buf.st_mtime;
  }
    
  long_t IndexReader::lastModified(const Directory& directory) {
  //Func - Static method
  //       Returns the time the index in this directory was last modified. 
  //Pre  - directory contains a valid reference
  //Post - The last modified time of the index has been returned

      return directory.fileModified(_T("segments"));
  }


  bool IndexReader::indexExists(const char_t* directory){
  //Func - Static method
  //       Checks if an index exists in the named directory
  //Pre  - directory != NULL
  //Post - Returns true if an index exists at the specified directory.
  //       If the directory does not exist or if there is no index in it.
  //       false is returned.

       CND_PRECONDITION(directory != NULL, "directory is NULL"); 

	   //Create a buffer of length CL_MAXDIR
       char_t f[CL_MAX_DIR];
	   //Copy the directory string to the buffer
       stringCopy(f,directory);
	   //Cat the name of the segments to buffer
       stringCat(f, _T("/segments"));
	   //Check if the segments file exists
       return lucene::util::Misc::dir_Exists(f);
  }
    
  bool IndexReader::indexExists(const Directory& directory){
  //Func - Static method
  //       Checks if an index exists in the directory
  //Pre  - directory is a valid reference
  //Post - Returns true if an index exists at the specified directory.
  //       If the directory does not exist or if there is no index in it.
  //       false is returned.

      return directory.fileExists(_T("segments"));
  }

  TermDocs& IndexReader::termDocs(Term* term) const {
  //Func - Returns an enumeration of all the documents which contain
  //       term. For each document, the document number, the frequency of
  //       the term in that document is also provided, for use in search scoring.
  //       Thus, this method implements the mapping: 
  //
  //       Term => <docNum, freq>*
  //	   The enumeration is ordered by document number.  Each document number
  //       is greater than all that precede it in the enumeration. 
  //Pre  - term != NULL
  //Post - A reference to TermDocs containing an enumeration of all found documents
  //       has been returned

      CND_PRECONDITION(term != NULL, "term is NULL");

      //Reference an instantiated TermDocs instance
      TermDocs& _termDocs = termDocs();
      //Seek all documents containing term
      _termDocs.seek(term);
      //return the enumaration
      return _termDocs;
  }

  TermPositions& IndexReader::termPositions(Term* term) {
  //Func - Returns an enumeration of all the documents which contain  term. For each 
  //       document, in addition to the document number and frequency of the term in 
  //       that document, a list of all of the ordinal positions of the term in the document 
  //       is available.  Thus, this method implements the mapping:
  //
  //       Term => <docNum, freq,<pos 1, pos 2, ...pos freq-1>>*
  //
  //       This positional information faciliates phrase and proximity searching.
  //       The enumeration is ordered by document number.  Each document number is greater than 
  //       all that precede it in the enumeration. 
  //Pre  - term != NULL
  //Post - A reference to TermPositions containing an enumeration of all found documents
  //       has been returned

      CND_PRECONDITION(term != NULL, "term is NULL");

      //Reference an instantiated termPositions instance
      TermPositions& _termPositions = termPositions();
	  //Seek all documents containing term
      _termPositions.seek(term);
	  //return the enumeration
      return _termPositions;
  }

#ifndef CLUCENE_LITE
  void IndexReader::Delete(const int_t docNum) {
  //Func - Deletes the document numbered docNum.  Once a document is deleted it will not appear 
  //       in TermDocs or TermPostitions enumerations. Attempts to read its field with the document 
  //       method will result in an error.  The presence of this document may still be reflected in 
  //       the docFreq statistic, though this will be corrected eventually as the index is further modified.  
  //Pre  - docNum >= 0
  //Post - If successful the document identified by docNum has been deleted. If no writelock
  //       could be obtained an exception has been thrown stating that the index was locked or has no write access

      CND_PRECONDITION(docNum >= 0, "docNum is negative");  

	  LOCK_MUTEX(Delete_LOCK);
	  //Check if a writeLock exists
      if (writeLock == NULL){
		  //Instantiate a LuceneLock using the filename write.lock
          lucene::store::LuceneLock* wl = directory.makeLock(_T("write.lock"));

          CND_CONDITION(wl != NULL, "wl is NULL"); 

		  //Obtain the write lock
          if (!wl->obtain()){	
			  //destroy the instance because no lock has been obtained
              delete wl;
			  //Throw an exception, because the index was locked or had no write access
              _THROWC("Index locked for write or no write access.");
              }
          //write lock has been obtained so assign it to writeLock for later use
          writeLock = wl;
      }
	  //Have the document identified by docNum deleted
      doDelete(docNum);
      UNLOCK_MUTEX(Delete_LOCK);
  }

  int_t IndexReader::Delete(Term* term) {
  //Func - Deletes all documents containing term. This is useful if one uses a 
  //       document field to hold a unique ID string for the document.  Then to delete such  
  //       a document, one merely constructs a term with the appropriate field and the unique 
  //       ID string as its text and passes it to this method.  
  //Pre  - term != NULL
  //Post - All documents containing term have been deleted. The number of deleted documents
  //       has been returned

      CND_PRECONDITION(term != NULL, "term is NULL");  

	  //Search for the documents contain term
      TermDocs* docs = &termDocs(term);

	  //Check if documents have been found
	  if ( docs == NULL ){
          return 0;
	      }
    
	  //initialize
	  int_t Counter = 0;
      _TRY {
		  //iterate through the found documents
          while (docs->next()) {
			  //Delete the document
              Delete(docs->Doc());
              Counter++;
              }
          }
	  _FINALLY(
		  //Close the enumeration
          docs->close();
          );

    //Delete the enumeration of found documents
    delete docs;

	//Return the number of deleted documents
    return Counter;
  }
#endif //ifndef CLUCENE_LITE

  void IndexReader::close() {
  //Func - Closes files associated with this index and also saves any new deletions to disk.
  //       No other methods should be called after this has been called.
  //Pre  - true
  //Post - All files associated with this index have been deleted and new deletions have been 
  //       saved to disk

      LOCK_MUTEX(close_LOCK);
      doClose();
	  //Check if the writeLock exists
      if (writeLock != NULL) {
          //release write lock
          writeLock->release();  
          //delete the writeLock
          delete writeLock;
	      //reset it to NULL
          writeLock = NULL;
          }
      UNLOCK_MUTEX(close_LOCK);
  }
   
  bool IndexReader::isLocked(const Directory& directory) {
  //Func - Static method 
  //       Checks if the index in the directory is currently locked.
  //Pre  - directory is a valid reference to a directory to check for a lock
  //Post - Returns true if the index in the named directory is locked otherwise false

	  //Check the existence of the file write.lock and return true when it does and false
	  //when it doesn't
      return directory.fileExists(_T("write.lock"));
  }

  bool IndexReader::isLocked(const char_t* directory) {
  //Func - Static method 
  //       Checks if the index in the named directory is currently locked.
  //Pre  - directory != NULL and contains the directory to check for a lock
  //Post - Returns true if the index in the named directory is locked otherwise false

      CND_PRECONDITION(directory != NULL, "directory is NULL");  

	  //Create a buffer of length CL_MAXDIR
      char_t f[CL_MAX_DIR];
	  //Copy the directory string to the buffer
      stringCopy(f,directory);
	  //Cat the name of the write.lock file to buffer
      stringCat ( f,_T("/write.lock") );
	  //Check if the write.lock file exists
      return lucene::util::Misc::dir_Exists(f);
  }

#ifndef CLUCENE_LITE

  void IndexReader::unlock(Directory& directory){
  //Func - Static method
  //       Forcibly unlocks the index in the named directory.
  //       Caution: this should only be used by failure recovery code,
  //       when it is known that no other process nor thread is in fact
  //       currently accessing this index.
  //Pre  - directory is a valid reference to a directory 
  //Post - The directory has been forcibly unlocked

      try{
		  //Try to delete the write.lock file
          directory.deleteFile(_T("write.lock"),false);
          }
	  catch(...)
	      {}

      try{
		  //Try to delete the commit.lock file
          directory.deleteFile(_T("commit.lock"),false);
          }
	  catch(...)
	      {}
  }
#endif //CLUCENE_LITE

}}
