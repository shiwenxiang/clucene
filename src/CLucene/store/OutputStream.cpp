#include "CLucene/StdHeader.h"
#ifndef CLUCENE_LITE
#include "OutputStream.h"

#include "CLucene/util/Arrays.h"

using namespace std;
using namespace lucene::util;
namespace lucene{ namespace store{


  OutputStream::OutputStream()
  {
    buffer = new l_byte_t[ LUCENE_STREAM_BUFFER_SIZE ];
    bufferStart = 0;
    bufferPosition = 0;
  }

  OutputStream::~OutputStream(){
  }

  bool OutputStream::isClosed() const { /* DSR:PROPOSED */
    return buffer == NULL;
  }

  void OutputStream::close(){
    flush();
    //_DELETE( buffer );
    delete[] buffer;  // new[] (in constructor above) must be matched by
                                  // delete[] --RGR
    buffer = NULL; /* DSR:PROPOSED (in support of proposed method isClosed) */


    bufferStart = 0;
    bufferPosition = 0;
  }

  /** OutputStream-like methods @see java.io.InputStream */
  void OutputStream::writeByte(const l_byte_t b) {
    if (bufferPosition >= LUCENE_STREAM_BUFFER_SIZE)
      flush();
    buffer[bufferPosition++] = b;
  }

  void OutputStream::writeBytes(const l_byte_t* b, const int_t length) {
    if ( length < 0 )
      _THROWC( "IO Argument Error. Value must be a positive value.");
    for (int_t i = 0; i < length; i++)
      writeByte(b[i]);
  }

  void OutputStream::writeInt(const int_t i) {
    writeByte((l_byte_t)(i >> 24));
    writeByte((l_byte_t)(i >> 16));
    writeByte((l_byte_t)(i >>  8));
    writeByte((l_byte_t) i);
  }

  void OutputStream::writeVInt(const int_t vi) {
    //TODO: FIX THIS!!!!
    uint_t i = vi;
    while ((i & ~0x7F) != 0) {
      writeByte((l_byte_t)((i & 0x7f) | 0x80));
      i >>= 7; // TODO: check if this is correct (i >>>= 7;)
    }
    writeByte( (l_byte_t)i );
  }

  void OutputStream::writeLong(const long_t i) {
    writeInt((int_t) (i >> 32));
    writeInt((int_t) i);
  }

  void OutputStream::writeVLong(const long_t vi) {
    //TODO: FIX THIS!!!!
    ulong_t i = vi;
    while ((i & ~0x7F) != 0) {
      writeByte((l_byte_t)((i & 0x7f) | 0x80));
      i >>= 7; //TODO: check if this is correct (i >>>= 7;)
    }
    writeByte((l_byte_t)i);
  }

  void OutputStream::writeString(const char_t* s) {
    int_t length = 0;
    if ( s != NULL ) length = stringLength(s);
    writeVInt(length);
    writeChars(s, 0, length);
  }

  void OutputStream::writeChars(const char_t* s, const int_t start, const int_t length){
    if ( length < 0 || start < 0 )
      _THROWC( "IO Argument Error. Value must be a positive value.");

	#ifdef UTF8
    writeBytes((const l_byte_t*)s+start, length);
	#else
        const int_t end = start + length;
        for (int_t i = start; i < end; i++) {
          const int_t code = (int_t)s[i];
          if (code >= 0x01 && code <= 0x7F)
            writeByte((l_byte_t)code);
          else if (((code >= 0x80) && (code <= 0x7FF)) || code == 0) {
            writeByte((l_byte_t)(0xC0 | (code >> 6)));
            writeByte((l_byte_t)(0x80 | (code & 0x3F)));
          } else {
            writeByte((l_byte_t)(0xE0 | (((uint_t)code) >> 12))); //TODO: used to be - writeByte((l_byte_t)(0xE0 | (code >>> 12)));
            writeByte((l_byte_t)(0x80 | ((code >> 6) & 0x3F)));
            writeByte((l_byte_t)(0x80 | (code & 0x3F)));
          }
        }
    #endif
  }


  long_t OutputStream::getFilePointer() {
    return bufferStart + bufferPosition;
  }

  void OutputStream::seek(const long_t pos) {
    flush();
    bufferStart = pos;
  }

  void OutputStream::flush() {
    flushBuffer(buffer, bufferPosition);
    bufferStart += bufferPosition;
    bufferPosition = 0;
  }

}}
#endif
