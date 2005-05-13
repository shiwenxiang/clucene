#include "CLucene/StdHeader.h"
#include "FieldInfos.h"

#include "CLucene/store/Directory.h"
#include "CLucene/document/Document.h"
#include "CLucene/document/Field.h"
#include "CLucene/util/VoidMap.h"
#include "FieldInfo.h"

using namespace std;
namespace lucene{ namespace index {

	FieldInfos::FieldInfos() {
		addInternal(_T(""), true,false); //add( _T(""), false);
		byName.setDoDelete(false,DELETE_TYPE_DELETE);
		byName.setDoDelete(true,DELETE_TYPE_DELETE_ARRAY);
	}
	
	FieldInfos::~FieldInfos(){

	}

	FieldInfos::FieldInfos(Directory& d, const char_t* name) {
		InputStream& input = d.openFile(name);
		_TRY {
			read(input);
		} _FINALLY (
		    input.close();
		    delete &input;
		);

		byName.setDoDelete(false,DELETE_TYPE_DELETE);
		byName.setDoDelete(true,DELETE_TYPE_DELETE_ARRAY);
	}

#ifndef CLUCENE_LITE
	void FieldInfos::add(lucene::document::Document& doc) {
		lucene::document::DocumentFieldEnumeration* fields  = doc.fields();
		while (fields->hasMoreElements()) {
			lucene::document::Field* field = fields->nextElement();
			add(field->Name(), field->IsIndexed());
		}
		delete fields;
	}

	void FieldInfos::add(FieldInfos& other) {
		for (int_t i = 0; i < other.size(); i++) {
			FieldInfo& fi = other.fieldInfo(i);
			add(fi.name, fi.isIndexed);
		}
	}

	void FieldInfos::add( char_t* name, const bool isIndexed) {
		if ( byName.exists(name) ){
			FieldInfo& fi = fieldInfo(name);
			if (fi.isIndexed != isIndexed)
				fi.isIndexed = true;
		}else
			addInternal(name, true,isIndexed);
	}

#endif // CLUCENE_LITE

	int_t FieldInfos::fieldNumber(const char_t* fieldName)const {
		try{
			FieldInfo* fi = &fieldInfo(fieldName);
			return fi->number;
		}catch(...){}
		return -1;
	}


	FieldInfo& FieldInfos::fieldInfo(const char_t* fieldName) const {
		return *byName.get(fieldName);
	}
	const char_t* FieldInfos::fieldName(const int_t fieldNumber)const {
		return fieldInfo(fieldNumber).name;
	}

	FieldInfo& FieldInfos::fieldInfo(const int_t fieldNumber) const {
		return *byNumber.at(fieldNumber);
	}

	int_t FieldInfos::size()const {
		return byNumber.size();
	}

#ifndef CLUCENE_LITE
	void FieldInfos::write(lucene::store::Directory& d, const char_t* name) {
		OutputStream& output = d.createFile(name);
		_TRY {
			write(output);
		} _FINALLY (
		    output.close();
		    delete &output;
		);
	}

	void FieldInfos::write(lucene::store::OutputStream& output) {
		output.writeVInt(size());
		for (int_t i = 0; i < size(); i++) {
			FieldInfo& fi = fieldInfo(i);
			output.writeString(fi.name);
			output.writeByte((l_byte_t)(fi.isIndexed ? 1 : 0));
		}
	}
#endif //CLUCENE_LITE

	void FieldInfos::read(lucene::store::InputStream& input) {
		int_t size = input.readVInt();
		for (int_t i = 0; i < size; i++){
			char_t* rs = input.readString(true); //calling convention is right to left ihere
			addInternal(rs, false, input.readByte() != 0);
		}
	}
	void FieldInfos::addInternal( char_t* name, const bool dupName, const bool isIndexed) {
		FieldInfo* fi = new FieldInfo(name, isIndexed, byNumber.size());
		byNumber.push_back(fi);
		const char_t* tmp = name;
		if ( dupName )
			tmp = stringDuplicate( name );
        byName.put( tmp, fi);
	}

}}
