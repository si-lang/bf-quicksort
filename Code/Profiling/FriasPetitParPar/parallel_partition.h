#include <algorithm>
#include <iostream>
#include <vector>
#include <iterator>
#include <assert.h>

#include <sys/resource.h>
#include <sys/time.h>
#include <omp.h>

#include "partition_constants.h"
#include "parallel_partition_mcstl_mod.h"

#ifndef _PART
#define _PART


inline unsigned int ceil_pot2(const unsigned int size){
    if (size==0) return 1;
    unsigned int l = std::__lg(size);
    unsigned int pot= 1 << l;
    if (pot!=size) return pot<<1;
    return pot; 
}

/** Auxilliary classes **/
/* An element from a static tree */
struct element{
    unsigned int i_node;
    unsigned int i;         //count from left

    element(unsigned int i_node, unsigned int i): i_node(i_node), i(i){} 
    element(){}
    
    
    void print() {
        //std::cout << i_node << " " << i << std::endl;
    }

};

/* Static tree */
struct array_tree{
    unsigned int n_leaves;
    unsigned int ceil_n_leaves;
    unsigned int height;
    std::vector<unsigned int> size_l;
    std::vector<unsigned int> mis_l;
    std::vector<unsigned int> mis_r;

    array_tree(){
    }

    array_tree(const unsigned int n_l):  n_leaves(n_l), ceil_n_leaves(ceil_pot2(n_l)), height(std::__lg(ceil_pot2(n_l))),
        size_l(2*ceil_pot2(n_l)-1,0), mis_l(2*ceil_pot2(n_l)-1,0), mis_r(2*ceil_pot2(n_l)-1,0){ 
    }

    inline void init(const unsigned int n_l){
        assert(n_l > 0);
        n_leaves = n_l;
        ceil_n_leaves = ceil_pot2(n_l);
        height = std::__lg(ceil_n_leaves);
        /* Initialization
            - could be done in parallel
            - for small values of p is actually better like this important
            - it is really only necessary to initialize to 0 the extra ceil_n_leaves - n_leaves 
        */
        size_l = std::vector<unsigned int>(2*ceil_n_leaves-1,0);
        mis_l = std::vector<unsigned int>(2*ceil_n_leaves-1,0);
        mis_r = std::vector<unsigned int>(2*ceil_n_leaves-1,0);
    }
    
    unsigned int i_root() const{
        return 0;
    }

    unsigned int i_parent(const unsigned int index) const{
        return (index-1)/2;    
    }
    
    unsigned int i_left(const unsigned int index) const{
        return 2*index+1;
    }

    unsigned int i_right(const unsigned int index) const{
        return 2*index+2;
    }

    unsigned int i_right_sib(const unsigned int index) const{
        return index+1;
    }

    bool is_right(const unsigned int index) const{
        return index%2==0;
    }
    
    bool is_left(const unsigned int index) const{
        return not is_right(index);
    }

    unsigned int begin_leaves() const{
        return (1<<height)-1;
    }
    
    unsigned int end_leaves() const{
        return begin_leaves()+n_leaves; 
    }

    bool is_leaf(const unsigned int index) const{
        return index >= begin_leaves();
    }

    unsigned int i_ith_leaf(const unsigned int index) const{
        return index + begin_leaves();
    }

    unsigned int i_rank_leaf(const unsigned int index) const{
        return index - begin_leaves();
    }

    void sum_children_size_l(const unsigned int index){
        size_l[index] = size_l[i_left(index)] + size_l[i_right(index)];
    }

    void sum_children_mis_l(const unsigned int index){
        mis_l[index] = mis_l[i_left(index)] + mis_l[i_right(index)];
    }

    void sum_children_mis_r(const unsigned int index){
        mis_r[index] = mis_r[i_left(index)] + mis_r[i_right(index)];
    }

    //pre: it is called on a leaf
    void advance_r(unsigned int inc, element& elem) const{
        unsigned int r_inc = size_l[elem.i_node] + mis_r[elem.i_node] - (elem.i + 1);
        if (r_inc >= inc )
            elem.i += inc;
        else{
            do{
                inc -= r_inc;
                elem.i_node = subtree_upper_bound(elem.i_node);
                assert(mis_r[elem.i_node] == mis_r[i_right(elem.i_node)] + mis_r[i_left(elem.i_node)]);
                r_inc = mis_r[i_right(elem.i_node)];
            } while (inc > r_inc);
            ith(inc, i_right(elem.i_node), mis_r, elem);
            elem.i += size_l[elem.i_node];        
        }            
    }    

    //pre: it is called on a leaf
    void backward_l(unsigned int dec, element& elem) const{ 
        unsigned int l_dec =  elem.i - (size_l[elem.i_node] - mis_l[elem.i_node]);
        if (l_dec >= dec)
            elem.i -= dec;
        else{
            do{
                dec -= l_dec;
                elem.i_node = subtree_lower_bound(elem.i_node);
                l_dec = mis_l[i_left(elem.i_node)];
            } while (dec > l_dec);
            ith(l_dec - dec + 1, i_left(elem.i_node), mis_l, elem);
            elem.i += (size_l[elem.i_node] - mis_l[elem.i_node]);
        } 
    }
    
    //attention : returns index respect to v, not absolute
    void ith(unsigned int i, unsigned int i_node_root, const std::vector<unsigned int>& v, element& elem) const{
        assert (v[i_node_root]>=i);
        if (not is_leaf(i_node_root)){
            do{
                assert(v[i_node_root] == v[i_left(i_node_root)] + v[i_right(i_node_root)]);
                unsigned int i_root= v[i_left(i_node_root)];
                if (i <= i_root) i_node_root = i_left(i_node_root);
                else{
                    i_node_root = i_right(i_node_root);
                    i -= i_root;
                }
            } while(not is_leaf(i_node_root));
        }
        elem.i_node = i_node_root;
        elem.i = i - 1; //because indices in nodes count since one
        assert(v[i_node_root]>0);
    }  

    //attention : returns index respect to v, not absolute
    unsigned int rank(const std::vector<unsigned int>& v, unsigned int cur_node) const{
        assert(is_leaf(v[cur_node]));
        return 1 + rank_parent(v, cur_node); 
    }

    unsigned int rank_parent(const std::vector<unsigned int>& v, unsigned int cur_node) const{
        if (cur_node == i_root()) return 0;
        unsigned int i_node_parent = i_parent(cur_node);
        if (is_left(cur_node)) return rank_parent(v,i_node_parent);
        return v[i_left(i_node_parent)] + rank_parent(v,i_node_parent);    
    }

    unsigned int subtree_upper_bound(unsigned int i_node) const{
        while (is_right(i_node)){
            i_node = i_parent (i_node);            
        }
        return i_parent(i_node);
    }    

    unsigned int subtree_lower_bound(unsigned int i_node) const{
        while (is_left(i_node)){
            i_node = i_parent (i_node);            
        }
        return i_parent(i_node);
    }
    
    void print(const unsigned int i) const {
        std::cout << i << " " << size_l[i] << " " << mis_l[i] << " " << mis_r[i] << std::endl;
    }

    void print_tree() const{
        for (unsigned int i = 0; i < begin_leaves() + n_leaves; ++i)
            print(i);
    }
   
};

template<typename RandomAccessIterator>
struct block{
    RandomAccessIterator cur;
    RandomAccessIterator first;
    RandomAccessIterator last;

    block()
    {
    }
    
    block(const RandomAccessIterator& first, const RandomAccessIterator& last,  const RandomAccessIterator& cur):
     cur(cur), first(first), last(last)
    {
    }

    bool operator<(const block<RandomAccessIterator>& b2) const{
        return first < b2.cur;
    }

    bool operator<(const RandomAccessIterator b2) const{
        return first < b2;
    }
};

template<typename RandomAccessIterator>
struct block_ext : public block<RandomAccessIterator>{
    bool compared;

    block_ext()
    {
    }
    
    block_ext(const RandomAccessIterator& first, const RandomAccessIterator& last,  const RandomAccessIterator& cur):
    block<RandomAccessIterator>(first,last,cur)
    {
    }

};

template<typename RandomAccessIterator> 
class iterator_with_shift{
  public:
    typedef typename std::iterator_traits<RandomAccessIterator>::iterator_category
                                                            iterator_category;
    typedef typename std::
iterator_traits<RandomAccessIterator>::value_type  value_type;
    typedef typename std::iterator_traits<RandomAccessIterator>::difference_type
                                                             difference_type;
    typedef typename std::iterator_traits<RandomAccessIterator>::reference reference;
    typedef typename std::iterator_traits<RandomAccessIterator>::pointer   pointer;

    RandomAccessIterator it;
  private: 
    difference_type shift;

  public:
      iterator_with_shift(const RandomAccessIterator& it, const difference_type shift): it(it), shift(shift){
      }
     
      iterator_with_shift(const iterator_with_shift<RandomAccessIterator>& it_s){
        it = it_s.it;
        shift = it_s.shift;
      }

      iterator_with_shift<RandomAccessIterator>& operator=(const iterator_with_shift<RandomAccessIterator>& it_s){
        it = it_s.it;
        shift = it_s.shift;
        return *this;
      }

      // Allow iterator to constRandomAccessIterator conversion
      template<typename _Iter>
        iterator_with_shift(const iterator_with_shift<_Iter> __i)
        : it(__i.base()) { }

      // Forward iterator requirements
      reference
      operator*() const
      { return *it; }

      pointer
      operator->() const
      { return it.operator->(); }

      iterator_with_shift<RandomAccessIterator>&
      operator++()
      {
        it += shift;
        return *this;
      }

      iterator_with_shift<RandomAccessIterator>
      operator++(int)
      { iterator_with_shift<RandomAccessIterator> it_s(it , shift);
        operator++();
        return it_s; 
      }

      // Bidirectional iterator requirements
      iterator_with_shift<RandomAccessIterator>&
      operator--()
      {
        it -= shift;
        return *this;
      }

      iterator_with_shift<RandomAccessIterator>
      operator--(int)
      { iterator_with_shift<RandomAccessIterator> it_s(it , shift);
        operator--();
        return it_s; 
      }

      // Random access iterator requirements
      reference
      operator[](const difference_type& __n) const
      { return it[__n]; }

      iterator_with_shift<RandomAccessIterator>&
      operator+=(const difference_type& __n)
      { it += __n*shift; return *this; }

      iterator_with_shift<RandomAccessIterator>
      operator+(const difference_type& __n) const
      { return iterator_with_shift(it + __n*shift, shift); }

      iterator_with_shift<RandomAccessIterator>&
      operator-=(const difference_type& __n)
      { it -= __n*shift; return *this; }

      iterator_with_shift<RandomAccessIterator>
      operator-(const difference_type& __n) const
      { return iterator_with_shift(it - __n*shift, shift); }

      difference_type
      operator-(const iterator_with_shift<RandomAccessIterator>& it_s) const
      { return base() - it_s.base();}

      bool
      operator<(const iterator_with_shift<RandomAccessIterator>& it_s) const
      { return it < it_s.it; }

      bool
      operator>(const iterator_with_shift<RandomAccessIterator>& it_s) const
      { return it > it_s.it; }

      bool
      operator<=(const iterator_with_shift<RandomAccessIterator>& it_s) const
      { return it <= it_s.it; }

