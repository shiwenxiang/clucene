#include "CLucene/StdHeader.h"

#include "FilteredTermEnum.h"

using namespace lucene::index;
namespace lucene{ namespace search {

    FilteredTermEnum::FilteredTermEnum(IndexReader& reader, Term* term){
	//Func - Constructor
	//Pre  - true
	//Post - Instance has been created
		
        currentTerm = NULL;
        actualEnum = NULL;
    }

    FilteredTermEnum::~FilteredTermEnum() {
    //Func - Destructor
	//Pre  - true
	//Post - The instance has been destroyed
      
        close();
    }

	int_t FilteredTermEnum::DocFreq() const {
	//Func - Returns the docFreq of the current Term in the enumeration.
	//Pre  - next() must have been called at least once
	//Post - if actualEnum is NULL result is -1 otherwise the frequencey is returned

		if (actualEnum == NULL){
			return -1;
		    }
        return actualEnum->DocFreq();
    }
    
    bool FilteredTermEnum::next() {
    //Func - Increments the enumeration to the next element.  
	//Pre  - true
	//Post - Returns True if the enumeration has been moved to the next element otherwise false

		//The actual enumerator is not initialized!
		if (actualEnum == NULL){
			return false; 
		    }

		//Finalize the currentTerm and reset it to NULL
        currentTerm->finalize();
        currentTerm = NULL;

		//Iterate through the enumeration
        while (currentTerm == NULL) {
            if (EndEnum()) 
				return false;
            if (actualEnum->next()) {
                //Order getTerm not to return reference ownership here. */
                Term* term = actualEnum->getTerm(false);
				//Compare the retrieved term
                if (termCompare(term)){
					//Matched so finalize the current
                    currentTerm->finalize();
					//Get a reference to the matched term
                    currentTerm = term->pointer();
                    return true;
                    }
                }
            else 
                return false;
        }
        currentTerm->finalize();
        currentTerm = NULL;

        return false;
    }

    Term* FilteredTermEnum::getTerm(const bool pointer) {
	//Func - Returns the current Term in the enumeration.
	//Pre  - next() must have been called at least once
	//       pointer is true or false
	//Post - if pre(pointer) is true the reference counter of currentTerm is increased
	//       and current Term is returned otherwise currentTerm is only returned

        if ( pointer && currentTerm!=NULL )
            return currentTerm->pointer();
        else
            return currentTerm;
    }

    void FilteredTermEnum::close(){
	//Func - Closes the enumeration to further activity, freeing resources.
	//Pre  - true
	//Post - The Enumeration has been closed

		//Check if actualEnum is valid
		if (actualEnum){
			//Close the enumeration
            actualEnum->close();
		    }

        //Destroy the enumeration
        _DELETE(actualEnum);

		//Destroy currentTerm
        currentTerm->finalize();
        currentTerm = NULL;
    }

	void FilteredTermEnum::setEnum(TermEnum* actualEnum) {
	//Func - Sets the actual Enumeration
	//Pre  - actualEnum != NULL
	//Post - The instance has been created

		CND_PRECONDITION(actualEnum != NULL,"actualEnum is NULL");       

		_DELETE(this->actualEnum);

        this->actualEnum = actualEnum;

        // Find the first term that matches
        //Ordered getTerm not to return reference ownership here.
        Term* term = actualEnum->getTerm(false);
        if (termCompare(term)){
            currentTerm->finalize();
            currentTerm = term->pointer();
            }
		else{
            next();
		    }
    }

}}
