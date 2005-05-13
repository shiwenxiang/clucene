#include "CLucene/StdHeader.h"
#include "SegmentInfos.h"

#include "CLucene/store/Directory.h"
#include "SegmentInfo.h"
#include "CLucene/util/VoidMap.h"

namespace lucene{ namespace index {

  SegmentInfos::SegmentInfos()
      :lucene::util::VoidList<SegmentInfo*>(true,DELETE_TYPE_DELETE){
  //Func - Constructor
  //Pre  - true
  //Post - An instance of SegmentInfos has been created. During creation
  //       the instance is told to delete all the SegmentInfo instances it
  //       manages when the instance is destroyed

	  //initialize counter to 0
      counter = 0;
  }

  SegmentInfos::SegmentInfos(bool deleteMembers) :
      lucene::util::VoidList<SegmentInfo*>(deleteMembers,DELETE_TYPE_DELETE){
  //Func - Constructor
  //       This (bool) form of constructor has been added to allow client to instantiate
  //       SegmentInfos object that does not delete its members upon its own deletion.  
  //       This change was prompted by a bug in IndexWriter::addIndexes.
  //Pre  - deleteMembers indicates if the instance to be created must delete
  //       all SegmentInfo instances it manages when the instance is destroyed or not
  //       true -> must delete, false may not delete
  //Post - An instance of SegmentInfos has been created.
  
      //initialize counter to 0
      counter = 0;
  }

  SegmentInfos::~SegmentInfos(){
  //Func - Destructor
  //Pre  - true
  //Post - The instance has been destroyed. Depending on the constructor used
  //       the SegmentInfo instances that this instance managed have been deleted or not.

	  //Clear the list of SegmentInfo instances
      clear();
  }
  
  SegmentInfo& SegmentInfos::info(int_t i) {
  //Func - Returns a reference to the i-th SegmentInfo in the list.
  //Pre  - i >= 0
  //Post - A reference to the i-th SegmentInfo instance has been returned

      CND_PRECONDITION(i >= 0, "i contains negative number")

	  //Get the i-th SegmentInfo instance
      SegmentInfo *ret = at(i);
      //Condition check to see if the i-th SegmentInfo has been retrieved
      CND_CONDITION(ret != NULL,"No SegmentInfo instance found")	

      return *ret;
  }

  void SegmentInfos::read(Directory& directory){
  //Func - Reads segments file that resides in directory. 
  //Pre  - directory contains a valid reference
  //Post - The segments file has been read and for each segment found
  //       a SegmentsInfo intance has been created and stored.

	  //Open an InputStream to the segments file
      InputStream* input = &directory.openFile(_T("segments"));
	  //Check if input is valid
	  if (input){
        _TRY {
            //read counter
            counter = input->readInt();		  

            //Temporary variable for storing the name of the segment
            char_t* name     = NULL;
            SegmentInfo* si  = NULL;

            //read segmentInfos
            for (int_t i = input->readInt(); i > 0; i--){ 
                // read the name of the segment
                name = input->readString();
                //Instantiate a new SegmentInfo Instance
                si = new SegmentInfo(name, input->readInt(),directory);
                //Condition check to see if si points to an instance
                CND_CONDITION(si != NULL,"Memory allocation for si failed")	
                //delete name
                delete[] name;
                //store SegmentInfo si
                push_back(si);
                } 
           } 
        _FINALLY(
            //destroy the inputStream input. The destructor of InputStream will 
		    //also close the Inputstream input
            delete input;
            );
	  }
  }

#ifndef CLUCENE_LITE
  void SegmentInfos::write(Directory& directory){
  //Func - Writes a new segments file based upon the SegmentInfo instances it manages
  //Pre  - directory is a valid reference to a Directory
  //Post - The new segment has been written to disk
    
	  //Open an OutputStream to the segments file
	  OutputStream* output = &directory.createFile(_T("segments.new"));
	   //Check if output is valid
	  if (output){
          _TRY {
			  //Write the counter
              output->writeInt(counter);
			  //Write the number of SegmentInfo Instances
			  //which is equal to the number of segments in directory as
			  //each SegmentInfo manages a single segment
              output->writeInt(size());			  

			  SegmentInfo *si = NULL;

			  //Iterate through all the SegmentInfo instances
              for (uint_t i = 0; i < size(); i++) {
				  //Retrieve the SegmentInfo
                  si = &info(i);
                  //Condition check to see if si has been retrieved
                  CND_CONDITION(si != NULL,"No SegmentInfo instance found")	
				  //Write the name of the current segment
                  output->writeString(si->name);
				  //Write the number of documents in the segment 
                  output->writeInt(si->docCount);
                  }
              } 
	      _FINALLY(
              output->close();
              delete output;
              );

          // install new segment info
          directory.renameFile(_T("segments.new"), _T("segments"));
	      }
  }
#endif //CLUCENE_LITE

}}
