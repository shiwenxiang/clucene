#include "CLucene/StdHeader.h"
#ifndef CLUCENE_LITE
#include "SegmentMerger.h"

using namespace lucene::util;
namespace lucene{ namespace index {

  SegmentMerger::SegmentMerger(Directory& dir, const char_t* name): directory(dir){
  //Func - Constructor
  //Pre  - dir holds a valid reference to a Directory
  //       name != NULL
  //Post - Instance has been created

      CND_PRECONDITION(name != NULL, "name is NULL")

      freqOutput       = NULL;
      proxOutput       = NULL;
      termInfosWriter  = NULL;
      queue            = NULL;
      segment          = name;
      fieldInfos       = NULL;
      readers.setDoDelete(DELETE_TYPE_DELETE);
  }

  SegmentMerger::~SegmentMerger(){
  //Func - Destructor
  //Pre  - true
  //Post - The instance has been destroyed
      
	  //Clear the readers set
	  readers.clear();

	  //Delete field Infos
      _DELETE(fieldInfos);     
      //Close and destroy the OutputStream to the Frequency File
      if (freqOutput != NULL){ 
		  freqOutput->close(); 
		  _DELETE(freqOutput); 
	  }
      //Close and destroy the OutputStream to the Prox File
      if (proxOutput != NULL){
		  proxOutput->close(); 
		  _DELETE(proxOutput); 
	      }
      //Close and destroy the termInfosWriter
      if (termInfosWriter != NULL){
		  termInfosWriter->close(); 
		  _DELETE(termInfosWriter); 
	      }
      //Close and destroy the queue
      if (queue != NULL){
		  queue->close(); 
		  _DELETE(queue)
	      }
  }

  void SegmentMerger::add(SegmentReader& reader) {
  //Func - Adds a SegmentReader to the set of readers
  //Pre  - reader contains a valid reference to a SegmentReader
  //Post - The SegementReader reader has been added to the set of readers

      readers.push_back(&reader);
  }

  SegmentReader& SegmentMerger::segmentReader(const int_t i) {
  //Func - Returns a reference to the i-th SegmentReader
  //Pre  - 0 <= i < readers.size()
  //Post - A reference to the i-th SegmentReader has been returned

	  CND_PRECONDITION(i >= 0, "i is a negative number")
      CND_PRECONDITION(i < readers.size(), "i is bigger than the number of SegmentReader instances")

	  //Retrieve the i-th SegmentReader
      SegmentReader* ret = readers.at(i);

      CND_CONDITION(ret != NULL,"No SegmentReader found")	

      return *ret;
  }

  void SegmentMerger::merge() {
  //Func - Merge all the segments managed by the SegmentReaders in readers
  //Pre  - true
  //Post - All segments have been merged and all readers have been closed

      _TRY{
          //Merge the field infos
          mergeFields();
		  //Merge the terms
          mergeTerms();
		  //Merge the norms
          mergeNorms();      
		  } 
      _FINALLY( 

		  SegmentReader* reader = NULL;

		  //Iterate through all readers
          for (uint_t i = 0; i < readers.size(); i++) {  // close readers
			  //Get the reader
              reader = (SegmentReader*)readers.at(i);
              //Condition check to see if reader points to a valid instance
              CND_CONDITION(reader != NULL,"No SegmentReader found")	
			  //Close the reader
              reader->close();
              } 
		  );
  }

  const char_t* SegmentMerger::segmentname(const char_t* ext, const int_t x){
 //Func -  Returns an allocated buffer in which it creates a filename by 
  //       concatenating segment with ext and x
  //Pre    ext != NULL and holds the extension
  //       x contains a number
  //Post - A buffer has been instantiated an when x = -1 buffer contains the concatenation of 
  //       segment and ext otherwise buffer contains the contentation of segment, ext and x

	  CND_PRECONDITION(ext != NULL, "ext is NULL")

      char_t* buf = new char_t[MAX_PATH];
      if ( x==-1 )
          stringPrintF(buf,_T("%s%s"), segment,ext );
      else
          stringPrintF(buf,_T("%s%s%d"), segment,ext,x );
      return buf;
  }

