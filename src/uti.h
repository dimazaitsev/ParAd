// ParAd: Parallel Adriana 
// uti.h       
// Utility functions

#define min(a,b) ((a<b)?a:b)
#define max(a,b) ((a>b)?a:b)

#define FILENAMELEN 1024
#define MAXSTRLEN 4096

#define SOLVERS_NUM 2
typedef int (*solver_entry_type)( char * SolverName, char * SystemFileName, char * SolutionFileName, int debug_level );

int isempty( char * s );
/* checks is the string s empty */

void SwallowSpace( char * str, int *i );
/* swallow blanks */

int IsSpace( char * str, int i );
/* check blank */

int CopyFile( char * fnameFROM, char * fnameTO, char * attr );
/* copy (append) file */

int TransposeMatrix( char * InpFileName, char * OutFileName );
/* transposes matrix in SPM format */

int EternalSolverCall( char * SolverName, char * SystemFileName, char * SolutionFileName, int debug_level );
/* starts an external solver, name is passed as global solver_name */

// @ 2018 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org

