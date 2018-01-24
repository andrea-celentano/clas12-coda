
/* vtp1.c - first readout list for VTP boards (polling mode) */


#ifdef Linux_armv7l

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
#include "vtpLib.h"

/*****************************/
/* former 'crl' control keys */

/* readout list VTP1 */
#define ROL_NAME__ "VTP1"

/* polling */
#define POLLING_MODE


/* name used by loader */
#define INIT_NAME vtp1__init

#include "rol.h"

void usrtrig(unsigned long, unsigned long);
void usrtrig_done();

/* vtp readout */
#include "VTP_source.h"


/************************/
/************************/

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

/* for compatibility */
int
getTdcTypes(int *typebyslot)
{
  return(0);
}
int
getTdcSlotNumbers(int *slotnumbers)
{
  return(0);
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

  /* init trig source VTP */
  CDOINIT(VTP, 1);

  /************/
  /* init daq */

  daqInit();
  DAQ_READ_CONF_FILE;

  /* user code */

  printf("INFO: User Download 1 Executed\n");

  return;
}


static void
__prestart()
{
  int i;
  unsigned long jj, adc_id, sl;
  char *env;

  *(rol->nevents) = 0;

#ifdef POLLING_MODE
  /* Register a sync trigger source (polling mode)) */
  CTRIGRSS(VTP, 1, usrtrig, usrtrig_done);
  rol->poll = 1; /* not needed here ??? */
#else
  /* Register a async trigger source (interrupt mode) */
  CTRIGRSA(VTP, 1, usrtrig, usrtrig_done);
  rol->poll = 0; /* not needed here ??? */
#endif

  sprintf(rcname,"RC%02d",rol->pid);
  printf("rcname >%4.4s<\n",rcname);

  printf("calling VTP_READ_CONF_FILE ..\n");fflush(stdout);

  VTP_READ_CONF_FILE;
  
  vtpSerdesCheckLinks();

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

  CDODISABLE(VTP,1,0);

  printf("INFO: User End 1 Executed\n");

  return;
}

static void
__pause()
{
  CDODISABLE(VTP,1,0);

  printf("INFO: User Pause 1 Executed\n");

  return;
}

static void
__go()
{
  int i;
  char *env;

//  vtpSerdesStatusAll();
  block_level = vtpTiLinkGetBlockLevel(0);
  printf("Setting VTP block level to: %d\n", block_level);
  vtpSetBlockLevel(block_level);


  printf("INFO: User Go 1 Enabling\n");
  CDOENABLE(VTP,1,1);
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
  len = vtpEbTiReadEvent(buf, MAXBUFSIZE);
  /*for(ii=0; ii<len; ii++) printf("vtpti[%2d] = 0x%08x\n",ii,buf[ii]);*/
  BANKOPEN(0xe10A,1,rol->pid);
  for(ii=0; ii<len; ii++)
  {
    /*printf("vtpti[%2d] = 0x%08x\n",ii,buf[ii]);*/
    *rol->dabufp++ = buf[ii];
  }
  BANKCLOSE;
#endif

#if 1
  len = vtpEbReadEvent(buf, MAXBUFSIZE);
  /*for(ii=0; ii<len; ii++) printf("vtp[%2d] = 0x%08x\n",ii,buf[ii]);*/
  BANKOPEN(0xe122,1,rol->pid);
  for(ii=0; ii<len; ii++)
  {
    /*printf("vtp[%2d] = 0x%08x\n",ii,buf[ii]);*/
    *rol->dabufp++ = buf[ii];
  }
  BANKCLOSE;
#endif

TIMERL_STOP(10000,1000+rol->pid);


#if 0
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
#endif










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


  /* Acknowledge TI */
  CDOACK(VTP,1,1);

  return;
}
  
static void
__status()
{
  return;
}  



#else

void
vtp1_dummy()
{
  return;
}

#endif

