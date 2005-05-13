#include "CLucene/StdHeader.h"
#include "HitQueue.h"

#include "CLucene/util/PriorityQueue.h"
#include "ScoreDoc.h"

namespace lucene{ namespace search {
	

	HitQueue::HitQueue(const int_t size) {
		initialize(size,true);
	}
	HitQueue::~HitQueue(){
	}

	bool HitQueue::lessThan(ScoreDoc* hitA, ScoreDoc* hitB) {
		//ScoreDoc* hitA = (ScoreDoc*)a;
		//ScoreDoc* hitB = (ScoreDoc*)b;
		if (hitA->score == hitB->score)
			return hitA->doc > hitB->doc; 
		else
			return hitA->score < hitB->score;
	}
}}
