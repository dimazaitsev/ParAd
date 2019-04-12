// ParAd: Parallel Adriana 
// readnet.c          
// Reads .net or .ndr file and write it into sparse matrix and names' tables

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include "uti.h"

//#define __MAIN__

//#define MAXINPSTRLEN 16384
//#define MAXFILENAME 256

#define nINIT 1024
#define mINIT 1024
#define atpINIT 2048
#define aptINIT 2048
#define namesINIT 16384

#define nDELTA 1024
#define mDELTA 1024
#define atpDELTA 2048
#define aptDELTA 2048
#define namesDELTA 8192

#define NDR 1
#define NET 2

static char str[ MAXSTRLEN + 1 ]; /* line buffer */
 
static int n, m, maxn, maxm; /* net size: trs, pls, arcs */
static int *tn, fat; /* trs */
static int *pn, fap; /* pls */
 
static char *names; /* all the names */
static int fnames, maxnames;
static int netname=-1;
 
static int *atpp, *atpt, *atpw; /* arcs t->p */
static int *aptp, *aptt, *aptw; /* arcs p->t */
static int fatp, fapt, maxatp, maxapt;
	
void GetName( int *i, int *j )
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

} /* GetName */

int RecognizeFile( FILE * f )
{
  int i, len, format;

  while( ! feof( f ) )
  {
    fgets( str, MAXSTRLEN, f ); 
    if( ferror( f ) ) {printf("*** wrong input file\n"); exit(2);}
    if( str[0]=='#' ) continue; /* comment line */
   
    len=strlen(str); i=0;
    SwallowSpace( str, &i );
    if( i==len ) continue; /*empty line */
   
    names[0]=str[i]; names[1]=str[i+1]; names[2]='\0';
   
    if( strcmp( names, "p ")==0 || strcmp( names, "t ")==0 || strcmp( names, "h ")==0 )
      format=NDR; else format=NET;
     
    break;
  }
  
  return( format );
  
} /* RecognizeInputFile */

void ExpandNames()
{
  char * newnames;

  if( fnames+MAXSTRLEN > maxnames )
  { 
    maxnames+=namesDELTA;
    newnames = (char*) realloc( names, maxnames );
    if( newnames==NULL ) { printf( "*** not enough memory (ExpandNames)\n" ); exit(3); }
    else names=newnames;
  }

} /* ExpandNames */

void ExpandP()
{
  int *newpn;

  if( m >= maxm - 2 ) 
  {
    maxm+=mDELTA;
    newpn = (int*) realloc( pn, maxm * sizeof(int) );
    if( newpn==NULL ) 
      { printf( "*** not enough memory (ExpandP)\n" ); exit(3); }
    else 
      { pn=newpn; }
  }

} /* ExpandP */

void ExpandT()
{
  int *newtn;

  if( n >= maxn - 2 ) 
  {
    maxn+=nDELTA;
    newtn = (int*) realloc( tn, maxn * sizeof(int) );
    if( newtn==NULL ) 
      { printf( "*** not enough memory (ExpandT)\n" ); exit(3); }
    else 
      { tn=newtn; }
  }
  
} /* ExpandT */

void ExpandAtp()
{
  int *newatpp, *newatpt, *newatpw;

  if( fatp>=maxatp ) 
  {
    maxatp+=atpDELTA;
    newatpp = (int*) realloc( atpp, maxatp * sizeof(int) );
    newatpt = (int*) realloc( atpt, maxatp * sizeof(int) );
    newatpw = (int*) realloc( atpw, maxatp * sizeof(int) );
    if( newatpp==NULL || newatpt==NULL || newatpw==NULL )
      { printf( "*** not enough memory (ExpandAtp)\n" ); exit(3); }
    else 
      { atpp=newatpp; atpt=newatpt; atpw=newatpw; }
  }

} /* ExpandAtp */

void ExpandApt()
{
  int *newaptp, *newaptt, *newaptw;

  if( fapt>=maxapt ) 
  {
    maxapt+=aptDELTA;
    newaptp = (int*) realloc( aptp, maxapt * sizeof(int) );
    newaptt = (int*) realloc( aptt, maxapt * sizeof(int) );
    newaptw = (int*) realloc( aptw, maxapt * sizeof(int) );
    if( newaptp==NULL || newaptt==NULL || newaptw==NULL )
      { printf( "*** not enough memory (ExpandApt)\n" ); exit(3); }
    else 
      { aptp=newaptp; aptt=newaptt; aptw=newaptw; }
  }

} /* ExpandApt */

