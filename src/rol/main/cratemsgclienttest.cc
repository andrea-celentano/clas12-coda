
/* cratemsgclienttest.cc - testbed for cratemsgclient class
      usage example:      ./Linux_i686_vme/bin/cratemsgclienttest hps11 6102
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "cratemsgclient.h"

CrateMsgClient *tcp;

int
main(int argc, char *argv[])
{
  char hostname[256];
  int hostport, len, ii, ret, slot;
  unsigned int **buf;

  if(argc==3)
  {
    strncpy(hostname, argv[1], 255);
    hostport = atoi(argv[2]);
    printf("use arguments >%s< as hostname and >%d< as hostport\n",hostname,hostport);
  }
  else
  {
    printf("Usage: cratemsgclienttest <hostname> <port>\n");
    exit(0);
  }

  tcp = new CrateMsgClient(hostname,hostport);
  if(tcp->IsValid())
  {
    printf("Connected\n");
  }
  else
  {
    printf("NOT CONNECTED - EXIT\n");
    exit(0);
  }


  slot = 2;
  while(1)
  {
    ret = tcp->ReadScalers(slot, buf, &len);
    printf("ret=%d, len=%d\n",ret,len);
    for(ii=0; ii<len; ii++) printf("  [%2d] 0x%08x\n",ii,(*buf)[ii]);fflush(stdout);
    sleep(1);
  }


  exit(0);
}

