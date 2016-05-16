
/* vscminit.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vscmLib.h"



#ifdef Linux_vme

int
main(int argc, char *argv[])
{
  int res, nvscm;
  char myname[256];
  unsigned int addr, laddr;
  int slot = 0;

  /* Open the default VME windows */
  vmeOpenDefaultWindows();
  printf("\n");

  /* update firmware */
  
  nvscm = vscmInit(0x100000,0x80000,20,0);
  vscmConfig ("");

  printf("NVSCM=%d\n",nvscm);

  {
#define MAXWORDS 4096
    int nw, slot=3, tdcbuf[MAXWORDS];
    nw = vscmReadScalers(slot, tdcbuf, MAXWORDS, 0xff, 0);
	printf("NW for scalers=%d\n",nw);
  }

  exit(0);
}

#else

int
main()
{
  return(0);
}

#endif
