#include "CLucene/StdHeader.h"
#include "FSDirectory.h"

#include "CLucene/util/Misc.h"
namespace lucene{ 	namespace store{
using namespace std;
using namespace lucene::util;

  FSInputStream::FSInputStream(const char_t* path) {
  //Func - Constructor.
  //       Opens the file named path
  //Pre  - path != NULL
  //Post - if the file could not be opened  an exception is thrown.

	  CND_PRECONDITION(path != NULL, "path is NULL")

	  //Open the file
	  fhandle  = openFile(path, O_BINARY | O_RDONLY | O_RANDOM, _S_IREAD );

	  //Check if a valid handle was retrieved
	  if (fhandle == -1){
		  //Throw an exception.
		  _THROWC("File IO Open error");
	  }
	  //Store the file length
	  length = fileSize(fhandle);

	  //Keep in mind that this instance is not a clone
	  isClone = false;
  }

  FSInputStream::FSInputStream(FSInputStream& clone): InputStream(clone){
  //Func - Constructor
  //       Uses clone for its initialization
  //Pre  - clone is a valide instance of FSInputStream
  //Post - The instance has been created and initialized by clone

	  //clone the file handle
	  fhandle = clone.fhandle;
	  //clone the file length
	  length  = clone.length;
	  //Keep in mind that this instance is a clone
	  isClone = true;
  }

  FSInputStream::~FSInputStream(){
  //Func - Destructor
  //Pre  - True
  //Post - The file for which this instance is responsible has been closed.
  //       The instance has been destroyed

	  close();
  }

  InputStream& FSInputStream::clone()
  {
	return *new FSInputStream(*this);
  }
  void FSInputStream::close()  {
	InputStream::close();
	if (!isClone && fhandle != 0){
	  if ( _close(fhandle) != 0 )
		_THROWC( "File IO Close error");
	  else
		fhandle = 0;
	}
  }

  void FSInputStream::seekInternal(const long_t position)  {
  }

  /** InputStream methods */
void FSInputStream::readInternal(l_byte_t* b, int_t offset, const int_t len) {
	LOCK_MUTEX(file_mutex);
	  long_t position = getFilePointer();

	  if (position != fileTell(fhandle) ) {
		if ( fileSeek(fhandle,position,SEEK_SET) == -1 ){
		  //UNLOCK_MUTEX(file_mutex);
		  _THROWC( "File IO Seek error");
		}
	  }
	  //TODO check that this is right. Code is fairly different than
	  //the java code.
	  bufferLength = _read(fhandle,b+offset,len); // 2004.10.31:SF 1037836
	  if (bufferLength == 0){
		//UNLOCK_MUTEX(file_mutex);
		_THROWC( "read past EOF");
	  }
	  if (bufferLength == -1){
		//UNLOCK_MUTEX(file_mutex);
		_THROWC( "read error");
	  }
	UNLOCK_MUTEX(file_mutex)
  }
#ifndef CLUCENE_LITE
  FSOutputStream::FSOutputStream(const char_t* path){
	//O_BINARY - Opens file in binary (untranslated) mode
	//O_CREAT - Creates and opens new file for writing. Has no effect if file specified by filename exists
	//O_RANDOM - Specifies that caching is optimized for, but not restricted to, random access from disk.
	//O_WRONLY - Opens file for writing only;
	if ( lucene::util::Misc::dir_Exists(path) ){
	  fhandle = openFile( path, O_BINARY | O_RDWR | O_RANDOM | O_TRUNC, _S_IREAD | _S_IWRITE);
	  //fileSeek( fhandle, 0L, SEEK_END );
	}
	else
	  // added by JBP
	  fhandle = openFile( path, O_BINARY | O_RDWR | O_RANDOM | O_CREAT, _S_IREAD | _S_IWRITE);

	if ( fhandle == -1 )
	  _THROWC( "File IO Open error");
  }
  FSOutputStream::~FSOutputStream(){
	if ( fhandle != 0 ){
	  try{
		flush();
	  }catch(...){
	  }
	}
  }

  /** output methods: */
  void FSOutputStream::flushBuffer(const l_byte_t* b, const int_t size) {
	  if ( size > 0 && _write(fhandle,b,size) != size )
		_THROWC( "File IO Write error");
  }
  void FSOutputStream::close() {
	try{
	  OutputStream::close();
	}catch(...){}

	if ( _close(fhandle) != 0 )
	  _THROWC( "File IO Close error");
	else
	  fhandle = 0;
  }

