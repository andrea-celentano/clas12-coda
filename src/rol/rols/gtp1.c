
/* gtp1.c - UNIX first readout list (polling mode) */

#ifndef VXWORKS

#define DMA_TO_BIGBUF /*if want to dma directly to the big buffers*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/types.h>
#ifndef VXWORKS
#include <sys/time.h>
#endif

#include "circbuf.h"
#include "gtpLib.h"


/*****************************/
/* former 'crl' control keys */

/* readout list GTP1 */
#define ROL_NAME__ "GTP1"

/* polling */
#define POLLING_MODE


/* name used by loader */
#define INIT_NAME gtp1__init

#include "rol.h"

void usrtrig(unsigned long, unsigned long);
void usrtrig_done();

/* gtp readout */
#include "GTP_source.h"


/************************/
/************************/
/*
#include "coda.h"
#include "tt.h"
*/
static char rcname[5];
static int block_level = 1;


#define ABS(x)      ((x) < 0 ? -(x) : (x))

#define TIMERL_VAR \
  static hrtime_t startTim, stopTim, dTim; \
  static int nTim; \
  static hrtime_t Tim, rmsTim, minTim=10000000, maxTim, normTim=1

#define TIMERL_START \
{ \
  startTim = gethrtime(); \
}

#define TIMERL_STOP(whentoprint_macros,histid_macros) \
{ \
  stopTim = gethrtime(); \
  if(stopTim > startTim) \
  { \
    nTim ++; \
    dTim = stopTim - startTim; \
    /*if(histid_macros >= 0)   \
    { \
      uthfill(histi, histid_macros, (int)(dTim/normTim), 0, 1); \
    }*/														\
    Tim += dTim; \
    rmsTim += dTim*dTim; \
    minTim = minTim < dTim ? minTim : dTim; \
    maxTim = maxTim > dTim ? maxTim : dTim; \
    /*logMsg("good: %d %ud %ud -> %d\n",nTim,startTim,stopTim,Tim,5,6);*/ \
    if(nTim == whentoprint_macros) \
    { \
      logMsg("timer: %7llu microsec (min=%7llu max=%7llu rms**2=%7llu)\n", \
                Tim/nTim/normTim,minTim/normTim,maxTim/normTim, \
                ABS(rmsTim/nTim-Tim*Tim/nTim/nTim)/normTim/normTim,5,6); \
      nTim = Tim = 0; \
    } \
  } \
  else \
  { \
    /*logMsg("bad:  %d %ud %ud -> %d\n",nTim,startTim,stopTim,Tim,5,6);*/ \
  } \
}


static void
__download()
{

#ifdef POLLING_MODE
  rol->poll = 1;
#else
  rol->poll = 0;
#endif

  printf("\n>>>>>>>>>>>>>>> ROCID=%d, CLASSID=%d <<<<<<<<<<<<<<<<\n",rol->pid,rol->classid);
  printf("CONFFILE >%s<\n\n",rol->confFile);

  /* Clear some global variables etc for a clean start */
  CTRIGINIT;

  /* init trig source GTP */
  CDOINIT(GTP, 1);

  printf("INFO: User Download 1 Executed\n");

  return;
}


static void
__prestart()
{
  unsigned long jj, adc_id, sl;
  char *env;

  *(rol->nevents) = 0;

#ifdef POLLING_MODE
  /* Register a sync trigger source (polling mode)) */
  CTRIGRSS(GTP, 1, usrtrig, usrtrig_done);
  rol->poll = 1; /* not needed here ??? */
#else
  /* Register a async trigger source (interrupt mode) */
  CTRIGRSA(GTP, 1, usrtrig, usrtrig_done);
  rol->poll = 0; /* not needed here ??? */
#endif

  sprintf(rcname,"RC%02d",rol->pid);
  printf("rcname >%4.4s<\n",rcname);

  /* set clock source to VXS */
  gtpSetClock(1);

  /* reset the link to TI */
  gtpTiGtpLinkReset();

  /* set sync source to VXS */
  gtpSetSync(GTP_SD_SRC_SEL_SYNC);
  /* enable payloads 3-16 for HPS running with 14 FADCs, missing 2 in middle */
  gtpPayloadTriggerReset(0xFFFC);
  gtpPayloadTriggerEnable(0xFFFC);

  gtpFiberTriggerReset();
  gtpFiberTriggerEnable();

  GTP_READ_CONF_FILE;

  printf("INFO: User Prestart 1 executed\n");

  /* from parser (do we need that in rol2 ???) */
  *(rol->nevents) = 0;
  rol->recNb = 0;

  return;
}