      bool
      operator>=(const iterator_with_shift<RandomAccessIterator>& it_s) const
      { return it >= it_s.it; }

      bool
      operator==(const iterator_with_shift<RandomAccessIterator>& it_s) const
      { return it == it_s.it; }
      
      bool
      operator!=(const iterator_with_shift<RandomAccessIterator>& it_s) const
      { return it != it_s.it; }
      
      RandomAccessIterator&
      base() 
      { return it; }      

      RandomAccessIterator
      base() const
      { return it; }
};


template<typename RandomAccessIterator> 
class blocked_iterator_with_shift{
  public:
    typedef typename std::iterator_traits<RandomAccessIterator>::iterator_category
                                                            iterator_category;
    typedef typename std::
iterator_traits<RandomAccessIterator>::value_type  value_type;
    typedef typename std::iterator_traits<RandomAccessIterator>::difference_type
                                                             difference_type;
    typedef typename std::iterator_traits<RandomAccessIterator>::reference reference;
    typedef typename std::iterator_traits<RandomAccessIterator>::pointer   pointer;

  private:
    iterator_with_shift<RandomAccessIterator> it; 
    difference_type block_size;
  public:
    difference_type index_in_block;

  public:
      blocked_iterator_with_shift(RandomAccessIterator& it_rand, const difference_type shift, const difference_type block_size, const difference_type  index_in_block = 0): it(it_rand , (shift - 1)*block_size), block_size(block_size), index_in_block(index_in_block) {
        
      }

      blocked_iterator_with_shift(const RandomAccessIterator it, const difference_type shift, const difference_type block_size, const difference_type  index_in_block = 0): it(it, (shift - 1)*block_size), block_size(block_size), index_in_block(index_in_block) {
        
      }

      blocked_iterator_with_shift<RandomAccessIterator>& operator=(blocked_iterator_with_shift<RandomAccessIterator>& it_s){
        it = it_s.it;
        block_size = it.block_size;
        index_in_block = it.index_in_block;
        return *this;
      }

      blocked_iterator_with_shift<RandomAccessIterator>& operator=(const blocked_iterator_with_shift<RandomAccessIterator> it_s){
        it = it_s.it;
        block_size = it_s.block_size;
        index_in_block = it_s.index_in_block;
        return *this;
      }

      // Allow iterator to constRandomAccessIterator conversion
      template<typename _Iter>
        blocked_iterator_with_shift(const blocked_iterator_with_shift<_Iter> __i)
        : it(__i.it.base(), __i.it.shift()), block_size(__i.block_size), index_in_block(__i.index_in_block) { }

      // Forward iterator requirements
      reference
      operator*() const
      { return *it; }

      pointer
      operator->() const
      { return it.operator->(); }

      blocked_iterator_with_shift<RandomAccessIterator>&
      operator++()
      {
        ++index_in_block;
        ++(it.base());
        if (index_in_block == block_size){
            ++it;
            index_in_block = 0;
        }
        return *this;
      }

      blocked_iterator_with_shift<RandomAccessIterator> 
      operator++(int)
      { blocked_iterator_with_shift<RandomAccessIterator> it_s(*this);
        operator++();
        return it_s; 
      }

      // Bidirectional iterator requirements
      blocked_iterator_with_shift<RandomAccessIterator>&
      operator--()
      {
        if (index_in_block == 0){
            --it;
            index_in_block = block_size;
        }
        --index_in_block;
        --(it.base());
        return *this;
      }

      blocked_iterator_with_shift<RandomAccessIterator>
      operator--(int)
      { blocked_iterator_with_shift<RandomAccessIterator> it_s(*this);
        operator--();
        return it_s; 
      }

      // Random access iterator requirements
      reference
      operator[](const difference_type& __n) const
      { return it[__n]; }

      blocked_iterator_with_shift<RandomAccessIterator>&
      operator+=(const difference_type& __n)
      { 
        it.base() += __n;
        index_in_block += __n;
        if (index_in_block >= block_size){            
            it += (index_in_block/block_size);
            index_in_block %= block_size;
        }      
        return *this; 
      }

      blocked_iterator_with_shift<RandomAccessIterator>
      operator+(const difference_type& __n) const
      { blocked_iterator_with_shift<RandomAccessIterator> ret(*this);
        ret += __n;
        return ret;
      }

      blocked_iterator_with_shift<RandomAccessIterator>&
      operator-=(const difference_type& __n)
      { 
        index_in_block -= __n;
        it.base() -= __n;
        if (index_in_block < 0){            
            it += (index_in_block/block_size);
            index_in_block %= block_size;
            index_in_block += block_size;
        }
        return *this; 
      }


      blocked_iterator_with_shift<RandomAccessIterator>
      operator-(const difference_type& __n) const
      { blocked_iterator_with_shift ret(*this);
        ret -= __n;
        return ret;
      }

      difference_type
      operator-(const blocked_iterator_with_shift<RandomAccessIterator>& b) const
      { 
            return base().base() - b.base().base();
      }

      bool
      operator<(const blocked_iterator_with_shift<RandomAccessIterator>& it_s) const
      { return it < it_s.it; }

      bool
      operator>(const blocked_iterator_with_shift<RandomAccessIterator>& it_s) const
      { return it > it_s.it; }

      bool
      operator<=(const blocked_iterator_with_shift<RandomAccessIterator>& it_s) const
      { return it <= it_s.it; }

      bool
      operator>=(const blocked_iterator_with_shift<RandomAccessIterator>& it_s) const
      { return it >= it_s.it; }      

      bool
      operator==(const blocked_iterator_with_shift<RandomAccessIterator>& it_s) const
      { return it == it_s.it; }

      bool
      operator!=(const blocked_iterator_with_shift<RandomAccessIterator>& it_s) const
      { return it != it_s.it; }

      iterator_with_shift<RandomAccessIterator>&
      base() 
      { return it; }      

      iterator_with_shift<RandomAccessIterator>
      base() const
      { return it; }
};


template<typename RandomAccessIterator, typename ThisIterator>
class iterator_wrapper{
    RandomAccessIterator first;
    unsigned int num_threads;

  public:
    iterator_wrapper(RandomAccessIterator& first, const typename RandomAccessIterator::difference_type num_threads): first(first), num_threads(num_threads){

    }

    iterator_wrapper(const RandomAccessIterator first, const typename RandomAccessIterator::difference_type num_threads): first(first), num_threads(num_threads){

    }

    ThisIterator operator()(const unsigned int block) const{
        return  ThisIterator(first + block, num_threads);
    } 
};

template<typename RandomAccessIterator>
class iterator_wrapper<RandomAccessIterator, blocked_iterator_with_shift<RandomAccessIterator> >{
    RandomAccessIterator first;
    unsigned int num_threads;
    typename RandomAccessIterator::difference_type block_size;

  public:
    iterator_wrapper(RandomAccessIterator& first, const typename RandomAccessIterator::difference_type num_threads, const typename RandomAccessIterator::difference_type block_size): first(first), num_threads(num_threads), block_size(block_size){

    }

    iterator_wrapper(const RandomAccessIterator first, const typename RandomAccessIterator::difference_type num_threads, const typename RandomAccessIterator::difference_type block_size): first(first), num_threads(num_threads), block_size(block_size){

    }

    blocked_iterator_with_shift<RandomAccessIterator> operator()(const typename RandomAccessIterator::difference_type block) const{
        return blocked_iterator_with_shift<RandomAccessIterator>(first + block*block_size, num_threads, block_size, 0);
    } 
};

template<typename RandomAccessIterator>
class iterator_wrapper<block_ext<RandomAccessIterator>, RandomAccessIterator>{
    const block_ext<RandomAccessIterator>* not_complete_blocks;

  public:
    iterator_wrapper(const block_ext<RandomAccessIterator>* block): not_complete_blocks(block){

    }

    RandomAccessIterator operator()(const unsigned int block) const{
        return not_complete_blocks[block].first;
    } 
};

template<typename RandomAccessIterator>
class iterator_wrapper<block<RandomAccessIterator>, RandomAccessIterator>{
    const block<RandomAccessIterator>* not_complete_blocks;

  public:
    iterator_wrapper(const block<RandomAccessIterator>* block): not_complete_blocks(block){

    }

    RandomAccessIterator operator()(const unsigned int block) const{
        return not_complete_blocks[block].first;
    } 
};


template<typename RandomAccessIterator>
class iterator_wrapper<RandomAccessIterator, RandomAccessIterator>{    
    const RandomAccessIterator first;
    unsigned int shift;
  
  public:  
    iterator_wrapper(RandomAccessIterator& first, const typename RandomAccessIterator::difference_type shift): first(first), shift(shift){

    }

    iterator_wrapper(const RandomAccessIterator first, const typename RandomAccessIterator::difference_type shift): first(first), shift(shift){

    }

    RandomAccessIterator operator()(const unsigned int block) const{
        return first + block*shift;
    }   
};


/* Helper to call parallel partition */
template<typename Elem>
class pred{
    const Elem& pivot;

public:
    pred(const Elem& e):pivot(e)
    {
    }

    bool operator()(const Elem& e) const
    {
        return e < pivot;
    } 
           
}; 

template<typename Elem, typename Comparator>
class predComp{
    const Elem& pivot;
    const Comparator& comp;

public:
    predComp(const Elem& e, const Comparator& c):pivot(e), comp(c)
    {
    }

    bool operator()(const Elem& e) const
    {
        return comp(e, pivot);
    } 
           
};


inline void reduce_tree_size_l(array_tree& sizes, const unsigned int id){
    unsigned int num_thr_used_orig = sizes.ceil_n_leaves;
    unsigned int num_thr_used = num_thr_used_orig/2;
    while (num_thr_used > 0){        
        #pragma omp barrier
        if (id < num_thr_used){              
            unsigned int i_node = id + num_thr_used -1;
            sizes.sum_children_size_l(i_node);
        }
        num_thr_used /= 2;
    }
    #pragma omp barrier 
}

inline void reduce_tree_mis(array_tree& sizes, const unsigned int id){
    unsigned int num_thr_used_orig = sizes.ceil_n_leaves;
    unsigned int num_thr_used = num_thr_used_orig/2;
    while (num_thr_used > 0){        
        #pragma omp barrier
        if (id < num_thr_used){ 
            unsigned int i_node = id + num_thr_used -1;
            sizes.sum_children_mis_l(i_node);
            sizes.sum_children_mis_r(i_node); 
            //std::cout << "mis " << i_node << " " << sizes.mis_l[i_node] << " " << sizes.mis_r[i_node] << std::endl;
        }
        num_thr_used /= 2;
    } 
    #pragma omp barrier
}


template<typename Element>
inline void reduce_array(std::pair<Element,Element>* array, const unsigned int num_thr_used_orig, const unsigned int id)
{
    unsigned int num_thr_used = num_thr_used_orig/2;
    unsigned int extra = num_thr_used_orig%2;
    int factor = 1;
    while (num_thr_used > 0){        
        #pragma omp barrier
        if (id < num_thr_used){              
            unsigned int left = id*2*factor;
            unsigned int right = left + factor;
            if (array[right].first < array[left].first)
                array[left].first = array[right].first;
        } 
               
        else if (id - num_thr_used < num_thr_used){ 
            unsigned int left = (id - num_thr_used)*2*factor;
            unsigned int right = left + factor;
            if (array[right].second > array[left].second)
                array[left].second = array[right].second;
        }

        num_thr_used += extra;
        extra = num_thr_used % 2;
        num_thr_used /= 2;
        factor *= 2;
    }
    #pragma omp barrier

}

