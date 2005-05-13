#include "CLucene/StdHeader.h"
#include "TopDocs.h"

#include "ScoreDoc.h"
namespace lucene{ namespace search {
	

	TopDocs::TopDocs(const int_t th, ScoreDoc **sds, const int_t sdLength):
        totalHits(th),scoreDocsLength(sdLength){
    //Func - Constructor
	//Pre  - sds may or may not be NULL
	//       sdLength >= 0
	//Post - The instance has been created

		CND_PRECONDITION(sdLength >= 0,"sdLength is a negative number")

		scoreDocs       = sds;
		deleteScoreDocs = true;

	}

	TopDocs::~TopDocs(){
    //Func - Destructor
	//Pre  - true
	//Post - The instance has been destroyed

		//Check if scoreDocs is valid
		if (scoreDocs){
            if ( deleteScoreDocs ){
				for ( int_t i=0;i<scoreDocsLength;i++ ){
				    _DELETE(scoreDocs[i]);
				    }
		        }
		    
            _DELETE_ARRAY(scoreDocs);
		    }
	}
}}
