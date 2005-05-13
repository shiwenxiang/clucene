#include "CLucene/StdHeader.h"
#ifndef CLUCENE_LITE

#include "IndexWriter.h"

#ifdef USE_INFO_STREAM
# include <ostream>
#endif

#include "CLucene/document/Document.h"
#include "CLucene/store/Directory.h"
#include "CLucene/store/Lock.h"
#include "CLucene/store/TransactionalRAMDirectory.h"
#include "CLucene/util/VoidList.h"

#include "DocumentWriter.h"
#include "SegmentInfos.h"
#include "SegmentMerger.h"

using namespace std;
using namespace lucene::store;
using namespace lucene::util;
namespace lucene{ namespace index {

  IndexWriter::IndexWriter(const char_t* path, lucene::analysis::Analyzer& a, const bool create):
	directory( FSDirectory::getDirectory(path, create) ),
	analyzer(a),
	segmentInfos (*new SegmentInfos){
  //Func - Constructor
  //       Constructs an IndexWriter for the index in path.
  //Pre  - path != NULL and contains a named directory path
  //       a holds a valid reference to an analyzer and analyzes the text to be indexed
  //       create indicates if the indexWriter must create a new index located at path or just open it
  //Post - If create is true, then a new, empty index has been created in path, replacing the index
  //       already there, if any. The named directory path is owned by this Instance

	  CND_PRECONDITION(path != NULL, "path is NULL")

	  //This instance owns the directory named path
	  ownDir = true;
	  //Continue initializing the instance by _IndexWriter
	  _IndexWriter ( create );
  }

  IndexWriter::IndexWriter(lucene::store::Directory& d, lucene::analysis::Analyzer& a, const bool create):
	  directory(d),
	  analyzer(a),
	  segmentInfos (*new SegmentInfos){
  //Func - Constructor
  //       Constructs an IndexWriter for the index in path.
  //Pre  - d contains a valid reference to a directory
  //       a holds a valid reference to an analyzer and analyzes the text to be indexed
  //       create indicates if the indexWriter must create a new index located at path or just open it
  //Post - If create is true, then a new, empty index has been created in path, replacing the index
  //       already there, if any. The directory d is not owned by this Instance

	  //This instance does not own the directory
	  ownDir     = false;
	  //Continue initializing the instance by _IndexWriter
	  _IndexWriter ( create );
  }

  void IndexWriter::_IndexWriter(const bool create){
  //Func - Initialises the instances
  //Pre  - create indicates if the indexWriter must create a new index located at path or just open it
  //Post -

	maxFieldLength = IndexWriter::FIELD_TRUNC_POLICY__WARN;

	//Create a ramDirectory
	ramDirectory = new lucene::store::TransactionalRAMDirectory;

	CND_CONDITION(ramDirectory != NULL,"ramDirectory is NULL")

	//Reset the infoStream to NULL
	infoStream = NULL;
	//Initialize the writeLock to
	writeLock  = NULL;
	//Initialize the mergeFactor to 10 indicating that a merge will occur after 10 documents
	//have been added to the index managed by this IndexWriter
	mergeFactor = 10;
	//Initialize maxMergeDocs to INT_MAX
	maxMergeDocs = INT_MAX;

	//Create a new lock using the name "write.lock"
	LuceneLock* newLock = directory.makeLock(_T("write.lock"));

	//Condition check to see if newLock has been allocated properly
	CND_CONDITION(newLock != NULL, "No memory could be allocated for LuceneLock newLock")

	//Try to obtain a write lock
	if (!newLock->obtain()){
		//Write lock could not be obtained so delete it
		_DELETE(newLock);
		//Reset the instance
		_finalize();
		//throw an exception because no writelock could be created or obtained
		_THROWC( "Index locked for write or no write access." );
		}

	//The Write Lock has been obtained so save it for later use
	writeLock = newLock;

	//Create a new lock using the name "commit.lock"
	LuceneLock* lock = directory.makeLock(_T("commit.lock"));

	//Condition check to see if lock has been allocated properly
	CND_CONDITION(lock != NULL, "No memory could be allocated for LuceneLock lock")

	IndexWriterLockWith with ( lock,this,create );

	LOCK_MUTEX(DIRECTORIES_MUTEX); // in- & inter-process sync
	with.run();
	UNLOCK_MUTEX(DIRECTORIES_MUTEX);

	//Release the commit lock
	_DELETE(lock);
  }

  void IndexWriter::_finalize(){
  //Func - Releases all the resources of the instance
  //Pre  - true
  //Post - All the releases have been released

	  if(writeLock != NULL){
		  //release write lock
		  writeLock->release();
		  _DELETE( writeLock );
		  }

	  //Delete the ramDirectory
	  _DELETE(ramDirectory)

	  //Check if segmentInfos is valid
	  if(&segmentInfos != NULL){
		  delete &segmentInfos;
		  }
  }