  void SegmentMerger::mergeFields() {
  //Func - Merge the fields of all segments 
  //Pre  - true
  //Post - The field infos and field values of all segments have been merged.
		
	  //Create a new FieldInfos
      fieldInfos = new FieldInfos();		  // merge field names

      //Condition check to see if fieldInfos points to a valid instance
      CND_CONDITION(fieldInfos != NULL,"Memory allocation for fieldInfos failed")	

	  SegmentReader* reader = NULL;

	  //Iterate through all readers
      for (unsigned int_t i = 0; i < readers.size(); i++){
          //get the i-th reader
          reader = (SegmentReader*)readers.at(i);
          //Condition check to see if reader points to a valid instance
          CND_CONDITION(reader != NULL,"No SegmentReader found")	
		  //add the fieldInfos of the reader to fieldInfos
          fieldInfos->add(*reader->fieldInfos);
          }
		
	  //Create the filename of the new FieldInfos file
      const char_t* buf = segmentname(_T(".fnm"));
	  //Write the new FieldInfos file to the directory
      fieldInfos->write(directory, buf );
	  //Destroy the buffer of the filename
      delete[] buf;
	    
	  // merge field values


	  //Instantiate Fieldswriter which will write in directory for the segment name segment
      //Using the new merged fieldInfos
      FieldsWriter* fieldsWriter = new FieldsWriter(directory, segment, *fieldInfos);
      
	  //Condition check to see if fieldsWriter points to a valid instance
      CND_CONDITION(fieldsWriter != NULL,"Memory allocation for fieldsWriter failed")	

      _TRY {  
          SegmentReader* reader = NULL;
		  int_t maxDoc          = 0;
          //Iterate through all readers
          for (unsigned int_t i = 0; i < readers.size(); i++) {
              //get the i-th reader
              reader = (SegmentReader*)readers.at(i);

			  //Condition check to see if reader points to a valid instance
              CND_CONDITION(reader != NULL,"No SegmentReader found")				  

			  //Get the map of deleted documents of the current reader
              lucene::util::BitVector* deletedDocs = reader->deletedDocs;
			  //Get the total number documents including the documents that have been marked deleted
              int_t maxDoc = reader->MaxDoc();
                  
			  //Iterate through all the documents managed by the current reader
			  for (int_t j = 0; j < maxDoc; j++){
				  //Check if the j-th document has been deleted, if so skip it
                  if (deletedDocs == NULL || !deletedDocs->get(j)){ 
                      //Get the document
                      lucene::document::Document& doc = reader->document(j);
					  //Add the document to the new FieldsWriter
                      fieldsWriter->addDocument( doc );
                      //doc is not used anymore so have it deleted
                      delete &doc;
					  }
			      } 
              }
		  } 
      _FINALLY(
		  //Close the fieldsWriter
          fieldsWriter->close();
	      //And have it deleted as it not used any more
          delete fieldsWriter;
          );
  }

