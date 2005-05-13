#include "CLucene/StdHeader.h"
#include "TermInfo.h"


namespace lucene{ namespace index {

	TermInfo::TermInfo(){
	//Func - Constructor
	//Pre  - true
	//Post - Instance has been created
				
		docFreq     = 0;
		freqPointer = 0;
		proxPointer = 0;
    }

    TermInfo::~TermInfo(){
	//Func - Destructor.
	//Pre  - true
	//Post - Instance has been destroyed
    }

	TermInfo::TermInfo(const int_t df, const long_t fp, const long_t pp){
	//Func - Constructor. 
	//Pre  - df >= 0, fp >= 0 pp >= 0
	//Post - An instance has been created with FreqPointer = fp, proxPointer=pp and docFreq= df

        CND_PRECONDITION(df >= 0, "df contains negative number")
        CND_PRECONDITION(fp >= 0, "fp contains negative number")
        CND_PRECONDITION(pp >= 0, "pp contains negative number")

        freqPointer = fp;
        proxPointer = pp;
		docFreq     = df;
	}

	TermInfo::TermInfo(const TermInfo& ti) {
    //Func - Constructor. 
	//       Initialises this instance by copying the values of another TermInfo ti
    //Pre  - ti is a reference to another TermInfo
	//       ti.docFreq >= 0
	//       ti.freqPointer >= 0
	//       ti.proxPointer >= 0
    //Post - Values of ti have been copied to the values of this Instance.

        CND_PRECONDITION(ti.docFreq     >= 0, "ti.docFreq contains negative number")
        CND_PRECONDITION(ti.freqPointer >= 0, "ti.freqPointer contains negative number")
        CND_PRECONDITION(ti.proxPointer >= 0, "ti.proxPointer contains negative number")

		docFreq     = ti.docFreq;
		freqPointer = ti.freqPointer;
		proxPointer = ti.proxPointer;
	}

	void TermInfo::set(const int_t df, const long_t fp, const long_t pp) {
    //Func - Sets a new document frequency, a new freqPointer and a new proxPointer
    //Pre  - df >= 0, fp >= 0 pp >= 0
    //Post - The new document frequency, a new freqPointer and a new proxPointer
	//       have been set

        CND_PRECONDITION(df >= 0, "df contains negative number")
        CND_PRECONDITION(fp >= 0, "fp contains negative number")
        CND_PRECONDITION(pp >= 0, "pp contains negative number")

		docFreq     = df;
		freqPointer = fp;
		proxPointer = pp;
	}

	void TermInfo::set(const TermInfo& ti) {
    //Func - Sets a new document frequency, a new freqPointer and a new proxPointer
	//       by copying these values from another instance of TermInfo
    //Pre  - ti is a reference to another TermInfo
	//       ti.docFreq >= 0
	//       ti.freqPointer >= 0
	//       ti.proxPointer >= 0
    //Post - Values of ti have been copied to the values of this Instance.

        CND_PRECONDITION(ti.docFreq     >= 0, "ti.docFreq contains negative number")
        CND_PRECONDITION(ti.freqPointer >= 0, "ti.freqPointer contains negative number")
        CND_PRECONDITION(ti.proxPointer >= 0, "ti.proxPointer contains negative number")

		docFreq     = ti.docFreq;
		freqPointer = ti.freqPointer;
		proxPointer = ti.proxPointer;
	}
}}

