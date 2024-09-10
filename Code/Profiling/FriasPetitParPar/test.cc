#include "parallel_partition_def.h"

#ifdef __MCSTL__
#include <mcstl.h>
#endif

#include <algorithm>
#include <sys/resource.h>
#include <sys/time.h>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <cassert>
#include <cmath>

template<typename Type>
class hard_to_compare{
    Type elem;

    public:  
    hard_to_compare():elem(){
        
    }

    hard_to_compare(const Type& a):elem(a){

    }

    hard_to_compare(const hard_to_compare<Type>& h):elem(h.elem){

    }

    bool operator< (const hard_to_compare<Type>& h) const{
        return int(sqrt(h.elem)) % 7 == int(sqrt(elem)) % 7;
    }
    
    hard_to_compare<Type>&  operator- (){
        elem = -elem;
        return *this;
    }
    
    hard_to_compare<Type>& operator= (const hard_to_compare<Type>& h){
        elem = h.elem;
        return *this;
    }
};

template<typename Type>
class hard_to_move{
    struct move{
        Type elem;
        Type other[15];
    };

    move m;
    
    public:  
    hard_to_move(){

    }

    hard_to_move(const Type& a){
        m.elem = a;
        m.other[0] = a;
        for (int i=1; i < 10; ++i){
        m.other[i] = m.other[i-1] + 1;
        }
    }

    bool operator< (const hard_to_move<Type>& h) const{
        return m.elem < h.m.elem;
    }
    
    hard_to_move<Type>&  operator- (){ 
        m.elem = - m.elem;
        return *this;
    }
    
    hard_to_move<Type>& operator= (const hard_to_move<Type>& h){  
        m.elem = m.elem;
        for (int i=0; i < 10; ++i){
            m.other[i] = h.m.other[i];
        }
        return *this;
    }
};

inline double now () {  
    struct rusage u;
    getrusage(RUSAGE_SELF,&u);    
    return 
        u.ru_utime.tv_sec  + u.ru_stime.tv_sec
    +(u.ru_utime.tv_usec + u.ru_stime.tv_usec)/1000000.0;
}


inline double now2 () {  
    struct timeval t;
    gettimeofday(&t, 0);
    return t.tv_sec + t.tv_usec/1000000.0;
}
    


template<typename vector_it, typename type,  typename predicate>
int check_partitioned(vector_it begin, vector_it end, vector_it part, type med, const predicate pred){
    bool is_partitioned = 1;
    int i = 0;
    for (vector_it it = begin; it < part and is_partitioned; ++it){
        ++i;
        if (not pred(*it)){
            is_partitioned = 0;
            std::cout << "false left" <<  part - begin << " " << it - begin << std::endl;
        }
    }
    for (vector_it it = part; it < end and is_partitioned; ++it){
        ++i;
        if (pred(*it)){
            is_partitioned = 0;
            std::cout << "false right " << end - part << " " <<end - it << std::endl;
        }
    }
    return is_partitioned;
}