  IndexWriter::~IndexWriter() {
  //Func - Destructor
  //Pre  - true
  //Post - The instance has been destroyed

	  _finalize();
  }


  void* IndexWriterLockWith::doBody() {
  //Func - Writes segmentInfos to or reads  segmentInfos from disk
  //Pre  - writer != NULL
  //Post - if create is true then segementInfos has been written to disk otherwise
  //       segmentInfos has been read from disk

	  CND_PRECONDITION(writer != NULL, "writer is NULL")

	  if (create){
		  writer->segmentInfos.write(writer->directory);
		  }
	  else{
		  writer->segmentInfos.read(writer->directory);
		  }

	  return NULL;
  }

  void* IndexWriterLockWith2::doBody(){
  //Func - Writes the segmentInfos to Disk and deletes unused segments
  //Pre  - writer != NULL
  //Post - segmentInfos have been written to disk and unused segments have been deleted

	  CND_PRECONDITION(writer != NULL, "writer is NULL")

	  //commit before deleting
	  writer->segmentInfos.write(writer->directory);
	  //delete now-unused segments
	  writer->deleteSegments(*segmentsToDelete);

	  return NULL;
  }

  void IndexWriter::close( const bool closeDir ) {
  //Func - Flushes all changes to an index, closes all associated files, and closes
  //       the directory that the index is stored in.
  //Pre  - closeDir indicates if the directory must be closed or not
  //Post - All the changes have been flushed to disk and the write lock has been released
  //       The ramDirectory has also been closed. The directory has been closed
  //       if closeDir = true or ownDir = true

	  LOCK_MUTEX(close_LOCK);

	  //Flush the Ram Segments
	  flushRamSegments();
	  //Close the ram directory
	  ramDirectory->close();

	  //Check if this instance must close the directory
	  if (closeDir || ownDir){
		  directory.close();
		  }
	  // release write lock
	  if (writeLock != NULL){
		  writeLock->release();
		  _DELETE( writeLock );
		  }

	UNLOCK_MUTEX(close_LOCK);
  }

  int_t IndexWriter::docCount(){
  //Func - Counts the number of documents in the index
  //Pre  - true
  //Post - The number of documents have been returned

	  //Initialize count
	  int_t count = 0;

	  LOCK_MUTEX(docCount_LOCK);
	  //Iterate through all segmentInfos
	  for (uint_t i = 0; i < segmentInfos.size(); i++) {
		  //Get the i-th SegmentInfo
		  SegmentInfo& si = segmentInfos.info(i);
		  //Retrieve the number of documents of the segment and add it to count
		  count += si.docCount;
		  }
	  UNLOCK_MUTEX(docCount_LOCK);


	  return count;
  }

  void IndexWriter::addDocument(lucene::document::Document& doc) {
  //Func - Adds a document to the index
  //Pre  - doc contains a valid reference to a document
  //       ramDirectory != NULL
  //Post - The document has been added to the index of this IndexWriter
	LOCK_MUTEX(THIS_LOCK); //don't need to use try/finally, because using the mutexguard
		CND_PRECONDITION(ramDirectory != NULL,"ramDirectory is NULL")

		ramDirectory->transStart();
		try {
			char_t* segmentName = newSegmentName();
			CND_CONDITION(segmentName != NULL, "segmentName is NULL")
			_TRY {
				//Create the DocumentWriter using a ramDirectory and analyzer
				// supplied by the IndexWriter (this).
				DocumentWriter* dw = new DocumentWriter(
					*ramDirectory, analyzer, maxFieldLength
				  );
				CND_CONDITION(dw != NULL, "dw is NULL")
				_TRY {
					//Add the client-supplied document to the new segment.
					dw->addDocument(segmentName, doc);
				} _FINALLY(
					_DELETE(dw);
				);

				//Create a new SegmentInfo instance about this new segment.
				SegmentInfo *Si = new SegmentInfo(segmentName, 1, *ramDirectory);
				CND_CONDITION(Si != NULL, "Si is NULL")

				//Add the info object for this particular segment to the list
				// of all SegmentInfos.
				segmentInfos.push_back(Si);
			} _FINALLY(
				_DELETE_ARRAY(segmentName);
			);

			//Check to see if the segments must be merged
			maybeMergeSegments();
		} catch (...) {
			ramDirectory->transAbort();
			throw;
		}
		ramDirectory->transCommit();

	UNLOCK_MUTEX(THIS_LOCK);
  }


