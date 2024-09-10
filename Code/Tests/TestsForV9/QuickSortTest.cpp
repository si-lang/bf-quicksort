#include "../../Profiling/profile.cpp"

#include "../../QuickSort/SingleThreadedQuicksort/v9/quickSort.cpp"
//#include "../../QuickSort/ParallelQuicksort/v3/parallelQuicksort.cpp"

#include <iostream> // For cout
#include <ctime> // For time() in srand()
#include <functional> // For function type

#ifndef PRINT_INPUT
#define PRINT_INPUT false
#endif

#ifndef PRINT_OUTPUT
#define PRINT_OUTPUT false
#endif


void generateRandomQSInputs(std::vector<uint64_t>& vecOut, const uint64_t size, const uint64_t maxElem) {
  for (int i=0; i<size; i++) {
    uint64_t r = rand()%maxElem;
    vecOut.push_back(r);
  }
}

void generateSpecificQSInputs(std::vector<uint64_t>& vecOut, uint64_t& size) {
  const uint64_t _size = 29;
	size = _size;
	uint64_t arr[_size] = {0, 0, 4, 0, 9, 4, 8, 7, 8, 9, 2, 3, 3, 0, 2, 4, 5, 1, 7, 2, 5, 1, 7, 5, 4, 2, 3, 9, 0};
	for (int i=0; i < size; i++) {
		vecOut.push_back(arr[i]);
	}
}

void generateSpecificQSInputsTwo(std::vector<uint64_t>& vecOut, uint64_t& size) {
	size = 21;
	uint64_t arr[size];// = {1, 49, 268, 207, 45, 30, 99, 116, 54, 271, 274, 157, 121, 203, 276, 263, 49, 64, 292, 116, 66};
	for (int i=0; i < size; i++) {
		vecOut.push_back(arr[i]);
	}
}

std::vector<uint64_t> v;
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
  std::vector<uint64_t> stdV = v;

  if (PRINT_INPUT) {
    std::cout << "Input:" << '\n';
    for (int i = 0; i < size; i++) {
      std::cout << "[" << i << "]: " << v[i] << '\n';
    }
    std::cout << '\n' << "------------------------------" << '\n';
  }

  std::sort(stdV.begin(), stdV.end(), [](uint64_t val1, uint64_t val2) {
    return val1<val2;
  });
  myQuickSort::quickSortDynBRANCHES(v.begin(), v.end(), [](uint64_t val1, uint64_t val2) {
    return val1<val2;
  });

	// myQuickSort::quickSortDynPivSelT(v.begin(), v.end(), [](uint64_t val1, uint64_t val2) {
  // 	return val1<val2;
  // }, 128);

	// typedef typename std::vector<uint64_t>::iterator iter;
	// std::function<iter(iter, iter)> pivSel = [](iter begin, iter end) { return median::median_of_5(begin, end, [](uint64_t v1, uint64_t v2){ return v1 < v2; }); };
	// myQuickSort::quickSortDynPivSel(v.begin(), v.end(), [](uint64_t v1, uint64_t v2){
	// 	return v1 < v2;
	// }, pivSel);

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
  uint64_t noOfTests = 100;
  uint64_t noOfPasTests = 0;
  uint64_t arrSize = 0;
  uint64_t maxElem = 0;

  for (int i=0; i<noOfTests; i++) {
    std::srand(i+time(0)*123);
    arrSize = (std::rand()%1000000)+2;
    maxElem = (std::rand()%10000)+1;
    noOfPasTests += performTestIteration(arrSize, maxElem, i);
    std::cout << '\n' << "------------------------------" << '\n' << "------------------------------" << '\n';
  }

  std::cout << "\n\n" << "Overall passed " << noOfPasTests << " of " << noOfTests << " tests." << '\n';
}
