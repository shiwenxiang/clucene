#include "CLucene/StdHeader.h"
#include "PhrasePositions.h"

#include "CLucene/index/Terms.h"

using namespace lucene::index;
namespace lucene{ namespace search{

  PhrasePositions::PhrasePositions(TermPositions* Tp, const int_t OffSet){
  //Func - Constructor
  //Pre  - t != NULL
  //       OffSet != NULL
  //Post - The instance has been created

      CND_PRECONDITION(Tp != NULL,"Tp is NULL")
      CND_PRECONDITION(OffSet >= 0 ,"OffSet is a negative number")

      tp       = Tp;
      offset   = OffSet;
      position = 0;
      count    = 0;

      next     = NULL;
      
	  Next();
  }
	
  PhrasePositions::~PhrasePositions(){
  //Func - Destructor
  //Pre  - true
  //Post - The instance has been deleted

    //delete next Phrase position and by doing that
    //all PhrasePositions in the list
    _DELETE(next);

    //Check if tp is valid
    if ( tp != NULL ){
        //Close TermPositions tp
        tp->close();
        _DELETE(tp);
        }
  }

  void PhrasePositions::Next(){
  //Func - Increments to next doc
  //Pre  - tp != NULL
  //Post - if there was no next then doc = INT_MAX otherwise
  //       doc contains the current document number

      CND_PRECONDITION(tp != NULL,"tp is NULL")
    
	  //Move to the next in TermPositions tp
      if (!tp->next()) {
		  //There is no next so close the stream
          tp->close();				  
		  //delete tp and reset tp to NULL
          _DELETE(tp)
		  //Assign Doc sentinel value
          doc = INT_MAX; 
		  }
	  else{
          doc  = tp->Doc();
		  position = 0;
	      }
  }

  void PhrasePositions::firstPosition(){
  //Func - Read the first TermPosition
  //Pre  - tp != NULL
  //Post - 

      CND_PRECONDITION(tp != NULL,"tp is NULL")    

      //read first pos
      count = tp->Freq();				  
      //Move to the next TermPosition
	  nextPosition();
  }

  bool PhrasePositions::nextPosition(){
  //Func - Move to the next position
  //Pre  - tp != NULL
  //Post -

      CND_PRECONDITION(tp != NULL,"tp is NULL")    

      if (count-- > 0) {				  
		  //read subsequent pos's
          position = tp->nextPosition() - offset;

		  //Check position always bigger than or equal to 0
          CND_CONDITION(position >= 0, "position has become a negative number")
          return true;
          }  
	  else{
          return false;
          }
	}
}}
