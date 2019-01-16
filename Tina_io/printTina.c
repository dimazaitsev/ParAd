///////////////////////////////// ParAd: Parallel Adriana //////////////////////////////////////////////
// printTina.c: prints .spn invariant in Tina format                                                  //
// Read ndr./net. file of Tina and .spm file of invariants, print invariants according to Tina format //
// @ 2018 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org                            //
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <bits/time.h>
#include <sys/time.h>
#include "uti.h"
#include "readnet.h"
#include "writeinv.h"
#include "chi.h"

static char Help[] =
"printTina - version 1.0.1\n\n"
"action: prints .spm invariant in Tina textual format\n"
"file formats: .ndr, .net (www.laas.fr/tina), and .spm - sparse matrix (i j x_{i,j})\n"
"usage: printTina [-h]\n"
"                 [-v | -q]\n"
"                 [-P | -T]\n"
"                 ndr_or_net_file invariants_file\n"
"FLAGS            WHAT                                           DEFAULT\n"
"-h               print help (this text)\n"
"-P | -T          place or transition invariants                 -P\n"
"-v | -q          invariant format (full | digest)               -v\n"
"ndr_or_net_file  Petri net file .net/.ndr\n"
"invariants_file  .spm file of invariants\n\n"
"@ 2018 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: zaitsevd@icl.utk.edu\n";

int main( int argc, char *argv[] )
{
  char fname1[ FILENAMELEN+1 ];
  char fname2[ FILENAMELEN+1 ];
  char fname3[ FILENAMELEN+1 ];
  char * NetFileName;
  char * InvFileName;
  int i,numf, snumf, inv;
  char *s;
  int place=1;
  int transition=0;
  int mode=0;
  int debug_level=0;
  int verbose=1;
  double dt;
  char c, c1;
  int len;
  char * buf;
  int z;
  double t1,t2;

    /* parse command line */
    numf=0;
    for( i=1; i<argc; i++ )
    {
      if( strcmp( argv[i], "-h" )==0 )
      {
        printf( "%s", Help ); return( 0);
      }
      else if( strcmp( argv[i], "-q" )==0 ) verbose=0;
      else if( strcmp( argv[i], "-v" )==0 ) verbose=1;
      else if( strcmp( argv[i], "-P" )==0 ) { transition=0; place=1; }
      else if( strcmp( argv[i], "-T" )==0 ) { place=0; transition=1; }
           
      else if( numf==0 ) { NetFileName=argv[i]; numf++; }
      else if( numf==1 ) { InvFileName=argv[i]; numf++; }
      else
        { printf( "*** unknown option: %s\n", argv[i] ); return(4); }
    } /* for */
  
//    if( numf==0 ) {printf( "%s", Help ); return( 0);}
//    if( numf<=1 ) InvFileName = "-";  
    if( numf<2 )
    {
        fprintf( stderr, "*** absent input/otput file(s) \n" );
        printf( "%s", Help ); return( 5);
    }
//  strict decision: no stdin/stdout defaults for clarity
   
    if(transition||place)
    {
      printf( "ParAd - Parallel Adriana - version 1.1.2\n\n" );
    }
  
    ReadNDRNET( NetFileName, NetFileName, "-", verbose );

    if( place )
    {
      sprintf( fname1, "%s.spm", NetFileName );
      inv = CheckInv( fname1, InvFileName );  
  
      printf( "P-SEMI-FLOWS GENERATING SET --------------------------------\n\n" );
      s = (inv)? "invariant" : "not invariant";
      printf( "%s\n", s );
    
      sprintf( fname1, "%s.nmp", NetFileName );
      WriteInv( fname1, InvFileName, "-", verbose );    	
        
        if( debug_level == 0 )
        {  
          sprintf( fname1, "%s.spm", NetFileName );
          remove( fname1 );
          sprintf( fname1, "%s.nmp", NetFileName );
          remove( fname1 );
          sprintf( fname1, "%s.nmt", NetFileName );
          remove( fname1 );
        }
     
    } else 
    if( transition )
    {
        sprintf( fname1, "%s.spm", NetFileName );
        sprintf( fname2, "%s.spmt", NetFileName );
        TransposeMatrix( fname1, fname2 );
        inv = CheckInv( fname2, InvFileName );

        printf( "T-SEMI-FLOWS GENERATING SET --------------------------------\n\n" );
        s = (inv)? "consistent" : "not consistent";
        printf( "%s\n", s );
    
        sprintf( fname1, "%s.nmt", NetFileName );
        WriteInv( fname1, InvFileName, "-", verbose );
        
        if( debug_level == 0 )
        {
          sprintf( fname1, "%s.spm", NetFileName );
          remove( fname1 );
          sprintf( fname1, "%s.spmt", NetFileName );
          remove( fname1 );
          sprintf( fname1, "%s.nmp", NetFileName );
          remove( fname1 );
          sprintf( fname1, "%s.nmt", NetFileName );
          remove( fname1 );
        }
      } 
        
    if(transition||place)
    {
      printf( "%#fs\n\n", 0.0 );
      printf( "ANALYSIS COMPLETED -----------------------------------------\n" );
    }
  
  return(0);
  
} /* main */

// @ 2018 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org 