  void SegmentMerger::mergeTerms() {
  //Func - Merge the terms of all segments
  //Pre  - fieldInfos != NULL
  //Post - The terms of all segments have been merged

	  CND_PRECONDITION(fieldInfos != NULL, "fieldInfos is NULL")

      _TRY{
		  //create a filename for the new Frequency File for segment
          const char_t* buf = segmentname(_T(".frq"));
		  //Open an OutputStream to the new Frequency File
          freqOutput = &directory.createFile( buf );
          //Destroy the buffer of the filename
          delete[] buf;
		
		  //create a filename for the new Prox File for segment
          buf = segmentname(_T(".prx"));
		  //Open an OutputStream to the new Prox File
          proxOutput = &directory.createFile( buf );
		  //delete buffer
          delete[] buf;
		
		  //Instantiate  a new termInfosWriter which will write in directory
		  //for the segment name segment using the new merged fieldInfos
          termInfosWriter = new TermInfosWriter(directory, segment, *fieldInfos);  
          
          //Condition check to see if termInfosWriter points to a valid instance
          CND_CONDITION(termInfosWriter != NULL,"Memory allocation for termInfosWriter failed")	

		  //And merge the Term Infos
          mergeTermInfos();	      
          } 
	  _FINALLY(
		  //Close and destroy the OutputStream to the Frequency File
          if (freqOutput != NULL) 		{ freqOutput->close(); _DELETE(freqOutput); }
          //Close and destroy the OutputStream to the Prox File
          if (proxOutput != NULL) 		{ proxOutput->close(); _DELETE(proxOutput); }
		  //Close and destroy the termInfosWriter
          if (termInfosWriter != NULL) 	{ termInfosWriter->close(); _DELETE(termInfosWriter); }
		  //Close and destroy the queue
          if (queue != NULL)            { queue->close(); _DELETE(queue)}
		  );
  }

  void SegmentMerger::mergeTermInfos(){
  //Func - Merges all TermInfos into a single segment
  //Pre  - true
  //Post - All TermInfos have been merged into a single segment

	  //Instantiate a new SegementMergeQueue of size readers.size()
      queue = new SegmentMergeQueue(readers.size());

      //Condition check to see if queue points to a valid instance
      CND_CONDITION(queue != NULL,"Memory allocation for queue failed")	

	  //base is the id of the first document in a segment
      int_t base = 0;

      SegmentReader* reader = NULL;
	  SegmentMergeInfo* smi = NULL;

	  //iterate through all the readers
      for (unsigned int_t i = 0; i < readers.size(); i++) {
		  //Get the i-th reader
          reader = (SegmentReader*)readers.at(i);

          //Condition check to see if reader points to a valid instance
          CND_CONDITION(reader != NULL,"No SegmentReader found")				  

		  //Get the term enumeration of the reader
          SegmentTermEnum& termEnum = (SegmentTermEnum&)reader->getTerms();
          //Instantiate a new SegmentMerginfo for the current reader and enumeration
          smi = new SegmentMergeInfo(base, termEnum, *reader);

          //Condition check to see if smi points to a valid instance
          CND_CONDITION(smi != NULL,"Memory allocation for smi failed")	

		  //Increase the base by the number of documents that have not been marked deleted
		  //so base will contain a new value for the first document of the next iteration
          base += reader->NumDocs();
		  //Get the next current term
		  if (smi->next()){
              //Store the SegmentMergeInfo smi with the initialized SegmentTermEnum TermEnum
			  //into the queue
              queue->put(smi);
              }
          else{
			  //Apparently the end of the TermEnum of the SegmentTerm has been reached so
			  //close the SegmentMergeInfo smi
              smi->close();
			  //And destroy the instance and set smi to NULL (It will be used later in this method)
              _DELETE(smi);
              }
          }

	  //Instantiate an array of SegmentMergeInfo instances called match
      SegmentMergeInfo** match = new SegmentMergeInfo*[readers.size()];

      //Condition check to see if match points to a valid instance
      CND_CONDITION(match != NULL,"Memory allocation for match failed")	
     
      SegmentMergeInfo* top = NULL;

      //As long as there are SegmentMergeInfo instances stored in the queue
      while (queue->Size() > 0) {
          int_t matchSize = 0;			  
		  
		  // pop matching terms
          
		  //Pop the first SegmentMergeInfo from the queue
          match[matchSize++] = queue->pop();
		  //Get the Term of match[0]
          const Term* term = match[0]->term;

          //Condition check to see if term points to a valid instance
          CND_CONDITION(term != NULL,"term is NULL")	

          //Get the current top of the queue
		  top = queue->top();

          //For each SegmentMergInfo still in the queue 
		  //Check if term matches the term of the SegmentMergeInfo instances in the queue
          while (top != NULL && term->compareTo(*top->term) == 0){
			  //A match has been found so add the matching SegmentMergeInfo to the match array
              match[matchSize++] = queue->pop();
			  //Get the next SegmentMergeInfo
              top = queue->top();
              }

		  //add new TermInfo
          mergeTermInfo(match, matchSize);		  
		  
          //Restore the SegmentTermInfo instances in the match array back into the queue
          while (matchSize > 0){
              smi = match[--matchSize];
			  
              //Condition check to see if smi points to a valid instance
              CND_CONDITION(smi != NULL,"smi is NULL")	

			  //Move to the next term in the enumeration of SegmentMergeInfo smi
			  if (smi->next()){
                  //There still are some terms so restore smi in the queue
                  queue->put(smi);
                  }
              else{
				  //Done with a segment
				  //No terms anymore so close this SegmentMergeInfo instance
                  smi->close();				  
                  delete smi;
                  }
              }
          }

          _DELETE_ARRAY(match);
  }