void ReadNDR( FILE * f )
{
 int i, found, p, t, inames, len, w;
 char *name1, *name2;

 m=0; n=0; 
 while( ! feof( f ) )
 {
   ExpandNames();
      
   fgets( str, MAXSTRLEN, f ); 
   if( feof(f) ) break;
   if( str[0]=='#' ) continue; /* comment line */
   
   len=strlen(str); i=0;
   SwallowSpace( str, &i );
   if( i==len ) continue; /*empty line */
   
   switch( str[i++] )
   {
     case 'p':
     	SwallowSpace( str, &i );
	while( ! IsSpace( str,i) && i<len )i++; /* x */
	SwallowSpace( str, &i );
	while( ! IsSpace( str,i) && i<len )i++; /* y */
	SwallowSpace( str, &i );
	ExpandP();
	pn[ ++m ] = fnames;
	GetName( &i, &fnames );
	found=0;
	for( p=1; p<m; p++ )
	  if( strcmp( names+pn[p], names+pn[m] )==0 ) { found=1; break; }
	if( ! found )
	{  
	  for( t=1; t<=n; t++ )
	      if( strcmp( names+tn[t], names+pn[m] )==0 ) { found=1; break; }
	}
	if( found ) { printf( "*** duplicate name: %s\n", names+pn[m] ); exit(2); }
	break;
	
     case 't':
       	SwallowSpace( str, &i );
	while( ! IsSpace( str,i) && i<len )i++; /* x */
	SwallowSpace( str, &i );
	while( ! IsSpace( str,i) && i<len )i++; /* y */
	SwallowSpace( str, &i );
	ExpandT();
	tn[ ++n ] = fnames;
	GetName( &i, &fnames );
	found=0;
	for( p=1; p<=m; p++ )
	  if( strcmp( names+pn[p], names+tn[n] )==0 ) { found=1; break; }
	if( ! found )
	{  
	  for( t=1; t<n; t++ )
	      if( strcmp( names+tn[t], names+tn[n] )==0 ) { found=1; break; }
	}
	if( found ) { printf( "*** duplicate name: %s\n", names+tn[n] ); exit(2); }
	break;
      
     case 'e':
	SwallowSpace( str, &i );
	inames=fnames;
	name1=names+inames;
	GetName( &i, &inames );
	SwallowSpace( str, &i );
	if( isdigit(str[i])) while( ! IsSpace( str,i) && i<len )i++; /* rad */
	SwallowSpace( str, &i );
	if( isdigit(str[i])) while( ! IsSpace( str,i) && i<len )i++; /* ang */
	SwallowSpace( str, &i );
	name2=names+inames;
	GetName( &i, &inames );
	/* start from end */
	i=strlen( str )-1;
	while( IsSpace( str,i) && i>0 )i--; 
	while( ! IsSpace( str,i) && i>0 )i--; /* anchor */
	while( IsSpace( str,i) && i>0 )i--;
	while( ! IsSpace( str,i) && i>0 )i--; /* weight */
	w=atoi( str+i+1 ); /* multiplicity */
		
	/* recognize arc */
	
	if( fapt>=maxapt || fatp>=maxatp ) { ExpandAtp(); ExpandApt(); }
	
	found=0;
	for( p=1; p<=m; p++ )
	  if( strcmp( names+pn[p], name1 )==0 ) { ExpandApt(); aptp[fapt]=p; found=1; break; }
	if( found )
	{
	  found=0;
	  for( t=1; t<=n; t++ )
	    if( strcmp( names+tn[t], name2 )==0 ) { aptw[fapt]=w; aptt[fapt++]=t; found=1; break; }
	  if( ! found ) { printf( "*** unknown arc: %s -> %s\n", name1, name2 ); exit(2); }
	}
	else
	{	
	  found=0;
	  for( t=1; t<=n; t++ )
	    if( strcmp( names+tn[t], name1 )==0 ) { ExpandAtp(); atpt[fatp]=t; found=1; break; }
	  if( ! found ) { printf( "*** unknown arc: %s -> %s\n", name1, name2 ); exit(2); }
	  found=0;
	  for( p=1; p<=m; p++ )
	    if( strcmp( names+pn[p], name2 )==0 ) { atpw[fatp]=w; atpp[fatp++]=p; found=1; break; }
	  if( ! found ) { printf( "*** unknown arc: %s -> %s\n", name1, name2 ); exit(2); }
	}
	break;
     
     case 'h':
       SwallowSpace( str, &i );
       netname = fnames;
       GetName( &i, &fnames );
       break;
	
   } /* switch */    
 } /* while */
}/* ReadNDR */

