// ParAd: Parallel Adriana 
// deo1.c                              
// Petri Net Decomposition into clans
// version for decomposition-based solution of equations + parallel-sequential

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "uti.h"

//#define MAXINPSTRLEN 256
//#define FILENAMELEN 256
#define initPL 1024
#define deltaPL 1024
#define initTR 1024
#define deltaTR 1024

struct netarc {
  int p;
  int t;
  int v;
};

typedef struct netarc arcty;

int DecomposeIntoFSN( char * NetFileName, int debug )
{
 char SubNetFileName[ FILENAMELEN + 1];
 char zFileName[ FILENAMELEN + 1];
 char dFileName[ FILENAMELEN + 1];
 char mFileName[ FILENAMELEN + 1];
 FILE * NetFile, * SubNetFile, * zFile, * dFile, * mFile;
 char str[ MAXSTRLEN + 1 ]; /* buffers */

 int n, m, l; /* net size */
 int *tx, *ty, *at, fat; /* trs */
 int *px, *py, *ap, fap; /* pos */

 int p, t, i, j, jp, jt, u, v, z; /* indexes */
 
 int *SubNet, iSubNet; /* SubNets: subnet num of trs */
 int *R, *S, *X, *Y, *Q;
 int nR, nS, nX, nY, nQ; /* SubNet */

 int pinp, pout, belong; /* flags */

 int *SX, *SY; /* subnet num of input/output pos */
 int *GA; /* arcs of subnet graph */
 
 int *pl, *tr;
 int maxpl, maxtr, found, k;
 
 arcty *a;

// {METIS
 int * nts, ne; // number of subnet transitions; number of edges
// METIS}

 /* open files */
 NetFile = fopen( NetFileName, "r" );
 if( NetFile == NULL ) {printf( "*** error open file %s\n", NetFileName );exit(2);}
 if( debug>1 )
 {
   sprintf( SubNetFileName, "%s.lst", NetFileName );
   SubNetFile = fopen( SubNetFileName, "w" );
 }

 maxpl=initPL; pl=malloc( maxpl * sizeof( int ) );
 if( pl==NULL ) {printf("*** not enough memory for pl\n");exit(1);};
 maxtr=initTR; tr=malloc( maxtr * sizeof( int ) );
 if( tr==NULL ) {printf("*** not enough memory for tr\n");exit(1);};
   
 /* scan input to obtaine m, n, l */
 n=0; m=0; l=0;
 while( ! feof( NetFile ) )
 {
   fgets( str, MAXSTRLEN, NetFile);
   
   if( feof( NetFile) ) break;
   
   if( str[0]=='#' || isempty(str) ){ continue; }
   
   v=1;
   sscanf( str, "%d %d %d", &p, &t, &v );
   
   l++;
   
   found = 0;
   for( i=1; i<=m; i++ ) { if( pl[i]==p ){ found=1; break; } }
   if( ! found ) 
   { 
     if( m+1 >= maxpl )
     {
       maxpl+=deltaPL;
       pl = realloc( pl, maxpl*sizeof(int) );
       if( pl == NULL ) {printf("*** not enough memory for pl\n");exit(1);};
     }
     pl[ ++m ]=p; 
   }
 
   found = 0;
   for( j=1; j<=n; j++ ) { if( tr[j]==t ){ found=1; break; } }
   if( ! found ) 
   { 
     if( n+1 >= maxtr )
     {
       maxtr+=deltaTR;
       tr = realloc( tr, maxtr*sizeof(int) );
       if( tr == NULL ) {printf("*** not enough memory for tr\n");exit(1);};
     }
     tr[ ++n ]=t; 
   }
   
   /*m=max(m,p);
   n=max(n,t);  */
 }

/*printf("estimated sizes\n");*/
 
/* allocate arrays */
 a = calloc( l, sizeof( arcty ) );
 if( a==NULL ) {printf("*** not enough memory for a\n");exit(1);};
 
/* read net */
 rewind( NetFile );
 k=0;
 while( ! feof( NetFile ) )
 {
   fgets( str, MAXSTRLEN, NetFile); if( str[0]==';' ) continue;

   if( feof( NetFile ) ) break;
      
   if( str[0]=='#' || isempty(str) ){ continue; }
   
   v=1;
   sscanf( str, "%d %d %d", &p, &t, &v );
   
   found = 0;
   for( i=1; i<=m; i++ ) { if( pl[i]==p ){ found=1; break; } }
   if( ! found ) { printf( "*** error net read\n" ); exit(3); }
   
   found = 0;
   for( j=1; j<=n; j++ ) { if( tr[j]==t ){ found=1; break; } }
   if( ! found ) { printf( "*** error net read\n" ); exit(3); }
   
   a[k].p=i; a[k].t=j; a[k].v=v;
   k++;
 } 
 
/*printf("input matr\n");*/
 
 tx = calloc( n+2, sizeof(int) );
 ty = calloc( n+2, sizeof(int) );
 at = calloc( l+1, sizeof(int) ); fat=0;
 if( tx==NULL || ty==NULL || at==NULL ) {printf("*** not enough memory for tx,ty,at\n");exit(1);};
 
 /* create tx, ty, at */
 for( t=1; t<=n; t++)
 {
   tx[t]=fat;
   for( i=0; i<l; i++ )
     if( a[i].t==t && a[i].v<0 ) at[fat++]=a[i].p;

   ty[t]=fat;
   for( i=0; i<l; i++ )
     if( a[i].t==t && a[i].v>0 ) at[fat++]=a[i].p;

 }
 tx[n+1]=fat; /* fictive trs */
 
 px = calloc( m+2, sizeof(int) );
 py = calloc( m+2, sizeof(int) );
 ap = calloc( l+1, sizeof(int) ); fap=0;
 if( px==NULL || py==NULL || ap==NULL ) {printf("*** not enough memory for px,py,ap\n");exit(1);};

 SubNet = calloc( n+1, sizeof(int) );
 if( SubNet==NULL ) {printf("*** not enough memory for SubNet\n");exit(1);};
 for( t=1; t<=n; t++) SubNet[t]=0;
 iSubNet=1;

 R = calloc( n, sizeof(int) ); nR=0;
 S = calloc( n, sizeof(int) ); nS=0;
 X = calloc( m, sizeof(int) ); nX=0;
 Y = calloc( m, sizeof(int) ); nY=0;
 Q = calloc( m, sizeof(int) ); nQ=0;
 if( R==NULL || S==NULL || X==NULL || Y==NULL || Q==NULL )
   {printf("*** not enough memory for R,S,X,Y,Q\n");exit(1);};

 SX = calloc( m+1, sizeof(int) );
 SY = calloc( m+1, sizeof(int) );
 if( SX==NULL || SY==NULL ) {printf("*** not enough memory for SX,SY\n");exit(1);};
 for( p=1; p<=m; p++) { SX[p]=0; SY[p]=0; }

 /* create px, py, ap */
 for( p=1; p<=m; p++)
 {
   px[p]=fap;
   for( t=1; t<=n; t++ )
     for( i=ty[t]; i<tx[t+1]; i++ )
       if( at[i]==p ) ap[fap++]=t;

   py[p]=fap;
   for( t=1; t<=n; t++ )
     for( i=tx[t]; i<ty[t]; i++ )
       if( at[i]==p ) ap[fap++]=t;

 }
 px[m+1]=fap; /* fictive pos */

if( debug>1 ) {
 /* print input net */
 fprintf( SubNetFile, "n: %d m: %d l: %d\n", n, m, l );
 /* print trs */
 fprintf( SubNetFile, "at:\n" );
 for( t=1; t<=n; t++ )
 {
   for( i=tx[t]; i<ty[t]; i++ )
     fprintf( SubNetFile, "%d ", -at[i] );

   for( i=ty[t]; i<tx[t+1]; i++ )
     fprintf( SubNetFile, "%d ", at[i] );

   fprintf( SubNetFile, "\n" );
 }
 /* print pos */
 fprintf( SubNetFile, "ap:\n" );
 for( p=1; p<=m; p++ )
 {
   for( i=px[p]; i<py[p]; i++ )
     fprintf( SubNetFile, "%d ", ap[i] );

   for( i=py[p]; i<px[p+1]; i++ )
     fprintf( SubNetFile, "%d ", -ap[i] );

   fprintf( SubNetFile, "\n" );
 }
} /* if( debug ) */

 /* assign initial R set */
 t=1; R[0]=t; nR=1; SubNet[t]=iSubNet;

 // open decomposition digest
 sprintf( dFileName, "%s.d", NetFileName );
 dFile = fopen( dFileName, "w" );
 if( dFile == NULL ) {printf( "*** error open file %s\n", dFileName );exit(2);}

// {METIS
 nts = malloc(n*sizeof(int));
 if( nts==NULL ) {printf("*** not enough memory for nts\n");exit(1);};
// METIS}

 loop:
 /* create R borned net B(R)=(X,Q,Y) */
 nX=0; nY=0; nQ=0;
 for( p=1; p<=m; p++)
 {
   pinp=0; pout=0;
   for( jt=0; jt<nR; ++jt )
   {
     t=R[jt];
     for( i=tx[t]; i<ty[t]; i++ )
       if( at[i]==p ) pinp=1;
     for( i=ty[t]; i<tx[t+1]; i++ )
       if( at[i]==p ) pout=1;
   }
   if( pinp && pout ) { Q[nQ++]=p; SX[p]=0; SY[p]=0; }
     else if( pinp ) { X[nX++]=p; SX[p]=iSubNet; }
       else if ( pout ) { Y[nY++]=p; SY[p]=iSubNet; }
 }

 /* check is subnet B(R) closed: create new trs set S */
 nS=0;
 /* check output trs of X */
 for( jp=0; jp<nX; jp++)
 {
   p=X[jp]; /* input pos */
   for( i=py[p]; i<px[p+1]; i++)
   {
     t=ap[i]; /* it's output trs */
     belong=0; /* to R */
     for( jt=0; jt<nR; jt++)
       if( t==R[jt] ){ belong=1; break; }
     if( ! belong )
     {
       /* add new trs to S if absent */
       belong=0; /* to S */
       for( jt=0; jt<nS; jt++ )
	 if( t==S[jt] ){ belong=1; break; }
       if( ! belong ) S[nS++]=t;
     }
   }
 }
 /* check input trs of Y */
 for( jp=0; jp<nY; jp++)
 {
   p=Y[jp]; /* output pos */
   for( i=px[p]; i<py[p]; i++)
   {
     t=ap[i]; /* it's input trs */
     belong=0; /* to R */
     for( jt=0; jt<nR; jt++)
       if( t==R[jt] ){ belong=1; break; }
     if( ! belong )
     {
       /* add new trs to S if absent */
       belong=0; /* to S */
       for( jt=0; jt<nS; jt++ )
	 if( t==S[jt] ){ belong=1; break; }
       if( ! belong ) S[nS++]=t;
     }
   }
 }
 /* check input & otput trs of Q */
 for( jp=0; jp<nQ; jp++)
 {
   p=Q[jp]; /* internal pos */
   for( i=px[p]; i<px[p+1]; i++)
   {
     t=ap[i]; /* it's input or output trs */
     belong=0; /* to R */
     for( jt=0; jt<nR; jt++)
       if( t==R[jt] ){ belong=1; break; }
     if( ! belong )
     {
       /* add new trs to S if absent */
       belong=0; /* to S */
       for( jt=0; jt<nS; jt++ )
	 if( t==S[jt] ){ belong=1; break; }
       if( ! belong ) S[nS++]=t;
     }
   }
 }

 /* check ready condition: is subnet N(R) closed (empty S) */
 if( nS==0 ) goto final;

 /* expand subnet N(R): add S to R */
 for( jt=0; jt<nS; jt++)
 {
   t=S[jt];
   R[nR++]=t;
   SubNet[t]=iSubNet;
 }
 goto loop;

 final:
// {METIS
 nts[iSubNet]=nR;
// METIS}
 // print decomposition digest file
 // z r x q y
 fprintf( dFile, "%d ", iSubNet );
 fprintf( dFile, "%d ", nR );
 fprintf( dFile, "%d ", nX );
 fprintf( dFile, "%d ", nQ );
 fprintf( dFile, "%d \n", nY );

 /* print R, X, Q, Y */
 if( debug>1)
 {
   fprintf( SubNetFile, "N%d:\n", iSubNet );
   fprintf( SubNetFile, "R: " );
   for( jt=0; jt<nR; jt++ )
     fprintf( SubNetFile, "%d ", R[jt] );
   fprintf( SubNetFile, "\nX: " );
   for( jp=0; jp<nX; jp++ )
     fprintf( SubNetFile, "%d ", X[jp] );
   fprintf( SubNetFile, "\nQ: " );
   for( jp=0; jp<nQ; jp++ )
     fprintf( SubNetFile, "%d ", Q[jp] );
   fprintf( SubNetFile, "\nY: " );
   for( jp=0; jp<nY; jp++ )
     fprintf( SubNetFile, "%d ", Y[jp] );
   fprintf( SubNetFile, "\n" );
 }

 /* check are all trs processed */
 for( t=1; t<=n; t++ )
   if( SubNet[t]==0 ) /* non processed trs */
   {
     /* new subnet */
     R[0]=t; nR=1; SubNet[t]=++iSubNet;
     goto loop;
   }

 /* print subnet */
 if( debug>1 )
 {
   fprintf( SubNetFile, "TS:\n" );
   for( t=1; t<=n; t++ )
     fprintf( SubNetFile, "%d ", SubNet[t] );
   fprintf( SubNetFile, "\nSX:\n" );
   for( p=1; p<=m; p++ )
     fprintf( SubNetFile, "%d ", SX[p] );
   fprintf( SubNetFile, "\nSY:\n" );
   for( p=1; p<=m; p++ )
     fprintf( SubNetFile, "%d ", SY[p] );
   fprintf( SubNetFile, "\n" );
 }

 /* create & write graph of subnets */
 //revised for METIS
 sprintf( zFileName, "%s.p", NetFileName );
 zFile = fopen( zFileName, "w" );
 if( zFile == NULL ) {printf( "*** error open file %s\n", zFileName );exit(2);}

// {METIS
 sprintf( mFileName, "%s.metis", NetFileName );
 mFile = fopen( mFileName, "w" );
 if( mFile == NULL ) {printf( "*** error open file %s\n", mFileName );exit(2);} 

 for(i=0;i<20;i++)fputc(' ',mFile);
 fputc('\n',mFile);
 ne=0;
// METIS}

 GA = calloc( iSubNet+1, sizeof(int) );
 if( GA==NULL ) {printf("*** not enough memory for GA\n");exit(1);};
 for( u=1; u<=iSubNet; u++ ) /* subnets */
 {
   for( v=1; v<=iSubNet; v++ )
     GA[v]=0;
  
   /* input arcs */
   for( jp=1; jp<=m; jp++ )
     if( SX[jp]==u ) /* p is input pos of subnet u */
     {
       v=SY[jp]; /* v is input node of u */
//       if( v <= u ) continue;
       (GA[v])++;
     }
       
   /* output arcs */
   for( jp=1; jp<=m; jp++ )
     if( SY[jp]==u ) /* p is output pos of subnet u */
     {
       v=SX[jp]; /* v is output node of u */
//       if( v <= u ) continue;
       (GA[v])++;
     }

// {METIS
     fprintf( mFile, "%d ", nts[u] );
// METIS}

   for( v=1; v<=iSubNet; v++ )
     if( GA[v] != 0 ) 
     {
       if( v > u )fprintf( zFile, "%d %d %d\n", u, v, GA[v] );
// {METIS
       fprintf( mFile, "%d %d ", v, GA[v] );
       ne++;
// METIS}
     }
// {METIS
     fprintf( mFile, "\n" );
// METIS}
 }

 free( GA );
 fclose( zFile );

// {METIS
 fseek(mFile,0,SEEK_SET);
 fprintf(mFile,"%d %d 011", iSubNet, ne/2 );
 fclose(mFile);
 free(nts);
// METIS}

 /* write contact place info */
 sprintf( zFileName, "%s.z", NetFileName );
 zFile = fopen( zFileName, "w" );
 if( zFile == NULL ) {printf( "*** error open file %s\n", zFileName );exit(2);}
 fprintf( zFile, "%d\n", iSubNet );
 for( p=1; p<=m; p++ )
 {
   if( SX[p]>0 && SY[p]>0 )
     fprintf( zFile, "%d %d %d\n", pl[ p ], SX[p], SY[p] );
 }
 fclose( zFile );
 
 /* write files of subnets */
 for( z=1; z<=iSubNet; z++ )
 {
   sprintf( zFileName, "%s.z%d", NetFileName, z );
   zFile = fopen( zFileName, "w" );
   if( zFile == NULL ) {printf( "*** error open file %s\n", zFileName );exit(2);}
   for( i=0; i<l; i++ )
     if( SubNet[ a[i].t ]==z )
       fprintf( zFile, "%d %d %d\n", pl[ a[i].p ], tr[ a[i].t ], a[i].v );
   fclose( zFile );    
 }
 
 free(pl);free(tr);free(a);free(tx);free(ty);free(at);free(px);free(py);free(ap);
 free(SubNet);free(R);free(S);free(X);free(Y);free(Q);free(SX);free(SY);
 
 fclose( NetFile );
 fclose( dFile );
 if( debug>1 ) fclose( SubNetFile );
 
 return( iSubNet ); 

}/*--- DecomposeIntoFSN ---*/

#ifdef __MAIN__
int main( int argc, char *argv[] )
{
  char * NetFileName;
  int debug=0;
  int nz;
  
  if( argc < 2 ) return;
  NetFileName =  argv[1];
  if( argc > 2 ) debug = atoi(argv[2]);
  
  nz = DecomposeIntoFSN( NetFileName, debug );  
  printf("%d\n", nz); 
}
#endif

// @ 2019 Dmitry Zaitsev: daze@acm.org


