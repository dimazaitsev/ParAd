///////////////////////////////// ParAd: Parallel Adriana //////////////////////////////////////////////
// fromTina.c: convertion of data from Tina to a sparse matrix                                        //
// Reads ndr./net. file of Tina, writes it as .spm                                                    //
// @ 2019 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org                            //
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
"fromTina - version 1.0.1\n\n"
"action: converts .ndr/.net file to .spm file with optional saving nodes' names\n"
"file formats: .ndr, .net (www.laas.fr/tina), and .spm - sparse matrix (i j x_{i,j})\n"
"usage:  fromTina [-h]\n"
"                 [-d]\n"
"                 ndr_or_net_file spm_file\n"
"FLAGS            WHAT                                           DEFAULT\n"
"-h               print help (this text)\n"
"-d               save dictionaries of place/transition names\n" 
"ndr_or_net_file  Petri net in .net/.ndr format\n"
"spm_file         Petri net in .spm format\n\n"
"@ 2018 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: zaitsevd@icl.utk.edu\n";

int main( int argc, char *argv[] )
{
  char fname1[ FILENAMELEN+1 ];
  char * NetFileName;
  char * SPMFileName;
  int i, numf=0, save_dictionary=0;
  
  

    /* parse command line */
    numf=0;
    for( i=1; i<argc; i++ )
    {
      if( strcmp( argv[i], "-h" )==0 )
      {
        printf( "%s", Help ); return( 0);
      }
      
      else if( strcmp( argv[i], "-d" )==0 ) save_dictionary=1;
      
      else if( numf==0 ) { NetFileName=argv[i]; numf++; }
      else if( numf==1 ) { SPMFileName=argv[i]; numf++; }
      else
        { printf( "*** unknown option: %s\n", argv[i] ); return(4); }
    } /* for */
  
//    if( numf==0 ) NetFileName = "-";
//    if( numf<=1 ) SPMFileName = "-";
    if( numf<2 )
    {
        fprintf( stderr, "*** absent input/otput file(s) \n" );
        printf( "%s", Help ); return( 5);
    }
//  strict decision: no stdin/stdout defaults for clarity
   
    ReadNDRNET( NetFileName, NetFileName, "/dev/null", 0 );

    sprintf( fname1, "%s.spm", NetFileName );
    if(strcmp(NetFileName,"-")!=0)
      rename( fname1, SPMFileName );
    else
    {
      CopyFile(fname1,"-","w");
      remove( fname1 );
    }
      

    if( save_dictionary == 0 )
    {  
      sprintf( fname1, "%s.nmp", NetFileName );
      remove( fname1 );
      sprintf( fname1, "%s.nmt", NetFileName );
      remove( fname1 );
    }

   
  return(0);
  
} /* main */

// @ 2018 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org 

