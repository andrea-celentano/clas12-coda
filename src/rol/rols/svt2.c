
/* svt2.c - second readout list for svt */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifndef VXWORKS
#include <sys/types.h>
#include <time.h>
#endif

#include "circbuf.h"

#define ROL_NAME__ "SVT2"
#define INIT_NAME svt2__init

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

#define DATA_TYPE_BLKHDR		0x00
#define DATA_TYPE_BLKTLR		0x01
#define DATA_TYPE_EVTHDR		0x02
#define DATA_TYPE_TRGTIME		0x03
#define DATA_TYPE_BCOTIME		0x04
#define DATA_TYPE_FSSREVT		0x08
#define DATA_TYPE_DNV			0x0E
#define DATA_TYPE_FILLER		0x0F

/* 22-max# of blocks=slots, 256-max# of events*/
#define MAXBLOCK 22
#define MAXEVENT 256

void
rol2trig(int a, int b)
{
  CPINIT;
  int ii, type;
  unsigned int word;
  unsigned long long timestamp;
  
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



  CPOPEN(0xe104,1,1);

  /* all banks including headers are created already in rol1, just copy here */
  for(ii=0; ii<lenin; ii++)
  {
    dataout[ii] = datain[ii];
    word = datain[ii];
    b08 += 4;
#ifdef DEBUG
    printf("[%5d] 0x%08x -> ",ii,word);
#endif
	if(word & 0x80000000)
	{
	  type = (word>>27)&0xF;
	  switch(type)
	  {
		case DATA_TYPE_BLKHDR:
#ifdef DEBUG
		  printf(" {BLKHDR} SLOTID: %d", (word>>22)&0x1f);
		  printf(" NEVENTS: %d", (word>>11)&0x7ff);
		  printf(" BLOCK: %d\n", (word>>0)&0x7ff);
#endif
		  break;
		case DATA_TYPE_BLKTLR:
#ifdef DEBUG
		  printf(" {BLKTLR} SLOTID: %d", (word>>22)&0x1f);
		  printf(" NWORDS: %d\n", (word>>0)&0x3fffff);
#endif
		  break;
		case DATA_TYPE_EVTHDR:
#ifdef DEBUG
		  printf(" {EVTHDR} EVENT: %d\n", (word>>0)&0x7ffffff);
#endif
		  break;
		case DATA_TYPE_TRGTIME:
		  timestamp = (((unsigned long long)word&0xffffff)<<24) | (datain[ii+1]&0xffffff);
#ifdef DEBUG
		  printf(" {TRGTIME} timestamp=%lld (bco style = %lld)\n",timestamp,((timestamp / (long long)(16)) % 256)); /*16-from par file*/
#endif
		  break;
        case DATA_TYPE_BCOTIME:
#ifdef DEBUG
          printf(" {BCOTIME} START: %u STOP: %u\n", (word>>0) & 0xFF, (word>>16) & 0xFF);
#endif
          break;
		case DATA_TYPE_FSSREVT:

          /* calculate 'latency' and put it in a data instead of 'bco' (26*8 is bco period (ns)) */
		  /*
          oldbco = (word>>4)&0xFF;
          latency = ABS( ((timestamp / (long long)(26*8)) % 256) - (long long)((word>>4)&0xFF) );
          dataout[ii] = (dataout[ii]&0xFFFFF00F)|((latency&0xff)<<4);
          word = dataout[ii];
		  */

#ifdef DEBUG
		  printf(" {FSSREVT}");
		  printf(" HFCBID: %1u", (word>>22)&0x1);
		  printf(" CHIPID: %1u", (word>>19)&0x7);
		  printf(" CH: %3u", (word>>12)&0x7F);
		  printf(" BCO: %3u", (word>>4)&0xFF); /*printf(" BCO: %3u (old=%3u)", (word>>4)&0xFF, oldbco);*/
		  printf(" ADC: %1u", (word>>0)&0x7);
		  /*
          printf(" latency=%d BCO's (%d ns)",latency,latency*(26*8));
		  */
          printf("\n");
#endif
		  break;
		case DATA_TYPE_DNV:
		  printf(" {***DNV***}\n");
          goto exit;
		  break;
		case DATA_TYPE_FILLER:
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
#ifdef DEBUG
  printf("\n");
#endif

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
