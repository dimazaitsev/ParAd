#include <stdio.h>
#include <stdlib.h>
#include "../src/uti.h"

#define __MAIN__

#ifdef __MAIN__
int main( int argc, char *argv[] )
{
  if( argc < 2 )
  {
    printf("transpose: transposition of sparse matrixes\n");
    printf("transpose matrAfile matrBfile\n");
    
    exit(0);
  }
  TransposeMatrix( argv[1], argv[2] );
}/* main */
#endif

