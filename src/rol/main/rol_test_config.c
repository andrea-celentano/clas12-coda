
/* rol_test_config.c - testing program for boards config files */

#ifdef Linux_vme


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "fadcLib.h"

#define NDIM 10000



/* possible arguments:

    ""
    "/usr/clas12/release/0.2/parms/fadc250/hps1.cnf"
    "/usr/clas12/release/0.2/parms/trigger/clasdev.cnf"

*/

int
main(int argc,char* argv[])
{
  char *txt = "/usr/clas12/release/0.2/parms/trigger/clasdev.cnf";

  if (argc>1) sprintf(txt,"%s",argv[1]);
  printf(stderr,txt);//"\nReading %s ....\n\n",txt);

  fadc250Config(txt);
  tdc1190Config(txt);
  tiConfig(txt);
  dsc2Config(txt);
  sspConfig(txt);
  /*gtpConfig(txt); NIOS only*/

  exit(0);
}

#else

main(int argc,char* argv[])
{
  exit(0);
}

#endif