inline unsigned int calculate_elements_to_swap(array_tree& sizes, element& first_el, element& last_el, element& this_first_el, element& this_last_el, const unsigned int num_threads, const unsigned int id){
    unsigned int misplaced = sizes.mis_l[sizes.i_root()];
    assert(misplaced == sizes.mis_r[sizes.i_root()]);
    
    //Ith-calculation + swapping in parallel                         
    if (misplaced > 0){
        unsigned int base_part = misplaced/num_threads;
        unsigned int extra =  misplaced%num_threads;
        #pragma omp single nowait
        {
            sizes.ith(1, sizes.i_root(), sizes.mis_r, first_el);  
            first_el.i += sizes.size_l[first_el.i_node]; 
        }
        #pragma omp single
        { 
            sizes.ith(sizes.mis_l[sizes.i_root()], sizes.i_root(), sizes.mis_l, last_el); 
            last_el.i += (sizes.size_l[last_el.i_node] - sizes.mis_l[last_el.i_node]);               
        }

        this_first_el = first_el;
        this_last_el = last_el;
        unsigned int add = (id < extra)? 1:0;
        
        //std::cout << "before swap " << id << " "<< base_part + add << std::endl; 
        
        if (base_part + add > 0){
            unsigned int previous =  base_part*id + std::min(id, extra);       
            sizes.backward_l(previous, this_last_el);   
            //std::cout << " after backward " << this_last_el.i_node << " " << this_last_el.i << " " << this_last_el.i- (sizes.size_l[this_last_el.i_node] - sizes.mis_l[this_last_el.i_node])  << std::endl;    
            sizes.advance_r(previous, this_first_el);        
            //std::cout << " after advance " << this_first_el.i_node << " " << this_first_el.i << " " << this_first_el.i- sizes.size_l[this_first_el.i_node] << std::endl;
            return base_part + add;
        }            
    }
    return 0;
}


template<typename RandomAccessIterator, typename WrappedIterator>
inline void swap(array_tree& sizes, element& first_el, element& last_el, unsigned int problem_size, iterator_wrapper<RandomAccessIterator, WrappedIterator> wrapper){
    WrappedIterator first_it(wrapper(sizes.i_rank_leaf(first_el.i_node)));
    WrappedIterator last_it(wrapper(sizes.i_rank_leaf(last_el.i_node)));
    //std::cout << problem_size << " " << *last_it << " "<< *first_it << std::endl;
    first_it += first_el.i;
    last_it += last_el.i; 
    //std::cout << problem_size << " " << *last_it << " "<< *first_it << std::endl;
    
    while (problem_size > 0){
        unsigned int mis_right = sizes.mis_r[first_el.i_node] - (first_el.i - sizes.size_l[first_el.i_node]);
        assert(sizes.mis_r[first_el.i_node] > 0);
        assert(mis_right > 0);
        unsigned int mis_left = sizes.mis_l[last_el.i_node] - (sizes.size_l[last_el.i_node] - (last_el.i + 1));
        assert(sizes.mis_l[last_el.i_node] > 0);
        assert(mis_left > 0);
        unsigned int loop = std::min(problem_size, std::min(mis_left, mis_right));

       
        //std::swap_ranges(last_it - (loop - 1), last_it + 1, first_it);
        for (int i=0; i< loop; ++i){
            std::iter_swap(first_it, last_it);
            ++first_it;
            --last_it;
        }    
        
        problem_size -= loop;
        first_el.i += loop;
        last_el.i -= (loop - 1);

        //std::cout << problem_size << " " << loop << std::endl; 
        if (first_el.i == sizes.size_l[first_el.i_node] + sizes.mis_r[first_el.i_node] and problem_size > 0){
            do{
                ++first_el.i_node;    
            } while (sizes.mis_r[first_el.i_node] == 0);
            first_el.i = sizes.size_l[first_el.i_node];        
            first_it = wrapper(sizes.i_rank_leaf(first_el.i_node));
            first_it += first_el.i; 
        }
        
        if (last_el.i == sizes.size_l[last_el.i_node] - sizes.mis_l[last_el.i_node] and problem_size > 0){
            do{
                --last_el.i_node;
                assert(last_el.i_node >= sizes.begin_leaves());
            } while (sizes.mis_l[last_el.i_node] == 0);
            last_el.i = sizes.size_l[last_el.i_node];
            last_it = wrapper(sizes.i_rank_leaf(last_el.i_node));
            last_it += (last_el.i - 1); 
        }
        --last_el.i;
    }
}


template<typename It1, typename It2>
inline unsigned int reduce_and_swap(array_tree& sizes, element& first_el, element& last_el, const unsigned int id, const unsigned int num_threads, 
iterator_wrapper<It1, It2> itw){
        reduce_tree_mis(sizes, id);
        element this_first_el, this_last_el;
        unsigned int num_elems = calculate_elements_to_swap(sizes, first_el, last_el, this_first_el, this_last_el, num_threads, id);
        //first_el.print();       last_el.print();
        //this_first_el.print();  this_last_el.print();
        if (num_elems != 0)
            swap(sizes, this_first_el, this_last_el, num_elems, itw);
        return num_elems;
    
}



template<typename RandomAccessIterator, typename Predicate>
    inline void unguarded_parallel_partition_1(
            const RandomAccessIterator first,
            const RandomAccessIterator last, 
            RandomAccessIterator& pivot_cut,
            const Predicate& pred,
            const unsigned int num_threads){
    
    array_tree sizes(num_threads);

    //std::cout << " before partition " << pivot << std::endl;
    element first_el;
    element last_el;

#ifdef __COUNTERS__
    int swaps_optimal = bad_placed_left(first, first + count_left(first, last, pred), pred); 
    int swaps_parallel = 0; 
#endif

    #pragma omp parallel num_threads(num_threads)
    {
        unsigned int id = omp_get_thread_num();
        typename RandomAccessIterator::difference_type num_elems = ((last-first)/num_threads)*num_threads;
        if (id < (last-first)%num_threads ) num_elems += num_threads;

    #ifdef __COUNTERS__
        iterator_with_shift<RandomAccessIterator> f = iterator_with_shift<RandomAccessIterator>(first + id, num_threads);
        #pragma omp critical 
            swaps_parallel += bad_placed_left(f, f + count_left(f, 
            iterator_with_shift<RandomAccessIterator>(first + id + num_elems, num_threads),
            pred), pred);
    #endif

        /*std::cout << "part " << id << " ";
        for (RandomAccessIterator it = first+id; it < last; it+=num_threads){
            std::cout << *it << " ";
        }
        std::cout << std::endl;
        */
    #ifndef __MCSTL__
        RandomAccessIterator cut = std::partition(
            iterator_with_shift<RandomAccessIterator>(first + id, num_threads),
            iterator_with_shift<RandomAccessIterator>(first + id + num_elems, num_threads),
            pred).base();
    #else
        RandomAccessIterator cut = std::partition(
            iterator_with_shift<RandomAccessIterator>(first + id, num_threads),
            iterator_with_shift<RandomAccessIterator>(first + id + num_elems, num_threads), 
            pred, mcstl::sequential_tag()).base();
    #endif
        sizes.size_l[sizes.i_ith_leaf(id)] = (cut - first)/num_threads;
        unsigned int i_node_orig = sizes.begin_leaves() + id;
        /*
        std::cout << " after partition " << std::endl;
        
        std::cout << "part " << id << " ";
        for (RandomAccessIterator it = first+id; it < last; it+=num_threads){
            std::cout << *it << " ";
        }
        std::cout << std::endl;
        */
        //Reduction: calculate total sizes
        reduce_tree_size_l(sizes, id);
        //std::cout << " after reduction sizes " << std::endl;
        
        //Reduction: calculate misplaced
        unsigned int total_left = sizes.size_l[sizes.i_root()];
        unsigned int this_left = total_left/num_threads;
        if (id < total_left%num_threads) ++this_left;

        if (sizes.size_l[i_node_orig] >= this_left ){
            sizes.mis_l[i_node_orig] = sizes.size_l[i_node_orig] - this_left;
            sizes.mis_r[i_node_orig] = 0;
        }
        else{
            sizes.mis_l[i_node_orig] = 0;
            sizes.mis_r[i_node_orig] = this_left - sizes.size_l[i_node_orig]; 
        }
       //std::cout << "mis " << i_node_orig << " " << sizes.mis_l[i_node_orig] << " " << sizes.mis_r[i_node_orig] << std::endl;

        reduce_and_swap(sizes, first_el, last_el, id, num_threads, iterator_wrapper<RandomAccessIterator, iterator_with_shift<RandomAccessIterator> > (first, num_threads));     
    } 
    //std::cout << " after swapping " << std::endl;
    pivot_cut = first + sizes.size_l[sizes.i_root()];
    /*
    for (RandomAccessIterator it = first; it != pivot_cut; ++it){
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    std::cout << pivot << std::endl;
    for (RandomAccessIterator it = pivot_cut; it != last; ++it){
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    */
#ifdef __COUNTERS__
    //extra swaps, extra comparisons, swaps in last phase, comparison in last phase
    std::cout << swaps_parallel + sizes.mis_l[sizes.i_root()] - swaps_optimal << " " << 0 << " " <<
        (int) sizes.mis_l[sizes.i_root()] << " " << 0 << std::endl;
#endif 
}

template<typename RandomAccessIterator, typename Predicate>
    inline void unguarded_parallel_partition_1_no_tree(
            const RandomAccessIterator first,
            const RandomAccessIterator last, 
            RandomAccessIterator& pivot_cut,
            const Predicate& pred,
            const unsigned int num_threads){
    
    std::pair<RandomAccessIterator, RandomAccessIterator> cuts[num_threads];     

#ifdef __COUNTERS__
    int swaps_optimal = bad_placed_left(first, first + count_left(first, last, pred), pred); 
    int swaps_parallel = 0; 
#endif

    #pragma omp parallel num_threads(num_threads)
    {
        unsigned int id = omp_get_thread_num();
        typename RandomAccessIterator::difference_type num_elems = ((last-first)/num_threads)*num_threads;
        if (id < (last-first)%num_threads ) num_elems += num_threads;

    #ifdef __COUNTERS__
        iterator_with_shift<RandomAccessIterator> f = iterator_with_shift<RandomAccessIterator>(first + id, num_threads);
        #pragma omp critical 
            swaps_parallel += bad_placed_left(f, f + count_left(f, 
            iterator_with_shift<RandomAccessIterator>(first + id + num_elems, num_threads),
            pred), pred);
    #endif

    #ifndef __MCSTL__
        cuts[id].first = std::partition(
            iterator_with_shift<RandomAccessIterator>(first + id, num_threads),
            iterator_with_shift<RandomAccessIterator>(first + id + num_elems, num_threads),
            pred).base();        
    #else
        cuts[id].first = std::partition(
            iterator_with_shift<RandomAccessIterator>(first + id, num_threads),
            iterator_with_shift<RandomAccessIterator>(first + id + num_elems, num_threads),
            pred, mcstl::sequential_tag()).base();
    #endif
        cuts[id].second = cuts[id].first;
        #pragma omp barrier
        
        reduce_array(cuts, num_threads, id);
    }
    //sequential partition
    if (cuts[0].second > last){
        cuts[0].second = last;
    }

#ifdef __COUNTERS__        
    int swaps_clean = bad_placed_left(cuts[0].first, cuts[0].first + count_left(cuts[0].first, cuts[0].second, pred), pred);
    //extra swaps, extra comparisons, swaps in last phase, comparison in last phase
    std::cout << swaps_parallel + swaps_clean - swaps_optimal << " " << cuts[0].second - cuts[0].first << " " <<
    swaps_clean << " " <<cuts[0].second - cuts[0].first<< std::endl;
#endif 
#ifndef __MCSTL__
    pivot_cut = std::partition(cuts[0].first, cuts[0].second, pred);  
#else
    pivot_cut = std::partition(cuts[0].first, cuts[0].second, pred, mcstl::sequential_tag());
#endif
}

