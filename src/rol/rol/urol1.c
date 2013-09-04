
/* urol1.c - UNIX first readout list (polling mode) */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifndef VXWORKS
#include <sys/types.h>
#include <time.h>
#endif

#include "circbuf.h"


/*****************************/
/* former 'crl' control keys */

/* readout list UROL1 */
#define ROL_NAME__ "UROL1"

/* polling */
#define POLLING_MODE


/* name used by loader */
#define INIT_NAME urol1__init

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

static void
__download()
{

#ifdef POLLING_MODE
  rol->poll = 1;
#else
  rol->poll = 0;
#endif


  printf(">>>>>>>>>>>>>>>>>>>>>>>>>> ROCID = %d <<<<<<<<<<<<<<<<\n",rol->pid);

  printf("rol1: downloading DDL table ...\n");
  clonbanks_();
  printf("rol1: ... done.\n");


  printf("INFO: User Download 1 Executed\n");

  return;
}


static void
__prestart()
{
  unsigned long jj, adc_id, sl;
  char *env;

  /* Clear some global variables etc for a clean start */
  CTRIGINIT;
  *(rol->nevents) = 0;

  /* init trig source TEST */
  TEST_INIT; /*CDOINIT ??*/

  /* Register a sync trigger source */
#ifdef POLLING_MODE
  CTRIGRSS(TEST, 1, usrtrig, usrtrig_done);
#else
  CTRIGRSA(TEST, 1, usrtrig, usrtrig_done);
#endif

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

  CDOENABLE(TEST,1,1);

  printf("INFO: User Go 1 Executed\n");

  return;
}

void
usrtrig(unsigned long EVTYPE, unsigned long EVSOURCE)
{
  long EVENT_LENGTH;
  int ii, len, len1, type, lock_key, *tmp;
  int *adrlen, *bufin, *bufout, i, ind, ind2, ncol, nrow, len2;
  unsigned long *secondword, *jw, *buf, *dabufp1, *dabufp2;

  /*
sleep(1);
  */

  rol->dabufp = (long *) 0;

  CEOPEN(EVTYPE, BT_BANKS);

  jw = rol->dabufp;
  jw[-2] = 1;
  secondword = rol->dabufp - 1; /* pointer to the second CODA header word */




    /* open data bank */
    if((ind = bosMopen_(jw, rcname, 0, 1, 0)) <=0)
    {
      printf("bosMopen_ Error: %d\n",ind);
    }
    rol->dabufp += NHEAD;



    len = 20;
    for(ii=0; ii<len; ii++)
    {
      *rol->dabufp++ = ii;
    }


    if(bosMclose_(jw,ind,1,len) == 0)
    {
      printf("ERROR in bosMclose_ - space is not enough !!!\n");
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
