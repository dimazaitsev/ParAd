// ParAd: Parallel Adriana 
// fic.c                              
// Filters contact places for fusion of clans

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "uti.h"

//#define FILENAMELEN 256
//#define MAXSTRLEN 256

struct contact {
  int p;
  int x;
  int y;
};

typedef struct contact coty;

int InFusion( int z, int *fu, int nfu, int *i )
{
  int j, found=0;
  
  for( j=0; j<nfu; j++ )
  {
    if( fu[j] == z ) { (*i)=j; found=1; break; }
  }
  return( found );

} /* InFusion */

int FilterContactPlaces( char * NetFileName, int l )
{
 char FileName[ FILENAMELEN+1 ];
 char str[ MAXSTRLEN+1 ];
 FILE * zFile, * yFile;
 int p, x, y, i, j, k, z;
 int nz, nc, ic, ii, ll;
 int ifu, nfu;
 
 coty * c;
 int * fu;
 
 sprintf( FileName, "%s.y", NetFileName );
 zFile = fopen( FileName, "r" );
 if( zFile == NULL ) {printf( "*** error open file %s\n", FileName );exit(2);}
  
 /* get size */
 nc=0;
 fgets( str, MAXSTRLEN, zFile);
 sscanf( str, "%d", &nz );
 while( ! feof( zFile ) )
 {
   fgets( str, MAXSTRLEN, zFile);
   
   if( feof( zFile ) ) break;
   
   if( str[0]=='#' || isempty(str) ){ continue; }
   
   sscanf( str, "%d %d %d", &p, &x, &y );
   
   nc++;
 }
 
 c = calloc( nc, sizeof( coty ) );
 if( c==NULL ) {printf("*** not enough memory for c\n");exit(1);};
 
 rewind( zFile );
 
 /* read contact info */
 ic=0;
 fgets( str, MAXSTRLEN, zFile);
 sscanf( str, "%d", &nz );
 while( ! feof( zFile ) )
 {
   fgets( str, MAXSTRLEN, zFile);
   
   if( feof( zFile ) ) break;
   
   if( str[0]=='#' || isempty(str) ){ continue; }
   
   sscanf( str, "%d %d %d", &p, &x, &y );
   c[ic].p=p; c[ic].x=x; c[ic].y=y;
   
   ic++;
 }
 
 fclose( zFile );
 
 /* read file of sequence */
 sprintf( FileName, "%s.t", NetFileName );
 zFile = fopen( FileName, "r" );
 if( zFile == NULL ) {printf( "*** error open file %s\n", FileName );exit(2);}
  
/*printf( "l=%d\n", l );*/
 
 /* get size */
 ll=l;
 while( --ll && ! feof( zFile ) ) fgets( str, MAXSTRLEN, zFile);
 
 nfu=0;
 if( ! feof( zFile ) )
 {
   fgets( str, MAXSTRLEN, zFile);
   i=0;
   while( ! isempty( str+i ) )
   {
     SwallowSpace( str, &i );
     while( ! IsSpace( str, i ) ) i++;
     nfu++;
   }
 }
 else { printf( "*** unexpected end of sequence file\n" ); exit(2);}
 
/*printf( "nfu=%d\n", nfu );*/
 
 fu = calloc( nfu, sizeof( int ) );
 if( fu==NULL ) {printf("*** not enough memory for fu\n");exit(1);};
 
 rewind( zFile );
 
 /* read file */
 ll=l;
 while( --ll && ! feof( zFile ) ) fgets( str, MAXSTRLEN, zFile);
 
 ifu=0;
 if( ! feof( zFile ) )
 {
   fgets( str, MAXSTRLEN, zFile);
   i=0;
   while( ! isempty( str+i ) )
   {
     SwallowSpace( str, &i );
     fu[ifu]=atoi( str+i );
     while( ! IsSpace( str, i ) ) i++;
     ifu++;
   }
 }
 else { printf( "*** unexpected end of sequence file\n" ); exit(2);}
 
/*for( i=0; i<nfu; i++ )
  printf( "%d ", fu[i] );
printf( "\n" );*/
 
 fclose( zFile );
 
 /* write file of fusion on step .z and new .y */
 sprintf( FileName, "%s.z", NetFileName );
 zFile = fopen( FileName, "w" );
 if( zFile == NULL ) {printf( "*** error open file %s\n", FileName );exit(2);}
 fprintf( zFile, "%d\n", nfu );
 
 sprintf( FileName, "%s.y", NetFileName );
 yFile = fopen( FileName, "w" );
 if( yFile == NULL ) {printf( "*** error open file %s\n", FileName );exit(2);}
 fprintf( yFile, "%d\n", nz-nfu+1 );
 
 for( ic=0; ic<nc; ic++ )
 {
   if( InFusion( c[ic].x, fu, nfu, &i ) && InFusion( c[ic].y, fu, nfu, &j ) )
   {
     fprintf( zFile, "%d %d %d\n", c[ic].p, i+1, j+1 );
   }
   else
   { 
     if( InFusion( c[ic].x, fu, nfu, &i ) ) c[ic].x=fu[0];
     else if( InFusion( c[ic].y, fu, nfu, &j ) ) c[ic].y=fu[0];
     
     fprintf( yFile, "%d %d %d\n", c[ic].p, c[ic].x, c[ic].y );
   }
   
 }
  
 z=fu[0];
 free( c ); free( fu );
 
 fclose( zFile );
 fclose( yFile );
 
 return( z );
    
} /* FilterContactPlaces */