template<typename RandomAccessIterator, typename Predicate>
    inline void unguarded_parallel_partition_1_blocked(
            const RandomAccessIterator first,
            const RandomAccessIterator last, 
            RandomAccessIterator& pivot_cut,
            const Predicate& pred,
            const unsigned int num_threads){
    
    array_tree sizes(num_threads);
    typename RandomAccessIterator::difference_type block_size = MIX_BARRIER_PARALLEL_block_size;
    if ((last - first)/(num_threads*block_size) < 8)
        block_size = std::min(static_cast<typename RandomAccessIterator::difference_type>(block_size/8), static_cast<typename RandomAccessIterator::difference_type>((last - first)/(num_threads*8) + 1));

#ifdef __COUNTERS__
    int swaps_optimal = bad_placed_left(first, first + count_left(first, last, pred), pred); 
    int swaps_parallel = 0; 
#endif

    //std::cout << " before partition " << pivot << " " << block_size << std::endl;
    element first_el;
    element last_el;    
#ifndef __COUNTERS__
    #pragma omp parallel num_threads(num_threads)
#else
    #pragma omp parallel num_threads(num_threads) reduction(+:swaps_parallel)
#endif
    {
        unsigned int id = omp_get_thread_num();
        typename RandomAccessIterator::difference_type num_elems = ((last-first)/(num_threads*block_size))*block_size*num_threads;
        if (id < ((last-first)%(num_threads*block_size))/block_size ){
            num_elems += block_size*num_threads;
        }
        typename RandomAccessIterator::difference_type index_last_block = 0;
        if (id == (((last-first)%(num_threads*block_size))/block_size) and ((last-first)%block_size) > 0){
            index_last_block = (last-first)%(block_size);
            num_elems += index_last_block;
        }
    #ifdef __COUNTERS__
        blocked_iterator_with_shift<RandomAccessIterator> f = blocked_iterator_with_shift<RandomAccessIterator>(first + id*block_size, num_threads, block_size);
        swaps_parallel = bad_placed_left(f, f + count_left(f, 
        blocked_iterator_with_shift<RandomAccessIterator>(first + id*block_size + num_elems, num_threads, block_size, index_last_block),
        pred), pred);
   #endif

        //std::cout << "part " << id << " ";
        /*
        for (RandomAccessIterator it = first+id*block_size; it < last; it+=(num_threads-1)*block_size){
            for (int i=0; i < block_size and it < last; ++i){
                std::cout << *it << " ";
                ++it;
            }
        }
        */
        //std::cout << std::endl;
        //std::cout << "part " << id << " " << num_elems << std::endl;
        assert(last <= first + id*block_size + num_elems);        

    #ifndef __MCSTL__
        RandomAccessIterator cut = std::partition(
            blocked_iterator_with_shift<RandomAccessIterator>(first + id*block_size, num_threads, block_size),
            blocked_iterator_with_shift<RandomAccessIterator>(first + id*block_size + num_elems, num_threads, block_size, index_last_block),
            pred).base().base(); 
    #else
        RandomAccessIterator cut = std::partition(
            blocked_iterator_with_shift<RandomAccessIterator>(first + id*block_size, num_threads, block_size),
            blocked_iterator_with_shift<RandomAccessIterator>(first + id*block_size + num_elems, num_threads, block_size, index_last_block),
            pred, mcstl::sequential_tag()).base().base(); 
    #endif 
        sizes.size_l[sizes.i_ith_leaf(id)] = ((cut - (first + id*block_size))/(num_threads*block_size))*block_size + (cut - (first + id*block_size))%block_size;
        unsigned int i_node_orig = sizes.begin_leaves() + id;
        
        //std::cout << " after partition " <<id << " " << *cut  << " " <<  cut - first << std::endl;
        
        /*std::cout << "part (a)" << id << " ";
        for (RandomAccessIterator it = first+id*block_size; it < last; it+=(num_threads-1)*block_size){
            for (int i=0; i < block_size and it < last; ++i){
                if (it == cut) std::cout << std::endl << "part (b) " << id << " ";
                std::cout << *it << " ";
                ++it;
            } 
            if (it == cut) std::cout << std::endl << "part (b) " << id << " ";
        }
        std::cout << std::endl;
        */
        //Reduction: calculate total sizes
        reduce_tree_size_l(sizes, id);
        //std::cout << " after reduction sizes " << std::endl;

        unsigned int total_left = sizes.size_l[sizes.i_root()];
        unsigned int this_left = (total_left/(num_threads*block_size))*block_size;
        if (id < total_left%(num_threads*block_size)/block_size) 
            this_left+=block_size;
        if (id == total_left%(num_threads*block_size)/block_size ){
            this_left += total_left%(block_size);
        }

        if (sizes.size_l[i_node_orig] >= this_left ){
            sizes.mis_l[i_node_orig] = sizes.size_l[i_node_orig] - this_left;
            sizes.mis_r[i_node_orig] = 0;
        }
        else{
            sizes.mis_l[i_node_orig] = 0;
            sizes.mis_r[i_node_orig] = this_left - sizes.size_l[i_node_orig]; 
        }

        reduce_and_swap(sizes, first_el, last_el, id, num_threads, iterator_wrapper<RandomAccessIterator, blocked_iterator_with_shift<RandomAccessIterator> > (first, num_threads, block_size)); 

    } 
    //std::cout << " after swapping " << std::endl;
    pivot_cut = first + sizes.size_l[sizes.i_root()];
    /*
    for (RandomAccessIterator it = first; it != pivot_cut; ++it){
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    std::cout << pivot << std::endl;
    for (RandomAccessIterator it = pivot_cut; it != last; ++it){
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    */    
 
#ifdef __COUNTERS__
    //extra swaps, extra comparisons, swaps in last phase, comparison in last phase
    std::cout << swaps_parallel + sizes.mis_l[sizes.i_root()] - swaps_optimal << " " << 0 << " " <<
        (int) sizes.mis_l[sizes.i_root()] << " " << 0 << std::endl;
#endif 
}

template<typename RandomAccessIterator, typename Predicate>
    inline void unguarded_parallel_partition_1_blocked_no_tree(
            const RandomAccessIterator first,
            const RandomAccessIterator last, 
            RandomAccessIterator& pivot_cut,
            const Predicate& pred,
            const unsigned int num_threads){

    typename RandomAccessIterator::difference_type block_size = MIX_BARRIER_PARALLEL_block_size;
    if ((last - first)/(num_threads*block_size) < 16)
        block_size = std::min(static_cast<typename RandomAccessIterator::difference_type>(block_size/16), static_cast<typename RandomAccessIterator::difference_type>((last - first)/(num_threads*16) + 1));
    
#ifdef __COUNTERS__
    int swaps_optimal = bad_placed_left(first, first + count_left(first, last, pred), pred); 
    int swaps_parallel = 0; 
#endif

    std::pair<RandomAccessIterator, RandomAccessIterator> cuts[num_threads]; 

#ifndef __COUNTERS__
    #pragma omp parallel num_threads(num_threads)
#else
    #pragma omp parallel num_threads(num_threads) reduction(+:swaps_parallel)
#endif
    {    
        unsigned int id = omp_get_thread_num();
        typename RandomAccessIterator::difference_type num_elems = ((last-first)/(num_threads*block_size))*block_size*num_threads;
        if (id < ((last-first)%(num_threads*block_size))/block_size ){
            num_elems += block_size*num_threads;
        }
        typename RandomAccessIterator::difference_type index_last_block = 0;
        if (id == (((last-first)%(num_threads*block_size))/block_size) and ((last-first)%block_size) > 0){
            index_last_block = (last-first)%(block_size);
            num_elems += index_last_block;
        }

        assert(last <= first + id*block_size + num_elems);        
 
   #ifdef __COUNTERS__
        blocked_iterator_with_shift<RandomAccessIterator> f = blocked_iterator_with_shift<RandomAccessIterator>(first + id*block_size, num_threads, block_size);
        swaps_parallel = bad_placed_left(f, f + count_left(f, 
        blocked_iterator_with_shift<RandomAccessIterator>(first + id*block_size + num_elems, num_threads, block_size, index_last_block),
        pred), pred);
    #endif

    #ifndef __MCSTL__
        cuts[id].first = std::partition(
            blocked_iterator_with_shift<RandomAccessIterator>(first + id*block_size, num_threads, block_size),
            blocked_iterator_with_shift<RandomAccessIterator>(first + id*block_size + num_elems, num_threads, block_size, index_last_block),
            pred).base().base(); 
    #else 
        cuts[id].first = std::partition(
            blocked_iterator_with_shift<RandomAccessIterator>(first + id*block_size, num_threads, block_size),
            blocked_iterator_with_shift<RandomAccessIterator>(first + id*block_size + num_elems, num_threads, block_size, index_last_block),
            pred, mcstl::sequential_tag()).base().base();
    #endif 
        cuts[id].second = cuts[id].first;
        #pragma omp barrier
        
        reduce_array(cuts, num_threads, id);
    }
    //sequential partition    
    if (cuts[0].second > last){
        cuts[0].second = last;
    }
#ifdef __COUNTERS__        
    int swaps_clean = bad_placed_left(cuts[0].first, cuts[0].first + count_left(cuts[0].first, cuts[0].second, pred), pred);
        //extra swaps, extra comparisons, swaps in last phase, comparison in last phase
    std::cout << swaps_parallel + swaps_clean - swaps_optimal << " " << cuts[0].second - cuts[0].first << " " <<
        swaps_clean << " " <<cuts[0].second - cuts[0].first<< std::endl;
#endif 
#ifndef __MCSTL__
    pivot_cut = std::partition(cuts[0].first, cuts[0].second, pred); 
#else  
    pivot_cut = std::partition(cuts[0].first, cuts[0].second, pred, mcstl::sequential_tag());
#endif   
}


