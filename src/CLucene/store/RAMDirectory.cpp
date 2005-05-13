#include "CLucene/StdHeader.h"
#include "RAMDirectory.h"

#include "Lock.h"
#include "Directory.h"
#include "CLucene/util/VoidMap.h"
#include "CLucene/util/Arrays.h"
#include "CLucene/util/Misc.h"



namespace lucene{ namespace store{

  RAMFile::RAMFile()
  {
     length = 0;
     lastModified = lucene::util::Misc::currentTimeMillis();
     buffers.setDoDelete(DELETE_TYPE_DELETE_ARRAY);
  }
  RAMFile::~RAMFile(){
  }



#ifndef CLUCENE_LITE
  RAMLock::RAMLock(const char_t* name, RAMDirectory* dir):
    fname ( stringDuplicate(name) ),
    directory(dir)
  {
  }
  RAMLock::~RAMLock()
  {
    delete[] fname;
  }
  bool RAMLock::obtain(){
    LOCK_MUTEX(directory->files_mutex);
    if (!directory->fileExists(fname)) {
        OutputStream& tmp = directory->createFile(fname);
        tmp.close();
        delete &tmp;

        //UNLOCK_MUTEX(directory->files_mutex);
      return true;
    }
    UNLOCK_MUTEX(directory->files_mutex);
    return false;
  }

  void RAMLock::release(){
    directory->deleteFile(fname);
  }


  RAMOutputStream::~RAMOutputStream(){
  }
  RAMOutputStream::RAMOutputStream(RAMFile& f):file(f) {
    pointer = 0;
  }


  /** output methods: */
  void RAMOutputStream::flushBuffer(const l_byte_t* src, const int_t len) {
    uint_t bufferNumber = pointer/LUCENE_STREAM_BUFFER_SIZE;
    int_t bufferOffset = pointer%LUCENE_STREAM_BUFFER_SIZE;
    int_t bytesInBuffer = LUCENE_STREAM_BUFFER_SIZE - bufferOffset;
    int_t bytesToCopy = bytesInBuffer >= len ? len : bytesInBuffer;

    if (bufferNumber == file.buffers.size())
      file.buffers.push_back( new l_byte_t[LUCENE_STREAM_BUFFER_SIZE] );

    l_byte_t* buffer = file.buffers.at(bufferNumber);
    lucene::util::Arrays::arraycopy(src, 0, buffer, bufferOffset, bytesToCopy);

    if (bytesToCopy < len) {			  // not all in one buffer
      int_t srcOffset = bytesToCopy;
      bytesToCopy = len - bytesToCopy;		  // remaining bytes
      bufferNumber++;
      if (bufferNumber == file.buffers.size())
        file.buffers.push_back( new l_byte_t[LUCENE_STREAM_BUFFER_SIZE]);
      buffer = file.buffers.at(bufferNumber);
      lucene::util::Arrays::arraycopy(src, srcOffset, buffer, 0, bytesToCopy);
    }
    pointer += len;
    if (pointer > file.length)
      file.length = pointer;

    file.lastModified = lucene::util::Misc::currentTimeMillis();
  }

  void RAMOutputStream::close() {
    OutputStream::close();
  }

  /** Random-at methods */
  void RAMOutputStream::seek(const long_t pos){
    OutputStream::seek(pos);
    pointer = (int_t)pos;
  }
  long_t RAMOutputStream::Length() {
    return file.length;
  };
#endif // CLUCENE_LITE




  RAMInputStream::RAMInputStream(RAMFile& f):file(f) {
    pointer = 0;
    length = f.length;
  }
  RAMInputStream::RAMInputStream(RAMInputStream& clone):
    file(clone.file),
    InputStream(clone)
  {
    pointer = clone.pointer;
    length = clone.length;
  }
  RAMInputStream::~RAMInputStream(){
  }
  InputStream& RAMInputStream::clone()
  {
    RAMInputStream* ret = new RAMInputStream(*this);
    return *ret;
  }

