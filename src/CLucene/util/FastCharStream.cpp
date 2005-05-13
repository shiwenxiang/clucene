#include "CLucene/StdHeader.h"
#include "FastCharStream.h"

#include "CLucene/util/Arrays.h"
#include "CLucene/util/Reader.h"

namespace lucene{ namespace util {

  //Initializes a new instance of the FastCharStream class LUCENE_EXPORT.
  FastCharStream::FastCharStream(Reader& reader):
	input(reader),
	col(1),
	line(1)
  {
  }

  //Returns the next char_t from the stream.
  uchar_t FastCharStream::GetNext()	{
	/* WDG: UTF8 Reader.readChar will throw if it tries to read past the end so
		  no need to do it twice */
  #ifndef UTF8
	if (Eos())
	{
	  /* DSR:CL_BUG: Calling QueryParser::Parse("\\", ..., ...) triggered this
	  ** case, which didn't use a throw type that the QueryParser could handle,
	  ** resulting in "abnormal program termination"/"abort".  I switched from
	  **   throw "...";
	  ** to
	  **   _THROWC("...");  */
	  _THROWC("warning : FileReader.GetNext : Read char_t over EOS.");
	  /* DSR: The next statement is unreachable, isn't it? */
	  return '\0';
	}
  #endif
	uchar_t ch = input.readChar();
	//Though Reader::position returns long_t, we assume that no single line
	//will have more columns than int_t can represent.
	col = static_cast<int_t>(input.position()) + 1;

	if(ch == '\n')
	{
	  line++;
	  col = 1;
	}

	return ch;
  }

  void FastCharStream::UnGet(){
	if ( input.position() == 0 )
	  _THROWC("error : FileReader.UnGet : ungetted first char_t");

	input.seek(input.position()-1);
  }

  //Returns the current top char_t from the input stream without removing it.
  uchar_t FastCharStream::Peek(){
	try{
	  return input.peek();
	}catch(...){}
	return 0;
  }


  //Returns <b>True</b> if the end of stream was reached.
  bool FastCharStream::Eos()	{
	return input.available()==0;
  }

  // Gets the current column.
  int_t FastCharStream::Column() {
	return col;
  }

  // Gets the current line.
  int_t FastCharStream::Line() {
	return line;
  }
}}
