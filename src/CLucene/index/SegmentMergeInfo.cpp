#include "CLucene/StdHeader.h"
#include "SegmentMergeInfo.h"

#include "CLucene/util/BitVector.h"
#include "SegmentTermEnum.h"
#include "SegmentHeader.h"

namespace lucene{ namespace index {

	SegmentMergeInfo::SegmentMergeInfo(const int_t b, SegmentTermEnum& te, SegmentReader& r):
        reader(r),termEnum(te),postings ( *new SegmentTermPositions(&r) ){
    //Func - Constructor
    //Pre  - b >= 0
	//       te contains a valid reference to a SegmentTermEnum instance
	//       r contains a valid reference to a SegmentReader instance
    //Post - The instance has been created
	
        CND_PRECONDITION(b >= 0, "b is a negative number")

		docMap = NULL;
		
		base   = b;

		term   = te.getTerm();

		if (reader.deletedDocs != NULL) {
			// build array which maps document numbers around deletions 
			lucene::util::BitVector& deletedDocs = *reader.deletedDocs;
			//Get the total number of documents managed by the reader including the deleted ones
			int_t maxDoc = reader.MaxDoc();
			//Create a map for all documents
			docMap = new int_t[maxDoc];
			int_t j = 0;
			//Iterate through all the document numbers
			for (int_t i = 0; i < maxDoc; i++) {
                //Check if document i is marked deleted
				if (deletedDocs.get(i)){
					//Document i has not been marked deleted so assign -1
					docMap[i] = -1;
				    }
				else{
					docMap[i] = j++;
				    }
			}
		}
	}

	SegmentMergeInfo::~SegmentMergeInfo(){
    //Func - Destructor
    //Pre  - true
    //Post - The instance has been destroyed

        //First make sure posting has been closed
        postings.close();
        delete &postings;

		termEnum.close();
        delete &termEnum;

		term->finalize();
		_DELETE_ARRAY(docMap);
	}


	bool SegmentMergeInfo::next() {
    //Func - Moves the current term of the enumeration termEnum to the next and term
	//       points to this new current term
    //Pre  - true
    //Post - Returns true if the term has been moved to the next otherwise false

		
		if (termEnum.next()) {
			term->finalize();
			term = termEnum.getTerm();
			return true;
		} else {
			term->finalize(); //TODO: test HighFreqTerms errors with this
			term = NULL;
			return false;
		}
	}

	void SegmentMergeInfo::close() {
    //Func - Closes the the resources
    //Pre  - true
    //Post - The resources have been closed

		termEnum.close();
		postings.close();
	}

}}
