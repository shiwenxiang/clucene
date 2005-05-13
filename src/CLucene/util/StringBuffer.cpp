#include <assert.h>
#include "CLucene/StdHeader.h"
#include "StringBuffer.h"

namespace lucene{ namespace util {

  StringBuffer::StringBuffer(){
  //Func - Constructor. Allocates a buffer with the default length.
  //Pre  - true
  //Post - buffer of length bufferLength has been allocated
 
      //Initialize 
      bufferLength = LUCENE_DEFAULT_TOKEN_BUFFER_SIZE;
	  len          = 0;
	  //Allocate a buffer of length bufferLength
      buffer       = new char_t[bufferLength];
  }

  StringBuffer::StringBuffer(const int_t initSize){
  //Func - Constructor. Allocates a buffer of length initSize + 1
  //Pre  - initSize > 0
  //Post - A buffer has been allocated of length initSize + 1

	  //Initialize the bufferLength to initSize + 1 The +1 is for the terminator '\0'
      bufferLength = initSize + 1;
      len = 0;
      //Allocate a buffer of length bufferLength
      buffer = new char_t[bufferLength];
  }

  StringBuffer::StringBuffer(const char_t* value){
  //Func - Constructor. 
  //       Creates an instance of Stringbuffer containing a copy of the string value
  //Pre  - value != NULL
  //Post - An instance of StringBuffer has been created containing the copy of the string value
  
      //Initialize the length of the string to be stored in buffer
	  len = (int_t) stringLength(value);

	  //Calculate the space occupied in buffer by a copy of value
      const int_t occupiedLength = len + 1;
      
	  // Minimum allocated buffer length is LUCENE_DEFAULT_TOKEN_BUFFER_SIZE.
      bufferLength = (occupiedLength >= LUCENE_DEFAULT_TOKEN_BUFFER_SIZE 
	 	? occupiedLength : LUCENE_DEFAULT_TOKEN_BUFFER_SIZE);

	  //Allocate a buffer of length bufferLength
      buffer = new char_t[bufferLength];
      //Copy the string value into buffer
      stringNCopy(buffer, value, occupiedLength);
	  //Assert that the buffer has been terminated at the end of the string
      assert (buffer[len] == '\0');
  }

  StringBuffer::~StringBuffer() {
  // Func - Destructor
  // Pre  - true
  // Post - Instanc has been destroyed
	  
	  if(buffer){
		  delete[] buffer;
	      }
  }
  void StringBuffer::clear(){
  //Func - Clears the Stringbuffer and resets it to it default empty state
  //Pre  - true
  //Post - pre(buffer) has been destroyed and a new one has been allocated

      //Destroy the current buffer if present
	  if(buffer){
	      delete[] buffer;
		  }
	  
	  //Initialize 
      len = 0;
      bufferLength = LUCENE_DEFAULT_TOKEN_BUFFER_SIZE;
      //Allocate a buffer of length bufferLength
      buffer = new char_t[bufferLength];
  }

  void StringBuffer::append(const uchar_t character) {
  //Func - Appends a single character 
  //Pre  - true
  //Post - The character has been appended to the string in the buffer

      #ifdef UTF8
	  //Check if the current buffer length is sufficient to have the string value appended
	  if (len + 7 > bufferLength){
		   //Have the size of the current string buffer increased because it is too small	
          growBuffer(len + 7);
	      }
       
	  //Append the character to the string in the buffer and calculate the space that character will occupy
      int buflen = lc_unichar_to_utf8(character, buffer + len);
	  //Increase len by the occupied space in
      len += buflen;
      #else
	  //Check if the current buffer length is sufficient to have the string value appended
      if (len + 1 > bufferLength){
		   //Have the size of the current string buffer increased because it is too small	
          growBuffer(len + 1);
	      }
	  //Put character at position len which is the end of the string in the buffer
	  //Note that this action might overwrite the terminator of the string '\0', which
	  //is kind of tricky
      buffer[len] = character;
	  //Increase the len by to represent the correct length of the string in the buffer
      len++;
      #endif
  }

  void StringBuffer::append(const char_t* value) {
  //Func - Appends a copy of the string value
  //Pre  - value != NULL
  //Post - value has been copied and appended to the string in buffer
  
      append(value, stringLength(value));
  }
  void StringBuffer::append(const char_t* value, const int_t appendedLength) {
  //Func - Appends a copy of the string value
  //Pre  - value != NULL
  //       appendedLength contains the length of the string value which is to be appended
  //Post - value has been copied and appended to the string in buffer 
  
      //Check if the current buffer length is sufficient to have the string value appended
      if (len + appendedLength + 1 > bufferLength){
          //Have the size of the current string buffer increased because it is too small	
          growBuffer(len + appendedLength + 1);
          }

      //Copy the string value into the buffer at postion len
      stringNCopy(buffer + len, value, appendedLength);
    
      //Add the length of the copied string to len to reflect the new length of the string in
      //the buffer (Note: len is not the bufferlength!)
      len += appendedLength;
  }

  void StringBuffer::append(const int_t value) {
  //Func - Appends an integer (after conversion to a character string)
  //Pre  - true 
  //Post - The converted integer value has been appended to the string in buffer
  
      //instantiate a buffer of 30 charactes for the conversion of the integer
      char_t buf[30];
      //Convert the integer value to a string buf using the radix 10 (duh)
      integerToString(value, buf, 10);
	  //Have the converted integer now stored in buf appended to the string in buffer
      append(buf);
  }

