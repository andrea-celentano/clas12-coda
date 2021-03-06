
/* v1495firmware.c */

/*USAGE: must be on the VME controller, type:

cd $CLON_PARMS/firmwares

v1495firmware 0x11980000 ECALTrigger.rbf
v1495firmware 0x11980000 PCALTrigger.rbf

v1495firmware 0x11A00000 ECALTrigger.rbf
v1495firmware 0x11A00000 CTOFTrigger.rbf


cd "/usr/local/clas12/release/0.1/parms/firmwares"
mv6100:
v1495firmware(0x90A00000,"ECALTrigger.rbf",0,0)
mv5100:
v1495firmware(0xfa980000,"PCALTrigger_Board0.rbf",0,0)
v1495firmware(0xfaA00000,"PCALTrigger_Board1.rbf",0,0)
*/

/*
cd "/usr/local/clas12/release/0.1/parms/firmwares"
mv5100:
v1495firmware(0xfa980000,"PCALTrigger_Board0.rbf",0,0)
v1495firmware(0xfaA00000,"PCALTrigger_Board1.rbf",0,0)
mv6100:
v1495firmware(0x90980000,"PCALTrigger_Board0.rbf",0,0)
v1495firmware(0x90A00000,"PCALTrigger_Board1.rbf",0,0)
*/


/*
VME update
mv5100:
v1495firmware(0xfa980000,"v1495vtop.rbf",0,1)
v1495firmware(0xfaa00000,"v1495vtop.rbf",0,1)
mv6100:
v1495firmware(0x90980000,"v1495vtop.rbf",0,1)
v1495firmware(0x90a00000,"v1495vtop.rbf",0,1)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "v1495.h"

#ifdef Linux_vme

int
main(int argc, char *argv[])
{
  int res;
  char myname[256];
  unsigned int addr, laddr,user_vme;
  unsigned short rdata=0;

  unsigned int* addr1,addr2,addr3,addrBase;
  addr1=0;
  addr2=0;
  addr3=0;
  addrBase=0;
  if(argc==4)
  {
    addr = strtol(argv[1], (char **)NULL, 16);
    printf("use argument >0x%08x< as board VME address\n",addr);

    strncpy(myname, argv[2], 255);
    printf("use argument >%s< as rbf file name\n",myname);

    user_vme =  strtol(argv[3], (char **)NULL, 16);
    printf("use argument >%i< as user_VME \n",user_vme);
    if (user_vme<0||user_vme>1){
    	printf("invalid argument \n");
    	exit(1);
    }
  }
  else
  {
    printf("Usage: v1495firmware <vme_address> <rbf file> <USER(0)/VME(1)>\n");
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
	volatile V1495 *v1495 = (V1495 *) laddr;
	addrBase = &(v1495->BLTdata[0]);
	addr1 = &(v1495->configROM);
	addr2 = &(v1495->flashUSER);
	addr3 = &(v1495->geoAddr);

    printf("INFO2: 0x%08x 0x%08x 0x%08x 0x%08x \n",(addr1-addrBase),(addr2-addrBase),(addr3-addrBase),addrBase);

    /*Try to readout something go check ok*/
	res = vmeMemProbe((char *)(laddr+0x800C), 2, (char *) &rdata);
	printf("Trying to get VME firmware rev: 0x%08x\n",rdata);
    /* update firmware */
    v1495firmware(laddr,myname,0,user_vme);
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
