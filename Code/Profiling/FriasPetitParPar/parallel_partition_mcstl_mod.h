#ifdef __MCSTL__
#include <mcstl.h>
#endif

/**************************************************************************
The code of the following functions was extracted originally from the 
MCSTL 0.7.3-beta mcstl_partition.h file. 

We have modified them to perform the tests.

The copyright of the file is reproduced in the following: 
***************************************************************************/
/***************************************************************************
 *   Copyright (C) 2006 by Johannes Singler                                *
 *   singler@ira.uka.de                                                    *
 *   Distributed under the Boost Software License, Version 1.0.            *
 *   (See accompanying file LICENSE_1_0.txt or copy at                     *
 *   http://www.boost.org/LICENSE_1_0.txt)                                 *
 *   Part of the MCSTL   http://algo2.iti.uni-karlsruhe.de/singler/mcstl/  *
 ***************************************************************************/

#ifndef __PART_MOD__
#define __PART_MOD__

#include "parallel_partition.h"
#ifdef __MCSTL__
/* Parallel phase of the MCSTL */
template<typename RandomAccessIterator, typename Predicate>
inline unsigned int __guarded_partition_with_blocks_mcstl(const RandomAccessIterator begin_orig, typename std::iterator_traits<RandomAccessIterator>::difference_type&  thread_left, typename std::iterator_traits<RandomAccessIterator>::difference_type& thread_left_border, typename std::iterator_traits<RandomAccessIterator>::difference_type&  thread_right, typename std::iterator_traits<RandomAccessIterator>::difference_type& thread_right_border, 
volatile typename std::iterator_traits<RandomAccessIterator>::difference_type& left, volatile typename  std::iterator_traits<RandomAccessIterator>::difference_type& right, unsigned int& comps_parallel_l, const Predicate& pred, const unsigned int chunk_size){
            #ifdef __COUNTERS__
                unsigned int swaps_parallel_l = 0;
            #endif
            typedef typename std::iterator_traits<RandomAccessIterator>::difference_type
    DiffType;
            
            bool iam_finished = false;
            while(!iam_finished)
            {
                if(thread_left > thread_left_border){
                    #pragma omp critical(this_synchro)
                    {
                        if(left + (chunk_size - 1) <= right)
                        {
                            thread_left = left;
                            thread_left_border = left + (chunk_size - 1);
                            left += chunk_size;
                        }
                        else
                            iam_finished = true;
                    }
                }

                if(thread_right < thread_right_border){
                    #pragma omp critical(this_synchro)
                    {
                        if(left <= right - (chunk_size - 1))
                        {
                            thread_right = right;
                            thread_right_border = right - (chunk_size - 1);
                            right -= chunk_size;
                        }
                        else
                            iam_finished = true;
                    }
                }
                if (iam_finished)
                    break;

                while(thread_left < thread_right)   //swap as usual
                {
                    while(pred(begin_orig[thread_left]) && thread_left <= thread_left_border){
                        thread_left++;
                    #ifdef __COUNTERS__    
                        ++comps_parallel_l;
                    #endif
                    }
                    while(!pred(begin_orig[thread_right]) && thread_right >= thread_right_border){
                        thread_right--;
                    #ifdef __COUNTERS__    
                        ++comps_parallel_l;
                    #endif
                    }
                    #ifdef __COUNTERS__    
                        comps_parallel_l += 2;
                    #endif
                    if(thread_left > thread_left_border || thread_right < thread_right_border)
                        break;  //fetch new chunk(s)
                    
                    std::swap(begin_orig[thread_left], begin_orig[thread_right]);
                #ifdef __COUNTERS__    
                    ++swaps_parallel_l;
                #endif
                    thread_left++;
                    thread_right--;
                }
            }
    #ifdef __COUNTERS__
        return swaps_parallel_l;
    #else
        return 0;
    #endif
}