  void SegmentMerger::mergeTermInfo( SegmentMergeInfo** smis, const int_t n){
  //Func - Merge the TermInfo of a term found in one or more segments. 
  //Pre  - smis != NULL and it contains segments that are positioned at the same term.
  //       n is equal to the number of SegmentMergeInfo instances in smis
  //       freqOutput != NULL
  //       proxOutput != NULL
  //Post - The TermInfo of a term has been merged

	  CND_PRECONDITION(smis != NULL, "smis is NULL")
	  CND_PRECONDITION(freqOutput != NULL, "freqOutput is NULL")
	  CND_PRECONDITION(proxOutput != NULL, "proxOutput is NULL")


	  //Get the file pointer of the OutputStream to the Frequency File
      long_t freqPointer = freqOutput->getFilePointer();
	  //Get the file pointer of the OutputStream to the Prox File
      long_t proxPointer = proxOutput->getFilePointer();

      //Process postings from multiple segments all positioned on the same term.
      int_t df = appendPostings(smis, n);  

	  //df contains the number of documents across all segments where this term was found
      if (df > 0) {
          //add an entry to the dictionary with pointers to prox and freq files
          termInfo.set(df, freqPointer, proxPointer);
          //Precondition check for to be sure that the reference to
		  //smis[0]->term will be valid
          CND_PRECONDITION(smis[0]->term != NULL, "smis[0]->term is NULL")
		  //Write a new TermInfo
          termInfosWriter->add(*smis[0]->term, termInfo);
          }
  }
	       
  int_t SegmentMerger::appendPostings(SegmentMergeInfo** smis, const int_t n){
  //Func - Process postings from multiple segments all positioned on the
  //       same term. Writes out merged entries into freqOutput and
  //       the proxOutput streams.
  //Pre  - smis != NULL and it contains segments that are positioned at the same term.
  //       n is equal to the number of SegmentMergeInfo instances in smis
  //       freqOutput != NULL
  //       proxOutput != NULL
  //Post - Returns number of documents across all segments where this term was found

      CND_PRECONDITION(smis != NULL, "smis is NULL")
	  CND_PRECONDITION(freqOutput != NULL, "freqOutput is NULL")
	  CND_PRECONDITION(proxOutput != NULL, "proxOutput is NULL")

      int_t lastDoc = 0;
      int_t df = 0;       //Document Counter
	  int_t doc;

      SegmentMergeInfo* smi = NULL;

	  //Iterate through all SegmentMergeInfo instances in smis
      for (int_t i = 0; i < n; i++) {
		  //Get the i-th SegmentMergeInfo 
          smi = smis[i];

          //Condition check to see if smi points to a valid instance
          CND_CONDITION(smi != NULL,"smi is NULL")	

		  //Get the term positions 
          SegmentTermPositions& postings = smi->postings;
		  //Get the base of this segment
          int_t base = smi->base;
		  //Get the docMap so we can see which documents have been deleted
          int_t* docMap = smi->docMap;
		  //Get the TermInfo for the current term in the enumeration of smi
          smi->termEnum.getTermInfo(termInfo);
		  //Seek the termpost
          postings.seek(&termInfo);
          while (postings.next()) {
             
			  //Check if there are deletions
			  if (docMap == NULL){
				  // no deletions
                  doc = base + postings.doc;		  
                  }
			  else{
				  //re-map around deletions
                  doc = base + docMap[postings.doc];	  
			      }

              //Condition check to see doc is eaqual to or bigger than lastDoc
              CND_CONDITION(doc >= lastDoc,"docs out of order")	

			  //Calculate a new docCode 
			  //use low bit to flag freq=1
              int_t docCode = (doc - lastDoc) << 1;	  
              lastDoc = doc;

			  //Get the frequency of the Term
              int_t freq = postings.freq;
              if (freq == 1){
                  //write doc & freq=1
                  freqOutput->writeVInt(docCode | 1);	  
                  }
              else{
				  //write doc
                  freqOutput->writeVInt(docCode);	  
				  //write frequency in doc
                  freqOutput->writeVInt(freq);		  
                  }
				  
			  int_t lastPosition = 0;			  
			  // write position deltas
			  for (int_t j = 0; j < freq; j++) {
				  //Get the next position
                  int_t position = postings.nextPosition();
				  //Write the difference between position and the last position
                  proxOutput->writeVInt(position - lastPosition);			  
                  lastPosition = position;
                  }
			   //Increase the total frequency over all segments
               df++;
               }
          }

      //Return total number of documents across all segments where term was found		
      return df;
  }

