// ParAd: Parallel Adriana 
// ana.c                                    
// Solve a system via composition (parallel-sequential) of its clans

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include "deo.h"
#include "fic.h"
#include "coy.h"
#include "fui.h"
#include "mui.h"
#include "grb_mul2a.h"
#include "smb.h"
#include "uti.h"
#include "ana.h"
#include "cor.h"

//#define FILENAMELEN 256
//#define MAXSTRLEN 1024

// use global variables 
extern char * solver_name_array[];
extern char solver_name[FILENAMELEN+1];
extern solver_entry_type solver_entry_array[];

int CleanWorkFiles( int nz, char * NetFileName );
int ReadFusionSet( char * TraceFileName, int nline, char * FusionSet );
int NextFSN( char * FusionSet, int nitem );

int fsize(FILE *fp){
    int prev=ftell(fp);
    fseek(fp, 0L, SEEK_END);
    int sz=ftell(fp);
    fseek(fp,prev,SEEK_SET); //go back to where we were
    return sz;
}

int Adriana( char * NetFileName, char * InvFileName, int SolverNum, int ty, int numprocs, int debug )
{   
  int nz, z, nf, f, i, j, n, pn, nx;
  
  char fname1[ FILENAMELEN+1 ];
  char fname2[ FILENAMELEN+1 ];
  char fname3[ FILENAMELEN+1 ];
  
  char FusionSet[ MAXSTRLEN +1 ];

  char c, c1;
  int len;
  char * buf;
  FILE *fi, *fo;
  int zs, zf, zc, p;
  MPI_Status status;
  int *waiting_list=NULL, waiting_num, maybe_edge=1;
  struct edge ce, *e, *pe;

  if(debug>0)printf("# Processes: %d, solver: %s\n", numprocs, solver_name_array[SolverNum] );
  
  if( ty == DIRECT )
  {
    // ---------------------- close slaves for no need in them in direct solving a system

    if( numprocs > 1) 
    {
      for(i=1;i<numprocs;i++) 
      {
        MPI_Recv(&c,1,MPI_CHAR,MPI_ANY_SOURCE,51,MPI_COMM_WORLD,&status);
        p=status.MPI_SOURCE;
        c1='C'; // close slave
	MPI_Send(&c1,1,MPI_CHAR,p,52,MPI_COMM_WORLD);
      }
    }
    if(debug>0)printf("# Direct solution\n");
    return( (*solver_entry_array[SolverNum])( solver_name_array[SolverNum], NetFileName, InvFileName, debug ) );
  }
    
  nz = DecomposeIntoFSN( NetFileName, debug ); /* deo1 */

if(debug>0)printf("# Decomposed into %d clans\n", nz );

//if(debug>3){printf("# Press a key!\n");getchar();}

if( numprocs > 1) {

  // initialize workers with solver number and name

    len=strlen(solver_name_array[SolverNum])+1;
    for(p=1;p<numprocs;p++) 
    {
      MPI_Send(&SolverNum,1,MPI_INT,p,41,MPI_COMM_WORLD);
      MPI_Send(&len,1,MPI_INT,p,42,MPI_COMM_WORLD);
      MPI_Send(solver_name_array[SolverNum],len,MPI_CHAR,p,43,MPI_COMM_WORLD);
    }

  // MPI parallel solving systems for clans

    waiting_list=malloc((numprocs-1)*sizeof(int)); 
    if(waiting_list==NULL){fprintf(stderr,"error alloc waiting list\n");exit(2);}
    waiting_num=0;


    zs=zf=zc=0; 

    while(zs<nz || zf<nz || zc<numprocs-1 )
    {
if(debug>2)printf("# *** master loop zs=%d, zf=%d, zc=%d nz=%d numprocs=%d\n",zs,zf,zc, nz, numprocs );
      MPI_Recv(&c,1,MPI_CHAR,MPI_ANY_SOURCE,51,MPI_COMM_WORLD,&status);
      p=status.MPI_SOURCE;  
if(debug>2)printf("# *** master received %c from %d\n", c, p);  
      switch(c)
      {
      case 'R': // ready
	if(zs>=nz)
	{
          // c1='C'; // close slave
	  //MPI_Send(&c1,1,MPI_CHAR,p,52,MPI_COMM_WORLD);
          waiting_list[waiting_num++]=p;
if(debug>2)printf("# *** master added %d to waiting list\n", p);
	  zc++;
	  break;
	}
        c1='W'; // work for slave
        zs++;
	MPI_Send(&c1,1,MPI_CHAR,p,52,MPI_COMM_WORLD); 
        MPI_Send(NetFileName,FILENAMELEN+1,MPI_CHAR,p,521,MPI_COMM_WORLD); 
        MPI_Send(&zs,1,MPI_INT,p,53,MPI_COMM_WORLD);
if(debug>2)printf("# *** master sent c1=%c and zs=%d to %d\n", c1, zs, p);
	sprintf( fname1, "%s.z%d", NetFileName, zs );
        fi=fopen(fname1, "r");
        if(fi==NULL){fprintf(stderr,"error open file %s\n", fname1);exit(2);}
        len=fsize(fi);
	buf=malloc(len);
        if(buf==NULL){fprintf(stderr,"error alloc buf\n");exit(2);}
        fread(buf,len,1,fi);
        MPI_Send(&len,1,MPI_INT,p,54,MPI_COMM_WORLD);
	MPI_Send(buf,len,MPI_CHAR,p,55,MPI_COMM_WORLD);
if(debug>2)printf("# *** master sent subnet %d to %d\n", zs, p);
	free(buf);
        fclose(fi);
	break;
      case 'D': // data
	MPI_Recv(&z,1,MPI_INT,p,56,MPI_COMM_WORLD,&status);
	MPI_Recv(&len,1,MPI_INT,p,57,MPI_COMM_WORLD,&status);
	buf=malloc(len);
        if(buf==NULL){fprintf(stderr,"error alloc buf\n");exit(2);}
	MPI_Recv(buf,len,MPI_CHAR,p,58,MPI_COMM_WORLD,&status);
if(debug>2)printf("# *** master received subnet %d from %d\n", z, p);
	sprintf( fname2, "%s.i%d", NetFileName, z );
	fo=fopen(fname2, "w");
	fwrite(buf,len,1,fo);
	zf++;
        free(buf);
        fclose(fo);

	if( ty == SEQUEN )
        {
          sprintf( fname1, "%s.z%d", NetFileName, z );
          sprintf( fname2, "%s.y%d", NetFileName, z );
          rename( fname1, fname2 ); /* .z# -> .y# */
    
          sprintf( fname1, "%s.i%d", NetFileName, z );
          sprintf( fname2, "%s.x%d", NetFileName, z );
          rename( fname1, fname2 ); /* .i# -> .x# */
        }
        break;
      }
      
    }
if(debug>2)printf("# master finished with clans\n");

} else {

  // original sequential code
  // solve systems for FSN 
if(debug>2)printf("# single master\n");
  for( z=1; z<=nz; z++ ) 
  {
    sprintf( fname1, "%s.z%d", NetFileName, z );
    sprintf( fname2, "%s.i%d", NetFileName, z );
    
    (*solver_entry_array[SolverNum])( solver_name_array[SolverNum], fname1, fname2, debug );
    
    if( ty == SEQUEN )
    {
      sprintf( fname1, "%s.z%d", NetFileName, z );
      sprintf( fname2, "%s.y%d", NetFileName, z );
      rename( fname1, fname2 ); 
    
      sprintf( fname1, "%s.i%d", NetFileName, z );
      sprintf( fname2, "%s.x%d", NetFileName, z );
      rename( fname1, fname2 );
    }
  }
  // end of origina sequential code
}
  
  if( nz == 1 )
  {
    if( ty == SEQUEN ) 
      sprintf( fname1, "%s.x1", NetFileName ); 
    else
      sprintf( fname1, "%s.i1", NetFileName ); 
    rename( fname1, InvFileName );
    if( debug < 2 ) CleanWorkFiles( nz, NetFileName );
    return(1);
  }

  if( ty == SEQUEN )
  {
    sprintf( fname1, "%s.z", NetFileName );
    sprintf( fname2, "%s.y", NetFileName );
    rename( fname1, fname2 ); /* .z -> .y */
  }

  if( ty == SIMULT )
  {
if(debug>0)printf("# Simultaneous composition of clans\n");

// ---------------------- close slaves for no need in them for simultaneous composition

    if( numprocs > 1) 
    {
      c1='C'; // close slave
      for(p=0;p<waiting_num;p++) 
      {
	MPI_Send(&c1,1,MPI_CHAR,waiting_list[p],52,MPI_COMM_WORLD);
      }
      waiting_num=0;
    }

    /* composition */
    CreateCompositionSystem( NetFileName ); /* coy */
    if( debug > 1 )
    {
      sprintf( fname1, "%s.s", NetFileName );
      sprintf( fname2, "%s-s1", NetFileName );
      CopyFile( fname1, fname2, "w" );
    }

    sprintf( fname1, "%s.s", NetFileName );
    sprintf( fname2, "%s.r0", NetFileName );
    (*solver_entry_array[SolverNum])( solver_name_array[SolverNum], fname1, fname2, debug ); /* toy2 */
    
    sprintf( fname1, "%s.r0", NetFileName );
    sprintf( fname2, "%s.u", NetFileName );
    sprintf( fname3, "%s.r", NetFileName );
    AddUnitSolutions( fname1, fname2, fname3 ); /* fui */
if(debug>1)printf("# done AddUnitSolutions\n");    
    ComposeJointMatrix( NetFileName ); /* mui */
if(debug>1)printf("# done ComposeJointMatrix\n");   
    sprintf( fname1, "%s.r", NetFileName );
    sprintf( fname2, "%s.g", NetFileName );
    sprintf( fname3, "%s.h0", NetFileName );
    MultiplySPM( fname1, fname2, fname3 ); /* smmul2 */
if(debug>1)printf("# done MultiplySPM\n");
    sprintf( fname1, "%s.h0", NetFileName );
    sprintf( fname2, "%s.h", NetFileName );
    MinimizeBasis( fname1, fname2 ); /* smb2 */
    //CopyFile( fname1, fname2, "w" );
    /* end of composition */
   
    /* save solution */
    sprintf( fname1, "%s.h", NetFileName );
    rename( fname1, InvFileName );
  
    if( debug < 2 ) CleanWorkFiles( nz, NetFileName );
    free(waiting_list);
    return(0);
  } /* if ty == SIMULT */


// ******************************* parallel-sequential/sequential composition

if(debug>0)printf("# Parallel-sequential composition of clans\n");

if( numprocs > 1) {
// parallel-sequential solving composition system
    sprintf( fname1, "%s.p", NetFileName ); 
    ReadGraph( fname1, &e, &n ); /* cor1 */
    nx=nz;
if(debug>1)printf("# read graph having %d edges, waiting_num=%d\n",n, waiting_num);
    pe = calloc( n, sizeof(struct edge) );
    if(pe==NULL){fprintf(stderr,"error alloc pe\n");exit(2);}	
    pn=0; maybe_edge=1;

    zs=zf=zc=0;

    while(zs<nz-1 || zf<nz-1 || zc<numprocs-1)
    { 
if(debug>1){printf("# *** master loop zs=%d, zf=%d, zc=%d nz=%d n=%d pn=%d numprocs=%d\n",zs,zf,zc, nz, n, pn, numprocs );}
//printf("# e of %d edges: ",n);for(i=0;i<n;i++){printf("%d-%d>%d ",e[i].v1,e[i].w,e[i].v2);}printf("\n");
//printf("# pe of %d edges: ",pn);for(i=0;i<pn;i++){printf("%d-%d>%d ",pe[i].v1,pe[i].w,pe[i].v2);}printf("\n");
if(debug>1){printf("# waiting_num=%d, maybe_edge=%d\n", waiting_num, maybe_edge);}

      while(waiting_num>0 && maybe_edge)
      {
        if(ChooseEdge('f', e, &n, pe, &pn)) /* cor1 */
        {
if(debug>2){printf("# chosen edge %d->%d w=%d n=%d pn=%d\n",pe[pn-1].v1,pe[pn-1].v2,pe[pn-1].w,n, pn);}
if(debug>2){printf("# e of %d edges: ",n);for(i=0;i<n;i++){printf("%d-%d>%d ",e[i].v1,e[i].w,e[i].v2);}printf("\n");}
if(debug>2){printf("# pe of %d edges: ",pn);for(i=0;i<pn;i++){printf("%d-%d>%d ",pe[i].v1,pe[i].w,pe[i].v2);}printf("\n");}

          // create system
//********************************************************************************
          ce.v1=pe[pn-1].v1; ce.v2=pe[pn-1].v2; ce.w=pe[pn-1].w;

          i = psFilterContactPlaces( NetFileName, ce.v1, ce.v2 ); /* fic: .y -> .y, .z */

          // encode contracting edge
          z=ce.v1*(nx+1)+ce.v2;

          /* prepare FSN files */

          sprintf( fname1, "%s.x%d", NetFileName, ce.v1 );
          sprintf( fname2, "%s.i1", NetFileName );
          CopyFile( fname1, fname2, "w" );

          sprintf( fname1, "%s.x%d", NetFileName, ce.v2 );
          sprintf( fname2, "%s.i2", NetFileName );
          CopyFile( fname1, fname2, "w" );

          /* composition */
          CreateCompositionSystem( NetFileName ); /* coy: .z, .i%d -> .s, .u */
if(debug>1)printf( "# CreateCompositionSystem, z=%d, waiting_num=%d\n", z, waiting_num);

          ComposeJointMatrix( NetFileName ); /* mui: .z, .i%d -> .g */
if(debug>1)printf( "# ComposeJointMatrix, z=%d, waiting_num=%d\n", z, waiting_num);

          if( debug > 1 )
          {
            sprintf( fname1, "%s.s", NetFileName );
            sprintf( fname2, "%s-s%d", NetFileName, z );
            CopyFile( fname1, fname2, "w" );
            sprintf( fname1, "%s.y", NetFileName );
            sprintf( fname2, "%s-y%d", NetFileName, z );
            CopyFile( fname1, fname2, "w" );
            sprintf( fname1, "%s.z", NetFileName );
            sprintf( fname2, "%s-zz%d", NetFileName, z );
            CopyFile( fname1, fname2, "w" );
            sprintf( fname1, "%s.u", NetFileName );
            sprintf( fname2, "%s-u%d", NetFileName, z );
            CopyFile( fname1, fname2, "w" );
          }
    
          sprintf( fname1, "%s.s", NetFileName );
          sprintf( fname2, "%s.r0", NetFileName );
          // Solve( fname1, fname2 ); /* toy2 */
//***************************************************************************
// save files
             sprintf( fname1, "%s.z", NetFileName );
             sprintf( fname2, "%s.a%d", NetFileName, z );
             CopyFile( fname1, fname2, "w" );

             sprintf( fname1, "%s.u", NetFileName );
             sprintf( fname2, "%s.b%d", NetFileName, z );
             CopyFile( fname1, fname2, "w" );

             sprintf( fname1, "%s.g", NetFileName );
             sprintf( fname2, "%s.c%d", NetFileName, z );
             CopyFile( fname1, fname2, "w" );

          // send work
          c1='W'; // work for slave
          zs++;
          p=waiting_list[--waiting_num];
if(debug>2)printf("# waiting proces chosen p=%d\n",p);
	  MPI_Send(&c1,1,MPI_CHAR,p,52,MPI_COMM_WORLD); 
          MPI_Send(NetFileName,FILENAMELEN+1,MPI_CHAR,p,521,MPI_COMM_WORLD); 
          MPI_Send(&z,1,MPI_INT,p,53,MPI_COMM_WORLD);
if(debug>1)printf("# *** master sent c1=%c and z=%d to %d, nx=%d\n", c1, z, p, nx);
	  sprintf( fname1, "%s.s", NetFileName);
          fi=fopen(fname1, "r");
          if(fi==NULL){fprintf(stderr,"error open file %s\n", fname1);exit(2);}
          len=fsize(fi);
	  buf=malloc(len);
          if(buf==NULL){fprintf(stderr,"error alloc buf\n");exit(2);}
          fread(buf,len,1,fi);
          MPI_Send(&len,1,MPI_INT,p,54,MPI_COMM_WORLD);
	  MPI_Send(buf,len,MPI_CHAR,p,55,MPI_COMM_WORLD);
if(debug>2)printf("# *** master sent subnet %d to %d\n", zs, p);
	  free(buf);
          fclose(fi);
        }
        else maybe_edge=0;
      }
      
      // close waiting if nothing to do
      if(zs>=nz-1 && zf>=nz-1)
      {
        for(p=0;p<waiting_num;p++) 
        {
          c1='C'; // close slave
	  MPI_Send(&c1,1,MPI_CHAR,waiting_list[p],52,MPI_COMM_WORLD);
if(debug>1)printf("# sent %c to %d\n", c1, waiting_list[p]);
          zc++;
        }
        waiting_num=0;
        if(zs<nz-1 && zf<nz-1 && zc<numprocs-1) break;
      }

      // receive
if(debug>2)printf("# *** master waits a signal\n"); 
      MPI_Recv(&c,1,MPI_CHAR,MPI_ANY_SOURCE,51,MPI_COMM_WORLD,&status);
      p=status.MPI_SOURCE;  
if(debug>1)printf("# *** master received %c from %d\n", c, p);  
      switch(c)
      {
        case 'R':  
          if(zs<nx-1)
            waiting_list[waiting_num++]=p;
          else 
          {
            c1='C'; // close slave
	    MPI_Send(&c1,1,MPI_CHAR,p,52,MPI_COMM_WORLD);
            zc++;
          }  
          break;

        case 'D':
          // receive data
          MPI_Recv(&z,1,MPI_INT,p,56,MPI_COMM_WORLD,&status);

          // recover edge 
          ce.v1=z/(nx+1); ce.v2=z%(nx+1); ce.w=0;

	  MPI_Recv(&len,1,MPI_INT,p,57,MPI_COMM_WORLD,&status);
	  buf=malloc(len);
          if(buf==NULL){fprintf(stderr,"error alloc buf\n");exit(2);}
	  MPI_Recv(buf,len,MPI_CHAR,p,58,MPI_COMM_WORLD,&status);
if(debug>1)printf("# *** master received subnet %d (%d->%d) from %d, nx=%d\n", z, ce.v1, ce.v2, p, nx);
	  sprintf( fname2, "%s.r0", NetFileName );
	  fo=fopen(fname2, "w");
	  fwrite(buf,len,1,fo);
	  zf++;
          free(buf);
          fclose(fo);
          if( debug > 1 )
          {
            sprintf( fname1, "%s.r0", NetFileName );
            sprintf( fname2, "%s-r0-%d", NetFileName, z );
            CopyFile( fname1, fname2, "w" );
          }

// restore files .z, .u, .i%d
             sprintf( fname1, "%s.z", NetFileName );
             sprintf( fname2, "%s.a%d", NetFileName, z );
             rename( fname2, fname1 );

             sprintf( fname1, "%s.u", NetFileName );
             sprintf( fname2, "%s.b%d", NetFileName, z );
             rename( fname2, fname1 );

             sprintf( fname1, "%s.g", NetFileName );
             sprintf( fname2, "%s.c%d", NetFileName, z );
             rename( fname2, fname1 );

             sprintf( fname1, "%s.x%d", NetFileName, ce.v1);
             sprintf( fname2, "%s.i1", NetFileName);
             CopyFile( fname1, fname2, "w" );

             sprintf( fname1, "%s.x%d", NetFileName, ce.v2);
             sprintf( fname2, "%s.i2", NetFileName );
             CopyFile( fname1, fname2, "w" );

          // compose matrix
//***************************************************************************
          sprintf( fname1, "%s.r0", NetFileName );
          sprintf( fname2, "%s.u", NetFileName );
          sprintf( fname3, "%s.r", NetFileName );
          AddUnitSolutions( fname1, fname2, fname3 ); /* fui: .r0, .u -> .r */
	 
if(debug>1)printf( "# AddUnitSolutions, z=%d, waiting_num=%d\n", z, waiting_num);

          if( debug > 1 )
          {
            sprintf( fname1, "%s.r", NetFileName );
            sprintf( fname2, "%s-r%d", NetFileName, z );
            CopyFile( fname1, fname2, "w" );

            sprintf( fname1, "%s.i1", NetFileName );
            sprintf( fname2, "%s-i1-%d", NetFileName, z );
            CopyFile( fname1, fname2, "w" );

            sprintf( fname1, "%s.i2", NetFileName );
            sprintf( fname2, "%s-i2-%d", NetFileName, z );
            CopyFile( fname1, fname2, "w" );

            sprintf( fname1, "%s.z", NetFileName );
            sprintf( fname2, "%s-z-cjm%d", NetFileName, z );
            CopyFile( fname1, fname2, "w" );
          }
    
          //ComposeJointMatrix( NetFileName ); /* mui: .z, .i%d -> .g */

          if( debug > 1 )
          {
            sprintf( fname1, "%s.g", NetFileName );
            sprintf( fname2, "%s-g%d", NetFileName, z );
            CopyFile( fname1, fname2, "w" );
          }
   
          sprintf( fname1, "%s.r", NetFileName );
          sprintf( fname2, "%s.g", NetFileName );
          sprintf( fname3, "%s.h0", NetFileName );
          MultiplySPM( fname1, fname2, fname3 ); /* smmul2: .r, .g -> .h0 */
if(debug>1)printf( "MultiplySPM, z=%d, waiting_num=%d\n", z, waiting_num);

          sprintf( fname1, "%s.h0", NetFileName );
          sprintf( fname2, "%s.h", NetFileName );
          MinimizeBasis( fname1, fname2 ); /* smb2: .h0 -> .h */
          //CopyFile( fname1, fname2, "w" );
          /* end of composition */
if(debug>1)printf("# *** master finished composition of %s.\n", fname2);
if(debug>3){printf("# Press a key!\n");getchar();}
    
          if( debug > 1 ) /* store Inv */
          {
            sprintf( fname1, "%s.h", NetFileName );
            sprintf( fname2, "%s-i%d", NetFileName, z );
            CopyFile( fname1, fname2, "w" ); /* .h -> -i# */
          }
    
          sprintf( fname1, "%s.h", NetFileName );
          sprintf( fname2, "%s.x%d", NetFileName, ce.v1 );
          CopyFile( fname1, fname2, "w" ); /* .h -> .x# */
    
          /* merge FSN */

          sprintf( fname1, "%s.y%d", NetFileName, ce.v1 );
          sprintf( fname2, "%s.y%d", NetFileName, ce.v2 );
          CopyFile( fname2, fname1, "a" );

          sprintf( fname3, "%s.x%d", NetFileName, ce.v2 );
          remove( fname3 );
          sprintf( fname3, "%s.y%d", NetFileName, ce.v2 );
          remove( fname3 );

if(debug>2)printf("*** master finished merging subnets\n");
    
          if( debug > 1 ) /* store FSN */
          {
            sprintf( fname2, "%s-z%d", NetFileName, z );
            CopyFile( fname1, fname2, "w" );
          } 
 
//***************************************************************************

          CollapseEdge(&ce, e, &n, pe, &pn); /* cor1 */
          maybe_edge=1;

if(debug>1)printf("# *** master collapsed edge %d->%d\n",ce.v1,ce.v2);
          break;
      }

    }
    /* save solution */
    sprintf( fname1, "%s.h", NetFileName );
    rename( fname1, InvFileName );

    free(e); free(pe); free(waiting_list);

    if( debug < 2 ) CleanWorkFiles( nz, NetFileName );
if(debug>2)printf("# *** master finished with composition\n");

} else {

// *********************************************** source sequential composition here
if(debug>2)printf("# source sequential composition\n");  
  sprintf( fname1, "%s.p", NetFileName );
  sprintf( fname2, "%s.t", NetFileName );
  nf = CollapseOfGraph( 'm', fname1, fname2 ); /* cor */
 
  for( f=1; f<=nf; f++ )
  {
    z = FilterContactPlaces( NetFileName, f ); /* fic */
if(debug>2)printf( "# fic z=%d\n", z );

    /* prepare FSN files */
    sprintf( fname1, "%s.t", NetFileName );
    ReadFusionSet( fname1, f, FusionSet );
if(debug>2)printf( "# fusion set: %s\n", FusionSet );
    j=1;
    while( (i = NextFSN( FusionSet, j )) )
    {
if(debug>2)printf( "# next FSN: %d\n", i );
      sprintf( fname1, "%s.y%d", NetFileName, i );
      sprintf( fname2, "%s.z%d", NetFileName, j );
      remove( fname2 );
      rename( fname1, fname2 );
      
      sprintf( fname1, "%s.x%d", NetFileName, i );
      sprintf( fname2, "%s.i%d", NetFileName, j );
      remove( fname2 );
      rename( fname1, fname2 );
      
      j++;
    }
    
    /* composition */
/*if( f==2 ) return(0);  */
    CreateCompositionSystem( NetFileName ); /* coy */

    if( debug > 1 )
    {
      sprintf( fname1, "%s.s", NetFileName );
      sprintf( fname2, "%s-s%d", NetFileName, f );
      CopyFile( fname1, fname2, "w" );
    }
    
    sprintf( fname1, "%s.s", NetFileName );
    sprintf( fname2, "%s.r0", NetFileName );
    (*solver_entry_array[SolverNum])( solver_name_array[SolverNum], fname1, fname2, debug ); /* toy2 */
    
    sprintf( fname1, "%s.r0", NetFileName );
    sprintf( fname2, "%s.u", NetFileName );
    sprintf( fname3, "%s.r", NetFileName );
    AddUnitSolutions( fname1, fname2, fname3 ); /* fui */
    
    ComposeJointMatrix( NetFileName ); /* mui */
   
    sprintf( fname1, "%s.r", NetFileName );
    sprintf( fname2, "%s.g", NetFileName );
    sprintf( fname3, "%s.h0", NetFileName );
    MultiplySPM( fname1, fname2, fname3 ); /* smmul2 */

    sprintf( fname1, "%s.h0", NetFileName );
    sprintf( fname2, "%s.h", NetFileName );
    MinimizeBasis( fname1, fname2 ); /* smb2 */
    //CopyFile( fname1, fname2, "w" );
    /* end of composition */
    
    if( debug > 1 ) /* store Inv */
    {
      sprintf( fname1, "%s.h", NetFileName );
      sprintf( fname2, "%s-i%d", NetFileName, f );
      CopyFile( fname1, fname2, "w" ); /* .h -> -i# */
    }
    
    sprintf( fname1, "%s.h", NetFileName );
    sprintf( fname2, "%s.x%d", NetFileName, z );
    CopyFile( fname1, fname2, "w" ); /* .h -> .x# */
    
    /* merge FSN */
    sprintf( fname1, "%s.y%d", NetFileName, z );
    remove( fname1 );
    j=1;
    while( (i = NextFSN( FusionSet, j )) )
    {
      sprintf( fname2, "%s.z%d", NetFileName, j );
      CopyFile( fname2, fname1, "a" );
     
      if( i != z )
      {
        sprintf( fname3, "%s.x%d", NetFileName, i );
        remove( fname3 );
        sprintf( fname3, "%s.y%d", NetFileName, i );
        remove( fname3 );
      }
      
      j++;
    }
    
    if( debug > 1 ) /* store FSN */
    {
      sprintf( fname2, "%s-z%d", NetFileName, f );
      CopyFile( fname1, fname2, "w" );
    }

  } /* sequential composition (for) */
    
  /* save solution */
  sprintf( fname1, "%s.h", NetFileName );
  rename( fname1, InvFileName );

  if( debug < 2 ) CleanWorkFiles( nz, NetFileName );

}

return(0);

} /* Adriana */

