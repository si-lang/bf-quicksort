#include <iostream>
#include <thread> // Used for threads

#include "../../SingleThreadedQuicksort/v8/partition.cpp"

/*
--------------------------------------------
---------- Starting Profilingsuite ---------
--------------------------------------------

------ Average of 20 for 10^4 elements ------
StandardImplementation 0M/s 0s
SuperScalarSampleSort 0M/s 0s
MyBlockImplementation 404.25M/s 2.73347e-05s
MyParallelBlockImplementation 83.0042M/s 0.000162053s


------ Average of 20 for 10^5 elements ------
StandardImplementation 0M/s 0s
SuperScalarSampleSort 0M/s 0s
MyBlockImplementation 516.375M/s 0.00020721s
MyParallelBlockImplementation 525.662M/s 0.000208235s


------ Average of 20 for 10^6 elements ------
StandardImplementation 0M/s 0s
SuperScalarSampleSort 0M/s 0s
MyBlockImplementation 881.925M/s 0.00113639s
MyParallelBlockImplementation 1401.64M/s 0.000731337s


------ Average of 20 for 10^7 elements ------
StandardImplementation 0M/s 0s
SuperScalarSampleSort 0M/s 0s
MyBlockImplementation 970.78M/s 0.0105937s
MyParallelBlockImplementation 1752.05M/s 0.00584089s


------ Average of 20 for 10^8 elements ------
StandardImplementation 0M/s 0s
SuperScalarSampleSort 0M/s 0s
MyBlockImplementation 924.512M/s 0.110139s
MyParallelBlockImplementation 1685.73M/s 0.0600617s
*/



#ifndef PARALLEL_MAX_PARTITION_CPP
#define PARALLEL_MAX_PARTITION_CPP


namespace myPartition {
  /*
   * Maximal speedup possible.
   */
  template<typename iter, typename Comp>
  inline iter parallelMaxPartition(const iter begin, const iter end, const iter pivot_iter, const Comp comp, uint32_t numThreads) {
    typedef typename std::iterator_traits<iter>::value_type T;
    std::thread threads[numThreads];

    uint64_t blockSize = (end-begin)/numThreads;

    // Partition naively in parallel
    for(int i=0; i<numThreads; i++) {
      threads[i] = std::thread(partition<iter, Comp>,
          begin+i*blockSize,
          begin+(i+1)*blockSize-1,
          pivot_iter,
          comp);
    }


    for(int i=0; i<numThreads; i++) {
    	threads[i].join();
    }
    // End partition in parallel

    return begin;
  }

  template<typename iter, typename UnaryComp>
  inline iter stdMaxPartition(const iter begin, const iter end, const UnaryComp comp, uint32_t numThreads) {
    typedef typename std::iterator_traits<iter>::value_type T;
    std::thread threads[numThreads];

    uint64_t blockSize = (end-begin)/numThreads;

    // Partition naively in parallel
    for(int i=0; i<numThreads; i++) {
      threads[i] = std::thread(std::partition<iter, UnaryComp>,
          begin+i*blockSize,
          begin+(i+1)*blockSize-1,
          comp);
    }


    for(int i=0; i<numThreads; i++) {
      threads[i].join();
    }
    // End partition in parallel

    return begin;
  }
}

#endif
