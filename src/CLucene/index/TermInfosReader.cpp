#include "CLucene/StdHeader.h"
#include "TermInfosReader.h"

#include "Term.h"
#include "Terms.h"
#include "SegmentTermEnum.h"
#include "CLucene/store/Directory.h"
#include "FieldInfos.h"
#include "TermInfo.h"
#include "TermInfosWriter.h"
#include "CLucene/util/Arrays.h"

namespace lucene{ namespace index {


  TermInfosReader::TermInfosReader(Directory& dir, const char_t* seg, FieldInfos& fis):
      directory (dir),fieldInfos (fis){
  //Func - Constructor.
  //       Reads the TermInfos file (.tis) and eventually the Term Info Index file (.tii)
  //Pre  - dir is a reference to a valid Directory 
  //       Fis contains a valid reference to an FieldInfos instance
  //       seg != NULL and contains the name of the segment
  //Post - An instance has been created and the index named seg has been read. (Remember
  //       a segment is nothing more then an independently readable index)
	    
      CND_PRECONDITION(seg != NULL, "seg is NULL")

	  //Initialize the name of the segment
      segment    =  seg;
      //There are no indexTerms yet
      indexTerms    = NULL;
	  //So there are no indexInfos
	  indexInfos    = NULL;
	  //So there are no indexPointers
	  indexPointers = NULL; 	
      //Create a filname fo a Term Info File
      const char_t* TermInfosFile = segmentname(_T(".tis"));

      //Create an SegmentTermEnum for storing all the terms read of the segment
      _enum = new SegmentTermEnum( directory.openFile( TermInfosFile ), fieldInfos, false);

	  //Check if _enum points to a valid instance
      CND_CONDITION(_enum != NULL, "No memory could be allocated for _enum")

      //TermInfosFile is not needed anymore
      delete[] TermInfosFile;

      //Get the size of the enumeration and store it in size
      size =  _enum->size;
		
      //Read the segment
      readIndex();
  }

  TermInfosReader::~TermInfosReader(){
  //Func - Destructor
  //Pre  - true
  //Post - The instance has been destroyed

      //Close the TermInfosReader to be absolutly sure that _enum has been closed
	  //and the arrays indexTerms, indexPointers and indexInfos and  their elements 
	  //have been destroyed
      close();

	  //Get a pointer to InputStream used by the enumeration but 
	  //instantiated in the constructor by directory.openFile( TermInfosFile )
      InputStream *Is = &_enum->input;

      //Delete the enumuration _enum
      _DELETE(_enum);
      //Delete the InputStream 
	  _DELETE(Is);	
  }

  void TermInfosReader::close() {
  //Func - Close the enumeration of TermInfos
  //Pre  - true
  //Post - The _enumeration has been closed and the arrays

	  //Check if indexTerms and indexInfos exist
	  if (indexTerms && indexInfos){

          //Iterate through arrays indexTerms and indexPointer to
	      //destroy their elements
          for ( int_t i=0;i<indexTermsLength;i++ ){
              //finalize Term at indexTerms[i]
		      indexTerms[i]->finalize();
              //delete TermInfo at indexInfos[i]
              _DELETE(indexInfos[i]);
              }
          }

      //Delete the arrays
      _DELETE_ARRAY(indexTerms);
      _DELETE_ARRAY(indexPointers);
      _DELETE_ARRAY(indexInfos);


      if (_enum != NULL){
          _enum->close();
          }
  }

  int_t TermInfosReader::Size() {
  //Func - Return the size of the enumeration of TermInfos
  //Pre  - true
  //Post - size has been returened

      return size;
  }

  DEFINE_MUTEX(getInt_LOCK);
  
