#include "CLucene/StdHeader.h"

#include "InputStream.h"
#include "CLucene/util/Arrays.h"

using namespace std;
using namespace lucene::util;
namespace lucene{ namespace store{

  InputStream::InputStream():
    length(0),
    chars_length(0),
    bufferStart(0),
    bufferLength(0),
    bufferPosition(0),
    chars(NULL),
    buffer(NULL)
  {
  }

  InputStream::InputStream(InputStream& clone):
    length(clone.length),
    bufferStart(clone.bufferStart),
    bufferLength(clone.bufferLength),
    bufferPosition(clone.bufferPosition),
    buffer(NULL),
    chars(NULL),
    chars_length(0)
  {
    /* DSR: Does the fact that sometime clone.buffer is not NULL even when
    ** clone.bufferLength is zero indicate memory corruption/leakage?
    **   if ( clone.buffer != NULL) { */
    if (clone.bufferLength != 0 && clone.buffer != NULL) {
      buffer = new l_byte_t[bufferLength];
      lucene::util::Arrays::arraycopy( clone.buffer, 0, buffer, 0, bufferLength);
    }
  }


  l_byte_t InputStream::readByte() {
    if (bufferPosition >= bufferLength)
      refill();

    return buffer[bufferPosition++];
  }

  void InputStream::readBytes(l_byte_t* b, const int_t offset, const int_t len){
    if (len < LUCENE_STREAM_BUFFER_SIZE) {
      for (int_t i = 0; i < len; i++)		  // read byte-by-byte
        b[i + offset] = (l_byte_t)readByte();
    } else {					  // read all-at-once
      long_t start = getFilePointer();
      seekInternal(start);
      readInternal(b, offset, len);

      bufferStart = start + len;		  // adjust stream variables
      bufferPosition = 0;
      bufferLength = 0;				  // trigger refill() on read
    }
  }

  int_t InputStream::readInt() {
    l_byte_t b1 = readByte();
    l_byte_t b2 = readByte();
    l_byte_t b3 = readByte();
    l_byte_t b4 = readByte();
    return ((b1 & 0xFF) << 24) | ((b2 & 0xFF) << 16) | ((b3 & 0xFF) <<  8)
         | (b4 & 0xFF);
  }

  int_t InputStream::readVInt() {
    l_byte_t b = readByte();
    int_t i = b & 0x7F;
    for (int_t shift = 7; (b & 0x80) != 0; shift += 7) {
      b = readByte();
      i |= (b & 0x7F) << shift;
    }
    return i;
  }

  long_t InputStream::readLong() {
    int_t i1 = readInt();
    int_t i2 = readInt();
    return (((long_t)i1) << 32) | (i2 & 0xFFFFFFFFL);
  }

  long_t InputStream::readVLong() {
    l_byte_t b = readByte();
    long_t i = b & 0x7F;
    for (int_t shift = 7; (b & 0x80) != 0; shift += 7) {
      b = readByte();
      i |= (b & 0x7FL) << shift;
    }
    return i;
  }

  char_t* InputStream::readString(const bool unique){

    int_t len = readVInt();
    if (chars == NULL || len > chars_length){
      if ( len == 0 ){
        if ( unique )
          return stringDuplicate(_T(""));
        else
          return _T("");
      }

      if ( chars != NULL )
        delete[] chars;
      chars = new char_t[len+1];
      chars_length = len;
    }
    readChars(chars, 0, len);
    chars[len] = 0;

    if ( unique )
      return stringDuplicate(chars);
    else
      return chars;
  }
  char_t* InputStream::readString() {
    return readString(true);
  }

  void InputStream::readChars(char_t* buffer, const int_t start, const int_t len) {
    const int_t end = start + len;
    for (int_t i = start; i < end; i++) {
#ifdef UTF8
      buffer[i] = readByte();
#else
      l_byte_t b = readByte();
      if ((b & 0x80) == 0) {
        buffer[i] = (char_t)(b & 0x7F);
      } else if ((b & 0xE0) != 0xE0) {
        buffer[i] = (char_t)(((b & 0x1F) << 6)
          | (readByte() & 0x3F));
      } else {
          l_byte_t b2 = readByte();
          l_byte_t b3 = readByte();
          buffer[i] = (char_t)(
                ((b & 0x0F) << 12)
              | ((b2 & 0x3F) << 6)
              | (b3 & 0x3F)
            );
      }
#endif
    }
  }


  long_t InputStream::getFilePointer() {
    return bufferStart + bufferPosition;
  }

  void InputStream::seek(const long_t pos) {
    if ( pos < 0 )
      _THROWC( "IO Argument Error. Value must be a positive value.");
    if (pos >= bufferStart && pos < (bufferStart + bufferLength))
      bufferPosition = (int_t)(pos - bufferStart);  // seek within buffer
    else {
      bufferStart = pos;
      bufferPosition = 0;
      bufferLength = 0;				  // trigger refill() on read()
      seekInternal(pos);
    }
  }
  long_t InputStream::Length() {
    return length;
  }

  void InputStream::close(){
          if( chars != NULL ) {
            delete[] chars ; chars = NULL;
          }
          if ( buffer != NULL ) {
            delete[] buffer ; buffer = NULL;
          }

    bufferLength = 0;
    bufferPosition = 0;
    bufferStart = 0;
  }


  InputStream::~InputStream(){
    close();
  }

  void InputStream::refill() {
    long_t start = bufferStart + bufferPosition;
    long_t end = start + LUCENE_STREAM_BUFFER_SIZE;
    if (end > length)				  // don't read past EOF
      end = length;
    bufferLength = (int_t)(end - start);
    if (bufferLength == 0)
      _THROWC( "InputStream read past EOF");

    if (buffer == NULL){
      buffer = new l_byte_t[LUCENE_STREAM_BUFFER_SIZE];		  // allocate buffer lazily
      bufferLength = LUCENE_STREAM_BUFFER_SIZE;
    }
    readInternal(buffer, 0, bufferLength);


    bufferStart = start;
    bufferPosition = 0;
  }

}}

