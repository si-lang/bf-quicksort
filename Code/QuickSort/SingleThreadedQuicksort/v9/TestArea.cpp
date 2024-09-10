/**
 * Calculates which input should be left and which should be right.
 * Returns arrays with indexes of v.
 * Indexes contained in lOut should be left and indexes contained in rOut should
 * be right.
 */
template<class T, typename BinaryPredicate>
void computePositions(T* v, uint64_t* lOut, uint64_t* rOut,
		const uint64_t begin, const uint64_t end, uint64_t& lCnt, uint64_t& rCnt,
		T& pivot, BinaryPredicate pred) {
	// TODO: Evtl. extrahieren
	uint64_t lc = lCnt;
	uint64_t rc = rCnt;

	bool result = 0;
	for (uint64_t i=begin; i<=end; i++) {
		result = pred(pivot, v[i]);
		lOut[lc] = i;
		rOut[rc] = i;

		lc += !result;
		rc += result;
	}

	lCnt = lc;
	rCnt = rc;
}


/**
 * Swaps positions of elements that should be left with positions of elements
 * that should be right.
 */
template<class T>
void swapPositions(T* v, const uint64_t* lPos, const uint64_t* rPos,
		uint64_t& lpCnt, uint64_t& rpCnt) {
	const uint64_t minLen = std::min(lpCnt, rpCnt);

  for (uint64_t i=0; (i<minLen)&&(lPos[lpCnt-i-1]>rPos[i]); i++) {
    std::swap(v[rPos[i]], v[lPos[lpCnt-i-1]]);
  }
}


/**
 * Executes computePositions and swapPositions in one call.
 */
template<class T, typename BinaryPredicate>
void myPartition(T* v, const uint64_t begin, const uint64_t end, const T& pivot,
		BinaryPredicate pred) {
	uint64_t* lOut = new uint64_t[end+1];
	uint64_t* rOut = new uint64_t[end+1];
	uint64_t lc = 0;
	uint64_t rc = 0;

	bool result = 0;
	for (uint64_t i=begin; i<=end; i++) {
		result = pred(pivot, v[i]);
		lOut[lc] = i;
		rOut[rc] = i;
		/*
		* Only increase out index if there was a match.
		* Else: Override wrong element (except for result == 1|0 in last iteration!)
		*/
		lc += !result;
		rc += result;
	}

	/*------ Swap ------*/
	const uint64_t minLen = std::min(lc, rc);

	for (uint64_t i=0; (i<minLen)&&(lOut[lc-i-1]>rOut[i]); i++) {
		std::swap(v[rOut[i]], v[lOut[lc-i-1]]);
	}
}

/*
 * My own Horare partition
 */
template<class T, typename BinaryPredicate>
void myHorarePartition(T* v, uint64_t begin, uint64_t end, const T& pivot,
		BinaryPredicate pred) {

	uint64_t* lOut = new uint64_t[end+1];
	uint64_t* rOut = new uint64_t[end+1];
	uint64_t lc = 0;
	uint64_t rc = 0;

	bool resultL = 0;
	bool resultR = 0;
	uint64_t j = end;
	for (uint64_t i=begin; i<=j; i++) {
		resultL = !pred(pivot, v[j]);
		resultR = pred(pivot, v[i]);

		lOut[lc] = j;
		rOut[rc] = i;

		lc += resultL;
		rc += resultR;
		j--;
	}

	uint64_t lCnt = lc+j+1-rc;
	uint64_t rCnt = rc+j+1-lc;

	//------ Swap -------
	uint64_t minLen = std::min(lc, rc);

	uint64_t i=0;
	for (; i<minLen; i++) {
		std::swap(v[rOut[rc-i-1]], v[lOut[lc-i-1]]);
	}

	if (lc <= rc) {
		int64_t rStop = rc-i-1;
		for (uint64_t k=0; rStop>=0; k++) {
			std::swap(v[rOut[rStop]], v[lCnt+rStop]);
			rStop--;
		}
	} else {
		int64_t lStop = lc-i-1;
		for (uint64_t k=0; lStop>=0; k++) {
			std::swap(v[lCnt-1-lStop], v[lOut[lStop]]);
			lStop--;
		}
	}
}


/*
 * Tuned quick sort adaption (unfinished?)
 * Not really better than easier to read standard implementation.
 */