template<typename RandomAccessIterator, typename Predicate>
inline unsigned int __guarded_partition_with_blocks(RandomAccessIterator this_first_block_first, RandomAccessIterator this_first_block_last, RandomAccessIterator this_first_block_cur, RandomAccessIterator this_last_block_first, RandomAccessIterator this_last_block_last, RandomAccessIterator this_last_block_cur, block_ext<RandomAccessIterator> & not_complete_blocks,  
RandomAccessIterator& first_block, RandomAccessIterator& last_block, const Predicate& pred, const unsigned int block_size){
#ifdef __COUNTERS__
    unsigned int swaps_parallel = 0;
    unsigned int incomplete;
#endif
    --this_last_block_cur;
    bool left = true, right = true;
    while(true){
        if(not left and not right){
            left = true;
            right = true;
        }
        if (this_first_block_cur >= this_first_block_last){
            #pragma omp critical (last_first_block)
            {
                if (first_block < last_block){
                    first_block += block_size;
                    this_first_block_last = first_block;
                }
            }
            if (this_first_block_cur < this_first_block_last){
                this_first_block_first = this_first_block_last - block_size;
                this_first_block_cur = this_first_block_first;
                //std::cout<<"new left block got " << this_first_block.last -this_first_block.first << std::endl;
            }
            else{
                ++this_last_block_cur;
                //std::cout<<" right unfinished block " <<  this_last_block_cur - this_last_block_first << std::endl;

                not_complete_blocks.compared = not right;
                not_complete_blocks.cur=  this_last_block_cur;     
                not_complete_blocks.first=  this_last_block_first;  
                not_complete_blocks.last=  this_last_block_last;               
                assert(check_partitioned_right(this_last_block_last, this_last_block_cur, pred));
            #ifdef __COUNTERS__
                return swaps_parallel;
            #endif
                return 0;
            }
        }
        if(this_last_block_cur < this_last_block_first){
            #pragma omp critical (last_first_block)
            {
                if (first_block < last_block){
                    last_block -= block_size;
                    this_last_block_first = last_block;
                }
            }          
            if (this_last_block_cur >= this_last_block_first){
                this_last_block_last = this_last_block_first + block_size;
                this_last_block_cur = this_last_block_last - 1;
                //std::cout<<" new right block got " << this_last_block.last -this_last_block.first << std::endl;
            }
            else{                  
                //std::cout<<" left unfinished block " <<  this_first_block_cur - this_first_block_first << std::endl;                
                not_complete_blocks.compared = not left;
                not_complete_blocks.cur = this_first_block_cur;  
                not_complete_blocks.first = this_first_block_first; 
                not_complete_blocks.last = this_first_block_last;                 
                assert(check_partitioned_left(this_first_block_first, this_first_block_cur, pred));
            #ifdef __COUNTERS__
                return swaps_parallel;
            #endif
                return 0;
            }
        }
        if (left){
            while((left = pred(*this_first_block_cur)) and this_first_block_cur < (this_first_block_last-1)){
                ++this_first_block_cur;
            }
        }
        if (right){
            while((right =  not pred(*this_last_block_cur)) and this_last_block_cur > this_last_block_first){
                --this_last_block_cur;     
            } 
        }
        if(not left and not right){
            std::swap(*this_first_block_cur, *this_last_block_cur);     
            #ifdef __COUNTERS__
            ++swaps_parallel;
            #endif
            ++this_first_block_cur;
            --this_last_block_cur;
        }
        else{
            if (left) ++this_first_block_cur;
            if (right) --this_last_block_cur;
        } 
        while(this_first_block_cur < this_first_block_last and this_last_block_cur >= this_last_block_first){
            while((left = pred(*this_first_block_cur)) and this_first_block_cur < (this_first_block_last-1)){
                ++this_first_block_cur;
            }
            while((right =  not pred(*this_last_block_cur)) and this_last_block_cur > this_last_block_first){
                --this_last_block_cur;     
            } 
            if(not left and not right){
                std::swap(*this_first_block_cur, *this_last_block_cur);     
            #ifdef __COUNTERS__
                ++swaps_parallel;
            #endif
                ++this_first_block_cur;
                --this_last_block_cur;
            }
            else{
                if (left) ++this_first_block_cur;
                if (right) --this_last_block_cur;
            }
        }

        assert(check_partitioned_left(this_first_block_first, this_first_block_cur, pred));
        assert(check_partitioned_right(this_last_block_last, this_last_block_cur+1, pred));
    }        
    return 0;
}


template<typename RandomAccessIterator, typename BlockType> 
void init_extra_blocks_left(BlockType* not_complete_blocks, const unsigned int block_size, RandomAccessIterator& last_left, unsigned int& j){
    not_complete_blocks[j].last = last_left;
    not_complete_blocks[j].cur = last_left;
    last_left -= block_size;
    not_complete_blocks[j].first = last_left;
    ++j;
}

template<typename RandomAccessIterator, typename BlockType> 
void init_extra_blocks_right(BlockType* not_complete_blocks, const unsigned int block_size, RandomAccessIterator& first_right,  unsigned int& j){
    not_complete_blocks[j].first = first_right;
    not_complete_blocks[j].cur = first_right;
    first_right += block_size;
    not_complete_blocks[j].last = first_right;
    ++j;
}




