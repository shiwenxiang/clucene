#include "CLucene/StdHeader.h"
#include "SegmentMergeQueue.h"

#include "SegmentMergeInfo.h"
namespace lucene{ namespace index {


	SegmentMergeQueue::SegmentMergeQueue(const int_t size) {
	//Func - Constructor
	//       Creates a queue of length size
	//Pre  - size >= 0
	//Post - The queue has been created of length size

        CND_PRECONDITION(size >= 0, "size is too small")

		//Call the initialize method of its superclass. The boolean value  passed here
		//indicates that the superclass PriorityQueue takes the responsibility to have its elements deleted
		//The destructor of SegmentMergInfo will make sure that each intstance it will be closed properly 
		//before it is deleted
 		initialize(size,true);
	}

	SegmentMergeQueue::~SegmentMergeQueue(){
	//Func - Destructor
	//       Does nothing as its parent class will clean up everything
	//Pre  - true
	//Post - true
	}

	void SegmentMergeQueue::close() {
	//Func - Closes and destroyes all SegmentMergeInfo Instances in the queue
    //Pre  - true
	//post - All SegmentMergeInfo Instances in the queue have been closed and  deleted 
	//       The queue is now empty but can still be used

		//call the clear method of the parent class PriorityQueue
        clear();
	}

	bool SegmentMergeQueue::lessThan(SegmentMergeInfo* stiA, SegmentMergeInfo* stiB) {
   //Func - Overloaded method that implements the lessThan operator for the parent class
   //       This method is used by the parent class Priority queue to reorder its internal
   //       data structures. This implementation check if stiA is less than the current term of stiB.
   //Pre  - stiA != NULL
   //		stiB != NULL
   //Post - true is returned if stiA < stiB otherwise false

        CND_PRECONDITION(stiA != NULL, "stiA is NULL")
        CND_PRECONDITION(stiB != NULL, "stiB is NULL")

		//Compare the two terms 
		int_t comparison = stiA->term->compareTo(*stiB->term);
		//Check if they match
		if (comparison == 0){
			//If the match check if the base of stiA is smaller than the base of stiB
			//Note that different bases means that the terms of stiA an stiB ly in different segments
			return stiA->base < stiB->base; 
            }
		else{
			//Terms didn't match so return the difference in positions
			return comparison < 0;
            }
	}

}}
