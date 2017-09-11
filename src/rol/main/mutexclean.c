
/* mutexclean.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jvme.h"


#ifdef Linux_vme

int
main(int argc, char *argv[])
{
  vmeOpenDefaultWindows();
  vmeCheckMutexHealth(1);

  exit(0);
}

#else

int
main()
{
  return(0);
}
#endif
