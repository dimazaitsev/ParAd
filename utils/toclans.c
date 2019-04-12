// ParAd: Parallel Adriana - utilities: toclans
//

static char Help[] =
"Decompose a sparse matrix (with real elements) into clans.\n"
">toclans sparse_matrix_file_name [debug_level]\n"
"Matrix Market format (.mtx), header lines (starting from \%) are skipped.\n"
"debug_level greater than 0 gives more detailed output and saves files with clans.\n\n";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
//#include "uti.h"

#define MAXINPSTRLEN 256
#define FILENAMELEN 256
//#define initPL 1024
//#define deltaPL 1024
//#define initTR 1024
//#define deltaTR 1024
#define sign(x) (((x)<0)?-1:((x)>0)?1:0)

#define __MAIN__

struct netarc {
  int p;
  int t;
  int v;
  double x;
};

typedef struct netarc arcty;

int isempty( char * s )
{
  int i=0;
  int empty=0;
  
  while( ( s[i]==' ' || s[i]==0xa || s[i]==0xd || s[i]==0x9 ) && s[i]!='\0'  ) i++;
  if( s[i]=='\0' ) empty=1;
  
  return(empty);

} /* isempty */

int DecomposeIntoFSN( char * NetFileName, int debug )
{
// int debug = 0;

 char SubNetFileName[ FILENAMELEN + 1];
 char zFileName[ FILENAMELEN + 1];
 FILE * NetFile, * SubNetFile, * zFile;
 char str[ MAXINPSTRLEN + 1 ]; /* buffers */

 int n, m, l; /* net size */
 int *tx, *ty, *at, fat; /* trs */
 int *px, *py, *ap, fap; /* pos */

 int p, t, i, j, jp, jt, u, v, z, k; /* indexes */
 double x;
 
 int *SubNet, iSubNet; /* SubNets: subnet num of trs */
 int *R, *S, *X, *Y, *Q;
 int nR, nS, nX, nY, nQ; /* SubNet */

 int pinp, pout, belong; /* flags */

 int *SX, *SY; /* subnet num of input/output pos */
 int *GA, nGA; /* arcs of subnet graph */
 
 //int *pl, *tr;
 //int maxpl, maxtr, found, k;
 
 arcty *a;

 /* open files */
 NetFile = fopen( NetFileName, "r" );
 if( NetFile == NULL ) {printf( "*** error open file %s\n", NetFileName );exit(2);}
 if( debug )
 {
//   sprintf( SubNetFileName, "%s.lst", NetFileName );
//   SubNetFile = fopen( SubNetFileName, "w" );
SubNetFile=stdout;
 }

 /* scan input to obtaine m, n, l */

 while( ! feof( NetFile ) )
 {
   fgets( str, MAXINPSTRLEN, NetFile); if( str[0]==';' ) continue;

   if( feof( NetFile ) ) break;
      
   if( str[0]!='%' && !isempty(str) ) break;
 }
 if( feof( NetFile ) ) {printf("*** input file format\n");exit(1);};
 sscanf( str, "%d %d %d", &m, &n, &l );

if(debug>1) printf("m=%d, n=%d, l=%d\n",m,n,l);
  
 /* allocate arrays */
 
 a = calloc( l, sizeof( arcty ) );
 if( a==NULL ) {printf("*** not enough memory for a\n");exit(1);};
 
/* read net */
 k=0;
 while( ! feof( NetFile ) )
 {
   fgets( str, MAXINPSTRLEN, NetFile); if( str[0]==';' ) continue;

   if( feof( NetFile ) ) break;
      
   if( str[0]=='%' || isempty(str) ){ continue; }
   
   x=1;
   sscanf( str, "%d %d %lf", &p, &t, &x );

if(debug>1) printf("%d %d %lf\n",p,t,x);
   
   a[k].p=p; a[k].t=t; a[k].x=x; a[k].v=sign(x);
   k++;
 } 
 
if(debug>1) printf("input matr k=%d\n",k);
 
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

if(debug>1) printf("created at\n");
 
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

if(debug>1) printf("created px,py,ap\n");

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

if(debug>1) printf("finished loop\n");

 final:
 /* print R, X, Q, Y */
 if( debug)
 {
   fprintf( SubNetFile, "Z%d(%d):\n", iSubNet,nR );
if( debug>1)
 {
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
if( debug > 2 )
{
 sprintf( zFileName, "__%s.p", NetFileName );
 zFile = fopen( zFileName, "w" );
 if( zFile == NULL ) {printf( "*** error open file %s\n", zFileName );exit(2);}
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
       if( v <= u ) continue;
       (GA[v])++;
     }
       
   /* output arcs */
   for( jp=1; jp<=m; jp++ )
     if( SY[jp]==u ) /* p is output pos of subnet u */
     {
       v=SX[jp]; /* v is output node of u */
       if( v <= u ) continue;
       (GA[v])++;
     }
     
   for( v=1; v<=iSubNet; v++ )
     if( GA[v] != 0 ) 
       fprintf( zFile, "%d %d %d\n", u, v, GA[v] );
   
 }
 free( GA );
 fclose( zFile );
}

 /* write contact place info */
if( debug > 2 )
{
 sprintf( zFileName, "__%s.z", NetFileName );
 zFile = fopen( zFileName, "w" );
 if( zFile == NULL ) {printf( "*** error open file %s\n", zFileName );exit(2);}
 fprintf( zFile, "%d\n", iSubNet );
 for( p=1; p<=m; p++ )
 {
   if( SX[p]>0 && SY[p]>0 )
     fprintf( zFile, "%d %d %d\n", p, SX[p], SY[p] );
 }
 fclose( zFile );
}
 
 /* write files of subnets */
if( debug > 2 )
{
 for( z=1; z<=iSubNet; z++ )
 {
   sprintf( zFileName, "__%s.z%d", NetFileName, z );
   zFile = fopen( zFileName, "w" );
   if( zFile == NULL ) {printf( "*** error open file %s\n", zFileName );exit(2);}
   for( i=0; i<l; i++ )
     if( SubNet[ a[i].t ]==z )
       fprintf( zFile, "%d %d %d\n", a[i].p, a[i].t, a[i].v );
   fclose( zFile );    
 }
} 

 free(a);free(tx);free(ty);free(at);free(px);free(py);free(ap);
 free(SubNet);free(R);free(S);free(X);free(Y);free(Q);free(SX);free(SY);
 
 fclose( NetFile );
// if( debug ) fclose( SubNetFile );
 
 return( iSubNet ); 

}/*--- DecomposeIntoFSN ---*/

#ifdef __MAIN__
int main( int argc, char *argv[] )
{
  char * NetFileName;
  int details = 0;
  int nz;

  if( argc < 2 ) {printf("%s",Help); return(1);}
  NetFileName =  argv[1];
  if( argc > 2 ) details = atoi(argv[2]);
  
  nz = DecomposeIntoFSN( NetFileName, details );  
  printf("%d\n", nz); 
  return(nz);
}
#endif
