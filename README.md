# db_amp
Search for amplicons in a .fasta for database of .fasta's using MPI

## Compiling
Make sure that you have a version of GCC that supports OpenMP and a version of OpenMPI in your PATH, and do `make`.

## Running
After building, run
`mpirun -np 10 -hostfile myhostfile.txt ./dbamp -i test_data/ -o test.txt -f CAGCATAAAAGAGGAGGATG -r CTGGTTCCKGATACCGATAG`

## Flags
* `i` -- Path to input directory [REQUIRED]
* `o` -- Path to output file [REQUIRED]
* `f` -- Forward primer [REQUIRED]
* `r` -- Reverse primer [REQUIRED]
* `l` -- Lower bound for amplicon size (default 50)
* `u` -- Upper bound for amplicon size (default 500)
