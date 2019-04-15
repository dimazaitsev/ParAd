// ParAd: Parallel Adriana 
// agg_metis.c                              
// Aggregate clans via graph partitioning by METIS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "uti.h"
#include "metis.h"
#include "agg_metis.h"

struct agg {
  int oldc;
  int r;
  int newc;
};

struct asp {
  int r;
  int n;
  int *z;
};

struct deo {
  int v1;
  int v2;
  int w;
};

struct cop {
  int p;
  int z1;
  int z2;
};

int belongs_to_clan1(int x,int *a,int n)
{
  int i;
  for(i=0;i<n;i++)if(a[i]==x) return 1;
  return 0;
}

int same_clan1(struct asp * a,int m,int z1,int z2)
{
  int i;
  for(i=0;i<m;i++)
  {
    if(belongs_to_clan1(z1,a[i].z,a[i].n) &&
       belongs_to_clan1(z2,a[i].z,a[i].n)) return 1;
  }
  return 0;
}

int comp_int(const void * v1, const void * v2)
{
  int * e1 = (int *) v1;
  int * e2 = (int *) v2;

  if( *e1 > *e2 ) return 1;
  else if( *e1 < *e2 ) return -1;
  else return 0;
}

int GoodPartition(int * part,int nz,int nparts)
{
  int i, ii, good=1;
  int *p=malloc(nz*sizeof(int));
  if( p==NULL ) 
   {printf("*** not enough memory for p\n");exit(1);};
  memcpy(p,part,nz*sizeof(int));
  qsort( p, nz, sizeof(int), comp_int);
  ii=p[0];
  if(ii!=0) good=0;
  for(i=0;(i<nz)&good;i++)
  {
    if(p[i]==ii) continue;
    else if(p[i]==ii+1) ii++;
    else good=0;
  }
  if(ii!=nparts-1) good=0;
  free(p);
  return(good);
}

