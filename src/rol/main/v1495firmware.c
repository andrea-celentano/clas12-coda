
/* v1495firmware.c */

/*USAGE: must be on the VME controller, type:

v1495firmware 0x11110000 FCALTrigger_panel1b.rbf

*/




/*
v1495firmware(0xfa510000,"v1495USER1.0.rbf",0,0)
v1495firmware(0xfa510000,"v1495vtop.rbf",0,1)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef VXWORKS

int
v1495firmware()
{
  return(0);
}

#else

int
main(int argc, char *argv[])
{
  int res;
  char myname[256];
  unsigned int addr, laddr;

  if(argc==3)
  {
    addr = strtol(argv[1], (char **)NULL, 16);
    printf("use argument >0x%08x< as board VME address\n",addr);

    strncpy(myname, argv[2], 255);
    printf("use argument >%s< as rbf file name\n",myname);
  }
  else
  {
    printf("Usage: v1495firmware <vme_address> <rbf file>\n");
    exit(0);
  }

  /* Open the default VME windows */
  vmeOpenDefaultWindows();

  /* get address in A24 space */
  res = vmeBusToLocalAdrs(0x39,(char *)addr,(char **)&laddr);
  if (res != 0) 
  {
	printf("ERROR in vmeBusToLocalAdrs(0x39,0x%x,&laddr) \n",addr);
  }
  else
  {
    printf("INFO: addr=0x%08x, laddr=0x%08x\n",addr,laddr);
    /* update firmware */
    v1495firmware(laddr,myname,0,0);
  }

  exit(0);
}

#endif
