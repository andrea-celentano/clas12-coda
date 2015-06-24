
/* tdc2.c - second readout list for v1190/v1290 TDCs */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifndef VXWORKS
#include <sys/types.h>
#include <time.h>
#endif

#include "circbuf.h"

#define ROL_NAME__ "TDC2"
#define INIT_NAME tdc2__init

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
  unsigned int word;
  int islot, ichan, ii, jj, npulses, type, nw, tdc, edge, tmpx0, tmpx2;
  int banknum = 0;
  
  mynev ++; /* needed by ttfa.c */

  /*
  CPOPEN(rol->pid,1,0);
  for(ii=0; ii<lenin; ii++)
  {
    dataout[ii] = datain[ii];
    b08 += 4;
  }
  CPCLOSE;

  CPOPEN(rol->pid,1,1);
  for(ii=0; ii<lenin; ii++)
  {
    dataout[ii] = datain[ii]+0x100;
    b08 += 4;
  }
  CPCLOSE;
  */

  banknum=rol->pid;

#ifndef VXWORKS
  /* swap input buffer (assume that data from VME is big-endian, and we are on little-endian Intel) */
  for(ii=0; ii<lenin; ii++) datain[ii] = LSWAP(datain[ii]);
#endif

#ifdef DEBUG
  printf("\n**********************\ntttdc: mini-bos bank: lenin=%d\n",lenin);
#endif

  CPOPEN(0xe107,1,banknum);

  while(lenin>0)
  {
#ifndef VXWORKS
    word = LSWAP(datain[0]);
#else
    word = datain[0];
#endif
    nw = (word>>5)&0xFFFF;

#ifdef DEBUG
    printf("tttdc: lenin=%d, nw=%d\n",lenin,nw);
#endif

    if(nw>0)
    {
#ifdef DEBUG
      printf("tttdc: ROL2 TDC HEADER: 0x%08x (nw=%d)\n",word,nw);
#endif
      for(ii=1; ii<nw; ii++)
      {
#ifndef VXWORKS
        word = LSWAP(datain[ii]);
#else
        word = datain[ii];
#endif

        islot = (word>>27)&0x1F;
        edge = (word>>26)&0x1;
        ichan = (word>>19)&0x7F;
        tdc = word&0x7FFFF;

#ifdef DEBUG
	    printf("tttdc: ROL2 TDC DATA: 0x%08x (slot=%d channel=%d tdc=%d edge=%d)\n",word,islot,ichan,tdc,edge);
#endif

        /* output data in 'standard' format: slot[27-31]  edge[26]  chan[19-25]  tdc[0-18] */
        *dataout++ = (islot<<27) + (edge<<26) + (ichan<<19) + (tdc&0x7FFFF);
        b08 += 4;
      }
      datain += nw;
      lenin -= nw;
    }
  }

  CPCLOSE;

  /* returns full fragment length (long words) */  

#ifdef DEBUG 
  printf("tttdc: return lenout=%d\n**********************\n\n",lenout);
#endif


  CPEXIT;

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