  void IndexWriter::optimize() {
  //Func - Optimizes the index for which this Instance is responsible
  //Pre  - true
  //Post -

	  LOCK_MUTEX(optimize_LOCK);
	  //Flush the RamSegments to disk
	  flushRamSegments();

	  while (segmentInfos.size() > 1 ||
			(segmentInfos.size() == 1 &&
				(SegmentReader::hasDeletions(segmentInfos.info(0)) ||&segmentInfos.info(0).dir !=  &directory ))){

		  int_t minSegment = segmentInfos.size() - mergeFactor;

		  mergeSegments(minSegment < 0 ? 0 : minSegment);
	  }
	  UNLOCK_MUTEX(optimize_LOCK);
  }

  void IndexWriter::addIndexes(Directory** dirs, const int_t dirsLength) {
  //Func - Add several indexes located in different directories into the current
  //       one managed by this instance
  //Pre  - dirs != NULL and contains directories of several indexes
  //       dirsLength > 0 and contains the number of directories
  //Post - The indexes located in the directories in dirs have been merged with
  //       the pre(current) index. The Resulting index has also been optimized

	  CND_PRECONDITION(dirs != NULL, "dirs is NULL")
	  CND_PRECONDITION(dirsLength > 0, "dirsLength is a negative number")

	  LOCK_MUTEX(addIndexes_LOCK);
	  // start with zero or 1 seg so optimize the current
	  optimize();

	  //Iterate through the directories
	  for (int_t i = 0; i < dirsLength; i++) {
		  // DSR: Changed SegmentInfos constructor arg (see bug discussion below).
		  SegmentInfos sis(false);
		  sis.read( *dirs[i]);

		  for (uint_t j = 0; j < sis.size(); j++) {
		  /* DSR:CL_BUG:
		  ** In CLucene 0.8.11, the next call placed a pointer to a SegmentInfo
		  ** object from stack variable $sis into the vector this->segmentInfos.
		  ** Then, when the call to optimize() is made just before exiting this
		  ** function, $sis had already been deallocated (and has deleted its
		  ** member objects), leaving dangling pointers in this->segmentInfos.
		  ** I added a SegmentInfos constructor that allowed me to order it not
		  ** to delete its members, invoked the new constructor form above for
		  ** $sis, and the problem was solved. */
		  segmentInfos.push_back(&sis.info(j));	  // add each info
		  }
	}
	optimize();					  // cleanup
	UNLOCK_MUTEX(addIndexes_LOCK);
  }


  void IndexWriter::flushRamSegments(){
  //Func - Merges all RAM-resident segments.
  //Pre  - ramDirectory != NULL
  //Post - The RAM-resident segments have been merged to disk

	CND_PRECONDITION(ramDirectory != NULL,"ramDirectory is NULL")

	int_t minSegment = segmentInfos.size()-1; //don't make this unsigned...
	CND_CONDITION(minSegment >= -1, "minSegment must be >= -1")

	int_t docCount = 0;
	//Iterate through all the segements and check if the directory is a ramDirectory
	while (minSegment >= 0 &&
	  &segmentInfos.info(minSegment).dir == ramDirectory) {
	  docCount += segmentInfos.info(minSegment).docCount;
	  minSegment--;
	}
	if (minSegment < 0 ||			  // add one FS segment?
		(docCount + segmentInfos.info(minSegment).docCount) > mergeFactor ||
		!(&segmentInfos.info(segmentInfos.size()-1).dir == ramDirectory))
	  minSegment++;

	CND_CONDITION(minSegment >= 0, "minSegment must be >= 0")
	if (static_cast<uint_t>(minSegment) >= segmentInfos.size())
	  return;					  // none to merge
	mergeSegments(minSegment);
  }

  void IndexWriter::maybeMergeSegments() {
  //Func - Incremental Segment Merger
  //Pre  -
  //Post -

	long_t targetMergeDocs = mergeFactor;

	// find segments smaller than current target size
	while (targetMergeDocs <= maxMergeDocs) {
		int_t minSegment = segmentInfos.size();
		int_t mergeDocs = 0;

		while (--minSegment >= 0) {
			SegmentInfo& si = segmentInfos.info(minSegment);
			if (si.docCount >= targetMergeDocs)
				break;
			mergeDocs += si.docCount;
			}

		if (mergeDocs >= targetMergeDocs){
			// found a merge to do
			mergeSegments(minSegment+1);
			}
		else
			break;

		//increase target size
		targetMergeDocs *= mergeFactor;
		}
  }

