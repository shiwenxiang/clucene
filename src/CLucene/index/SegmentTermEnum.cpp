#include "CLucene/StdHeader.h"
#include "SegmentTermEnum.h"

#include "Terms.h"
#include "FieldInfos.h"
#include "Term.h"
#include "TermInfo.h"
#include "CLucene/util/Arrays.h"

namespace lucene{ namespace index {

	SegmentTermEnum::SegmentTermEnum(InputStream& i, FieldInfos& fis, const bool isi):
		fieldInfos(fis),
		input(i){
	//Func - Constructor
	//Pre  - i holds a reference to an instance of InputStream
	//       fis holds a reference to an instance of FieldInfos
	//       isi
	//Post - An instance of SegmentTermEnum has been created

		position     = -1;
		//Instantiate a Term with empty field, empty text and which is interned (see term.h what interned means)
		term         = new Term( _T(""),_T("") ,true);
		isIndex      = isi;
		termInfo     = new TermInfo();
		indexPointer = 0;
		buffer       = NULL;
		bufferLength = 0;
		prev         = NULL;

		//Set isClone to false as the instance is not clone of another instance
		isClone      = false;


		size = input.readInt();
	}

	SegmentTermEnum::SegmentTermEnum(SegmentTermEnum& clone):
		fieldInfos(clone.fieldInfos),
		input(clone.input.clone()){
	//Func - Constructor
	//       The instance is created by cloning all properties of clone
	//Pre  - clone holds a valid reference to SegmentTermEnum
	//Post - An instance of SegmentTermEnum with the same properties as clone

		//Copy the postion from the clone
		position     = clone.position;

		term         = clone.term==NULL?NULL:clone.term->pointer();
		isIndex      = clone.isIndex;
		termInfo     = new TermInfo(*clone.termInfo);
		indexPointer = clone.indexPointer;
		buffer       = new char_t[clone.bufferLength];
		bufferLength = clone.bufferLength;
		prev         = clone.prev==NULL?NULL:clone.prev->pointer();
		size         = clone.size;
		//Set isClone to true as this instance is a clone of another instance
		isClone      = true;

		//Copy the contents of buffer of clone to the buffer of this instance
		lucene::util::Arrays::arraycopy(clone.buffer,0,buffer,0,bufferLength);
	}

	SegmentTermEnum::~SegmentTermEnum(){
	//Func - Destructor
	//Pre  - true
	//Post - The instance has been destroyed. If this instance was a clone
	//       then the inputstream is closed and deleted too.

		//Finalize prev
		prev->finalize();
		//Finalize term
		term->finalize();
		//Delete the buffer if necessary
		_DELETE_ARRAY(buffer);
		//Delete termInfo if necessary
		_DELETE(termInfo);

		//Check if this instance is a clone
		if ( isClone ){
			//Close the inputstream
			input.close();
			//delete the inputstream
			delete &input;
			}
	}

	bool SegmentTermEnum::next(){
	//Func - Moves the current of the set to the next in the set
	//Pre  - true
	//Post - If the end has been reached NULL is returned otherwise the term has
	//       become the next Term in the enumeration

		//Increase position by and and check if the end has been reached
		if (position++ >= size-1) {
			//delete term
			term->finalize();
			//term does not point to any term any more
			term = NULL;
			return false;
		}

		//delete the previous enumerated term
		prev->finalize();
		//prev becomes the current enumerated term
		prev = term;
		//term becomes the next term read from inputStream input
		term = readTerm();

		//Read docFreq, the number of documents which contain the term.
		termInfo->docFreq = input.readVInt();
		//Read freqPointer, a pointer into the TermFreqs file (.frq)
		termInfo->freqPointer += input.readVLong();
		//Read proxPointer, a pointer into the TermPosition file (.prx).
		termInfo->proxPointer += input.readVLong();

		//Check if the enumeration is an index
		if (isIndex)
			//read index pointer
			indexPointer += input.readVLong();

		return true;
	}

	Term* SegmentTermEnum::getTerm(const bool pointer) {
	//Func - Returns the current term.
	//Pre  - pointer is true or false and indicates if the reference counter
	//       of term must be increased or not
	//       next() must have been called once!
	//Post - pointer = true -> term has been returned with an increased reference counter
	//       pointer = false -> term has been returned

		if ( pointer && term!=NULL ){
			return term->pointer();
			}
		else{
			return term;
			}
	}

	void SegmentTermEnum::scanTo(const Term *Scratch){
	//Func - Scan for Term without allocating new Terms
	//Pre  - term != NULL
	//Post - The iterator term has been moved to the position where Term is expected to be
	//       in the enumeration

		while ( Scratch->compareTo(*term) > 0 && next()) {}
	}

