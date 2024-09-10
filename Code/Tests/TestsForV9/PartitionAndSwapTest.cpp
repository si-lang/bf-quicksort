#include "../../QuickSort/SingleThreadedQuicksort/v9/partition.cpp"
#include "../../QuickSort/SingleThreadedQuicksort/v9/median.cpp"

#include <iostream> // For cout
#include <ctime> // For time() in srand()
#include <functional> // For function type
#include <vector>


#ifndef PRINT_INPUT
#define PRINT_INPUT false
#endif

#ifndef PRINT_OUTPUT
#define PRINT_OUTPUT false
#endif


void generateRandomParInputs(std::vector<uint64_t>& vecOut, uint64_t size, uint64_t maxElem) {
  for (int i=0; i<size; i++) {
    uint64_t r = rand()%maxElem;
    vecOut.push_back(r);
  }
}

void generateSpecificParInputs(std::vector<uint64_t>& vecOut, uint64_t& size) {
	size = 101;
	uint64_t arr[size] = {0, 2, 0, 1, 1, 2, 1, 0, 1, 2, 0, 2, 1, 1, 0, 1, 2, 1, 0, 1, 2, 1, 2, 1, 0, 1, 2, 2, 0, 0, 1, 0, 1, 1, 2, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 2, 0, 2, 2, 0, 0, 1, 1, 2, 0, 2, 1, 0, 2, 2, 1, 1, 0, 1, 2, 0, 2, 0, 2, 1, 2, 1, 1, 0, 0, 0, 0, 0, 0, 2, 1, 0, 0, 0, 2, 0, 2, 2, 1, 2, 1, 2, 2, 0, 1, 1, 1, 1, 1};
	for (int i=0; i < size; i++) {
		vecOut.push_back(arr[i]);
	}
}

void generateSpecificParInputsTwo(std::vector<uint64_t>& vecOut, uint64_t& size) {
	size = 11;
	uint64_t arr[size] = {0, 1, 1, 5, 1, 8, 6, 5, 0, 5, 0};
	for (int i=0; i < size; i++) {
		vecOut.push_back(arr[i]);
	}
}

std::vector<uint64_t> v;
bool performPartTestIteration(uint64_t noOfElem, uint64_t maxElem,
		uint64_t iteration) {
	typedef typename std::vector<uint64_t>::iterator iter;
  uint64_t size = noOfElem;
  v.clear();
  long long int lCnt = 0;

  generateRandomParInputs(v, size, maxElem);
  /*if (iteration==0) {
  	generateSpecificParInputs(v, size);
  } else {
  	generateSpecificParInputsTwo(v, size);
  }*/
  //generateSpecificParInputs(v, size);
  std::vector<uint64_t> vInput = v;
  iter pivotIter = myMedian::medianOfThree(v.begin(), v.end());
	//iter pivotIter = std::find(v.begin(), v.end(), 215);
  uint64_t pivot = *pivotIter;

	if (PRINT_INPUT) {
		std::cout << "Input:" << '\n';
		for (int i = 0; i < size; i++) {
			std::cout << "[" << i << "]: " << v[i] << '\n';
		}
		std::cout << "With pivot: " << pivot << '\n';
		std::cout << '\n' << "------------------------------" << '\n';
	}

  lCnt = myPartition::partitionDynQsBRANCHES(v.begin(), v.end(), pivotIter, [](uint64_t val1, uint64_t val2){
    return (val1 < val2);
  }, 512);

	// uint64_t pivotPos = pivotIter-v.begin();
	// uint64_t pivLength = 0;
	// std::iter_swap(pivotIter, v.end()-1);
	// lCnt = myPartition::partitionDynQsDC(v.begin(), v.end()-1, v.end()-1, [](uint64_t val1, uint64_t val2){
  //  return (val1 < val2);
  // }, 512, pivLength);
	// std::iter_swap(v.begin()+lCnt, v.end()-1);
	// pivLength+=1; // !!!!!!
	//
	// /*int pivLength = 0;
	// lCnt = myPartition::hoare_block_partition_unroll_loop(v.begin(), v.end(), pivotIter, [](uint64_t val1, uint64_t val2){
  //   return (val1 < val2);
  // }, pivLength)-v.begin();*/
	//
	// std::cout << "Returned pivot position " << lCnt << " and pivot length " << pivLength << std::endl;

  bool result = true;
  if (PRINT_OUTPUT) std::cout << "Partitioned output:" << '\n';
  for (uint64_t i = 0; i < size; i++) {
    if (PRINT_OUTPUT) std::cout << "[" << i << "]: " << v[i];
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

	if (PRINT_OUTPUT && !isEmpty) {
		std::cout << "Missing: ";
		for (uint64_t e=0; e<vInput.size(); e++) {
			std::cout << vInput[e] << ", ";
		}
		std::cout << std::endl;
	}


  std::string resultStr = "";
  if (result) {
    resultStr = "valid";
  } else {
    resultStr = "invalid";
  }


  std::cout << "The result is " << resultStr;
  if (!isEmpty) {
  	std::cout << " and the elements differ from the input";
  }
  std::cout << "!" << '\n';
  return result;
}

int main(int argc, char** argv) {
  uint64_t noOfTests = 1000;
  uint64_t noOfPasTests = 0;
  uint64_t arrSize = 0;
  uint64_t maxElem = 0;

  for (int i=0; i<noOfTests; i++) {
    std::srand(i+time(NULL));
    arrSize = (std::rand()%100000)+2;
    maxElem = (std::rand()%1000)+1;
    noOfPasTests += performPartTestIteration(arrSize, maxElem, i);
		if (noOfPasTests<i) break;
    std::cout << '\n' << "------------------------------" << '\n' << "------------------------------" << '\n';
  }

  std::cout << "\n\n" << "Overall passed " << noOfPasTests << " of " << noOfTests << " tests." << '\n';
}
