///////////////////////////////// ParAd: Parallel Adriana //////////////////////////////////////////////
// Solving a linear homogeneous Diophantine system in the process of composition of its minimal clans //
// Solving in nonnegatives for calculation linear invariants of Petri nets (ParTou)                   //
// @ 2018 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org                            //
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <bits/time.h>
#include <sys/time.h>
#include <mpi.h>
#include "uti.h"
#include "ana.h"
#include "TouOMP.h"
#include "call4ti2.h"

static char Help[] =
"ParAd - Parallel Adriana - version 1.1.2\n\n"
"action: solves linear homogeneous Diophantine system via\n"
"        composition of its clans on multi-core clusters\n"
"file format: matrix in a simple coordinate format (i j x_{i,j})\n"
"usage:   ParAd [-h]\n"
"               [-T]\n"
"               [-c] [-s]\n"
"               [-r name]\n"
"               infile outfile\n"
"FLAGS          WHAT                                           DEFAULT\n"
"-h             print help (this mode)\n"
"-c             simultaneous composition of clans\n"
"-s             parallel-sequential composition of clans\n"
"-r name        solver name (ParTou, zsolve) or a new name     ParTou\n"
"-T             transpose matrix\n"
"infile         input file\n"
"outfile        output file\n\n"
"@ 2018 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org\n";

// declare global variables 
char * solver_name_array[] = {"ParTou","zsolve",""};
char solver_name[FILENAMELEN+1];
solver_entry_type solver_entry_array[] = {Toudic,zsolve4ti2,EternalSolverCall};

double magma_wtime( void )
{
  struct timeval t;
  gettimeofday( &t, NULL );
  return t.tv_sec + t.tv_usec*1e-6;
}

int fsize(FILE *fp);

