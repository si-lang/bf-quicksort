#!/bin/perl

#constants
$SEQUENTIAL = 0;
$BARRIER_PARALLEL = 1;
$BARRIER_PARALLEL_NO_TREE = 2;
$MIX_BARRIER_PARALLEL = 3;
$MIX_BARRIER_PARALLEL_NO_TREE = 4;
$FETCH_ADD_PARALLEL_MCSTL = 5;
$FETCH_ADD_PARALLEL_NO_TREE = 6;
$FETCH_ADD_PARALLEL = 7;
$FETCH_ADD_PARALLEL_NO_TREE_REF = 8;
@algo = ($SEQUENTIAL, $BARRIER_PARALLEL, $BARRIER_PARALLEL_NO_TREE, $MIX_BARRIER_PARALLEL, $MIX_BARRIER_PARALLEL_NO_TREE, $FETCH_ADD_PARALLEL_MCSTL, $FETCH_ADD_PARALLEL_NO_TREE, $FETCH_ADD_PARALLEL, $FETCH_ADD_PARALLEL_NO_TREE_REF);
@algo_names = ("Sequential", "Strided\\_tree", "Strided", "BlockedStrided\\_tree", "BlockedStrided", "F\\&A\\_MCSTL\\_tree", "F\\&A\\_MCSTL","F\\&A\\_tree","F\\&A");

@algo_sort = ($SEQUENTIAL, $FETCH_ADD_PARALLEL, $FETCH_ADD_PARALLEL_NO_TREE, $FETCH_ADD_PARALLEL_NO_TREE_REF);
@algo_compress_sort = (0, 0, 0, 0, 0, 1, 2);
@algo_names_sort = ("Sequential", "F\\&A\\_MCSTL\\_tree", "F\\&A_\\MCSTL","F\\&A\\_tree","F\\&A");

$INT = 0;
$HCOMP = 1; 
$HCOPY = 2; 
#@data_type = ($INT, $HCOMP, $HCOPY);
#@data_type_names = ("int", "hcomp", "hcopy");
@data_type = ($INT, $HCOMP);
@data_type_names = ("int", "hcomp");
#@data_type = ($HCOMP);
#@data_type_names = ("hcomp");

$RAND = 0;
$BIASED = 1; 
$SORTED = 2;
#@input_kind = ($RAND, $BIASED, $SORTED);
#@input_kind_names = ("rand", "biased", "sort");
#@input_kind = ($BIASED);
#@input_kind_names = ("biased");
@input_kind = ($RAND);
@input_kind_names = ("rand");

$N_i = 0;
$PROCS_i = 1;
$ALG_i = 2;
$BS_i = 3;
@titles = ("n", "thr", "algo", "block");

1;
