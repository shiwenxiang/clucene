#include "CLucene/StdHeader.h"
#ifndef CLUCENE_LITE
#include "TermInfosWriter.h"

#include "CLucene/store/Directory.h"
#include "FieldInfos.h"
#include "Term.h"
#include "TermInfo.h"

using namespace lucene::util;
namespace lucene{ namespace index {

    TermInfosWriter::TermInfosWriter(Directory& directory, const char_t* segment, FieldInfos& fis):
        fieldInfos(fis){
    //Func - Constructor
	//Pre  - directory contains a valid reference to a Directory
    //       segment != NULL
	//       fis contains a valid reference to a reference FieldInfos
	//Post - The instance has been created

        CND_PRECONDITION(segment != NULL,"segment is NULL")
		//Initialize instance
        _TermInfosWriter(directory,segment,false);

		other = new TermInfosWriter(directory, segment,fieldInfos, true);

		CND_CONDITION(other != NULL,"other is NULL")

		other->other = this;
	}

    TermInfosWriter::TermInfosWriter(Directory& directory, const char_t* segment, FieldInfos& fis, bool isIndex):
	    fieldInfos(fis){
    //Func - Constructor
	//Pre  - directory contains a valid reference to a Directory
    //       segment != NULL
	//       fis contains a valid reference to a reference FieldInfos
	//       isIndex is true or false
	//Post - The instance has been created

        CND_PRECONDITION(segment != NULL,"segment is NULL")
        _TermInfosWriter(directory,segment,isIndex);
    }

    void TermInfosWriter::_TermInfosWriter(Directory& directory, const char_t* segment, bool IsIndex){
    //Func - Helps constructors to initialize Instance
	//Pre  - directory contains a valid reference to a Directory
    //       segment != NULL
	//       fis contains a valid reference to a reference FieldInfos
	//Post - The instance has been initialized

		lastTerm = new Term( _T(""), _T(""),true);

		CND_CONDITION(lastTerm != NULL,"Could not allocate memory for lastTerm")
    
		lastTi  = new TermInfo();

        CND_CONDITION(lastTi != NULL,"Could not allocate memory for lastTi")

		lastIndexPointer = 0;
		size             = 0;
		isIndex          = IsIndex;

	    const char_t* buf = segmentname(segment, (isIndex ? _T(".tii") : _T(".tis")));
	    output = &directory.createFile( buf );
	    _DELETE_ARRAY(buf);

		//leave space for size
        output->writeInt(0);
        //Set other to NULL by Default
        other = NULL;
    }

	TermInfosWriter::~TermInfosWriter(){
    //Func - Destructor
	//Pre  - true
	//Post - de instance has been destroyed

		close();
	}
	
	void TermInfosWriter::add(Term& term, const TermInfo& ti) {
	//Func - Writes a Term and TermInfo to the outputstream
	//Pre  - Term must be lexicographically greater than all previous Terms added.
    //       Pointers of TermInfo ti (freqPointer and proxPointer) must be positive and greater than all previous.

		CND_PRECONDITION(isIndex || (!isIndex  && term.compareTo(*lastTerm) > 0),"term out of order")
		CND_PRECONDITION(ti.freqPointer >= lastTi->freqPointer,"freqPointer out of order")
		CND_PRECONDITION(ti.proxPointer >= lastTi->proxPointer,"proxPointer out of order")

		if (!isIndex && size % LUCENE_WRITER_INDEX_INTERVAL == 0){
            //add an index term
			other->add(*lastTerm, *lastTi);		  
		    }

		//write term
		writeTerm(term);				      
		// write doc freq
        output->writeVInt(ti.docFreq);		  
        //write pointers
		output->writeVLong(ti.freqPointer - lastTi->freqPointer);
		output->writeVLong(ti.proxPointer - lastTi->proxPointer);

		if (isIndex){
			output->writeVLong(other->output->getFilePointer() - lastIndexPointer);
			lastIndexPointer = other->output->getFilePointer(); // write pointer
            }

		lastTi->set(ti);
		size++;
	}

	void TermInfosWriter::close() {
    //Func - Closes the TermInfosWriter
	//Pre  - true
	//Post - The TermInfosWriter has been closed
    
		if (output){
			//write size at start
		    output->seek(0);				  
		    output->writeInt(size);
		    output->close();
		    }
    
		_DELETE(output);

		if (!isIndex){
			if(other){
			    other->close();
			    _DELETE( other );
			    }
		    }
        
        lastTerm->finalize();
		lastTerm = NULL;

		_DELETE(lastTi);
	}

	void TermInfosWriter::writeTerm(Term& term) {
		int_t start = stringDifference(lastTerm->Text(), term.Text());
		int_t length = stringLength(term.Text()) - start;
	    
		output->writeVInt(start);			  // write shared prefix length
		output->writeVInt(length);			  // write delta length
		output->writeChars(term.Text(), start, length);  // write delta chars

		output->writeVInt(fieldInfos.fieldNumber(term.Field())); // write field num

		lastTerm->finalize();
		lastTerm = term.pointer();
	}

    //static 
	int_t TermInfosWriter::stringDifference(const char_t* s1, const char_t* s2) {
		int_t len1 = stringLength(s1);
		int_t len2 = stringLength(s2);
		int_t len = len1 < len2 ? len1 : len2;
		for (int_t i = 0; i < len; i++)
			if ( s1[i] != s2[i])
				return i;
		return len;
	}

	//static 
	const char_t* TermInfosWriter::segmentname( const char_t* segment, const char_t* ext ){
		char_t* buf = new char_t[MAX_PATH];
		stringPrintF(buf,_T("%s%s"), segment,ext );
		return buf;
	}


}}
#endif