/* Parallel phase using our implementation but MCSTL cleanup */
template<typename RandomAccessIterator, typename Predicate>
inline typename std::iterator_traits<RandomAccessIterator>::difference_type
parallel_partition_new(
    RandomAccessIterator begin,
    RandomAccessIterator end,
    Predicate pred,
    mcstl::PIndex max_num_threads)
{   
#ifdef __COUNTERS__
    unsigned int comps_optimal = end - begin;
    unsigned int swaps_optimal = bad_placed_left(begin, begin + count_left(begin, end, pred), pred); 
    unsigned int swaps_parallel = 0;
    unsigned int comps_parallel = 0; 
    unsigned int swaps_clean = 0;
    unsigned int comps_clean = 1;    
#endif
    typedef typename std::iterator_traits<RandomAccessIterator>::value_type
    ValueType;
    typedef typename std::iterator_traits<RandomAccessIterator>::difference_type
    DiffType;
    
    DiffType n = end - begin;

    mcstl::MCSTL_CALL(n)

    HEURISTIC::set_number_of_threads();
        
    DiffType left = 0, right = n - 1;    //shared
    volatile DiffType leftover_left, leftover_right;
    DiffType leftnew, rightnew;   //shared
    bool* reserved_left, * reserved_right;  //shared
    reserved_left = new bool[max_num_threads];
    reserved_right = new bool[max_num_threads];

    DiffType chunk_size;
    if(mcstl::HEURISTIC::partition_chunk_share > 0.0)
        chunk_size = std::max((DiffType)mcstl::HEURISTIC::partition_chunk_size, (DiffType)((double)n * mcstl::HEURISTIC::partition_chunk_share / (double)max_num_threads));
    else
        chunk_size = mcstl::HEURISTIC::partition_chunk_size;
    
    while(right - left + 1 >= 2 * max_num_threads * chunk_size) //at least good for two processors
    {
        DiffType num_chunks = (right - left + 1) / chunk_size;
        mcstl::PIndex num_threads = (int)std::min((DiffType)max_num_threads, num_chunks / 2);

        for(int r = 0; r < num_threads; r++)
        {
            reserved_left[r] = false;
            reserved_right[r] = false;
        }
        leftover_left = 0;
        leftover_right = 0;    
    
        const unsigned int num_blocks_full = n/chunk_size;
        const unsigned int num_blocks = num_blocks_full  + ((n%chunk_size == 0) ? 0 : 1);
        RandomAccessIterator first_block = begin + max_num_threads*chunk_size;
        //The uncompleted block, if exists, is at the end
         RandomAccessIterator last_block = begin + (num_blocks - max_num_threads)*chunk_size;    
        unsigned int swaps_parallel_l = 0;
    #ifndef __COUNTERS__
        #pragma omp parallel num_threads(num_threads)
    #else

        unsigned int comps_parallel_l = 0; 
        unsigned int swaps_clean_l = 0;
        unsigned int comps_clean_l = 0;     
        #pragma omp parallel num_threads(num_threads) reduction(+:swaps_parallel_l,comps_parallel_l,swaps_clean_l,comps_clean_l)
    #endif
        {            
            unsigned int id = omp_get_thread_num(); 
            block_ext<RandomAccessIterator>  not_complete_block;

            RandomAccessIterator this_f = begin + id*chunk_size;
            RandomAccessIterator this_l = begin + (num_blocks - id)*chunk_size;
            RandomAccessIterator this_last_r = std::min(this_l, end);
                           
            swaps_parallel_l = __guarded_partition_with_blocks(this_f, this_f + chunk_size, this_f, this_l - chunk_size, this_last_r, this_last_r, not_complete_block, first_block, last_block, pred, chunk_size);   

            DiffType thread_left = left + 1;
            DiffType thread_left_border = thread_left - 1;  //just to satify the condition below
            DiffType thread_right = n - 1;
            DiffType thread_right_border = thread_right + 1;    //just to satify the condition below
            
            if (not_complete_block.last <= first_block){
            #ifdef __COUNTERS__
                comps_parallel_l = not_complete_block.cur - not_complete_block.first;
            #endif
                thread_left = not_complete_block.cur - begin;
                thread_left_border = not_complete_block.last - 1 - begin;
            }
            else{                        
            #ifdef __COUNTERS__
                comps_parallel_l = not_complete_block.last - not_complete_block.cur;
            #endif
                thread_right = not_complete_block.cur - 1 - begin;
                thread_right_border = not_complete_block.first - begin;
             }
            
            //now swap the leftover chunks to the right places
            
            if(thread_left <= thread_left_border)
            #pragma omp atomic
                leftover_left++;
            if(thread_right >= thread_right_border)
            #pragma omp atomic
                leftover_right++;
            
            #pragma omp barrier

            #pragma omp single
            {
                left = first_block - begin;
                right = first_block - begin;
                leftnew = left - leftover_left * chunk_size;
                rightnew = right + leftover_right * chunk_size - 1;
            }

            #pragma omp barrier

            if(thread_left <= thread_left_border && thread_left_border >= leftnew)//<=> thread_left_border + (chunk_size - 1) >= leftnew
            {
                //chunk already in place, reserve spot
                reserved_left[(left - (thread_left_border + 1)) / chunk_size] = true;               
            }
            if(thread_right >= thread_right_border && thread_right_border <= rightnew)//<=> thread_right_border - (chunk_size - 1) <= rightnew
            {
                //chunk already in place, reserve spot
                reserved_right[((thread_right_border - 1) - right) / chunk_size] = true;                                                          

            }

            #pragma omp barrier

            if(thread_left <= thread_left_border && thread_left_border < leftnew)
            {   //find spot and swap
                DiffType swapstart = -1;
                #pragma omp critical
                {
                    for(int r = 0; r < leftover_left; r++)
                        if(!reserved_left[r])
                        {
                            reserved_left[r] = true;
                            swapstart = left - (r + 1) * chunk_size;
                            break;
                        }
                }

#ifdef MCSTL_ASSERTIONS
                assert(swapstart != -1);
#endif
            #ifdef __COUNTERS__
                 swaps_clean_l += chunk_size;
            #endif
                std::swap_ranges(begin + thread_left_border - (chunk_size - 1), begin + thread_left_border + 1, begin + swapstart);
            }
        
            if(thread_right >= thread_right_border && thread_right_border > rightnew)
            {   //find spot and swap
                DiffType swapstart = -1;
                #pragma omp critical
                {
                    for(int r = 0; r < leftover_right; r++)
                        if(!reserved_right[r])
                        {
                            reserved_right[r] = true;
                            swapstart = right + r * chunk_size + 1;
                            break;
                        }
                }
                
#ifdef MCSTL_ASSERTIONS
                assert(swapstart != -1);
#endif
                #ifdef __COUNTERS__
                    swaps_clean_l += chunk_size;
                #endif
                std::swap_ranges(begin + thread_right_border, begin + thread_right_border + chunk_size, begin + swapstart);
            }
#ifdef MCSTL_ASSERTIONS
            #pragma omp barrier

            #pragma omp single
            {
                for(int r = 0; r < leftover_left; r++)
                    assert(reserved_left[r]);
                for(int r = 0; r < leftover_right; r++)
                    assert(reserved_right[r]);
            }

            #pragma omp barrier
#endif
            
            #pragma omp barrier
            left = leftnew;
            right = rightnew; 
        }   //parallel
    #ifdef __COUNTERS__
        comps_parallel += n - (right - left + 1) + comps_parallel_l; 
        swaps_parallel += swaps_parallel_l;
        swaps_clean += swaps_clean_l;
        comps_clean += comps_clean_l; 
    #endif     
    }   //"recursion"

    DiffType final_left = left, final_right = right;

#ifdef __COUNTERS__
    if (final_left < final_right)
        comps_clean += (final_right - final_left);
#endif

    while(final_left < final_right)
    {
        while(pred(begin[final_left]) && final_left < final_right){
            final_left++;   //go right until key is geq than pivot    
        }
        while(!pred(begin[final_right]) && final_left < final_right){
            final_right--;//go left until key is less than pivot            
        }          
        if(final_left == final_right)
            break;
        std::swap(begin[final_left], begin[final_right]);
    
    #ifdef __COUNTERS__
        ++swaps_clean;
    #endif
        final_left++;
        final_right--;
    }

    #ifdef __COUNTERS__
        std::cout << swaps_clean + swaps_parallel - swaps_optimal << " " << comps_clean + comps_parallel - comps_optimal << " " << swaps_clean << " " << comps_clean << std::endl;
    #endif  

    //all elements on the left side are < piv, all elements on the right are >= piv
    
    delete[] reserved_left;
    delete[] reserved_right;
    
    //element "between" final_left and final_right might not have been regarded yet            
    if(final_left < n && !pred(begin[final_left]))
        //really swaps_clean
        return final_left;
    else
        return final_left + 1;
}
#endif

#endif
