// ParAd: Parallel Adriana 
// grb_mul2a.c                            
// Multiplies sparse matrices uging GraphBLAS

#include <stdio.h>
#include <stdlib.h>
#include "uti.h"
#include "GraphBLAS.h"

static int first_call=1;

//#define max(x,y) (((x)>(y))?(x):(y))

//#define LINESIZE 256

//#define __MAIN__

/*int isempty( char * s )
{
  int i=0;
  int empty=0;
  
  while( ( s[i]==' ' || s[i]==0xa || s[i]==0xd || s[i]==0x9 ) && s[i]!='\0'  ) i++;
  if( s[i]=='\0' ) empty=1;
  
  return(empty);

} */

int input_matr(GrB_Matrix *pA, char * AFileName)
{
  FILE * AFile;
  GrB_Info ret;
  GrB_Index *pi, *pj;
  int32_t *px; 
  int i,j,l,k,m,n,x; 
  char line[ MAXSTRLEN+1 ];

  AFile = fopen( AFileName, "r" );
  if( AFile == NULL ) {printf("*** error open file %s\n", AFileName ); exit(0);}

  /* estimate matrix's A size */
  l=m=n=0;
  while ( ! feof( AFile ) )
  {
    fgets( line, MAXSTRLEN, AFile );
    if( line[0] == '#' || isempty(line) ) continue;
    if( feof( AFile ) ) break;   
    x=1;
    sscanf( line, "%d %d %d", &i, &j, &x );
    m=max(m,i);
    n=max(n,j);
    l++;   
  } /* feof AFile */
  
  rewind( AFile );
  pi = malloc( l * sizeof(GrB_Index) );
  pj = malloc( l * sizeof(GrB_Index) );
  px = malloc( l * sizeof(GrB_INT32) );
  if( pi==NULL || pj==NULL || px==NULL ) {printf("*** not enough memory for pi, pj, px\n");exit(1);}
  
  /* read matrix A */
  k=0;
  while ( ! feof( AFile ) )
  {
    fgets( line, MAXSTRLEN, AFile );
    if( line[0] == '#' || isempty(line) ) continue;
    if( feof( AFile ) ) break;
    x=1;
    sscanf( line, "%d %d %d", &i, &j, &x );i--;j--;
//printf("%d %d %d\n", i, j, x );
    pi[k]=(GrB_Index)i;
    pj[k]=(GrB_Index)j;
    px[k++]=(int32_t)x;    
  } /* feof AFile */

//printf("m=%d, n=%d, l=%d\n",m,n,l);
  
  fclose( AFile );

  ret = GrB_Matrix_new(pA,GrB_INT32,m,n);
  if(ret!=GrB_SUCCESS) {printf("*** GrB error (GrB_Matrix_new=%d) \n",ret); exit(0);}

  ret = GrB_Matrix_build(*pA,pi,pj,px,(int64_t)l,GrB_PLUS_INT32);
  if(ret!=GrB_SUCCESS) {printf("*** GrB error (GrB_Matrix_build=%d) \n",ret); exit(0);}

  free(pi);free(pj);free(px);
  return(0);
}

int output_matr(GrB_Matrix C, char * CFileName)
{
  FILE * CFile;
  GrB_Index *pi, *pj, l;
  int32_t *px;
  GrB_Info ret;
  int k;
   
  // get tuples
  ret = GrB_Matrix_nvals(&l,C);
  pi = calloc( l, sizeof(GrB_Index) );
  pj = calloc( l, sizeof(GrB_Index) );
  px = calloc( l, sizeof(GrB_INT32) );
  if( pi==NULL || pj==NULL || px==NULL ) {printf("*** not enough memory for pi, pj, px\n");exit(1);}
  ret = GrB_Matrix_extractTuples(pi,pj,px,&l,C);

  // write tuples
  CFile = fopen( CFileName, "w" );
  if( CFile == NULL ) {printf("*** error open file %s\n", CFileName ); exit(0);}
  for(k=0;k<l;k++)
  {
    if((int)px[k]!=0) fprintf(CFile,"%d %d %d\n",(int)++(pi[k]),(int)++(pj[k]),(int)px[k]);
  }
  free(pi);free(pj);free(px);
  fclose( CFile );
  return(0);
}

