
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
main()
{
  char *txt = "/usr/clas12/release/0.2/parms/trigger/clasdev.cnf";

  fadc250Config(txt);
  tdc1190Config(txt);
  tiConfig(txt);
  dsc2Config(txt);
  sspConfig(txt);
  /*gtpConfig(txt);*/

  exit(0);
}

#else

main()
{
  return;
}

#endif
