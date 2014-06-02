
/* dcrb2.c - second readout list for dcrb */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifndef VXWORKS
#include <sys/types.h>
#include <time.h>
#endif

#include "circbuf.h"

#define ROL_NAME__ "DCRB2"
#define INIT_NAME dcrb2__init

#define POLLING_MODE
#define EVENT_MODE

#include "rol.h"
#include "EVENT_source.h"

/************************/
/************************/

#include "coda.h"
#include "tt.h"


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


#define DC_DATA_BLOCK_HEADER      0x00
#define DC_DATA_BLOCK_TRAILER     0x01
#define DC_DATA_EVENT_HEADER      0x02
#define DC_DATA_TRIGGER_TIME      0x03
#define DC_DATA_DCRBEVT           0x08
#define DC_DATA_INVALID           0x0E
#define DC_DATA_FILLER            0x0F

/* 22-max# of blocks=slots, 256-max# of events*/
#define MAXBLOCK 22
#define MAXEVENT 256

void
rol2trig(int a, int b)
{
  CPINIT;
  unsigned long long timestamp;
  int dcrbref;
  int islot, ichan, ii, jj, npulses, type, nw, tdc, edge, tmpx0, tmpx2;
  unsigned int word;
  
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


#ifndef VXWORKS
  /* swap input buffer (assume that data from VME is big-endian, and we are on little-endian Intel) */
  for(ii=0; ii<lenin; ii++) datain[ii] = LSWAP(datain[ii]);
#endif


#ifdef DEBUG
  printf("\n**********************\nmini-bos bank: lenin=%d\n",lenin);
#endif


  CPOPEN(0xe105,1,1);

/*extract v1290 info and calculate reference*/
#ifndef VXWORKS
  word = LSWAP(datain[0]);
#else
  word = datain[0];
#endif
  nw = (word>>5)&0xFFFF;
#ifdef DEBUG
  printf("ROL2 TDC HEADER: 0x%08x (nw=%d)\n",word,nw);
#endif
dataout[0]=nw;
  for(ii=1; ii<nw; ii++)
  {
#ifndef VXWORKS
    word = LSWAP(datain[ii]);
#else
    word = datain[ii];
#endif
	ichan = (word>>21)&0x1F;
    tdc = word&0x1FFFFF;
    edge = (word>>26)&0x1;
#ifdef DEBUG
	printf("ROL2 TDC DATA: 0x%08x (channel=%d tdc=%d edge=%d)\n",word,ichan,tdc,edge);
#endif
dataout[ii]=word;

    if(edge==0)
	{
      if(ichan==0)
	  {
        tmpx0 = tdc/40;
	  }
	  else
	  {
        tmpx2 = tdc/40;

        dcrbref = tmpx0-tmpx2;
#ifdef DEBUG
        printf("dcrbref=%d\n",dcrbref);
#endif
      }
    }


  }
  datain += nw;
  lenin -= nw;
dataout+=nw;
b08+=(nw*4);
/*end if v1290*/





  /* all banks including headers are created already in rol1, just copy here */
  for(ii=0; ii<lenin; ii++)
  {
    dataout[ii] = datain[ii];
    b08 += 4;
    word = datain[ii];
#ifdef DEBUG
    printf("[%5d] 0x%08x -> ",ii,word);
#endif
	if(word & 0x80000000)
	{
	  type = (word>>27)&0xF;
	  switch(type)
	  {
		case DC_DATA_BLOCK_HEADER:
#ifdef DEBUG
		  printf(" {BLKHDR} SLOTID: %d", (word>>22)&0x1f);
		  printf(" NEVENTS: %d", (word>>11)&0x7ff);
		  printf(" BLOCK: %d\n", (word>>0)&0x7ff);
#endif
		  break;
		case DC_DATA_BLOCK_TRAILER:
#ifdef DEBUG
		  printf(" {BLKTLR} SLOTID: %d", (word>>22)&0x1f);
		  printf(" NWORDS: %d\n", (word>>0)&0x3fffff);
#endif
		  break;
		case DC_DATA_EVENT_HEADER:
#ifdef DEBUG
		  printf(" {EVTHDR} EVENT: %d\n", (word>>0)&0x7ffffff);
#endif
		  break;
		case DC_DATA_TRIGGER_TIME:
		  timestamp = (((unsigned long long)word&0xffffff)<<24) | (datain[ii+1]&0xffffff);
#ifdef DEBUG
		  printf(" {TRGTIME} timestamp=%lld ns (%lld us)\n",timestamp,timestamp/(long long)1000);
#endif
		  break;
		case DC_DATA_DCRBEVT:
#ifdef DEBUG
		  printf(" {DCRBEVT}");
		  printf(" CH: %3u", (word>>16)&0x7F);
		  printf(" TDC: %6u\n", word&0xFFFF);
#endif
		  break;
		case DC_DATA_INVALID:
		  printf(" {***DNV***}\n");
          goto exit;
		  break;
		case DC_DATA_FILLER:
#ifdef DEBUG
		  printf(" {FILLER}\n");
#endif
		  break;
		default:
		  printf(" {***DATATYPE ERROR***}\n");
          goto exit;
		  break;
	  }
	}
#ifdef DEBUG
	else
	{
	  printf("\n");
	}
#endif
  }

exit:

  CPCLOSE;
  
  /* returns full fragment length (long words) */  

#ifdef DEBUG 
  printf("return lenout=%d\n**********************\n\n",lenout);
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
