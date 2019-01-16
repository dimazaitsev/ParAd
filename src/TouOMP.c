/* ParTou: TouOMP.c: */
/* Solves a homogenous Diophantine system in ninegative integer numbers */
/* by Toudic method on multicore parallel architectures using OpenMP    */

// schedule(dynamic, 10)
// #define __MAIN__

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "uti.h"

#include <time.h>
#include <bits/time.h>

size_t daze;

#define LINESIZE 4096

#define ec( c, i, j, nc ) (*((c)+(((i)-1)*(nc))+((j)-1)))
#define ex( x, i, j, nx ) (*((x)+(((i)-1)*(nx))+((j)-1)))
#define eb( b, i )    (*((b)+((i)-1)))

#define lc( c, i, nc ) ((c)+(((i)-1)*(nc)))
#define lx( x, i, nx ) ((x)+(((i)-1)*(nx)))

#define zsign(x) (((x)<0)?-1:(((x)==0)?0:1))

struct Compare { int val; int index; };   

#define initROWS 1024
#define deltaROWS 1024
#define initCOLS 1024
#define deltaCOLS 1024
#define deltaSOL 1024

// moved from Toudic parameters
static int n_th = 0;

void onex( int * X, int mx, int nx )
{
 int i;

 for( i=1; i<=mx; ++i )
   ex( X, i, i, nx )=1;
} /* onex */

void zerob( int * B, int mx )
{
 int i;

 for( i=1; i<=mx; ++i )
   eb( B, i )=0;
} /* zerob */

int nod( int y, int z )
{
 int a=y;
 int b=z;

 while( a != b )
 {
   if(a>b) a=a-b; else b=b-a;
 }

 return( a );
 
} /* nod */

int nodv( int * a, int n )
{
 int i,d;
 int yes=0;

 for( i=0; i<n; ++i) if(a[i]!=0) { yes=1; break;}

 if(!yes) return 0;

 d=abs(a[i]);

 for( i=0; i<n; ++i) if(a[i]!=0) { d=nod(d,abs(a[i]));}

 return d;

} /* nodm */

void divv( int * a, int n, int d )
{
 int i;

 for( i=0; i<n; ++i) if(a[i]!=0) {a[i]=a[i]/d;}

} /* nodm */

void printBCX(int *b, int *c, int mc, int nc, int *x, int mx, int nx)
{
  int i,j;

  printf("C:\n");
  for(i=1;i<=mc;i++)
  {
    printf("   ");
    for(j=1;j<=nc;j++)
    {
      printf("%2d",ec(c,i,j,nc));
    }
    printf("\n");
  }

  printf("BX:\n");
  for(i=1;i<=mx;i++)
  {  
    printf("%2d:",eb(b,i));
    for(j=1;j<=nx;j++)
    {
      printf("%2d",ex(x,i,j,nx));
    }
    printf("\n");
  }
}