int AdjustAB(GrB_Matrix *pA,GrB_Matrix *pB)
{
  GrB_Index m1,n1,m2,n2;
  GrB_Index *pi, *pj, l;
  int32_t *px;
  GrB_Info ret;

  GrB_Matrix_nrows(&m1,*pA); 
  GrB_Matrix_ncols(&n1,*pA);
  GrB_Matrix_nrows(&m2,*pB);
  GrB_Matrix_ncols(&n2,*pB);

//printf("AdjustAB: m1=%d, n1=%d, m2=%d, n2=%d\n",(int)m1,(int)n1,(int)m2,(int)n2);

  if(n1<m2)
  {
    ret = GrB_Matrix_nvals(&l,*pA);
    pi = calloc( l, sizeof(GrB_Index) );
    pj = calloc( l, sizeof(GrB_Index) );
    px = calloc( l, sizeof(GrB_INT32) );
    if( pi==NULL || pj==NULL || px==NULL ) {printf("*** not enough memory for pi, pj, px\n");exit(1);}
    ret = GrB_Matrix_extractTuples(pi,pj,px,&l,*pA);
    GrB_free(pA);
    ret = GrB_Matrix_new(pA,GrB_INT32,m1,m2);
    if(ret!=GrB_SUCCESS) {printf("*** GrB error (GrB_Matrix_new=%d) \n",ret); exit(0);}
    ret = GrB_Matrix_build(*pA,pi,pj,px,(int64_t)l,GrB_PLUS_INT32);
    if(ret!=GrB_SUCCESS) {printf("*** GrB error (GrB_Matrix_build=%d) \n",ret); exit(0);}
    free(pi);free(pj);free(px);
  }
  else if(n1>m2)
  {
    ret = GrB_Matrix_nvals(&l,*pB);
    pi = calloc( l, sizeof(GrB_Index) );
    pj = calloc( l, sizeof(GrB_Index) );
    px = calloc( l, sizeof(GrB_INT32) );
    if( pi==NULL || pj==NULL || px==NULL ) {printf("*** not enough memory for pi, pj, px\n");exit(1);}
    ret = GrB_Matrix_extractTuples(pi,pj,px,&l,*pB);
    GrB_free(pB);
    ret = GrB_Matrix_new(pB,GrB_INT32,n1,n2);
    if(ret!=GrB_SUCCESS) {printf("*** GrB error (GrB_Matrix_new=%d) \n",ret); exit(0);}
    ret = GrB_Matrix_build(*pB,pi,pj,px,(int64_t)l,GrB_PLUS_INT32);
    if(ret!=GrB_SUCCESS) {printf("*** GrB error (GrB_Matrix_build=%d) \n",ret); exit(0);}
    free(pi);free(pj);free(px);
  }
  else ;

  return(0);
}

void MultiplySPM( char * AFileName, char * BFileName, char * CFileName )
{
  GrB_Matrix A, B, C;
  GrB_Info ret; 
  GrB_Index m1,n1,m2,n2;

  if(first_call)
  {
    ret = GrB_init(GrB_BLOCKING); 
    if(ret!=GrB_SUCCESS) {printf("*** GrB error (GrB_init=%d) \n",ret); exit(0);}
    first_call=0;
  }

  if( input_matr(&A,AFileName)!=0 ) {printf("*** error open file %s\n", AFileName ); exit(0);}

  if( input_matr(&B,BFileName)!=0 ) {printf("*** error open file %s\n", BFileName ); exit(0);} 

  GrB_Matrix_nrows(&m1,A);
  GrB_Matrix_ncols(&n1,A);
  GrB_Matrix_nrows(&m2,B);
  GrB_Matrix_ncols(&n2,B);

//printf("MultiplyingAB: m1=%d, n1=%d, m2=%d, n2=%d\n",(int)m1,(int)n1,(int)m2,(int)n2);fflush(stdout);

  if(n1!=m2) 
  {
    if(AdjustAB(&A,&B)!=0) {printf("*** error AdjustAB\n"); exit(0);} 
  }  

  ret = GrB_Matrix_new(&C,GrB_INT32,m1,n2);
  if(ret!=GrB_SUCCESS) {printf("*** GrB error (GrB_Matrix_new=%d) \n",ret); exit(0);}

  ret = GrB_mxm(C, NULL, NULL, GxB_PLUS_TIMES_UINT32, A, B, NULL);
  if( ret!=GrB_SUCCESS) {printf("*** GrB error (GrB_mxm=%d) \n",ret); exit(0);}

  GrB_free(&A);
  GrB_free(&B);

  if( output_matr(C,CFileName)!=0 ) {printf("*** error open file %s\n", CFileName ); exit(0);}

//printf("MultipliedAB: m1=%d, n1=%d, m2=%d, n2=%d\n",(int)m1,(int)n1,(int)m2,(int)n2);

  GrB_free(&C); 
  GrB_finalize();

}/* MultiplySPM */

#ifdef __MAIN__
int main( int argc, char *argv[] )
{
  if( argc < 3 )
  {
    printf("mul2m: multiplication of sparse matrixes\n");
    printf(">mul2m matrAfile matrBfile matrCfile\n");
    
    exit(0);
  }
  MultiplySPM( argv[1], argv[2], argv[3] );
}/* main */
#endif

// @ 2018 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org