int psFilterContactPlaces( char * NetFileName, int v1, int v2 )
{
 char FileName[ FILENAMELEN+1 ];
 char str[ MAXSTRLEN+1 ];
 FILE * zFile, * yFile;
 int p, x, y, i, j, z;
 int nz, nc, ic;
 int nfu;
 
 coty * c;
 int * fu;
 
 sprintf( FileName, "%s.y", NetFileName );
 zFile = fopen( FileName, "r" );
 if( zFile == NULL ) {printf( "*** error open file %s\n", FileName );exit(2);}
  
 /* get size */
 nc=0;
 fgets( str, MAXSTRLEN, zFile);
 sscanf( str, "%d", &nz );
 while( ! feof( zFile ) )
 {
   fgets( str, MAXSTRLEN, zFile);
   
   if( feof( zFile ) ) break;
   
   if( str[0]=='#' || isempty(str) ){ continue; }
   
   sscanf( str, "%d %d %d", &p, &x, &y );
   
   nc++;
 }
 
 c = calloc( nc, sizeof( coty ) );
 if( c==NULL ) {printf("*** not enough memory for c\n");exit(1);};
 
 rewind( zFile );
 
 /* read contact info */
 ic=0;
 fgets( str, MAXSTRLEN, zFile);
 sscanf( str, "%d", &nz );
 while( ! feof( zFile ) )
 {
   fgets( str, MAXSTRLEN, zFile);
   
   if( feof( zFile ) ) break;
   
   if( str[0]=='#' || isempty(str) ){ continue; }
   
   sscanf( str, "%d %d %d", &p, &x, &y );
   c[ic].p=p; c[ic].x=x; c[ic].y=y;
   
   ic++;
 }
 
 fclose( zFile );

 // new part instead of reading trace
 
 nfu=2;
 fu = calloc( nfu, sizeof( int ) );
 if( fu==NULL ) {printf("*** not enough memory for fu\n");exit(1);};
 fu[0]=v1; fu[1]=v2;

 
 /* write file of fusion on step .z and new .y */
 sprintf( FileName, "%s.z", NetFileName );
 zFile = fopen( FileName, "w" );
 if( zFile == NULL ) {printf( "*** error open file %s\n", FileName );exit(2);}
 fprintf( zFile, "%d\n", nfu );
 
 sprintf( FileName, "%s.y", NetFileName );
 yFile = fopen( FileName, "w" );
 if( yFile == NULL ) {printf( "*** error open file %s\n", FileName );exit(2);}
 fprintf( yFile, "%d\n", nz-nfu+1 );
 
 for( ic=0; ic<nc; ic++ )
 {
   if( InFusion( c[ic].x, fu, nfu, &i ) && InFusion( c[ic].y, fu, nfu, &j ) )
   {
     fprintf( zFile, "%d %d %d\n", c[ic].p, i+1, j+1 );
   }
   else
   { 
     if( InFusion( c[ic].x, fu, nfu, &i ) ) c[ic].x=fu[0];
     else if( InFusion( c[ic].y, fu, nfu, &j ) ) c[ic].y=fu[0];
     
     fprintf( yFile, "%d %d %d\n", c[ic].p, c[ic].x, c[ic].y );
   }
   
 }
  
 z=fu[0];
 free( c ); free( fu );
 
 fclose( zFile );
 fclose( yFile );
 
 return( z );
    
} /* psFilterContactPlaces */


#ifdef __MAIN__
main( int argc, char * argv[] )
{
  int l, z;  

  if( argc < 3 ) return;
  l = atoi( argv[2]);
  
  z = FilterContactPlaces( arfv[1], l );
  
  printf( "%d\n", z );
}
#endif

// @ 2019 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org