  /** Random-access methods */
  void FSOutputStream::seek(const long_t pos) {
	OutputStream::seek(pos);
	if ( fileSeek(fhandle,pos,SEEK_SET) != 0 )
	  _THROWC( "File IO Seek error");
  }
  long_t FSOutputStream::Length(){
	return fileSize(fhandle);
  }
#endif //CLUCENE_LITE





  FSDirectory::FSDirectory(const char_t* path, const bool createDir)
  {
	stringCopy(directory,path);
	refCount=0;
	if (createDir)
	  create();

	if (!lucene::util::Misc::dir_Exists(directory)){
	  StringBuffer e(path);
	  e.append(_T(" not a directory"));
	  _THROWX( e.getBuffer());
	}
  }

  void FSDirectory::create(){
	LOCK_MUTEX(FSDIR_CREATE);
	if ( !lucene::util::Misc::dir_Exists(directory) )
	  if ( makeDirectory(directory) == -1 ){
		StringBuffer e(_T("Cannot create directory: "));
		e.append(directory);
		//UNLOCK_MUTEX(FSDIR_CREATE);
		_THROWX( e.getBuffer() );
	  }

	DIR* dir = opendir(directory);
	struct dirent* fl = readdir(dir);
	struct Struct_Stat buf;

	char_t path[CL_MAX_DIR];
	while ( fl != NULL ){
	  stringPrintF(path,_T("%s/%s"),directory,fl->d_name);
	  int_t ret = Cmd_Stat(path,&buf);
	  if ( ret==0 && !(buf.st_mode & S_IFDIR) ) {
		if ( (stringCompare(fl->d_name, _T("."))) && (stringCompare(fl->d_name, _T(".."))) ) {
		if ( unlinkFile( path ) == -1 ) {
		  //UNLOCK_MUTEX(FSDIR_CREATE);
		  closedir(dir);
		  _THROWC( "Couldn't delete file ");
		}
		}
	  }
	  fl = readdir(dir);
	}
	closedir(dir);
	UNLOCK_MUTEX(FSDIR_CREATE);
  }

  void FSDirectory::priv_getFN(char_t* buffer, const char_t* name) const{
	  buffer[0] = 0;
	  stringCopy(buffer,directory);
	  stringCat(buffer, PATH_DELIMITER );
	  stringCat(buffer,name);
  }
  int_t FSDirectory::priv_getStat(const char_t* name, struct Struct_Stat* ret) const{
	char_t buffer[CL_MAX_DIR];
	priv_getFN(buffer,name);
	return Cmd_Stat( buffer, ret );
  }

  FSDirectory::~FSDirectory(){
	DIRECTORIES.remove( getDirName());
  }

  void FSDirectory::list(char_t**& list, int_t& size) {
	DIR* dir = opendir(directory);
	struct dirent* fl = readdir(dir);
	struct Struct_Stat buf;
	VoidList<char_t*> names;

	char_t path[CL_MAX_DIR];
	stringCopy(path,directory);
	stringCat(path,PATH_DELIMITER);
	char_t* pathP = path + stringLength(path);

	while ( fl != NULL ){
	  stringCopy(pathP,fl->d_name); // 2004.10.31:SF 1047357
	  Cmd_Stat(path,&buf);
	  if ( buf.st_mode & S_IFDIR ) {
		names.put( stringDuplicate(fl->d_name) );
		fl = readdir(dir);
	  }
	}
	closedir(dir);

	size = names.size();
	list = new char_t*[size];
	for ( int i=0;i<size;i++ )
	  list[i] = names[i];
  }

  bool FSDirectory::fileExists(const char_t* name) const {
	char_t fl[CL_MAX_DIR];
	priv_getFN(fl, name);
	return lucene::util::Misc::dir_Exists( fl );
  }

  const char_t* FSDirectory::getDirName() const{
	return directory;
  }

  //static
  FSDirectory& FSDirectory::getDirectory(const char_t* file, const bool create){
	LOCK_MUTEX(DIRECTORIES_MUTEX);
	if ( DIRECTORIES.exists(file)  ){
	  FSDirectory* itm = (FSDirectory*)DIRECTORIES.get(file);
	  if ( create )
		itm->create();

	  LOCK_MUTEX(itm->LOCK_MUTEX);
	  itm->refInc();
	  UNLOCK_MUTEX(itm->LOCK_MUTEX);

	  //UNLOCK_MUTEX(DIRECTORIES_MUTEX);
	  return *itm;
	}
	UNLOCK_MUTEX(DIRECTORIES_MUTEX);

	FSDirectory* fs = new FSDirectory(file, create);

	LOCK_MUTEX(fs->LOCK_MUTEX);
	fs->refInc();
	UNLOCK_MUTEX(fs->LOCK_MUTEX);

	DIRECTORIES.put( stringDuplicate(file), fs);
	return *fs;
  }

