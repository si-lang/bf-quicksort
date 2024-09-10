// Constraints:
//	- Minimum 16 elements (???) (Because of unrolls)
//	- Minimum elem_per_block>=PP_BLOCKSIZE elements (Because of last partitioning phase, since a whole buffer might be swapped into one block)

#include <algorithm> // Used for swap()
#include <iostream>
#include <functional> // Used for std::ref
#include <mutex> // Used std::mutex
#include <cstring> // Used for memmove

#include "../../../Profiling/EdelkampWeiss/blocked.h++"
#include "../../../Profiling/EdelkampWeiss/median.h"
#include "../../../Profiling/EdelkampWeiss/insertionsort.h"
#include "../../../Profiling/EdelkampWeiss/partition.h"


#ifndef PARALLELPARTITION_BASE_1_CPP
#define PARALLELPARTITION_BASE_1_CPP

#ifndef PP_BLOCKSIZE
#define PP_BLOCKSIZE 512
#endif

#ifndef PP_BUFFERSIZE
#define PP_BUFFERSIZE PP_BLOCKSIZE
#endif

#ifndef PP_DEBUG
#define PP_DEBUG false
#endif


#ifndef checkAndAdvanceToLeft
#define checkAndAdvanceToLeft(ri, rResult, pivot, lOut, lc) {\
	ri--;\
	rResult = !pred(pivot, *ri);\
	lOut[lc] = ri;\
	lc += rResult;\
}
#endif

#ifndef checkAndAdvanceToRight
#define checkAndAdvanceToRight(li, lResult, pivot, rOut, rc) {\
	lResult = pred(pivot, *li);\
	rOut[rc] = li;\
	rc += lResult;\
	li++;\
}
#endif


namespace myPartitionA {

	template<typename iter, typename Comp>
	struct Parallel_Partitioner {
		std::mutex work_mtx;
		iter l_next, r_next;
		uint32_t elem_per_block;
		uint32_t size;
  	iter* right_rem, *left_rem;

  	Parallel_Partitioner(uint32_t noThreads, uint32_t noElemPerBlock, iter lNext, iter rNext) {
			size = noThreads;
      right_rem = new iter[noThreads*2];
      left_rem = new iter[noThreads*2];
			elem_per_block = noElemPerBlock;
			l_next = lNext;
			r_next = rNext;
  	}

    ~Parallel_Partitioner() {
      delete [] right_rem;
      delete [] left_rem;
    }

    inline void parallel_partition(const iter& _l_begin, const iter& _l_end,
				const iter& _r_begin, const iter& _r_end, const iter& pivot_iter,
				Comp pred, const uint64_t thNo) {

			typedef typename std::iterator_traits<iter>::value_type T;
			T pivot = *pivot_iter; // Pivot might get swapped
			T temp;
			//const uint32_t size = PP_BLOCKSIZE+(l_end-l_begin);
			iter lOut[PP_BUFFERSIZE]; // Direct writeout to struct
			iter rOut[PP_BUFFERSIZE];
			int64_t lc = 0;
			int64_t rc = 0;

			// The indexes we actually want to work with
			// and which will be subject to change
			iter l_begin = _l_begin;
			iter l_end = _l_end;
			iter r_begin = _r_begin;
			iter r_end = _r_end+1;

			bool lResult = 0;
			bool rResult = 0;
			iter ri = r_begin+1;
			iter li = l_begin;
			int64_t off;

			while((li!=l_end||rc>0) && (ri!=r_end||lc>0)) {
				/*// TODO
				work_mtx.lock();
				std::cout << "Thread " << thNo << ": " 
						<< "l_range " << (_l_end-_l_begin)
						<< " r_range " << (_r_begin-r_end) << std::endl;
				work_mtx.unlock();
				// TODO*/

				while((lc<PP_BLOCKSIZE-15)&&(ri>(r_end+15))) {
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
				}
				while((lc<PP_BLOCKSIZE)&&(ri!=r_end)) {
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
				}

				while((rc<PP_BLOCKSIZE-15)&&(li<(l_end-15))) {
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
				}
				while((rc<PP_BLOCKSIZE)&&(li<l_end)) {
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
				}

				uint64_t minc = std::min(lc, rc);
				off = 0;
				if (minc > 0) {
					temp = *rOut[off];
					*rOut[off] = *lOut[off];
					off++;
					while (off<minc) {
						*lOut[off-1] = *rOut[off];
						*rOut[off] = *lOut[off];
						off++;
					}
					*lOut[off-1] = temp;
				}

				rc -= off;
				lc -= off;

        // Refill
        if (li==l_end || ri==r_end) {
          // Prepare buffer
          if (rc>0) {
            memmove(rOut, rOut+off, rc*sizeof(iter));
          } else if (lc>0) {
            memmove(lOut, lOut+off, lc*sizeof(iter));
          }
          off = 0;

          // Perform actual refill
          int32_t block_size=0;
          if (ri==r_end&&lc==0) {
            work_mtx.lock();
            block_size = r_next-l_next+1;
            if (block_size>=elem_per_block) {
              r_begin = r_next;
              r_next = r_begin-elem_per_block;
              work_mtx.unlock();
              r_end = r_begin-elem_per_block+1;
              ri = r_begin+1;
            } else if (block_size>0) {
							r_begin = r_next;
              r_next = r_begin-block_size;
              work_mtx.unlock();
              r_end = r_begin-block_size+1;
              ri = r_begin+1;
						} else {
              work_mtx.unlock();
            }
          }

          if (li==l_end&&rc==0) {
            work_mtx.lock();
            block_size = r_next-l_next+1;
            if (block_size>=elem_per_block) {
              l_begin = l_next;
              l_next = l_begin+elem_per_block;
              work_mtx.unlock();
              l_end = l_begin+elem_per_block;
              li = l_begin;
            } else if (block_size>0) {
              l_begin = l_next;
              l_next = l_begin+block_size;
              work_mtx.unlock();
              l_end = l_begin+block_size;
              li = l_begin;
            } else {
              work_mtx.unlock();
            }
          }
        }
			}

			// Store not neutralized block
			if (ri!=r_end||lc>0) {
        left_rem[thNo*2] = l_end;
        left_rem[thNo*2+1] = l_end;
        right_rem[thNo*2] = r_end;
        right_rem[thNo*2+1] = r_begin+1;
			} else if (li!=l_end||rc>0) {
        left_rem[thNo*2] = l_begin;
        left_rem[thNo*2+1] = l_end;
        right_rem[thNo*2] = r_begin+1;
        right_rem[thNo*2+1] = r_begin+1;
			} else {
        left_rem[thNo*2] = l_end;
        left_rem[thNo*2+1] = l_end;
        right_rem[thNo*2] = r_begin+1;
        right_rem[thNo*2+1] = r_begin+1;
      }
		}


