#include "CLucene/StdHeader.h"
#include "Arrays.h"

#include <algorithm>
#include <functional>

using namespace std;
namespace lucene{ namespace util{
	
	//static 
	/*void Arrays::arraycopy (const char_t** src, const int_t srcStart, char_t** dest, const int_t destStart, const int_t length){
		for ( int_t i=0;i<length;i++ ){
			dest[destStart+i] = stringDuplicate( src[srcStart+i] );
		}
	}*/
	void Arrays::arraycopy (const char_t* src, const int_t srcStart, char_t* dest, const int_t destStart, const int_t length){
		for ( int_t i=0;i<length;i++ ){
			dest[destStart+i] = src[srcStart+i];
		}
	}
	void Arrays::arraycopy (const l_byte_t* src, const int_t srcStart, l_byte_t* dest, const int_t destStart, const int_t length){
		for ( int_t i=0;i<length;i++ ){
			dest[destStart+i] = src[srcStart+i];
		}
	}

	//static 
	/*void Arrays::mergeSort(char_t** src, char_t** dest, const int_t low, const int_t high) {
		int_t length = high - low;

		// Insertion sort on smallest arrays
		if (length < 7) {
			for (int_t i=low; i<high; i++)
				for (int_t j=i; j>low && stringCompare(dest[j-1],dest[j])>0; j--){
					char_t* tmp = dest[j];
					dest[j] = dest[j-1];
					dest[j-1] = dest[j];
				}
			return;
		}

		// Recursively sort halves of dest into src
		int_t mid = (low + high)/2;
		mergeSort(dest, src, low, mid);
		mergeSort(dest, src, mid, high);

		// If list is already sorted, just copy from src to dest.  This is an
		// optimization that results in faster sorts for nearly ordered lists.
		if ( stringCompare(src[mid-1], src[mid]) <= 0) {
			arraycopy((const char_t**)src, low, dest, low, length);
			return;
		}

		// Merge sorted halves (now in src) into dest
		for(int_t i = low, p = low, q = mid; i < high; i++) {
			if (q>=high || p<mid && stringCompare(src[p],src[q])<=0)
				dest[i] = src[p++];
			else
				dest[i] = src[q++];
		}
	}*/

}}
