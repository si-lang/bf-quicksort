#include "parallel_partition.h"
#include <algorithm>
#ifdef __MCSTL__
#include <mcstl.h>
#endif


#ifndef __PART_DEF__
#define __PART_DEF__

template<typename _RandomAccessIterator, typename _Predicate, unsigned int algo>
    inline void
    parallel_partition_algorithms(const _RandomAccessIterator __first,
              const _RandomAccessIterator __last, 
                          _RandomAccessIterator& __cut,  _Predicate __pred, const unsigned int num_threads){
    switch(algo){
        case SEQUENTIAL:
        #ifdef __MCSTL__
            __cut = std::partition(__first, __last, __pred, mcstl::sequential_tag());
        #else
            __cut = std::partition(__first, __last, __pred);
        #endif 
            break;
        case BARRIER_PARALLEL:
            unguarded_parallel_partition_1(__first, __last, __cut, __pred, num_threads);
            break;        
        case BARRIER_PARALLEL_NO_TREE:
            unguarded_parallel_partition_1_no_tree(__first, __last, __cut, __pred, num_threads);
            break;
        case MIX_BARRIER_PARALLEL:
            unguarded_parallel_partition_1_blocked(__first, __last, __cut, __pred,num_threads);
            break;        
        case MIX_BARRIER_PARALLEL_NO_TREE:
            unguarded_parallel_partition_1_blocked_no_tree(__first, __last, __cut, __pred, num_threads);
            break;
        case FETCH_ADD_PARALLEL_MCSTL: 
        #ifdef __MCSTL__
            unguarded_parallel_partition_2_mcstl(__first, __last, __cut, __pred, num_threads);
        #endif
            break;
        case FETCH_ADD_PARALLEL_NO_TREE:
        #ifdef __MCSTL__
            __cut = __first + mcstl::parallel_partition(__first,__last, __pred, num_threads) ;
        #endif
            break;
        case FETCH_ADD_PARALLEL:
            unguarded_parallel_partition_2(__first, __last, __cut, __pred, num_threads);
            break;                
        case FETCH_ADD_PARALLEL_NO_TREE_REF: 
        #ifdef __MCSTL__
            __cut = __first + parallel_partition_new(__first,__last, __pred, num_threads) ;
        #endif
            break;        
    }
}


template<typename _RandomAccessIterator, typename _Predicate>
    inline void
    parallel_partition_algorithms_with_check(const _RandomAccessIterator __first,
              const _RandomAccessIterator __last, 
                          _RandomAccessIterator& __cut,  _Predicate __pred, unsigned int num_threads = NUM_PROCS, const unsigned int algo = 0){
    num_threads = std::min(static_cast<typename _RandomAccessIterator::difference_type>(num_threads), __last - __first); 

    switch(algo){
        case SEQUENTIAL:
        #ifdef __MCSTL__
            __cut = std::partition(__first, __last, __pred, mcstl::sequential_tag());
        #else
            __cut = std::partition(__first, __last, __pred);
        #endif 
            break;
        case BARRIER_PARALLEL:
            unguarded_parallel_partition_1(__first, __last, __cut, __pred, num_threads);
            break;        
        case BARRIER_PARALLEL_NO_TREE:
            unguarded_parallel_partition_1_no_tree(__first, __last, __cut, __pred, num_threads);
            break;
        case MIX_BARRIER_PARALLEL:
            unguarded_parallel_partition_1_blocked(__first, __last, __cut, __pred,num_threads);
            break;        
        case MIX_BARRIER_PARALLEL_NO_TREE:
            unguarded_parallel_partition_1_blocked_no_tree(__first, __last, __cut, __pred, num_threads);
            break;   
        case FETCH_ADD_PARALLEL_MCSTL: 
        #ifdef __MCSTL__
            unguarded_parallel_partition_2_mcstl(__first, __last, __cut, __pred, num_threads);
        #endif
            break;
        case FETCH_ADD_PARALLEL_NO_TREE:
        #ifdef __MCSTL__
            __cut = __first + mcstl::parallel_partition(__first,__last, __pred, num_threads) ;
        #endif
            break;
        case FETCH_ADD_PARALLEL:
            unguarded_parallel_partition_2(__first, __last, __cut, __pred, num_threads);
            break;                
        case FETCH_ADD_PARALLEL_NO_TREE_REF: 
        #ifdef __MCSTL__
            __cut = __first + parallel_partition_new(__first,__last, __pred, num_threads) ;
        #endif
            break;  
    }
}


/*template<typename RandomAccessIterator, typename Comparator, unsigned int algo>
inline void nth_element_parallel_algorithms(RandomAccessIterator begin, RandomAccessIterator nth, RandomAccessIterator end, Comparator comp, int num_threads)
{
    typedef typename std::iterator_traits<RandomAccessIterator>::value_type ValueType;
    typedef typename std::iterator_traits<RandomAccessIterator>::difference_type DiffType;

#ifdef __MCSTL__
    DiffType minimal_n =    mcstl::HEURISTIC::partition_minimal_n;
#else
    DiffType minimal_n = 100000;      
#endif

    RandomAccessIterator smaller;
    ValueType pivot;
    
    while((end - begin) >= minimal_n)     // break if input range to small
    {
        pivot = std::__median(*begin, *(begin + (end - begin) / 2), *(end - 1), comp);
        predComp<ValueType, Comparator> pred = predComp<ValueType, Comparator>(pivot, comp);
        if((end - begin) >=  minimal_n){
            parallel_partition_algorithms<RandomAccessIterator, predComp<ValueType, Comparator>, algo>(begin, end, smaller, pred, num_threads);
        }
        else
        #ifdef __MCSTL__
            smaller = std::partition(begin, end, pred, mcstl::sequential_tag());
        #else
            smaller = std::partition(begin, end, pred);
        #endif
    
        if (smaller <= nth) //compare iterators
            begin = smaller;
        else
            end = smaller;
    }
    #ifdef __MCSTL__
        std::sort(begin, end, comp, mcstl::sequential_tag());   
    #else
       std::sort(begin, end, comp);    
    #endif
}*/

#endif
