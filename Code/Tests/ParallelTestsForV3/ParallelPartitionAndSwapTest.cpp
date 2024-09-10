#include "../../QuickSort/SingleThreadedQuicksort/v9/partition.cpp"
#include "../../QuickSort/SingleThreadedQuicksort/v9/median.cpp"
//#include "../../QuickSort/ParallelQuicksort/v3/parallelPartition.cpp"
#include "../../QuickSort/ParallelQuicksort/v3/parallelPartitionDyn.cpp"

#include <iostream> // For cout
#include <ctime> // For time() in srand()
#include <functional> // For function type

#ifndef PRINT_INPUT
#define PRINT_INPUT false
#endif

#ifndef PRINT_OUTPUT
#define PRINT_OUTPUT false
#endif

#ifndef BREAK_AT_ERROR
#define BREAK_AT_ERROR true
#endif


typedef uint64_t T;
void generateRandomParInputs(std::vector<T>& vecOut, uint64_t size, uint64_t maxElem) {
  for (int i=0; i<size; i++) {
    T r = rand()%maxElem;
    vecOut.push_back(r);
  }
}

void generateSpecificParInputs(std::vector<T>& vecOut, uint64_t& size) {
	const uint64_t _size = 208;
	size = _size;
	T arr[_size] = {18, 48, 13, 31, 6, 5, 10, 11, 2, 14, 9, 46, 3, 16, 44, 6, 43, 47, 32, 19, 47, 22, 36, 45, 20, 12, 19, 1, 33, 17, 13, 28, 42, 2, 7, 24, 7, 45, 35, 37, 35, 21, 8, 38, 13, 28, 21, 32, 23, 1, 0, 47, 4, 12, 16, 19, 0, 35, 49, 33, 29, 38, 37, 47, 40, 20, 19, 23, 14, 30, 8, 49, 27, 44, 12, 40, 20, 33, 21, 20, 34, 49, 15, 34, 37, 31, 29, 37, 15, 2,46, 20, 40, 32, 15, 4, 0, 10, 27, 42, 40, 12, 40, 44, 30, 28, 32, 1, 37, 29, 21, 47, 2, 36, 5, 39, 15, 11, 24, 6, 13, 19, 2, 2, 27, 17, 34, 27, 27, 10, 46, 44, 22, 10, 36,2, 38, 44, 3, 51, 22, 24, 46, 0, 36, 0, 40, 28, 11, 40, 34, 0, 35, 13, 30, 10, 41, 13, 14, 34, 23, 8, 2, 21, 18, 14, 23, 32, 6, 3, 31, 4, 27, 1, 5, 12, 29, 45, 16, 16, 33,26, 17, 17, 39, 47, 3, 46, 8, 17, 4, 7, 1, 6, 28, 47, 20, 28, 27, 2, 31, 34, 7, 34, 12, 12, 46, 51};
	for (int i=0; i < size; i++) {
		vecOut.push_back(arr[i]);
	}
}

void generateSpecificParInputsTwo(std::vector<uint64_t>& vecOut, uint64_t& size) {
	size = 40;
	T arr[size];/*= {676, 247, 598, 245, 501, 503, 554, 367, 705, 657,
			704, 638, 344, 265, 207, 736, 469, 57, 510, 721, 588, 338, 480, 394, 232,
			701, 421, 237, 442, 670, 212, 324, 124, 16, 569, 625, 519, 329, 198, 430};*/
	for (int i=0; i < size; i++) {
		vecOut.push_back(arr[i]);
	}
}

std::vector<T> v;
bool performPartTestIteration(uint64_t noOfElem, uint64_t maxElem,
		uint64_t iteration) {
	typedef typename std::vector<T>::iterator iter;
  uint64_t size = noOfElem;
  long long int lCnt = 0;

  v = std::vector<T>();
  generateRandomParInputs(v, size, maxElem);
  //if (iteration==0) {
  	//generateSpecificParInputs(v, size);
  //} else {
  //	generateSpecificParInputsTwo(v, size);
  //}
  //generateSpecificParInputs(v, size);
  std::vector<T> vInput = v;
  std::vector<T> vInput2 = v;
  iter pivotIter = median::median_of_5(v.begin(), v.end(), [](T val1, T val2) {
    return val1<val2;
  });
  T pivot = *pivotIter;

  if (PRINT_INPUT) {
		std::cout << "Input:" << '\n';
		/*for (int i = 0; i < size; i++) {
			std::cout << "[" << i << "]: " << pv[i] << '\n';
		}*/
		// Inline input
		std::cout << "{" << v[0];
		for (int i=1; i<size; i++) {
			std::cout << ", " << v[i];
		}
		std::cout << "};\n";
		std::cout << "With pivot: " << pivot << '\n';
		std::cout << '\n' << "------------------------------" << '\n';
	}

	uint16_t numThreads = 4;
	//uint64_t elemPerThread = (v.end()-v.begin()-1)/numThreads;
  //uint64_t elemPerSide = std::min((uint64_t)30000, (elemPerThread/2)/32); // TODO: Min: 16?
  iter mid = myParallelPartition::parallelPartition(v.begin(), v.end(), pivotIter, [](T val1, T val2) {
      return val1<val2;
    }, numThreads);
  lCnt = mid-v.begin();

  /*lCnt = myPartition::qsBlockPartition(v.begin(), v.end(), pivotIter, [](T val1, T val2){
    return (val1 < val2);
  });*/

  /*lCnt = myQuickSort::parallelQuickSort(v.begin(), v.end(), [](T val1, T val2) {
    return val1<val2;
  });*/


  bool result = true;
  if (PRINT_OUTPUT) {
    std::cout << "Partitioned output:" << '\n';
  }
  for (uint64_t i = 0; i < size; i++) {
    if (PRINT_OUTPUT) {
      std::cout << "[" << i << "]: " << v[i];
    }
    bool compRes = 0;
    if (i < lCnt) {
    	compRes = (v[i]<pivot);
      result = result && compRes;
    } else {
    	compRes = (v[i]>=pivot);
      result = result && compRes;
    }

    if (PRINT_OUTPUT) {
      if (!compRes) {
      	std::cout << " <- Wrong!";
      }
      std::cout << '\n';
    }

    iter found = std::find(vInput.begin(), vInput.end(), v[i]);
    if (found != vInput.end()) {
    	vInput.erase(found);
    }
  }
  bool isEmpty = (vInput.size() == 0);
  result = result && isEmpty;


  std::string resultStr = "";
  if (result) {
    resultStr = "valid";
  } else {
    resultStr = "invalid";
  }


  std::cout << "The result is " << resultStr;
	if (!isEmpty) {
		std::cout << " and the elements differ from the input:" << "\n";
		std::cout << "Missing: " << "\n";
		for (auto e : vInput) {
			std::cout << e << "\n";
		}
		std::cout << "\n";
		std::cout << "Additional:" << "\n";
		for (uint64_t i=0; i<size; i++) {
			iter found = std::find(v.begin(), v.end(), vInput2[i]);
			if (found != v.end()) {
				v.erase(found);
			}
		}
		for (auto e : v) {
			std::cout << e << "\n";
		}
	} else {
		std::cout << "!" << '\n';
	}
  return result;
}

int main(int argc, char** argv) {
	const uint64_t minSize = 100;
	const uint64_t maxSize = 10000;
	const uint64_t noOfTests = 1000;
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
			arrSize = (std::rand()%(size/10))+size; // Adds a bit of oddness to the size
			maxElem = (std::rand()%1000)+1;
			result = performPartTestIteration(arrSize, maxElem, i);
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
