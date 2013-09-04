/******************************************************************************
*
* header file for use Linux VME defined rols with CODA
*
*                             DJA   Nov 2000
*
* SVN: $Rev: 396 $
*
*******************************************************************************/
#ifndef __TIPRIMARY_ROL__
#define __TIPRIMARY_ROL__


#include <stdio.h>

#ifndef VXWORKS
#include "../jvme/jlabgef.h"
#include "../jvme/jvme.h"
#endif

#include "../code.s/tiLib.h"
#ifdef VXWORKS
extern unsigned int tsIntCount;
extern struct TI_A24RegStruct *TIp;
extern int tiDoAck;
#endif

/* Define Hardware sources */
#define TIR_SOURCE 1
#define TS_SOURCE  2

#define TS_LEVEL 3 /*do we have it in one of the TS registers ???*/


/*??????????*/
static unsigned int *TIPRIMARYPollAddr = NULL;
static unsigned int TIPRIMARYPollMask;
static unsigned int TIPRIMARYPollValue;
static unsigned long TIPRIMARY_prescale = 1;
static unsigned long TIPRIMARY_count = 0;
/*??????????*/


/*----------------------------------------------------------------------------
  tiprimary_trigLib.c -- Dummy trigger routines for GENERAL USER based ROLs

 File : tiprimary_trigLib.h

 Routines:
	   void tiprimarytenable();        enable trigger
	   void tiprimarytdisable();       disable trigger
	   char tiprimaryttype();          return trigger type 
	   int  tiprimaryttest();          test for trigger  (POLL Routine)
------------------------------------------------------------------------------*/

static int TIPRIMARY_handlers;
static int TIPRIMARYflag;
static int TIPRIMARY_isAsync;

/*max tested value is 40*/
static int block_level = /*40*/1;

#ifdef VXWORKS

static unsigned int tdcbuftmp[100000];

void
TIPRIMARY_int_handler()
{
  /*logMsg("TIPRIMARY_int_handler reached\n",1,2,3,4,5,6);*/
  theIntHandler(TIPRIMARY_handlers);                   /* Call our handler */
  tiDoAck=0; /* Make sure the library doesn't automatically ACK */
}

#else

void
TIPRIMARY_int_handler(int arg)
{
  theIntHandler(TIPRIMARY_handlers);                   /* Call our handler */
}

#endif


static unsigned int i2_from_rol1;

static void
tiprimarytinit(int code)
{
  int i1, i2, i3;
  int overall_offset=0x80;

#ifdef VXWORKS
#else
 /* Open the default VME windows */
  vmeOpenDefaultWindows();
#endif

  /* DMA setup */
  /*usrVmeDmaSetMemSize(0x200000);*/
  usrVmeDmaInit();
#ifdef VXWORKS
  i2 = &tdcbuftmp[0];
#else
  usrVmeDmaMemory(&i1, &i2, &i3);
#endif
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
  printf("tiprimarytinit: i2_from_rol1 = 0x%08x\n",i2_from_rol1);



  /*!!! MUST CALL following somewhere ???
  vmeCloseDefaultWindows();
  */

  /* Disable IRQ for VME Bus Errors
  vmeDisableBERRIrq();
  */

  /* Initialize VME Interrupt interface - use defaults */
  tiInit(TI_ADDR,TI_READOUT,0);
  tiIntDisable();


  /*
tcpClient hps2 'tiSetFiberDelay(0xe,0xcf)'
tcpClient hps1 'tiSyncReset'
  */

#ifdef TI_SLAVE

  tiDisableTSInput(TI_TSINPUT_ALL);

/*
tiSetFiberDelay(overall_offset+0x7+4,0xcf);
*/


  tiSetFiberDelay(0x13,0xcf);

#else

  tiSetFiberDelay(overall_offset,0xcf);

#ifdef TI_MASTER
  tiAddSlave(1);
#endif

  /*TRG on front panel: external pulser*/
  /*tiSetTriggerSource(TI_TRIGGER_FPTRG);*/


  /* only 1 trigger type for physics trigger */
  tiSetTriggerSource(TI_TRIGGER_TSINPUTS);  


  tiDisableTSInput(TI_TSINPUT_ALL);

  tiEnableTSInput( TI_TSINPUT_1 );
  tiLoadTriggerTable();
  




  /*tiSetTriggerHoldoff(1,31,0);*/ /*for SVT: program frontend busy to 32 us = (1 clk = 4 ns)*/
  /*tiSetTriggerHoldoff(2,127,0);*/

  /* no SD board: BUSY from Loopback */
  /*tiSetBusySource(TI_BUSY_LOOPBACK,1);*/
  /* have SD board: BUSY from Loopback and Switch Slot B */
  /*in tiInit
  tiSetBusySource(TI_BUSY_LOOPBACK | TI_BUSY_SWB,1);
  */

  /* Loopback Sync Source (need it ???) */
  /*tiSetSyncSource(TI_SYNC_LOOPBACK);*/

#endif



  /* adjust for the fiber length: par1 - delay (4ns ticks), par2 - offset */

  tiSetBlockLevel(block_level);

  /* 0 - pipeline mode, 1 - ROC Lock mode, 2 - buffered mode */
  /*NOTE: in pipeline mode block count may goto 0 over 255, and tittest will returns
	0 and coda stops !!! be aware ... */
  tiSetBlockBufferLevel(/*0*//*1*/1);

  tiStatus();

}

