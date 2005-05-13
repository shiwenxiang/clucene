#include "stdafx.h"

namespace lucene{ namespace test{

	void StoreTest(int_t count, bool ram){
		srand(1251971);
		int_t i;
		
		long_t veryStart = lucene::util::Misc::currentTimeMillis();
		long_t start = lucene::util::Misc::currentTimeMillis();

		Directory& store = (ram?(Directory&)*new RAMDirectory(): (Directory&)FSDirectory::getDirectory(_T("test.store"), true) );
		int_t LENGTH_MASK = 0xFFF;
		char_t name[260];

		for (i = 0; i < count; i++) {
			stringPrintF(name,_T("%d.dat"),i);

			int_t length = rand() & LENGTH_MASK;
			l_byte_t b = (l_byte_t)(rand() & 0x7F);

			OutputStream& file = store.createFile(name);

			for (int_t j = 0; j < length; j++)
				file.writeByte(b);
			    
			file.close();
			delete &file;
		}
		_cout << (lucene::util::Misc::currentTimeMillis() - start) << _T(" total milliseconds to create") << endl;

		if (!ram){
			//store.close();
			store = (Directory&)FSDirectory::getDirectory(_T("test.store"), false);
		}
		srand(1251971);
		start = lucene::util::Misc::currentTimeMillis();

		for (i = 0; i < count; i++) {
			stringPrintF(name,_T("%d.dat"),i);
			int_t length = rand() & LENGTH_MASK;
			l_byte_t b = (l_byte_t)(rand() & 0x7F);
			InputStream& file = store.openFile(name);

			if (file.Length() != length)
			  throw; // exception( "length incorrect" );

			for (int_t j = 0; j < length; j++)
				if (file.readByte() != b)
				  throw;// exception( "contents incorrect" );

			file.close();
			delete &file;
		}


		_cout << (lucene::util::Misc::currentTimeMillis() - start) <<  _T(" total milliseconds to read") << endl;

		srand(1251971);
		start = lucene::util::Misc::currentTimeMillis();

		for (i = 0; i < count; i++) {
			stringPrintF(name,_T("%d.dat"),i);
			store.deleteFile(name);
		}

		_cout << (lucene::util::Misc::currentTimeMillis() - start) << _T(" total milliseconds to delete") << endl;
		_cout << (lucene::util::Misc::currentTimeMillis() - veryStart) << _T(" total milliseconds") << endl;

		store.close();
		if ( ram )
			delete &store;
	}
}}
