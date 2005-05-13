#include "CLucene/StdHeader.h"
#include "ExactPhraseScorer.h"

#include "PhraseScorer.h"
#include "CLucene/index/Terms.h"

using namespace lucene::index;
namespace lucene{ namespace search{

	ExactPhraseScorer::ExactPhraseScorer(TermPositions** tps, int_t tpsLength, l_byte_t* n, float_t w):
		PhraseScorer(tps, tpsLength, n, w){
	//Func - Constructor
	//Pre  - tps != NULL
	//       tpsLength >= 0
	//       n != NULL
	//Post - Instance has been created

		CND_PRECONDITION(tps != NULL,"tps is NULL")
		CND_PRECONDITION(tpsLength >= 0,"tpsLength is a negative number")
		CND_PRECONDITION(n != NULL,"n is NULL")

	}

	float_t ExactPhraseScorer::phraseFreq(){
	//Func - Returns the freqency of the phrase
	//Pre  - first != NULL
	//       last  != NULL
	//       pq    != NULL
	//       size of the PhraseQueue pq is 0
	//Post - The frequency of the phrase has been returned

		CND_PRECONDITION(first != NULL,"first is NULL")
		CND_PRECONDITION(last  != NULL,"last is NULL")
		CND_PRECONDITION(pq    != NULL,"pq is NULL")
		CND_PRECONDITION(pq->Size()==0,"pq is not empty")

		//declare iterator
		PhrasePositions* pp = NULL;

		//build pq from list

		//Add the nodes of the list of PhrasePositions and store them
		//into the PhraseQueue pq so it can used to build
		//a list of sorted nodes
		for (pp = first; pp != NULL; pp = pp->next) {
			//Read the first TermPosition of the current PhrasePositions pp
			pp->firstPosition();
			//Store the current PhrasePositions pp into the PhraseQueue pq
			pq->put(pp);
		}
		//pqToList requires that first and last be NULL when it's called.  In
		//this case, the nodes of the linked list are referenced by pq (see
		//above loop), so we can clear our pointers to the head and tail of the
		//linked list without fear of leaking the nodes.
		first = NULL;
		last = NULL;
		//rebuild list from pq
		pqToList();

		//Initialize freq at 0
		int_t freq = 0;

		//find position with all terms
		do {
			//scan forward in first
			while (first->position < last->position){
				do{
					if (!first->nextPosition()){
						return (float_t)freq;
					}
				} while (first->position < last->position);
				//Make the current first node the last node in the list
				firstToLast();
			}
			//all equal: a match has been found
			freq++;
		} while (last->nextPosition());

		return (float_t)freq;
	}
}}
