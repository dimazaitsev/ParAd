// ParAd: Parallel Adriana 
// coy.c                              
// Creates a composition system for contact places

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

void CreateCompositionSystem( char * NetFileName )
{
 char FileName[ FILENAMELEN+1 ];
 char str[ MAXSTRLEN+1 ];
 FILE * zFile, * sFile, * uFile;
 int p, x, y, v, i, k, z;
 int nz, nc, ni, ic;
 int *in;
 int *ai, found, ii, aimax;
 coty * c;
 
 sprintf( FileName, "%s.z", NetFileName );
 zFile = fopen( FileName, "r" );
 if( zFile == NULL ) {printf( "*** error open file %s\n", FileName );exit(2);}
 sprintf( FileName, "%s.s", NetFileName );
 sFile = fopen( FileName, "w" );
 if( sFile == NULL ) {printf( "*** error open file %s\n", FileName );exit(2);}
 
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
   sprintf( FileName, "%s.i%d", NetFileName, z );
   zFile = fopen( FileName, "r" );
   if( zFile == NULL ) {printf( "*** error open file %s\n", FileName );exit(2);}
   
   /* read inv of subnet */
   ni=0;
   while( ! feof( zFile ) )
   {
     fgets( str, MAXSTRLEN, zFile);
     
     if( feof( zFile ) ) break;
     
     if( str[0]==';' || isempty(str) ){ continue; }
   
     sscanf( str, "%d %d %d", &i, &p, &v );

// vnum(i) enumerate 'i' and return its number
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
     
     for( ic=0; ic<nc; ic++ )
     {
       if( c[ic].p==p && c[ic].x==z )
         fprintf( sFile, "%d %d %d\n", vnum(ii), p, -v );
       
       if( c[ic].p==p && c[ic].y==z )
         fprintf( sFile, "%d %d %d\n", vnum(ii), p, v );
     }
         
     //nl=max(nl,i);
   }
   
   is+=ni;
   
   fclose( zFile );
  
 } /* on subnets */
 
 fclose( sFile );
 free(ai);
 
 /* compose additional solutions for internal invariants */
 in=calloc((is+1), sizeof(int));
 if( in==NULL ) {printf("*** not enough memory for in\n");exit(1);};
 
 sprintf( FileName, "%s.s", NetFileName );
 sFile = fopen( FileName, "r" );
 if( sFile == NULL ) {printf( "*** error open file %s\n", FileName );exit(2);}
 
 while( ! feof(sFile) )
 {
     fgets( str, MAXSTRLEN, sFile);
     
     if( feof( sFile ) ) break;
     
     if( str[0]==';' || isempty(str) ){ continue; }
   
     sscanf( str, "%d %d %d", &i, &p, &v );
     
     in[i]=1;
     
 }/* sFile */
 
 fclose(sFile);
 
 /* write unit solutions */
 sprintf( FileName, "%s.u", NetFileName );
 uFile = fopen( FileName, "w" );
 if( uFile == NULL ) {printf( "*** error open file %s\n", FileName );exit(2);}
 
 k=1;
 for( i=1; i<=is; i++)
   if( in[i]==0 )
     fprintf( uFile, "%d %d %d\n", k++, i, 1 );
 
 free(c); free(in);
 
 fclose( uFile ); 
     
} /* CreateCompositionSystem */

#ifdef __MAIN__
main( int argc, char * argv[] )
{
  if( argc < 2 ) return;
  CreateCompositionSystem( argv[1] );
}
#endif

// @ 2018 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org

