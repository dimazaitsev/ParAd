// ParAd: Parallel Adriana 
// writeinv.c          
// Write invariants (given in SPM format) into text file according to table of names

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include "uti.h"

//#define MAXINPSTRLEN 1024
//#define MAXFILENAME 256

#define mINIT 1024
#define namesINIT 16384

#define mDELTA 1024
#define namesDELTA 16384

#define LINESIZE 256

#define ec( c, i, j ) (*((c)+(((i)-1)*nc)+((j)-1)))

static char str[ MAXSTRLEN + 1 ]; /* line buffer */
 
static int m, maxm; /* net size: trs, pls, arcs */
static int *pn; /* pls */
 
static char *names; /* all the names */
static int fnames, maxnames;

void GetName1( int *i, int *j )
{
 int state;

 if( str[(*i)]=='{' )
 {
   state=1;
   while( str[(*i)]!='\0' && state && str[(*i)]!=0xa && str[(*i)]!=0xd ) 
   {
    names[ (*j)++ ]=str[ (*i)++ ]; 
    if( str[(*i)-1]=='}' && state==1 ) state=0; else
      if( str[(*i)-1]=='\\' && state==2 ) state=1; else
        if( str[(*i)-1]=='\\' && state==1 ) state=2; else
	  if( str[(*i)-1]=='}' && state==2 ) state=1; else 
	    if( state==2 ) state=1;
   }
   names[ (*j)++ ]='\0';
 }
 else
 {
   while( str[(*i)]!=' ' && str[(*i)]!='\0' && str[(*i)]!=0xa && str[(*i)]!=0xd && str[(*i)]!=0x9 && str[(*i)]!='*' )
    names[ (*j)++ ]=str[ (*i)++ ];
   names[ (*j)++ ]='\0';
 }

} /* GetName1 */

void ExpandNames1()
{
  char * newnames;
  
  if( fnames+MAXSTRLEN > maxnames )
  { 
    maxnames+=namesDELTA;
    newnames = (char*) realloc( names, maxnames );
    if( newnames==NULL ) { printf( "*** not enough memory for names\n" ); exit(1); }
    else names=newnames;
  }

} /* ExpandNames1 */

void ExpandP1()
{
  int *newpn;

  if( m >= maxm - 2 ) 
  {
    maxm+=mDELTA;
    newpn = (int*) realloc( pn, maxm * sizeof(int) );
    if( newpn==NULL ) 
      { printf( "*** not enough memory for pn\n" ); exit(1); }
    else 
      { pn=newpn; }
  }

} /* ExpandP1 */

void ReadTable( FILE * f )
{
  int i, len, p; 
      
  while( ! feof( f ) )
  {
     fgets( str, MAXSTRLEN, f ); 
     if( feof( f) ) break;
     if( str[0]=='#' ) continue; /* comment line */
   
     len=strlen(str); i=0;
     SwallowSpace( str, &i );
     if( i==len ) continue; /*empty line */
   
     ExpandNames1();
     ExpandP1();
     
     p=atoi( str+i );
     while( ! IsSpace( str, i++ ));
     SwallowSpace( str, &i );
     pn[ p ]=fnames; m++;
     GetName1( &i, &fnames );
 
  }
  
}/* ReadTable */

int WriteInv( char * TableFileName, char * InvFileName, char * OutFileName, int verbose )
{
 FILE * TableFile, * InvFile, * OutFile;
 int i, i0, j, v, len;
 int mc, nc, kc, *C, p, t;
 char line[LINESIZE+1];
 
 /* open files */
 TableFile = fopen( TableFileName, "r" );
 if( TableFile == NULL ) {printf( "*** error open file %s\n", TableFileName );exit(2);}
 InvFile = fopen( InvFileName, "r" );
 if( InvFile == NULL ) {printf( "*** error open file %s\n", InvFileName );exit(2);}
 if( strcmp( OutFileName, "-" )==0 ) OutFile = stdout; 
   else OutFile = fopen( OutFileName, "a" );
 if( OutFile == NULL ) {printf( "*** error open file %s\n", OutFileName );exit(2);}
    
 /* init net size  */
 maxm=mINIT; 
 maxnames=namesINIT;

 /* allocate arrays */
 pn = (int*) calloc( maxm, sizeof(int) ); m=1;
 
 names = (char*) calloc( maxnames, sizeof(char) ); fnames=0;
 
 if( 
     pn==NULL || 
     names==NULL )
   { printf( "*** not enough memory for pn,names\n" ); return(1); }  
   
 ReadTable( TableFile ); 
 fclose( TableFile );
 
// new part

  /* estimate matrix's size */
  mc=nc=kc=0;
  while ( ! feof( InvFile ) )
  {
    fgets( line, LINESIZE, InvFile );
    
    if( feof( InvFile ) ) break;

    if( line[0] == '#' || isempty(line) ) continue;
    
    v=1;
    sscanf( line, "%d %d %d", &p, &t, &v ); 

    kc++;
    mc=max(mc,p);
    nc=max(nc,t);
  } /* feof InputFile */
  
 C=calloc(mc*nc,sizeof(int));
 if( C==NULL ) {printf("*** not enough memory for C in writeinv \n");exit(1);}
 rewind( InvFile );

 while( ! feof( InvFile ) )
 {
   fgets( str, MAXSTRLEN, InvFile ); 
   if( feof( InvFile ) ) break;
   if( str[0]=='#' ) continue; /* comment line */
   
   len=strlen(str); i=0;
   SwallowSpace( str, &i );
   if( i==len ) continue; /*empty line */
   
   sscanf( str, "%d %d %d", &i, &j, &v );
   if(v==0) continue; 
   ec( C, i, j )+=v;
 
 }/* while */

 fprintf( OutFile, "\n" );
 i0=kc=0;
   for(i=1;i<=mc;i++)
   {
     for(j=1;j<=nc;j++)
     {
       v=ec( C, i, j );
       if(v!=0)
       {
         if(i0!=i){kc++;i0=i;}
         if(verbose)
         {
           fprintf( OutFile, "%s", names+pn[j] );
           if(v!=1)fprintf( OutFile, "*%d", v );
           fprintf( OutFile, " " );
         }
       }
     }
     if(verbose&&i0==i) fprintf( OutFile, "\n" );
   }

   free(C);
// end new part

 /*if(!verbose)*/ { fprintf( OutFile, "\n%d semiflow(s)", kc ); }
 /*else*/ { if(kc==0) fprintf( OutFile, "\nno semiflows" ); }
 fprintf( OutFile, "\n\n" );
 
 fclose( InvFile );
  
 if( OutFile != stdout ) fclose( OutFile );
  
 free( pn );
  
 free( names );

 return(0);
 
}/* WriteInv */

#ifdef __MAIN__
int main( int argc, char *argv[] )
{
  if( argc>1 ) { TableFileName=argv[1]; } else exit( 2 );
  if( argc>2 ) { InvFileName=argv[2]; }
  if( argc>3 ) { strcpy(OutFileName,argv[3]); } else { sprintf( OutFileName, "%s.txt", InvFileName ); }
  WriteInv( TableFileName, InvFileName, OutFileName );
}
#endif

// @ 2019 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org

