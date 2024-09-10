This folder contains the following files:

- partition_constants.h: constants used in the parallel partition implementation and the tests
- parallel_partition_def.h: definition of the partition and nthelement methods (pure interface)
- parallel_partition.h: implementation of the parallel partition algorithms presented in this paper
- mcstl_partition.h: modification of the mcstl_partition.h to run the counters tests
- parallel_partition_mcstl_mod.h: implementation of the 'hybrid' methods that combines part of the mcstl code and our own
- test.cc: source code of the time and counters tests

- compile: example of how to compile the tests, should be modified for the specific environment
- compile_no_mcstl: example to compile the tests without having the mcstl installed, should be modified for the specific environment

- constants.pl: perl script with the definition of constants to be used by test_exec.pl and test_plot.pl
- test_exec.pl: perl script to run tests using an executable created with test.cc (both counter and time tests)
- test_plot.pl: perl script to draw plots once the data from the tests have been generated with test_plot

- readme.txt: this file