  void IndexWriter::mergeSegments(const uint_t minSegment) {
	const char_t* mergedName = newSegmentName();
	int_t mergedDocCount = 0;
#ifdef USE_INFO_STREAM
	if (infoStream != NULL)
	  *infoStream<< "merging segments" << "\n";
#endif
	SegmentMerger merger(directory, mergedName);
	lucene::util::VoidList<SegmentReader*> segmentsToDelete;
	for (uint_t i = minSegment; i < segmentInfos.size(); i++) {
	  SegmentInfo& si = segmentInfos.info(i);
#ifdef USE_INFO_STREAM
	  if ( infoStream != NULL)
		*infoStream << " " << si.name << " (" << si.docCount << " docs)";
#endif
	  SegmentReader* reader = new SegmentReader(si,false);
	  merger.add(*reader);
	  if ((&reader->directory == &this->directory) || // if we own the directory
		(&reader->directory == this->ramDirectory))
		segmentsToDelete.push_back(reader);	  // queue segment for deletion

	  mergedDocCount += si.docCount;
	}
#ifdef USE_INFO_STREAM
	if (infoStream != NULL) {
	  *infoStream<<"\n into "<<mergedName<<" ("<<mergedDocCount<<" docs)";
	}
#endif
	merger.merge();

	//TODO: check this code. Should delete on pop?
	//segmentInfos.setSize(minSegment);		  // pop old infos & add new
	while ( segmentInfos.size() > minSegment )
	  segmentInfos.pop_back();

	segmentInfos.push_back( new SegmentInfo(mergedName, mergedDocCount, directory));



	LuceneLock* lock = directory.makeLock(_T("commit.lock"));
	IndexWriterLockWith2 with ( lock,this,&segmentsToDelete );

	LOCK_MUTEX(DIRECTORIES_MUTEX); // in- & inter-process sync
	with.run();
	UNLOCK_MUTEX(DIRECTORIES_MUTEX);

	delete lock;
	delete[] mergedName; //ADD:
  }


  void IndexWriter::deleteSegments(lucene::util::VoidList<SegmentReader*> &segments) {
	StringArrayConst deletable;
	deletable.setDoDelete(DELETE_TYPE_DELETE_ARRAY);

	StringArrayConst &deleteArray = readDeleteableFiles();
	deleteFiles(deleteArray, deletable); // try to delete deleteable
	delete &deleteArray;

	for (uint_t i = 0; i < segments.size(); i++) {
	  SegmentReader* reader = segments.at(i);
	  StringArrayConst& files = reader->files();
	  if (&reader->directory == &this->directory)
		deleteFiles(files, deletable);	  // try to delete our files
	  else
		deleteFiles(files, reader->directory); // delete, eg, RAM files

	  delete &files;
	}

	writeDeleteableFiles(deletable);		  // note files we can't delete
  }

  void IndexWriter::deleteFiles(const StringArrayConst& files, lucene::store::Directory& directory) {
	for (uint_t i = 0; i < files.size(); i++)
	  directory.deleteFile( files[i] );
  }

  void IndexWriter::deleteFiles(const StringArrayConst& files, StringArrayConst& deletable) {
	for (uint_t i = 0; i < files.size(); i++) {
	  const char_t* file = files[i];
	  try {
		directory.deleteFile(file);		  // try to delete each file
	  } catch (THROW_TYPE e) {			  // if delete fails
		if (directory.fileExists(file)) {
#ifdef USE_INFO_STREAM
		  if (infoStream != NULL)
			*infoStream << e.errstr << "; Will re-try later.\n";
#endif
		  deletable.push_back(stringDuplicate(file));		  // add to deletable
		}
	  }
	}
  }

  StringArrayConst& IndexWriter::readDeleteableFiles() {
	StringArrayConst& result = *new StringArrayConst(true,DELETE_TYPE_DELETE_ARRAY);

	if (!directory.fileExists(_T("deletable")))
	  return result;

	InputStream& input = directory.openFile(_T("deletable"));
	_TRY {
	  for (int_t i = input.readInt(); i > 0; i--)	  // read file names
		result.push_back(input.readString());
	} _FINALLY(
		input.close();
		delete &input;
	);


	return result;
  }

  void IndexWriter::writeDeleteableFiles(StringArrayConst& files) {
	OutputStream& output = directory.createFile(_T("deleteable.new"));
	_TRY {
	  output.writeInt(files.size());
	  for (uint_t i = 0; i < files.size(); i++)
		output.writeString( files.at(i) );
	} _FINALLY(
		output.close();
		delete &output;
	);

	directory.renameFile(_T("deleteable.new"), _T("deletable"));
  }

  char_t* IndexWriter::newSegmentName() {
	char_t buf[9];
	LOCK_MUTEX(newSegmentName_LOCK);
//    integerToString(segmentInfos.counter++,buf,CHAR_RADIX); //36 is RADIX of 10 digits and 26 numbers
	integerToString(segmentInfos.counter++,buf,9);

	UNLOCK_MUTEX(newSegmentName_LOCK);
	return lucene::util::Misc::join( _T("_"),buf);
  }
}}
#endif