template<typename iter, typename BinaryPredicate>
uint64_t qsBlockPartition(const iter beginIter, const iter endIter,
		iter pivotIter, BinaryPredicate pred) {
	auto pivot = *pivotIter; // Pivot might get swapped
	iter lOut[MYBLOCKSIZE];
	iter rOut[MYBLOCKSIZE];
	uint64_t lc = 0;
	uint64_t rc = 0;

	bool lResult = 0;
	bool rResult = 0;
	iter li = endIter;
	iter ri = beginIter;
	long long int lCnt = 0; //TODO Type
	uint64_t off = 0;

	while(ri<li) {
		while((lc<MYBLOCKSIZE)&&(ri<li)) {
			li--;
			lResult = !pred(pivot, *li);
			lOut[lc] = li;
			lc += lResult;
		}
		lCnt += lc-(ri-beginIter);

		while((rc<MYBLOCKSIZE)&&(ri<li)) {
			rResult = pred(pivot, *ri);
			rOut[rc] = ri;
			rc += rResult;
			ri++;
		}
		lCnt += (ri-beginIter)-rc;

		off = 0;
		uint64_t minc = std::min(lc, rc);
		for (; off<minc; off++) {
			std::iter_swap(rOut[off], lOut[off]);
		}
		lc-=off;
		rc-=off;
	}

	if (lc > 0) {
		uint64_t i = off;
		const iter lMidIter = beginIter+lCnt-1;
		iter swpIter = lMidIter;
		for (uint64_t j=1; (i<lc)&&(lOut[i]>lMidIter); j++) {
			if (pred(pivot, *(swpIter))) {
				std::iter_swap(swpIter, lOut[i]);
				i++;
			}
			swpIter--;
		}
	} else {
		uint64_t i = off;
		const iter rMidIter = beginIter+lCnt;
		iter swpIter = rMidIter;
		while ((i<rc)&&(rOut[i]<rMidIter)) {
			if (!pred(pivot, *(swpIter))) {
				std::iter_swap(rOut[i], swpIter);
				i++;
			}
			swpIter++;
		}
	}

	return lCnt;
}

// Parallel random number generators (attempts)
/*template<typename T>
void generatePRandomInputs(std::vector<T>& vecOut, uint64_t size, uint64_t seed=0) {
	uint numThreads = std::thread::hardware_concurrency();
  std::srand(time(NULL)*(12345+seed));
	uint64_t off = size/numThreads;

	//std::vector<uint32_t> rands;
	std::vector<std::thread> v;
	T* arr = new T[size];
  for (uint16_t i=0; i<numThreads; i++) {
		//rands.push_back(rand());
    v.push_back(std::thread([i, &vecOut, off, numThreads, arr]() {
			auto start = off*i;
			auto elem = i==numThreads ? vecOut.end()-(vecOut.begin()+off*numThreads) : off;

			uint64_t r = 0;
			uint seed = i;
      for (uint64_t j=0; j<elem; j++) {
        r = rand_r(&seed);
				arr[start+j] = r;
			}
    }));
  }

  for (uint16_t i=0; i<v.size(); i++) {
      v[i].join();
  }

	vecOut.insert(vecOut.begin(), arr, arr+size);
	delete[] arr;
}*/

/*template<typename T>
void generatePRandomInputs(std::vector<T>& vecOut, uint64_t size, uint64_t seed=0) {
	uint numThreads = std::thread::hardware_concurrency();
  std::srand(time(NULL)*(12345+seed));
	uint64_t off = size/numThreads;

	std::vector<uint32_t> rands;
	std::vector<std::thread> v;
  for (uint16_t i=0; i<numThreads; i++) {
		rands.push_back(rand());
    v.push_back(std::thread([i, &vecOut, off, numThreads, &rands]() {
			auto start = vecOut.begin()+off*i;
			auto end = i==numThreads-1 ? vecOut.end() : start+off;
			std::cout << "thread " << i << " start: " << start-vecOut.begin() << " end: " << end-vecOut.begin() << std::endl; 
			
			uint64_t j = off*i;
      for (auto iter=start; iter<end; iter++) {
        uint64_t r = rand_r(&rands[i]);
				vecOut[j]=r;
				j++;
      }
    }));
  }

  for (uint16_t i=0; i<v.size(); i++) {
      v[i].join();
  }
}*/
