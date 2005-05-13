#include "CLucene/StdHeader.h"
#ifndef CLUCENE_LITE
#include "FieldsWriter.h"

#include "CLucene/util/VoidMap.h"
#include "CLucene/util/Reader.h"
#include "CLucene/store/Directory.h"
#include "CLucene/store/OutputStream.h"
#include "CLucene/document/Document.h"
#include "CLucene/document/Field.h"
#include "FieldInfos.h"

namespace lucene{ namespace index {


	//static
	const char_t* FieldsWriter::segmentname( const char_t* segment, const char_t* ext ){
		char_t* buf = new char_t[MAX_PATH];
		stringPrintF(buf,_T("%s%s"), segment,ext );
		return buf;
	}

	FieldsWriter::FieldsWriter(Directory& d, const char_t* segment, FieldInfos& fn):
		fieldInfos(fn){
	//Func - Constructor
	//Pre  - d contains a valid reference to a directory
	//       segment != NULL and contains the name of the segment
	//Post - fn contains a valid reference toa a FieldInfos

		CND_PRECONDITION(segment != NULL,"segment is NULL")

		const char_t* buf = segmentname(segment,_T(".fdt"));
		fieldsStream = &d.createFile ( buf );
		_DELETE_ARRAY(buf);

		CND_CONDITION(fieldsStream != NULL,"fieldsStream is NULL")

		buf = segmentname(segment,_T(".fdx"));
		indexStream = &d.createFile( buf );
		_DELETE_ARRAY(buf);

		CND_CONDITION(indexStream != NULL,"indexStream is NULL")
	}

	FieldsWriter::~FieldsWriter(){
	//Func - Destructor
	//Pre  - true
	//Post - Instance has been destroyed

		close();
	}

	void FieldsWriter::close() {
	//Func - Closes all streams and frees all resources
	//Pre  - true
	//Post - All streams have been closed all resources have been freed

		//Check if fieldsStream is valid
		if (fieldsStream){
			//Close fieldsStream
			fieldsStream->close();
			_DELETE( fieldsStream );
			}

		//Check if indexStream is valid
		if (indexStream){
			//Close indexStream
			indexStream->close();
			_DELETE( indexStream );
			}
	}

	void FieldsWriter::addDocument(lucene::document::Document& doc) {
	//Func - Adds a document
	//Pre  - doc contains a valid reference to a Document
	//       indexStream != NULL
	//       fieldsStream != NULL
	//Post - The document doc has been added

		CND_PRECONDITION(indexStream != NULL,"indexStream is NULL")
		CND_PRECONDITION(fieldsStream != NULL,"fieldsStream is NULL")

		indexStream->writeLong(fieldsStream->getFilePointer());

		int_t storedCount = 0;
		lucene::document::DocumentFieldEnumeration* fields = doc.fields();
		while (fields->hasMoreElements()) {
			lucene::document::Field* field = fields->nextElement();
			if (field->IsStored())
				storedCount++;
		}
		_DELETE(fields);
		fieldsStream->writeVInt(storedCount);

		fields = doc.fields();
		while (fields->hasMoreElements()) {
			lucene::document::Field* field = fields->nextElement();
			if (field->IsStored()) {
				fieldsStream->writeVInt(fieldInfos.fieldNumber(field->Name()));

				l_byte_t bits = 0;
				if (field->IsTokenized())
					bits |= 1;
				//FEATURE: can this data be compressed? A bit could be set to compress or not compress???
				fieldsStream->writeByte(bits);

				//FEATURE: this problem in Java Lucene too, if using Reader, data is not stored.
				if ( field->StringValue() == NULL ){
					lucene::util::Reader* r = field->ReaderValue();
					// Though Reader::position returns long_t, we assume that
					// a single field won't exceed int_t's capacity.
					int_t rp = static_cast<int_t>(r->position());
					r->seek(0);
					// Though Reader::available returns long_t, we assume that
					// a single field won't exceed int_t's capacity.
					int_t rl = static_cast<int_t>(r->available());

					char_t* rv = new char_t[rl];
					r->read(rv,0,rl);
					r->seek(rp); //reset position

					//simulate writeString
					fieldsStream->writeVInt(rl);
					fieldsStream->writeChars(rv,0,rl);
					_DELETE_ARRAY(rv);
				}else
					fieldsStream->writeString(field->StringValue());
			}
		}
		_DELETE(fields);
	}

}}
#endif