template<typename RandomAccessIterator, typename Predicate>
    inline void unguarded_parallel_partition_2(
            RandomAccessIterator first,
            RandomAccessIterator last, 
            RandomAccessIterator& pivot_cut,
            const Predicate& pred,
            unsigned int num_threads){

#ifdef __COUNTERS__
    unsigned int comps_optimal = last - first;
    unsigned int swaps_optimal = bad_placed_left(first, first + count_left(first, last, pred), pred); 
    unsigned int comps_parallel = 0; 
    unsigned int swaps_parallel = 0;
    unsigned int swaps_cleanup = 0;
#endif
    RandomAccessIterator first_first = first;

    typename RandomAccessIterator::difference_type num_elems = last - first;

    unsigned int block_size;
#ifdef __MCSTL__

    //Same criteria as the mcstl
    if(mcstl::HEURISTIC::partition_chunk_share > 0.0)
		block_size = std::max((unsigned int)mcstl::HEURISTIC::partition_chunk_size, (unsigned int)((double)num_elems * mcstl::HEURISTIC::partition_chunk_share / (double)num_threads));
    else
		block_size = mcstl::HEURISTIC::partition_chunk_size;
#else    
    block_size = FETCH_ADD_PARALLEL_block_size;
#endif

    const unsigned int min_block_size = block_size/num_threads;

    if (2*block_size*num_threads > num_elems){
        block_size = std::max(min_block_size, static_cast<unsigned int>(num_elems /(2*num_threads)));
        num_threads = static_cast<unsigned int>(num_elems /(2*block_size));
    }
        
    block_ext<RandomAccessIterator> not_complete_blocks_left[num_threads*2];
    block_ext<RandomAccessIterator> not_complete_blocks_right[num_threads*2];
    
    bool before = omp_get_nested();
    omp_set_nested(true);    

    while(num_threads > 1){

        /* At the end of each iteration, num_elems/num_threads <= block_size,
            therefore, block_size is divided at least by 2*min_factor_blocks at each new iteration, 
            guaranteeing as well, that eventually min_block_size is achieved
        */        

        const unsigned int num_blocks_full = num_elems/block_size;
        const unsigned int num_blocks = num_blocks_full  + ((num_elems%block_size == 0) ? 0 : 1);

        //iterators to advance between the blocks
        RandomAccessIterator first_block = first + num_threads*block_size;
        //The uncompleted block, if exists, is at the end
        RandomAccessIterator last_block = first + (num_blocks - num_threads)*block_size;    

        unsigned int swaps_parallel_l = 0;
    #ifndef __COUNTERS__
        #pragma omp parallel num_threads(num_threads) 
    #else
        unsigned int swapped_left = 0; 
        unsigned int swapped_right = 0;
        
        #pragma omp parallel num_threads(num_threads) reduction(+:swaps_parallel_l)
    #endif
        {
            unsigned int id = omp_get_thread_num(); 
            //std::cout<<"thread "<< id << " " << num_threads << " " << block_size << " " << num_blocks << std::endl;
            //At least there are num_threads*2 full blocks
            RandomAccessIterator this_f = first + id*block_size;
            RandomAccessIterator this_l = first + (num_blocks - id)*block_size;
            RandomAccessIterator this_last_r = std::min(this_l, last);
            
            //std::cout<<"before partition"<<std::endl;
            /* Partition */
            swaps_parallel_l = __guarded_partition_with_blocks(this_f, this_f + block_size, this_f, this_l - block_size, this_last_r, this_last_r, not_complete_blocks_left[id], first_block, last_block, pred, block_size);          
            //std::cout<<"after partition"<<std::endl;
        }
            
    
    /* Construction of the trees with the accumulated sizes of the blocks
            - We have a tree for the uncompleted left_blocks and another for the uncompleted right blocks
            - For the left blocks, the unprocessed elements make the role of the right elements
            - For the right blocks, the unprocessed elements make the role of the left elements
    */
    /*  One thread for each side (left or right) must take care of:
            - Sorting the blocks, so that extra blocks to be allocated can be identified efficiently
            - Initialize the extra blocks whose elements are already partitioned but are badly placed.
            - Then, we build the corresponding tree whose number of leaves is equivalent to the number of blocks
    */       

    /* Calculation of the leaves values (the threads get tasks according to their id, not related with the last block they have touched). Each thread: 
            - Calculates the values of one proper not complete leaf
            - Calculates the values of zero or one complete leaves but badly placed
        
    */

    /* Get the sum of the values on the trees leaves and then, swap accordingly the elements */

    #ifndef __MCSTL__
        std::sort(not_complete_blocks_left, not_complete_blocks_left + num_threads);
    #else
        std::sort(not_complete_blocks_left, not_complete_blocks_left + num_threads, mcstl::sequential_tag());
    #endif
        block_ext<RandomAccessIterator>* first_right = std::lower_bound(not_complete_blocks_left, not_complete_blocks_left + num_threads, first_block);
    
    
        //initialize left and right blocks
        unsigned int num_unfinished_blocks_left = first_right - not_complete_blocks_left;
        unsigned int num_unfinished_blocks_right = num_threads - num_unfinished_blocks_left;
        /* Move the last elements that have been compared in the main loop to the correct side 
            Step 1: couple of elements */
        int i_right = num_threads - 1;
        int i_left = 0;
        while (i_left < int(num_unfinished_blocks_left) and not not_complete_blocks_left[i_left].compared) {
            ++i_left;
        }
        while (i_right >= int(num_unfinished_blocks_left) and not not_complete_blocks_left[i_right].compared){                                --i_right; 
        }
        while(i_left < int(num_unfinished_blocks_left) and i_right >= int(num_unfinished_blocks_left)){
            --not_complete_blocks_left[i_right].cur;
            std::swap(*not_complete_blocks_left[i_left].cur, *not_complete_blocks_left[i_right].cur);
            ++not_complete_blocks_left[i_left].cur;
            do{
               ++i_left; 
            }while (i_left < int(num_unfinished_blocks_left) and not not_complete_blocks_left[i_left].compared);
            do{
              --i_right;  
            }while (i_right >= int(num_unfinished_blocks_left) and not not_complete_blocks_left[i_right].compared);
        }


        //else, all the elements are already in place
        std::copy(not_complete_blocks_left + num_unfinished_blocks_left, not_complete_blocks_left + num_threads, not_complete_blocks_right);
        
        RandomAccessIterator first_unprocessed = first_block;
        RandomAccessIterator last_unprocessed = first_unprocessed;
        array_tree sizes_left;
        array_tree sizes_right;
        //parallel forces flush
        #pragma omp parallel sections num_threads(2)
        {
            #pragma omp section 
            {
                if (num_unfinished_blocks_left > 0){
                    //Find the extra blocks with which exchange elements
                    RandomAccessIterator last_left = first_block;
                    unsigned int num_blocks_left = num_unfinished_blocks_left;
                    //unperfect block
                    unsigned int last_block_size = (last_left - not_complete_blocks_left[num_blocks_left-1].last)%block_size;
                    if ( last_block_size != 0){
                        init_extra_blocks_left(not_complete_blocks_left, last_block_size, last_left, num_blocks_left); 
                        //std::cout <<  "imperfect block left" << std::endl;
                    }
                    else{
                        last_block_size = not_complete_blocks_left[num_blocks_left-1].last - not_complete_blocks_left[num_blocks_left-1].first;
                    }
                    //The number of iterations is not worst than num_threads
                    for(int i = num_unfinished_blocks_left - 1; i>=0; --i){
                        while (last_left >  not_complete_blocks_left[i].last and num_blocks_left - (i+1) < num_unfinished_blocks_left){
                            init_extra_blocks_left(not_complete_blocks_left, block_size, last_left, num_blocks_left);
                        }
                        if (num_blocks_left - (i + 1) == num_unfinished_blocks_left) break;
                        last_left = not_complete_blocks_left[i].first;
                        //std::cout << "i left " << i << std::endl;
                    }
                    sizes_left.init(num_blocks_left);

                    if (num_blocks_left == 1){
                        first_unprocessed = not_complete_blocks_left[0].cur;
                    }
                    else{
                        //std::cout << "left branch" << std::endl;                              
                        element first_el, last_el;
                    #ifndef __COUNTERS__
                        #pragma omp parallel num_threads(num_blocks_left)
                    #else    
                        #pragma omp parallel num_threads(num_blocks_left) reduction(+:swapped_left)
                    #endif
                        {
                            unsigned int id = omp_get_thread_num();
                            //Proper not complete leaf
                            if (id  < num_unfinished_blocks_left){
                                sizes_left.size_l[sizes_left.i_ith_leaf(id)] =  not_complete_blocks_left[id].cur - not_complete_blocks_left[id].first; 
                            }
                            else{
                                sizes_left.size_l[sizes_left.i_ith_leaf(id)] =  not_complete_blocks_left[id].last - not_complete_blocks_left[id].first;
                                sizes_left.mis_r[sizes_left.i_ith_leaf(id)] = 0;
                            }

                            reduce_tree_size_l(sizes_left, id); 
                            // Calculate the total number of elements in the left: the last block can be of not full size 
                            unsigned int total_elements_left = (num_blocks_left - 1)*block_size + last_block_size;
                            unsigned int processed = sizes_left.size_l[sizes_left.i_root()];
                            first_unprocessed = first_block - (total_elements_left - processed);
                            

                            if (id  < num_unfinished_blocks_left){
                                if (not_complete_blocks_left[id].cur < first_unprocessed){
                                    sizes_left.mis_l[sizes_left.i_ith_leaf(id)] =  0;
                                    if (not_complete_blocks_left[id].last <= first_unprocessed)
                                        sizes_left.mis_r[sizes_left.i_ith_leaf(id)] = not_complete_blocks_left[id].last - not_complete_blocks_left[id].cur;
                                    else
                                        sizes_left.mis_r[sizes_left.i_ith_leaf(id)] = first_unprocessed - not_complete_blocks_left[id].cur; 
                                }
                                else{
                                    sizes_left.mis_r[sizes_left.i_ith_leaf(id)] =  0;
                                    if (not_complete_blocks_left[id].first >= first_unprocessed)
                                        sizes_left.mis_l[sizes_left.i_ith_leaf(id)] = sizes_left.size_l[sizes_left.i_ith_leaf(id)];
                                    else
                                        sizes_left.mis_l[sizes_left.i_ith_leaf(id)] =  not_complete_blocks_left[id].cur - first_unprocessed;
                                }
                            }
                            //Complete leaf but badly placed
                            else{
                                if (not_complete_blocks_left[id].first < first_unprocessed){
                                    if (not_complete_blocks_left[id].last > first_unprocessed){
                                        sizes_left.mis_l[sizes_left.i_ith_leaf(id)] = not_complete_blocks_left[id].last - first_unprocessed;
                                    }
                                    else
                                        sizes_left.mis_l[sizes_left.i_ith_leaf(id)] = 0;
                                }
                                else{
                                    sizes_left.mis_l[sizes_left.i_ith_leaf(id)] = not_complete_blocks_left[id].last - not_complete_blocks_left[id].first;
                                }
                            }
                        #ifdef __COUNTERS__
                            unsigned int sw_left = reduce_and_swap(sizes_left, first_el, last_el, id, num_blocks_left, 
                            iterator_wrapper<block_ext<RandomAccessIterator>, RandomAccessIterator> (not_complete_blocks_left));
                            swapped_left += sw_left;
                        #else
                            reduce_and_swap(sizes_left, first_el, last_el, id, num_blocks_left, 
                            iterator_wrapper<block_ext<RandomAccessIterator>, RandomAccessIterator> (not_complete_blocks_left));
                        #endif
                        }
                    }
                }
                first = first_unprocessed;        
            }
            #pragma omp section
            {   
                if (num_unfinished_blocks_right > 0){
                    RandomAccessIterator first_right = first_block;
                    unsigned int num_blocks_right = num_unfinished_blocks_right;
                    //unperfect block
                    assert(not_complete_blocks_right[0].first >= first_right);
                    unsigned int first_block_size = (not_complete_blocks_right[0].first - first_right)%block_size; 
                    if (first_block_size != 0){
                        init_extra_blocks_right(not_complete_blocks_right, first_block_size, first_right, num_blocks_right);
                        //std::cout <<  "imperfect block right" << first_block_size <<  std::endl;
                    } 
                    //The extra condition first_right < last_unprocessed is needed because new elements (smaller) are added at the end
                    // Alternatively, we could leave extra space at the beginning, initializing first component to global first
                    for(unsigned int i = 0; i < num_unfinished_blocks_right; ++i){
                        while (first_right < not_complete_blocks_right[i].first and (num_blocks_right + i) < 2*num_unfinished_blocks_right){  
                            init_extra_blocks_right(not_complete_blocks_right, block_size, first_right, num_blocks_right);
                             //std::cout << "added " <<  not_complete_blocks_right[num_blocks_right-1].first - first_block <<  std::endl;
                        }
                        if(num_blocks_right + i == 2*num_unfinished_blocks_right)
                            break;
                        first_right = not_complete_blocks_right[i].last;
                        //std::cout << "i right " << i << not_complete_blocks_right[i].first - first_block <<  std::endl;
                    }
                    sizes_right.init(num_blocks_right);

                    if (num_blocks_right == 1){
                        last_unprocessed = not_complete_blocks_right[0].cur;
                    }
                    else{
                        //std::cout << "right branch " << num_blocks_right << std::endl;
                        element first_el, last_el;
                    #ifndef __COUNTERS__
                        #pragma omp parallel num_threads(num_blocks_right)
                    #else
                        #pragma omp parallel num_threads(num_blocks_right) reduction(+:swapped_right)
                    #endif
                        {
                            unsigned int id = omp_get_thread_num();
                            //Proper not complete leaf
                            if (id  < num_unfinished_blocks_right){
                                sizes_right.size_l[sizes_right.i_ith_leaf(id)] = (not_complete_blocks_right[id].cur - not_complete_blocks_right[id].first);
                            }
                            else{
                            //Complete leaf but badly placed
                                sizes_right.size_l[sizes_right.i_ith_leaf(id)] = 0;
                                sizes_right.mis_l[sizes_right.i_ith_leaf(id)] = 0;
                            }
                            reduce_tree_size_l(sizes_right, id); 
                            // Calculate the total number of elements in the right: the first block can be of not full size 
                            unsigned int unprocessed = sizes_right.size_l[sizes_right.i_root()];
                            last_unprocessed = first_block + unprocessed;        

                            if (id  < num_unfinished_blocks_right){
                                //right blocks
                                if (not_complete_blocks_right[id].cur >= last_unprocessed){
                                    sizes_right.mis_r[sizes_right.i_ith_leaf(id)] =  0;
                                    if (not_complete_blocks_right[id].first >= last_unprocessed)
                                        sizes_right.mis_l[sizes_right.i_ith_leaf(id)] = not_complete_blocks_right[id].cur - not_complete_blocks_right[id].first;
                                    else
                                        sizes_right.mis_l[sizes_right.i_ith_leaf(id)] = not_complete_blocks_right[id].cur - last_unprocessed;
                                }        
                                else{
                                    sizes_right.mis_l[sizes_right.i_ith_leaf(id)] =  0;
                                    if (not_complete_blocks_right[id].last <= last_unprocessed)
                                        sizes_right.mis_r[sizes_right.i_ith_leaf(id)] = not_complete_blocks_right[id].last - not_complete_blocks_right[id].cur; 
                                    else
                                        sizes_right.mis_r[sizes_right.i_ith_leaf(id)] = last_unprocessed - not_complete_blocks_right[id].cur;
                                }           
                            }
                            else{
                                if (not_complete_blocks_right[id].last < last_unprocessed){
                                    sizes_right.mis_r[sizes_right.i_ith_leaf(id)] = not_complete_blocks_right[id].last - not_complete_blocks_right[id].first;
                                }
                                else{
                                    if(not_complete_blocks_right[id].first < last_unprocessed){
                                        sizes_right.mis_r[sizes_right.i_ith_leaf(id)] = last_unprocessed - not_complete_blocks_right[id].first;
                                    }
                                    else{
                                        sizes_right.mis_r[sizes_right.i_ith_leaf(id)] = 0;
                                    }
                                }
                            }                            
                        #ifdef __COUNTERS__
                            unsigned int sw_right = reduce_and_swap(sizes_right, first_el, last_el, id, num_blocks_right, iterator_wrapper<block_ext<RandomAccessIterator>, RandomAccessIterator> (not_complete_blocks_right)); 
                            swapped_right += sw_right;
                        #else 
                            reduce_and_swap(sizes_right, first_el, last_el, id, num_blocks_right, iterator_wrapper<block_ext<RandomAccessIterator>, RandomAccessIterator> (not_complete_blocks_right)); 
                        #endif
                        }   
                    }
                }
                last = last_unprocessed;
            }
        }

        /* Move the last elements that have been compared in the main loop to the correct side
            Step 2: not-coupled elements (ith misplaced element from the left must be swapped with the ith misplaced */
        
        if (i_left < int(num_unfinished_blocks_left)){
            int orig_i_left = i_left;
            do { 
                if (not_complete_blocks_left[i_left].cur < first){ //the element has been moved;
                    unsigned int rank = sizes_left.rank(sizes_left.mis_r,  sizes_left.i_ith_leaf(i_left));
                    unsigned int inverse_rank = sizes_left.mis_r[0] - rank + 1;
                    element e;
                    sizes_left.ith(inverse_rank, sizes_left.i_root(), sizes_left.mis_l, e);
                    e.i += (sizes_left.size_l[e.i_node] - sizes_left.mis_l[e.i_node]);   
                    unsigned int i_block = sizes_left.i_rank_leaf(e.i_node);
                    not_complete_blocks_left[i_left].cur = not_complete_blocks_left[i_block].first + e.i;
                    //pred.swap(not_complete_blocks_left[i_block].first + e.i, last);
                }
                do{
                     ++i_left;                    
                } while (i_left < int(num_unfinished_blocks_left) and not not_complete_blocks_left[i_left].compared);
            } while  (i_left < int(num_unfinished_blocks_left));            
            // In not_complete_blocks_left[i_left].cur are the positions with left elements    
        #ifndef __MCSTL__
            std::sort(not_complete_blocks_left + orig_i_left, not_complete_blocks_left + num_unfinished_blocks_left);
        #else
            std::sort(not_complete_blocks_left + orig_i_left, not_complete_blocks_left + num_unfinished_blocks_left, mcstl::sequential_tag());
        #endif
            do{
                --i_left;                    
            } while (i_left >= orig_i_left and not not_complete_blocks_left[i_left].compared);
            while (i_left >= orig_i_left){
                --last;
                std::swap(*not_complete_blocks_left[i_left].cur, *last);
                do{
                    --i_left;                    
                } while (i_left >= orig_i_left and not not_complete_blocks_left[i_left].compared);
            }
        }
        else if (i_right >= int(num_unfinished_blocks_left)){
            i_right -= num_unfinished_blocks_left;
            int orig_i_right = i_right;
            do {               
                if (not_complete_blocks_right[i_right].cur <= last){ // the element has not been moved by reduce and swap
                    --not_complete_blocks_right[i_right].cur;
                    //pred.swap(not_complete_blocks_right[i_right].cur - 1, first);
                }
                else{ //the element has been moved;
                    unsigned int leaf = sizes_right.i_ith_leaf(i_right);
                    unsigned int rank = sizes_right.rank(sizes_right.mis_l, leaf) + sizes_right.mis_l[leaf] - 1;
                    unsigned int inverse_rank = sizes_right.mis_l[0] + 1 - rank;
                     element e;  
                    sizes_right.ith(inverse_rank, sizes_right.i_root(), sizes_right.mis_r, e);
                    e.i += sizes_right.size_l[e.i_node];
                    unsigned int i_block = sizes_right.i_rank_leaf(e.i_node);
                    not_complete_blocks_right[i_right].cur = not_complete_blocks_right[i_block].first + e.i;
                }
                do{
                    --i_right;
                }while (i_right >= 0 and not not_complete_blocks_right[i_right].compared);
            } while (i_right >= 0);
            // In not_complete_blocks_right[i_right].cur are the positions with left elements    
        #ifndef __MCSTL__
            std::sort(not_complete_blocks_right, not_complete_blocks_right + orig_i_right);        
        #else
            std::sort(not_complete_blocks_right, not_complete_blocks_right + orig_i_right, mcstl::sequential_tag()); 
        #endif 
            do{
                ++i_right;
            }while (i_right <= orig_i_right and not not_complete_blocks_right[i_right].compared);
            while (i_right <= orig_i_right){
                std::swap(*not_complete_blocks_right[i_right].cur, *first);
                ++first;
                do{
                    ++i_right;
                }while (i_right <= orig_i_right and not not_complete_blocks_right[i_right].compared);
            } 

        }

        //unguarded_parallel_partition_2(first_unprocessed, last_unprocessed, pivot_cut, pred, num_threads);
        /* Prepare next iteration */        
    #ifdef __COUNTERS__
        swaps_parallel += swaps_parallel_l;
        swaps_cleanup += swapped_left + swapped_right;   
        comps_parallel += num_elems - (last - first);        
    #endif
        num_elems = last - first;      
        block_size = std::max(min_block_size, block_size/2);
        num_threads = num_elems /(2*block_size); 
    }          
    omp_set_nested(before);        

#ifdef __COUNTERS__
    comps_parallel += last - first;
    swaps_parallel += bad_placed_left(first, first + count_left(first, last, pred), pred); 
#endif
#ifndef __MCSTL__
    pivot_cut = std::partition(first, last, pred);
#else
    pivot_cut = std::partition(first, last, pred, mcstl::sequential_tag());
#endif    
#ifdef __COUNTERS__
    std::cout << swaps_cleanup + swaps_parallel - swaps_optimal << " " << comps_parallel - comps_optimal << " " << swaps_cleanup << " " << 0 << std::endl;
#endif

}   


