// ParAd: Parallel Adriana 
// agg_clans.c                              
// Aggregate clans

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "uti.h"
#include "agg_clans.h"

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

int comp_agg(const void * v1, const void * v2)
{
  struct agg * e1 = (struct agg *) v1;
  struct agg * e2 = (struct agg *) v2;

  if( e1->r < e2->r ) return 1;
  else if( e1->r > e2->r ) return -1;
  else return 0;
}

int belongs_to_clan(int x,int *a,int n)
{
  int i;
  for(i=0;i<n;i++)if(a[i]==x) return 1;
  return 0;
}

int same_clan(struct asp * a,int m,int z1,int z2)
{
  int i;
  for(i=0;i<m;i++)
  {
    if(belongs_to_clan(z1,a[i].z,a[i].n) &&
       belongs_to_clan(z2,a[i].z,a[i].n)) return 1;
  }
  return 0;
}

int find_clan(int z,struct agg * g,int n)
{
  int i;
  for(i=0;i<n;i++)
    if(g[i].oldc==z) return g[i].newc;
  return 0;
}

int AggregateClans( char * NetFileName, int nz, int np, double ap, int debug )
{
 struct agg * g = malloc( nz*sizeof(struct agg) );
 struct asp * a;
 struct deo * d;
 struct cop * co;
 char xFileName[ FILENAMELEN + 1];
 char yFileName[ FILENAMELEN+1 ];
 char buf[MAXSTRLEN+1];
 FILE * xFile, * yFile;
 int i, c, r, x, q, y, ii, j, n, z, mr, m;
 int k, p, z1, z2, sr=0;

 if( g==NULL ) 
   {printf("*** not enough memory for g\n");exit(1);};

 // read decomposition digest
 sprintf( xFileName, "%s.d", NetFileName );
 xFile = fopen( xFileName, "r" );
 if( xFile == NULL ) {printf( "*** error open file %s\n", xFileName );exit(2);}
 // z r x q y
 for(i=0;i<nz;i++)
 {
   fscanf(xFile, "%d %d %d %d %d", &c, &r, &x, &q, &y);
   g[i].oldc=c; g[i].r=r; g[i].newc=0;
   sr+=r;
 }
 fclose( xFile );

// debug
// printf("%s\n", "g:");
// for(i=0;i<nz;i++)
// {
//   printf("%d %d %d\n", g[i].oldc, g[i].r, g[i].newc);
// }

 // sort decomposition digest
 
 qsort( g, nz, sizeof(struct agg), comp_agg);
 
// debug
// printf("%s\n", "sorted g:");
// for(i=0;i<nz;i++)
// {
//   printf("%d %d %d\n", g[i].oldc, g[i].r, g[i].newc);
// }

 // aggregate

 z=0;
 mr=g[0].r;
 if( ap==1.0 ) ap=4.0*(float)sr/3.0/(float)np/(float)mr; 

 for(i=0;i<nz;i++)
 {
//printf("aggregate %d\n", i);
   if( g[i].newc > 0 ) continue;
   r=g[i].r;
   g[i].newc=++z;
   ii=i+1;
   while( r <= mr*ap && ii<nz )
   {
     if( r+g[ii].r <= mr*ap )
     {
       r+=g[ii].r;
       g[ii].newc=z;
     }
     ii++;
   }
 }
//printf("aggregated\n");

 // write aggregation
 sprintf( xFileName, "%s.a", NetFileName );
 xFile = fopen( xFileName, "w" );
 if( xFile == NULL ) {printf( "*** error open file %s\n", xFileName );exit(2);}
 a = malloc(z*sizeof(struct asp));
 if( a==NULL ) 
   {printf("*** not enough memory for a\n");exit(1);};

 for(c=1;c<=z;c++)
 {
   n=0;r=0;ii=0;
   for(i=0;i<nz;i++)if(g[i].newc==c){n++; r+=g[i].r;}
   a[c-1].r=r;
   a[c-1].n=n;
   a[c-1].z=malloc(n*sizeof(int));
   if( a[c-1].z==NULL ) 
     {printf("*** not enough memory for z\n");exit(1);};
   fprintf(xFile, "%d %d %d", c, r, n);
   for(i=0;i<nz;i++)if(g[i].newc==c){fprintf(xFile, " %d", g[i].oldc); a[c-1].z[ii++]=g[i].oldc;}
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
     {printf("*** not enough memory for z\n");exit(1);};
 
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
     if(belongs_to_clan(d[i].v1,a[c-1].z,a[c-1].n) && belongs_to_clan(d[i].v2,a[c-1].z,a[c-1].n) )
       { d[i].v1=0; d[i].v2=0; d[i].w=0; }
   }

   // b) encode ends of edges 
   for(i=0;i<m;i++)
   {
     if(belongs_to_clan(d[i].v1,a[c-1].z,a[c-1].n) ) d[i].v1=-c;
     else if(belongs_to_clan(d[i].v2,a[c-1].z,a[c-1].n)) d[i].v2=-c; else ;
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
   if(same_clan(a,z,z1,z2))continue;
//printf("not same clan:\n");
   fprintf(yFile,"%d %d %d\n",p, find_clan(z1,g,nz), find_clan(z2,g,nz));
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

 return z;

} // AggregateClans

// @ 2019 Dmitry Zaitsev: daze@acm.org