template<typename Elem>
double test(const int n, const int num_procs, int part_sort_sel, const unsigned int algo, std::vector<Elem> v){
    typedef typename std::vector<Elem>::iterator vector_it;
    double t1=0;
    
    Elem med;
    
    if (n > 1)
        med = Elem(std::__median(v[0], *(v.begin() + (v.end() - v.begin())/ 2), *(v.end() - 1)));
    else
        med = v[0];


    vector_it part_it;
    #ifdef __COUNTERS__
        int bad_placed = 0;
    #endif
    if (part_sort_sel == 2){
        switch(algo){
            case SEQUENTIAL:
                //t1 = now2();         
                #ifdef __COUNTERS__
                    //the sequential case makes no extra comparisons or swaps
                    bad_placed = bad_placed_left(v.begin(), v.begin() + count_left(v.begin(), v.end(), pred<Elem>(med)), pred<Elem>(med));
                #endif
                #ifdef __MCSTL__                
                    nth_element_parallel_algorithms<typename std::vector<Elem>::iterator, std::less<Elem>, SEQUENTIAL>(v.begin(), v.begin()+ (v.begin() - v.end())/2, v.end(), std::less<Elem>(), num_procs);
                #else
                    std::nth_element(v.begin(), v.begin()+ (v.begin() - v.end())/2, v.end(), std::less<Elem>());
                #endif 
                //t1 = now2() - t1;  
                #ifdef __COUNTERS__
                    std::cout << bad_placed << " " << v.end() - v.begin() << " " << bad_placed << " " << v.end() - v.begin() << std::endl;
                #endif
                break;
            case BARRIER_PARALLEL:                 
                //t1 = now2();           
                #ifdef __MCSTL__
                    nth_element_parallel_algorithms<typename std::vector<Elem>::iterator, std::less<Elem>, BARRIER_PARALLEL> (v.begin(), v.begin()+ (v.begin() - v.end())/2, v.end(), std::less<Elem>(), num_procs);
                #endif
                //t1 = now2() - t1;  
                break;
            case BARRIER_PARALLEL_NO_TREE:
                //t1 = now2();  
                #ifdef __MCSTL__
                    nth_element_parallel_algorithms<typename std::vector<Elem>::iterator, std::less<Elem>, BARRIER_PARALLEL_NO_TREE> (v.begin(), v.begin()+ (v.begin() - v.end())/2, v.end(), std::less<Elem>(), num_procs);
                #endif
                //t1 = now2() - t1;  
                break;
            case MIX_BARRIER_PARALLEL:
                //t1 = now2();  
                #ifdef __MCSTL__
                    nth_element_parallel_algorithms<typename std::vector<Elem>::iterator, std::less<Elem>, MIX_BARRIER_PARALLEL> (v.begin(), v.begin()+ (v.begin() - v.end())/2, v.end(), std::less<Elem>(), num_procs);
                #endif
                //t1 = now2() - t1;  
                break;
            case MIX_BARRIER_PARALLEL_NO_TREE:
                //t1 = now2();  
                #ifdef __MCSTL__
                    nth_element_parallel_algorithms<typename std::vector<Elem>::iterator, std::less<Elem>, MIX_BARRIER_PARALLEL_NO_TREE> (v.begin(), v.begin()+ (v.begin() - v.end())/2, v.end(), std::less<Elem>(), num_procs);
                #endif
                //t1 = now2() - t1;  
                break;
            case FETCH_ADD_PARALLEL_MCSTL:
                //t1 = now2();  
                #ifdef __MCSTL__
                    nth_element_parallel_algorithms<typename std::vector<Elem>::iterator, std::less<Elem>, FETCH_ADD_PARALLEL_MCSTL> (v.begin(), v.begin()+ (v.begin() - v.end())/2, v.end(), std::less<Elem>(), num_procs);
                #endif
                //t1 = now2() - t1;  
                break; 
            case FETCH_ADD_PARALLEL_NO_TREE:
                #ifdef __MCSTL__
                //t1 = now2();                     
                nth_element_parallel_algorithms<typename std::vector<Elem>::iterator, std::less<Elem>, FETCH_ADD_PARALLEL_NO_TREE> (v.begin(), v.begin()+ (v.begin() - v.end())/2, v.end(), std::less<Elem>(), num_procs);               
                //t1 = now2() - t1;    
                #endif
                break;                
            case FETCH_ADD_PARALLEL:
                //t1 = now2();  
                #ifdef __MCSTL__
                    nth_element_parallel_algorithms<typename std::vector<Elem>::iterator, std::less<Elem>, FETCH_ADD_PARALLEL> (v.begin(), v.begin()+ (v.begin() - v.end())/2, v.end(), std::less<Elem>(), num_procs);
                #endif
                //t1 = now2() - t1;  
                break;         
            case FETCH_ADD_PARALLEL_NO_TREE_REF:
                #ifdef __MCSTL__
                //t1 = now2();                
                nth_element_parallel_algorithms<typename std::vector<Elem>::iterator, std::less<Elem>, FETCH_ADD_PARALLEL_NO_TREE_REF> (v.begin(), v.begin()+ (v.begin() - v.end())/2, v.end(), std::less<Elem>(), num_procs); 
                //t1 = now2() - t1;    
                #endif
                break;
	    default:
	        break;	
        }
    }
    else{
    #ifdef __COUNTERS__
        //the sequential case makes no extra comparisons or swaps
        if (algo == SEQUENTIAL)
            bad_placed = bad_placed_left(v.begin(), v.begin() + count_left(v.begin(), v.end(), pred<Elem>(med)), pred<Elem>(med));
    #endif
        if (part_sort_sel == 0){
            parallel_partition_algorithms_with_check(v.begin(),v.end(), part_it, pred<Elem>(med), num_procs, algo);
        }
    #ifdef __COUNTERS__
        if (algo == SEQUENTIAL)
            std::cout << bad_placed << " " << v.end() - v.begin() << " " << bad_placed << " " << v.end() - v.begin() << std::endl;
    #endif
    }

    /*if(part_sort_sel == 0 and not check_partitioned(v.begin(), v.end(), part_it, med, pred<Elem>(med)))
        std::cout<< "not partitioned" << std::endl; 
	*/

    #ifndef __COUNTERS__
        //std::cout << t1 << std::endl; 
    #endif
    return t1;
}




