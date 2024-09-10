#include <iostream>
#include <cstring>
#include <string>
#include <sys/time.h>
#include <functional>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>
#include <vector>

class PerfEvent {
   struct perf_event_attr pe;
   int fd;
public:
   PerfEvent(uint64_t type, uint64_t event) {
      memset(&pe, 0, sizeof(struct perf_event_attr));
      pe.type = type;
      pe.size = sizeof(struct perf_event_attr);
      pe.config = event;
      pe.disabled = true;
      pe.exclude_kernel = true;
      pe.exclude_hv = true;
      fd = syscall( __NR_perf_event_open, &pe, 0, -1, -1, 0);
      if (fd < 0)
         fprintf(stderr, "Error opening leader %llx\n", pe.config);
      ioctl(fd, PERF_EVENT_IOC_RESET, 0);
      ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);
   }
   ~PerfEvent() {
      close(fd);
   }
   uint64_t readCounter() {
      uint64_t count;
      if (read(fd, &count, sizeof(uint64_t))!=sizeof(uint64_t))
         fprintf(stderr, "Error reading counter\n");
      return count;
   }
};

class PerfEventMT {
   std::vector<struct perf_event_attr> pes;
   std::vector<int> fd;
   unsigned cpucount;

public:
   PerfEventMT(uint64_t type, uint64_t event) {
      cpucount=sysconf(_SC_NPROCESSORS_ONLN);
      pes.reserve(cpucount);
      fd.reserve(cpucount);

      for (unsigned i=0; i<cpucount; i++) {
         struct perf_event_attr& pe=pes[i];
         memset(&pe, 0, sizeof(struct perf_event_attr));
         pe.type = type;
         pe.size = sizeof(struct perf_event_attr);
         pe.config = event;
         pe.disabled = true;
         pe.exclude_kernel = true;
         pe.exclude_hv = true;

         fd[i] = syscall( __NR_perf_event_open, &pe, -1, i, -1, 0);

         if (fd[i] < 0) {
            fprintf(stderr, "Error opening leader %llx\n", pe.config);
         }
      }

      for (unsigned i=0; i<cpucount; i++) {
         ioctl(fd[i], PERF_EVENT_IOC_RESET, 0);
         ioctl(fd[i], PERF_EVENT_IOC_ENABLE, 0);
      }
   }

   ~PerfEventMT() {
      for (unsigned i=0; i<cpucount; i++)
         close(fd[i]);
   }

   uint64_t readCounter() {
      uint64_t sum=0;
      for (unsigned i=0; i<cpucount; i++) {
         uint64_t count;
         if (read(fd[i], &count, sizeof(uint64_t))!=sizeof(uint64_t))
            fprintf(stderr, "Error reading counter\n");
         sum+=count;
      }
      return sum;
   }
};


inline double gettime() {
  struct timeval now_tv;
  gettimeofday (&now_tv, NULL);
  return ((double)now_tv.tv_sec) + ((double)now_tv.tv_usec)/1000000.0;
}

size_t getCurrentRSS() {
   long rss = 0L;
   FILE* fp = NULL;
   if ( (fp = fopen( "/proc/self/statm", "r" )) == NULL )
      return (size_t)0L;/* Can't open? */
   if ( fscanf( fp, "%*s%ld", &rss ) != 1 ) {
      fclose( fp );
      return (size_t)0L;/* Can't read? */
   }
   fclose( fp );
   return (size_t)rss * (size_t)sysconf( _SC_PAGESIZE);
}

void timeAndProfile(std::string s,uint64_t n,std::function<void()> fn,bool mem=false) {
   if (getenv("WAIT")) {
      std::cout << s << " start ..." << std::endl;
      std::cin.ignore();
      double start = gettime();
      fn();
      double end = gettime();
      std::cout << s << " " << ((n/1e6)/(end-start)) << "M/s ..." << std::endl;
      std::cin.ignore();
      return;
   }

   uint64_t memStart=0;
   if (mem)
      memStart=getCurrentRSS();
   PerfEvent misses(PERF_TYPE_HARDWARE,PERF_COUNT_HW_CACHE_MISSES);
   PerfEvent instructions(PERF_TYPE_HARDWARE,PERF_COUNT_HW_INSTRUCTIONS);
   PerfEvent l1misses(PERF_TYPE_HW_CACHE,PERF_COUNT_HW_CACHE_L1D|(PERF_COUNT_HW_CACHE_OP_READ<<8)|(PERF_COUNT_HW_CACHE_RESULT_MISS<<16));
   PerfEvent bmiss(PERF_TYPE_HARDWARE,PERF_COUNT_HW_BRANCH_MISSES);
   double start = gettime();
   fn();
   double end = gettime();
   std::cout << s << " " << ((n/1e6)/(end-start)) << "M/s " << (misses.readCounter()/(float)n) << " misses " << (l1misses.readCounter()/(float)n) << " L1misses " << (instructions.readCounter()/(float)n) << " instructions " << (bmiss.readCounter()/(float)n) << " bmisses ";
   if (mem)
      std::cout << (getCurrentRSS()-memStart)/(1024.0*1024) << "MB";
   std::cout << std::endl;
}


