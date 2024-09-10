#include <iostream>
#include <vector>
#include <thread>
#include <random>


#ifndef PROFILE_HELPER
#define PROFILE_HELPER

/*
 * Instance of "size" byte record used for profiling.
 */
template<uint32_t size>
struct Record {
private:
	uint32_t id;
	uint8_t payload[size-4];

public:
	Record(uint32_t val) : id(val) {};
	Record() : id(0) {};
	bool operator<(const Record &r) const { return id < r.id; }
	bool operator<=(const Record &r) const { return id <= r.id; }
	bool operator>(const Record &r) const { return id > r.id; }
	bool operator>=(const Record &r) const { return id >= r.id; }
	bool operator==(const Record &r) const { return id == r.id; }
	bool operator!=(const Record &r) const { return id != r.id; }
};

void performWarmUp(bool out=true) {
  if (out) std::cout << "Performing CPU warm-up..." << '\n';
  std::vector<std::thread> v;

  for (uint16_t i=0; i<std::thread::hardware_concurrency(); i++) {
    v.push_back(std::thread([](){
      for (uint64_t i = 0; i < 1e5; i++) {
        std::srand(time(NULL)*(12345+i));
        auto rnd = rand();
      }
    }));
  }

  for (uint16_t i=0; i<v.size(); i++) {
      v[i].join();
  }

  // std::cout << '\n';
  if (out) std::cout << "Warm-up done!" << '\n' << '\n' << '\n';
}

template<typename T, typename Seed>
void generateRandomInputs(std::vector<T>& vecOut, uint64_t size, Seed seed) {
	std::mt19937_64 gen(seed);
  for (uint64_t i=0; i<size; i++) {
    T r(gen());
    vecOut.push_back(r);
  }
}

template<typename T, typename Seed>
void generateAscInputs(std::vector<T>& vecOut, uint64_t size, Seed seed) {
	std::mt19937_64 gen(seed);
	uint64_t r = 0;
  for (uint64_t i=0; i<size; i++) {
		r += gen()%1000;
    T t(r);
    vecOut.push_back(t);
  }
}

template<typename T, typename Seed>
void generateDescInputs(std::vector<T>& vecOut, uint64_t size, Seed seed) {
	std::mt19937_64 gen(seed);
	uint64_t r = UINT32_MAX;
  for (uint64_t i=0; i<size; i++) {
		r -= gen()%1000;
    T t(r);
    vecOut.push_back(t);
  }
}

template<typename T, typename Seed>
void generatePipeInputs(std::vector<T>& vecOut, uint64_t size, Seed seed) {
	uint64_t firstHalf = size/2;
  uint64_t secondHalf = size-firstHalf;

	generateAscInputs(vecOut, firstHalf, seed);
	generateDescInputs(vecOut, secondHalf, seed);
}

template<typename T, typename Seed>
void generateEvenOddInputs(std::vector<T>& vecOut, uint64_t size, Seed seed) {
	std::mt19937_64 gen(seed);
  for (uint64_t i=0; i<size; i++) {
		uint64_t r = gen();
		if (i%2==0) r &= 0;
		else r |= 1;
    T t(r);
    vecOut.push_back(t);
  }
}

template<typename T, typename Seed>
void generateAllEqualInputs(std::vector<T>& vecOut, uint64_t size, Seed seed) {
	std::mt19937_64 gen(seed);
  uint64_t r = gen();
  for (uint64_t i=0; i<size; i++) {
    vecOut.push_back(r);
  }
}

/*template<typename T>
void generateAscendingInputs(std::vector<T>& vecOut, uint64_t size,
    uint64_t seed=0) {
  std::srand(time(NULL)*(12345+seed));
  uint64_t r = 0;
  for (uint64_t i=0; i<size; i++) {
    r += rand()%1000;
    vecOut.push_back(r);
  }
}

template<typename T>
void generateDescendingInputs(std::vector<T>& vecOut, uint64_t size,
    uint64_t seed=0) {
  std::srand(time(NULL)*(12345+seed));
  uint64_t r = size*1000;
  for (uint64_t i=0; i<size; i++) {
    r -= rand()%1000;
    vecOut.push_back(r);
  }
}

template<typename T>
void generatePipeOrganInputs(std::vector<T>& vecOut, uint64_t size,
    uint64_t seed=0) {
  std::srand(time(NULL)*(12345+seed));

  uint64_t firstHalf = size/2;
  uint64_t secondHalf = size-firstHalf;

  uint64_t r = 0;
  for (uint64_t i=0; i<firstHalf; i++) {
    r += rand()%1000;
    vecOut.push_back(r);
  }

  std::srand(time(NULL)*(12345+seed+1));

  for (uint64_t i=firstHalf-1; i<secondHalf; i++) {
    r -= rand()%1000;
    vecOut.push_back(r);
  }
}

void generateEvenOddInputs() {

}

void generateReverseEvenOddInputs() {

}

void generatePushFrontInputs() {

}

void generatePushMiddleInputs() {

}

template<typename T>
void generateAllEqualInputs(std::vector<T>& vecOut, uint64_t size,
    uint64_t seed=0) {
  std::srand(time(NULL)*12345+seed);
  uint64_t r = rand();
  for (uint64_t i=0; i<size; i++) {
    vecOut.push_back(r);
  }
}*/
#endif
