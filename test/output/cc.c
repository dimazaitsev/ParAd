// print clan structure on .d file

#include <stdio.h>
#include <stdlib.h>

struct ccs {
  int r;
  int n;
};

int find_clan(int r, int n, struct ccs * cc)
{
  int i,ii=-1;
//printf("find_clan: r=%d n=%d ar: ", r, n);  for(i=0;i<n;i++)printf("%d ",cc[i].r); printf("\n");
  for(i=0;i<n;i++){if(r==cc[i].r)ii=i;}

  return ii;
}

int comp_cc(const void * v1, const void * v2)
{
  struct ccs * e1 = (struct ccs *) v1;
  struct ccs * e2 = (struct ccs *) v2;

  if( e1->r < e2->r ) return 1;
  else if( e1->r > e2->r ) return -1;
  else return 0;
}

int main(int argc, char *argv[])
{
  int i=0,c,r,x,q,y,ci=0,s=0,st=0;
  FILE *f = fopen(argv[1],"r");
  struct ccs * cc;

  while(!feof(f))
  {
    fscanf(f,"%d %d %d %d %d",&c,&r,&x,&q,&y);
    if(feof(f))break;
    i++;
  }
//printf("i=%d\n",i);

  cc=malloc(i*sizeof(struct ccs));

  rewind(f);
  while(!feof(f))
  {
    fscanf(f,"%d %d %d %d %d",&c,&r,&x,&q,&y);
    if(feof(f))break;
    i=find_clan(r,ci,cc);
//printf("r=%d i=%d\n",r,i);
    if(i>=0)
      (cc[i].n)++;
    else
    {
      cc[ci].r=r;
      cc[ci].n=1;
      ci++;
    }
  }

  qsort( cc, ci, sizeof(struct ccs), comp_cc);
  
  for(i=0;i<ci;i++){
    printf("%dx%d+",cc[i].n,cc[i].r);
    s+=cc[i].n;
    st+=cc[i].n*cc[i].r;
  }
  printf(" = %drec %dcls %dtrs\n", ci, s, st);

  fclose(f);
  free(cc);
}

