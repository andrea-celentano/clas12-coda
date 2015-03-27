
/* rol1.c - UNIX first readout list (polling mode) */

#define DMA_TO_BIGBUF /*if want to dma directly to the big buffers*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <sys/types.h>
#ifndef VXWORKS
#include <sys/time.h>
#endif

#include "circbuf.h"


/*****************************/
/* former 'crl' control keys */

/* readout list ROL1 */
#define ROL_NAME__ "DPM"

/* polling */
#define POLLING_MODE


/* name used by loader */
#define INIT_NAME dpm__init

#include "rol.h"

void usrtrig(unsigned long, unsigned long);
void usrtrig_done();

/* test readout */
#include "DPM_source.h"

/************************/
/************************/

#include "coda.h"
#include "tt.h"

static char rcname[5];


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


/*
long decrement;
extern long nevent;
*/

static void
__download()
{

#ifdef POLLING_MODE
  rol->poll = 1;
#else
  rol->poll = 0;
#endif

  printf("\n>>>>>>>>>>>>>>> ROCID=%d, CLASSID=%d <<<<<<<<<<<<<<<<\n\n",rol->pid,rol->classid);

  /* Clear some global variables etc for a clean start */
  CTRIGINIT;

  /* init trig source TEST */
  DPM_INIT;
  /*CDOINIT(TIPRIMARY,TIR_SOURCE);*/

   // Open DMA
   myfd = open("/dev/axi_stream_dma_0",O_RDWR);
   if ( myfd < 0 ) printf("Failed to open DMA device\n");
   else printf ("Opened DMA device\n");

   // Memory map
   int mapFd = open("/dev/mem", O_RDWR | O_SYNC);
   if (mapFd == -1) printf ("Memory map failed\n");
   else {

      // Map memory space 
      map = mmap(0, MEM_MAP_SIZE, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_SHARED, mapFd, MEM_MAP_BASE);
      if (map == (void *) -1) {
         printf ("Memory map failed\n");
         close(mapFd);
      }
   }

  printf("INFO: User Download 1 Executed\n");

  return;
}


static void
__prestart()
{
  unsigned long jj, adc_id, sl;
  char *env;
  char buffer[1024*1024];

  *(rol->nevents) = 0;

  /* Register a sync trigger source */
#ifdef POLLING_MODE
  CTRIGRSS(DPM, 1, usrtrig, usrtrig_done);
#else
  CTRIGRSA(DPM, 1, usrtrig, usrtrig_done);
#endif

  /* not needed here ??? */
  rol->poll = 1;

  pollCount = 0;
  lastTime  = 0;
  trigCount = 0;
  lastCount = 0;
  lastSize  = 0;
  ackTrig   = 0;

  sprintf(rcname,"RC%02d",rol->pid);
  printf("rcname >%4.4s<\n",rcname);
  map[0] = 1;      // Trig enable


  // CONFIG
  uint hitsPerApv    = 1;
  uint numApv        = 5;
  uint numHybrids    = 3;
  uint bytesPerEvent = 16;
  uint bufferCnt     = 5;

  // Size
  //map[1] = (hitsPerApv * numApv * numHybrids * bytesPerEvent * bufferCnt)/4; // size

  int count = 0;
  while ( read(myfd,buffer,1024*1024) > 0 ) count++;
  printf("Flushed %i buffers\n",count);

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

  CDODISABLE(DPM,1,0);

  printf("INFO: User End 1 Executed\n");

  return;
}

static void
__pause()
{
  CDODISABLE(DPM,1,0);

  printf("INFO: User Pause 1 Executed\n");

  return;
}

static void
__go()
{
  char *env;

  printf("INFO: User Go 1 Enabling\n");
  CDOENABLE(DPM,1,1);
  printf("INFO: User Go 1 Enabled\n");

  printf("INFO: User Go 1 Executed\n");

  printf("Go done! enable=%i\n",map[0]);

  pollCount = 0;
  lastTime  = 0;

  // Enable data

  return;
}

void
usrtrig(unsigned long EVTYPE, unsigned long EVSOURCE)
{
  int len, ii;
  TIMERL_VAR;

  rol->dabufp = (long *) 0;

TIMERL_START;

  //CEOPEN(EVTYPE, BT_BANKS);
  CEOPEN(EVTYPE, BT_UI4);

   // Invalid buffer
   if(rol->dabufp == NULL) printf("ERROR rol->dabufp is NULL -- Event lost\n");
   else {
      int ret = read(myfd,rol->dabufp,1024*1024);
      if ( ret > 0 ) rol->dabufp += ret/4;
      lastSize = ret;
   }

  CECLOSE;

TIMERL_STOP(10000,1000+rol->pid);

  ackTrig = 1;
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

  if (ackTrig) {
     map[2] = 1; // Ack
     ackTrig =  0;
  }

  /* from parser */
  poolEmpty = 0; /* global Done, Buffers have been freed */

  return;
}
  
static void
__status()
{
  return;
}  


