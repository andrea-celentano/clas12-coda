

/* main for codaterm */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int codaterm(int argc, char *argv[]);

int
main(int argc, char *argv[])
{
  int myargc;
  char *myargv[7];

  myargc = 0;
  myargv[myargc++] = strdup( "codaterm" );
  myargv[myargc++] = strdup( "-into" );
  myargv[myargc++] = strdup( "00_01" );

  codaterm(myargc, myargv);
  printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
  printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
  printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
}

