
/* dcrbfirmware.c */
/*
     cd $CLON_PARMS/firmwares
     dcrbfirmware fe_dcrb.bin
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dcrbLib.h"


#ifdef Linux_vme

int
main(int argc, char *argv[])
{
  int res;
  char myname[256];
  unsigned int addr, laddr;
  int slot = 0;

  printf("\n");
  if(argc==2)
  {
    strncpy(myname, argv[1], 255);
    printf("Use argument >%s< as bin file name\n",myname);
    printf("Upgrade all boards in crate\n");
  }
  else
  {
    printf("Usage: dcrbfirmware <bin file>\n");
    exit(0);
  }
  printf("\n");

  /* Open the default VME windows */
  vmeOpenDefaultWindows();
  printf("\n");

  dcrbInit((3<<19), 0x80000, 20, 0);
  
  dcrbGFirmwareUpdateVerify(argv[1]);

  exit(0);
}

#else

int
main()
{
  return(0);
}
#endif
