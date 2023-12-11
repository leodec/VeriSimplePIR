# VeriSimplePIR

This library was tested on Ubuntu 20.04.6 LTS on a machine with an Intel i7 core and 32 GB of RAM. 

## Building the Library

### Requirements

- `clang++` (tested with version 10.0.0-4ubuntu1)
- OpenSSL (used to run SHA)

To install all requirements on a fresh installation of Ubuntu 20, run the following command:
    
    sudo apt install make clang libssl-dev

### Instructions

The library binary will be placed by default in `./bin/lib`. The directory `~/VeriSimplePIR/bin/lib` should be in your `LD_LIBRARY_PATH` environment variable, or you should manually place the binary in a directory that's in this variable. If `VeriSimplePIR` is cloned in your home directory, running this command or adding it to your `.bashrc` and reloading your terminal will add `~/VeriSimplePIR/bin/lib` to your `LD_LIBRARY_PATH` environment variable. 

    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/VeriSimplePIR/bin/lib

Assuming the requirements are present, the library can be built by simply running `make` in the top directory. 

The executable files are located in the `bin/demo` directory. To execute a basic test, run `./bin/demo/test/pir_test` from the top directory. This will run the following four tests with a 1 MiB database:
    
1. Semi-honest PIR (SimplePIR).
2. VLHE PIR.
3. VeriSimplePIR without the proof generation. This is functionally equivalent to the first test, but it uses the VeriSimplePIR class.
4. Full VeriSimplePIR test. 

## Running the Benchmarks

### Compile-time Parameters

There are three compile-time parameters that can be set to alter the performance of the benchmarks. Setting some of these values incorrectly will result in insecure parameters or benchmarks that fail to run. 

- `Elem`: This parameter is defined in `src/lib/pir/mat.h`. This parameter determines the datatype used throughout the PIR protocol. The only values of this parameter that have been tested are `uint32_t` and `uint64_t`. Setting `Elem` to `uint32_t` corresponds to an online ciphertext modulus of `2^32`, while setting `Elem` to `uint64_t` corresponds to an online ciphertext modulus of `2^64`. The parameter generation functions automatically take this modulus into account when selecting scheme parameters. Note that these functions may fail if VLHE tries to use `2^32` for larger databases. 

- `STAT_SEC_PARAM`: This parameter is defined in `src/lib/pir/mat_packed.h`. This parameter can be changed to adjust the statistical security level of the protocol. Note that all LWE parameters are chosen to satisfy at least `128` bits of computational security. 

- `BASIS`: This parameter is defined in `src/lib/pir/mat_packed.cpp`. This parameter determines the basis of the packed elements of `Z_p` within a machine word. The constraint on `BASIS` is that it cannot be less than `log(p)`, and the optimal choice of `BASIS = log(p)`. Note that the parameter generation scripts selects `log(p)` to be an integer. If `BASIS` is greater than `log(p)` for a particular benchmark, set `BASIS` equal to the `log(p)` value, run `make`, then rerun the benchmark. This will only affect the performance of `Answer`, which is the entirety of the server's online computation in `SimplePIR` and `VeriSimplePIR`. 

### Computation Benchmarks

The VeriSimplePIR benchmarks are contained in the file `src/demo/bench/preproc_pir_bench.cpp`. These can be run by building the library and running `bin/demo/bench/preproc_pir_bench` from the top directory. 

Similarly, the VLHE PIR benchmarks are given in `src/demo/bench/pir_bench.cpp` and the SimplePIR benchmarks are given in `src/demo/bench/simplepir_bench.cpp`.

### Communication Benchmarks

The parameter generation scripts take a database size (parametrized by the number of entries `N` and the entry bitwidth `d`) and compute the parameters to optimize the online communication. A script to compute the size of the messages is given in `src/demo/scripts/params.cpp`. This file contains the full set of flags to toggle between the various protocols as well as to add the honest digest assumption. 