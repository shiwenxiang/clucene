#include "CLucene/StdHeader.h"
#include "Reader.h"

using namespace lucene::store;

namespace lucene{namespace util{

  FileReader::FileReader(const char_t* fname):
    stream( new FSInputStream(fname) )
  {
    cachepos = cache+FILEREADER_CACHE-1;
  }
  FileReader::~FileReader(){
    close();
  }
  void FileReader::close(){
    if (stream != NULL) {
      stream->close();
      _DELETE(stream);
    }
  }
  int_t FileReader::read(char_t* buf, const int_t start, const int_t length){
  long_t av = available();
  if ( av > 0 ){
    if ( av < length ){
    av--;

    //av is long_t, but both InputStream::readChars and this method's
    //return value require int_t.  However, we've already determined that
    //av is less than length (which is a const int_t, so av couldn't
    //possibly exceed int_t's capacity.
    const int_t avInt = static_cast<int_t>(av);

    stream->readChars(buf,start,avInt);
    return avInt;
    }else{
    stream->readChars(buf,start,length);
    return length;
    }
  }else
    return 0;
  }
  int_t FileReader::read(char_t* buf){
  return read(buf,0,stringLength(buf));
  }
  long_t FileReader::available(){
  return stream->Length() - stream->getFilePointer();
  }
  uchar_t FileReader::readChar(){
  #ifdef UTF8
  #if 0
  char_t buf[6];
  int_t len=0;
  uchar_t ret;
  long_t avail = available();
  avail = (avail>=6 ? 6 : avail);
  if ( read(buf,0,avail) == avail ) {
    ret = lc_utf8_get_char(buf, &len);
    if (len==-1) return 0;
    stream->seek(stream->getFilePointer()-avail+len);
    //printf("FileReader read %d\n", ret);fflush(NULL);
    return ret;
  }
  else
    return 0;
  #endif

  //if you are within 6 of the end of the cache reread cache and set pointer to start of cache
  if (cachepos - cache >= FILEREADER_CACHE - 6)
  {
    long_t avail = available();
    avail = (avail>=FILEREADER_CACHE ? FILEREADER_CACHE : avail);
    if ( read(cache,0,avail) != avail )
    {
    throw "warning : FileReader::readChar : Failed to read into cache.";
    return '\0';
    }
    cachepos = cache;
    stream->seek(stream->getFilePointer()-avail);
    //printf("FileReader reloaded cache %s\n", cache);fflush(NULL);
  }
  int len=0;
  //get the char at current cachepos and set stream
  uchar_t ret = lc_utf8_get_char(cachepos, &len);
  if (len==-1)
  {
    throw "warning : FileReader::readChar : Read bad UTF-8 char.";
    return '\0';
  }
  stream->seek(stream->getFilePointer()+len);
  cachepos += len;
  //printf("FileReader read %d\n", ret);fflush(NULL);
  return ret;

  #else
  char_t buf[2];
  if ( read(buf,0,1) == 1 )
    return buf[0];
  else
    return 0;
  #endif
  }
  uchar_t FileReader::peek(){
  if ( available() == 0 )
    return 0;
  #ifdef UTF8
  char_t buf[6];
  int_t len=0;
  uchar_t ret;
  long_t avail = (available()>=6 ? 6 : available());
  if ( read(buf,0,avail) == avail ) {
    ret = lc_utf8_get_char(buf, &len);
    if (len==-1) return 0;
    stream->seek(stream->getFilePointer()-avail);
    //printf("FileReader peeked %d\n", ret);fflush(NULL);
    return ret;
  }
  else
    return 0;
  #else
  uchar_t ret = readChar();
  stream->seek(stream->getFilePointer()-1);
  return ret;
  #endif
  }
  long_t FileReader::position(){
  return stream->getFilePointer();
  }
  void FileReader::seek(long_t position){
  stream->seek(position);
  cachepos = cache+FILEREADER_CACHE-1;
  }




  StringReader::StringReader ( const char_t* value ):
  data(value),
  delVal(false)
  {
  pt = 0;
  len = stringLength(value);

  }
  StringReader::StringReader ( const char_t* value, const int length, const bool deletevalue ):
  data(value),
  len(length),
  delVal(deletevalue)
  {
  pt = 0;
  }
  StringReader::~StringReader(){
    close();
  }

  long_t StringReader::available(){
  return len-pt;
  }
  int_t StringReader::read ( char_t* buf ){
  return read(buf,0,stringLength(buf));
  }
  int_t StringReader::read ( char_t* buf, const int_t start, const int_t length ){
  if ( pt >= len )
    return -1;
  int_t rd = 0;
  while ( pt < len && rd < length ){
    buf[start+rd] = data[pt];
    rd ++;
    pt ++;
  }
  return rd;
  }
  uchar_t StringReader::readChar(){
  if ( pt>=len )
  {
    if (pt==len)
    return 0;
    printf("StringReader throwing EOF %d/%d\n", pt, len);fflush(NULL);
    throw "String reader EOF";
  }
  #ifdef UTF8
  int length;
  //printf("StringReader reading from %d/%d)\n", pt, len);fflush(NULL);
  uchar_t ret = lc_utf8_get_char(data+pt, &length);
  //printf("StringReader read %d from %s(%d)\n", ret, data+pt, length);fflush(NULL);
  pt+=length;
  #else
  char_t ret = data[pt];
  pt++;
  #endif
  return ret;
  }

  uchar_t StringReader::peek(){
  if ( pt>=len )
  {
    if (pt==len)
    return 0;
    printf("StringReader throwing EOF %d/%d\n", pt, len);fflush(NULL);
    throw "String reader EOF";
  }
  #ifdef UTF8
  int length=0;
  uchar_t ret = lc_utf8_get_char(data+pt, &length);
  //printf("StringReader peeked %d\n", ret);fflush(NULL);
  return ret;
  #else
  return data[pt];
  #endif
  }
  void StringReader::close() {
    if (data != NULL && delVal) {
      _DELETE_ARRAY(data);
    }
  }
  long_t StringReader::position(){
  return pt;
  }
  void StringReader::seek(long_t position){
  if (position > INT_T_MAX) {
    _THROWC("position parameter to StringReader::seek exceeds theoretical"
      " capacity of StringReader's internal buffer."
    );
  }
  pt = static_cast<int_t>(position);
  }

}}

