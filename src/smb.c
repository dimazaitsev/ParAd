// ParAd: Parallel Adriana 
// smb2.c          
// Minimize basis

#include <stdio.h>
#include <stdlib.h>
#include "uti.h"

/* sizes of matrices */
static int ma, na;

#define ea( a, i, j ) (*((a)+(((i)-1)*na)+((j)-1)))

#define initROWS 1024
#define deltaROWS 1024
#define initCOLS 1024
#define deltaCOLS 1024

//#define LINESIZE 256

void MinimizeBasis( char * AFileName, char * BFileName )
{
 FILE * AFile, * BFile;
 char   line[ MAXSTRLEN+1 ];
 int * A, * B, * rowsa, * colsa;
 int maxrowsa, maxcolsa;
 int i, j, k, v, p, t, found;
 int i1, i2, i1lei2;

 /* matrix A */
  AFile = fopen( AFileName, "r" );
  if( AFile == NULL ) {printf("*** error open file %s\n", AFileName ); exit(0);}
 
  maxrowsa = initROWS; rowsa = malloc( maxrowsa*sizeof(int) );
  maxcolsa = initCOLS; colsa = malloc( maxcolsa*sizeof(int) );
  if( rowsa==NULL || colsa==NULL ) {printf("*** not enough memory for rowsa,colsa\n");exit(1);};

  /* estimate matrix's A size */
  ma=na=0;
  while ( ! feof( AFile ) )
  {
    fgets( line, MAXSTRLEN, AFile );
    
    if( feof( AFile ) ) break;

    if( line[0] == '#' || isempty(line) ) continue;
    
    v=1;
    sscanf( line, "%d %d %d", &p, &t, &v );
    
    found=0;
    for( i=1; i<=ma; i++ ) if( rowsa[i]==p ) { found=1; break; }
    if( ! found )
    { 
      if( ma+1 >= maxrowsa )
      {
        maxrowsa+=deltaROWS;
	rowsa = realloc( rowsa, maxrowsa*sizeof(int) );
	if( rowsa == NULL ) {printf("*** not enough memory for rowsa\n");exit(1);};
      }
      rowsa[++ma]=p; 
    }
    found=0;
    for( j=1; j<=na; j++ ) if( colsa[j]==t ) { found=1; break; }
    if( ! found ) 
    { 
      if( na+1 >= maxcolsa ) 
      {
        maxcolsa+=deltaCOLS;
	colsa = realloc( colsa, maxcolsa*sizeof(int) );
	if( colsa == NULL ) {printf("*** not enough memory for colsa\n");exit(1);};
      } 
      colsa[++na]=t; 
    }
     
  } /* feof AFile */
  
/*printf("rows (mc=%d): ",mc);
for(i=1;i<=mc;i++)
  printf("%d ", rows[i] );
printf("\ncols (nc=%d): ",nc);
for(i=1;i<=nc;i++)
  printf("%d ", cols[i] );
printf("\n");*/
  
  rewind( AFile );
  rowsa = realloc( rowsa, (ma+1)*sizeof(int) );
  colsa = realloc( colsa, (na+1)*sizeof(int) );
  A = calloc( ma*na, sizeof(int) );
  if( rowsa==NULL || colsa==NULL || A==NULL ) {printf("*** not enough memory for rowsa,colsa,A\n");exit(1);}

  /* read matrix A */
  while ( ! feof( AFile ) )
  {
    fgets( line, MAXSTRLEN, AFile );
    
    if( feof( AFile ) ) break;

    if( line[0] == '#' || isempty(line) ) continue;
    
    v=1;
    sscanf( line, "%d %d %d", &p, &t, &v );
    
    found=0;
    for( i=1; i<=ma; i++ ) if( rowsa[i]==p ) { found=1; break; }
    if( ! found ) { printf("***inconsistent input"); exit(4); }
    found=0;
    for( j=1; j<=na; j++ ) if( colsa[j]==t ) { found=1; break; }
    if( ! found ) { printf("***inconsistent input"); exit(4); }
    
    ea( A, i, j )+=v;    
     
  } /* feof AFile */
  
  fclose( AFile );
 
 B = calloc( ma+1, sizeof(int) ); 
 if( B==NULL ) {printf("*** not enough memory for B\n");exit(1);}
  
 for( i1=1; i1<=ma; ++i1 )
 {
   if( B[i1] > 0 ) continue;
   for( i2=1; i2<=ma; ++i2 )
   {
     if( B[i2] > 0 ) continue;
     if( i1 == i2 ) continue;
     i1lei2=1;
     for( j=1; j<=na; ++j )
     {
        if( ea( A, i1, j ) > ea( A, i2, j ) ) { i1lei2=0; break; }    
     }
     B[i2]=i1lei2;
   }
 } 

 BFile = fopen( BFileName, "w" );
 if( BFile == NULL ) {printf( "*** error open file %s\n", BFileName );exit(2);}
 
 k=0;
 for( i=1; i<=ma; ++i )
 {
   if( B[i] > 0 ) continue;
   k++;
   for( j=1; j<=na; ++j )
   {
     if( ea( A, i, j )!=0 )
       fprintf( BFile, "%d %d %d\n", k, colsa[j], ea( A, i, j ) );
   }
   
 }

 free(rowsa); free(colsa); free(A);
 free(B);
 
 fclose( BFile );

}/* MinimizeBasis */

#ifdef __MAIN__
int main( int argc, char *argv[] )
{
  if( argc < 3 )
  {
    printf("mb: minimization of basis\n");
    printf("MB matrAfile matrBfile\n");
    exit(0);
  }
  MinimizeBasis( argv[1], argv[2] );
}/* main */
#endif

// @ 2019 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org