  Term* TermInfosReader::get(const int_t position) {
  //Func - Returns the nth term in the set
  //Pre  - position > = 0
  //Post - The n-th term in the set has been returned

      CND_PRECONDITION(position >= 0,"position contains a negative number")

      Term* ret = NULL;
	
      LOCK_MUTEX(getInt_LOCK);
      
	  //Check if the size is 0 because then there are no terms
      if (size == 0) 
          return NULL;
    
      //if
      if ( 
          //an enumeration exists
          _enum != NULL && 
          //which has a current term
          _enum->getTerm(false) != NULL && 
          //where the position of the request term is larger or equal to current position
          //in the enumeration
          position >= _enum->position &&
          //but where the position of the requested term  is smaller than the 
          //last one in the current enumeration
          position < (_enum->position + LUCENE_WRITER_INDEX_INTERVAL )){

          //Get the Term at position
          ret = scanEnum(position);

          //Seek can be avoided so return the found term
          return ret;		  
          }

    //Reposition
    seekEnum(position / LUCENE_WRITER_INDEX_INTERVAL); 

	//Get the Term at position
    ret = scanEnum(position);

    UNLOCK_MUTEX(getInt_LOCK);
	//Return the found term
    return ret;
  }


  TermInfo* TermInfosReader::get(const Term& term){
  //Func - Returns a TermInfo for a term
  //Pre  - term holds a valid reference to term
  //Post - if term can be found its TermInfo has been returned otherwise NULL

      //If the size of the enumeration is 0 then no Terms have been read
	  if (size == 0){      
		  return NULL;
	      }

	TermInfo* ret;
    LOCK_MUTEX(getTerm_LOCK);

    // optimize sequential access: first try scanning cached _enum w/o seeking

    //if
    if (
		//the current term of the enumeration _enum is not at the end AND
		_enum->getTerm(false) != NULL	 && 
		(
		    (
			     //there exists a previous current called prev and term is positioned after this prev OR
			     _enum->prev != NULL && term.compareTo(*_enum->prev) > 0) || 
				 //term is positioned at the same position as the current of _enum or at a higher position
		         term.compareTo(*_enum->getTerm(false)) >= 0
			)
		){

		//Calculate the offset for the position
        int_t _enumOffset = (_enum->position/LUCENE_WRITER_INDEX_INTERVAL)+1;

        // but before end of block

        //if
        if (
            //the length of indexTerms (the number of terms in _enum) equals
		    //_enum_offset OR
            indexTermsLength == _enumOffset	 || 
            //term is positioned in front of term found at _enumOffset in indexTerms
            term.compareTo(*indexTerms[_enumOffset]
            ) < 0){

            //no need to seek, retrieve the TermInfo for term
            ret = scanEnum(term);			  

            return ret;
            }
        }

    //Reposition current term in the enumeration 
    seekEnum(getIndexOffset(term));

	//Retrieve the TermInfo for term
    ret = scanEnum(term);

    UNLOCK_MUTEX(getTerm_LOCK);
    return ret;
  }


  int_t TermInfosReader::getPosition(const Term& term) {
  //Func - Returns the position of a Term in the set
  //Pre  - term holds a valid reference to a Term
  //       _enum != NULL
  //Post - If term was found then its position is returned otherwise -1

      CND_PRECONDITION(_enum != NULL,"_enum is NULL")

	  //if the enumeration is empty then return -1
	  if (size == 0){
		  return -1;
          }

       LOCK_MUTEX(getPosition_LOCK);
      //Retrieve the indexOffset for term
      int_t indexOffset = getIndexOffset(term);
      seekEnum(indexOffset);

      while(term.compareTo(*_enum->getTerm(false)) > 0 && _enum->next()) {}

      UNLOCK_MUTEX(getPosition_LOCK);
      if (term.compareTo(*_enum->getTerm(false)) == 0)
          return _enum->position;
      else
          return -1;
  }

  SegmentTermEnum& TermInfosReader::getTerms(){
  //Func - Returns an enumeration of all the Terms and TermInfos in the set
  //Pre  - _enum != NULL
  //Post - _enum has been cloned a reference to the clone has been returned
      
      CND_PRECONDITION(_enum != NULL,"_enum is NULL")

 	  //Declare ret outside the scope of LOCK_MUTEX 
	  //This is required in case we are building with multithreading enabled

      SegmentTermEnum* cln = NULL;

      LOCK_MUTEX(getTerms_LOCK);
      //if the position is not at the start
      if (_enum->position != -1){			  
          //reset the position of the current within the enumeration to the start
          seekEnum(0);				  
	      }

      //TODO2: check this code
	  //Clone the enumeration of terms
      cln = _enum->clone();

	  //Check if cln points to a valid instance
	  CND_CONDITION(cln != NULL,"cln is NULL")

      UNLOCK_MUTEX(getTerms_LOCK);
	  //Return the reference to the clone
      return *cln;
  }

