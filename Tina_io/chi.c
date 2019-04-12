// ParAd: Parallel Adriana 
// chi.c                                   
// Check invariance of a Petri net    

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uti.h"

//#define LINESIZE 256

#define initROWS 1024
#define deltaROWS 1024

int CheckInv( char * NetFileName, char * InvFileName )
{
  char line[ MAXSTRLEN+1 ];
  FILE * NetFile, * InvFile;
  int mc, maxrows;
  int i, p, t, v, l;
  int inv, found;
  int * rows, * inds;
  
  NetFile = fopen( NetFileName, "r" );
  if( NetFile == NULL ) {printf( "*** error open file %s\n", NetFileName );exit(2);}
  if(strcmp(InvFileName,"-")==0)
    InvFile = stdin;
  else
  {
    InvFile = fopen( InvFileName, "r" );
    if( InvFile == NULL ) {printf( "*** error open file %s\n", InvFileName );exit(2);}
  }
  
  maxrows = initROWS; rows = malloc( maxrows*sizeof(int) );
  if( rows==NULL ) {printf("*** not enough memory for rows\n");exit(1);};

  /* collect numbers of places */
  mc=0;
  while ( ! feof( NetFile ) )
  {
    fgets( line, MAXSTRLEN, NetFile );
    
    if( feof( NetFile ) ) break;

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
  } /* feof NetFile */
  
  fclose( NetFile );
  rows = realloc( rows, (mc+1)*sizeof(int) );
  inds = calloc( (mc+1), sizeof(int) );
  if( rows==NULL || inds==NULL ) {printf("*** not enough memory for rows, inds\n");exit(1);}

  /* check invariants */
  while ( ! feof( InvFile ) )
  {
    fgets( line, MAXSTRLEN, InvFile );
    
    if( feof( InvFile ) ) break;

    if( line[0] == '#' || isempty(line) ) continue;
    
    v=1;
    sscanf( line, "%d %d %d", &l, &p, &v );
    
    if( v > 0 )
    {
      found=0;
      for( i=1; i<=mc; i++ ) if( rows[i]==p ) { found=1; break; }
      if( found )
      { 
        inds[i]=1;
      }
    }
  } /* feof InvFile */
  
  if(InvFile != stdin) fclose( InvFile );

  inv=1;
  for( i=1; i<=mc; i++ ) {inv = inv && inds[i];}

  free(rows); free(inds);
  
  return( inv );
    
} /* CheckInv */

#ifdef __MAIN__
int main( int argc, char * argv[] )
{
  int i;
  
  i = CheckInv( argv[1], argv[2] );
  printf( "%d\n", i );
} 
#endif

// @ 2018 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org

