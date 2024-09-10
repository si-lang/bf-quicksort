#ifdef __MCSTL__
    #include <mcstl.h>
#endif

#ifndef _PART_CONST

#define _PART_CONST

const int NUM_PROCS = 2;

#define SEQUENTIAL 0
#define BARRIER_PARALLEL 1
#define BARRIER_PARALLEL_NO_TREE 2
#define MIX_BARRIER_PARALLEL 3
#define MIX_BARRIER_PARALLEL_NO_TREE 4
#define FETCH_ADD_PARALLEL_MCSTL 5
#define FETCH_ADD_PARALLEL_NO_TREE 6
#define FETCH_ADD_PARALLEL 7
#define FETCH_ADD_PARALLEL_NO_TREE_REF 8

#define INT 0
#define HARD_COMPARE 1 
#define HARD_MOVE 2 

#define RAND 0
#define BIASED 1 
#define SORTED 2 

#ifdef __MCSTL__
    unsigned int MIX_BARRIER_PARALLEL_block_size = mcstl::Heuristic<int>::partition_chunk_size;
    unsigned int FETCH_ADD_PARALLEL_block_size = mcstl::Heuristic<int>::partition_chunk_size/16;
#else
    unsigned int MIX_BARRIER_PARALLEL_block_size = 512;
    unsigned int FETCH_ADD_PARALLEL_block_size = 8092;
#endif

#endif