  SegmentTermEnum& TermInfosReader::getTerms(const Term& term) {
  //Func - Returns an enumeration of terms starting at or after the named term.
  //Pre  - term holds a valid reference to a Term
  //       _enum != NULL
  //Post - An enumeration of terms starting at or after the named term has been returned

      CND_PRECONDITION(_enum != NULL,"_enum is NULL")

      SegmentTermEnum* cln;
      LOCK_MUTEX(getTermsTerm_LOCK);

      //Seek _enum to term
      TermInfo* termInfo = get(term);

      //Seek _enum to term; delete the new TermInfo that's returned.
      if(termInfo){
          delete termInfo;
          }

      //Clone the entire enumeration
      cln = _enum->clone();

      //Check if cln points to a valid instance
      CND_CONDITION(cln != NULL,"cln is NULL")

      UNLOCK_MUTEX(getTermsTerm_LOCK);

      return *cln;
  }

  void TermInfosReader::readIndex() {
  //Func - Reads the term info index file or .tti file.
  //       This file contains every IndexInterval-th entry from the .tis file, 
  //       along with its location in the "tis" file. This is designed to be read entirely 
  //       into memory and used to provide random access to the "tis" file.
  //Pre  - indexTerms    = NULL
  //       indexInfos    = NULL
  //       indexPointers = NULL
  //Post - The term info index file has been read into memory

      CND_PRECONDITION(indexTerms == NULL,    "indexTerms is not NULL")
      CND_PRECONDITION(indexInfos == NULL,    "indexInfos is not NULL")
      CND_PRECONDITION(indexPointers == NULL, "indexPointers is not NULL")

      //Create the name of the term info index file and store it in buf
      const char_t* buf = segmentname(_T(".tii"));
	  //Open the file and return an InputStream reference to it
      InputStream* is = &directory.openFile( buf );

      //Check if is points to a valid instance
      CND_CONDITION(is != NULL,"is is NULL")

	  //buf is not necessary anymore so have it deleted
      delete[] buf;

      //Instantiate an enumeration
	  SegmentTermEnum* indexEnum = new SegmentTermEnum(*is, fieldInfos, true);

      //Check if indexEnum points to a valid instance
      CND_CONDITION(indexEnum != NULL,"No memory could be allocated for indexEnum")

      _TRY {
          indexTermsLength = indexEnum->size;

		  //Instantiate an array indexTerms which contains pointers to read Terms
          indexTerms    = new Term*[indexTermsLength];
          //Check if is indexTerms is a valid array
          CND_CONDITION(indexTerms != NULL,"No memory could be allocated for indexTerms")
		  //Instantiate an array indexInfos which contains pointer to TermInfo instances
          indexInfos    = new TermInfo*[indexTermsLength];
          //Check if is indexInfos is a valid array
          CND_CONDITION(indexInfos != NULL,"No memory could be allocated for indexInfos")
          //Instantiate an array indexPointers that contains pointers to the term info index file
          indexPointers = new long_t[indexTermsLength];
          //Check if is indexPointers is a valid array
          CND_CONDITION(indexPointers != NULL,"No memory could be allocated for indexPointers")

		  //Iterate through the terms of indexEnum
          for (int_t i = 0; indexEnum->next(); i++){
              indexTerms[i]    = indexEnum->getTerm();
              indexInfos[i]    = indexEnum->getTermInfo();
              indexPointers[i] = indexEnum->indexPointer;
              }
          }
     _FINALLY(
          indexEnum->close(); 
          delete indexEnum; 
		  //Close and delete the InputStream is. The close is done by the destructor.
          delete is;
          );
  }


