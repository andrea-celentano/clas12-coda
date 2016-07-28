
/* dsc2init.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "flpLib.h"



#ifdef Linux_vme

int
main(int argc, char *argv[])
{
  int res;
  unsigned int addr, laddr;
  int slot = 0;
  float v1 = 0, v2 = 0, v3 = 0;
  
  /* Open the default VME windows */
  vmeOpenDefaultWindows();
 
  /* from Download */
  vmeBusLock();
  flpInit(0x300000,20); /* 0x300000 - FLP in slot 6 */
  if (argc == 3){
    v3 = atof(argv[2]);
    printf("set voltage, %4.3f\n", v3);
    if (atoi(argv[1]) == 1){
      
      flpEnableOutput(0);
      flpEnableOutput(1);
      flpEnableIntPulser(0);
      flpEnableIntPulser(1);
      flpSetOutputVoltages(0, 3.5, 3.5, v3);
      flpSetOutputVoltages(1, 3.5, 3.5, v3);
      flpSetPulserPeriod(0, 200000);
      flpSetPulserPeriod(1, 200000);
    }
    else{
      if (atoi(argv[1]) == 0){
	flpDisableOutput(0);
	flpDisableOutput(1);
	flpDisableIntPulser(1);
	flpDisableIntPulser(0);
      }
      else {
	printf("Input 1 to tunr LED on, 0 to turn LED off.\n");
      }
    }
  }
  else printf ("Must be two arguments\n");
  flpStatus(0);
  vmeBusUnlock();
  exit(0);
}

#else

int
main()
{
  return(0);
}

#endif
