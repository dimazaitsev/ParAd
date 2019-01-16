## `ParAd: Parallel Adriana, version 1.1.2` 

# Solve a linear Diophantine homogeneous (sparse) system via composition of its clans 


Compatibility: 
-------------- 

Source code: `C` (gcc version 5.4.1) with `OpenMP` (version 4.0) and `libc` (version 2.23) functions. 

Operating system: `Linux` (tested on `Ubuntu 16.04`). 

Libraries: `MPI` -- Message Passing Interface (tested on `MPICH 3.2`, www.mpich.org ); `SuiteSparse:GraphBLAS` -- a set of sparse matrix operations on an extended algebra of semirings using an almost unlimited variety of operators and types (version 1.1.2, http://faculty.cse.tamu.edu/davis/suitesparse.html ). 

Solvers: `zsolve` from `4ti2` package ( www.4ti2.de ). 
    
Data formats: (sparse) matrix in a simple coordinate format; import a Petri net in .net/.ndr format of system `Tina` ( www.laas.fr/tina ). 


Installation: 
-------------

### Prerequisites:

1. Download and install `GraphBLAS` (version and location are specified in Compatibility section).

2. Download and install `4ti2` (version and location are specified in Compatibility section).

3. Add path to `zsolve` to PATH variable, for example:

    >PATH=$PATH:/usr/local/bin

    >export PATH

### Install ParAd-1.1.2

1. Download ParAd-1.1.2.tar.gz

2. Extract contents of ParAd-1.1.2.tar.gz into directory ParAd-1.1.2

    >tar -xvf ParAd-1.1.2.tar.gz

3. Correct `ParAd-1.1.2/Makefile` and `ParAd-1.1.2/utils/Makefile` with actual paths to include and library directories of GraphBLAS (variables GRAPGBLASINCLUDEDIR and GRAPGBLASLIBDIR, respectively).

4. Come to ParAd-1.1.2 directory

    >cd ParAd-1.1.2

5. Compile and build ParAd executable files

    >make

The main executable file ParAd is put into `test` subdirectory. Besides, subdirectory `utils` contains a few auxiliary executable files briefly specified in `utils/README.txt`. 


Description: 
------------ 

Solving a sparse linear Diophantine homogeneous system is a widespread task for manifold application areas. Sometimes systems are rather big, and gaining some speed-up, employing useful properties of a sparse system [1,2], is a good idea. Besides, in a Petri net domain, solving such a system in nonnegative integer numbers is required that constitutes an intractable task. For Petri nets, the technique of using decomposition into clans (functional subnets) for solving linear systems [3,4] has been implemented as a software tool `Adriana` in 2005. 

`ParAd` [1], in essence, is a remake of `Adriana` which uses multi-core clusters to gain a considerable speed-up of computations. We implement two schemes of clans composition (simultaneous and parallel-sequential) independently from a linear system solver. We implement one internal solver `ParTou` using `OpenMP` to find solutions in nonnegative domain that is valuable for Petri net applications. We also implement an interface routine to launch `zsolve` from `4ti2` ( www.4ti2.de ) package to solve a system in the entire integer domain. Besides, we provide an option to attach a new external solver.

To be consistent with general theory and tools for solving linear Diophantine systems, we provide a simple coordinate format to input/output a (sparse) matrix. Note that at present we use 32-bits representation of integer numbers which can be easily extended or replaced by some variable length representation.

Since `Adriana` had been implemented as a plug-in of system `Tina` ( www.laas.fr/tina ) to draw and analyse Petri nets, historically we provide import from its formats of data, namely .net and .ndr types for logical and graphical specification of a Petri net, respectively. Besides, Tina implements import/export of nets in some other formats including the recent standard for Petri nets exchange -- .pnml.

Actually we solve a system 

     `xC=0` (or `Cy=0`),

in integer numbers: `c_{i,j}` and `x_i` (`y_i`) are integer numbers.

Note that the task corresponds to finding Petri net place (or transition) invariants, respectively, in case matrix C is a Petri net incidence matrix; for Petri nets, nonnegative integer invariants are of some use usually.

To apply composition of clans to speed-up solving a system, the matrix should be decomposable into clans that supposes it is, at first, sparse. Matrices of clans obtained in the process of solution can be either dense or sparse; usually they are more dense than the source matrix. Let us remind that decomposition into clans is represented by a union of a block column matrix with a block diagonal matrix:

     | A1 C1 0  ... 0  | 
     | A2 0  C2 ... 0  | 
     |          ...    | 
     | Ak 0  0  ... Ck | 

Clans are represented with block-rows of the matrix, where block-matrices C1 ... Ck specify internal parts of each clan, while block-matrices A1 ... Ak define connection of clans. Thus, matrix of i-th clan is represented as (Ai,Ci). 

Analysis of about a hundred of real-life Petri net models from Model Checking Contest ( https://mcc.lip6.fr/ ) collection shows that about 3/4 of them are rather good decomposable into clans.


Command line format: 
-------------------- 

   >mpirun [mpirun_options] ParAd [options] system_file_name solutions_file_name 

ParAd uses MPI facilities that is why it should be run via `mpirun` or some other command attached to MPI environment, for instance `srun` command of Slurm.

   
### Parameters: 

`mpirun_options`      are described in MPI documentation, for instance `-n N` specifies the number of MPI processes equal to `N`; 

`system_file_name`    a file which contains the linear system matrix (input); 

`solutions_file_name` a file which contains the linear system solution (output); 

#### options: 

  `-h`              print help; 

  `-c`              simultaneous composition of clans; 

  `-s`              parallel-sequential composition of clans; 

  `-r solver_name`  defines a solver name ("ParTou", "zsolve" or a new external solver name); 

  `-T`              use transposed matrix (for Petri nets, find invariants of transitions). 

#### defaults: 

   >mpirun ParAd -r ParTou 

### Details: 

In case of absence of either of keys `-c` or `-s`, the given system is solved directly (without decomposition into clans) using a specified solver.

In case of absence of key `-T`, the source matrix is used (for Petri nets, find invariants of places). 

With `-r` key a solver is specified. The default `ParTou` solver is built-in. If you run an external `zsolse` solver, it should be installed beforehand; ParAd contains an interface routine to call `zsolve`. If you specify a new external solver `solver_name`, each time when it is required to solve a system, the following operating system command is issued `solver_name system_file_name solutions_file_name`. It is supposed that both files are represented in a simple coordinate format. 

All the keys and their parameters should be separated with a blank character.

PaAd also supports the following auxiliary options: `-d D` level of debugging information (D=0 by default); `-w work_file_name` a new prefix for names of temporary working files (`system_file_name` by default).


Format of files: 
----------------- 

### Matrix in a simple coordinate format (`.spm`): 

    i j x_{i,j} 

lines starting from '#' are considered comments. 

Note that the file does not contain neither the matrix size nor the number of nonzero elements.

For example, the following matrix 

    1 0 2 0 3 
    0 4 0 5 0 
    6 0 7 0 8 

is represented as 

    # an example of ParAd matrix 
    1 1 1 
    1 3 2 
    1 5 3 
    2 2 4 
    2 4 5 
    3 1 6 
    3 3 7 
    3 5 8 


Examples of command lines: 
-------------------------- 

    >mpirun -n 1 ParAd tcp.spm tcp.pi.spm 

solve the system directly, `ParTou` solver (in nonegative integer), a single process, read matrix from tcp.spm, write results into tcp.pi.spm; 

    >mpirun -n 4 ParAd -c -T tcp.spm tcp.ti.spm 

solve the system via simultaneous composition of its clans, ParTou solver (in nonnegative integer), 4 processes, transpose matrix read from tcp.spm, write results into tcp.ti.spm; 

    >srun -N 10 ParAd -s ht_d4k3p2b08.spm ht_d4k3p2b08.pi.spm 

solve the system via parallel-sequential composition of its clans, ParTou solver (in nonnegative integer), use 10 nodes with Slurm workload manager, read matrix from ht_d4k3p2b08.spm, write results into ht_d4k3p2b08.pi.spm; 

    >mpirun -n 20 ParAd -c -r zsolve -T ht_d4k3p2b08.spm ht_d4k3p2b08.ti.spm 

solve the system via simultaneous composition of its clans, `zsolve` solver (integer solution), 20 processes, read matrix from ht_d4k3p2b08.spm, write results into ht_d4k3p2b08.ti.spm; 

    >mpirun -n 8 ParAd -s -r zsolve ht_d4k3p2b08.spm ht_d4k3p2b08.ti.spm 

solve the system via parallel-sequential composition of its clans, `zsolve` solver (integer solution), 8 processes, read matrix from ht_d4k3p2b08.spm, write results into ht_d4k3p2b08.ti.spm. 


Tests: 
------ 

Test matrices and scripts are situated in subdirectory `test`, the obtained results are stored in subdirectory `test/output`. Bash scripts to run directly on MPI have suffix `_mpi` while scripts to run on a cluster using Slurm have suffix `_slurm`. There are 3 basic tests: `test1` -- test one given matrix; `test_all` -- test all `.spm` matrices in the current directory; `test` -- test selected list of matrices. Test of a given matrix includes: solving the system directly and solving the system via simultaneous and parallel-sequential composition of clans on 1, 4, and 10 nodes comparing the obtained solutions (and printing solving times). Please mind that `test_all` requires much time.

### Run MPI tests: 

    >cd test 
    >./test_mpi 
    >./test_all_mpi 

### Run Slurm tests: 

    >cd test 
    >sbatch -N 10 -w "d[00-09]" -o output/test_slurm.stdout test_slurm 
    >sbatch -N 10 -w "d[00-09]" -o output/test_all_slurm.stdout test_all_slurm 


Compatibility with Tina 
----------------------- 

### Petri net files of system Tina: 

`.net`      Time Petri nets textual format 

`.ndr`      Time Petri nets graphic format 

are specified in formats.txt file of Tina ( www.laas.fr/tina ) documentation. 

We developed a pair of utilities to provide compatibility with Tina. A utility `fromTina` imports .net/.ndr files from Tina to .spm format with optional storing names of the Petri net places and transitions in separate files. A utility `printTina` prints a solution as a place/transition invariant in textual format adopted in Tina. 

### Import incidence matrix of a Petri net from Tina 

    >fromTina [options] ndr_or_net_file spm_file 

`ndr_or_net_file` Petri net in .net/.ndr format; 

`spm_file`        Petri net in .spm format; 

`options`: 

`-h`               print help;

`-d`               save dictionaries of place/transition names. 

Example of command line: 

    >fromTina tcp.ndr tcp.spm 


### Print (to standard output) a vector as a Petri net invariant in Tina format 

    >printTina [options] ndr_or_net_file invariants_file 

`ndr_or_net_file` Petri net in .net/.ndr format; 

`invariants_file` p- ot t- invariant in .spm format; 

`options`: 

`-h`              print help; 

`-P | -T`         place or transition invariants (default `-P`); 

`-v | -q`         full or digest invariant format (default `-v). 

Examples of command lines: 

    >printTina tcp.ndr tcp.pi.spm

    >printTina -T tcp.ndr tcp.ti.spm > tcp.ti.spm.txt 

Subdirectory `Tina_io` contains a few `.net` and `.ndr` files to try conversions. 


References: 
----------- 

1. Zaitsev D.A., Tomov S., Dongarra J. Solving Linear Diophantine Systems on Parallel Architectures, IEEE Transactions on Parallel and Distributed Systems, 05 October 2018, http://dx.doi.org/10.1109/TPDS.2018.2873354

2. Zaitsev D.A. Sequential composition of linear systems307. Online 12 February 2016, http://dx.doi.org/10.1016/j.ins.2016.02.016 

3. Zaitsev D.A. Compositional analysis of Petri nets, Cybernetics and Systems Analysis, Volume 42, Number 1 (2006), 126-136, http://dx.doi.org/10.1007/s10559-006-0044-0 

4. Zaitsev D.A. Decomposition of Petri Nets, Cybernetics and Systems Analysis, Volume 40, Number 5 (2004), 739-746, http://dx.doi.org/10.1007/s10559-005-0012-0 


----------------------------------------------------------------------- 
@ 2018 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org 

