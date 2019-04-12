// ParAd: Parallel Adriana 
// uti.c          
// Utility functions

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uti.h"

extern int debug_level;
extern char solver_name[FILENAMELEN+1];

//#define MAXSTRLEN 1024

int isempty( char * s )
{
  int i=0;
  int empty=0;
  
  while( ( s[i]==' ' || s[i]==0xa || s[i]==0xd || s[i]==0x9 ) && s[i]!='\0'  ) i++;
  if( s[i]=='\0' ) empty=1;
  
  return(empty);

} /* isempty */

void SwallowSpace( char * str, int *i )
{
  
  while( ( str[(*i)]==' ' || str[(*i)]==0xa || str[(*i)]==0xd || str[(*i)]==0x9 ) && str[(*i)]!='\0'  ) (*i)++;

} /* SwallowSpace */

int IsSpace( char * str, int i )
{
   
  if( str[i]==' ' || str[i]==0xa || str[i]==0xd || str[i]==0x9 || str[i]=='\0' )
    return( 1 );
  else
    return( 0 );
  
} /* IsSpace */

int CopyFile( char * fnameFROM, char * fnameTO, char * attr )
{
  FILE * fileFROM, * fileTO;
  char str[ MAXSTRLEN+1 ];
 
  fileFROM = fopen( fnameFROM, "r" );
  if( fileFROM == NULL ) {printf( "*** error open file %s\n", fnameFROM );exit(2);}
  fileTO = fopen( fnameTO, attr );
  if( fileTO == NULL ) {printf( "*** error open file %s\n", fnameTO );exit(2);}
 
 /* read solutions */
 while( ! feof( fileFROM ) )
 {
   fgets( str, MAXSTRLEN, fileFROM );
   
   if( feof( fileFROM ) ) break;
   
   fputs( str, fileTO );
   
 }
 
 fclose( fileFROM );
 fclose( fileTO );
 
 return(0);
 
} /* CopyFile */

int TransposeMatrix( char * InpFileName, char * OutFileName )
{
 char str[ MAXSTRLEN+1 ];
 FILE * iFile, * oFile;
 int i, j, v;
  
 iFile = fopen( InpFileName, "r" );
 if( iFile == NULL ) {printf( "*** error open file %s\n", InpFileName );exit(2);}
 oFile = fopen( OutFileName, "w" );
 if( oFile == NULL ) {printf( "*** error open file %s\n", OutFileName );exit(2);}
 
 while( ! feof( iFile ) )
 {
   fgets( str, MAXSTRLEN, iFile);
   
   if( feof( iFile ) ) break;
   
   if( str[0]=='#' || isempty(str) ){ fputs( str, oFile ); continue; }
   
   sscanf( str, "%d %d %d", &i, &j, &v );
   fprintf( oFile, "%d %d %d\n", j, i, v );   
 }
 
 fclose( iFile );
 fclose( oFile );
 return(0);
 
} /* TransposeMatrix */


int EternalSolverCall( char * SolverName, char * SystemFileName, char * SolutionFileName, int debug_level )
{
  char command[ FILENAMELEN+1 ];
  int ret=0;

  sprintf(command,"%s %s %s",SolverName,SystemFileName,SolutionFileName);
  ret=system(command);  
  if(ret!=0) 
    printf("*** error %d running: %s\n", ret, command);
  return(ret);
}

// @ 2019 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org