void ReadNET( FILE * f )
{
 int i, j, found, p, t, inames, len, w;
 char *name1;
 char arrow[3];

 while( ! feof( f ) )
 {
   ExpandNames();
   fgets( str, MAXSTRLEN, f );
   if( feof(f) ) break;
   if( str[0]=='#' ) continue; /* comment line */
   len=strlen(str); i=0;
   
   SwallowSpace( str, &i );
   if( i==len ) continue; /*empty line */
   
   inames=fnames;
   GetName( &i, &inames ); /* command */
      
/* pl */   
   if( strcmp( names+fnames, "pl" )==0 )
   {
     	SwallowSpace( str, &i );
	inames=fnames;
	ExpandP();
	pn[ ++m ] = fnames;
	GetName( &i, &inames ); /* pl name */
	if( strcmp( names+fnames,"->")==0 ){ printf( "*** empty place name: %s\n", str ); exit(2); }
	found=0;
	for( p=1; p<m; p++ )
	  if( strcmp( names+pn[p], names+pn[m] )==0 ) { found=1; break; }
	if( found )
	{
	  --m;
	}
	else
	{  
	  p=m;
	  found=0;
	  for( t=1; t<=n; t++ )
	      if( strcmp( names+tn[t], names+pn[p] )==0 ) { found=1; break; }
	
	  if( found ) { printf( "*** duplicate name: %s\n", names+pn[p] ); exit(2); }
	    else { fnames=inames; }
	}

	SwallowSpace( str, &i );
	if( str[i]=='(' ) while( ! IsSpace( str, i ) ) i++; /* marking */
	SwallowSpace( str, &i );
			
	/* input arcs of pl */
	if( i<len-2) { arrow[0]=str[i]; arrow[1]=str[i+1]; arrow[2]='\0'; } else arrow[0]='\0';
	while( strcmp( arrow, "->" )!=0 && i<len )
	{
	  inames=fnames;
	  name1=names+inames;
	  GetName( &i, &inames ); /* input tr name */
	  
	  found=0;
	  for( t=1; t<=n; t++ )
	    if( strcmp( names+tn[t], name1 )==0 ) { found=1; break; }
	  if( ! found ) { ExpandT(); t=++n; tn[t]=fnames; fnames=inames; }
	  	  
	  /* create pl input arc */
          found=0;
	  for( j=0; j<fatp; j++ )
	    if( atpt[j]==t && atpp[j]==p ) { found=1; break; }
	  if( ! found ) { ExpandAtp(); atpt[fatp]=t; atpw[fatp]=1; atpp[fatp++]=p; }
	  
	  /* multiplicity */
	  if( str[i]=='*' )
	  {
	    i++;
	    w=atoi( str+i );
	    while( ! IsSpace( str, i ) ) i++;
	    
	    atpw[fatp-1]=w;
	  }
	  	  
	  SwallowSpace( str, &i );
	  if( i<len-2) { arrow[0]=str[i]; arrow[1]=str[i+1]; arrow[2]='\0'; } else arrow[0]='\0';
	} /* while */
	
	i+=2;
	SwallowSpace( str, &i );
		
	/* output arcs of pl */
	while( i<len )
	{
	  inames=fnames;
	  name1=names+inames;
	  GetName( &i, &inames ); /* output tr name */
	  
	  found=0;
	  for( t=1; t<=n; t++ )
	    if( strcmp( names+tn[t], name1 )==0 ) { found=1; break; }
	  if( ! found ) { ExpandT(); t=++n; tn[t]=fnames; fnames=inames; }
	  
	  /* create pl output arc */
          found=0;
	  for( j=0; j<fapt; j++ )
	    if( aptt[j]==t && aptp[j]==p ) { found=1; break; }
	  if( ! found ) { ExpandApt(); aptt[fapt]=t; aptw[fapt]=1; aptp[fapt++]=p; }
	  
	  /* multiplicity */
	  if( str[i]=='*' )
	  {
	    i++;
	    w=atoi( str+i );
	    while( ! IsSpace( str, i ) ) i++;
	    
	    aptw[fapt-1]=w;
	  }
	  
          SwallowSpace( str, &i );
	  
	} /* while */
		
    } else
    
/* tr */    
    if( strcmp( names+fnames, "tr" )==0 )
    {
     	SwallowSpace( str, &i );
	inames=fnames;
	ExpandT();
	tn[ ++n ] = fnames;
	GetName( &i, &inames ); /* tr name */
	if( strcmp( names+fnames,"->")==0 ){ printf( "*** empty transition name: %s\n", str ); exit(2); }
	found=0;
	for( t=1; t<n; t++ )
	  if( strcmp( names+tn[t], names+tn[n] )==0 ) { found=1; break; }
	if( found )
	{
	  --n;
	}
	else
	{ 
	  t=n; 
	  found=0;
	  for( p=1; p<=m; p++ )
	      if( strcmp( names+pn[p], names+tn[t] )==0 ) { found=1; break; }
	
	  if( found ) { printf( "*** duplicate name: %s\n", names+tn[t] ); exit(2); }
	    else { fnames=inames; }
	}

	SwallowSpace( str, &i );
	if( str[i]=='[' || str[i]==']' ) { while( ! IsSpace( str, i ) ) i++; } /* interval */ 
	SwallowSpace( str, &i );
	
	/* input arcs of tr */
	if( i<len-2) { arrow[0]=str[i]; arrow[1]=str[i+1]; arrow[2]='\0'; } else arrow[0]='\0';
	
	while( strcmp( arrow, "->" )!=0 && i<len )
	{
	  inames=fnames;
	  name1=names+inames;
	  GetName( &i, &inames ); /* input pl name */
	  
	  found=0;
	  for( p=1; p<=m; p++ )
	    if( strcmp( names+pn[p], name1 )==0 ) { found=1; break; }
	  if( ! found ) { ExpandP(); p=++m; pn[p]=fnames; fnames=inames; }
	  
	  /* create tr input arc */
          found=0;
	  for( j=0; j<fapt; j++ )
	    if( aptt[j]==t && aptp[j]==p ) { found=1; break; }
	  if( ! found ) { ExpandApt(); aptt[fapt]=t; aptw[fapt]=1; aptp[fapt++]=p; }
	  
	  /* multiplicity */
	  if( str[i]=='*' )
	  {
	    i++;
	    w=atoi( str+i );
	    while( ! IsSpace( str, i ) )i++;
	    
	    aptw[fapt-1]=w;
	  }
	  	  
	  SwallowSpace( str, &i );
	  if( i<len-2) { arrow[0]=str[i]; arrow[1]=str[i+1]; arrow[2]='\0'; } else arrow[0]='\0';
	} /* while */
	
	i+=2;
	SwallowSpace( str, &i );

	/* output arcs of tr */
	while( i<len )
	{
	  inames=fnames;
	  name1=names+inames;
	  GetName( &i, &inames ); /* output pl name */
	  
	  found=0;
	  for( p=1; p<=m; p++ )
	    if( strcmp( names+pn[p], name1 )==0 ) { found=1; break; }
	  if( ! found ) { ExpandP(); p=++m; pn[p]=fnames; fnames=inames; }
	  
	  /* create tr output arc */
          found=0;
	  for( j=0; j<fatp; j++ )
	    if( atpt[j]==t && atpp[j]==p ) { found=1; break; }
	  if( ! found ) { ExpandAtp(); atpt[fatp]=t; atpw[fatp]=1; atpp[fatp++]=p; }
	  
	  /* multiplicity */
	  if( str[i]=='*' )
	  {
	    i++;
	    w=atoi( str+i );
	    while( ! IsSpace( str, i ) )i++;
	    
	    atpw[fatp-1]=w;
	  }
	  
	  SwallowSpace( str, &i );
	  	  
	} /* while */

     
    } else
    
/* lb */ 
    if( strcmp( names+fnames, "lb" )==0 ) ;
    else
    
/* net */     
    if( strcmp( names+fnames, "net" )==0 )
    {
       SwallowSpace( str, &i );
       netname = fnames;
       GetName( &i, &fnames ); /* net name */
    }
    else 
      { printf( "*** unknown command: %s\n", names+fnames ); exit(2); }
      
  } /* while */
  
}/* ReadNET */