/* called at prestart (CTRIGRSA) */
static void
tiprimarytriglink(int code, VOIDFUNCPTR isr)
{

  switch(code)
  {
    case TIR_SOURCE:
      /* Initialize VME Interrupt variables
      tirClearIntCount();
      */
      /* Connect User Trigger Routine */
#ifdef VXWORKS	  
      tiIntConnect(TI_INT_VEC, isr, 0);
#endif
      break;
	  /*
    case TS_SOURCE:
      if(ts)
      {
        ts->intVec = TIR_INT_VEC;
      }
      else
      {
        printf(" tiprimarytriglink: ERROR: ts uninitialized\n");
        return;
      }

#ifdef VXWORKSPPC
      if((intDisconnect(INUM_TO_IVEC(TI_INT_VEC)) !=0))
      {
        printf("Error disconnecting Interrupt\n");
      }
      intConnect(INUM_TO_IVEC(TI_INT_VEC),isr,0);
#endif

      break;
	  */

    default:
      printf(" tiprimarytriglink: ERROR: source type %d undefined\n",code);
      return;
  }

  printf("tiprimarytriglink: register int handler 0x%08x\n",isr);fflush(stdout);
}

static void 
tiprimarytenable(int val, unsigned int intMask)
{
  TIPRIMARYflag = 1;

#ifdef VXWORKS
  tiIntEnable(val);
#else
  tiEnableTriggerSource();
#endif

}

static void 
tiprimarytdisable(int val, unsigned int intMask)
{

#ifndef TI_SLAVE
    tiDisableTriggerSource(1);
    tiBlockStatus(0,1);
#endif

  tiStatus();


  /* clear all buffers here !!! */


  tiIntDisable();
#ifdef VXWORKS
  tiIntDisconnect();
#endif

  TIPRIMARYflag = 0;
}

static void 
tiprimarytack(int code, unsigned int intMask)
{
  /*if(code == TIR_SOURCE)*/
  {
    /*printf("TI_PRIMARY: call tiIntAck()\n");*/
    tiIntAck();
  }
  /*
  if(code == TS_SOURCE)
  {
    ts->lrocBufStatus = 0x100;
  }
  */
}

static unsigned int
tiprimaryttype(unsigned int code)
{
  return(1);
}

/* for polling mode only */
static int 
tiprimaryttest(unsigned int code)
{
  int val=0;

  if(code == TIR_SOURCE)
  {
    val = tiBReady(); /*see tiIntPoll(),tiGetIntCount(),tiBReady()*/
    if(val)
    {
	  /*printf("tiprimaryttest: val=%d\n",val);*/
      return(1);
    }
    else
    {
      return(0);
    }
  }

}



/* Define CODA readout list specific Macro routines/definitions */

#define TIPRIMARY_TEST  tiprimaryttest

#ifdef VXWORKS
#define TIPRIMARY_INIT {TIPRIMARY_handlers=0;TIPRIMARY_isAsync=0; TIPRIMARYflag=0; tiprimarytinit(0);}
#else
#define TIPRIMARY_INIT(code) {TIPRIMARY_handlers=0;TIPRIMARY_isAsync=0; TIPRIMARYflag=0; tiprimarytinit(code);}
#endif

#define TIPRIMARY_ASYNC(code)  {TIPRIMARY_handlers = 1; TIPRIMARY_isAsync = 1; tiprimarytriglink(code,TIPRIMARY_int_handler);}

#define TIPRIMARY_SYNC(code)   {TIPRIMARY_handlers = 1; TIPRIMARY_isAsync = 0;}

#define TIPRIMARY_SETA(code) TIPRIMARYflag = code;

#define TIPRIMARY_SETS(code) TIPRIMARYflag = code;

#define TIPRIMARY_ENA(code,val) tiprimarytenable(code,val);

#define TIPRIMARY_DIS(code,val) tiprimarytdisable(code,val);

#define TIPRIMARY_ACK(code,val) tiprimarytack(code,val);

#define TIPRIMARY_CLRS(code) TIPRIMARYflag = 0;

#define TIPRIMARY_GETID(code) TIPRIMARY_handlers

#define TIPRIMARY_TTYPE tiprimaryttype

#define TIPRIMARY_START(val)	 {;}

#define TIPRIMARY_STOP(val)	 {tiprimarytdisable(val);}

#define TIPRIMARY_ENCODE(code) (code)


#endif