int main( int argc, char *argv[] )
{
  char fname1[ FILENAMELEN+1 ];
  char fname2[ FILENAMELEN+1 ];
  char fname3[ FILENAMELEN+1 ];
  char * SystemFileName;
  char * SolutionFileName;
  char * WorkFileName;
  int i,numf, snumf, inv;
  char *s;
  int transpose=0;
  int mode=0;
  int work_file=0;
  int debug_level=0;
  double dt;
  int  namelen, numprocs, rank;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  MPI_Status status;
  char c, c1;
  int len;
  char * buf;
  FILE *fi, *fo;
  int z;
  double t1,t2;
  int SolverNum = 0;

  MPI_Init(NULL,NULL);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  MPI_Comm_size(MPI_COMM_WORLD,&numprocs);    
  MPI_Get_processor_name(processor_name,&namelen);

  if (rank == 0) // === MASTER
  { 
if(debug_level>2) printf("# i am master %d of %d run on %s\n", rank, numprocs, processor_name); 
 
    t1=magma_wtime();

    /* parse command line */
    numf=0;
    for( i=1; i<argc; i++ )
    {
      if( strcmp( argv[i], "-h" )==0 )
      {
        printf( "%s", Help ); return( 0);
      }
      else if( strcmp( argv[i], "-T" )==0 ) { transpose=1; }
      else if( strcmp( argv[i], "-c" )==0 ) mode=1;
      else if( strcmp( argv[i], "-s" )==0 ) mode=2;
      else if( strcmp( argv[i], "-w" )==0 ) { work_file=1; snumf=numf; numf=-1; }
      else if( numf==-1 ) { strcpy(WorkFileName,argv[ i ]); numf=snumf; }
      else if( strcmp( argv[i], "-d" )==0 ) { snumf=numf; numf=-2; }
      else if( numf==-2 ) { debug_level = atoi( argv[ i ] ); numf=snumf; }
      else if( strcmp( argv[i], "-r" )==0 ) { SolverNum=-1; snumf=numf; numf=-3; }
      else if( numf==-3 ) { strcpy(solver_name, argv[ i ]); numf=snumf; }
      else if( numf==0 ) { SystemFileName=argv[i]; numf++; }
      else if( numf==1 ) { SolutionFileName=argv[i]; numf++; }
      else
        { fprintf( stderr, "*** unknown option: %s\n", argv[i] ); return(4); }
    } /* for */
  
//    if( numf==0 ) SystemFileName = "-";
//    if( numf<=1 ) SolutionFileName = "-";  
    if( numf<2 )
    {
        fprintf( stderr, "*** absent input/otput file(s) \n" );
        printf( "%s", Help ); return( 5);
    }
//  strict decision: no stdin/stdout defaults for clarity
 
    if( work_file )  sprintf( fname1, "%s", WorkFileName ); 
    else             sprintf( fname1, "%s__", SystemFileName );
  
    if(SolverNum==-1)
    {
      solver_name_array[ SOLVERS_NUM ] = solver_name;
      for(i=0;i<SOLVERS_NUM;i++)
        if(strcmp(solver_name,solver_name_array[i])==0)
          break;
      SolverNum=i;
if(debug_level>2) printf("# solvername %s, solvernum %d\n", solver_name_array[SolverNum], SolverNum);
    }  

    // solve a system xC=0
    if( transpose )
    {
      TransposeMatrix( SystemFileName, fname1 );
    }
    else 
    {
      CopyFile( SystemFileName, fname1, "w");
    }

    Adriana( fname1, SolutionFileName, SolverNum, mode, numprocs, debug_level ); // solve a given system

    t2=magma_wtime();
  
if(debug_level>0) printf( "# %#fs\n", t2-t1 );

    remove( fname1 );

  } // master ends

  else // === WORKERS: solve a system sent by master
  { 
if(debug_level>2) printf("# i am a worker %d of %d run on %s for solving a system for clan\n", rank, numprocs, processor_name);

    // initialize worker with solver number and name

    MPI_Recv(&SolverNum,1,MPI_INT,0,41,MPI_COMM_WORLD,&status);
    MPI_Recv(&len,1,MPI_INT,0,42,MPI_COMM_WORLD,&status);
    MPI_Recv(solver_name,len,MPI_CHAR,0,43,MPI_COMM_WORLD,&status);

    // worker's working loop

    while(1)
    {
      c='R';
      MPI_Send(&c,1,MPI_CHAR,0,51,MPI_COMM_WORLD); // i am ready
      MPI_Recv(&c1,1,MPI_CHAR,0,52,MPI_COMM_WORLD,&status);
      if(c1!='W') break; // break if no work
      MPI_Recv(fname3,FILENAMELEN+1,MPI_CHAR,0,521,MPI_COMM_WORLD,&status); 
      SystemFileName=fname3;
      MPI_Recv(&z,1,MPI_INT,0,53,MPI_COMM_WORLD,&status); 
      MPI_Recv(&len,1,MPI_INT,0,54,MPI_COMM_WORLD,&status);
      buf=malloc(len); 
      if(buf==NULL){fprintf(stderr,"error alloc buf\n");exit(2);}
      MPI_Recv(buf,len,MPI_CHAR,0,55,MPI_COMM_WORLD,&status);
if(debug_level>2) printf("# worker %d received job of %d for subnet %d\n", rank, len, z);
      sprintf( fname1, "%s.w%d.z%d", SystemFileName, rank, z );
      fo=fopen(fname1, "w");
      if(fo==NULL){fprintf(stderr,"error open file %s\n", fname1);exit(2);}
      fwrite(buf,len,1,fo);
      free(buf);
      fclose(fo);
      sprintf( fname2, "%s.w%d.i%d", SystemFileName, rank, z );

      (*solver_entry_array[SolverNum])( solver_name, fname1, fname2, debug_level ); /* uti */ 
//sleep(rand()%10+10); 
if(debug_level>2) printf("# worker %d subnet %d Solver finished\n", rank, z);

      remove(fname1);
      fi=fopen(fname2, "r");
      len=fsize(fi);
      buf=malloc(len);
      if(buf==NULL){fprintf(stderr,"error alloc buf\n");exit(2);}
      fread(buf,1,len,fi);
      c='D';
      MPI_Send(&c,1,MPI_CHAR,0,51,MPI_COMM_WORLD);
      MPI_Send(&z,1,MPI_INT,0,56,MPI_COMM_WORLD);
      MPI_Send(&len,1,MPI_INT,0,57,MPI_COMM_WORLD);
      MPI_Send(buf,len,MPI_CHAR,0,58,MPI_COMM_WORLD);
if(debug_level>2) printf("# slave %d sent job of %d\n", rank, len);
      free(buf); 
      fclose(fi);
      remove(fname2);
    } // while ends

if(debug_level>1) printf("# worker %d finished\n", rank);
  } // worker ends

  MPI_Finalize();

  // MPI ends  
  return(0);
  
} /* main */

// @ 2018 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org 

