/******************************************************************************
*
* header file for use Linux VME defined rols with CODA
*
*                             DJA   Nov 2000
*
* SVN: $Rev: 396 $
*
*******************************************************************************/
#ifndef __TIDPRIMARY_ROL__
#define __TIDPRIMARY_ROL__


#include <stdio.h>

#include "../jvme/jlabgef.h"
#include "../jvme/jvme.h"
#include "../code.s/tidLib.h"

/* Define Hardware sources */
#define TIR_SOURCE 1
#define TS_SOURCE  2

#define TS_LEVEL 3 /*do we have it in one of the TS registers ???*/


/*??????????*/
static unsigned int *TIDPRIMARYPollAddr = NULL;
static unsigned int TIDPRIMARYPollMask;
static unsigned int TIDPRIMARYPollValue;
static unsigned long TIDPRIMARY_prescale = 1;
static unsigned long TIDPRIMARY_count = 0;
/*??????????*/


/*----------------------------------------------------------------------------
  tidprimary_trigLib.c -- Dummy trigger routines for GENERAL USER based ROLs

 File : tidprimary_trigLib.h

 Routines:
	   void tidprimarytenable();        enable trigger
	   void tidprimarytdisable();       disable trigger
	   char tidprimaryttype();          return trigger type 
	   int  tidprimaryttest();          test for trigger  (POLL Routine)
------------------------------------------------------------------------------*/

static int TIDPRIMARY_handlers;
static int TIDPRIMARYflag;
static int TIDPRIMARY_isAsync;

/*max tested value is 40*/
static int block_level = /*40*/1;

#ifdef VXWORKS

void
TIDPRIMARY_int_handler()
{
  /*logMsg("TIDPRIMARY_int_handler reached\n",1,2,3,4,5,6);*/
  theIntHandler(TIDPRIMARY_handlers);                   /* Call our handler */
}

#else

void
TIDPRIMARY_int_handler(int arg)
{
  theIntHandler(TIDPRIMARY_handlers);                   /* Call our handler */
}

#endif


static unsigned int i2_from_rol1;

static void
tidprimarytinit(int code)
{
  int i1, i2, i3;
  int overall_offset=0x80;

 /* Open the default VME windows */
  vmeOpenDefaultWindows();

  /* DMA setup */
  /*usrVmeDmaSetMemSize(0x200000);*/
  usrVmeDmaInit();
  usrVmeDmaMemory(&i1, &i2, &i3);

  i2_from_rol1 = i2;
  if( (i2_from_rol1&7) == 0xc )
  {
    i2_from_rol1 += 1;
  }
  else if( (i2_from_rol1&7) == 0x8 )
  {
    i2_from_rol1 += 2;
  }
  else if( (i2_from_rol1&7) == 0x4 )
  {
    i2_from_rol1 += 3;
  }
  printf("tidprimarytinit: i2_from_rol1 = 0x%08x\n",i2_from_rol1);




  /*!!! MUST CALL following somewhere ???
  vmeCloseDefaultWindows();
  */

  /* Disable IRQ for VME Bus Errors
  vmeDisableBERRIrq();
  */

  /* Initialize VME Interrupt interface - use defaults */
  tidInit(TID_ADDR,TID_READOUT,0);
  tidIntDisable();


  /*
#ifdef TID_MASTER
  tidAddSlave(5);
  tidAddSlave(6);
#endif
  */

  /*
tcpClient hps2 'tidSetFiberDelay(0xe,0xcf)'
tcpClient hps1 'tidSyncReset'
  */

#ifdef TID_SLAVE

  tidDisableTSInput(TID_TSINPUT_ALL);

  tidSetFiberDelay(overall_offset+0x7+4,0xcf);
  /*tidSetFiberDelay(0x13,0xcf);*/

#else

  /*TRG on front panel: external pulser*/
  /*tidSetTriggerSource(TID_TRIGGER_FPTRG);*/


  /* only 1 trigger type for physics trigger */
  tidSetTriggerSource(TID_TRIGGER_TSINPUTS);  

tidSetFiberDelay(overall_offset,0xcf);

  tidDisableTSInput(TID_TSINPUT_ALL);

  tidEnableTSInput( TID_TSINPUT_1 );
  tidLoadTriggerTable();
  




  tidSetTriggerHoldoff(1,31); /*for SVT: program frontend busy to 32 us = (1 clk = 4 ns)*/
  /*tidSetTriggerHoldoff(2,127);*/

  /* no SD board: BUSY from Loopback */
  /*tidSetBusySource(TID_BUSY_LOOPBACK,1);*/
  /* have SD board: BUSY from Loopback and Switch Slot B */
  /*in tidInit
  tidSetBusySource(TID_BUSY_LOOPBACK | TID_BUSY_SWB,1);
  */

  /* Loopback Sync Source (need it ???) */
  /*tidSetSyncSource(TID_SYNC_LOOPBACK);*/

#endif



  /* adjust for the fiber length: par1 - delay (4ns ticks), par2 - offset */

  tidSetBlockLevel(block_level);

  /* 0 - pipeline mode, 1 - ROC Lock mode, 2 - buffered mode */
  /*NOTE: in pipeline mode block count may goto 0 over 255, and tidttest will returns
	0 and coda stops !!! be aware ... */
  tidSetBlockAckThreshold(/*0*//*1*/7);

  tidStatus();

}

