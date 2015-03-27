
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
#define ROL_NAME__ "DTM"

/* polling */
#define POLLING_MODE


/* name used by loader */
#define INIT_NAME dtm__init

#include "rol.h"

void usrtrig(unsigned long, unsigned long);
void usrtrig_done();

/* test readout */
#include "DTM_source.h"

/************************/
/************************/

#include "coda.h"
#include "tt.h"

static char rcname[5];

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
  DTM_INIT;
  /*CDOINIT(TIPRIMARY,TIR_SOURCE);*/

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

  printf("INFO: User Download 1 Executed.\n");

  return;
}


static void
__prestart()
{
  unsigned long jj, adc_id, sl;
  char *env;

  *(rol->nevents) = 0;

  /* Register a sync trigger source */
#ifdef POLLING_MODE
  CTRIGRSS(DTM, 1, usrtrig, usrtrig_done);
  rol->poll = 1;
#else
  CTRIGRSA(DTM, 1, usrtrig, usrtrig_done);
  rol->poll = 0;
#endif


   pollCount = 0;
   trigCount = 0;
   lastTime  = 0;
   lastCount = 0;

   map[0x178/4] = 0x55; //syncLoad
   map[0x0C/4] = 0x2; //syncSrcEn
   map[0x10/4] = 1; //trgSyncOutEn
   map[0x1C/4] = 1; //boardActive 

   map[0x18/4] = 0; //rocEn
   map[0x68/4] = 0; //trgSrcEn
   map[0x28/4] = 1; //trgSrcSet
   map[0x28/4] = 0; //trgSrcSet

   map[0x34/4] = 0; //running 



  sprintf(rcname,"RC%02d",rol->pid);
  printf("rcname >%4.4s<\n",rcname);

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

  CDODISABLE(DTM,1,0);

  printf("INFO: User End 1 Executed\n");

	// In Stop
   map[0x18/4] = 0; //rocEn
   map[0x68/4] = 0; //trgSrcEn 
   map[0x28/4] = 1; //trgSrcSet
   map[0x28/4] = 0; //trgSrcSet

   map[0x34/4] = 0; //running 

  return;
}

static void
__pause()
{
  CDODISABLE(DTM,1,0);

  printf("INFO: User Pause 1 Executed\n");

  return;
}

static void
__go()
{
  char *env;

  printf("INFO: User Go 1 Executed\n");

  pollCount = 0;
  trigCount = 0;
  lastTime  = 0;

  // In GO
  map[0x68/4] = 0x2; //trgSrcEn
  map[0x28/4] = 1; //trgSrcSet
  map[0x28/4] = 0; //trgSrcSet
  map[0x18/4] = map[0x98/4]; //rocEn = dpmMask after go

  map[0x38/4] = 1; //count reset
  map[0x38/4] = 0; //count reset

  map[0x34/4] = 1; //running 


  printf("INFO: User Go 1 Enabling\n");
  CDOENABLE(DTM,1,1);
  printf("INFO: User Go 1 Enabled\n");

  return;
}

void
usrtrig(unsigned long EVTYPE, unsigned long EVSOURCE)
{
  int len, ii;

  rol->dabufp = (long *) 0;

  //CEOPEN(EVTYPE, BT_BANKS);
  CEOPEN(EVTYPE, BT_UI4);

  len = 10;
  for(ii=0; ii<len; ii++)
  {
    *rol->dabufp++ = ii;
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

  return;
}
  
static void
__status()
{
  return;
}  

