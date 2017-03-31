
/* rol2.c - default second readout list */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifndef VXWORKS
#include <sys/types.h>
#include <time.h>
#endif

#include "circbuf.h"

#define ROL_NAME__ "ROL2"
#define INIT_NAME rol2__init

#define POLLING_MODE
#define EVENT_MODE

#include "rol.h"
#include "EVENT_source.h"

/************************/
/************************/
/*
#include "coda.h"
#include "tt.h"
*/

int mynev; /*defined in tttrans.c */

/* user routines */

void rol2trig(int a, int b);
void rol2trig_done();

static void
__download()
{
  rol->poll = 1;

  return;
}

static void
__prestart()
{
  printf("INFO: Entering Prestart ROL2\n");

  /* Clear some global variables etc for a clean start */
  CTRIGINIT;

  /* init trig source EVENT */
  EVENT_INIT;

  /* Register a sync trigger source (up to 32 sources) */
  CTRIGRSS(EVENT, 1, rol2trig, rol2trig_done); /* second arg=1 - what is that ? */

  rol->poll = 1;

  rol->recNb = 0;

  printf("INFO: Prestart ROL22 executed\n");

  return;
}

static void
__end()
{
  printf("INFO: User End 2 Executed\n");

  return;
}

static void
__pause()
{
  printf("INFO: User Pause 2 Executed\n");

  return;
}

static void
__go()
{
  printf("User Go 2 Reached\n");fflush(stdout);

  mynev = 0;

  printf("INFO: User Go 2 Executed\n");fflush(stdout);

  return;
}

void
rol2trig(int a, int b)
{
  CPINIT;
  BANKINIT;
  int nASIS, iASIS[MAXBANKS];
  int ii, jj, kk;
  
  mynev ++; /* needed by ttfa.c */

  /*printf("befor bankscan\n");fflush(stdout);*/
  BANKSCAN;
  /*printf("after bankscan\n");fflush(stdout);*/

/*
  printf("\n\n\n\n\n nbanks=%d\n",nbanks);
  for(jj=0; jj<nbanks; jj++) printf("bankscan[%d]: tag 0x%08x typ=%d nr=%d nw=%d dataptr=0x%08x\n",
									jj,banktag[jj],banktyp[jj],banknr[jj],banknw[jj],bankdata[jj]);fflush(stdout);
*/
  nASIS = 0;
  for(jj=0; jj<nbanks; jj++)
  {
    datain = bankdata[jj];
    lenin = banknw[jj];
#ifndef VXWORKS
#ifndef NIOS
    /* swap input buffer (assume that data from VME is big-endian, and we are on little-endian Intel) */
    if(banktyp[jj] != 3) for(ii=0; ii<lenin; ii++) datain[ii] = LSWAP(datain[ii]);
#endif
#endif
    iASIS[nASIS++] = jj; /* remember bank number as it reported by BANKSCAN */
  }


  for(ii=0; ii<nASIS; ii++)
  {
    jj = iASIS[ii]; /* bank number as it reported by BANKSCAN */
    datain = bankdata[jj];
    lenin = banknw[jj];
    /*printf("mynev=%d: coping bank number %d (header %d 0x%08x))\n",mynev,jj,*(datain-2),*(datain-1));*/
	    
    CPOPEN(banktag[jj],banktyp[jj],banknr[jj]);
    for(kk=0; kk<lenin; kk++)
    {
      dataout[kk] = datain[kk];
      b08 += 4;
    }        
    CPCLOSE;
  }


  rol->user_storage[0] = lenout;
  CPEXIT;
  rol->user_storage[1] = 1;

  return;
}

void
rol2trig_done()
{
  return;
}  


void
__done()
{
  /* from parser */
  poolEmpty = 0; /* global Done, Buffers have been freed */

  return;
}
  
static void
__status()
{
  return;
}  
