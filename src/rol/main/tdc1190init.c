
/* tdc1190init.c */

/*USAGE: must be on the VME controller, type: 'tdc1190init' */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef Linux_vme

#include "tdc1190.h"


int
main(int argc, char *argv[])
{
  int ntdc;

  /* Open the default VME windows */
  vmeOpenDefaultWindows();

vmeBusLock();
  tiInit((21<<19),2,0);
  tiStatus(1);
vmeBusUnlock();

  ntdc = tdc1190Init(0x11100000,0x80000,20,0);
  if(ntdc) tdc1190Config("");

  exit(0);
}

#else

int
main()
{
  return(0);
}

#endif
