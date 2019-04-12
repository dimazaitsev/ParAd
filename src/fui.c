// ParAd: Parallel Adriana 
// fiu.c                             
// Fusion of composition system's solutions
// Adds unit solution to solutions of composition system

#include <stdio.h>
#include <stdlib.h>
#include "uti.h"

//#define MAXSTRLEN 256

void AddUnitSolutions( char * aFileName, char * bFileName, char * cFileName )
{
 FILE * aFile, * bFile, * cFile;
 char str[ MAXSTRLEN+1 ];
 int k, i, z, v;
 
 aFile = fopen( aFileName, "r" );
 if( aFile == NULL ) {printf( "*** error open file %s\n", aFileName );exit(2);}
 bFile = fopen( bFileName, "r" );
 if( bFile == NULL ) {printf( "*** error open file %s\n", bFileName );exit(2);}
 cFile = fopen( cFileName, "w" );
 if( cFile == NULL ) {printf( "*** error open file %s\n", cFileName );exit(2);}
 
 k=0;
 /* read solutions */
 while( ! feof( aFile ) )
 {
   fgets( str, MAXSTRLEN, aFile);
   
   if( feof( aFile ) ) break;
   
   if( str[0]=='#' || isempty(str) ){ continue; }
   
   sscanf( str, "%d %d %d", &i, &z, &v );
   fprintf( cFile, "%d %d %d\n", i, z, v );
   
   k=max(k,i);
 }
 fclose(aFile);
 
 /* read internal solutions */
 while( ! feof( bFile ) )
 {
   fgets( str, MAXSTRLEN, bFile);
   
   if( feof( bFile ) ) break;
   
   if( str[0]=='#' || isempty(str) ){ continue; }
   
   sscanf( str, "%d %d %d", &i, &z, &v );
   
   fprintf( cFile, "%d %d %d\n", ++k, z, v );
 }
 fclose( bFile );
 fclose( cFile );
     
} /* AddUnitSolutions */

#ifdef __MAIN__
main( int argc, char * argv[] )
{
  if( argc < 3 ) return;
  AddUnitSolutions( argv[1], argv[2], argv[3] );
}
#endif

// @ 2019 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org