	void SegmentTermEnum::close() {
	//Func - Closes the enumeration to further activity, freeing resources.
	//Pre  - true
	//Post - The inputStream input has been closed

		input.close();
	}

	int_t SegmentTermEnum::DocFreq() const {
	//Func - Returns the document frequency of the current term in the set
	//Pre  - termInfo != NULL
	//       next() must have been called once
	//Post  - The document frequency of the current enumerated term has been returned

		return termInfo->docFreq;
	}

	void SegmentTermEnum::seek(const long_t pointer, const int_t p, Term& t, TermInfo& ti) {
	//Func - Repositions term and termInfo within the enumeration
	//Pre  - pointer >= 0
	//       p >= 0 and contains the new position within the enumeration
	//       t is a valid reference to a Term and is the new current term in the enumeration
	//       ti is a valid reference to a TermInfo and is corresponding TermInfo form the new
	//       current Term
	//Post - term and terminfo have been repositioned within the enumeration

		//Reset the InputStream input to pointer
		input.seek(pointer);
		//Assign the new position
		position = p;

		//finalize the current term
		term->finalize();
		//Get a pointer from t and increase the reference counter of t
		term = t.pointer();

		//finalize prev
		prev->finalize();
		//prev is unknown so set it to NULL
		prev = NULL;

		//Change the current termInfo so it matches the new current term
		termInfo->set(ti);

		//Have the buffer grown if needed
		growBuffer( stringLength(term->Text()) );		  // copy term text into buffer
	}

	TermInfo* SegmentTermEnum::getTermInfo()const {
	//Func - Returns a clone of the current termInfo
	//Pre  - termInfo != NULL
	//       next() must have been called once
	//Post - A clone of the current termInfo has been returned

		return new TermInfo(*termInfo); //clone
	}

	void SegmentTermEnum::getTermInfo(TermInfo& ti)const {
	//Func - Retrieves a clone of termInfo through the reference ti
	//Pre  - ti contains a valid reference to TermInfo
	//       termInfo != NULL
	//       next() must have been called once
	//Post - ti contains a clone of termInfo

		ti.set(*termInfo);
	}

	long_t SegmentTermEnum::freqPointer()const {
	//Func - Returns the freqpointer of the current termInfo
	//Pre  - termInfo != NULL
	//       next() must have been called once
	//Post - The freqpointer of the current termInfo has been returned

		return termInfo->freqPointer;
	}

	long_t SegmentTermEnum::proxPointer()const {
	//Func - Returns the proxPointer of the current termInfo
	//Pre  - termInfo != NULL
	//       next() must have been called once
	//Post - the proxPointer of the current termInfo has been returned

		return termInfo->proxPointer;
	}

	SegmentTermEnum* SegmentTermEnum::clone() {
	//Func - Returns a clone of this instance
	//Pre  - true
	//Post - An clone of this instance has been returned

		//TODO: check this
		return new SegmentTermEnum(*this);
	}

	Term* SegmentTermEnum::readTerm() {
	//Func - Reads the next term in the enumeration
	//Pre  - true
	//Post - The next Term in the enumeration has been read and returned

		//Read the start position from the inputStream input
		int_t start = input.readVInt();
		//Read the length of term in the inputStream input
		int_t length = input.readVInt();

		//Calculated the total lenght of bytes that buffer must be to contain the current
		//chars in buffer and the new ones yet to be read
		uint_t totalLength = start + length;

		//TODO: check this, not copying buffer every time.
		if (static_cast<uint_t>(bufferLength) < totalLength+1)
			growBuffer(totalLength);

		//Read a length number of characters into the buffer from position start in the inputStream input
		input.readChars(buffer, start, length);
		//Null terminate the string
		buffer[totalLength] = 0;
		//Return a new Term
		return new Term( fieldInfos.fieldName(input.readVInt()), buffer );
	}

	void SegmentTermEnum::growBuffer(const int_t length) {
	//Func - Instantiate a buffer of length length+1
	//Pre  - length > 0
	//Post - pre(buffer) has been deleted with its contents. A new buffer
	//       has been allocated of length length+1 and the text of term has been copied
	//       to buffer

		//Delete the current buffer with its contents if necessary
		if (buffer){
			delete[] buffer;
			}

		//Instantiate the new buffer + 1 is needed for terminator '\0'
		buffer = new char_t[length+1];
		//Store the new bufferLength
		bufferLength = length+1;
		//Copy the text of term into buffer
		stringCopy(buffer,term->Text());
	}

}}
