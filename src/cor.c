// ParAd: Parallel Adriana 
// cor1.c                              
// Collapse of a weighted graph

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cor.h"
#include "uti.h"

//#define LINESIZE 1024

//#define min(a,b) (((a)<(b))?(a):(b))
//#define max(a,b) (((a)>(b))?(a):(b))


int maxwe( struct edge *e, int n, int b )
{
  int mw, mi, i;
  
  mw = e[b].w;
  mi = b;
  
  for( i=b+1; i<n; i++) 
  {
    if( e[i].w < mw ) // min !!!
    {
        mw = e[i].w;
        mi = i;
    }
  }

  return mi;
  
} /* maxwe */

void sorte( struct edge *e, int n)
{
 int i, m, v1, v2, w;

  for( i=0; i<n-1; i++)
  {
    m = maxwe( e, n, i );
    
    v1 = e[i].v1;
    v2 = e[i].v2;
    w = e[i].w;
    
    e[i].v1 = e[m].v1;
    e[i].v2 = e[m].v2;
    e[i].w = e[m].w;
    
    e[m].v1 = v1;
    e[m].v2 = v2;
    e[m].w = w;
    
  }

}/* sorte */

void shiftr( struct edge *e, int *n, int d )
{
  int i;
 
  (*n)--;
  for( i=d; i < (*n); i++)
  {
    e[i].v1 = e[i+1].v1;
    e[i].v2 = e[i+1].v2;
    e[i].w = e[i+1].w;
  }
} /* shiftr */

void merge( struct edge * e, int *n, int m )
{
  int i, j;
  int v, v1, v2, v1c, v2c;
  
  v =  e[m].v1;
  v1 =  e[m].v1;
  v2 =  e[m].v2;
  
  /* merge edge */
  shiftr( e, n, m );
      
 /* renumerate nodes */
 for( i=0; i<(*n); i++)
 {
   v1c = e[i].v1;
   v2c = e[i].v2;
 
   if( v1c == v2 ) { e[i].v1 = min( v, v2c); e[i].v2 = max( v, v2c); }
      
   if( v2c == v2 ) { e[i].v1 = min( v, v1c); e[i].v2 = max( v, v1c); }
 } 
 
/* merge parell edges */
i=0;
while( i < (*n) )
{
  
  j = i+1;
  while( j < (*n) )
  {
    if( (e[i].v1 == e[j].v1) && (e[i].v2 == e[j].v2))
    {
      e[i].w+=e[j].w;
      shiftr( e, n, j );
    }
    else
      j++;
  }
  
  i++;
  
}

 } /* merge */

