// ParAd: Parallel Adriana - utilities: gen_clans
//

static char Help[] =
"Generates an ideal clan structure\n"
">gen_clans k mc type d r\n"
"  k - number of clans;\n"
"  mc - number of equation in a clan;\n"
"  type - output format: \"spm\" - sparse matrix,\"mat\" - matrix;\n"
"  d - density (0.0-1.0);\n"
"  r - ration for the number of clan's variables.\n\n";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ELT 10

#define RAND_DENS ((((double)rand()/(double)RAND_MAX)<d)?rand():0)
#define RAND_SIGN ((rand()%2==0)?1:-1)
#define RAND_ELT (RAND_DENS%MAX_ELT)
#define RAND_CLAN (rand()%k)
#define RAND_ROW (rand()%m)
#define RAND_COL (rand()%n)
#define max(x,y) (((x)>(y))?(x):(y))

#define aa(i,j) (a[(i)*n+(j)])

int main(int argc, char* argv[]) 
{
    int k, mc;
    double d, r; 
    int i1,i2, j1, j2, w;
    int c, c1, c2, i, j, nc, m, n, *a;
    
    if(argc != 6) {printf("%s",Help); return 1;}

    k = atoi(argv[1]);
    mc = atoi(argv[2]);
    d = atof(argv[4]);
    r = atof(argv[5]);
    nc=(int)((double)mc*r);
    m = mc*k;		// rows
    n = nc*k+max(k,nc);	// columns
    a=malloc(m*n*sizeof(int));
    if(a==NULL){fprintf(stderr,"no memory\n"); return 1;};

    printf("# k=%d mc=%d f=%s d=%lf r=%lf\n", k, mc, argv[3], d, r);

    // fill-in clans
    for(c=0; c<k; c++)
    {
      for(i=c*mc;i<(c+1)*mc;i++)
      {
        for(j=c*nc;j<(c+1)*nc;j++)
        {
          aa(i,j)=RAND_SIGN*RAND_ELT;
        }
      }
    }

    // fill-in connections
    for(c=0; c<k; c++)
    {
      j=k*nc+c;
      c1=c;
      c2=(c<k-1)?c1+1:0;
      for(i=0;i<mc;i++)
      {
        aa(c1*mc+i,j)=-RAND_ELT;
        aa(c2*mc+i,j)=RAND_ELT;
      }
    }
    if(nc>k)
      for(c=k; c<nc; c++)
      {
        j=k*nc+c;
        c1=RAND_CLAN;
        c2=RAND_CLAN;
        for(i=0;i<mc;i++)
        {
          aa(c1*mc+i,j)=-RAND_ELT;
          aa(c2*mc+i,j)=RAND_ELT;
        }
      }

    // permute rows

    for(i=0; i<m; i++)
    {
      i1=1;
      i2=RAND_ROW;
      for(j=0; j<n; j++)
      {
        w=aa(i1,j);
        aa(i1,j)=aa(i2,j);
        aa(i2,j)=w;
      }
    }

    // permute columns

    for(j=0; j<n; j++)
    {
      j1=j;
      j2=RAND_COL;
      for(i=0; i<m; i++)
      {
        w=aa(i,j1);
        aa(i,j1)=aa(i,j2);
        aa(i,j2)=w;
      }
    }

    // print a 
    if(strcmp(argv[3],"mat")==0)
    {
      printf("%d %d\n", m, n);
      for(i=0;i<m;i++)
      {
        for(j=0;j<n;j++)
        {
          printf("%4d",aa(i,j));
        }
        printf("\n");
      }
    }
    else if(strcmp(argv[3],"spm")==0)
    {
      // print a as spm
      for(i=0;i<m;i++)
      {
        for(j=0;j<n;j++)
        {
          if(aa(i,j)!=0) printf("%d %d %d\n",j+1, i+1, aa(i,j));
        }
      }
    }
    else ;

    free(a);
    
    return 0;
}