template<typename Elem>
double test_rep(const int n, const int num_procs, int part_sort_sel, const unsigned int algo, int input_data, int block_size, const int REP){
    std::vector<Elem> v(n);
    Elem elem0(0);

    for (int i = 0; i < n; ++i) v[i] = rand();
    /*if (input_data == BIASED){
        for (int i = 1; i < n; i += 2){
            if (not(v[i] < elem0)) v[i] = -v[i];
            if (v[i-1] < elem0) v[i-1] = -v[i-1];
        }
        v[0] = elem0;
    }*/    
    if (input_data == BIASED){
        for (int i = 0; i < num_procs; ++i){
            for (int k = i*block_size; k < n; k+= num_procs* block_size){
                for (int j = k; j < (k + block_size) and j < n; ++j){           
                    if (i%2 == 0) {
                        if (not(v[j] < elem0)) v[j] = -v[j];
                    }
                    else{
                        if (v[j] < elem0) v[j] = -v[j];
                    }
                }
            }
        }
        v[0] = elem0;
    }
    else if (input_data == SORTED)
        sort(v.begin(), v.end());

    double par = now2();
    for (int i=0; i< REP; ++i){
        test<Elem>(n,num_procs, part_sort_sel, algo, v);
    }	
    double seq = now2();
    par = seq - par;
#ifndef __COUNTERS__
    for (int i=0; i< REP; ++i){
        test<Elem>(n,num_procs, part_sort_sel, 9, v);
    }
#endif
    seq = now2()- seq;

#ifndef __COUNTERS__
    std::cout << par - seq << std::endl;
#endif
}


int main(int argc, char* argv[])
{

    int seed = atoi(argv[1]);
    int num_procs = atoi(argv[2]);
    int n = atoi(argv[3]);
    int type = atoi(argv[4]);
    int input_data = atoi(argv[5]);
    int algo = atoi(argv[6]);
    int block_size = atoi(argv[7]);
    int part_sort = 0;
    if (argv[8])
        part_sort = atoi(argv[8]); 

    int error;

    //Fix variables   
    
    MIX_BARRIER_PARALLEL_block_size = block_size;
    FETCH_ADD_PARALLEL_block_size = block_size;
    
    #ifdef __MCSTL__
        mcstl::Heuristic<int>::num_threads = num_procs;
        mcstl::Heuristic<int>::partition_minimal_n =  block_size;
        mcstl::Heuristic<int>::partition_chunk_size = block_size;
    #endif  
    std::cout << seed << " " << num_procs << " " << n << " " << type << " " << input_data << " " << algo << " " << block_size <<  " " << part_sort <<  " ";
    srand(seed);

#ifndef __COUNTERS__
    const int REP = 10;    
#else
    const int REP = 1;  
#endif

    switch(type){
        case INT:
            test_rep<int>(n,num_procs,part_sort,algo,input_data, block_size, REP);
            break;
        case HARD_COMPARE:
            test_rep< hard_to_compare<int> >(n,num_procs,part_sort,algo,input_data, block_size, REP);
            break;
        case HARD_MOVE:
            test_rep<hard_to_move <int> >(n,num_procs,part_sort,algo,input_data, block_size, REP);
            break;
    }
}
    

