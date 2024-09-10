#include <algorithm>


namespace naiveSort {
	template<typename T>
	inline T swp(T& a, T& b) {
		T tmp = a;
		a = b;
		b = tmp;
	}

	template<typename T>
	inline T pickRandomPivot(T arr[], int64_t left, int64_t right) {
		return arr[(left + right) / 2];
	}

	template<typename T>
	inline T pickMedianOfThreePivot(T arr[], int64_t left, int64_t right) {
		if (right-left>2) {
			const T felem = arr[left];
			const T selem = arr[(left + right) / 2];
			const T telem = arr[right-1];

			/*if (selem>felem && selem<telem || selem>telem && selem<felem) return selem;
			else if (telem>felem && telem<selem || telem>selem && telem < felem) return telem;
			return felem;*/

			if (felem < selem) {
				// f < s
				if (selem < telem) return selem; // f < s < t
				if (felem < telem) return telem; // f < t <! s
				return felem; // t <! f < s
			}

			if (selem < telem) {
				// s <! f && s < t
				if (felem < telem) return felem; // s < f < t
				return telem; // s < t < f
			}

			return selem; // t < s < f
		}
		
		return arr[left];
	}

	/*
	 * Naive quicksort routine from http://www.algolist.net/Algorithms/Sorting/Quicksort
	 */
	template<typename T>
	void quickSort(T arr[], int64_t left, int64_t right) {
		int64_t i = left, j = right;
		T tmp;
		//T pivot = pickRandomPivot(arr, left, right);
		T pivot = pickMedianOfThreePivot(arr, left, right);

		/* partition */
		while (i <= j) {
			while (arr[i] < pivot)
				i++;
			while (arr[j] > pivot)
				j--;
			if (i <= j) {
				swp(arr[i], arr[j]);
				i++;
				j--;
			}
		};

		/* recursion */
		if (left < j)
			quickSort(arr, left, j);
		if (i < right)
			quickSort(arr, i, right);
}


	/*template<typename Iter>
	Iter pickPerfectPivot(Iter begin, Iter end) {
		using T = typename std::iterator_traits<Iter>::value_type;
		std::vector<T> temp(begin, end);
		std::nth_element(temp.begin(), temp.begin()+temp.size()/2, temp.end());
		return begin+((temp.begin+temp.size()/2)-temp.begin());
	}

	template<typename Iter>
	Iter pickMedianOfThreePivot(const Iter& begin, const Iter& end) {
		if (end-begin>2) {
			const Iter firstIter = begin;
			const Iter secIter = begin+(std::distance(begin, end)/2);
			const Iter thirdIter = end-1;

			if (*firstIter < *secIter) {
				// f < s
				if (*secIter < *thirdIter) return secIter; // f < s < t
				if (*firstIter < *thirdIter) return thirdIter; // f < t <! s
				return firstIter; // t <! f < s
			}

			if (*secIter < *thirdIter) {
				// s <! f && s < t
				if (*firstIter < *thirdIter) return firstIter; // s < f < t
				return thirdIter; // s < t < f
			}

			return secIter; // t < s < f
		}
		
		return begin;
	}

	template<typename Iter, typename Pred>
	Iter partition(Iter begin, Iter end, Pred predicate) {
		end = end-1;
		while (begin<end) {
			while (!predicate(*begin)&&begin<end) { begin++; }
			while (predicate(*end)&&begin<end) { end--; }
			if (begin<end) {
				std::iter_swap(begin, end);
				begin++;
				end--;
			}
		}
		return begin;
	}

	template<typename Iter, typename Pred>
	void sort(const Iter& begin, const Iter& end, const Pred& predicate) {
		for (uint32_t j=0; j<(end-begin); j++) {
			std::cout << "v[" << j << "]: " << *(begin+j) << std::endl;
		}
		std::cout << std::endl;
		using T = typename std::iterator_traits<Iter>::value_type;
		Iter pivot = pickMedianOfThreePivot(begin, end);
		std::iter_swap(pivot, (end-1));
		Iter mid = naiveSort::partition(begin, end-1, [&](T v){
			return predicate(*(end-1), v);
		});
		std::iter_swap(mid, (end-1));

		if (begin<mid) {
			naiveSort::sort(begin, mid+1, predicate);
		}
		if ((mid+1)<(end-1)) {
			naiveSort::sort(mid+1, end, predicate);
		}
	}*/
}