  int_t TermInfosReader::getIndexOffset(const Term& term)  {
  //Func - Returns the offset of the greatest index entry which is less than or equal to term.
  //Pre  - term holds a reference to a valid term
  //       indexTerms != NULL
  //Post - The new offset has been returned

      //Check if is indexTerms is a valid array
      CND_PRECONDITION(indexTerms != NULL,"indexTerms is NULL")

      int_t lo = 0;					  
      int_t hi = indexTermsLength - 1;

      while (hi >= lo) {
          //Start in the middle betwee hi and lo
          int_t mid = (lo + hi) >> 1;

          //Check if is indexTerms[mid] is a valid instance of Term
          CND_PRECONDITION(indexTerms[mid] != NULL,"indexTerms[mid] is NULL")

		  //Determine if term is before mid or after mid
          int_t delta = term.compareTo(*indexTerms[mid]);
          if (delta < 0){
              //Calculate the new hi   
              hi = mid - 1;
              }
          else{
			  if (delta > 0){
                  //Calculate the new lo 
                  lo = mid + 1;
			      }
			  else{
                  //term has been found so return its position
                  return mid;
                  }
              }
          }
    // the new starting offset
    return hi;
  }

  void TermInfosReader::seekEnum(const int_t indexOffset) {
  //Func - Reposition the current Term and TermInfo to indexOffset
  //Pre  - indexOffset >= 0
  //       indexTerms    != NULL
  //       indexInfos    != NULL
  //       indexPointers != NULL
  //Post - The current Term and Terminfo have been repositioned to indexOffset

      CND_PRECONDITION(indexOffset >= 0, "indexOffset contains a negative number")
      CND_PRECONDITION(indexTerms != NULL,    "indexTerms is NULL")
      CND_PRECONDITION(indexInfos != NULL,    "indexInfos is NULL")
      CND_PRECONDITION(indexPointers != NULL, "indexPointers is NULL")

	  _enum->seek( 
          indexPointers[indexOffset],
		  (indexOffset * LUCENE_WRITER_INDEX_INTERVAL) - 1,
          *indexTerms[indexOffset], 
		  *indexInfos[indexOffset]
	      );
  }


  TermInfo* TermInfosReader::scanEnum(const Term& term) {
  //Func - Scans the Enumeration of terms for term and returns the corresponding TermInfo instance if found.
  //       The search is started from the current term.
  //Pre  - term contains a valid reference to a Term
  //       _enum != NULL
  //Post - if term has been found the corresponding TermInfo has been returned otherwise NULL
  //       has been returned

      CND_PRECONDITION(_enum != NULL, "_enum is NULL")

      //move the iterator of _enum to the position where term is expected to be
      _enum->scanTo(&term);

      //Check if the at the position the Term term can be found
	  if (_enum->getTerm(false) != NULL && term.compareTo(*_enum->getTerm(false)) == 0){
		  //Return the TermInfo instance about term
          return _enum->getTermInfo();
          }
	  else{
          //term was not found so no TermInfo can be returned
          return NULL;
          }
  }

  Term* TermInfosReader::scanEnum(const int_t position) {
  //Func - Scans the enumeration to the requested position and returns the
  //       Term located at that position
  //Pre  - position > = 0
  //       _enum != NULL
  //Post - The Term at the requested position has been returned

      CND_PRECONDITION(position >= 0, "position is a negative number")
      CND_PRECONDITION(_enum != NULL, "_enum is NULL")

	  //As long the position of the enumeration _enum is smaller than the requested one
      while(_enum->position < position){
		  //Move the current of _enum to the next
		  if (!_enum->next()){
			  //If there is no next it means that the requested position was to big
              return NULL;
              }
	      }

	  //Return the Term a the requested position
	  return _enum->getTerm();
  }

  const char_t* TermInfosReader::segmentname( const char_t* ext ){
  //Func - Return a filename by concatenating segment with ext
  //Pre  - ext != NULL, segment != NULL
  //Post - A filename which is the concatenation of segment and ext has been returned

      CND_PRECONDITION(ext != NULL, "ext is NULL")

      //Allocate a new buffer to hold the concatenated filename
      char_t* buf = new char_t[MAX_PATH];
      //Concatenate segment and ext and store result in buf
      stringPrintF(buf,_T("%s%s"), segment,ext );
      return buf;
  }

}}