/**
 * Also returns the results for later use.
 */
void timeAndProfileOut(std::string s, uint64_t n, std::function<void()> fn, double& elemPerSecOut, double& missesOut, double& l1missesOut, double& instructionsOut, double& bmissesOut, bool mem=false, bool dispIntermediate=false) {
   if (getenv("WAIT")) {
      std::cout << s << " start ..." << std::endl;
      std::cin.ignore();
      double start = gettime();
      fn();
      double end = gettime();
      std::cout << s << " " << ((n/1e6)/(end-start)) << "M/s ..." << std::endl;
      std::cin.ignore();
      return;
   }

   uint64_t memStart=0;
   if (mem)
      memStart=getCurrentRSS();
   PerfEvent misses(PERF_TYPE_HARDWARE,PERF_COUNT_HW_CACHE_MISSES);
   PerfEvent instructions(PERF_TYPE_HARDWARE,PERF_COUNT_HW_INSTRUCTIONS);
   PerfEvent l1misses(PERF_TYPE_HW_CACHE,PERF_COUNT_HW_CACHE_L1D|(PERF_COUNT_HW_CACHE_OP_READ<<8)|(PERF_COUNT_HW_CACHE_RESULT_MISS<<16));
   PerfEvent bmiss(PERF_TYPE_HARDWARE,PERF_COUNT_HW_BRANCH_MISSES);
   double start = gettime();
   fn();
   double end = gettime();

   elemPerSecOut = ((n/1e6)/(end-start));
   missesOut = (misses.readCounter()/(double)n);
   l1missesOut = (l1misses.readCounter()/(double)n);
   instructionsOut = (instructions.readCounter()/(double)n);
   bmissesOut = (bmiss.readCounter()/(double)n);
   if (dispIntermediate) {
      std::cout << s << " " << elemPerSecOut << "M/s " << missesOut << " misses " << l1missesOut << " L1misses " << instructionsOut << " instructions " << bmissesOut << " bmisses ";
      if (mem)
        std::cout << (getCurrentRSS()-memStart)/(1024.0*1024) << "MB";
      std::cout << std::endl;
    }
}


void timeOut(std::string s, uint64_t n, std::function<void()> fn, double& elemPerSecOut, double& secOut, bool dispIntermediate=false) {
  if (getenv("WAIT")) {
     std::cout << s << " start ..." << std::endl;
     std::cin.ignore();
     double start = gettime();
     fn();
     double end = gettime();
     std::cout << s << " " << ((n/1e6)/(end-start)) << "M/s ..." << std::endl;
     std::cin.ignore();
     return;
  }

  double start = gettime();
  fn();
  double end = gettime();

  secOut = end-start;
  elemPerSecOut = ((n/1e6)/secOut);
  if (dispIntermediate) {
     std::cout << s << " " << elemPerSecOut << "M/s " << secOut << "s " << std::endl;
  }
}


void performWarmUp() {
  std::cout << "Performing CPU warm-up..." << '\n';

  for (int i = 0; i < 5000000; i++) {
    std::srand(time(NULL)*(12345+i));
    auto rnd = rand();
    //std::cout << rnd;
  }
  // std::cout << '\n';
  std::cout << "Warm-up done!" << '\n' << '\n' << '\n';
}

template<typename T>
void generateRandomInputs(std::vector<T>& vecOut, uint64_t size,
    uint64_t seed=0) {
  std::srand(time(NULL)*(12345+seed));
  for (int i=0; i<size; i++) {
    uint64_t r = rand();
    vecOut.push_back(r);
  }
}

template<typename T>
void generateAscendingInputs(std::vector<T>& vecOut, uint64_t size,
    uint64_t seed=0) {
  std::srand(time(NULL)*(12345+seed));
  uint64_t r = 0;
  for (int i=0; i<size; i++) {
    r += rand()%1000;
    vecOut.push_back(r);
  }
}

template<typename T>
void generateDescendingInputs(std::vector<T>& vecOut, uint64_t size,
    uint64_t seed=0) {
  std::srand(time(NULL)*(12345+seed));
  uint64_t r = size*1000;
  for (int i=0; i<size; i++) {
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
  for (int i=0; i<firstHalf; i++) {
    r += rand()%1000;
    vecOut.push_back(r);
  }

  std::srand(time(NULL)*(12345+seed+1));

  for (int i=firstHalf-1; i<secondHalf; i++) {
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
  for (int i=0; i<size; i++) {
    vecOut.push_back(r);
  }
}
