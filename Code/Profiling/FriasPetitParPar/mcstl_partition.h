/**************************************************************************
In the following, we give a reproduction of the MCSTL 0.7.3-beta
mcstl_partition.h file with the changes to run the counters experiments.

To run time experiments, the original file can be used.

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

#ifndef _MCSTL_PARTITION_H
#define _MCSTL_PARTITION_H 1

//#define MCSTL_ASSERTIONS
#define MCSTL_VOLATILE volatile
#include <iostream>

#ifdef __COUNTERS__
#include <parallel_partition.h>
#endif

namespace mcstl
{

template<typename T, typename Comparator>
struct LessThanPivot
{
	Comparator comp;
	T pivot;
	
	LessThanPivot() {}

	LessThanPivot(Comparator c, T p) : comp(c), pivot(p) {}
	
 	inline bool operator()(T t) const
	{
		return comp(t, pivot);
	}
};


template<typename RandomAccessIterator, typename Predicate>
inline typename std::iterator_traits<RandomAccessIterator>::difference_type
parallel_partition(
	RandomAccessIterator begin,
	RandomAccessIterator end,
	Predicate pred,
	PIndex max_num_threads)
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

	MCSTL_CALL(n)

	HEURISTIC::set_number_of_threads();
		
	MCSTL_VOLATILE  DiffType left = 0, right = n - 1;	//shared
	MCSTL_VOLATILE DiffType leftover_left, leftover_right;
    MCSTL_VOLATILE DiffType leftnew, rightnew;	//shared
	bool* reserved_left, * reserved_right;	//shared
	reserved_left = new bool[max_num_threads];
	reserved_right = new bool[max_num_threads];

	DiffType chunk_size;
	if(HEURISTIC::partition_chunk_share > 0.0)
		chunk_size = std::max((DiffType)HEURISTIC::partition_chunk_size, (DiffType)((double)n * HEURISTIC::partition_chunk_share / (double)max_num_threads));
	else
		chunk_size = HEURISTIC::partition_chunk_size;
	
	while(right - left + 1 >= 2 * max_num_threads * chunk_size)	//at least good for two processors
	{
		DiffType num_chunks = (right - left + 1) / chunk_size;
		PIndex num_threads = (int)std::min((DiffType)max_num_threads, num_chunks / 2);

		for(int r = 0; r < num_threads; r++)
		{
			reserved_left[r] = false;
			reserved_right[r] = false;
		}
		leftover_left = 0;
		leftover_right = 0;

    #ifndef __COUNTERS__
		#pragma omp parallel num_threads(num_threads)
    #else
        unsigned int swaps_parallel_l = 0;
        unsigned int comps_parallel_l = 0; 
        unsigned int swaps_clean_l = 0;
        unsigned int comps_clean_l = 0;
        
        #pragma omp parallel num_threads(num_threads) reduction(+:swaps_parallel_l,comps_parallel_l,swaps_clean_l,comps_clean_l)
    #endif
		{
            DiffType thread_left, thread_left_border, thread_right, thread_right_border;	//private
			thread_left = left + 1;
			thread_left_border = thread_left - 1;	//just to satify the condition below
			thread_right = n - 1;
			thread_right_border = thread_right + 1;	//just to satify the condition below

	    	bool iam_finished = false;
			while(!iam_finished)
			{
				if(thread_left > thread_left_border)
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
			
				if(thread_right < thread_right_border)
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
				
				if(iam_finished)
					break;
				
				while(thread_left < thread_right)	//swap as usual
				{
					while(pred(begin[thread_left]) && thread_left <= thread_left_border){
						thread_left++;
                    #ifdef __COUNTERS__    
                        ++comps_parallel_l;
                    #endif
                    }
					while(!pred(begin[thread_right]) && thread_right >= thread_right_border){
						thread_right--;
                    #ifdef __COUNTERS__    
                        ++comps_parallel_l;
                    #endif
                    }
					#ifdef __COUNTERS__    
                        comps_parallel_l += 2;
                    #endif
					if(thread_left > thread_left_border || thread_right < thread_right_border)
						break;	//fetch new chunk(s)
					
					std::swap(begin[thread_left], begin[thread_right]);
                #ifdef __COUNTERS__    
                    ++swaps_parallel_l;
                #endif
					thread_left++;
					thread_right--;
				}
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
				leftnew = left - leftover_left * chunk_size;
				rightnew = right + leftover_right * chunk_size;
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
			{	//find spot and swap
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
			{	//find spot and swap
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
		}	//parallel
    #ifdef __COUNTERS__
        swaps_parallel += swaps_parallel_l;
        comps_parallel += comps_parallel_l; 
        swaps_clean += swaps_clean_l;
        comps_clean += comps_clean_l; 
    #endif     
	}	//"recursion"

	DiffType final_left = left, final_right = right;

#ifdef __COUNTERS__
    if (final_left < final_right)
        comps_clean += (final_right - final_left);
#endif

	while(final_left < final_right)
	{
		while(pred(begin[final_left]) && final_left < final_right){
			final_left++;	//go right until key is geq than pivot    
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

}	//namespace mcstl

#undef MCSTL_ASSERTIONS
#undef MCSTL_VOLATILE

#endif