template<typename RandomAccessIterator, typename Predicate>
    inline void unguarded_parallel_partition_2_mcstl(
            RandomAccessIterator first,
            RandomAccessIterator last, 
            RandomAccessIterator& pivot_cut,
            const Predicate& pred,
            unsigned int num_threads){

#ifdef __COUNTERS__
    unsigned int comps_optimal = last - first;
    unsigned int swaps_optimal = bad_placed_left(first, first + count_left(first, last, pred), pred); 
    unsigned int comps_parallel = 0; 
    unsigned int swaps_parallel = 0;
    unsigned int swaps_cleanup = 0;
#endif

    typename RandomAccessIterator::difference_type num_elems = last - first;
    unsigned int block_size;
#ifdef __MCSTL__

    //Same criteria as the mcstl
    if(mcstl::HEURISTIC::partition_chunk_share > 0.0)
        block_size = std::max((unsigned int)mcstl::HEURISTIC::partition_chunk_size, (unsigned int)((double)num_elems * mcstl::HEURISTIC::partition_chunk_share / (double)num_threads));
    else
        block_size = mcstl::HEURISTIC::partition_chunk_size;
#else    
    block_size = FETCH_ADD_PARALLEL_block_size;
#endif

    const unsigned int min_block_size = block_size/num_threads;

    if (2*block_size*num_threads > num_elems){
        block_size = std::max(min_block_size, static_cast<unsigned int>(num_elems /(2*num_threads)));
        num_threads = static_cast<unsigned int>(num_elems /(2*block_size));
    }
        
    block<RandomAccessIterator> not_complete_blocks_left[num_threads*2];
    block<RandomAccessIterator> not_complete_blocks_right[num_threads*2];
    
    bool before = omp_get_nested();
    omp_set_nested(true);    

    while(num_threads > 1){

        /* At the end of each iteration, num_elems/num_threads <= block_size,
            therefore, block_size is divided at least by 2*min_factor_blocks at each new iteration, 
            guaranteeing as well, that eventually min_block_size is achieved
        */        

        const unsigned int num_blocks_full = num_elems/block_size;
        const unsigned int num_blocks = num_blocks_full  + ((num_elems%block_size == 0) ? 0 : 1);

        typedef typename std::iterator_traits<RandomAccessIterator>::difference_type
    DiffType;
       
        volatile DiffType first_diff = 0;
        volatile DiffType last_diff =  num_elems - 1;

        unsigned int swaps_parallel_l = 0;
        unsigned int comps_parallel_l = 0;
    #ifndef __COUNTERS__
        #pragma omp parallel num_threads(num_threads) 
    #else
        unsigned int swapped_left = 0; 
        unsigned int swapped_right = 0;
        
        #pragma omp parallel num_threads(num_threads) reduction(+:swaps_parallel_l, comps_parallel_l)
    #endif
        {
            unsigned int id = omp_get_thread_num(); 
            //std::cout<<"thread "<< id << " " << num_threads << " " << block_size << " " << num_blocks << std::endl;
            //At least there are num_threads*2 full blocks
            /* Partition */
            DiffType thread_left = DiffType(first_diff) + 1;
            DiffType thread_left_border = thread_left - 1;   //just to satify the condition below
            DiffType thread_right = DiffType(last_diff);
            DiffType thread_right_border = thread_right + 1; //just to satify the condition below

            swaps_parallel_l = __guarded_partition_with_blocks_mcstl(first, thread_left, thread_left_border, thread_right, thread_right_border, first_diff, last_diff, comps_parallel_l, pred, block_size);
            
            if(thread_left > thread_left_border){
                not_complete_blocks_left[id].first = first + thread_right_border;
                not_complete_blocks_left[id].cur = first + thread_right + 1;                    
                not_complete_blocks_left[id].last = first + thread_right_border + block_size;
            }
            else{
                not_complete_blocks_left[id].first =  first + thread_left_border - block_size + 1;
                not_complete_blocks_left[id].cur =   first + thread_left;                  
                not_complete_blocks_left[id].last =   first + thread_left_border + 1;
            }
        }
        //iterators to advance between the blocks
        RandomAccessIterator first_block = first + DiffType(first_diff);
        //The uncompleted block, if exists, is at the end
        RandomAccessIterator last_block = first + DiffType(last_diff) + 1;
    
    /* Construction of the trees with the accumulated sizes of the blocks
            - We have a tree for the uncompleted left_blocks and another for the uncompleted right blocks
            - For the left blocks, the unprocessed elements make the role of the right elements
            - For the right blocks, the unprocessed elements make the role of the left elements
    */
    /*  One thread for each side (left or right) must take care of:
            - Sorting the blocks, so that extra blocks to be allocated can be identified efficiently
            - Initialize the extra blocks whose elements are already partitioned but are badly placed.
            - Then, we build the corresponding tree whose number of leaves is equivalent to the number of blocks
    */       

    /* Calculation of the leaves values (the threads get tasks according to their id, not related with the last block they have touched). Each thread: 
            - Calculates the values of one proper not complete leaf
            - Calculates the values of zero or one complete leaves but badly placed
        
    */

    /* Get the sum of the values on the trees leaves and then, swap accordingly the elements */

    #ifndef __MCSTL__
        std::sort(not_complete_blocks_left, not_complete_blocks_left + num_threads);
    #else
        std::sort(not_complete_blocks_left, not_complete_blocks_left + num_threads, mcstl::sequential_tag());
    #endif
        block<RandomAccessIterator>* first_right_b = std::lower_bound(not_complete_blocks_left, not_complete_blocks_left + num_threads, last_block);
    
    
        //initialize left and right blocks
        unsigned int num_unfinished_blocks_left = first_right_b - not_complete_blocks_left;
        unsigned int num_unfinished_blocks_right = num_threads - num_unfinished_blocks_left;
        std::copy(not_complete_blocks_left + num_unfinished_blocks_left, not_complete_blocks_left + num_threads, not_complete_blocks_right);
        
        RandomAccessIterator first_unprocessed = first_block;
        RandomAccessIterator last_unprocessed = last_block;


        //#pragma omp parallel sections num_threads(2)
        #pragma omp sections
        {
            #pragma omp section 
            {
                if (num_unfinished_blocks_left > 0){
                    //Find the extra blocks with which exchange elements
                    RandomAccessIterator last_left = first_block;
                    unsigned int num_blocks_left = num_unfinished_blocks_left;
                    //unperfect block
                    unsigned int last_block_size = (last_left - not_complete_blocks_left[num_blocks_left-1].last)%block_size;
                    if ( last_block_size != 0){
                        init_extra_blocks_left(not_complete_blocks_left, last_block_size, last_left, num_blocks_left); 
                        //std::cout <<  "imperfect block left" << std::endl;
                    }
                    else{
                        last_block_size = not_complete_blocks_left[num_blocks_left-1].last - not_complete_blocks_left[num_blocks_left-1].first;
                    }
                    //The number of iterations is not worst than num_threads
                    for(int i = num_unfinished_blocks_left - 1; i>=0; --i){
                        while (last_left >  not_complete_blocks_left[i].last and num_blocks_left - (i+1) < num_unfinished_blocks_left){
                            init_extra_blocks_left(not_complete_blocks_left, block_size, last_left, num_blocks_left);
                        }
                        if (num_blocks_left - (i + 1) == num_unfinished_blocks_left) break;
                        last_left = not_complete_blocks_left[i].first;
                        //std::cout << "i left " << i << std::endl;
                    }
                    array_tree sizes_left(num_blocks_left);

                    if (num_blocks_left == 1){
                        first_unprocessed = not_complete_blocks_left[0].cur;
                    }
                    else{
                        //std::cout << "left branch" << std::endl;                              
                        element first_el, last_el;
                    #ifndef __COUNTERS__
                        #pragma omp parallel num_threads(num_blocks_left)
                    #else    
                        #pragma omp parallel num_threads(num_blocks_left) reduction(+:swapped_left)
                    #endif
                        { 
                            unsigned int id = omp_get_thread_num();
                            //Proper not complete leaf
                            if (id  < num_unfinished_blocks_left){
                                sizes_left.size_l[sizes_left.i_ith_leaf(id)] =  not_complete_blocks_left[id].cur - not_complete_blocks_left[id].first; 
                            }
                            else{
                                sizes_left.size_l[sizes_left.i_ith_leaf(id)] =  not_complete_blocks_left[id].last - not_complete_blocks_left[id].first;
                                sizes_left.mis_r[sizes_left.i_ith_leaf(id)] = 0;
                            }
  
                            reduce_tree_size_l(sizes_left, id); 
                            // Calculate the total number of elements in the left: the last block can be of not full size 
                            unsigned int total_elements_left = (num_blocks_left - 1)*block_size + last_block_size;
                            unsigned int processed = sizes_left.size_l[sizes_left.i_root()];
                            first_unprocessed = first_block - (total_elements_left - processed);

                            if (id  < num_unfinished_blocks_left){
                                if (not_complete_blocks_left[id].cur < first_unprocessed){
                                    sizes_left.mis_l[sizes_left.i_ith_leaf(id)] =  0;
                                    if (not_complete_blocks_left[id].last <= first_unprocessed)
                                        sizes_left.mis_r[sizes_left.i_ith_leaf(id)] = not_complete_blocks_left[id].last - not_complete_blocks_left[id].cur;
                                    else
                                        sizes_left.mis_r[sizes_left.i_ith_leaf(id)] = first_unprocessed - not_complete_blocks_left[id].cur; 
                                }
                                else{
                                    sizes_left.mis_r[sizes_left.i_ith_leaf(id)] =  0;
                                    if (not_complete_blocks_left[id].first >= first_unprocessed)
                                        sizes_left.mis_l[sizes_left.i_ith_leaf(id)] = sizes_left.size_l[sizes_left.i_ith_leaf(id)];
                                    else
                                        sizes_left.mis_l[sizes_left.i_ith_leaf(id)] =  not_complete_blocks_left[id].cur - first_unprocessed;
                                }
                            }
                            //Complete leaf but badly placed
                            else{
                                if (not_complete_blocks_left[id].first < first_unprocessed){
                                    if (not_complete_blocks_left[id].last > first_unprocessed){
                                        sizes_left.mis_l[sizes_left.i_ith_leaf(id)] = not_complete_blocks_left[id].last - first_unprocessed;
                                    }
                                    else
                                        sizes_left.mis_l[sizes_left.i_ith_leaf(id)] = 0;
                                }
                                else{
                                    sizes_left.mis_l[sizes_left.i_ith_leaf(id)] = not_complete_blocks_left[id].last - not_complete_blocks_left[id].first;
                                }
                            }
                            unsigned int sw_left = reduce_and_swap(sizes_left, first_el, last_el, id, num_blocks_left, 
                            iterator_wrapper<block<RandomAccessIterator>, RandomAccessIterator> (not_complete_blocks_left));
                        #ifdef __COUNTERS__
                            swapped_left += sw_left;
                        #endif
                        }
                    }
                }
                first = first_unprocessed;        
            }
            #pragma omp section
            {    
                if (num_unfinished_blocks_right > 0){
                    RandomAccessIterator first_right = last_block;
                    unsigned int num_blocks_right = num_unfinished_blocks_right;
                    //unperfect block
                    assert(not_complete_blocks_right[0].first >= first_right);
                    unsigned int first_block_size = (not_complete_blocks_right[0].first - first_right)%block_size; 
                    if (first_block_size != 0){
                        init_extra_blocks_right(not_complete_blocks_right, first_block_size, first_right, num_blocks_right);
                        //std::cout <<  "imperfect block right" << first_block_size <<  std::endl;
                    } 
                    //The extra condition first_right < last_unprocessed is needed because new elements (smaller) are added at the end
                    // Alternatively, we could leave extra space at the beginning, initializing first component to global first
                    for(int i = 0; i < num_unfinished_blocks_right; ++i){
                        while (first_right < not_complete_blocks_right[i].first and (num_blocks_right + i) < 2*num_unfinished_blocks_right){  
                            init_extra_blocks_right(not_complete_blocks_right, block_size, first_right, num_blocks_right);
                             //std::cout << "added " <<  not_complete_blocks_right[num_blocks_right-1].first - first_block <<  std::endl;
                        }
                        if(num_blocks_right + i == 2*num_unfinished_blocks_right)
                            break;
                        first_right = not_complete_blocks_right[i].last;
                        //std::cout << "i right " << i << not_complete_blocks_right[i].first - first_block <<  std::endl;
                    }
                    array_tree sizes_right(num_blocks_right);

                    if (num_blocks_right == 1){
                        last_unprocessed = not_complete_blocks_right[0].cur;
                    }
                    else{
                        //std::cout << "right branch " << num_blocks_right << std::endl;
                        element first_el, last_el;
                    #ifndef __COUNTERS__
                        #pragma omp parallel num_threads(num_blocks_right)
                    #else
                        #pragma omp parallel num_threads(num_blocks_right) reduction(+:swapped_right)
                    #endif
                        {
                            unsigned int id = omp_get_thread_num();
                            //Proper not complete leaf
                            if (id  < num_unfinished_blocks_right){
                                sizes_right.size_l[sizes_right.i_ith_leaf(id)] = (not_complete_blocks_right[id].cur - not_complete_blocks_right[id].first);
                            }
                            else{
                            //Complete leaf but badly placed
                                sizes_right.size_l[sizes_right.i_ith_leaf(id)] = 0;
                                sizes_right.mis_l[sizes_right.i_ith_leaf(id)] = 0;
                            }
                            reduce_tree_size_l(sizes_right, id); 
                            // Calculate the total number of elements in the right: the first block can be of not full size 
                            unsigned int unprocessed = sizes_right.size_l[sizes_right.i_root()];
                            last_unprocessed = last_block + unprocessed;        

                            if (id  < num_unfinished_blocks_right){
                                //right blocks
                                if (not_complete_blocks_right[id].cur >= last_unprocessed){
                                    sizes_right.mis_r[sizes_right.i_ith_leaf(id)] =  0;
                                    if (not_complete_blocks_right[id].first >= last_unprocessed)
                                        sizes_right.mis_l[sizes_right.i_ith_leaf(id)] = not_complete_blocks_right[id].cur - not_complete_blocks_right[id].first;
                                    else
                                        sizes_right.mis_l[sizes_right.i_ith_leaf(id)] = not_complete_blocks_right[id].cur - last_unprocessed;
                                }        
                                else{
                                    sizes_right.mis_l[sizes_right.i_ith_leaf(id)] =  0;
                                    if (not_complete_blocks_right[id].last <= last_unprocessed)
                                        sizes_right.mis_r[sizes_right.i_ith_leaf(id)] = not_complete_blocks_right[id].last - not_complete_blocks_right[id].cur; 
                                    else
                                        sizes_right.mis_r[sizes_right.i_ith_leaf(id)] = last_unprocessed - not_complete_blocks_right[id].cur;
                                }           
                            }
                            else{
                                if (not_complete_blocks_right[id].last < last_unprocessed){
                                    sizes_right.mis_r[sizes_right.i_ith_leaf(id)] = not_complete_blocks_right[id].last - not_complete_blocks_right[id].first;
                                }
                                else{
                                    if(not_complete_blocks_right[id].first < last_unprocessed){
                                        sizes_right.mis_r[sizes_right.i_ith_leaf(id)] = last_unprocessed - not_complete_blocks_right[id].first;
                                    }
                                    else{
                                        sizes_right.mis_r[sizes_right.i_ith_leaf(id)] = 0;
                                    }
                                }
                            }                            
                            unsigned int sw_right = reduce_and_swap(sizes_right, first_el, last_el, id, num_blocks_right, iterator_wrapper<block<RandomAccessIterator>, RandomAccessIterator> (not_complete_blocks_right));                            
                        #ifdef __COUNTERS__
                            swapped_right += sw_right;
                        #endif
                        }   
                    }
                }
                last = last_unprocessed;
            }
        }
        //unguarded_parallel_partition_2(first_unprocessed, last_unprocessed, pivot_cut, pred, num_threads);
        /* Prepare next iteration */        
    #ifdef __COUNTERS__
        swaps_parallel += swaps_parallel_l;
        swaps_cleanup += swapped_left + swapped_right;   
        comps_parallel += comps_parallel_l;        
    #endif
        num_elems = last - first;      
        block_size = std::max(min_block_size, block_size/2);
        num_threads = num_elems /(2*block_size); 
    }          
    omp_set_nested(before);        

#ifdef __COUNTERS__
    comps_parallel += last - first;
    swaps_parallel += bad_placed_left(first, first + count_left(first, last, pred), pred); 
#endif
#ifndef __MCSTL__
    pivot_cut = std::partition(first, last, pred);
#else
    pivot_cut = std::partition(first, last, pred, mcstl::sequential_tag());
#endif    
#ifdef __COUNTERS__
    std::cout << swaps_cleanup + swaps_parallel - swaps_optimal << " " << comps_parallel - comps_optimal << " " << swaps_cleanup << " " << 0 << std::endl;
#endif

}   

