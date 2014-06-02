
/* dcrbgtp2.c - second readout list for gtp in dcrb crate */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifndef VXWORKS
#include <sys/types.h>
#include <time.h>
#endif

#include "circbuf.h"

#define ROL_NAME__ "DCRBGTP2"
#define INIT_NAME dcrbgtp2__init

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

void
rol2trig(int a, int b)
{
  CPINIT;
  int ii;
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

  CPOPEN(0xe108,1,1);


  /* all banks including headers are created already in rol1, just copy here */
  for(ii=0; ii<lenin; ii++)
  {
    dataout[ii] = datain[ii];
    b08 += 4;
    word = datain[ii];
#ifdef DEBUG
    printf("data word [%5d] = 0x%08x\n",ii,word);
#endif
	if(ii==0) /* gtp event number */
	{
      printf("\n\nGTP event %d\n",word);
	}
	else if(ii==1) /* first trigger time word */
	{
      ;
	}
	else if(ii==2) /* second trigger time word */
	{
	  timestamp = (((unsigned long long)datain[ii]&0xffff)<<32) | (datain[ii-1]);
#ifdef DEBUG
	  printf("GTP timestamp=%lld ns (%lld us)\n",timestamp,timestamp/(long long)1000);
#endif
	}
  }

  CPCLOSE;
  
  /* returns full fragment length (long words) */  

#ifdef DEBUG
  {
    unsigned int *gsegm;

    for(ii=0; ii<16; ii++)
    {
      for(jj=0; jj<112; jj++)
      {
        sl[0][ii][jj] = 0;
        sl[1][ii][jj] = 0;
	  }
      k1 = k2 = k3 = 0;
    }

    gsegm = (unsigned int *) &datain[3];


        k3 = 0; /*2 superlayers*/
        k2 = 0; /*16 angles*/
        k1 = 0; /*112 wires*/
        for(ii=0; ii<112; ii++)
		{ 
#ifdef DEBUG1
          printf("===> gsegm[%3d] = 0x%08x\n",ii,gsegm[ii]);
#endif
          if(ii==56)
		  {
            k3 = 1;
            k1 = k1 - 112;
		  }

          for(k2=0; k2<16; k2++)
	      {
#ifdef DEBUG1
            printf("ii=%d k1=%d k2=%d k3=%d\n",ii,k1,k2,k3);
#endif
            if(gsegm[ii]&(1<<k2))
			{
              sl[k3][k2][k1] = 1;
#ifdef DEBUG1
              printf("---> HIT !!!!!!!!!!!!!!!!!!!!!\n");
#endif
			}
	      }

          for(k2=16; k2<32; k2++)
	      {
#ifdef DEBUG1
            printf("ii=%d k1=%d k2=%d k3=%d\n",ii,k1+1,k2-16,k3);
#endif
            if(gsegm[ii]&(1<<k2))
			{
              sl[k3][k2-16][k1+1] = 1;
#ifdef DEBUG1
              printf("---> HIT !!!!!!!!!!!!!!!!!!!!!\n");
#endif
			}
	      }

          k1+=2;
		}

        printf("\n");
        for(ii=0; ii<112; ii++)
		{
          if(!(ii%10) && ii>0 && ii<100) printf("%2d",ii);
          else if((ii%9)) printf("+");
		}
        printf("+++\n");
        for(k3=1; k3>=0; k3--) /* 2 sl */
        {
          for(k2=15; k2>=0; k2--) /* 16 angles */
          {
            for(k1=0; k1<112; k1++) /* 112 wires */
            {
              if(sl[k3][k2][k1]==0) printf(" ");
              else printf("X");
            }
            printf("\n");
  	      }
          printf("\n");
          for(ii=0; ii<112; ii++) printf("+");
          printf("\n");
        }
        printf("\n");
  }

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