  /** InputStream methods */
  /* DSR:CL_BUG:
  ** The CLucene 0.8.11 implementation of readInternal caused invalid memory
  ** ops if fewer than the requested number of bytes were available and the
  ** difference between requested and available happened to cross a buffer
  ** boundary.
  ** A bufferNumber would then be generated for a buffer that did not exist,
  ** so an invalid buffer pointer was fetched from beyond the end of the
  ** file.buffers vector, and an attempt was made to read bytes from the
  ** bad buffer pointer.  Often this resulted in reading uninitialized memory;
  ** it only segfaulted occasionally.
  ** I replaced the dysfunctional CLucene 0.8.11 impl with a fixed version. */
  void RAMInputStream::readInternal(l_byte_t* dest, int_t destOffset, const int_t len) {
    const long_t bytesAvailable = file.length - pointer;
    long_t remainder = len <= bytesAvailable ? len : bytesAvailable;
    int_t start = pointer;
    while (remainder != 0) {
      int_t bufferNumber = start / LUCENE_STREAM_BUFFER_SIZE;
      int_t bufferOffset = start % LUCENE_STREAM_BUFFER_SIZE;
      int_t bytesInBuffer = LUCENE_STREAM_BUFFER_SIZE - bufferOffset;
      /* The buffer's entire length (bufferLength) is defined by InputStream.h
      ** as int_t, so obviously the number of bytes in a given segment of the
      ** buffer won't exceed the the capacity of int_t.  Therefore, the
      ** long_t->int_t cast on the next line is safe. */
      int_t bytesToCopy = bytesInBuffer >= remainder ? static_cast<int_t>(remainder) : bytesInBuffer;
      l_byte_t* buffer = (l_byte_t*)file.buffers.at(bufferNumber);

      lucene::util::Arrays::arraycopy(buffer, bufferOffset, dest, destOffset, bytesToCopy);

      destOffset += bytesToCopy;
      start += bytesToCopy;
      remainder -= bytesToCopy;
      pointer += bytesToCopy;
    }
  }

  void RAMInputStream::close() {
    InputStream::close();
  }

  /** Random-at methods */
  void RAMInputStream::seekInternal(const long_t pos) {
    pointer = (int_t)pos;
  }






  /** Returns an array of strings, one for each file in the directory. */
  void RAMDirectory::list(char_t**& list, int_t& size) {
    size = 0;
    list = new char_t*[files.size()];

    map<const char_t*,RAMFile*,lucene::util::charCompare>::iterator itr = files.begin();
    while (itr != files.end()){
      list[size] = (char_t*)itr->first;
      itr++;
      size++;
    }
  }

  RAMDirectory::RAMDirectory(){
    files.setDoDelete(true,DELETE_TYPE_DELETE_ARRAY);
    files.setDoDelete(false,DELETE_TYPE_DELETE);
  }
  RAMDirectory::~RAMDirectory(){
  }

  /** Returns true iff the named file exists in this directory. */
  bool RAMDirectory::fileExists(const char_t* name) const {
    return files.exists(name);
  }

  /** Returns the time the named file was last modified. */
  long_t RAMDirectory::fileModified(const char_t* name) const {
    return files.get(name)->lastModified;
  }

  /** Returns the length in bytes of a file in the directory. */
  long_t RAMDirectory::fileLength(const char_t* name) const{
    return files.get(name)->length;
  }


  /** Returns a stream reading an existing file. */
  InputStream& RAMDirectory::openFile(const char_t* name) {
    RAMFile* file = files.get(name);
    if (file == NULL) {
      _THROWC("[RAMDirectory::openFile] The requested file does not exist.");
    }
    return *new RAMInputStream( *file );
  }

  void RAMDirectory::close(){
    files.clear();
  }

#ifndef CLUCENE_LITE

  /** Removes an existing file in the directory. */
  void RAMDirectory::deleteFile(const char_t* name, const bool throwError) {
    files.remove(name);
  }

  /** Removes an existing file in the directory. */
  void RAMDirectory::renameFile(const char_t* from, const char_t* to) {
    RAMFile* file = files.get(from);
    //files.dv = false; //temporary disable delete value
    files.remove(from,false,true);
    //files.dv = true;

    /* DSR:CL_BUG_LEAK:
    ** If a file named $to already existed, its old value was leaked.
    ** My inclination would be to prevent this implicit deletion with an
    ** exception, but it happens routinely in CLucene's internals (e.g., during
    ** IndexWriter.addIndexes with the file named 'segments'). */
    if (files.exists(to)) {
      files.remove(to);
    }
    files.put(stringDuplicate(to), file);
  }

  /* Creates a new, empty file in the directory with the given name.
  ** Returns a stream writing this file. */
  OutputStream& RAMDirectory::createFile(const char_t* name) {
    /* Check the $files VoidMap to see if there was a previous file named
    ** $name.  If so, delete the old RAMFile object, but reuse the existing
    ** char_t buffer ($n) that holds the filename.  If not, duplicate the
    ** supplied filename buffer ($name) and pass ownership of that memory ($n)
    ** to $files. */
    char_t* n = const_cast<char_t*>( files.getKey(name) );
    if (n != NULL) {
      delete files.get(name);
    } else {
      n = stringDuplicate(name);
    }

    RAMFile& file = *new RAMFile();
    #ifdef _DEBUG
      file.filename = n;
    #endif
    files.put(n, &file);

    OutputStream* ret = new RAMOutputStream(file);
    return *ret;
  }

  /** Construct a {@link Lock}.
  * @param name the name of the lock file
  */
  LuceneLock* RAMDirectory::makeLock(const char_t* name) {
    return new RAMLock(name,this);
  }
#endif //CLUCENE_LITE

}}