/* DEBUG */
/* Begin */
template<typename RandomAccessIterator, typename Predicate>
bool check_partitioned_left(RandomAccessIterator begin, RandomAccessIterator part, const Predicate& pred){
    bool is_partitioned = true;
    for (RandomAccessIterator it = begin; it < part and is_partitioned; ++it){
        if (not pred(*it)){
            is_partitioned = false;
            std::cout << "false left " <<  part - begin << " " << it - begin << std::endl;
        }
    }
    return is_partitioned;
}


template<typename RandomAccessIterator, typename Predicate>
inline int bad_placed_left(RandomAccessIterator begin, RandomAccessIterator part, const Predicate& pred){
    int i = 0;
    for (RandomAccessIterator it = begin; it != part; ++it){
        if (not pred(*it)){
            ++i;
        }
    }
    return i;
}

template<typename RandomAccessIterator, typename Predicate>
inline int count_left(RandomAccessIterator begin, RandomAccessIterator end, Predicate pred){
    int i = 0;
    for (RandomAccessIterator it = begin; it != end; ++it){
        if (pred(*it)){
            ++i;
        }
    }
    return i;
}

template<typename RandomAccessIterator, typename Predicate>
bool check_partitioned_right(RandomAccessIterator end, RandomAccessIterator part, const Predicate pred){
    bool is_partitioned = true;
    for (RandomAccessIterator it = part; it < end and is_partitioned; ++it){
        if (pred(*it)){
            is_partitioned = false;
            std::cout << "false right " << end - part << " " <<end - it << std::endl;
        }
    }
    return is_partitioned;
}

/* End */
#endif