/* called at prestart (CTRIGRSA) */
static void
tidprimarytriglink(int code, VOIDFUNCPTR isr)
{

  switch(code)
  {
    case TIR_SOURCE:
      /* Initialize VME Interrupt variables
      tirClearIntCount();
      */
      /* Connect User Trigger Routine */
	  /*
      tidIntConnect(TID_INT_VEC, isr, 0);
	  */
      break;
	  /*
    case TS_SOURCE:
      if(ts)
      {
        ts->intVec = TIR_INT_VEC;
      }
      else
      {
        printf(" vmetriglink: ERROR: ts uninitialized\n");
        return;
      }
	  */
#ifdef VXWORKSPPC
      if((intDisconnect(INUM_TO_IVEC(TIR_INT_VEC)) !=0))
      {
        printf("Error disconnecting Interrupt\n");
      }
      intConnect(INUM_TO_IVEC(TIR_INT_VEC),isr,0);
#endif

      break;

    default:
      printf(" vmetriglink: ERROR: source type %d undefined\n",code);
      return;
  }

  printf("vmetriglink: register int handler 0x%08x\n",isr);fflush(stdout);
}

static void 
tidprimarytenable(int val, unsigned int intMask)
{
  TIDPRIMARYflag = 1;

  tidEnableTriggerSource();

  /*
  tidIntEnable(val);
  */
}

static void 
tidprimarytdisable(int val, unsigned int intMask)
{

#ifndef TID_SLAVE
    tidDisableTriggerSource(1);
    tidBlockStatus(0,1);
#endif

  tidStatus();


  /* clear all buffers here !!! */


  tidIntDisable();

  /*
  tidIntDisconnect();
  */
  TIDPRIMARYflag = 0;
}

static void 
tidprimarytack(int code, unsigned int intMask)
{
  /*if(code == TIR_SOURCE)*/
  {
    /*printf("TID_PRIMARY: call tidIntAck()\n");*/
    tidIntAck();
  }
  /*
  if(code == TS_SOURCE)
  {
    ts->lrocBufStatus = 0x100;
  }
  */
}

static unsigned int
tidprimaryttype(unsigned int code)
{
  return(1);
}

/* for polling mode only */
static int 
tidprimaryttest(unsigned int code)
{
  int val=0;

  if(code == TIR_SOURCE)
  {
    val = tidBReady(); /*see tidIntPoll(),tidGetIntCount(),tidBReady()*/
    if(val)
    {
	  /*printf("tidprimaryttest: val=%d\n",val);*/
      return(1);
    }
    else
    {
      return(0);
    }
  }

}



/* Define CODA readout list specific Macro routines/definitions */

#define TIDPRIMARY_TEST  tidprimaryttest

#define TIDPRIMARY_INIT(code) {TIDPRIMARY_handlers=0;TIDPRIMARY_isAsync=0; TIDPRIMARYflag=0; tidprimarytinit(code);}

#define TIDPRIMARY_ASYNC(code)  {TIDPRIMARY_handlers = 1; TIDPRIMARY_isAsync = 1; vmetriglink(code,TIDPRIMARY_int_handler);}

#define TIDPRIMARY_SYNC(code)   {TIDPRIMARY_handlers = 1; TIDPRIMARY_isAsync = 0;}

#define TIDPRIMARY_SETA(code) TIDPRIMARYflag = code;

#define TIDPRIMARY_SETS(code) TIDPRIMARYflag = code;

#define TIDPRIMARY_ENA(code,val) tidprimarytenable(code,val);

#define TIDPRIMARY_DIS(code,val) tidprimarytdisable(code,val);

#define TIDPRIMARY_ACK(code,val) tidprimarytack(code,val);

#define TIDPRIMARY_CLRS(code) TIDPRIMARYflag = 0;

#define TIDPRIMARY_GETID(code) TIDPRIMARY_handlers

#define TIDPRIMARY_TTYPE tidprimaryttype

#define TIDPRIMARY_START(val)	 {;}

#define TIDPRIMARY_STOP(val)	 {tidprimarytdisable(val);}

#define TIDPRIMARY_ENCODE(code) (code)


#endif

