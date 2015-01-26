
/* rol1.c - UNIX first readout list (polling mode) */

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


/*****************************/
/* former 'crl' control keys */

/* readout list ROL1 */
#define ROL_NAME__ "ROL1"

/* polling */
#define POLLING_MODE


/* name used by loader */
#define INIT_NAME rol1__init

#include "rol.h"

void usrtrig(unsigned long, unsigned long);
void usrtrig_done();

/* test readout */
#include "TEST_source.h"

/************************/
/************************/

#include "coda.h"
#include "tt.h"

static char rcname[5];

/*
long decrement;
extern long nevent;
*/

/* for compatibility with hps1.c */
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

  printf("\n>>>>>>>>>>>>>>> ROCID=%d, CLASSID=%d <<<<<<<<<<<<<<<<\n\n",rol->pid,rol->classid);

  /* Clear some global variables etc for a clean start */
  CTRIGINIT;

  /* init trig source TEST */
  TEST_INIT;
  /*CDOINIT(TIPRIMARY,TIR_SOURCE);*/

  printf("INFO: User Download 1 Executed\n");

  /*
  UDP_user_request(MSGINF,"rol1","information");
  UDP_user_request(MSGWRN,"rol1","warnings");
  UDP_user_request(MSGERR,"rol1","errors");
  */

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
  CTRIGRSS(TEST, 1, usrtrig, usrtrig_done);
#else
  CTRIGRSA(TEST, 1, usrtrig, usrtrig_done);
#endif

  /* not needed here ??? */
  rol->poll = 1;


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

  CDODISABLE(TEST,1,0);

  printf("INFO: User End 1 Executed\n");

  return;
}

static void
__pause()
{
  CDODISABLE(TEST,1,0);

  printf("INFO: User Pause 1 Executed\n");

  return;
}

static void
__go()
{
  char *env;

  printf("INFO: User Go 1 Enabling\n");
  CDOENABLE(TEST,1,1);
  printf("INFO: User Go 1 Enabled\n");

  printf("INFO: User Go 1 Executed\n");

  return;
}

void
usrtrig(unsigned long EVTYPE, unsigned long EVSOURCE)
{
  int len, ii;

  
usleep(1000);
  

  rol->dabufp = (long *) 0;

  CEOPEN(EVTYPE, BT_BANKS);

  BANKOPEN(0xe1FF,1,rol->pid);
  len = 20;
  for(ii=0; ii<len; ii++)
  {
    *rol->dabufp++ = ii;
  }
  BANKCLOSE;

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
