#include "CLucene/StdHeader.h"
#include "PhraseScorer.h"

#include "PhraseQueue.h"
#include "PhrasePositions.h"
#include "HitCollector.h"
#include "Scorer.h"
#include "Similarity.h"

namespace lucene{ namespace search{

	PhraseScorer::PhraseScorer(TermPositions** tps, const int_t tpsLength, l_byte_t* n, float_t w){
	//Func - Constructor
	//Pre  - tps != NULL and is an array of TermPositions
	//       tpsLength >= 0
	//       n != NULL
	//Post - The instance has been created

		CND_PRECONDITION(tps != NULL,"tps is NULL")
		CND_PRECONDITION(tpsLength >= 0,"tpsLength is a negative number")
		CND_PRECONDITION(n != NULL,"n is NULL")

		norms   = n;
		weight  = w;

		//reset internal pointers
		first   = NULL;
		last    = NULL;

		pq = new PhraseQueue(tpsLength);

		CND_CONDITION(pq != NULL,"Could not allocate memory for pq")

		PhrasePositions *PhrasePos = NULL;

		//use pq to build a sorted list of PhrasePositions
		for (int_t i = 0; i < tpsLength; i++){
			PhrasePos  = new PhrasePositions(tps[i], i);

			CND_CONDITION(PhrasePos != NULL,"Could not allocate memory for PhrasePos")
			//Store PhrasePos into the PhrasePos pq
			pq->put(PhrasePos);
		}

		pqToList();
	}

	PhraseScorer::~PhraseScorer() {
	//Func - Destructor
	//Pre  - true
	//Post - The instance has been destroyed

		//The PhraseQueue pq (which is a PriorityQueue) pq is actually empty at present, the elements
		//having been transferred by pqToList() to the linked list starting with
		//first.  The nodes of that linked list are deleted by the destructor of
		//first, rather than the destructor of pq.
		_DELETE(first);
		_DELETE(pq);
	}

	void PhraseScorer::score(HitCollector& results, const int_t end){
	//Func -
	//Pre  - first != NULL
	//       last != NULL
	//Post -

		CND_PRECONDITION(first != NULL,"first is NULL")
		CND_PRECONDITION(last  != NULL,"last is NULL")

		// find doc with all the terms
		while (last->doc < end) {

			// scan forward in first
			while (first->doc < last->doc){
				do  {
					first->Next();
				} while (first->doc < last->doc);

				//Make the current first the last node
				firstToLast();
				if (last->doc >= end){
					return;
				}
			}

			//found doc with all terms

			//check for phrase
			float_t freq = phraseFreq();

			if (freq > 0.0){
				//Compute score
				float_t score = Similarity::tf(freq)*weight;
				//Normalize
				score *= Similarity::normf(norms[first->doc]);
				//Add to results
				results.collect(first->doc, score);
			}
			//Resume scanning
			last->Next();
		}
	}


	void PhraseScorer::pqToList(){
	//Func - Transfers the PhrasePositions from the PhraseQueue pq to
	//       the PhrasePositions list with first as its first element
	//Pre  - pq != NULL
	//       first = NULL
	//       last = NULL
	//Post - All PhrasePositions have been transfered to the list
	//       of PhrasePositions of which the first element is pointed to by first
	//       and the last element is pointed to by last

		CND_PRECONDITION(pq != NULL,"pq is NULL")
		CND_PRECONDITION(first == NULL,"first must be NULL")
		CND_PRECONDITION(last == NULL,"last must be NULL")

		PhrasePositions* PhrasePos = NULL;

		//As long pq is not empty
		while (pq->top() != NULL){
			//Pop a PhrasePositions instance
			PhrasePos = pq->pop();

			// add next to end of list
			if (last != NULL) {
				last->next = PhrasePos;
			} else {
				first = PhrasePos;
			}

			//Let last point to the new last PhrasePositions instance just added
			last = PhrasePos;
			//Reset the next of last to NULL
			last->next = NULL;
		}

		//Check to see that pq is empty now
		CND_CONDITION(pq->Size()==0,"pq is not empty while it should be")
	}

	void PhraseScorer::firstToLast(){
	//Func - Moves first to the end of the list
	//Pre  - first is NULL or points to an PhrasePositions Instance
	//       last  is NULL or points to an PhrasePositions Instance
	//       first and last both are NULL or both are not NULL
	//Post - The first element has become the last element in the list

		CND_PRECONDITION(((first==NULL && last==NULL) ||(first !=NULL && last != NULL)),
					   "Either first or last is NULL but not both")

		//Check if first and last are valid pointers
		if(first && last){
			last->next = first;
			last = first;
			first = first->next;
			last->next = NULL;
		}
	}
}}
