#include "../../Profiling/profile.cpp"
#include "../../QuickSort/ParallelQuicksort/v3/parallelQuicksortDyn.cpp"
//#include "../../QuickSort/ParallelQuicksort/v3/parallelQuicksortDynNoTP.cpp"

#include <iostream> // For cout
#include <ctime> // For time() in srand()
#include <functional> // For function type
//#include "../../Profiling/libstdc++/parallel/balanced_quicksort.h"

#ifndef PRINT_INPUT
#define PRINT_INPUT false
#endif

#ifndef PRINT_OUTPUT
#define PRINT_OUTPUT false
#endif

#ifndef BREAK_AT_ERROR
#define BREAK_AT_ERROR true
#endif


typedef uint8_t T;
void generateRandomQSInputs(std::vector<T>& vecOut, const uint64_t size, const uint64_t maxElem) {
  for (int i=0; i<size; i++) {
    uint64_t r = rand()%maxElem;
    vecOut.push_back(r);
  }
}

void generateSpecificQSInputs(std::vector<T>& vecOut, uint64_t& size) {
  const uint64_t _size = 100;
	size = _size;
	uint64_t arr[_size] = {232, 141, 87, 231, 213, 39, 144, 307, 214, 241, 33, 235, 8, 143, 7, 62, 200, 165, 221, 181, 3, 238, 202, 245, 128, 226, 277, 51, 229, 191, 118, 139, 272, 145, 310, 102, 185, 71, 87, 76, 312, 60, 251, 260, 143, 198, 322, 21, 41, 160, 142, 307, 76, 22, 229, 144, 248, 123, 196, 155, 255, 254, 234, 204, 77, 161, 247, 202, 232, 274, 278, 162, 11, 147, 99, 154, 22, 39, 115, 3, 139, 258, 311, 215, 280, 157, 300, 145, 221, 113, 240, 153, 44, 91, 297, 121, 252, 221, 0, 162};
	for (int i=0; i < size; i++) {
		vecOut.push_back(arr[i]);
	}
}

void generateSpecificQSInputsTwo(std::vector<T>& vecOut, uint64_t& size) {
	size = 21;
	uint64_t arr[size];// = {1, 49, 268, 207, 45, 30, 99, 116, 54, 271, 274, 157, 121, 203, 276, 263, 49, 64, 292, 116, 66};
	for (int i=0; i < size; i++) {
		vecOut.push_back(arr[i]);
	}
}


std::vector<T> v; // For global debugging
bool performTestIteration(uint64_t noOfElem, uint64_t maxElem,
		uint64_t iteration) {
  uint64_t size = noOfElem;
	v.clear();
	generateRandomQSInputs(v, size, maxElem);
  //if (iteration==0) {
  	//generateSpecificQSInputs(v, size);
  //} else {
  	//generateSpecificQSInputsTwo(v, size);
  //}
  //generateSpecificQSInputs(v, size);

  //generateAllEqualInputs(v, size);
  std::vector<T> stdV = v;

  if (PRINT_INPUT) {
		/*for (int i = 0; i < size; i++) {
			std::cout << "[" << i << "]: " << pv[i] << '\n';
		}*/
		// Inline input
		std::cout << "{" << v[0];
		for (int i=1; i<size; i++) {
			std::cout << ", " << v[i];
		}
		std::cout << "};\n";
    std::cout << '\n' << "------------------------------" << '\n';
  }

  std::sort(stdV.begin(), stdV.end(), [](T val1, T val2) {
    return val1<val2;
  });
  myParallelQuickSort::parallelQuickSortDebug(v.begin(), v.end(), [](T val1, T val2) {
     return val1<val2;
  }, 4);
	//__gnu_parallel_mod::__parallel_sort_qsb(v.begin(), v.end(), [](T val1, T val2){ return val1<val2; }, 2);


  bool result = true;
  if (PRINT_OUTPUT) {
    std::cout << "Sorted output:" << '\n';
  }
  for (int i = 0; i < size; i++) {
    if (PRINT_OUTPUT) {
    	std::cout << "[" << i << "]: " << v[i];
  		std::cout << "; std:" << stdV[i];
    }

		bool compRes = v[i] == stdV[i];

    if (PRINT_OUTPUT) {
  		if (!compRes) {
  			std::cout << " <- Wrong!";
  		}
		  std::cout << '\n';
    }

		result = result && compRes;
  }

  std::string resultStr = "";
  if (result) {
    resultStr = "valid";
  } else {
    resultStr = "invalid";
  }

  std::cout << "The result is " << resultStr << "!" << " (for " << size << " elements)" << '\n';
  return result;
}

int main(int argc, char** argv) {
	const uint64_t minSize = 100;
	const uint64_t maxSize = 1000000;
	const uint64_t noOfTests = 100;
	std::vector<bool> testsSucc;

	for (uint64_t size=minSize; size<=maxSize; size*=10) {
		std::cout << "Testing for size " << size << std::endl;
		uint64_t noOfPasTests = 0;
		uint64_t arrSize = 0;
		uint64_t maxElem = 0;
		bool result = false;

		uint64_t i=0;
		for (; i<noOfTests; i++) {
			std::srand((i*10000)+time(NULL));
			arrSize = (std::rand()%1)+size;
			maxElem = (std::rand()%1000)+1;
			result = performTestIteration(arrSize, maxElem, i);
			noOfPasTests += result;
			std::cout << '\n' << "------------------------------" << '\n' << "------------------------------" << '\n';
			if (BREAK_AT_ERROR && !result) {
				i++;
				break;
			}
		}

		std::cout << "\n\n" << "Overall passed " << noOfPasTests << " of " << i << " tests for " << size << " elements." << "\n\n\n";
		testsSucc.push_back(result);
	}

	uint64_t i = 0;
	std::cout << "Summary: " << "\n";
	for (uint64_t size=minSize; size<=maxSize; size*=10) {
		std::cout << "[" << size << "]: ";
		if (testsSucc[i]) std::cout << "true";
		else std::cout << "false";
		std::cout << std::endl; 
		i++;
	}
}