int CollapseOfGraph( char ty, char * GraphFileName, char * TraceFileName )
{
  char line[ MAXSTRLEN+1 ];
  FILE * InputFile, * OutputFile;
  struct edge *e;
  int n, i, j;
  int b, f, w;
  int maxw, m;
  int ri, maxc, maxn;
  double r;
  int sw;
  double per;
  int k;
  
  InputFile = fopen( GraphFileName, "r" );
  if( InputFile == NULL ) {printf( "*** error open file %s\n", GraphFileName );exit(2);}
  OutputFile = fopen( TraceFileName, "w" );
  if( OutputFile == NULL ) {printf( "*** error open file %s\n", TraceFileName );exit(2);}

  n=0;
  while ( ! feof( InputFile ) )
  {
    fgets( line, MAXSTRLEN, InputFile );

    if( line[0] == ';' || feof(InputFile) ) continue;
      
    n++;
  } /* feof InputFile */
  
  e = calloc( n, sizeof(struct edge) );
  if( e == NULL )
  {
    printf( "*** not enough memory for e\n" );
    exit( 1 );
  }
  rewind ( InputFile );
  
  i = 0; sw=0;
  while ( ! feof( InputFile ) )
  {
    fgets( line, MAXSTRLEN, InputFile );

    if( line[0] == ';' || feof(InputFile ) ) continue;
        
    w=1;
    sscanf( line, "%d %d %d", &b, &f, &w );
        
    e[i].v1 = min( b, f );
    e[i].v2 = max( b,f );
    e[i].w = w;
    
    sw+=w;
    
    if( b == f ) {printf("*** error input file: loop %d %d\n", b, f ); return 2;}
    for( j=0; j<i; j++)
      if( (e[j].v1==e[i].v1) && (e[j].v2==e[i].v2) )  {printf("*** error input file: duplicated arcs %d %d\n", b, f ); return 2;}
    
    i++;
      
  } /* feof InputFile */

  fclose( InputFile );

  maxw=0; 
  k=0;
  while( n > 0 )
  {
    sorte( e,n );
    
    /*fprintf( OutputFile, "------------------------------------------\n" );
    for( i=0; i<n; i++ )
      { fprintf( OutputFile, " %d %d %d\n", e[i].v1, e[i].v2, e[i].w ); }*/
    
      
    switch( ty )
    {
      case 'f': m = 0; break;
      
      case 'm':
        /* random choice of max */
        maxc = e[0].w;
        maxn = 1;
        while( e[maxn].w==maxc && maxn<n ) maxn++;
        
	r = rand();
        ri=r*maxn/RAND_MAX;  
    
        m=ri-1; if( m < 0 ) m=0; if( m>maxn-1 ) m=maxn-1;
	break;
      
      case 'r':
        r = rand();
        ri=r*n/RAND_MAX;  
    
        m=ri-1; if( m < 0 ) m=0; if( m>n-1 ) m=n-1;
        break;
	
      case 'i': m = n-1; break;
    
    default: m=0;
    }  
       
    /*fprintf( OutputFile, "mergenum %d of %d\n",m, n );*/
     
    maxw = max( maxw, e[m].w );
    
    /*fprintf( OutputFile, "merge %d %d %d\n", e[m].v1, e[m].v2, e[m].w );*/
    
    fprintf( OutputFile, "%d %d\n", e[m].v1, e[m].v2 );
    
    merge(  e, &n, m );
    k++;
      
  } /* while */

  /*per = maxw*100.0/sw;
  fprintf( OutputFile, "sumw: %d       maxw: %d        per: %e\n", sw, maxw, per );*/
     
  free( e );
  fclose( OutputFile );
  
  return( k );

} /* CollapseOfGraph */


int ReadGraph( char * GraphFileName, struct edge **ae, int *nn )
{
  char line[ MAXSTRLEN+1 ];
  FILE * InputFile;
  struct edge *e;
  int n, i, j;
  int b, f, w, sw;
  
  InputFile = fopen( GraphFileName, "r" );
  if( InputFile == NULL ) {printf( "*** error open file %s\n", GraphFileName );exit(2);}
  
  n=0;
  while ( ! feof( InputFile ) )
  {
    fgets( line, MAXSTRLEN, InputFile );

    if( line[0] == ';' || feof(InputFile) ) continue;
      
    n++;
  } /* feof InputFile */
  
  e = calloc( n, sizeof(struct edge) );
  if( e == NULL )
  {
    printf( "*** not enough memory for e\n" );
    exit( 1 );
  }
  rewind ( InputFile );
  
  i = 0; sw=0;
  while ( ! feof( InputFile ) )
  {
    fgets( line, MAXSTRLEN, InputFile );

    if( line[0] == ';' || feof(InputFile ) ) continue;
        
    w=1;
    sscanf( line, "%d %d %d", &b, &f, &w );
        
    e[i].v1 = min( b, f );
    e[i].v2 = max( b,f );
    e[i].w = w;
    
    sw+=w;
    
    if( b == f ) {printf("*** error input file: loop %d %d\n", b, f ); return 2;}
    for( j=0; j<i; j++)
      if( (e[j].v1==e[i].v1) && (e[j].v2==e[i].v2) )  {printf("*** error input file: duplicated arcs %d %d\n", b, f ); return 2;}
    
    i++;
      
  } /* feof InputFile */

  fclose( InputFile );

  *ae=e;
  *nn=n;
  return(0);

} /* ReadGraph */

 
int ChooseEdge(char ty, struct edge *e, int *n, struct edge *pe, int *pn) // for ty='f' only
{
    int nn=*n, npn=*pn;
    int i, j, v;
//printf("ChooseEdge\n");
    sorte( e,nn );

    for(i=0;i<nn;i++)
    {
      v=1;
      for(j=0;j<npn;j++) 
      { 
        if(e[i].v1==pe[j].v1 || e[i].v1==pe[j].v2 || e[i].v2==pe[j].v1 || e[i].v2==pe[j].v2 )
        {
          v=0;
          break;
        }
      }
      if(!v) continue;

      pe[npn].v1=e[i].v1;
      pe[npn].v2=e[i].v2;
      pe[npn].w=e[i].w;
      shiftr( e, n, i );

      (*pn)++;
      return 1;
    }
    return 0;

} /* ChooseEdge */