  void SegmentMerger::mergeNorms() {
  //Func - Merges the norms for all fields 
  //Pre  - fieldInfos != NULL
  //Post - The norms for all fields have been merged

      CND_PRECONDITION(fieldInfos != NULL, "fieldInfos is NULL")

	  SegmentReader* reader  = NULL;
	  InputStream*   input   = NULL;
	  OutputStream*  output  = NULL;

	  //iterate through all the Field Infos instances
      for (int_t i = 0; i < fieldInfos->size(); i++) {
		  //Get the i-th FieldInfo
          FieldInfo& fi = fieldInfos->fieldInfo(i);
		  //Is this Field indexed?
          if (fi.isIndexed){
			  //Create an new filename for the norm file
              const char_t* buf = segmentname(_T(".f"), i);
			  //Instantiate  an OutputStream to that norm file
              output = &directory.createFile( buf );

			  //Condition check to see if output points to a valid instance
              CND_CONDITION(output != NULL,"No Outputstream retrieved")	

              //Destroy the buffer of the filename
              delete[] buf;
              
			  _TRY{
				  //Iterate throug all SegmentReaders
                  for (unsigned int_t j = 0; j < readers.size(); j++) {
					  //Get the i-th SegmentReader
                      reader = (SegmentReader*)readers.at(j);

                      //Condition check to see if reader points to a valid instance
                      CND_CONDITION(reader != NULL,"No reader found")	

					  //Get an InputStream to the norm file for this field in this segment
                      input = reader->normStream(fi.name);

					  //Get the total number of documents including the documents that have been marked deleted
                      int_t maxDoc = reader->MaxDoc();
                      _TRY{
						  //Iterate through all the documents
                          for(int_t k = 0; k < maxDoc; k++) {
                              //Get the norm
							  //Note that the byte must always be read especially when the document
							  //is marked deleted to remain in sync
                              l_byte_t norm = input != NULL ? input->readByte() : (l_byte_t)0;

                              //Check if document k is deleted
						      if (!reader->isDeleted(k)){
								  //write the new norm
                                  output->writeByte(norm);
                                  }
                              }
                          } 
                      _FINALLY(
						  //Check if the InputStream input exists
                          if (input != NULL){
							  //Close it
                              input->close();
                              //Destroy it
                              _DELETE(input);
                              }
                          );
                      }
                  }
              _FINALLY(
				  if (output != NULL){
				      //Close the OutputStream output
                      output->close();
			          //destroy it
                      _DELETE(output);
				      }
				  );
			  }
		 }
  }

}}
#endif
