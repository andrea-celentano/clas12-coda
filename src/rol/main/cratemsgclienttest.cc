
/* cratemsgclienttest.cc - testbed for cratemsgclient class
      usage example:      ./Linux_i686_vme/bin/cratemsgclienttest hps11 6102
*/

#if defined(Linux_vme)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "cratemsgclient.h"

CrateMsgClient *tcp;

#define LSWAP(x)        ((((x) & 0x000000ff) << 24) | \
                         (((x) & 0x0000ff00) <<  8) | \
                         (((x) & 0x00ff0000) >>  8) | \
                         (((x) & 0xff000000) >> 24))



unsigned int *buf;

int
main(int argc, char *argv[])
{
  char hostname[256];
  int hostport, len, ii, ret, slot, chan, partype;
  unsigned int buffer[100];

  printf("argc=%d\n",argc);fflush(stdout);

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

  printf("111\n");fflush(stdout);
  ret = tcp->GetCrateMap(&buf, &len);
  printf("222: len=%d\n",len);fflush(stdout);
  for(ii=0; ii<len; ii++) {printf("slot %2d, boardID 0x%08x\n",ii,buf[ii]);fflush(stdout);}


  slot = 2;
  partype = SCALER_PARTYPE_THRESHOLD;
  ret = tcp->GetBoardParams(slot, partype, &buf, &len);

  partype = SCALER_PARTYPE_THRESHOLD2;
  ret = tcp->GetBoardParams(slot, partype, &buf, &len);

  chan = 5;
  ret = tcp->GetChannelParams(slot, chan, partype, &buf, &len);

  ret = tcp->GetChannelParams(slot, chan, partype, &buf, &len);

  sleep(3);


  printf("-------------------------\n");
  while(1)
  {
	/*
    slot=3;
    ret = tcp->ReadScalers(slot, &buf, &len);
    printf("ret=%d, len=%d slot=%d\n",ret,len, slot);
    for(ii=0; ii<len; ii++) printf("  [%2d] 0x%08x (swap 0x%08x)\n",ii,buf[ii],LSWAP(buf[ii]));fflush(stdout);
    delete [] buf;

    slot=4;
    ret = tcp->ReadScalers(slot, &buf, &len);
    printf("ret=%d, len=%d slot=%d\n",ret,len, slot);
    for(ii=0; ii<len; ii++) printf("  [%2d] 0x%08x (swap 0x%08x)\n",ii,buf[ii],LSWAP(buf[ii]));fflush(stdout);
    delete [] buf;
	*/

    slot=2;
    ret = tcp->ReadScalers(slot, &buf, &len);
    printf("ret=%d, len=%d slot=%d\n",ret,len, slot);
    for(ii=0; ii<len; ii++) printf("  [%2d] 0x%08x (swap 0x%08x)\n",ii,buf[ii],LSWAP(buf[ii]));fflush(stdout);
    delete [] buf;

    slot=10;
    ret = tcp->ReadScalers(slot, &buf, &len);
    printf("ret=%d, len=%d slot=%d\n",ret,len, slot);
    for(ii=0; ii<len; ii++) printf("  [%2d] 0x%08x (swap 0x%08x)\n",ii,buf[ii],LSWAP(buf[ii]));fflush(stdout);
    delete [] buf;

    printf("\n\n");

    sleep(1);
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
