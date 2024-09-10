#ifndef MYMEDIAN_CPP
#define MYMEDIAN_CPP

namespace myMedian {
  template<typename iter>
	iter medianOfThree(const iter begin, const iter end) {
		const iter firstIter = begin;
		const iter secIter = begin+(std::distance(begin, end)/2);
		const iter thirdIter = end-1;

		iter arr[3];
		short curPos = 0;

		arr[curPos] = firstIter;
		curPos += (*secIter <= *firstIter && *firstIter <= *thirdIter)
				|| (*secIter >= *firstIter && *firstIter >= *thirdIter);
		arr[curPos] = secIter;
		curPos += (*firstIter <= *secIter && *secIter <= *thirdIter)
				|| (*firstIter >= *secIter && *secIter >= *thirdIter);
		arr[curPos] = thirdIter;
		curPos += (*secIter <= *thirdIter && *thirdIter <= *firstIter)
				|| (*secIter >= *thirdIter && *thirdIter >= *firstIter);

		return arr[0];
	}
}
#endif