void WriteSPM( FILE * f )
{
  int i; 
  
  for( i=0; i<fapt; i++ )
    fprintf( f, "%d %d %d\n", aptp[i], aptt[i], -aptw[i] );
    
  for( i=0; i<fatp; i++ )
    fprintf( f, "%d %d %d\n", atpp[i], atpt[i], atpw[i] );

}/* WriteSPM */

void WriteNMP( FILE * f )
{
  int p; 
  
  for( p=1; p<=m; p++ )
  {
    fprintf( f, "%d %s\n", p, names + pn[p]  );
  }

}/* WriteNMP */

void WriteNMT( FILE * f )
{
  int t; 
  
  for( t=1; t<=n; t++ )
  {
    fprintf( f, "%d %s\n", t, names + tn[t] );
  }

}/* WriteNMT */

void WriteNet( FILE * f )
{
  int t, i; 
  
  fprintf( f, "net %s\n", (netname==-1)? "noname": names+netname );
  for( t=1; t<=n; t++ )
  {
    fprintf( f, "tr %s", names + tn[t] );
    for( i=0; i<fapt; i++ )
    {
      if( aptt[i] == t )
      {
        fprintf( f, " %s", names + pn[aptp[i]] );
        if( aptw[i] > 1 )
          fprintf( f, "*%d", aptw[i] );
      }
    }
    
    fprintf( f, " ->" );
    
    for( i=0; i<fatp; i++ )
    {
      if( atpt[i] == t )
      {
        fprintf( f, " %s", names + pn[atpp[i]] );
        if( atpw[i] > 1 )
          fprintf( f, "*%d", atpw[i] );
      }
    }
    
    fprintf( f, "\n" );
  }
  
  fprintf( f, "\n" );

}/* WriteNet */

