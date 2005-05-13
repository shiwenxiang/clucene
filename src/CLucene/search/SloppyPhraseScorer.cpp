#include "CLucene/StdHeader.h"
#ifndef NO_FUZZY_QUERY
#include "SloppyPhraseScorer.h"

#include "PhraseScorer.h"
#include "CLucene/index/Terms.h"

using namespace lucene::index;
namespace lucene{ namespace search{
/*
    changed pq.into pq->

	Added Documenation and condition checks
*/

  SloppyPhraseScorer::SloppyPhraseScorer(TermPositions** tps, int_t tpsLength, int_t s, l_byte_t* n, float_t w):
      PhraseScorer(tps,tpsLength,n,w){
  //Func - Constructor
  //Pre  - tps != NULL 
  //       tpsLength >= 0
  //       n != NULL
  //Post - Instance has been created

      CND_PRECONDITION(tps != NULL,"tps is NULL")
      CND_PRECONDITION(tpsLength >= 0,"tpsLength is a negative number")
      CND_PRECONDITION(n != NULL,"n is NULL")
        
      slop = s;
  }

  float_t SloppyPhraseScorer::phraseFreq() {
  //Func - Returns the freqency of the phrase
  //Pre  - first != NULL
  //       last  != NULL
  //       pq    != NULL
  //Post - The frequency of the phrase has been returned

	  CND_PRECONDITION(first != NULL,"first is NULL")
	  CND_PRECONDITION(last  != NULL,"last is NULL")
	  CND_PRECONDITION(pq    != NULL,"pq is NULL")

	  //Clear the PhraseQueue pq;
      pq->clear();

      int_t end = 0;

	  //declare iterator
      PhrasePositions* pp = NULL;

      // build pq from list

	  //Sort the list of PhrasePositions using pq
      for (pp = first; pp != NULL; pp = pp->next) {
		  //Read the first TermPosition of the current PhrasePositions pp
          pp->firstPosition();
		  //Check if the position of the pp is bigger than end
		  if (pp->position > end){
              end = pp->position;
		      }
          //Store the current PhrasePositions pp into the PhraseQueue pp
          pq->put(pp);		
          }

     float_t freq = 0.0f;
     
	 bool done = false;
     
	 do {
		 //Pop a PhrasePositions pp from the PhraseQueue pp
         pp = pq->pop();
         //Get start position
         int_t start = pp->position;
		 //Get next position
		 int_t next = pq->top()->position;

         for (int_t pos = start; pos <= next; pos = pp->position) {
             //advance pp to min window
             start = pos;				  
             
             if (!pp->nextPosition()) {
				 //ran out of a term -- done
                 done = true;
                 break;
                 }
             }
         
         //Calculate matchLength
		 int_t matchLength = end - start;
         //Check if matchLength is smaller than slop
         if (matchLength <= slop){
             // penalize longer matches
             freq += 1.0 / (matchLength + 1);	  
             }

		 if (pp->position > end){
             end = pp->position;
             }
         
         //restore pq
		 pq->put(pp);				  
		 } 
     while (!done);

     return freq;
  }
}}
#endif
