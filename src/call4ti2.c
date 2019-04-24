// ParAd: Parallel Adriana 
// call4ti2.c                                   
// Solve a linear Diophantine homogeneous system running "zsolve" from 4ti2    

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "uti.h"

size_t daze;

//#define LINESIZE 1024
//#define FILENAMELEN 1024

/* sizes of matrices */
static int mc, nc, nx;

#define ec( c, i, j ) (*((c)+(((i)-1)*nc)+((j)-1)))
#define ex( x, i, j ) (*((x)+(((i)-1)*nx)+((j)-1)))

#define lc( c, i ) ((c)+(((i)-1)*nc))
#define lx( x, i ) ((x)+(((i)-1)*nx))

#define sizelc (nc*sizeof(int))
#define sizelx (nx*sizeof(int))

#define initROWS 1024
#define deltaROWS 1024
#define initCOLS 1024
#define deltaCOLS 1024
#define deltaSOL 1024

int zsolve4ti2( char * SolverName, char * InputFileName, char * OutputFileName, int debug )
{
  char line[ MAXSTRLEN+1 ];
  char ti42_InputFileName[ FILENAMELEN+1 ];
  char ti42_OutputFileName[ FILENAMELEN+1 ];
  char command[ FILENAMELEN+1 ];
  FILE * InputFile, * OutputFile, * ti42_InputFile, * ti42_OutputFile;
  int *C;
  int i, j;
  int v, found;
  int in, p, t;
  int * rows, * cols; 
  int maxrows, maxcols;
  int ret=0;

if(debug>1)printf("4ti2 for %s %s\n",InputFileName, OutputFileName);

  InputFile = fopen( InputFileName, "r" );
  if( InputFile == NULL ) {printf( "*** error open file %s\n", InputFileName );exit(2);}
  OutputFile = fopen( OutputFileName, "w" );
  if( OutputFile == NULL ) {printf( "*** error open file %s\n", OutputFileName );exit(2);}
  
  maxrows = initROWS; rows = malloc( maxrows*sizeof(int) );
  maxcols = initCOLS; cols = malloc( maxcols*sizeof(int) );
  if( rows==NULL || cols==NULL ) {printf("*** not enough memory for rows,cols\n");exit(1);};

  /* estimate matrix's size */
  mc=nc=0;
  while ( ! feof( InputFile ) )
  {
    fgets( line, MAXSTRLEN, InputFile );
    
    if( feof( InputFile ) ) break;

    if( line[0] == '#' || isempty(line) ) continue;
    
    v=1;
    sscanf( line, "%d %d %d", &p, &t, &v );
    
    found=0;
    for( i=1; i<=mc; i++ ) if( rows[i]==p ) { found=1; break; }
    if( ! found )
    { 
      if( mc+1 >= maxrows )
      {
        maxrows+=deltaROWS;
	rows = realloc( rows, maxrows*sizeof(int) );
	if( rows == NULL ) {printf("*** not enough memory for rows\n");exit(1);};
      }
      rows[++mc]=p; 
    }
    found=0;
    for( j=1; j<=nc; j++ ) if( cols[j]==t ) { found=1; break; }
    if( ! found ) 
    { 
      if( nc+1 >= maxcols ) 
      {
        maxcols+=deltaCOLS;
	cols = realloc( cols, maxcols*sizeof(int) );
	if( cols == NULL ) {printf("*** not enough memory for cols\n");exit(1);};
      } 
      cols[++nc]=t; 
    }
     
  } /* feof InputFile */

if(debug>1)printf("=== syssolve: rows mc=%d, cols nc=%d\n",mc,nc); fflush(stdout);
  
/*printf("rows (mc=%d): ",mc);
for(i=1;i<=mc;i++)
  printf("%d ", rows[i] );
printf("\ncols (nc=%d): ",nc);
for(i=1;i<=nc;i++)
  printf("%d ", cols[i] );
printf("\n");*/
  
  rewind( InputFile );
  rows = realloc( rows, (mc+1)*sizeof(int) );
  cols = realloc( cols, (nc+1)*sizeof(int) );
  C = calloc( mc*nc, sizeof(int) );
  if( rows==NULL || cols==NULL || C==NULL ) {printf("*** not enough memory for rows,cols,C\n");exit(1);}

  /* read matrix */
  while ( ! feof( InputFile ) )
  {
    fgets( line, MAXSTRLEN, InputFile );
    
    if( feof( InputFile ) ) break;

    if( line[0] == '#' || isempty(line) ) continue;
    
    v=1;
    sscanf( line, "%d %d %d", &p, &t, &v );
    
    found=0;
    for( i=1; i<=mc; i++ ) if( rows[i]==p ) { found=1; break; }
    if( ! found ) { printf("***inconsistent input"); exit(4); }
    found=0;
    for( j=1; j<=nc; j++ ) if( cols[j]==t ) { found=1; break; }
    if( ! found ) { printf("***inconsistent input"); exit(4); }
    
    ec( C, i, j )+=v;    
     
  } /* feof InputFile */
  in=mc;

  fclose( InputFile );

  // call 4ti2 zsolve here
  sprintf(ti42_InputFileName,"%s.mat",InputFileName);
  ti42_InputFile = fopen( ti42_InputFileName, "w" );
  if( ti42_InputFile == NULL ) {printf( "*** error open file %s.mat\n", InputFileName );exit(2);}
  fprintf( ti42_InputFile, "%d %d\n", nc, in );  
  /*for( i=1; i<=in; i++ )
  {
      for( j=1; j<=nc; j++ )
	fprintf( ti42_InputFile, "%d ", ec( C, i, j ) );
      fprintf( ti42_InputFile, "\n" );
  }*/
  for( j=1; j<=nc; j++ )
  {
      for( i=1; i<=in; i++ )
	fprintf( ti42_InputFile, "%d ", ec( C, i, j ) );
      if(j<nc)fprintf( ti42_InputFile, "\n" );
  }
  fclose( ti42_InputFile );
  free(C); 
 
  sprintf(command,"zsolve -q %s",InputFileName);

  ret=system(command);  
  if(ret!=0) 
    printf("*** error %d running: %s\n", ret, command);  

  remove(ti42_InputFileName);
  sprintf(ti42_OutputFileName,"%s.zfree",InputFileName); 
  ti42_OutputFile = fopen( ti42_OutputFileName, "r" );
  if( ti42_OutputFile == NULL ) //{printf( "*** error open .zfree file for %s\n", OutputFileName );exit(2);}
  {
    //fprintf( OutputFile, "%d %d %d\n", 1, rows[mc], 0);
if(debug>1)printf("zero solution of %d obtained from 4ti2\n",mc);
  }
  else 
  {
    fscanf(ti42_OutputFile,"%d %d\n",&in,&nx);

if(debug>1)printf("%d solutions of %d obtained from 4ti2\n",in,nx);
   
    for( i=1; i<=in; i++ )
    {
        for( j=1; j<=nx; j++ )
        {
          fscanf(ti42_OutputFile,"%d",&v);
  	  if(v != 0) fprintf( OutputFile, "%d %d %d\n", i, rows[j], v);
        }
    }
    //fprintf( OutputFile, "%d %d %d\n", in, rows[nx], 0);
  }

  free(rows); free(cols);
  
  if( ti42_OutputFile != NULL )
  {
    fclose( ti42_OutputFile );
    remove(ti42_OutputFileName);
  }
  fclose( OutputFile );
  sprintf(ti42_OutputFileName,"%s.zhom",InputFileName);
  remove(ti42_OutputFileName);
  sprintf(ti42_OutputFileName,"%s.zinhom",InputFileName);
  remove(ti42_OutputFileName);
//printf("return from toy\n");
  return( ret );
    
} // zsolve4ti2 ends

#ifdef __MAIN__
int main( int argc, char * argv[] )
{
  if (argc < 3 )
  {
    printf( "Solve linear homogenious Diophantine system\n");
    printf( "Usage: call4ti2 <inputfile> <outputfile>\n" );
    printf( "<inputfile>, <outputfile> : SPM format: i, j, e(i,j)\n" );
    
    return 0;
  }
  zsolve4ti2( "zsolve", argv[1], argv[2], 0 );
} 
#endif

// @ 2019 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org