		inline void neutralizeRemainder(const iter& pivot_iter, Comp pred) {
			int thNo = 0;
			typedef typename std::iterator_traits<iter>::value_type T;
			T pivot = *pivot_iter;
			T temp;

			iter lOut[PP_BUFFERSIZE]; // Direct writeout to struct
			iter rOut[PP_BUFFERSIZE];
			int64_t lc = 0;
			int64_t rc = 0;

			// The indexes we actually want to work with
			// and which will be subject to change
			int l_begin_i = size-2;
			iter l_begin = left_rem[l_begin_i];
			iter l_end = left_rem[l_begin_i+1];
			l_begin_i -= 2;
			int r_begin_i = size-1;
			iter r_begin = right_rem[r_begin_i];
			iter r_end = right_rem[r_begin_i-1]-1;
			r_begin_i -= 2;

			bool lResult = 0;
			bool rResult = 0;
			iter ri = r_begin;
			iter li = l_begin;
			int64_t off;

			while((li!=l_end||rc>0) && (ri!=r_end||lc>0)) {
				while((lc<PP_BLOCKSIZE-15)&&(ri>(r_end+15))) {
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
				}
				while((lc<PP_BLOCKSIZE)&&(ri!=r_end)) {
					checkAndAdvanceToLeft(ri, lResult, pivot, lOut, lc)
				}

				while((rc<PP_BLOCKSIZE-15)&&(li<(l_end-15))) {
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
				}
				while((rc<PP_BLOCKSIZE)&&(li<l_end)) {
					checkAndAdvanceToRight(li, rResult, pivot, rOut, rc)
				}

				uint64_t minc = std::min(lc, rc);
				off = 0;
				if (minc > 0) {
					temp = *rOut[off];
					*rOut[off] = *lOut[off];
					off++;
					while (off<minc) {
						*lOut[off-1] = *rOut[off];
						*rOut[off] = *lOut[off];
						off++;
					}
					*lOut[off-1] = temp;
				}

				rc -= off;
				lc -= off;

        // Refill
        if (li==l_end || ri==r_end) {
          // Prepare buffer
          if (rc>0) {
            memmove(rOut, rOut+off, rc*sizeof(iter));
          } else if (lc>0) {
            memmove(lOut, lOut+off, lc*sizeof(iter));
          }
          off = 0;

          // Perform actual refill
					if ((ri==r_end&&lc==0)&&r_begin_i>0) {
						right_rem[r_begin_i+3] = r_begin;

						r_begin = right_rem[r_begin_i];
						r_end = right_rem[r_begin_i-1]-1;
						r_begin_i -= 2;
					}

					if ((li==l_end&&rc==0)&&l_begin_i>-1) {
						left_rem[l_begin_i+2] = l_end;

						l_begin = left_rem[l_begin_i];
						l_end = left_rem[l_begin_i+1];
						l_begin_i -= 2;
					}
        }
			}
			
		}
	};
}

#endif
