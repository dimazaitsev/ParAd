// ParAd: Parallel Adriana - utilities: spm_bas
//

static char Help[] =
"Sort spm basis and print a row in one line as a sequence of \"index*value\"\n"
">spm_bas spm_matrix_input_file\n\n";

#include <stdio.h>
#include <stdlib.h>

struct elt {
  int i;
  int j;
  int xij;
};

int comp (const void * elem1, const void * elem2) 
{
    struct elt *e1 = (struct elt *)elem1;
    struct elt *e2 = (struct elt *)elem2;
    if (e1->i > e2->i) return  1;
    else if (e1->i < e2->i) return  -1;
    else if (e1->j > e2->j) return  1;
    else if (e1->j < e2->j) return  -1;
    return 0;
}

#define BUFLEN 256

int main(int argc, char* argv[]) 
{
    FILE *fi;
    struct elt *e = NULL;
    char buf[BUFLEN+1];
    int i, ii, n=0;

    if(argc != 2) {printf("%s",Help); return 1;}

    fi = fopen(argv[1],"r");
    
    while(!feof(fi))
    {
      fgets(buf,BUFLEN,fi);
      if(!feof(fi))n++;
    }

    e = malloc(n*sizeof(struct elt));
    if(e==NULL){fprintf(stderr,"no memory for %d elements\n",n);return 2;}
    
    rewind(fi);

    for(i=0;i<n;i++)
    {
      fscanf(fi,"%d %d %d", &(e[i].i), &(e[i].j), &(e[i].xij));
    }

    fclose(fi);

    qsort (e, n, sizeof(struct elt), comp);

    ii=e[0].i;
fprintf(stderr,"first solution number: %d, ", ii);
    for(i=0;i<n;i++)
    {
      if(e[i].i!=ii) {ii=e[i].i;printf("\n");}
      printf("%d*%d ", e[i].j, e[i].xij);
    }
fprintf(stderr,"last solution number: %d\n", ii);

    free(e);

    return 0;
}

