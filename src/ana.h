// ParAd: Parallel Adriana 
// ana.h                                    
// Solve a system via composition (parallel-sequential) of its clans    

int Adriana( char * NetFileName, char * InvFileName, int SolverNum, int ty, int numprocs, int debug_level );

/* INPUT: NetFileName - system in SPM format */
/*        SolverNum - number of solve in the solvers array */
/*        ty - type of solution; defs for ty */

#define DIRECT 0
/* without composition */
#define SIMULT 1
/* simultaneous composition */
#define SEQUEN 2
/* sequential composition */
/*        numprocs - number of started MPI processes */
/*        debug_level - level of debug info; debug = 0, 1, 2 */

/* OUTPUT: InvFileName - basis invariants */

// @ 2018 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org

