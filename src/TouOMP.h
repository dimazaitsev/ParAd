/* ParTou: TouOMP.h: */
/* Solves a homogenous Diophantine system in ninegative integer numbers */
/* by Toudic method on multicore parallel architectures using OpenMP    */
/* file format - sparse matrix, coordinate form: i j x_{i,j} */

int Toudic( char * SolverName, char * SystemFileName, char * SolutionFileName, int debug_level );

// @ 2018 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org

