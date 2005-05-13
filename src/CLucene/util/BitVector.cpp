#include "CLucene/StdHeader.h"
#include "BitVector.h"

#include "CLucene/store/Directory.h"

using namespace std;
namespace lucene{ namespace util{

  BitVector::BitVector(int_t n)
  {
    //BYTE_COUNTS =
    count=-1;
    size = n;
    int len = (size >> 3) + 1;
    bits = new l_byte_t[len];
    for ( int i=0;i<len;i++ )
      bits[i] = 0;
  }

  BitVector::~BitVector(){
    delete[] bits; /* DSR:CL_BUG: Needed to use delete[] operator instead of delete. */
  }

  BitVector::BitVector(lucene::store::Directory& d, const char_t* name) {
    count=-1;
    InputStream& input = d.openFile( name );
    _TRY {
      size = input.readInt();			  // read size
      count = input.readInt();			  // read count
      bits = new l_byte_t[(size >> 3) + 1];		  // allocate bits
      input.readBytes(bits, 0, (size >> 3) + 1);	  // read bits
    } _FINALLY (
        input.close();
        delete ( &input );
    );
  }


  void BitVector::set(const int_t bit) {
    bits[bit >> 3] |= 1 << (bit & 7);
    count = -1;
  }

  void BitVector::clear(const int_t bit) {
    bits[bit >> 3] &= ~(1 << (bit & 7));
    count = -1;
  }

  bool BitVector::get(const int_t bit) {
    return (bits[bit >> 3] & (1 << (bit & 7))) != 0;
  }

  int_t BitVector::Size() {
    return size;
  }

  int_t BitVector::Count() {
    if (count == -1) {
      l_byte_t BYTE_COUNTS[] = {  0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
                    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
                    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
                    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
                    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
                    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
                    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
                    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
                    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
                    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
                    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
                    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
                    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
                    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
                    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
                    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8};

      int_t c = 0;
      int_t end = (size >> 3) + 1;
      for (int_t i = 0; i < end; i++)
        c += BYTE_COUNTS[bits[i] & 0xFF];	  // sum bits per l_byte_t
      count = c;
    }
    return count;
  }

#ifndef CLUCENE_LITE
  void BitVector::write(lucene::store::Directory& d, const char_t* name) {
    OutputStream& output = d.createFile(name);
    _TRY {
      output.writeInt(Size());			  // write size
      output.writeInt(Count());			  // write count
      output.writeBytes(bits, (size >> 3) + 1);	  // write bits TODO: CHECK THIS, was bit.length NOT (size >> 3) + 1
    } _FINALLY (
        output.close();
        delete (&output);
      );
  }
#endif

}}