int Toudic( char * SolverName, char * InputFileName, char * OutputFileName, int debug_level )
{
  char line[ LINESIZE+1 ];
  FILE * InputFile, * OutputFile;
  int * C, * C1;
  int * X, * X1;
  int * B, * B1;
  int m, nc, nx, m1;
  int col, nIp, nIz, nIm;
  int ilp, ilm, x1gex2, x2gex1;
  int * Im, * Ip, * Iz;
  int i, j, k, i1, i2, ii;
  int v, found;
  int lm, lz, lp;
  int cm, cp, d, d1, d2;
  int p, t, np, nm, nn;
  int * rows, * cols; 
  int nr, ret=0, nth, nos;
  int maxrows, maxcols;
  struct Compare mj,mj1;

  if( strcmp( InputFileName, "-" )==0 ) InputFile = stdin;
   else InputFile = fopen( InputFileName, "r" );
  if( InputFile == NULL ) {fprintf( stderr, "*** error open file %s\n", InputFileName );exit(2);}
  if( strcmp( OutputFileName, "-" )==0 ) OutputFile = stdout;
   else OutputFile = fopen( OutputFileName, "w" );
  if( OutputFile == NULL ) {fprintf( stderr, "*** error open file %s\n", OutputFileName );exit(2);}

  maxrows = initROWS; rows = malloc( maxrows*sizeof(int) );
  maxcols = initCOLS; cols = malloc( maxcols*sizeof(int) );
  if( rows==NULL || cols==NULL ) {fprintf( stderr, "*** not enough memory for rows,cols\n");exit(1);};

  /* estimate matrix's size */
  m=nc=0;
  while ( ! feof( InputFile ) )
  {
    fgets( line, LINESIZE, InputFile );
    
    if( feof( InputFile ) ) break;

    if( line[0] == '#' || isempty(line) ) continue;
    
    v=1;
    sscanf( line, "%d %d %d", &p, &t, &v );
    
    found=0;
    for( i=1; i<=m; i++ ) if( rows[i]==p ) { found=1; break; }
    if( ! found )
    { 
      if( m+1 >= maxrows )
      {
        maxrows+=deltaROWS;
	rows = realloc( rows, maxrows*sizeof(int) );
	if( rows == NULL ) {fprintf( stderr, "*** not enough memory for rows\n" );exit(1);};
      }
      rows[++m]=p; 
    }
    found=0;
    for( j=1; j<=nc; j++ ) if( cols[j]==t ) { found=1; break; }
    if( ! found ) 
    { 
      if( nc+1 >= maxcols ) 
      {
        maxcols+=deltaCOLS;
	cols = realloc( cols, maxcols*sizeof(int) );
	if( cols == NULL ) {fprintf( stderr, "*** not enough memory for cols\n" );exit(1);};
      } 
      cols[++nc]=t; 
    }
     
  } /* feof InputFile */

if(debug_level>1) printf("=== Toudic: rows m=%d, cols nc=%d\n",m,nc);
  
  rewind( InputFile );
  rows = realloc( rows, (m+1)*sizeof(int) );
  cols = realloc( cols, (nc+1)*sizeof(int) );
  C = calloc( m*nc, sizeof(int) );
  if( rows==NULL || cols==NULL || C==NULL ) {fprintf( stderr, "*** not enough memory for rows,cols,C\n" );exit(1);}

  /* read matrix */
  while ( ! feof( InputFile ) )
  {
    fgets( line, LINESIZE, InputFile );
    
    if( feof( InputFile ) ) break;

    if( line[0] == '#' || isempty(line) ) continue;
    
    v=1;
    sscanf( line, "%d %d %d", &p, &t, &v );
    
    found=0;
    for( i=1; i<=m; i++ ) if( rows[i]==p ) { found=1; break; }
    if( ! found ) { fprintf( stderr, "***inconsistent input" ); exit(4); }
    found=0;
    for( j=1; j<=nc; j++ ) if( cols[j]==t ) { found=1; break; }
    if( ! found ) { fprintf( stderr, "***inconsistent input" ); exit(4); }
    
    ec( C, i, j, nc )+=v;    
     
  } /* feof InputFile */

  if( InputFile != stdin ) fclose( InputFile );

  nx=m;
  X = calloc( m*nx, sizeof(int) );
  if( X==NULL ) { fprintf( stderr, "*** not enough memory for X\n" ); exit(1); };
  onex( X, m, nx );
  B = calloc( m, sizeof(int) );
  if( B==NULL ) { fprintf( stderr, "*** not enough memory for X\n" ); exit(1); };
  zerob( B, m );

  omp_set_nested(1); 
  nth = (n_th == 0)? omp_get_max_threads(): n_th;

  if(debug_level>1) printf("number of threads: %d\n",nth);

  // clear columns of C

  while( nc>0 )
  {
if(debug_level>2) printBCX(B,C,m,nc,X,m,nx);
    Iz=malloc( m*sizeof(int) );
    if( Iz==NULL ) { fprintf( stderr, "*** not enough memory for Iz\n" ); exit(1); };
    Ip=malloc( m*sizeof(int) );
    Im=malloc( m*sizeof(int) );
    if( Ip==NULL || Im==NULL || Iz==NULL ) { fprintf( stderr, "*** not enough memory for Ip,Im,Iz\n" ); exit(1); };
if(debug_level>1) printf("=== Toudic: before findcol m=%d nc=%d\n", m, nc); 

    // mark rows as absent for coloumns which contain variables of the same sign

    do {
      nos=0;
  
      #pragma omp parallel for private(j,i,np,nm,nos) num_threads(nth) 
      for(j=1;j<=nc;j++) 
      {      
        np=nm=0;
          #pragma omp simd reduction(+:np,nm)
          #pragma unroll
          for(i=1;i<=m;i++) 
          { 
            if(eb(B,i)>0) continue;
            np+=(ec(C,i,j,nc)>0)?1:0;
            nm+=(ec(C,i,j,nc)<0)?1:0;
          } 
        
        if(np==0&&nm>0 || nm==0&&np>0)
        {
            for(i=1;i<=m;i++)
            {
              #pragma omp critical
              {     
                if(eb(B,i)==0){         
                if(ec(C,i,j,nc)!=0)eb(B,i)=1; 
                nos++;}
              }              
            } 
          } 
        }

    } while(nos>0);

    // find the best column

    mj.val=INT_MAX; mj.index=-1;
      #pragma omp parallel for private(j,i,np,nm,nn) num_threads(nth) 
      for(j=1;j<=nc;j++)
      {      
        np=nm=0;
 
         #pragma omp simd reduction(+:np,nm) 
         #pragma unroll
          for(i=1;i<=m;i++)
          {
            if(eb(B,i)>0) continue;
            np+=(ec(C,i,j,nc)>0)?1:0;
            nm+=(ec(C,i,j,nc)<0)?1:0;
          } 
        
        nn=np*nm;
        nn=(nn>0)?nn:INT_MAX;
        #pragma omp critical
        {
           if(nn<mj.val)
           {
             mj.val=nn;
             mj.index=j;
           }
        }  
      }
    col=mj.index;
    if(mj.index<0) {col=0; break;}
 
    // create minus-zero-pluz indices
    nIp=nIm=nIz=0;
    for(i=1;i<=m;i++)
    {
      if(eb(B,i)>0) continue;
      if(ec(C,i,col,nc)>0) Ip[nIp++]=i;
      else if(ec(C,i,col,nc)<0) Im[nIm++]=i;
      else Iz[nIz++]=i;     
    } 
     
if(debug_level>1) printf("=== Toudic: findcol col=%d nIz=%d nIp=%d nIm=%d\n", col,nIz,nIp,nIm);
 
    m1=nIz+(nIp*nIm);
    C1 = calloc( m1*nc, sizeof(int) );
    X1 = calloc( m1*nx, sizeof(int) );
    B1 = calloc( m1, sizeof(int) );
    if( C1==NULL || X1==NULL || B1==NULL ) { fprintf( stderr, "*** not enough memory for C1,X1,B1\n" ); exit(1); };
    
    // copy rows for zero coefficients in the column col

      #pragma omp parallel for private(i,k) num_threads(nth) 
      for( i=0; i<nIz; i++ )
      {
        int *C1b=lc( C1, i+1, nc );
        int *Cbz=lc( C, Iz[i], nc );
        #pragma omp simd 
        #pragma unroll
	for( k=0; k<nc; k++ ) // C(1<=nc)
	{ 
            // ec( C1, i+1, k, nc ) = ec( C, Iz[i], k, nc ); 
            C1b[k]=Cbz[k];
        }
        int *X1b=lx( X1, i+1, nx );
        int *Xbz=lx( X, Iz[i], nx );
        #pragma omp simd
        #pragma unroll
	for( k=0; k<nx; k++ ) // X(1<=nc)
	{ 
            // ex( X1, i+1, k, nx ) = ex( X, Iz[i], k, nx ); 
            X1b[k]=Xbz[k];
        } 
        eb( B1, i+1 )=0;
      }

if(debug_level>1) printf("=== Toudic: copied zero rows nIz=%d\n", nIz); 


    // combine rows for each plus-minus pair

      #pragma omp parallel for private(i1,i2,ii,k,ilp,ilm,cp,cm,d,d1,d2) num_threads(nth) // collapse(2) 
      for(i1=0;i1<nIm;i1++)
      {
        for(i2=0;i2<nIp;i2++) 
        {
          ilp=Ip[i2];
    	  ilm=Im[i1];
          cp=abs( ec( C, ilp, col, nc ) ); 
	  cm=abs( ec( C, ilm, col, nc ) );
	  d=nod( cp, cm );
	  if( d > 1 ) 
	  { 
	    cp=cp/d;
	    cm=cm/d;
	  }
	 
         ii=nIz+i1*nIp+i2+1;
         int *C1b=lc( C1, ii, nc );
         int *Cbp=lc( C, ilp, nc );
         int *Cbm=lc( C, ilm, nc );
         #pragma omp simd 
         #pragma unroll	
	 for( k=0; k<nc; k++ ) // old 1<=nc 
         {
	   // ec( C1, ii, k, nc ) = ec( C, ilp, k, nc )*cm + ec( C, ilm, k, nc )*cp; 
           C1b[k]=Cbp[k]*cm + Cbm[k]*cp;
         }
         int *X1b=lx( X1, ii, nx );
         int *Xbp=lx( X, ilp, nx );
         int *Xbm=lx( X, ilm, nx );
	 #pragma omp simd
         #pragma unroll 
         for( k=0; k<nx; k++ ) // old 1<=nx
         {
	   // ex( X1, ii, k, nx ) = ex( X, ilp, k, nx )*cm + ex( X, ilm, k, nx )*cp; 
           X1b[k]=Xbp[k]*cm + Xbm[k]*cp;
         }
         eb( B1, ii )=0;
	  
	 // Reduce New Solution by GCD
	 d1=nodv( C1b, nc ); 
         d2=nodv( X1b, nx );
         if( (d1>1 || d1==0) && (d2>1) )
	 {    
	   if( d1==0 ) d=d2; else d=nod(d1,d2); 
           #pragma omp simd 
           #pragma unroll
           for( k=0; k<nc; k++ ) // old 1<=nc
           {
	     // ec( C1, ii, k, nc )/=d;
             C1b[k]/=d; 
           }
           #pragma omp simd 
           #pragma unroll
           for( k=0; k<nx; k++ ) // old 1<=nc
           {
	     // ex( X1, ii, k, nx )/=d; 
             X1b[k]/=d; 
           }
	 }

        }
      }
    
if(debug_level>1) printf("=== Toudic: combined plus-minis pairs nIm=%d nIp=%d (nIz=%d)\n", nIm, nIp, nIz);

    free(Ip); free(Im); free(Iz); 
    free( C ); C = C1; m=m1;
    free( X ); X = X1; 
    free( B ); B = B1;

    // Filter Solutions
 
      #pragma omp parallel for private(i1,i2,j,x1gex2,x2gex1) num_threads(nth) //collapse(2) 
      for(i1=nIz+1;i1<=m;i1++)
      {
        for(i2=1;i2<=m;i2++) 
        { 
          if(i2>=i1) continue;
          x1gex2=x2gex1=1;
          int *Xi1=lx( X, i1, nx );
          int *Xi2=lx( X, i2, nx );
          #pragma omp simd reduction(&&:x1gex2,x2gex1)
          #pragma unroll
            for(j=0;j<nx;j++) //old 1<=nx
            {  
              x1gex2=x1gex2&&(zsign(Xi1[j])>=zsign(Xi2[j]));
              x2gex1=x2gex1&&(zsign(Xi2[j])>=zsign(Xi1[j]));
            }
         
          if(x2gex1) {
            #pragma omp atomic
            eb( B, i2 )++; 
          } else if(x1gex2) {
            #pragma omp atomic
            eb( B, i1 )++;   
          }else ;
       
        }
      }
   
if(debug_level>1) printf("=== Toudic: filtered solutions m=%d\n", m);

  } /* while */
 
if(debug_level>2) printBCX(B,C,m,nc,X,m,nx);

if(debug_level>1) printf("=== Toudic: write inv m=%d nc=%d\n", m, nc);
   
  if( col == 0 )
  { 
    k=0;
    for( i=1; i<=m; ++i )
    { 
      if(eb(B,i)>0) continue;
      k++;
      for( j=1; j<=nx; ++j )
	if( ex( X, i, j, nx ) != 0 ) fprintf( OutputFile, "%d %d %d\n", k, rows[j], ex( X, i, j, nx ) );
    }
    ret=0;
  }
  else
  { fprintf( OutputFile, "# dust\n" ); ret=2;}
     
  free(C); free(X); free(rows); free(cols);
  
  if(OutputFile != stdout ) fclose( OutputFile );
  
  return( ret );
    
} /* Toudic */

#ifdef __MAIN__
int main( int argc, char * argv[] )
{
  if (argc < 3 )
  {
    printf( "Solve linear homogenious Diophantine system solution by Toudic method\n");
    printf( "Usage: TouOMP <inputfile> <outputfile>\n" );
    printf( "<inputfile>, <outputfile> : SPM format: i, j, e(i,j)\n" );
    printf( "(c) 2018 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: zaitsevd@icl.utk.edu\n" );
    return 3;
  }
  Toudic( "", argv[1], argv[2], 0 );
} 
#endif

// @ 2018 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org

