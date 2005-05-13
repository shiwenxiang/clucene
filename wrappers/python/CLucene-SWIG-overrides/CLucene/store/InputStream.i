/* Reviewed 2004.08.20: necessary */

namespace lucene{ namespace store{

  class InputStream {
  public:
    InputStream();
    virtual ~InputStream();

    virtual InputStream& clone()=0;
    l_byte_t readByte();
    void readBytes(l_byte_t* b, const int_t offset, const int_t len);
    int_t readInt();
    int_t readVInt();
    long_t readLong();
    long_t readVLong();
    char_t* readString(const bool unique);
    char_t* readString();
    void readChars(char_t* buffer, const int_t start, const int_t len);
    virtual void close();
    long_t getFilePointer();
    void seek(const long_t pos);
    long_t Length();
  };

}}