int CollapseEdge(struct edge *ce, struct edge *e, int *n, struct edge *pe, int *pn)
{
  int nn=*n, npn=*pn, i, w;

  // find ce index in pe
  for(i=0;i<npn;i++)
  {
    if(pe[i].v1==ce->v1 && pe[i].v2==ce->v2) break;
  }
  // better check here id not found
  
  e[nn].v1=pe[i].v1;
  e[nn].v2=pe[i].v2;
  e[nn].w=pe[i].w;
  w=e[nn].w;
  shiftr( pe, pn, i );
  (*n)++;

  merge(  e, n, (*n)-1 );

  return w;

} /* CollapseEdge */


#ifdef __MAIN__
int main( int argc, char * argv[] )
{
  int c, k;
  
  if (argc < 4 )
  {
    printf( "Collapse of weighted graph\n");
    printf( "USAGE: COR s InFile OutFile\n" );
    printf( "s: f - first max, m - random max, r - random, i - last min\n" );
    return 3;
  }
  c = argv[1][0];
  if( strchr( "fmri", c ) == NULL ) { printf( "*** error: wrong switch %c\n", argv[1][0] ); return 2; }
  
  k=CollapseOfGraph( c, argv[2], argv[3] );
  printf( "%d\n", k );
}
#endif



#ifdef __MAIN1__
int main( int argc, char * argv[] )
{
  int c, k;
  struct edge *ce, *e, *pe;
  int n, pn=0, maxw=0, i; 
  
  if (argc < 4 )
  {
    printf( "Parallel-sequential collapse of weighted graph\n");
    printf( "USAGE: COR s InFile OutFile\n" );
    printf( "s: f - first max, m - random max, r - random, i - last min\n" );
    return 3;
  }
  c = argv[1][0];
  if( strchr( "fmri", c ) == NULL ) { printf( "*** error: wrong switch %c\n", argv[1][0] ); return 2; }
  
  // k=CollapseOfGraph( c, argv[2], argv[3] );

  ReadGraph( argv[2], &e, &n );
/*printf("has read graph n=%d\n", n);
printf( "------------------------------------------\n" );
    for( i=0; i<n; i++ )
      { printf( " %d %d %d\n", e[i].v1, e[i].v2, e[i].w ); }*/
  pe = calloc( n, sizeof(struct edge) );
  ce = calloc( 1, sizeof(struct edge) );

  while(1)
  {
     if(!ChooseEdge('f', e, &n, pe, &pn)) break;
//printf("chosen result n=%d pn=%d\n", n,pn);
     ce->v1=pe[pn-1].v1;
     ce->v2=pe[pn-1].v2;
     ce->w=pe[pn-1].w;
     maxw = max( maxw, ce->w );
//printf("chosen edge %d %d %d\n", ce->v1, ce->v2, ce->w);
 
     CollapseEdge(ce, e, &n, pe, &pn);
printf("%d %d %d\n", ce->v1, ce->v2, ce->w);
  }
  free(pe);
  free(ce);

  printf( "%d\n", maxw );
}
#endif

// @ 2018 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org

