/******************************************************************************
*
* header file for use Linux VME defined rols with CODA
*
*                             DJA   Nov 2000
*
* SVN: $Rev: 396 $
*
*******************************************************************************/
#ifndef __HPSTRACKPRIMARY_ROL__
#define __HPSTRACKPRIMARY_ROL__

#include <time.h>
#include <linux/poll.h>

#include "../hps/daq/generic/ControlCmdMem.h"
#include "../hps/daq/tracker/OnlineDataMem.h"
#include "../hps/daq/driver/HpPciMod.h"

static int HPSTRACKPRIMARY_handlers,HPSTRACKPRIMARYflag;
static int HPSTRACKPRIMARY_isAsync;
static unsigned int *HPSTRACKPRIMARYPollAddr = NULL;
static unsigned int HPSTRACKPRIMARYPollMask;
static unsigned int HPSTRACKPRIMARYPollValue;
static unsigned long HPSTRACKPRIMARY_prescale = 1;
static unsigned long HPSTRACKPRIMARY_count = 0;

static ControlCmdMemory *cmem;
static OnlineDataMem    *dmem;
static int              myfd;
static unsigned int     tiData[16];
static unsigned int     trigType;
static unsigned int     enable;
static unsigned int     pollCount;
static time_t           pollTime;
static unsigned int     errorCnt;
static unsigned int     perTotal;
static unsigned int     perSize;
static unsigned int     total;
static unsigned int     lastSize;
static time_t           perTime;
static time_t           currTime;
static time_t           totTime;
static unsigned int     cardMask;
static unsigned int     bufferDepth;
static unsigned int     intTrig;
static unsigned int     liveDisplay;


/*----------------------------------------------------------------------------
  hpstrackprimary_trigLib.c -- Dummy trigger routines for GENERAL USER based ROLs

 File : hpstrackprimary_trigLib.h

 Routines:
	   void hpstrackprimarytenable();        enable trigger
	   void hpstrackprimarytdisable();       disable trigger
	   char hpstrackprimaryttype();          return trigger type 
	   int  hpstrackprimaryttest();          test for trigger  (POLL Routine)
------------------------------------------------------------------------------*/

static void 
hpstrackprimarytenable(int val)
{
  HPSTRACKPRIMARYflag = 1;
}

static void 
hpstrackprimarytdisable(int val)
{
  HPSTRACKPRIMARYflag = 0;
}

static unsigned long 
hpstrackprimaryttype()
{
  return(trigType);
}

static int 
hpstrackprimaryttest()
{
   HpPciRx hpRx;
   int     ret;
   time_t  currTime;

   // The following is needed to prevent coda from freezing up
   if ( total == 0 ) sleep(1);

   // Setup descriptor
   hpRx.model      = sizeof(void *);
   hpRx.data       = tiData;
   hpRx.rxMax      = 16;
   hpRx.debugLevel = 0;
   hpRx.rxLane     = 7;

   // Attempt to read TI frame
   ret = read(myfd,&hpRx,sizeof(hpRx));

   if ( ret == 10 ) {
      if ( intTrig ) trigType  = 1;
      else trigType = 1 /*tiData[5] & 0xFFFF*/;
      pollCount = 0;
      return(enable);
   }
   else {

      if ( ret > 0 ) printf("Poll: Bad TI size returned %i\n",ret);

      time(&currTime);
      if ( pollCount++ > 10 ) {
         if ( (currTime - pollTime) > 60 ) {
            printf("Poll: No Data In 60 Seconds! Total=%i, PollCount=%i, Time: %s",total,pollCount,ctime(&currTime));
            pollTime = currTime;
            if ( errorCnt > 0 ) {
               printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
               printf("!!!!!!!!!! An ERROR HAS OCCURED !!!!!!!!!!!!!!!!\n");
               printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            }
         }
      }
      else pollTime = currTime;
      return(0);
   }
}

/* Define CODA readout list specific Macro routines/definitions */

#define HPSTRACKPRIMARY_TEST  hpstrackprimaryttest

#define HPSTRACKPRIMARY_INIT { HPSTRACKPRIMARY_handlers =0;HPSTRACKPRIMARY_isAsync = 0;HPSTRACKPRIMARYflag = 0;}

#define HPSTRACKPRIMARY_ASYNC(code)  {HPSTRACKPRIMARY_handlers = 1; HPSTRACKPRIMARY_isAsync = 0;}

#define HPSTRACKPRIMARY_SYNC(code)   {HPSTRACKPRIMARY_handlers = 1; HPSTRACKPRIMARY_isAsync = 0;}

#define HPSTRACKPRIMARY_SETA(code) HPSTRACKPRIMARYflag = code;

#define HPSTRACKPRIMARY_SETS(code) HPSTRACKPRIMARYflag = code;

#define HPSTRACKPRIMARY_ENA(code,val) hpstrackprimarytenable(val);

#define HPSTRACKPRIMARY_DIS(code,val) hpstrackprimarytdisable(val);

/*#define HPSTRACKPRIMARY_ACK(code,val) hpstrackprimarytack(code,val);*/

#define HPSTRACKPRIMARY_CLRS(code) HPSTRACKPRIMARYflag = 0;

#define HPSTRACKPRIMARY_GETID(code) HPSTRACKPRIMARY_handlers

#define HPSTRACKPRIMARY_TTYPE hpstrackprimaryttype

#define HPSTRACKPRIMARY_START(val)	 {;}

#define HPSTRACKPRIMARY_STOP(val)	 {hpstrackprimarytdisable(val);}

#define HPSTRACKPRIMARY_ENCODE(code) (code)


#endif

