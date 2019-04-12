// ParAd: Parallel Adriana 
// mui.c           
// Composes joint matrix of basis invariants

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "uti.h"

//#define FILENAMELEN 256
//#define MAXSTRLEN 256
#define aiINIT 1024
#define aiDELTA 2014

struct contact {
  int p;
  int x;
  int y;
};

typedef struct contact coty;

static int is=0;

#define vnum( j ) (is+(j))

void ComposeJointMatrix( char * NetFileName )
{
 char zFileName[ FILENAMELEN+1 ];
 char gFileName[ FILENAMELEN+1 ];
 char str[ MAXSTRLEN+1 ];
 FILE * zFile, * gFile;
 int p, x, y, v, i, z;
 int nz, nc, ni, ic, nl;
 coty * c;
 int yes;
 int *ai, found, ii, aimax;
 
 sprintf( zFileName, "%s.z", NetFileName );
 zFile = fopen( zFileName, "r" );
 if( zFile == NULL ) {printf( "*** error open file %s\n", zFileName );exit(2);}
 sprintf( gFileName, "%s.g", NetFileName );
 gFile = fopen( gFileName, "w" );
 if( gFile == NULL ) {printf( "*** error open file %s\n", gFileName );exit(2);}
 
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

 ai=malloc(aiINIT*sizeof(int));
 if(ai==NULL){ printf( "*** not enough memory for ai\n" ); exit(3); }
 aimax=aiINIT;
 
 for( z=1; z<=nz; z++ )
 {
   sprintf( zFileName, "%s.i%d", NetFileName, z );
   zFile = fopen( zFileName, "r" );
   if( zFile == NULL ) {printf( "*** error open file %s\n", zFileName );exit(2);}
   
   /* read inv of subnet */
   ni=0;
   nl=0;
   while( ! feof( zFile ) )
   {
     fgets( str, MAXSTRLEN, zFile);
     
     if( feof( zFile ) ) break;
     
     if( str[0]=='#' || isempty(str) ){ continue; }
   
     sscanf( str, "%d %d %d", &i, &p, &v );

// vnum(i) enumerate i and return its number
     if( ni >= aimax )
     { 
       aimax+=aiDELTA;
       ai = (int*) realloc( ai, aimax * sizeof(int) );
       if( ai==NULL ) { printf( "*** not enough memory for ai\n" ); exit(3); }
     }

     found=0;
     for(ii=0;ii<ni;ii++)if(ai[ii]==i){found=1;ii++;break;}
     if(!found){ai[ni++]=i;ii=ni;}
//
     
     yes=1;
     for( ic=0; ic<nc; ic++ )
       if( c[ic].p==p && ( c[ic].y==z && c[ic].x>0 ) ) { yes=0; break; }
       
     if(yes) fprintf( gFile, "%d %d %d\n", vnum(ii), p, v );  
     
     //nl=max(nl,i);
   }
   
   is+=ni;
   
   fclose( zFile );
   
 } /* on subnets */
 
 free(c); free(ai);
 
 fclose(gFile);
 
} /* ComposeJointMatrix */

#ifdef __MAIN__
main( int argc, char * argv[] )
{
  if( argc < 2 ) return;
  ComposeJointMatrix( argv[1] );
} /* main */
#endif

// @ 2019 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org

