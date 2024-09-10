#include <string>
#include <vector>
#include <algorithm> // Needed for median

#include "../TimeAndProfile/profile.hpp"

#include "../EdelkampWeiss/median.h"

#include "quicksort.cpp"
#include "stdQuicksort.cpp"
#include <stdlib.h>
#include <time.h>


int cmpint(const void* v1, const void* v2) {
	if (*(const int*)v1 < *(const int*)v2) return -1;
	else return 1;/*
	if (*(const int*)v1 == *(const int*)v2) return 0;
	if (*(const int*)v1 > *(const int*)v2) return 1;*/
}

int cmpintArg(const void* v1, const void* v2, void* args) {
	if (*(const int*)v1 < *(const int*)v2) return -1;
	else return 1;/*
	if (*(const int*)v1 == *(const int*)v2) return 0;
	if (*(const int*)v1 > *(const int*)v2) return 1;*/
}

int main(int argc, char** argv) {
	assert(argc>2); // 1: size; 2: repetitions
	typedef int T;
	typedef typename std::vector<T>::iterator iter;

	int size = atoi(argv[1]);
	uint64_t repetitions = atoi(argv[2]);
	std::vector<T> v;
	T* arr = new T[size];
	T* worArr = new T[size];

	//PerfEvents e;
	std::cout << "overallTime, pivotTime, partitionTime" << std::endl;
	for (uint32_t i=0; i<repetitions; i++) {
		srand(time(NULL)+i*123);
		for (uint64_t i=0; i<size; i++) {
			arr[i] = rand()+1;
		}
		memcpy(worArr, arr, size*sizeof(T));

		void* test = malloc(1);
		/*e.timeAndProfile("Perf", size, [&](){
			//naiveSort::quickSort(worArr, 0, size-1);
			_quicksort(worArr, size, sizeof(T), &cmpintArg, test);
			//std::qsort(worArr, size, sizeof(T), &cmpint);
			//std::sort(worArr, worArr+size);
		}, 1);*/
		_quicksort(worArr, size, sizeof(T), &cmpintArg, test);

		//memcpy(sanity, arr, size*sizeof(T));
		std::sort(arr, arr+size);
		for (uint32_t j=0; j<size; j++) {
			//std::cout << "my[" << j << "]: " << worArr[j] << " == " << "sanity[" << j << "]: " << sanity[j] << std::endl;
			assert(worArr[j]==arr[j]);
		}
	}
}
