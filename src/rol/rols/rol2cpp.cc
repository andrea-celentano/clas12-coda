
/* rol2cpp.cc - default second readout list */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

#ifndef VXWORKS
#include <sys/types.h>
#include <time.h>
#endif

#include "circbuf.h"

#define ROL_NAME__ "ROL2CPP"
#define INIT_NAME rol2cpp__init

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

#define USE_GEM

#ifdef USE_GEM

#include "../../gem/src.s/zero_suppression.h"
#include "../../gem/src.s/PRDMapping.h"
#include "../../gem/src.s/GEMZeroSup.h"

/* in following: [FEC#][ADC#][CHANNEL#] */ 
static int buf_offset[8][16][128];
static int buf_noise[8][16][128];


//const char *pedestal_file = "/usr/clas12/release/1.3.0/coda/src/gem/run/pedestal.txt";
const char *pedestal_file = "/home/clasrun/PRad/Convert_Pedestal/pedestal.txt";
static PRDMapping* mapping;

#endif







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
  int i;
  printf("INFO: Entering Prestart ROL2CPP\n");

  /* Clear some global variables etc for a clean start */
  CTRIGINIT;

  /* init trig source EVENT */
  EVENT_INIT;

  /* Register a sync trigger source (up to 32 sources) */
  CTRIGRSS(EVENT, 1, rol2trig, rol2trig_done); /* second arg=1 - what is that ? */

  rol->poll = 1;

  rol->recNb = 0;


#ifdef USE_GEM
  printf("GEM Intitialization ..\n");

  mapping = PRDMapping::GetInstance();
  mapping->LoadMapping("/usr/clas12/release/1.3.0/coda/src/gem/run/mapping.cfg");
  //mapping->LoadMapping("/usr/clas12/release/1.3.0/coda/src/gem/run/mappingzeroxinzhan.cfg");
  mapping->SaveMapping("/tmp/saved_mapping.cfg");

  i = load_pedestal(pedestal_file, buf_noise, buf_offset, 72);

  printf("GEM Intitialization done\n");
#endif


  printf("INFO: Prestart ROL2CPP executed\n");

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
  BANKINIT;
  int nASIS, iASIS[MAXBANKS];
  int nhits, ii, jj, kk;
  
  mynev ++; /* needed by ttfa.c */

  /*printf("befor bankscan\n");fflush(stdout);*/
  BANKSCAN;
  /*printf("after bankscan\n");fflush(stdout);*/

  /*
    printf("\n\n\n\n\n nbanks=%d\n",nbanks);
    for(jj=0; jj<nbanks; jj++) printf("bankscan[%d]: tag 0x%08x typ=%d nr=%d nw=%d dataptr=0x%08x\n",
    jj,banktag[jj],banktyp[jj],banknr[jj],banknw[jj],bankdata[jj]);fflush(stdout);
  */
  nASIS = 0;
  for(jj=0; jj<nbanks; jj++)
    {
      datain = bankdata[jj];
      lenin = banknw[jj];
#ifndef VXWORKS
#ifndef NIOS
      /* swap input buffer (assume that data from VME is big-endian, and we are on little-endian Intel) */
      if(banktyp[jj] != 3) for(ii=0; ii<lenin; ii++) datain[ii] = /*LSWAP*/(datain[ii]);
#endif
#endif
      iASIS[nASIS++] = jj; /* remember bank number as it reported by BANKSCAN */
    }


  for(ii=0; ii<nASIS; ii++)
    {
      jj = iASIS[ii]; /* bank number as it reported by BANKSCAN */
      datain = bankdata[jj];
      lenin = banknw[jj];
      /*printf("mynev=%d: coping bank number %d (header %d 0x%08x))\n",mynev,jj,*(datain-2),*(datain-1));*/

      /*PRESCALE IT FOR REAL RUN  !!!*/
      if( banktag[jj]<9 || banktag[jj]>13)
      //if(banktag[jj]!=57631)
	{
	  CPOPEN(banktag[jj],banktyp[jj],banknr[jj]);
	  for(kk=0; kk<lenin; kk++)
	    {
	      dataout[kk] = datain[kk];
	    }
	  b08 += (lenin<<2);
	  CPCLOSE;
	}
#ifdef USE_GEM
      if( banktag[jj]>=9 && banktag[jj]<=13) banktag[jj]=57631; // xb test
      if(banktag[jj]==57631)
	{
          
	  /*printf("GEM: bank number=%d nw=%d\n",banknr[jj],lenin);fflush(stdout);*/
	  printf("GEM: bank number=%d nw=%d\n",banknr[jj],lenin);fflush(stdout);
	  CPOPEN(banktag[jj],banktyp[jj],banknr[jj]+100);

#if 1
	  nhits = zero_suppression((uint32_t *)datain, lenin, (uint32_t *)dataout, buf_offset, buf_noise);
	  b08 += (nhits<<2);
	  /*printf(" GEM: nhits=%d\n",nhits);fflush(stdout);*/
	  printf(" GEM: nhits=%d\n", nhits); fflush(stdout);
#endif

#if 0
	  for(kk=0; kk<lenin; kk++)
	    {
	      dataout[kk] = datain[kk];
	    }

	  b08 += (lenin<<2);
#endif

	  CPCLOSE;
	}
      //----------------------------------------------------------
      // Save un zero suppressed data
      banktag[jj]=57632; // xb test: to save un zero suppressed data
      if(banktag[jj]==57632)
	{

	  /*printf("GEM: bank number=%d nw=%d\n",banknr[jj],lenin);fflush(stdout);*/
	  printf("GEM: bank number=%d nw=%d\n",banknr[jj],lenin);fflush(stdout);
	  CPOPEN(banktag[jj],banktyp[jj],banknr[jj]+100);

#if 0
	  nhits = zero_suppression((uint32_t *)datain, lenin, (uint32_t *)dataout, buf_offset, buf_noise);
	  b08 += (nhits<<2);
	  /*printf(" GEM: nhits=%d\n",nhits);fflush(stdout);*/
	  printf(" GEM: nhits=%d\n", nhits); fflush(stdout);
#endif

#if 1
	  for(kk=0; kk<lenin; kk++)
	    {
	      dataout[kk] = datain[kk];
	    }

	  b08 += (lenin<<2);
#endif

	  CPCLOSE;
	}
      //---------------------------------------------------------------
#endif



    }


  rol->user_storage[0] = lenout;
  CPEXIT;
  rol->user_storage[1] = 1;

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