int ReadNDRNET( char * NetFileName, char * SPMFileName, char * OutFileName, int verbose )
{
 char nFileName[ FILENAMELEN+1 ];
 char MatrFileName[ FILENAMELEN+1 ];
 FILE * NetFile, * MatrFile, * nFile, * OutFile;
 int format;
 int z;
 
 sprintf( MatrFileName, "%s.spm", SPMFileName ); 
  
 /* open files */
 if( strcmp( NetFileName, "-" )==0 ) NetFile = stdin;
   else NetFile = fopen( NetFileName, "r" );
 if( NetFile == NULL ) {printf( "*** error open file %s\n", NetFileName );exit(2);}
 MatrFile = fopen( MatrFileName, "w" );
 if( MatrFile == NULL ) {printf( "*** error open file %s\n", MatrFileName );exit(2);}
    
 /* init net size  */
 maxn=nINIT;
 maxm=mINIT; 
 maxnames=namesINIT;
 maxatp=atpINIT;
 maxapt=aptINIT;

 /* allocate arrays */
 tn = (int*) calloc( maxn, sizeof(int) ); n=0;
 
 pn = (int*) calloc( maxm, sizeof(int) ); m=0;
 
 names = (char*) calloc( maxnames, sizeof(char) ); fnames=0;
 aptp = (int*) calloc( maxapt, sizeof(int) );
 aptw = (int*) calloc( maxapt, sizeof(int) );
 aptt = (int*) calloc( maxapt, sizeof(int) ); fapt=0;
 atpp = (int*) calloc( maxatp, sizeof(int) );
 atpw = (int*) calloc( maxatp, sizeof(int) );
 atpt = (int*) calloc( maxatp, sizeof(int) ); fatp=0;

 if( tn==NULL || 
     pn==NULL || 
     names==NULL ||
     aptp==NULL || aptt==NULL || aptw==NULL ||
     atpp==NULL || atpt==NULL || atpw==NULL )
   { printf( "*** not enough memory for net\n" ); return(3); }  
   
 /* recognize input file */
 format=RecognizeFile( NetFile );

 rewind( NetFile ); 
 
 if( format==NDR )
 { 
   ReadNDR( NetFile ); 
 }
 else
 {
   ReadNET( NetFile );
 }
 if( NetFile != stdin ) fclose( NetFile );

 WriteSPM( MatrFile );
 fclose( MatrFile );
 
 sprintf( nFileName, "%s.nmp", SPMFileName );
 nFile = fopen( nFileName, "w" );
 if( nFile == NULL ) {printf( "*** error open file %s\n", nFileName );exit(2);}
 WriteNMP( nFile );
 fclose( nFile );
 
 sprintf( nFileName, "%s.nmt", SPMFileName );
 nFile = fopen( nFileName, "w" );
 if( nFile == NULL ) {printf( "*** error open file %s\n", nFileName );exit(2);}
 WriteNMT( nFile );
 fclose( nFile );

 if( strcmp( OutFileName, "-" )==0 ) OutFile = stdout; 
 else OutFile = fopen( OutFileName, "a" );
 if( OutFile == NULL ) {printf( "*** error open file %s\n", OutFileName );exit(2);}
 fprintf( OutFile, "parsed net %s\n\n", (netname==-1)? "noname": names+netname );
 fprintf( OutFile, "%d places, %d transitions\n\n", m, n );
 if( verbose ) WriteNet( OutFile );
 if( OutFile != stdout ) fclose( OutFile );  
 
 free( tn ); free(atpp); free(atpw); free(atpt);
 
 free( pn ); free(aptp); free(aptw); free(aptt);
  
 free( names );
 
 return(0);
 
}/* ReadNDRNET */

#ifdef __MAIN__
int main( int argc, char *argv[] )
{
  if( argc < 3 ) return 2;
  
  ReadNDRNET( argv[1], argv[2], "/dev/null", 1 );
}
#endif

// @ 2019 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org