int CleanWorkFiles( int z, char * NetFileName )
{
  char fname[ FILENAMELEN+1 ];
  int i;
   
  sprintf( fname, "%s.z", NetFileName );
  remove( fname );
  sprintf( fname, "%s.y", NetFileName );
  remove( fname );
  sprintf( fname, "%s.s", NetFileName );
  remove( fname );
  sprintf( fname, "%s.g", NetFileName );
  remove( fname );
  sprintf( fname, "%s.r", NetFileName );
  remove( fname );
  sprintf( fname, "%s.r0", NetFileName );
  remove( fname );
  sprintf( fname, "%s.u", NetFileName );
  remove( fname );
  sprintf( fname, "%s.h0", NetFileName );
  remove( fname );
  sprintf( fname, "%s.p", NetFileName );
  remove( fname );
  sprintf( fname, "%s.t", NetFileName );
  remove( fname );
  sprintf( fname, "%s.x", NetFileName );
  remove( fname );
  
  for( i=1; i<=z; i++ )
  {
    sprintf( fname, "%s.x%d", NetFileName, i );
    remove( fname );
  }
  
  for( i=1; i<=z; i++ )
  {
    sprintf( fname, "%s.y%d", NetFileName, i );
    remove( fname );
  }
  
  for( i=1; i<=z; i++ )
  {
    sprintf( fname, "%s.z%d", NetFileName, i );
    remove( fname );
  }
  
  for( i=1; i<=z; i++ )
  {
    sprintf( fname, "%s.i%d", NetFileName, i );
    remove( fname );
  }
  
  return(0);  

} /* CleanWorkFiles */

int ReadFusionSet( char * TraceFileName, int nline, char * FusionSet )
{
  FILE * ftrace;
  char str[ MAXSTRLEN+1 ];
  int l=0;
 
  ftrace = fopen( TraceFileName, "r" );
  if( ftrace == NULL ) {fprintf(stderr, "*** error open file %s\n", TraceFileName );exit(2);}
 
 /* read solutions */
 while( ! feof( ftrace ) )
 {
   fgets( str, MAXSTRLEN, ftrace );
   
   if( feof( ftrace ) ) break;
   
   l++;
   
   if( l == nline ) { strcpy( FusionSet, str ); break; }
 }
 
 fclose( ftrace );
 return(0);
 
} /* ReadFusionSet */

int NextFSN( char * str, int nitem )
{
  int i, len, item=0; 

  len=strlen( str ); i=0;
  
  while( i<len )
  {
    SwallowSpace( str, &i );
    item++;
    if( item == nitem ) { return( atoi( str+i ) ); }
    
    while( ! IsSpace(str,i) && i<len )i++;
    
  }
  
  return(0);
  
} /* NextFSN */

// @ 2018 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org

