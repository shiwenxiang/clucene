#include <string>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <time.h>
#include "CLucene/util/PriorityQueue.h"
#include "CLucene/util/Misc.h"

using namespace std;
namespace lucene{ namespace util{ 
	class int_tegerQueue: public lucene::util::PriorityQueue<int_t>{
	public:
		int_tegerQueue(int_t count)
		{ 
			initialize(count,false);
		}
	protected:
		bool lessThan(int_t a, int_t b) {
			cout << "CHECK THIS... what is the void?"<<endl;
			return (a) < ( b);
		}
	};

	class test_PQ{
	public:
		void static test_PriorityQueue(int_t count){
			int_tegerQueue pq(count);
			srand ( (unsigned)time(NULL) );
			int_t sum = 0, sum2 = 0;
		    
			long_t start = lucene::util::Misc::currentTimeMillis();

			for (int_t i = 0; i < count; i++) {
				int_t next = -rand();
				//int_t next = (count+1)-i;
				sum += next;
				pq.put( next );
				//cout << next << endl;
			}
			cout << (lucene::util::Misc::currentTimeMillis()-start) << " milliseconds/" << count << " puts" << endl;
			start = lucene::util::Misc::currentTimeMillis();

			int_t last = -16500000;
			for (int_t i = 0; i < count; i++) {
				int_t next = (int_t)pq.pop();
				if ( next < last){
					cout << "TEST FAILED WHILE POPPING - next:" << next << " last:" << last << " i:" << i<< endl;
					return;
				}
				//cout << next << endl;
				last = next;
				sum2 += last;
			}

			cout << (lucene::util::Misc::currentTimeMillis()-start) << " milliseconds/" << count << " pops" << endl;

			if ( sum == sum )
				cout << "Test works." << endl;
			else
				cout << "Test FAILED." << endl;

		}
	};
}}
