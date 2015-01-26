
#ifdef Linux_vme

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "fadcLib.h"

#define NDIM 10000

int
main()
{
  char string[NDIM+1];
  int length = NDIM;
  fadc250UploadAll(string, length);
  printf("%s\n",string);
  exit(0);
}

#else

main()
{
  return;
}

#endif