  long_t FSDirectory::fileModified(const char_t* name) const {
	struct Struct_Stat buf;
	if ( priv_getStat(name,&buf) == -1 )
	  return 0;
	else
	  return buf.st_mtime;
  }

  //static
  long_t FSDirectory::fileModified(const char_t* dir, const char_t* name){
	struct Struct_Stat buf;
	char_t buffer[CL_MAX_DIR];
	stringCopy(buffer,dir);
	stringCat(buffer,PATH_DELIMITER);
	stringCat(buffer,name);
	Cmd_Stat( buffer, &buf );
	return buf.st_mtime;
  }

  long_t FSDirectory::fileLength(const char_t* name) const {
	struct Struct_Stat buf;
	if ( priv_getStat(name, &buf) == -1 )
	  return 0;
	else
	  return buf.st_size;
  }


  InputStream& FSDirectory::openFile(const char_t* name) {
	char_t fl[CL_MAX_DIR];
	priv_getFN(fl, name);
	return *new FSInputStream( fl );
  }

  void FSDirectory::refInc(){
	refCount ++;
  }

  void FSDirectory::close(){
	  int_t adjustedRefCount;

	  LOCK_MUTEX(FSDIR_CLOSE);
	  //Decrease the refcount and store it for later use when the mutex is unlocked
	  adjustedRefCount = --refCount;
	  UNLOCK_MUTEX(FSDIR_CLOSE);
	  if (adjustedRefCount <= 0) {
		  LOCK_MUTEX(DIRECTORIES_MUTEX);
		  Directory* d = DIRECTORIES.get(getDirName());
		  if (d){
			  DIRECTORIES.remove( getDirName());
			  delete d ;
			  }
		  UNLOCK_MUTEX(DIRECTORIES_MUTEX);
		  }
   }
#ifndef CLUCENE_LITE

  void FSDirectory::deleteFile(const char_t* name, const bool throwError)  {
	char_t fl[CL_MAX_DIR];
	priv_getFN(fl, name);
	if ( unlinkFile(fl) == -1 && throwError){
	  char_t buffer[200];
	  stringPrintF(buffer,_T("couldn't delete %s"),name);
	  _THROWX( buffer );
	}




  }

  void FSDirectory::renameFile(const char_t* from, const char_t* to){
	  LOCK_MUTEX(FSDIR_RENAME);
	char_t old[CL_MAX_DIR];
	priv_getFN(old, from);

	char_t nu[CL_MAX_DIR];
	priv_getFN(nu, to);

	/* This is not atomic.  If the program crashes between the call to
	delete() and the call to renameTo() then we're screwed, but I've
	been unable to figure out how else to do this... */

	if ( lucene::util::Misc::dir_Exists(nu) )
	  if( unlinkFile(nu) != 0 ){
		StringBuffer e(_T("couldn't delete "));
		e.append(to);
		//UNLOCK_MUTEX(FSDIR_RENAME);
		_THROWX( e.getBuffer() );
	  }
	if ( fileRename(old,nu) != 0 ){
		char_t buffer[200];
		stringPrintF(buffer, _T("couldn't rename %s to %s"), from, to );
		//UNLOCK_MUTEX(FSDIR_RENAME);
	  _THROWX( buffer );
	}
	UNLOCK_MUTEX(FSDIR_RENAME);
  }

  OutputStream& FSDirectory::createFile(const char_t* name) {
	char_t fl[CL_MAX_DIR];
	priv_getFN(fl, name);

	return *new FSOutputStream( fl );
  }

  LuceneLock* FSDirectory::makeLock(const char_t* name) {
	char_t dr[CL_MAX_DIR];
	priv_getFN(dr,name);
	return new FSLock( dr );
  }



  FSLock::FSLock ( const char_t* name ):
	fname( stringDuplicate(name) )
  {
  }
  FSLock::~FSLock(){
		  delete[] fname;
  }
  bool FSLock::obtain() {
	if ( lucene::util::Misc::dir_Exists(fname) )
	  return false;

		int_t r = openFile(fname,  O_RDWR | O_CREAT | O_RANDOM , _S_IREAD | _S_IWRITE); //must do this or file will be created Read only

	if ( r == -1 )
	  return false;
	else{
	  _close(r);
	  return true;
	}

  }
  void FSLock::release() {
	unlinkFile( fname );
  }
#endif // CLUCENE_LITE


}// namespace store
}// namespace lucene