static void
__end()
{
  int ii, total_count, rem_count;

  CDODISABLE(GTP,1,0);

  printf("INFO: User End 1 Executed\n");

  return;
}

static void
__pause()
{
  CDODISABLE(GTP,1,0);

  printf("INFO: User Pause 1 Executed\n");

  return;
}

static void
__go()
{
  char *env;

  gtpTiGtpFifoReset();
  gtpEnableInt(1);

  block_level = gtpGetBlockLevel();
  printf("block_level is %d\n",block_level);

  printf("INFO: User Go 1 Enabling\n");
  CDOENABLE(GTP,1,1);
  printf("INFO: User Go 1 Enabled\n");

  return;
}




#define MAXBUFSIZE 4000

void
usrtrig(unsigned long EVTYPE, unsigned long EVSOURCE)
{
  int len, ii, nbytes, nwords;
  char *chptr, *chptr0;
  unsigned int buf[MAXBUFSIZE];
  TIMERL_VAR;

  /*
  usleep(1000);
  */
  if(syncFlag) printf("EVTYPE=%d syncFlag=%d\n",EVTYPE,syncFlag);

  rol->dabufp = (long *) 0;

TIMERL_START;



  CEOPEN(EVTYPE, BT_BANKS);

#if 1
  len = gtpReadBlock(buf, MAXBUFSIZE, 0);

  /* gtp produce the same format as TI, so give a bank the same tag as TI */
  BANKOPEN(0xe10A,1,rol->pid);
  /*printf("\nlen=%d\n",len);*/
  for(ii=0; ii<len; ii++)
  {
    /*printf("ti[%2d] = 0x%08x\n",ii,buf[ii]);*/
    *rol->dabufp++ = buf[ii];
  }
  BANKCLOSE;
#endif


TIMERL_STOP(10000,1000+rol->pid);



  /* read boards configurations */
  if(syncFlag==1 || EVENT_NUMBER==1)
  {
    BANKOPEN(0xe10E,3,rol->pid);
    chptr = chptr0 =(char *)rol->dabufp;
    nbytes = 0;

    /* add one 'return' to make evio2xml output nicer */
    *chptr++ = '\n';
    nbytes ++;

/*vmeBusLock();*/
    len = gtpUploadAll(chptr, 10000);
/*vmeBusUnlock();*/
    /*printf("len=%d\n",len);
    printf(">%s<\n",chptr);*/
    chptr += len;
    nbytes += len;

    /* 'nbytes' does not includes end_of_string ! */
    chptr[0] = '\n';
    chptr[1] = '\n';
    chptr[2] = '\n';
    chptr[3] = '\n';
    nbytes = (((nbytes+1)+3)/4)*4;
    chptr0[nbytes-1] = '\0';

    nwords = nbytes/4;
    rol->dabufp += nwords;

    BANKCLOSE;
  }











  CECLOSE;


  return;

}

void
usrtrig_done()
{
  return;
}  

void
__done()
{
  /* from parser */
  poolEmpty = 0; /* global Done, Buffers have been freed */

  /*printf("__done reached\n");*/


  /* Acknowledge tir register */
  CDOACK(GTP,1,1);

  return;
}
  
static void
__status()
{
  return;
}  

#endif
