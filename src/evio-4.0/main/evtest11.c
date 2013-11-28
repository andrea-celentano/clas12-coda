
/* evtest11.c - read CLAS12-style data file */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "evio.h"

int *makeEvent(int nev);

#define NEVMAX 100000
#define NRAWSAMPLES 40
#define NSAMPLES 15
unsigned int buffer[NEVMAX];

#define MIN(a,b) (a<b)? a : b

#define etNlink(buf_m, tag_m, num_m) \
{ \
  printf("looking for tag=%d\n",tag_m); \
  for(i=0; i<40; i++) printf("[%2d] 0x%08x (%d)\n",i,buf_m[i],buf_m[i]); \
  ind = 0; \
}

int
main()
{
  int i, ii, handle, status, nevents, nlong, ind;

  printf("Event I/O tests...\n");
  status = evOpen("bigfile","r",&handle);
  printf ("Opening status %d\n",status);

  for(ii=0; ii<3; ii++)
  {
    status = evRead(handle,buffer, NEVMAX);
    if(status<0) goto out1;
    printf ("[%5d] Reading status = %d\n",ii,status);
    etNlink(buffer, 0xe101, 0);
  }

 out1:

  status = evClose(handle);
  printf ("Closing status %d\n",status);


  exit(0);
}



