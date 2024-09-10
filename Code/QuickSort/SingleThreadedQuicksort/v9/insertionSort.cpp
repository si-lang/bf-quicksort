#pragma once
#include <iterator>

#ifndef INSERTION_SORT_CPP
#define INSERTION_SORT_CPP

namespace myInsertionSort {
  template<typename iter, typename Comp>
  inline void insertionSort (iter begin, iter end, Comp comp) {
    iter j;
    typename std::iterator_traits<iter>::value_type temp;

    for (iter i = begin; i != end; i++) {
      j = i;

      while (j > begin && comp(*j, *(j-1))) {
          temp = *j;
          *j = *(j-1);
          *(j-1) = temp;
          j--;
      }
    }
  }


	/*/// This is a helper function for the sort routine.
  template<typename _RandomAccessIterator, typename _Compare>
    void __unguarded_linear_insert(_RandomAccessIterator __last,
				 _Compare __comp)
    {
      typename iterator_traits<_RandomAccessIterator>::value_type
				__val = std::move(*__last);
      _RandomAccessIterator __next = __last;
      --__next;
      while (__comp(__val, __next))
			{
				*__last = std::move(*__next);
				__last = __next;
				--__next;
			}
      *__last = std::move(__val);
    }

  /// This is a helper function for the sort routine.
  template<typename _RandomAccessIterator, typename _Compare>
    void
    __insertion_sort(_RandomAccessIterator __first,
		     _RandomAccessIterator __last, _Compare __comp)
    {
      if (__first == __last) return;

      for (_RandomAccessIterator __i = __first + 1; __i != __last; ++__i)
	{
	  if (__comp(__i, __first))
	    {
	      typename iterator_traits<_RandomAccessIterator>::value_type
		__val = std::move(*__i);
	      _GLIBCXX_MOVE_BACKWARD3(__first, __i, __i + 1);
	      *__first = std::move(__val);
	    }
	  else
	    std::__unguarded_linear_insert(__i,
				__gnu_cxx::__ops::__val_comp_iter(__comp));
	}
    }

  /// This is a helper function for the sort routine.
  template<typename _RandomAccessIterator, typename _Compare>
    inline void
    __unguarded_insertion_sort(_RandomAccessIterator __first,
			       _RandomAccessIterator __last, _Compare __comp)
    {
      for (_RandomAccessIterator __i = __first; __i != __last; ++__i)
				std::__unguarded_linear_insert(__i,
					__gnu_cxx::__ops::__val_comp_iter(__comp));
	}*/
}
#endif
