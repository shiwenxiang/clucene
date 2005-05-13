#include "CLucene/StdHeader.h"
#include "FieldsReader.h"

#include "CLucene/util/VoidMap.h"
#include "CLucene/store/Directory.h"
#include "CLucene/document/Document.h"
#include "CLucene/document/Field.h"
#include "FieldInfos.h"

namespace lucene{ namespace index {

	//static 
	const char_t* FieldsReader::segmentname(const char_t* segment, const char_t* ext ){
		char_t* buf = new char_t[MAX_PATH];
		stringPrintF(buf,_T("%s%s"), segment,ext );
		return buf;
	}

	FieldsReader::FieldsReader(Directory& d, const char_t* segment, FieldInfos& fn):
		fieldInfos(fn){
    //Func - Constructor
	//Pre  - d contains a valid reference to a Directory
	//       segment != NULL
    //       fn contains a valid reference to a FieldInfos
	//Post - The instance has been created

		CND_PRECONDITION(segment != NULL,"segment != NULL")

		const char_t* buf = segmentname(segment,_T(".fdt"));
		fieldsStream = &d.openFile( buf );
		_DELETE_ARRAY(buf);

		buf = segmentname(segment,_T(".fdx"));
		indexStream = &d.openFile( buf );
		_DELETE_ARRAY(buf);

		size = (int_t)indexStream->Length()/8;
	}

    FieldsReader::~FieldsReader(){
    //Func - Destructor
	//Pre  - true
	//Post - The instance has been destroyed

		close();
    }
    
	void FieldsReader::close() {
    //Func - Closes the FieldsReader
    //Pre  - true
	//Post - The FieldsReader has been closed
        
		if (fieldsStream){
		    fieldsStream->close();
		    }
		if(indexStream){
		    indexStream->close();
		    }
		_DELETE(fieldsStream);
		_DELETE(indexStream);
	}

	int_t FieldsReader::Size() {
		return size;
	}

	lucene::document::Document& FieldsReader::doc(int_t n) {
		indexStream->seek(n * 8L);
		long_t position = indexStream->readLong();
		fieldsStream->seek(position);
	    
		lucene::document::Document& doc = *new lucene::document::Document();
		int_t numFields = fieldsStream->readVInt();
		for (int_t i = 0; i < numFields; i++) {
			int_t fieldNumber = fieldsStream->readVInt();
			FieldInfo& fi = fieldInfos.fieldInfo(fieldNumber);

			l_byte_t bits = fieldsStream->readByte();

			char_t* fvalue = fieldsStream->readString(true);
			lucene::document::Field& f = *new lucene::document::Field(
							fi.name,		  // name
							fvalue, // read value
							true,			  // stored
							fi.isIndexed,		  // indexed
							(bits & 1) != 0 );
			delete[] fvalue;

			doc.add( f );	  // tokenized
		}

		return doc;
	}

}}