  void StringBuffer::append(const double value, const int_t digits){
  //Func - Appends a double (after conversion to a character string)
  //Pre  - digits > 0. Indicates the minimum number of characters printed
  //Post - The converted double value has been appended to the string in buffer

    //instantiate a buffer of 30 charactes for the conversion of the integer
    char_t buf[30];
	//Convert the double value to a string buf. 
	stringPrintF(buf, _T("%.*g"), digits, value);
    //Have the converted double now stored in buf appended to the string in stringbuffer
    append(buf);
  }

#ifdef _UNICODE
  void StringBuffer::append(const char character) {
  //Func - Provides an interface for appending literal characters for instance '&'
  //       when compiled with _UNICODE
  //Pre  - character contains a character to be appended to the string in the buffer
  //Post - character has been appended to the string in buffer

      append((const uchar_t) character);
  }
#endif


  void StringBuffer::prepend(const char_t* value){
  //Func - Puts a copy of the string value infront of the current string in the StringBuffer
  //Pre  - value != NULL
  //Post - The string in pre(buffer) has been shifted n positions where n equals the length of value.
  //       The string value was then copied to the beginning of stringbuffer

      prepend(value, stringLength(value));
  }

  void StringBuffer::prepend(const char_t* value, const int_t prependedLength) {
  //Func - Puts a copy of the string value in front of the string in the StringBuffer
  //Pre  - value != NULL
  //       prependedLength contains the length of the string value which is to be prepended
  //Post - A copy of the string value is has been in front of the string in buffer
   
	  //Check if the current buffer length is sufficient to have the string value prepended
	  if (prependedLength + len + 1 > bufferLength){
		  //Have the size of the current string buffer increased because it is too small	
		  //Because prependedLength is passed as the second argument to growBuffer,
          //growBuffer will have left the first prependedLength characters empty
          //when it recopied buffer during reallocation.  
          growBuffer(prependedLength + len + 1, prependedLength);
	      }

      //Copy the string value into the buffer at postion 0
      stringNCopy(buffer, value, prependedLength);
      //Add the length of the copied string to len to reflect the new length of the string in
      //the buffer (Note: len is not the bufferlength!)
      len += prependedLength;
  }

  int_t StringBuffer::length() {
  //Func - Returns the length of the string in the StringBuffer
  //Pre  - true
  //Post - The length len of the string in the buffer has been returned

      return len;
  }
  char_t* StringBuffer::ToString() {
  //Func - Returns a copy of the current string in the StringBuffer sized equal to the length of the string 
  //       in the StringBuffer.
  //Pre  - true
  //Post - The copied string has been returned

	  //Instantiate a buffer equal to the length len + 1
      char_t* ret = new char_t[len + 1];
	  if (ret){
		  //Copy the string in buffer
	      stringNCopy(ret, buffer, len);
		  //terminate the string
          ret[len] = '\0';
	      }
      //return the the copy  
      return ret;
  }
  char_t* StringBuffer::getBuffer() {
  //Func - '\0' terminates the buffer and returns its pointer
  //Pre  - true
  //Post - buffer has been '\0' terminated and returned
    
      // Check if the current buffer is '\0' terminated
	  if (len == bufferLength){
          //Make space for terminator, if necessary.
          growBuffer(len + 1);
	      }
      //'\0' buffer so it can be returned properly
      buffer[len] = '\0';

     return buffer;
  }
  void StringBuffer::growBuffer(const int_t minLength) {
  //Func - Has the buffer grown to a minimum length of minLength or bigger
  //Pre  - minLength >= len + 1
  //Post - The buffer has been grown to a minimum length of minLength or bigger

      growBuffer(minLength, 0);
  }
  void StringBuffer::growBuffer(const int_t minLength, const int_t skippingNInitialChars) {
  //Func - Has the buffer grown to a minimum length of minLength or bigger and shifts the
  //       current string in buffer by skippingNInitialChars forward
  //Pre  - After growth, must have at least enough room for contents + terminator so
  //       minLength >= skippingNInitialChars + len + 1
  //       skippingNInitialChars >= 0
  //Post - The buffer has been grown to a minimum length of minLength or bigger and
  //       if skippingNInitialChars > 0, the contents of the buffer has beeen shifted
  //       forward by skippingNInitialChars positions as the buffer is reallocated,
  //       leaving the first skippingNInitialChars uninitialized (presumably to be
  //       filled immediately thereafter by the caller).

    assert (skippingNInitialChars >= 0);
    assert (minLength >= skippingNInitialChars + len + 1);

    //More aggressive growth strategy to offset smaller default buffer size:
    bufferLength *= 2;
	//Check that bufferLength is bigger than minLength
	if (bufferLength < minLength){
	    //Have bufferLength become minLength because it still was too small
        bufferLength = minLength;
	    }

	//Allocate a new buffer of length bufferLength
    char_t* tmp = new char_t[bufferLength];
    //The old buffer might not have been null-terminated, so we stringNCopy
    //only len bytes, not len+1 bytes (the latter might read one char off the
    //end of the old buffer), then apply the terminator to the new buffer.
    stringNCopy(tmp + skippingNInitialChars, buffer, len);
    tmp[skippingNInitialChars + len] = '\0';
	
	//destroy the old buffer
	if (buffer){
		delete[] buffer;
	    }
	//Assign the new buffer tmp to buffer
    buffer = tmp;
  }

}}
