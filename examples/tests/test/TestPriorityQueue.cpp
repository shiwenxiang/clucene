#include "stdafx.h"

namespace lucene{ namespace test{ namespace priorityQueue{
	class Integer{
	public:
		int_t value;
		Integer(int_t init):
			value(init)
		{
		}
	};

	class IntegerQueue: public lucene::util::PriorityQueue<Integer*>{
	public:
		IntegerQueue(const int_t count)
		{ 
			initialize(count,true);
		}
	protected:
		bool lessThan(Integer* a, Integer* b) {
			return (a->value) < ( b->value);
		}
	};

	void test_PriorityQueue(int_t count){
		IntegerQueue pq(count);
		srand ( (unsigned)time(NULL) );
		int_t sum = 0, sum2 = 0;
		
		long_t start = lucene::util::Misc::currentTimeMillis();

		for (int_t i = 0; i < count; i++) {
			int_t next = -rand();
			//int_t next = (count+1)-i;
			sum += next;
			pq.put( new Integer(next) );
			//_cout << next << endl;
		}
		_cout << (lucene::util::Misc::currentTimeMillis()-start) << _T(" milliseconds/") << count << _T(" puts") << endl;
		start = lucene::util::Misc::currentTimeMillis();

		int_t last = -0x7FFFFFFF;
		for (int_t i = 0; i < count; i++) {
			int_t next = *(int_t*)pq.pop();
			if ( next < last){
				_cout << _T("TEST FAILED WHILE POPPING - next:") << next << _T(" last:") << last << _T(" i:") << i<< endl;
				return;
			}
			//_cout << next << endl;
			last = next;
			sum2 += last;
		}

		_cout << (lucene::util::Misc::currentTimeMillis()-start) << _T(" milliseconds/") << count << _T(" pops") << endl;

		if ( sum == sum2 )
			_cout << _T("Test works.") << endl;
		else
			_cout << _T("Test FAILED.") << endl;

	}
}}}