int AggregateClansMETIS( char * NetFileName, int nz, int np, int kway, int debug )
{
 struct asp * a;
 struct deo * d;
 struct cop * co;
 char xFileName[ FILENAMELEN + 1];
 char yFileName[ FILENAMELEN+1 ];
 char buf[MAXSTRLEN+1];
 FILE * xFile, * yFile;
 int i, c, r, x, q, y, ii, j, n, z, mr, m;
 int k, p, z1, z2;

 char partFileName[ FILENAMELEN+1 ];
 char command[ FILENAMELEN+1 ];
 FILE * partFile;
 int nparts, ret;
 int *part;
 struct agg * g;

 nparts=(kway==1)?np:kway;
 z=nparts;
 if(nparts>=nz)
   return nz;

 part=malloc(nz*sizeof(int));
 g = malloc( nz*sizeof(struct agg) );
 if( part==NULL || g==NULL ) 
   {printf("*** not enough memory for part, g\n");exit(1);};

// read decomposition digest
 sprintf( xFileName, "%s.d", NetFileName );
 xFile = fopen( xFileName, "r" );
 if( xFile == NULL ) {printf( "*** error open file %s\n", xFileName );exit(2);}
 // z r x q y
 for(i=0;i<nz;i++)
 {
   fscanf(xFile, "%d %d %d %d %d", &c, &r, &x, &q, &y);
   g[i].oldc=c; g[i].r=r; g[i].newc=0;
 }
 fclose( xFile );


// run metis

 sprintf(command,"gpmetis %s.metis %d %s",NetFileName, nparts, (debug<2)?">/dev/null":"");

 ret=system(command);  
 if(ret!=0) 
    printf("*** error %d running: %s\n", ret, command);  
 

 // read partitioning file
 
 sprintf( partFileName, "%s.metis.part.%d", NetFileName, nparts );
 partFile = fopen( partFileName, "r" );
 if( partFile == NULL ) {printf( "*** error open file %s\n", partFileName );exit(2);}

 for(i=0;i<nz;i++)
   fscanf(partFile,"%d",part+i);

 fclose(partFile);

//printf("part: ");for(i=0;i<nz;i++)printf("%d ",part[i]);printf("\n");

 if(!GoodPartition(part,nz,nparts))
 {
   fprintf(stderr,"*** METIS partition into %d parts failed, no aggregation resumed\n",nparts);
   if(debug<2) 
     remove( partFileName ); 
   free(part);
   return nz;
 }

////////////////////////////////////////////////////////// old

// write aggregation
 sprintf( xFileName, "%s.a", NetFileName );
 xFile = fopen( xFileName, "w" );
 if( xFile == NULL ) {printf( "*** error open file %s\n", xFileName );exit(2);}
 a = malloc(nparts*sizeof(struct asp));
 if( a==NULL ) 
   {printf("*** not enough memory for a\n");exit(1);};

 for(c=0;c<nparts;c++)
 {
   n=0;r=0;ii=0;
   for(i=0;i<nz;i++)if(part[i]==c){n++; r+=g[i].r;}
   a[c].r=r;
   a[c].n=n;
   a[c].z=malloc(n*sizeof(int));
   if( a[c].z==NULL ) 
     {printf("*** not enough memory for z\n");exit(1);};
   fprintf(xFile, "%d %d %d", c+1, r, n);
//printf("agg: %d %d %d\n", c+1, r, n);
   for(i=0;i<nz;i++)if(part[i]==c){fprintf(xFile, " %d", i+1); a[c].z[ii++]=i+1;}
   fprintf(xFile, "\n");
 }
 fclose( xFile );
 
 // read .p - deco graph (edges)
 sprintf( xFileName, "%s.p", NetFileName );
 xFile = fopen( xFileName, "r" );
 if( xFile == NULL ) {printf( "*** error open file %s\n", xFileName );exit(2);}

 m=0;
 while( ! feof( xFile ) )
 {
   fgets( buf, MAXSTRLEN, xFile);
   if( feof( xFile) ) break;
   if( buf[0]=='#' || isempty(buf) ){ continue; }
   m++;
 }
 d = malloc(m*sizeof(struct deo));
 if( d==NULL ) 
   {printf("*** not enough memory for d\n");exit(1);};
 rewind( xFile );
 i=0;
 while( ! feof( xFile ) )
 {
   fgets( buf, MAXSTRLEN, xFile);
   if( feof( xFile) ) break;
   if( buf[0]=='#' || isempty(buf) ){ continue; }
   sscanf(buf,"%d %d %d", &(d[i].v1), &(d[i].v2), &(d[i].w));
   i++;
 }
 fclose( xFile );

 // aggregate on new clans
 for(c=1;c<=z;c++)
 {
   if(a[c-1].n==1)
   {
     sprintf( xFileName, "%s.z%d", NetFileName, a[c-1].z[0] );
     sprintf( yFileName, "%s.zz%d", NetFileName, c );
     //rename( xFileName, yFileName );
     CopyFile( xFileName, yFileName, "w" );
     
     // change clan number
     for(i=0;i<m;i++)
       if(d[i].v1==a[c-1].z[0]) d[i].v1=-c;
       else if(d[i].v2==a[c-1].z[0]) d[i].v2=-c; else ;

     continue;
   }

   // aggregate deco graph

   // a) delete internal edges - two ends belong to aggregated clan
   for(i=0;i<m;i++)
   {
     if(belongs_to_clan1(d[i].v1,a[c-1].z,a[c-1].n) && belongs_to_clan1(d[i].v2,a[c-1].z,a[c-1].n) )
       { d[i].v1=0; d[i].v2=0; d[i].w=0; }
   }

   // b) encode ends of edges 
   for(i=0;i<m;i++)
   {
     if(belongs_to_clan1(d[i].v1,a[c-1].z,a[c-1].n) ) d[i].v1=-c;
     else if(belongs_to_clan1(d[i].v2,a[c-1].z,a[c-1].n)) d[i].v2=-c; else ;
   }

   // aggregate files of clans

   sprintf( xFileName, "%s.zz%d", NetFileName, c );
   xFile = fopen( xFileName, "w" );
   if( xFile == NULL ) {printf( "*** error open file %s\n", xFileName );exit(2);}
   fclose( xFile );

   for(i=0;i<a[c-1].n;i++)
   {
     sprintf( yFileName, "%s.z%d", NetFileName, a[c-1].z[i] );
     CopyFile( yFileName, xFileName, "a" );
   }

 } // for c - aggregate on clans

 // write .p file

 // c) unite parallel edges
 for(i=0;i<m;i++)
    for(j=i+1;j<m;j++)
    {
      if((d[i].v1!=0) && 
         (((d[i].v1==d[j].v1) && (d[i].v2==d[j].v2)) ||
          ((d[i].v1==d[j].v2) && (d[i].v2==d[j].v1))))
      {
        d[i].w+=d[j].w; 
        d[j].v1=0; d[j].v2=0; d[j].w=0;
      } 
    }

 // write to file & recode from negative
 sprintf( xFileName, "%s.pp", NetFileName );
 xFile = fopen( xFileName, "w" );
 if( xFile == NULL ) {printf( "*** error open file %s\n", xFileName );exit(2);}
 for(i=0;i<m;i++)
   if(d[i].v1!=0) 
     fprintf(xFile,"%d %d %d\n",min(-d[i].v1,-d[i].v2),max(-d[i].v1,-d[i].v2),d[i].w);
 fclose( xFile );
 
 // aggregate contact places & write .zz file
 sprintf( xFileName, "%s.z", NetFileName );
 xFile = fopen( xFileName, "r" );
 if( xFile == NULL ) {printf( "*** error open file %s\n", xFileName );exit(2);}
 sprintf( yFileName, "%s.zz", NetFileName );
 yFile = fopen( yFileName, "w" );
 if( xFile == NULL ) {printf( "*** error open file %s\n", yFileName );exit(2);}
 fscanf(xFile,"%d",&k);
 fprintf(yFile,"%d\n",z);
//printf("zz:\n");
 while( ! feof( xFile ) )
 {
   fgets( buf, MAXSTRLEN, xFile);
   if( feof( xFile) ) break;
   if( buf[0]=='#' || isempty(buf) ){ continue; }
   sscanf(buf,"%d %d %d",&p, &z1, &z2);
//printf("%d %d %d\n",p, z1, z2);
   if(same_clan1(a,z,z1,z2))continue;
//printf("not same clan:\n");
   fprintf(yFile,"%d %d %d\n",p, part[z1-1]+1, part[z2-1]+1);
 }
 fclose( xFile );
 fclose( yFile );

 // delete & rename files & save files when debugging

 sprintf( xFileName, "%s.d", NetFileName );
 if(debug<2) remove( xFileName ); else
 {
   sprintf( yFileName, "%s.old.d", NetFileName );
   rename( xFileName, yFileName );
 }

 sprintf( xFileName, "%s.a", NetFileName );
 if(debug<2) remove( xFileName ); else
 {
   sprintf( yFileName, "%s.old.a", NetFileName );
   rename( xFileName, yFileName );
 }

 sprintf( xFileName, "%s.p", NetFileName );
 if(debug<2) remove( xFileName ); else
 {
   sprintf( yFileName, "%s.old.p", NetFileName );
   rename( xFileName, yFileName );
 }
 sprintf( yFileName, "%s.pp", NetFileName );
 rename( yFileName, xFileName ); 

 sprintf( xFileName, "%s.z", NetFileName );
 if(debug<2) remove( xFileName ); else
 {
   sprintf( yFileName, "%s.old.z", NetFileName );
   rename( xFileName, yFileName );
 }
 sprintf( yFileName, "%s.zz", NetFileName );
 rename( yFileName, xFileName ); 

 for(c=1;c<=nz;c++)
 {
   sprintf( xFileName, "%s.z%d", NetFileName, c );
   if(debug<2) remove( xFileName ); else
   {
     sprintf( yFileName, "%s.old.z%d", NetFileName, c );
     rename( xFileName, yFileName );
   }
 }

 for(c=1;c<=z;c++)
 {
   sprintf( xFileName, "%s.zz%d", NetFileName, c );
   sprintf( yFileName, "%s.z%d", NetFileName, c );
   rename( xFileName, yFileName );
 }
   
 if(debug<2) 
 {
   sprintf( partFileName, "%s.metis.part.%d", NetFileName, nparts );
   remove( partFileName ); 
 }

//////////////////////////////////////////////////////////

 free(part);
 return nparts;

} // AggregateClansMETIS

// @ 2019 Dmitry Zaitsev: daze@acm.org


