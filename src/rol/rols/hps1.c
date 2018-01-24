
/* hps1

net_thread:  waiting=    280    sending=      0 microsec per event (nev=728)
setHeartError: 0 >sys 0, mask 21<
WARN: HeartBeat[0]: heartbeat=1426780246(1426780246) heartmask=21
UDP_cancel: cancel >inf:hps1 sys 0, mask 21<
UDP_cancel: cancel >inf:hps1 sys 0, mask 21<
wait: request in progress
*/

/* hps1.c */

#if defined(VXWORKS) || defined(Linux_vme)

#define NEW

/* HPS firmware:
Board Firmware Rev/ID = 0x02c1 : ADC Processing Rev = 0x0a03

TI init:
tcpClient adcecal1 'tiInit(0xa80000,3,0)'
coda_roc_gef -s clasprod -o "adcecal1 ROC" -i
*/


/*#define SSIPC*/

static int nusertrig, ndone;


#undef DMA_TO_BIGBUF /*if want to dma directly to the big buffers*/

/* hps1.c - first readout list for VXS crates with FADC250 and new TI */

#define USE_FADC250
#define USE_DSC2
#define USE_V1190
#define USE_SSP
/*#define USE_VSCM*/
/*#define USE_DC*/
/*#define USE_DCRB*/
#define USE_VETROC

/* if event rate goes higher then 10kHz, with random triggers we have wrong
slot number reported in GLOBAL HEADER and/or GLOBAL TRAILER words; to work
around that problem temporary patches were applied - until fixed (Sergey) */
#define SLOTWORKAROUND

#undef DEBUG


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>

#ifndef VXWORKS
#include <sys/time.h>
/*
typedef      long long       hrtime_t;
*/
#endif

#ifdef SSIPC
#include <rtworks/ipc.h>
#include "epicsutil.h"
static char ssname[80];
#endif

#include "circbuf.h"

/* from fputil.h */
#define SYNC_FLAG 0x20000000

/* readout list name */
#define ROL_NAME__ "HPS1"

/* polling mode if needed */
#define POLLING_MODE

/* main TI board */
#define TI_ADDR   (21<<19)  /* if 0 - default will be used, assuming slot 21*/



/* name used by loader */

#ifdef TI_MASTER
#define INIT_NAME hps1_master__init
#define TI_READOUT TI_READOUT_EXT_POLL /* Poll for available data, front panel triggers */
#else
#ifdef TI_SLAVE
#define INIT_NAME hps1_slave__init
#define TI_READOUT TI_READOUT_TS_POLL /* Poll for available data, triggers from master over fiber */
#else
#define INIT_NAME hps1__init
#define TI_READOUT TI_READOUT_EXT_POLL /* Poll for available data, front panel triggers */
#endif
#endif

#include "rol.h"

void usrtrig(unsigned int EVTYPE, unsigned int EVSOURCE);
void usrtrig_done();

#define USE_HPS
#include "TIPRIMARY_source.h"



/* user code */

/*
#include "uthbook.h"
#include "coda.h"
#include "tt.h"
*/
#include "scaler7201.h"
#include "c792Lib.h"
#include "tdc1190.h"
#include "vscmLib.h"

#ifdef USE_SSP
#include "sspLib.h"
static int nssp;   /* Number of SSPs found with sspInit(..) */
#endif

static char rcname[5];

#define NBOARDS 22    /* maximum number of VME boards: we have 21 boards, but numbering starts from 1 */
#define MY_MAX_EVENT_LENGTH 3000/*3200*/ /* max words per board */
static unsigned int *tdcbuf;

/*#ifdef DMA_TO_BIGBUF*/
/* must be 'rol' members, like dabufp */
extern unsigned int dabufp_usermembase;
extern unsigned int dabufp_physmembase;
/*#endif*/

extern int rocMask; /* defined in roc_component.c */

#define NTICKS 1000 /* the number of ticks per second */
/*temporary here: for time profiling */




#ifndef VXWORKS

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

#endif



void
tsleep(int n)
{
#ifdef VXWORKS
  taskDelay ((sysClkRateGet() / NTICKS) * n);
#else
#endif
}



extern struct TI_A24RegStruct *TIp;

void
titest1()
{
  if(TIp==NULL) {printf("NULL\n");return;}
  printf("0x%08x(%d) 0x%08x(%d)\n",
		 vmeRead32(&TIp->fiberLatencyMeasurement),vmeRead32(&TIp->fiberLatencyMeasurement),
		 vmeRead32(&TIp->fiberAlignment),vmeRead32(&TIp->fiberAlignment));
}



/*
#ifdef USE_V1190
*/
static int tdctypebyslot[NBOARDS];
static int error_flag[NBOARDS];
static int ndsc2=0, ndsc2_daq=0;
static int ntdcs;


int
getTdcTypes(int *typebyslot)
{
  int jj;
  for(jj=0; jj<NBOARDS; jj++) typebyslot[jj] = tdctypebyslot[jj];
  return(ntdcs);
}



#ifdef SLOTWORKAROUND
static int slotnums[NBOARDS];
int
getTdcSlotNumbers(int *slotnumbers)
{
  int jj;
  for(jj=0; jj<NBOARDS; jj++) slotnumbers[jj] = slotnums[jj];
  return(ntdcs);
}
#endif

/*
#endif
*/

#ifdef USE_VETROC
#include "vetrocLib.h"
extern int nvetroc;                   /* Number of VETROCs in Crate */
static int VETROC_SLOT;
static int VETROC_ROFLAG = 1;           /* 0-noDMA, 1-board-by-board DMA, 2-chainedDMA */

/* for the calculation of maximum data words in the block transfer */
static unsigned int MAXVETROCWORDS = 10000;
static unsigned int vetrocSlotMask; /* bit=slot (starting from 0) */
#endif

#ifdef USE_DCRB
#include "dcrbLib.h"
extern int ndcrb;                     /* Number of DCs in Crate */
static int DCRB_SLOT;
static int DCRB_ROFLAG = 1;           /* 0-noDMA, 1-board-by-board DMA, 2-chainedDMA */

/* for the calculation of maximum data words in the block transfer */
static unsigned int MAXDCRBWORDS = 10000;
static unsigned int dcrbSlotMask; /* bit=slot (starting from 0) */
#endif

#ifdef USE_DC
#include "dcLib.h"
extern int ndc;                     /* Number of DCs in Crate */
static int DC_SLOT;
static int DC_ROFLAG = 1;           /* 0-noDMA, 1-board-by-board DMA, 2-chainedDMA */

/* for the calculation of maximum data words in the block transfer */
static unsigned int MAXDCWORDS = 10000;
static unsigned int dcSlotMask; /* bit=slot (starting from 0) */
#endif

#ifdef USE_SSP
static unsigned int sspSlotMask = 0; /* bit=slot (starting from 0) */
static int SSP_SLOT;
#endif

#ifdef USE_VSCM
static unsigned int vscmSlotMask = 0; /* bit=slot (starting from 0) */
static int nvscm1 = 0;
static int VSCM_ROFLAG = 1;
#endif

#ifdef USE_FADC250

#include "fadcLib.h"

#define DIST_ADDR  0xEA00	  /*  base address of FADC signal distribution board  (A16)  */


unsigned int fadcSlotMask   = 0;    /* bit=slot (starting from 0) */
static int nfadc;                 /* Number of FADC250s verified with the library */
static int NFADC;                   /* The Maximum number of tries the library will
                                     * use before giving up finding FADC250s */
static int FA_SLOT;                 /* We'll use this over and over again to provide
				                     * us access to the current FADC slot number */ 

static int FADC_ROFLAG           = 2;  /* 0-noDMA, 1-board-by-board DMA, 2-chainedDMA */

/* for the calculation of maximum data words in the block transfer */
static unsigned int MAXFADCWORDS = 0;
static unsigned int MAXTIWORDS  = 0;


/* IC lookup tables */

/*tdc's in slots 19 and 20*/
int ic_tdc_high[2][128] = {
0*256
};

int ic_tdc_low[2][128] = {
0*256
};



/*adc's in slots 3-10 and 13-18*/

char *
getFadcPedsFilename(int rocid)
{
  char *dir = NULL;
  char *expid = NULL;
  static char fname[1024];

  if((dir=getenv("CLAS")) == NULL)
  {
    printf("ERROR: environment variable CLAS is not defined - exit\n");
    return(NULL);
  }
  if((expid=getenv("EXPID")) == NULL)
  {
    printf("ERROR: environment variable EXPID is not defined - exit\n");
    return(NULL);
  }
  sprintf(fname,"%s/parms/peds/%s/fadc%02d.ped",dir,expid,rocid);

  return fname;
}

#endif

static void
__download()
{
  int i1, i2, i3;
#ifdef USE_FADC250

  int ii, id, isl, ichan, slot;
  unsigned short iflag;
  int fadc_mode = 1, iFlag = 0;
  int ich, NSA, NSB;
  unsigned int maxA32Address;
  unsigned int fadcA32Address = 0x09000000;

#endif
#ifdef POLLING_MODE
  rol->poll = 1;
#else
  rol->poll = 0;
#endif

  printf("\n>>>>>>>>>>>>>>> ROCID=%d, CLASSID=%d <<<<<<<<<<<<<<<<\n",rol->pid,rol->classid);
  printf("CONFFILE >%s<\n\n",rol->confFile);
  printf("LAST COMPILED: %s %s\n", __DATE__, __TIME__);

  /*
  printf("rol1: downloading DDL table ...\n");
  clonbanks_();
  printf("rol1: ... done.\n");
  */

  /**/
  CTRIGINIT;

  /* initialize OS windows and TI board */
#ifdef VXWORKS
  CDOINIT(TIPRIMARY);
#else
  CDOINIT(TIPRIMARY,TIR_SOURCE);
#endif


  /************/
  /* init daq */

  daqInit();
  DAQ_READ_CONF_FILE;


  /*************************************/
  /* redefine TI settings if neseccary */

#ifndef TI_SLAVE
  /* TS 1-6 create physics trigger, no sync event pin, no trigger 2 */
vmeBusLock();
  tiLoadTriggerTable(3);
  tiSetTriggerWindow(7);	// (7+1)*4ns trigger it coincidence time to form trigger type
vmeBusUnlock();
#endif


  /*********************************************************/
  /*********************************************************/



  /* set wide pulse */
vmeBusLock();
/*sergey: WAS tiSetSyncDelayWidth(1,127,1);*/
/*worked for bit pattern latch tiSetSyncDelayWidth(0x54,127,1);*/
vmeBusUnlock();

  usrVmeDmaSetConfig(2,5,1); /*A32,2eSST,267MB/s*/
  /*usrVmeDmaSetConfig(2,5,0);*/ /*A32,2eSST,160MB/s*/
  /*usrVmeDmaSetConfig(2,3,0);*/ /*A32,MBLT*/



  /*
if(rol->pid==18)
{
  usrVmeDmaSetConfig(2,3,0);
}
  */


  /*
  usrVmeDmaSetChannel(1);
  printf("===== Use DMA Channel %d\n\n\n",usrVmeDmaGetChannel());
  */

  tdcbuf = (unsigned int *)i2_from_rol1;



  /******************/
  /* USER code here */


#ifdef USE_DSC2
  printf("DSC2 Download() starts =========================\n");

#ifndef VXWORKS
  vmeSetQuietFlag(1); /* skip the errors associated with BUS Errors */
#endif
vmeBusLock();
  dsc2Init(0x100000,0x80000,16,0);
vmeBusUnlock();
#ifndef VXWORKS
  vmeSetQuietFlag(0); /* Turn the error statements back on */
#endif

  ndsc2 = dsc2GetNdsc();
  if(ndsc2>0)
  {
    DSC2_READ_CONF_FILE;
    maxA32Address = dsc2GetA32MaxAddress();
    fadcA32Address = maxA32Address + FA_MAX_A32_MEM;
    ndsc2_daq = dsc2GetNdsc_daq();
  }
  else
  {
    ndsc2_daq = 0;
  }
  printf("dsc2: %d boards set to be readout by daq\n",ndsc2_daq);
  printf("DSC2 Download() ends =========================\n\n");
#endif



#ifdef USE_FADC250
  printf("FADC250 Download() starts =========================\n");

  /* Here, we assume that the addresses of each board were set according
   to their geographical address (slot number): on FADC250 it must be set by jumpers,
   while some other boards (TI, DSC2 etc) can set it automatically if jumpers are set to 0

   * Slot  2:  (2<<19) = 0x00100000

   * Slot  3:  (3<<19) = 0x00180000
   * Slot  4:  (4<<19) = 0x00200000
   * Slot  5:  (5<<19) = 0x00280000
   * Slot  6:  (6<<19) = 0x00300000
   * Slot  7:  (7<<19) = 0x00380000
   * Slot  8:  (8<<19) = 0x00400000
   * Slot  9:  (9<<19) = 0x00480000
   * Slot 10: (10<<19) = 0x00500000

   * Slot 11: (11<<19) = 0x00580000
   * Slot 12: (12<<19) = 0x00600000

   * Slot 13: (13<<19) = 0x00680000
   * Slot 14: (14<<19) = 0x00700000
   * Slot 15: (15<<19) = 0x00780000
   * Slot 16: (16<<19) = 0x00800000
   * Slot 17: (17<<19) = 0x00880000
   * Slot 18: (18<<19) = 0x00900000
   * Slot 19: (19<<19) = 0x00980000
   * Slot 20: (20<<19) = 0x00A00000

   * Slot 21: (21<<19) = 0x00A80000

   */

  NFADC = 16 + 2; /* 16 slots + 2 (for the switch slots) */

  /* NOTE: starting from 'fadcA32Base' address, memory chunks size=FA_MAX_A32_MEM(=0x800000)
							will be used for every board in A32Blk space:
adc      A32BLK address
1        0x09000000
2        0x09800000
3        0x0A000000
4        0x0A800000
5        0x0B000000
6        0x0B800000
7        0x0C000000
8        0x0C800000
9        0x0D000000
10       0x0D800000
11       0x0E000000
12       0x0E800000
13       0x0F000000
14       0x0F800000
15       0x10000000
16       0x10800000

DSC2: the same as FADCs

CAEN BOARDS IN A32 SPACE MUST BE USING ADDRESSES FROM 0x11000000 AND ABOVE !!!

v1495: 0x11xx0000, where xx follows the same scheme as FADCs
v1190: 0x11xx0000, where xx follows the same scheme as FADCs

*/

  /* Setup the iFlag.. flags for FADC initialization */
  iFlag = 0;
  /* base address */
  iFlag = (DIST_ADDR)<<10;
  /* Sync Source */
  iFlag |= (1<<0);    /* VXS */
  /* Trigger Source */
  iFlag |= (1<<2);    /* VXS */
  /* Clock Source */
  /*iFlag |= (1<<5);*/    /* VXS */
  iFlag |= (0<<5);  /* Internal Clock Source */
#ifndef VXWORKS
  vmeSetQuietFlag(1); /* skip the errors associated with BUS Errors */
#endif

  faSetA32BaseAddress(fadcA32Address);
vmeBusLock();
  faInit((unsigned int)(3<<19),(1<<19),NFADC,iFlag); /* start from 0x00180000, increment 0x00080000 */
vmeBusUnlock();

  faGetMinA32MB(0);
  faGetMaxA32MB(0);

  nfadc = faGetNfadc(); /* acual number of FADC boards found  */
#ifndef VXWORKS
  vmeSetQuietFlag(0); /* Turn the error statements back on */
#endif

  if(nfadc>0)
  {
    if(nfadc==1) FADC_ROFLAG = 1; /*no chainedDMA if one board only*/

    if(FADC_ROFLAG==2) faEnableMultiBlock(1);
    else faDisableMultiBlock();

    /* configure all modules based on config file */
    FADC_READ_CONF_FILE;

    /* Additional Configuration for each module */
    fadcSlotMask=0;
    for(id=0; id<nfadc; id++) 
    {
      FA_SLOT = faSlot(id);      /* Grab the current module's slot number */
      fadcSlotMask |= (1<<FA_SLOT); /* Add it to the mask */
      printf("=======================> fadcSlotMask=0x%08x",fadcSlotMask);

	  {
        unsigned int PL, PTW, NSB, NSA, NP;
vmeBusLock();
        faGetProcMode(FA_SLOT, &fadc_mode, &PL, &PTW, &NSB, &NSA, &NP);
vmeBusUnlock();
        printf(", slot %d, fadc_mode=%d\n",FA_SLOT,fadc_mode);
	  }

      /* Bus errors to terminate block transfers (preferred) */
vmeBusLock();
      faEnableBusError(FA_SLOT);
vmeBusUnlock();

#ifdef NEW
      /*****************/
      /*trigger-related*/
vmeBusLock();
      faResetMGT(FA_SLOT,1);
      /* Enable FADC "live" trigger to go to P0 (to SD) and front panel */
      faSetTrigOut(FA_SLOT, 7);
vmeBusUnlock();
#endif

	  /*****************/
	  /*****************/
    }


    /* 1) Load FADC pedestals from file for trigger path.
       2) Offset FADC threshold for each channel based on pedestal for both readout and trigger */
	if(rol->pid>36 && rol->pid!=46 && rol->pid!=37 && rol->pid!=39 && rol->pid!=58 &&
           rol->pid!=70 && rol->pid!=71 && rol->pid!=72 /*adcft1, adcft2, adcft3 */
          )
    {
vmeBusLock();
      faGLoadChannelPedestals(getFadcPedsFilename(rol->pid), 1);
vmeBusUnlock();
    }
    /* read back and print trigger pedestals */
/*
    printf("\n\nTrigger pedestals readback\n");
    for(id=0; id<nfadc; id++) 
    {
      FA_SLOT = faSlot(id);
      for(ichan=0; ichan<16; ichan++)
      {
        printf("  slot=%2d chan=%2d ped=%5d\n",FA_SLOT,ichan,faGetChannelPedestal(FA_SLOT, ichan));
      }
    }
    printf("\n\n");
*/
  }


/*
STATUS for FADC in slot 18 at VME (Local) base address 0x900000 (0xa16b1000)
---------------------------------------------------------------------- 
 Board Firmware Rev/ID = 0x020e : ADC Processing Rev = 0x0907
 Alternate VME Addressing: Multiblock Enabled
   A32 Enabled at VME (Local) base 0x0f800000 (0xa95b1000)
   Multiblock VME Address Range 0x10800000 - 0x11000000

 Signal Sources: 
   Ref Clock : Internal
   Trig Src  : VXS (Async)
   Sync Reset: VXS (Async)

 Configuration: 
   Internal Clock ON
   Bus Error ENABLED
   MultiBlock transfer ENABLED (Last Board  - token via VXS)

 ADC Processing Configuration: 
   Channel Disable Mask = 0x0000
   Mode = 1  (ENABLED)
   Lookback (PL)    = 1360 ns   Time Window (PTW) = 400 ns
   Time Before Peak = 12 ns   Time After Peak   = 24 ns
   Max Peak Count   = 1 
   Playback Mode    = 0 

  CSR       Register = 0x00001800
  Control 1 Register = 0x10b00338 
  Control 2 Register = 0x00000000 - Disabled
  Internal Triggers (Live) = 0
  Trigger   Scaler         = 0
  Events in FIFO           = 0  (Block level = 1)
  MGT Status Register      = 0x00000400 
  BERR count (from module) = 0
*/

  /***************************************
   *   SD SETUP
   ***************************************/
vmeBusLock();
  sdInit(1);   /* Initialize the SD library */
  sdSetActiveVmeSlots(fadcSlotMask); /* Use the fadcSlotMask to configure the SD */
  sdSetTrigoutLogic(0, 1); /* SD Trigout when VSCM multiplicity >= 1. Need to move this to TI (or SD) config file */
  sdStatus();
vmeBusUnlock();

  printf("FADC250 Download() ends =========================\n\n");
#endif


#ifdef USE_V1190
  printf("V1190 Download() starts =========================\n");

vmeBusLock();
  ntdcs = tdc1190Init(0x11100000,0x80000,20,0);
vmeBusUnlock();
  if(ntdcs>0) TDC_READ_CONF_FILE;

  for(ii=0; ii<ntdcs; ii++)
  {
    slot = tdc1190Slot(ii);
    tdctypebyslot[slot] = tdc1190Type(ii);
    printf(">>> id=%d slot=%d type=%d\n",ii,slot,tdctypebyslot[slot]);
  }


#ifdef SLOTWORKAROUND
  for(ii=0; ii<ntdcs; ii++)
  {
vmeBusLock();
    slot = tdc1190GetGeoAddress(ii);
vmeBusUnlock();
	slotnums[ii] = slot;
    printf("[%d] slot %d\n",ii,slotnums[ii]);
  }
#endif


  /* if TDCs are present, set busy from P2 */
  if(ntdcs>0)
  {
    printf("Set BUSY from P2 for TDCs\n");
vmeBusLock();
    tiSetBusySource(TI_BUSY_P2,0);
vmeBusUnlock();
  }

  for(ii=0; ii<ntdcs; ii++)
  {
vmeBusLock();
    tdc1190Clear(ii);
vmeBusUnlock();
    error_flag[ii] = 0;
  }

  printf("V1190 Download() ends =========================\n\n");
#endif


#ifdef USE_VSCM
  printf("VSCM Download() starts =========================\n");
#ifndef VXWORKS
  vmeSetQuietFlag(1); /* skip the errors associated with BUS Errors */
#endif
vmeBusLock();
  nvscm1 = vscmInit((unsigned int)(3<<19),(1<<19),20,0);
vmeBusUnlock();
#ifndef VXWORKS
  vmeSetQuietFlag(0); /* Turn the error statements back on */
#endif

  if(VSCM_ROFLAG==2 && nvscm1==1) VSCM_ROFLAG = 1; /*no chainedDMA if one board only*/
  /*
  if(VSCM_ROFLAG==2) faEnableMultiBlock(1);
  else faDisableMultiBlock();
  */

  vscmSlotMask=0;
  for(ii=0; ii<nvscm1; ii++)
  {
    slot = vscmSlot(ii);      /* Grab the current module's slot number */
    vscmSlotMask |= (1<<slot); /* Add it to the mask */
  }

  printf("VSCM Download() ends =========================\n\n");
#endif




#ifdef USE_DC
  printf("DC Download() starts =========================\n");

#ifndef VXWORKS
  vmeSetQuietFlag(1); /* skip the errors associated with BUS Errors */
#endif

vmeBusLock();
  ndc = dcInit((3<<19), 0x80000, 16+2, 7); /* 7 boards from slot 4, 7 boards from slot 13 */
  if(ndc>0)
  {
    dcGSetDAC(/*20*/10); /* threshold in mV */
    dcGSetCalMask(0,0x3f);
    dcGSetProcMode(4000/*2000*/,4000/*2000*/,32);
  }
vmeBusUnlock();

#ifndef VXWORKS
  vmeSetQuietFlag(0); /* Turn the error statements back on */
#endif

  if(ndc==1) DC_ROFLAG = 1; /*no chainedDMA if one board only*/
  if((ndc>0) && (DC_ROFLAG==2)) dcEnableMultiBlock(1);
  else if(ndc>0) dcDisableMultiBlock();

  /* Additional Configuration for each module */
  dcSlotMask=0;
  for(ii=0; ii<ndc; ii++) 
  {
    DC_SLOT = dcSlot(ii);      /* Grab the current module's slot number */
    dcSlotMask |= (1<<DC_SLOT); /* Add it to the mask */
	printf("=======================> dcSlotMask=0x%08x\n",dcSlotMask);

  }


  /* DC stuff */

  for(id=0; id<ndc; id++) 
  {
    DC_SLOT = dcSlot(id);
vmeBusLock();
    dcTriggerPulseWidth(DC_SLOT, 8000);
	dcLinkReset(DC_SLOT);
vmeBusUnlock();

    /* will try to reset 5 times */
    for(ii=0; ii<5; ii++)
	{
      if(dcLinkStatus(DC_SLOT)) break;
	  printf("Reseting link at slot %d\n",DC_SLOT);
vmeBusLock();
      dcLinkReset(DC_SLOT);
vmeBusUnlock();
	}
  }


  for(id=0; id<ndc; id++)
  {
    DC_SLOT = dcSlot(id);
    if(dcLinkStatus(DC_SLOT)) printf("Link at slot %d is UP\n",DC_SLOT);
    else printf("Link at slot %d is DOWN\n",DC_SLOT);
  }
  printf("DC Download() ends =========================\n\n");
#endif

  sprintf(rcname,"RC%02d",rol->pid);
  printf("rcname >%4.4s<\n",rcname);

#ifdef SSIPC
  sprintf(ssname,"%s_%s",getenv("HOST"),rcname);
  printf("Smartsockets unique name >%s<\n",ssname);
  epics_msg_sender_init(getenv("EXPID"), ssname); /* SECOND ARG MUST BE UNIQUE !!! */
#endif

  logMsg("INFO: User Download Executed\n",1,2,3,4,5,6);
}



static void
__prestart()
{
  int ii, i1, i2, i3;
  int ret;
#ifdef USE_FADC250
  int id, isl, ichan, slot;
  unsigned short iflag;
  int iFlag = 0;
  int ich;
  unsigned short aa = 0;
  unsigned short bb;
  unsigned short thr = 400;
#endif

  /* Clear some global variables etc for a clean start */
  *(rol->nevents) = 0;
  event_number = 0;

  tiEnableVXSSignals();

#ifdef POLLING_MODE
  CTRIGRSS(TIPRIMARY, TIR_SOURCE, usrtrig, usrtrig_done);
#else
  CTRIGRSA(TIPRIMARY, TIR_SOURCE, usrtrig, usrtrig_done);
#endif

  printf(">>>>>>>>>> next_block_level = %d, block_level = %d, use next_block_level\n",next_block_level,block_level);
  block_level = next_block_level;


  /**************************************************************************/
  /* setting TI busy conditions, based on boards found in Download          */
  /* tiInit() does nothing for busy, tiConfig() sets fiber, we set the rest */
  /* NOTE: if ti is busy, it will not send trigger enable over fiber, since */
  /*       it is the same fiber and busy has higher priority                */

#ifndef TI_SLAVE
vmeBusLock();
tiSetBusySource(TI_BUSY_LOOPBACK,0);
  /*tiSetBusySource(TI_BUSY_FP,0);*/
vmeBusUnlock();
#endif


#ifdef USE_FADC250
  /* if FADCs are present, set busy from SD board */
  if(nfadc>0)
  {
    printf("Set BUSY from SWB for FADCs\n");
vmeBusLock();
    tiSetBusySource(TI_BUSY_SWB,0);
vmeBusUnlock();
  }
#endif



#ifdef USE_VSCM
  printf("VSCM Prestart() start =========================\n");

  /* if VSCMs are present, set busy from SD board */
  if(nvscm1>0)
  {
    printf("Set BUSY from SWB for FADCs\n");
vmeBusLock();
    tiSetBusySource(TI_BUSY_SWB,0);
	sdSetActiveVmeSlots(vscmSlotMask);
	 
    if(rol->pid == 4) sdSetTrigoutLogic(0, 2); /* SD Trigout when VSCM multiplicity >= 2 - DO FOR SVT4: R1+R2*/
    else              sdSetTrigoutLogic(0, 1); /* SD Trigout when VSCM multiplicity >= 1 - DO FOR SVT5: R3*/
vmeBusUnlock();

    VSCM_READ_CONF_FILE;
/*
    printf("vscmPrestart ...\n"); fflush(stdout);
//   vscmPrestart("VSCMConfig_ben_cosmic.txt");
    vscmPrestart("VSCMConfig.txt");
    printf("vscmPrestart done\n"); fflush(stdout);
*/
  }

  printf("VSCM Prestart() ends =========================\n\n");
#endif



#ifdef USE_VETROC
  printf("VETROC Prestart() starts =========================\n");

#ifndef VXWORKS
  vmeSetQuietFlag(1); /* skip the errors associated with BUS Errors */
#endif

vmeBusLock();
  nvetroc = vetrocInit((3<<19), 0x80000, 16+2, 0x111); /* 7 boards from slot 4, 7 boards from slot 13 */
  if(nvetroc>0)
  {
    vetrocGSetProcMode(/*4000*/2000,/*4000*/2000);
  }
vmeBusUnlock();

#ifndef VXWORKS
  vmeSetQuietFlag(0); /* Turn the error statements back on */
#endif

  if(nvetroc==1) VETROC_ROFLAG = 1; /*no chainedDMA if one board only*/
  if((nvetroc>0) && (VETROC_ROFLAG==2)) vetrocEnableMultiBlock(1);
  else if(nvetroc>0) vetrocDisableMultiBlock();

  /* Additional Configuration for each module */
  vetrocSlotMask=0;
  for(ii=0; ii<nvetroc; ii++) 
  {
    VETROC_SLOT = vetrocSlot(ii);      /* Grab the current module's slot number */
    vetrocSlotMask |= (1<<VETROC_SLOT); /* Add it to the mask */
	printf("=======================> vetrocSlotMask=0x%08x\n",vetrocSlotMask);

  }

  /* VETROC stuff */
  for(id=0; id<nvetroc; id++) 
  {
    VETROC_SLOT = vetrocSlot(id);
vmeBusLock();
    vetrocTriggerPulseWidth(VETROC_SLOT, 8000);
    vetrocLinkReset(VETROC_SLOT);
vmeBusUnlock();
  }

  printf("VETROC Prestart() ends =========================\n\n");
#endif


#ifdef USE_DCRB
  printf("DCRB Prestart() starts =========================\n");

#ifndef VXWORKS
  vmeSetQuietFlag(1); /* skip the errors associated with BUS Errors */
#endif

vmeBusLock();
  ndcrb = dcrbInit((4<<19), 0x80000, 16+2, 7); /* 7 boards from slot 4, 7 boards from slot 13 */
  if(ndcrb>0)
  {
    dcrbGSetDAC(-30); /* threshold in mV */

    /* last param is double-hit resolution in ns */
    dcrbGSetProcMode(/*4000*/2000,/*4000*/2000,1000);
  }
vmeBusUnlock();

#ifndef VXWORKS
  vmeSetQuietFlag(0); /* Turn the error statements back on */
#endif

  if(ndcrb==1) DCRB_ROFLAG = 1; /*no chainedDMA if one board only*/
  if((ndcrb>0) && (DCRB_ROFLAG==2)) dcrbEnableMultiBlock(1);
  else if(ndcrb>0) dcrbDisableMultiBlock();

  /* Additional Configuration for each module */
  dcrbSlotMask=0;
  for(ii=0; ii<ndcrb; ii++) 
  {
    DCRB_SLOT = dcrbSlot(ii);      /* Grab the current module's slot number */
    dcrbSlotMask |= (1<<DCRB_SLOT); /* Add it to the mask */
	printf("=======================> dcrbSlotMask=0x%08x\n",dcrbSlotMask);

  }


  /* DCRB stuff */

  for(id=0; id<ndcrb; id++) 
  {
    DCRB_SLOT = dcrbSlot(id);
vmeBusLock();
//dcrbTriggerPulseWidth(DCRB_SLOT, 8000);
	dcrbLinkReset(DCRB_SLOT);
vmeBusUnlock();

    /* will try to reset 5 times
    for(ii=0; ii<5; ii++)
	{
      if(dcrbLinkStatus(DCRB_SLOT)) break;
	  printf("Reseting link at slot %d\n",DCRB_SLOT);
vmeBusLock();
      dcrbLinkReset(DCRB_SLOT);
vmeBusUnlock();
	}
    */

  }


  for(id=0; id<ndcrb; id++)
  {
    DCRB_SLOT = dcrbSlot(id);
    if(dcrbLinkStatus(DCRB_SLOT)) printf("Link at slot %d is UP\n",DCRB_SLOT);
    else printf("Link at slot %d is DOWN\n",DCRB_SLOT);
  }
  printf("DCRB Prestart() ends =========================\n\n");
#endif


  /*********************************************************/
  /* hps only, busy from trigger board and ack mask for it */
  
  /*  
  if(rol->pid==37||rol->pid==39)
  {
    tiSetBusySource(TI_BUSY_SWA,0);
    tiAddRocSWA();
  }
  else
  {
    tiRemoveRocSWA();
  }
  */


  /*
tiRemoveRocSWA();
  */




  /*****************************************************************/
  /*****************************************************************/

#ifdef USE_FADC250
  printf("FADC250 Prestart() starts =========================\n");

  /* Program/Init VME Modules Here */
  for(id=0; id<nfadc; id++)
  {
    FA_SLOT = faSlot(id);
vmeBusLock();
    faSetClockSource(FA_SLOT,2);
vmeBusUnlock();
  }

  sleep(1);

  for(id=0; id<nfadc; id++)
  {
    FA_SLOT = faSlot(id);
vmeBusLock();
    faSoftReset(FA_SLOT,0); /*0-soft reset, 1-soft clear*/
vmeBusUnlock();

#ifdef NEW
    if(!faGetMGTChannelStatus(FA_SLOT))
    {
vmeBusLock();
      faResetMGT(FA_SLOT,1);
      faResetMGT(FA_SLOT,0);
vmeBusUnlock();
    }
#endif

vmeBusLock();
    faResetToken(FA_SLOT);
    faResetTriggerCount(FA_SLOT);
    faStatus(FA_SLOT,0);
    faPrintThreshold(FA_SLOT);
vmeBusUnlock();
  }

  /*  Enable FADC */
  for(id=0; id<nfadc; id++) 
  {
    FA_SLOT = faSlot(id);
    /*faSetMGTTestMode(FA_SLOT,0);*/
    /*faChanDisable(FA_SLOT,0xffff);enabled in download*/
vmeBusLock();
    faEnable(FA_SLOT,0,0);
vmeBusUnlock();
  }

  faGStatus(0);

  printf("FADC250 Prestart() ends =========================\n\n");
  sleep(2);
#endif



#ifdef USE_SSP
  printf("SSP Prestart() starts =========================\n");

 /*****************
  *   SSP SETUP - must do sspInit() after master TI clock is stable, so do it in Prestart
  *****************/
  iFlag  = SSP_INIT_MODE_DISABLED; /* Disabled, initially */
  iFlag |= SSP_INIT_SKIP_FIRMWARE_CHECK;
  iFlag |= SSP_INIT_MODE_VXS;
  iFlag |= SSP_INIT_FIBER0_ENABLE;         /* Enable hps1gtp fiber ports */
  iFlag |= SSP_INIT_FIBER1_ENABLE;         /* Enable hps1gtp fiber ports */
  iFlag |= SSP_INIT_GTP_FIBER_ENABLE_MASK; /* Enable all fiber port data to GTP */
  /*iFlag|= SSP_INIT_NO_INIT;*/ /* does not configure SSPs, just set pointers */
  nssp=0;
vmeBusLock();
  nssp = sspInit(0, 0, 0, iFlag); /* Scan for, and initialize all SSPs in crate */
vmeBusUnlock();
  printf("hps1: found %d SSPs (using iFlag=0x%08x)\n",nssp,iFlag);


  if(nssp>0)
  {
    SSP_READ_CONF_FILE;

    sspSlotMask=0;
    for(id=0; id<nssp; id++)
    {
      SSP_SLOT = sspSlot(id);      /* Grab the current module's slot number */
      sspSlotMask |= (1<<SSP_SLOT); /* Add it to the mask */
      printf("=======================> sspSlotMask=0x%08x\n",sspSlotMask);


      printf("Setting SSP %d, slot %d\n",id,sspSlot(id));

      /* SSP Status */
      /*sspPrintHpsScalers(sspSlot(id));*/
      sspPrintHpsConfig(sspSlot(id));
    }
  }

  printf("SSP Prestart() ends =========================\n\n");
#endif /* USE_SSP */

  /* USER code here */
  /******************/

vmeBusLock();
  tiIntDisable();
vmeBusUnlock();

#ifdef USE_DSC2
  printf("DSC2 Prestart() starts =========================\n");
  /* dsc2 configuration */
  if(ndsc2>0) DSC2_READ_CONF_FILE;
  printf("DSC2 Prestart() ends =========================\n\n");
#endif

  /* master and standalone crates, NOT slave */
#ifndef TI_SLAVE

  sleep(1);
vmeBusLock();
  tiSyncReset(1);
vmeBusUnlock();
  sleep(1);
vmeBusLock();
  tiSyncReset(1);
vmeBusUnlock();
  sleep(1);

vmeBusLock();
  ret = tiGetSyncResetRequest();
vmeBusUnlock();
  if(ret)
  {
    printf("ERROR: syncrequest still ON after tiSyncReset(); trying again\n");
    sleep(1);
vmeBusLock();
    tiSyncReset(1);
vmeBusUnlock();
    sleep(1);
  }

vmeBusLock();
  ret = tiGetSyncResetRequest();
vmeBusUnlock();
  if(ret)
  {
    printf("ERROR: syncrequest still ON after tiSyncReset(); try 'tcpClient <rocname> tiSyncReset'\n");
  }
  else
  {
    printf("INFO: syncrequest is OFF now\n");
  }

  printf("holdoff rule 1 set to %d\n",tiGetTriggerHoldoff(1));
  printf("holdoff rule 2 set to %d\n",tiGetTriggerHoldoff(2));

#endif

/* set block level in all boards where it is needed;
   it will overwrite any previous block level settings */

#ifdef TI_SLAVE /* assume that for master and standalone TIs block level is set from config file */
vmeBusLock();
  tiSetBlockLevel(block_level);
vmeBusUnlock();
#endif
#ifdef USE_V1190
  for(ii=0; ii<ntdcs; ii++)
  {
vmeBusLock();
    tdc1190SetBLTEventNumber(ii, block_level);
vmeBusUnlock();
  }
#endif
#ifdef USE_FADC250
    /* Calculate the maximum number of words per block transfer (assuming Pulse mode)
     *   MAX = NFADC * block_level * (EvHeader + TrigTime*2 + Pulse*2*chan) 
     *         + 2*32 (words for byte alignment) 
     */
    MAXFADCWORDS = NFADC * block_level * (1+2+100/*FADC_WINDOW_WIDTH*/*16) + 2*32;
  
    printf("**************************************************\n");
    printf("* Calculated MAX FADC words per block = %d\n",MAXFADCWORDS);
    printf("**************************************************\n");

    /* Check these numbers, compared to our buffer size.. */
    if( (MAXFADCWORDS+MAXTIWORDS)*4 > MAX_EVENT_LENGTH )
    {
      printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
      printf(" WARNING.  Event buffer size is smaller than the expected data size\n");
      printf("     Increase the size of MAX_EVENT_LENGTH and recompile!\n");
      printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    }

    for(id=0; id<nfadc; id++) 
    {
      slot = faSlot(id);
vmeBusLock();
      faSetBlockLevel(slot, block_level);
vmeBusUnlock();
    }
#endif
#ifdef USE_VSCM
  for(ii=0; ii<nvscm1; ii++)
  {
    slot = vscmSlot(ii);
vmeBusLock();
    vscmSetBlockLevel(slot, block_level);
vmeBusUnlock();
  }
#endif
#ifdef USE_SSP
  for(id=0; id<nssp; id++)
  {
    slot = sspSlot(id);
vmeBusLock();
    sspSetBlockLevel(slot, block_level);
    sspGetBlockLevel(slot);
vmeBusUnlock();
  }
#endif

#ifdef USE_VETROC
  for(id=0; id<nvetroc; id++)
  {
    slot = vetrocSlot(id);
vmeBusLock();
    vetrocSetBlockLevel(slot, block_level);
/*    vetrocGetBlockLevel(slot);*/
vmeBusUnlock();
  }
#endif

#ifdef USE_DC
  for(id=0; id<ndc; id++)
  {
    slot = dcSlot(id);
vmeBusLock();
    dcSetBlockLevel(slot, block_level);
/*    dcGetBlockLevel(slot);*/
vmeBusUnlock();
  }
#endif

#ifdef USE_DCRB
  for(id=0; id<ndcrb; id++)
  {
    slot = dcrbSlot(id);
vmeBusLock();
    dcrbSetBlockLevel(slot, block_level);
/*    dcrbGetBlockLevel(slot);*/
vmeBusUnlock();
  }
#endif

vmeBusLock();
  tiStatus(1);
vmeBusUnlock();

  printf("INFO: Prestart1 Executed\n");fflush(stdout);

  *(rol->nevents) = 0;
  rol->recNb = 0;

  return;
}       

static void
__end()
{
  int iwait=0;
  int blocksLeft=0;
  int id;

  printf("\n\nINFO: End1 Reached\n");fflush(stdout);

  CDODISABLE(TIPRIMARY,TIR_SOURCE,0);

  /* Before disconnecting... wait for blocks to be emptied */
vmeBusLock();
  blocksLeft = tiBReady();
vmeBusUnlock();
  printf(">>>>>>>>>>>>>>>>>>>>>>> %d blocks left on the TI\n",blocksLeft);fflush(stdout);
  if(blocksLeft)
  {
    printf(">>>>>>>>>>>>>>>>>>>>>>> before while ... %d blocks left on the TI\n",blocksLeft);fflush(stdout);
    while(iwait < 10)
	{
      taskDelay(10);
	  if(blocksLeft <= 0) break;
vmeBusLock();
	  blocksLeft = tiBReady();
      printf(">>>>>>>>>>>>>>>>>>>>>>> inside while ... %d blocks left on the TI\n",blocksLeft);fflush(stdout);
vmeBusUnlock();
	  iwait++;
	}
    printf(">>>>>>>>>>>>>>>>>>>>>>> after while ... %d blocks left on the TI\n",blocksLeft);fflush(stdout);
  }


#ifdef USE_FADC250
  /* FADC Disable */
  for(id=0; id<nfadc; id++) 
  {
    FA_SLOT = faSlot(id);
vmeBusLock();
    faDisable(FA_SLOT,0);
    faStatus(FA_SLOT,0);
vmeBusUnlock();
  }

vmeBusLock();
  sdStatus();
vmeBusUnlock();
#endif

vmeBusLock();
  tiStatus(1);
vmeBusUnlock();

  printf("INFO: End1 Executed\n\n\n");fflush(stdout);

  return;
}


static void
__pause()
{
  CDODISABLE(TIPRIMARY,TIR_SOURCE,0);
  logMsg("INFO: Pause Executed\n",1,2,3,4,5,6);
  
} /*end pause */


static void
__go()
{
  int ii, jj, id, slot;

  logMsg("INFO: Entering Go 1\n",1,2,3,4,5,6);

#ifndef TI_SLAVE
  /* set sync event interval (in blocks) */
vmeBusLock();
 tiSetSyncEventInterval(10000/*block_level*/);
vmeBusUnlock();
#endif


#ifdef USE_FADC250


  /*  Enable FADC - old place
  for(id=0; id<nfadc; id++) 
  {
    FA_SLOT = faSlot(id);
    faChanDisable(FA_SLOT,0x0);
  }
  sleep(1);
  */

  
  /*  Send Sync Reset to FADC */
  /*faSDC_Sync();*/
#endif

#ifdef USE_V1190
  for(jj=0; jj<ntdcs; jj++)
  {
vmeBusLock();
    tdc1190Clear(jj);
vmeBusUnlock();
    error_flag[jj] = 0;
  }
  taskDelay(100);

#endif



#ifdef USE_VSCM
  for(ii=0; ii<nvscm1; ii++)
  {
    for(jj=0; jj<8; jj++)
    {
vmeBusLock();
      fssrSCR(vscmSlot(ii), jj);
vmeBusUnlock();
	}
  }

  /* Reset the Token */
  if(VSCM_ROFLAG==2)
  {
	for(ii=0; ii<nvscm1; ii++)
    {
	  slot = vscmSlot(ii);
vmeBusLock();
      vscmResetToken(slot);
vmeBusUnlock();
	}
  }

	for(ii=0; ii<nvscm1; ii++)
    {
	  slot = vscmSlot(ii);
vmeBusLock();
	  vscmStat(slot);
vmeBusUnlock();
	}


  /*
  printf("\n\nFSSR Status:\n\n");
  for(ii=0; ii<nvscm1; ii++)
  {
    for(jj=0; jj<8; jj++)
    {
      fssrStatus(vscmSlot(ii), jj);
	}
  }
  printf("\n\n\n\n");
  */

#endif

#ifdef USE_DC

  for(ii=0; ii<ndc; ii++)
  {
    DC_SLOT = dcSlot(ii);
vmeBusLock();
    dcClear(DC_SLOT);
vmeBusUnlock();
  }

#endif

#ifdef USE_DCRB
  for(ii=0; ii<ndcrb; ii++)
  {
    DCRB_SLOT = dcrbSlot(ii);
vmeBusLock();
    dcrbClear(DCRB_SLOT);
vmeBusUnlock();
  }
#endif

#ifdef USE_VETROC
  for(ii=0; ii<nvetroc; ii++)
  {
    VETROC_SLOT = vetrocSlot(ii);
vmeBusLock();
    vetrocClear(VETROC_SLOT);
vmeBusUnlock();
  }
#endif

#ifdef USE_DSC2
  for(ii=0; ii<ndsc2_daq; ii++)
  {
    slot = dsc2Slot(ii);
vmeBusLock();
    dsc2ResetScalersGroupA(slot);
    dsc2ResetScalersGroupB(slot);
vmeBusUnlock();
  }
#endif

  /* always clear exceptions */
  jlabgefClearException(1);

  nusertrig = 0;
  ndone = 0;


  CDOENABLE(TIPRIMARY,TIR_SOURCE,0); /* bryan has (,1,1) ... */



  logMsg("INFO: Go 1 Executed\n",1,2,3,4,5,6);
}


#define	COPY_FROM_DMA_TO_URS_BUFFER \
   if(dCnt<=0) \
   { \
	 printf("FADCs: No data or error.  dCnt = %d (slots from %d)\n",dCnt,FA_SLOT); \
     dCnt=0; \
   } \
   else \
   { \
	 /*#ifndef DMA_TO_BIGBUF*/							   \
     for(jj=0; jj<dCnt; jj++) *rol->dabufp++ = tdcbuf[jj]; \
	 /*#endif*/											   \
   }



void
usrtrig(unsigned int EVTYPE, unsigned int EVSOURCE)
{
  int *jw, ind, ind2, i, ii, jj, jjj, blen, len, rlen, itdcbuf, nbytes;
  unsigned int *tdcbuf_save, *tdc;
  unsigned int *dabufp1, *dabufp2;
  int njjloops, slot;
  int nwords;
#ifndef VXWORKS
  TIMERL_VAR;
#endif
#ifdef USE_FADC250
  unsigned int datascan, mask;
  unsigned short *dabufp16, *dabufp16_save;
  int id;
  int dCnt, idata;
  int stat, itime, gbready;
#endif
#ifdef USE_V1190
  int nev, rlenbuf[22];
  unsigned long tdcslot, tdcchan, tdcval, tdc14, tdcedge, tdceventcount;
  unsigned long tdceventid, tdcbunchid, tdcwordcount, tdcerrorflags;
  unsigned int *tdchead;
#ifdef SLOTWORKAROUND
  unsigned long tdcslot_h, tdcslot_t, remember_h;
#endif
#endif
#ifdef DMA_TO_BIGBUF
  unsigned int pMemBase, uMemBase, mSize;
#endif
  char *chptr, *chptr0;

  /*printf("EVTYPE=%d syncFlag=%d\n",EVTYPE,syncFlag);*/


  if(syncFlag) printf("EVTYPE=%d syncFlag=%d\n",EVTYPE,syncFlag);

  rol->dabufp = (int *) 0;

  /*
usleep(100);
  */
  /*
  sleep(1);
  */



  CEOPEN(EVTYPE, BT_BANKS); /* reformatted on CODA_format.c !!! */

  if((syncFlag<0)||(syncFlag>1))         /* illegal */
  {
    printf("Illegal1: syncFlag=%d EVTYPE=%d\n",syncFlag,EVTYPE);
  }
  else if((syncFlag==0)&&(EVTYPE==0))    /* illegal */
  {
    printf("Illegal2: syncFlag=%d EVTYPE=%d\n",syncFlag,EVTYPE);
  }
  else if((syncFlag==1)&&(EVTYPE==0))    /* force_sync (scaler) events */
  {
    ;
/*
!!! we are geting here on End transition: syncFlag=1 EVTYPE=0 !!!
*/
  }
  else if((syncFlag==0)&&(EVTYPE==15)) /* helicity strob events */
  {
    ;
  }
  else           /* physics and physics_sync events */
  {

    /* for EVIO format, will dump raw data */
    tdcbuf_save = tdcbuf;

    /*************/
	/* TI stuff */

    /* Set high, the first output port 
    tiSetOutputPort(1,0,0,0);
    */

    /* Grab the data from the TI */
vmeBusLock();
    len = tiReadBlock(tdcbuf,900>>2,1);
vmeBusUnlock();
    if(len<=0)
    {
      printf("ERROR in tiReadBlock : No data or error, len = %d\n",len);
      sleep(1);
    }
    else
    {
	  ;
	  /*
      printf("ti: len=%d\n",len);
      for(jj=0; jj<len; jj++) printf("ti[%2d] 0x%08x\n",jj,LSWAP(tdcbuf[jj]));
	  */

	  
      BANKOPEN(0xe10A,1,rol->pid);
      for(jj=0; jj<len; jj++) *rol->dabufp++ = tdcbuf[jj];
      BANKCLOSE;
	  
    }

    /* Turn off all output ports 
    tiSetOutputPort(0,0,0,0);
    */
	/* TI stuff */
    /*************/




#ifdef DEBUG
    printf("fadc1: start fadc processing\n");fflush(stdout);
#endif



#ifndef VXWORKS
TIMERL_START;
#endif




#ifdef USE_V1190
	if(ntdcs>0)
	{
vmeBusLock();
      tdc1190ReadStart(tdcbuf, rlenbuf);
vmeBusUnlock();
	  /*
	  rlenbuf[0] = tdc1190ReadBoard(0, tdcbuf);
	  rlenbuf[1] = tdc1190ReadBoard(1, &tdcbuf[rlenbuf[0]]);
	  */

      /*check if anything left in event buffer; if yes, print warning message and clear event buffer
      for(jj=0; jj<ntdcs; jj++)
      {
        nev = tdc1190Dready(jj);
        if(nev > 0)
		{
          printf("WARN: v1290[%2d] has %d events - clear it\n",jj,nev);
          tdc1190Clear(jj);
		}
	  }
      for(ii=0; ii<rlenbuf[0]; ii++) tdcbuf[ii] = LSWAP(tdcbuf[ii]);
	  */

      itdcbuf = 0;
      njjloops = ntdcs;

      BANKOPEN(0xe10B,1,rol->pid);
      for(ii=0; ii<njjloops; ii++)
      {
        rlen = rlenbuf[ii];
		/*
        printf("rol1(TDCs): ii=%d, rlen=%d\n",ii,rlen);
		*/

	  /*	  
#ifdef DEBUG
        level = tdc1190GetAlmostFullLevel(ii);
        iii = tdc1190StatusAlmostFull(ii);
        logMsg("ii=%d, rlen=%d, almostfull=%d level=%d\n",ii,rlen,iii,level,5,6);
#endif
	  */	  

        if(rlen <= 0) continue;

        tdc = &tdcbuf[itdcbuf];
        itdcbuf += rlen;

        for(jj=0; jj<rlen; jj++) *rol->dabufp ++ = tdc[jj];
      }
      BANKCLOSE;

	}


#endif /* USE_V1190 */






#ifdef USE_SSP
    if(nssp>0)
    {
#ifdef DEBUG
      printf("Calling sspGBReady ...\n");fflush(stdout);
#endif
      for(itime=0; itime<100000; itime++) 
	  {
vmeBusLock();
	    gbready = sspGBReady();
vmeBusUnlock();
	    stat = (gbready == sspSlotMask);
	    if (stat>0) 
	    {
	      break;
	    }
#ifdef DEBUG
		else
		{
          printf("SSP NOT READY: gbready=0x%08x, expect 0x%08x\n",gbready,sspSlotMask);
		}
#endif
	  }


#ifdef DEBUG
      printf("SSP IS READY: gbready=0x%08x, expect 0x%08x\n",gbready,sspSlotMask);
#endif

vmeBusLock();
      len = sspReadBlock(0,tdcbuf,0x10000,1);
vmeBusUnlock();
	  
/*
      printf("ssp tdcbuf[%2d]: 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
          len,tdcbuf[0],tdcbuf[1],tdcbuf[2],tdcbuf[3],tdcbuf[4],tdcbuf[5],tdcbuf[6]);
*/

	  
      if(len>0)
      {
        BANKOPEN(0xe10C,1,rol->pid);
        for(jj=0; jj<len; jj++) *rol->dabufp++ = tdcbuf[jj];
        BANKCLOSE;
      }
	  
    }
#endif /* USE_SSP */



#ifdef USE_FADC250

    /* Configure Block Type... temp fix for 2eSST trouble with token passing */
    tdcbuf = tdcbuf_save;
    dCnt=0;
    if(nfadc>0)
    {


/*COMMENT OUT FOLLOWING 'FOR' LOOP FOR SPEED UP !!!*/
      for(itime=0; itime<100000; itime++) 
	  {
vmeBusLock();
	    gbready = faGBready();
vmeBusUnlock();
	    stat = (gbready == fadcSlotMask);
	    if (stat>0) 
	    {
	      break;
	    }
	  }

      if(stat>0)
	  {
        BANKOPEN(0xe109,1,rol->pid);

        FA_SLOT = faSlot(0);
        if(FADC_ROFLAG==2)
        {

#ifdef DMA_TO_BIGBUF
 		  /*
          printf("dabufp_usermembase=0x%08x\n",dabufp_usermembase);
          printf("dabufp_physmembase=0x%08x\n",dabufp_physmembase);
           */
 /*1ms->*/
          uMemBase = dabufp_usermembase;
          pMemBase = dabufp_physmembase;
          mSize = 0x100000;
 	      /*printf("desired: 0x%08x 0x%08x 0x%08x\n",pMemBase,uMemBase,mSize);*/
          usrChangeVmeDmaMemory(pMemBase, uMemBase, mSize);
 
          usrVmeDmaMemory(&pMemBase, &uMemBase, &mSize);
 	      /*printf("actual: 0x%08x 0x%08x 0x%08x\n",pMemBase,uMemBase,mSize);*/
 /*->1us*/
 
 
 /*25us->*/
vmeBusLock();
 	      dCnt = faReadBlock(FA_SLOT,rol->dabufp,0x100000/*MAXFADCWORDS*/,FADC_ROFLAG);
vmeBusUnlock();
 /*->25us*/
#ifdef DEBUG
  		  printf("dCnt=%d\n",dCnt);
          for(jjj=0; jjj<dCnt; jjj++) printf(" [%3d]  0x%08x\n",jjj,rol->dabufp[jjj]);
#endif
          rol->dabufp += dCnt;

          usrRestoreVmeDmaMemory();
          usrVmeDmaMemory(&pMemBase, &uMemBase, &mSize);
 	      /*printf("restored: 0x%08x 0x%08x 0x%08x\n",pMemBase,uMemBase,mSize);*/
#else









#ifdef DEBUG
          printf("fadc1: Starting DMA\n");fflush(stdout);
#endif


vmeBusLock();
	      dCnt = faReadBlock(FA_SLOT,tdcbuf,500000/*MAXFADCWORDS*/,FADC_ROFLAG);
		  COPY_FROM_DMA_TO_URS_BUFFER;
vmeBusUnlock();


#ifdef DEBUG
          printf("fadc1: Finished DMA, dCnt=%d\n",dCnt);fflush(stdout);
#endif







#endif



        }
        else
		{




          for(jj=0; jj<nfadc; jj++)
		  {
#ifdef DEBUG
            printf("fadc1: [%d] Starting DMA\n",jj);fflush(stdout);
#endif
#ifdef DMA_TO_BIGBUF

            uMemBase = dabufp_usermembase;
            pMemBase = dabufp_physmembase;
            mSize = 0x100000;
            usrChangeVmeDmaMemory(pMemBase, uMemBase, mSize);


vmeBusLock();
	        len = faReadBlock(faSlot(jj),rol->dabufp,500000/*MAXFADCWORDS*/,FADC_ROFLAG);
vmeBusUnlock();
            rol->dabufp += len;
            dCnt += len;

            usrRestoreVmeDmaMemory();
#else
vmeBusLock();
	        len = faReadBlock(faSlot(jj),&tdcbuf[dCnt],500000/*MAXFADCWORDS*/,FADC_ROFLAG);
vmeBusUnlock();
            dCnt += len;
#endif

#ifdef DEBUG
            printf("fadc1: [%d] len=%d dCnt=%d\n",jj,len,dCnt);
            for(jjj=0; jjj<len; jjj++) printf(" [%3d]  0x%08x\n",jjj,tdcbuf[(dCnt-len)+jjj]);
#endif
		  }

		  COPY_FROM_DMA_TO_URS_BUFFER;



	    }



		/*old place
		COPY_FROM_DMA_TO_URS_BUFFER;
		*/







        BANKCLOSE;

	  }
      else 
	  {
	    printf ("FADCs: no events   stat=%d  intcount = %d   gbready = 0x%08x  fadcSlotMask = 0x%08x\n",
		  stat,tiGetIntCount(),gbready,fadcSlotMask);
        printf("Missing slots:");
        for(jj=1; jj<21; jj++)
		{
          mask = 1<<jj;
          if((fadcSlotMask&mask) && !(gbready&mask)) printf("%3d",jj);
		}
        printf("\n");
	  }

      /* Reset the Token */
      if(FADC_ROFLAG==2)
	  {
/*2us->*/
	    for(id=0; id<nfadc; id++)
	    {
	      FA_SLOT = faSlot(id);
vmeBusLock();
	      faResetToken(FA_SLOT);
vmeBusUnlock();
	    }
/*->2us*/
	  }

    }

#endif /* USE_FADC250 */





#ifdef USE_VSCM

    /* Configure Block Type... temp fix for 2eSST trouble with token passing */
    tdcbuf = tdcbuf_save;
    dCnt=0;
    if(nvscm1 != 0)
    {

#ifdef DEBUG
      printf("Calling vscmGBReady ...\n");fflush(stdout);
#endif
      for(itime=0; itime<100000; itime++) 
	  {
vmeBusLock();
	    gbready = vscmGBReady();
vmeBusUnlock();
	    stat = (gbready == vscmSlotMask);
	    if (stat>0) 
	    {
	      break;
	    }
#ifdef DEBUG
		else
		{
          printf("VSCM NOT READY: gbready=0x%08x, expect 0x%08x\n",gbready,vscmSlotMask);
		}
#endif
	  }


#ifdef DEBUG
	  
	  /* print fifo info */
      printf("FIFOs info:\n");fflush(stdout);
      for(jj=0; jj<nvscm1; jj++)
	  {
vmeBusLock();
        vscmStat(vscmSlot(jj));
vmeBusUnlock();
	  }
      printf("mask=0x%08x gbready=0x%08x stat=%d\n",vscmSlotMask,gbready,stat);
	  
#endif
	  
      if(stat>0)
	  {
        BANKOPEN(0xe104,1,rol->pid);

        if(VSCM_ROFLAG==2)
        {
          slot = vscmSlot(0);
#ifdef DMA_TO_BIGBUF
          uMemBase = dabufp_usermembase;
          pMemBase = dabufp_physmembase;
          mSize = 0x100000;
          usrChangeVmeDmaMemory(pMemBase, uMemBase, mSize);
 
          usrVmeDmaMemory(&pMemBase, &uMemBase, &mSize);
 

 
vmeBusLock();
 	      dCnt = vscmReadBlock(slot,rol->dabufp,500000/*MAXFADCWORDS*/,VSCM_ROFLAG);
vmeBusUnlock();
#ifdef DEBUG
		  printf("readout ends, len=%d\n",dCnt);
          /*if(len>12) len=12;*/
          vscmPrintFifo(rol->dabufp,dCnt);
#endif
          rol->dabufp += dCnt;
		  /*  
 		  printf("dCnt=%d, data: 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",dCnt,
 		  rol->dabufp[0],rol->dabufp[1],rol->dabufp[2],rol->dabufp[3],rol->dabufp[4],rol->dabufp[5]);
 		  */
          usrRestoreVmeDmaMemory();
          usrVmeDmaMemory(&pMemBase, &uMemBase, &mSize);
#else

vmeBusLock();
	      dCnt = vscmReadBlock(slot,tdcbuf,500000/*MAXFADCWORDS*/,VSCM_ROFLAG);
vmeBusUnlock();

#ifdef DEBUG
		  printf("readout ends, len=%d\n",dCnt);
          /*if(len>12) len=12;*/
          vscmPrintFifo(tdcbuf,dCnt);
#endif




#endif
        }
        else
		{
#ifdef DEBUG
		  printf("readout, VSCM_ROFLAG=%d\n",VSCM_ROFLAG);
#endif
          for(jj=0; jj<nvscm1; jj++)
		  {
#ifdef DMA_TO_BIGBUF

            uMemBase = dabufp_usermembase;
            pMemBase = dabufp_physmembase;
            mSize = 0x100000;
            usrChangeVmeDmaMemory(pMemBase, uMemBase, mSize);


vmeBusLock();
	        len = vscmReadBlock(vscmSlot(jj),rol->dabufp,1000/*MAXFADCWORDS*/,VSCM_ROFLAG);
vmeBusUnlock();

#ifdef DEBUG
			printf("readout ends, len=%d\n",len);
            /*if(len>12) len=12;*/
            vscmPrintFifo(rol->dabufp,len);
#endif

			/*			
vscmPrintFifo(rol->dabufp,len);
			*/

            rol->dabufp += len;
            dCnt += len;

            usrRestoreVmeDmaMemory();

#else

vmeBusLock();
	        len = vscmReadBlock(vscmSlot(jj),&tdcbuf[dCnt],1000/*MAXFADCWORDS*/,VSCM_ROFLAG);
vmeBusUnlock();

#ifdef DEBUG
			printf("readout ends, len=%d\n",len);
            /*if(len>12) len=12;*/
            vscmPrintFifo(&tdcbuf[dCnt],len);
#endif

            dCnt += len;
#endif

#ifdef DEBUG
            printf("[%d] len=%d dCnt=%d\n",jj,len,dCnt);
#endif
		  }
	    }

	    if(dCnt<=0)
	    {
	      printf("VSCM: No data or error.  dCnt = %d (slots from %d)\n",dCnt,slot);
          dCnt=0;
	    }
	    else
	    {
#ifndef DMA_TO_BIGBUF
          for(jj=0; jj<dCnt; jj++) *rol->dabufp++ = tdcbuf[jj];
#endif
	    }

        BANKCLOSE;
	  }
      else 
	  {
	    printf ("VSCMs: no events   stat=%d  intcount = %d   gbready = 0x%08x  vscmSlotMask = 0x%08x\n",
		  stat,tiGetIntCount(),gbready,vscmSlotMask);
        printf("Missing slots:");
        for(jj=1; jj<21; jj++)
		{
          mask = 1<<jj;
          if((vscmSlotMask&mask) && !(gbready&mask)) printf("%3d",jj);
		}
        printf("\n");
	  }

      /* Reset the Token */
      if(VSCM_ROFLAG==2)
	  {
	    for(ii=0; ii</*1*/nvscm1; ii++)
	    {
	      slot = vscmSlot(ii);
vmeBusLock();
	      vscmResetToken(slot);
vmeBusUnlock();
	    }
	  }

    }

#endif /* USE_VSCM */




#ifdef USE_DC



    /* Configure Block Type... temp fix for 2eSST trouble with token passing */
    tdcbuf = tdcbuf_save;
    dCnt=0;
    if(ndc != 0)
    {
      stat = 0;
      for(itime=0; itime<100000; itime++) 
	  {
vmeBusLock();
	    gbready = dcGBready();
vmeBusUnlock();
	    stat = (gbready == dcSlotMask);
	    if (stat>0) 
	    {
          printf("expected mask 0x%08x, got 0x%08x\n",dcSlotMask,gbready);
	      break;
	    }
	  }
      if(stat==0) printf("dc not ready !!!\n");


      if(stat>0)
	  {
        BANKOPEN(0xe105,1,rol->pid);

        DC_SLOT = dcSlot(0);
        if(DC_ROFLAG==2)
        {
#ifdef DMA_TO_BIGBUF
          uMemBase = dabufp_usermembase;
          pMemBase = dabufp_physmembase;
          mSize = 0x100000;
          usrChangeVmeDmaMemory(pMemBase, uMemBase, mSize);
 
          usrVmeDmaMemory(&pMemBase, &uMemBase, &mSize);
 
vmeBusLock();
 	      dCnt = dcReadBlock(DC_SLOT,rol->dabufp,MAXDCWORDS,DC_ROFLAG);
vmeBusUnlock();
          rol->dabufp += dCnt;
          usrRestoreVmeDmaMemory();
#else
vmeBusLock();
	      dCnt = dcReadBlock(DC_SLOT,tdcbuf,MAXDCWORDS,DC_ROFLAG);
vmeBusUnlock();
#endif
        }
        else
		{
          for(jj=0; jj<ndc; jj++)
	      {
            DC_SLOT = dcSlot(jj);
	        /*dcPrintScalers(DC_SLOT);*/
#ifdef DMA_TO_BIGBUF
            uMemBase = dabufp_usermembase;
            pMemBase = dabufp_physmembase;
            mSize = 0x100000;
            usrChangeVmeDmaMemory(pMemBase, uMemBase, mSize);

vmeBusLock();
            len = dcReadBlock(DC_SLOT, rol->dabufp, MAXDCWORDS, DC_ROFLAG);
vmeBusUnlock();
/*#ifdef DEBUG*/
            printf("DC: slot=%d, nw=%d, data-> 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
			  DC_SLOT,len,LSWAP(rol->dabufp[0]),LSWAP(rol->dabufp[1]),LSWAP(rol->dabufp[2]),
			  LSWAP(rol->dabufp[3]),LSWAP(rol->dabufp[4]),LSWAP(rol->dabufp[5]),LSWAP(rol->dabufp[6]));
/*#endif*/
            rol->dabufp += len;
            dCnt += len;

            usrRestoreVmeDmaMemory();
#else
vmeBusLock();
            len = dcReadBlock(DC_SLOT, &tdcbuf[dCnt], MAXDCWORDS, DC_ROFLAG);
vmeBusUnlock();
/*#ifdef DEBUG*/
            printf("DC: slot=%d, nw=%d, data-> 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
			  DC_SLOT,len,LSWAP(tdcbuf[dCnt+0]),LSWAP(tdcbuf[dCnt+1]),LSWAP(tdcbuf[dCnt+2]),
              LSWAP(tdcbuf[dCnt+3]),LSWAP(tdcbuf[dCnt+4]),LSWAP(tdcbuf[dCnt+5]),LSWAP(tdcbuf[dCnt+6]));
/*#endif*/
            dCnt += len;
#endif
	      }
		}

	    if(dCnt<=0)
	    {
	      printf("DC: No data or error.  dCnt = %d\n",dCnt);
          dCnt=0;
	    }
        else
	    {
#ifndef DMA_TO_BIGBUF
          for(jj=0; jj<dCnt; jj++) *rol->dabufp++ = tdcbuf[jj];
#endif
	    }

        BANKCLOSE;
	  }
	  else
	  {
	    printf ("DCs: no events   stat=%d  intcount = %d   gbready = 0x%08x  dcSlotMask = 0x%08x\n",
		  stat,tiGetIntCount(),gbready,dcSlotMask);
        printf("Missing slots:");
        for(jj=1; jj<21; jj++)
		{
          mask = 1<<jj;
          if((dcSlotMask&mask) && !(gbready&mask)) printf("%3d",jj);
		}
        printf("\n");
	  }

      /* Reset the Token */
      if(DC_ROFLAG==2)
	  {
	    for(jj=0; jj<ndc; jj++)
	    {
	      DC_SLOT = dcSlot(jj);
vmeBusLock();
	      /*dcResetToken(DC_SLOT);not implemented yet !!!!!!!!!!*/
vmeBusUnlock();
	    }
	  }

    }

#endif /*USE_DC*/





#ifdef USE_DCRB



    /* Configure Block Type... temp fix for 2eSST trouble with token passing */
    tdcbuf = tdcbuf_save;
    dCnt=0;
    if(ndcrb != 0)
    {
      stat = 0;
      for(itime=0; itime<100000; itime++) 
	  {
vmeBusLock();
	    gbready = dcrbGBready();
vmeBusUnlock();
	    stat = (gbready == dcrbSlotMask);
	    if (stat>0) 
	    {
          printf("expected mask 0x%08x, got 0x%08x\n",dcrbSlotMask,gbready);
	      break;
	    }
	  }
      if(stat==0) printf("dcrb not ready !!!\n");


      if(stat>0)
	  {
        BANKOPEN(0xe105,1,rol->pid);

        DCRB_SLOT = dcrbSlot(0);
        if(DCRB_ROFLAG==2)
        {
#ifdef DMA_TO_BIGBUF
          uMemBase = dabufp_usermembase;
          pMemBase = dabufp_physmembase;
          mSize = 0x100000;
          usrChangeVmeDmaMemory(pMemBase, uMemBase, mSize);
 
          usrVmeDmaMemory(&pMemBase, &uMemBase, &mSize);
 
vmeBusLock();
 	      dCnt = dcrbReadBlock(DCRB_SLOT,rol->dabufp,MAXDCRBWORDS,DCRB_ROFLAG);
vmeBusUnlock();
          rol->dabufp += dCnt;
          usrRestoreVmeDmaMemory();
#else
vmeBusLock();
	      dCnt = dcrbReadBlock(DCRB_SLOT,tdcbuf,MAXDCRBWORDS,DCRB_ROFLAG);
vmeBusUnlock();
#endif
        }
        else
		{
          for(jj=0; jj<ndcrb; jj++)
	      {
            DCRB_SLOT = dcrbSlot(jj);
#ifdef DMA_TO_BIGBUF
            uMemBase = dabufp_usermembase;
            pMemBase = dabufp_physmembase;
            mSize = 0x100000;
            usrChangeVmeDmaMemory(pMemBase, uMemBase, mSize);

vmeBusLock();
            len = dcrbReadBlock(DCRB_SLOT, rol->dabufp, MAXDCRBWORDS, DCRB_ROFLAG);
vmeBusUnlock();
/*#ifdef DEBUG*/
            printf("DCRB: slot=%d, nw=%d, data-> 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
			  DCRB_SLOT,len,LSWAP(rol->dabufp[0]),LSWAP(rol->dabufp[1]),LSWAP(rol->dabufp[2]),
			  LSWAP(rol->dabufp[3]),LSWAP(rol->dabufp[4]),LSWAP(rol->dabufp[5]),LSWAP(rol->dabufp[6]));
/*#endif*/
            rol->dabufp += len;
            dCnt += len;

            usrRestoreVmeDmaMemory();
#else
vmeBusLock();
            len = dcrbReadBlock(DCRB_SLOT, &tdcbuf[dCnt], MAXDCRBWORDS, DCRB_ROFLAG);
vmeBusUnlock();
/*#ifdef DEBUG*/
            printf("DC: slot=%d, nw=%d, data-> 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
			  DCRB_SLOT,len,LSWAP(tdcbuf[dCnt+0]),LSWAP(tdcbuf[dCnt+1]),LSWAP(tdcbuf[dCnt+2]),
              LSWAP(tdcbuf[dCnt+3]),LSWAP(tdcbuf[dCnt+4]),LSWAP(tdcbuf[dCnt+5]),LSWAP(tdcbuf[dCnt+6]));
/*#endif*/
            dCnt += len;
#endif
	      }
		}

	    if(dCnt<=0)
	    {
	      printf("DCRB: No data or error.  dCnt = %d\n",dCnt);
          dCnt=0;
	    }
        else
	    {
#ifndef DMA_TO_BIGBUF
          for(jj=0; jj<dCnt; jj++) *rol->dabufp++ = tdcbuf[jj];
#endif
	    }

        BANKCLOSE;
	  }
	  else
	  {
	    printf ("DCRBs: no events   stat=%d  intcount = %d   gbready = 0x%08x  dcrbSlotMask = 0x%08x\n",
		  stat,tiGetIntCount(),gbready,dcrbSlotMask);
        printf("Missing slots:");
        for(jj=1; jj<21; jj++)
		{
          mask = 1<<jj;
          if((dcrbSlotMask&mask) && !(gbready&mask)) printf("%3d",jj);
		}
        printf("\n");
	  }

      /* Reset the Token */
      if(DCRB_ROFLAG==2)
	  {
	    for(jj=0; jj<ndcrb; jj++)
	    {
	      DCRB_SLOT = dcrbSlot(jj);
vmeBusLock();
	      /*dcResetToken(DC_SLOT);not implemented yet !!!!!!!!!!*/
vmeBusUnlock();
	    }
	  }

    }

#endif /*USE_DCRB*/




#ifdef USE_VETROC
    /* Configure Block Type... temp fix for 2eSST trouble with token passing */
    tdcbuf = tdcbuf_save;
    dCnt=0;
    if(nvetroc != 0)
    {
      stat = 0;
      for(itime=0; itime<100000; itime++) 
	  {
vmeBusLock();
	    gbready = vetrocGBready();
vmeBusUnlock();
	    stat = (gbready == vetrocSlotMask);
	    if (stat>0) 
	    {
          printf("expected mask 0x%08x, got 0x%08x\n",vetrocSlotMask,gbready);
	      break;
	    }
	  }
      if(stat==0) printf("vetroc not ready !!!\n");


      if(stat>0)
	  {
        BANKOPEN(0xe105,1,rol->pid);

        VETROC_SLOT = vetrocSlot(0);
        if(VETROC_ROFLAG==2)
        {
#ifdef DMA_TO_BIGBUF
          uMemBase = dabufp_usermembase;
          pMemBase = dabufp_physmembase;
          mSize = 0x100000;
          usrChangeVmeDmaMemory(pMemBase, uMemBase, mSize);
 
          usrVmeDmaMemory(&pMemBase, &uMemBase, &mSize);
 
vmeBusLock();
 	      dCnt = vetrocReadBlock(VETROC_SLOT,rol->dabufp,MAXVETROCWORDS,VETROC_ROFLAG);
vmeBusUnlock();
          rol->dabufp += dCnt;
          usrRestoreVmeDmaMemory();
#else
vmeBusLock();
	      dCnt = vetrocReadBlock(VETROC_SLOT,tdcbuf,MAXVETROCWORDS,VETROC_ROFLAG);
vmeBusUnlock();
#endif
        }
        else
		{
          for(jj=0; jj<nvetroc; jj++)
	      {
            VETROC_SLOT = vetrocSlot(jj);
#ifdef DMA_TO_BIGBUF
            uMemBase = dabufp_usermembase;
            pMemBase = dabufp_physmembase;
            mSize = 0x100000;
            usrChangeVmeDmaMemory(pMemBase, uMemBase, mSize);

vmeBusLock();
            len = vetrocReadBlock(VETROC_SLOT, rol->dabufp, MAXVETROCWORDS, VETROC_ROFLAG);
vmeBusUnlock();
/*#ifdef DEBUG*/
            printf("VETROC: slot=%d, nw=%d, data-> 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
			  VETROC_SLOT,len,LSWAP(rol->dabufp[0]),LSWAP(rol->dabufp[1]),LSWAP(rol->dabufp[2]),
			  LSWAP(rol->dabufp[3]),LSWAP(rol->dabufp[4]),LSWAP(rol->dabufp[5]),LSWAP(rol->dabufp[6]));
/*#endif*/
            rol->dabufp += len;
            dCnt += len;

            usrRestoreVmeDmaMemory();
#else
vmeBusLock();
            len = vetrocReadBlock(VETROC_SLOT, &tdcbuf[dCnt], MAXVETROCWORDS, VETROC_ROFLAG);
vmeBusUnlock();
/*#ifdef DEBUG*/
            printf("VETROC: slot=%d, nw=%d, data-> 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
			  VETROC_SLOT,len,LSWAP(tdcbuf[dCnt+0]),LSWAP(tdcbuf[dCnt+1]),LSWAP(tdcbuf[dCnt+2]),
              LSWAP(tdcbuf[dCnt+3]),LSWAP(tdcbuf[dCnt+4]),LSWAP(tdcbuf[dCnt+5]),LSWAP(tdcbuf[dCnt+6]));
/*#endif*/
            dCnt += len;
#endif
	      }
		}

	    if(dCnt<=0)
	    {
	      printf("VETROC: No data or error.  dCnt = %d\n",dCnt);
          dCnt=0;
	    }
        else
	    {
#ifndef DMA_TO_BIGBUF
          for(jj=0; jj<dCnt; jj++) *rol->dabufp++ = tdcbuf[jj];
#endif
	    }

        BANKCLOSE;
	  }
	  else
	  {
	    printf ("VETROCs: no events   stat=%d  intcount = %d   gbready = 0x%08x  VETROCSlotMask = 0x%08x\n",
		  stat,tiGetIntCount(),gbready,vetrocSlotMask);
        printf("Missing slots:");
        for(jj=1; jj<21; jj++)
		{
          mask = 1<<jj;
          if((vetrocSlotMask&mask) && !(gbready&mask)) printf("%3d",jj);
		}
        printf("\n");
	  }

      /* Reset the Token */
      if(VETROC_ROFLAG==2)
	  {
	    for(jj=0; jj<nvetroc; jj++)
	    {
	      VETROC_SLOT = vetrocSlot(jj);
vmeBusLock();
	      /*vetrocResetToken(DC_SLOT);not implemented yet !!!!!!!!!!*/
vmeBusUnlock();
	    }
	  }
    }
#endif /*USE_VETROC*/


#ifndef TI_SLAVE

  /* create HEAD bank if master and standalone crates, NOT slave */

	event_number = (EVENT_NUMBER) * block_level - block_level;

    BANKOPEN(0xe112,1,0);

	dabufp1 = rol->dabufp;

    *rol->dabufp ++ = LSWAP((0x10<<27)+block_level); /*block header*/

    for(ii=0; ii<block_level; ii++)
	{
      event_number ++;
	  /*
	  printf(">>>>>>>>>>>>> %d %d\n",(EVENT_NUMBER),event_number);
      sleep(1);
	  */
      *rol->dabufp ++ = LSWAP((0x12<<27)+(event_number&0x7FFFFFF)); /*event header*/

      nwords = 5; /* UPDATE THAT IF THE NUMBER OF WORDS CHANGED BELOW !!! */
      *rol->dabufp ++ = LSWAP((0x14<<27)+nwords); /*head data*/
      *rol->dabufp ++ = 0; /*version  number */
      *rol->dabufp ++ = LSWAP(RUN_NUMBER); /*run  number */
      *rol->dabufp ++ = LSWAP(event_number); /*event number */
      if(ii==(block_level-1))
	  {
        *rol->dabufp ++ = LSWAP(time(0)); /*event unix time */
        *rol->dabufp ++ = LSWAP(EVTYPE); /*event type */
	  }
      else
	  {
        *rol->dabufp ++ = 0;
        *rol->dabufp ++ = 0;
	  }
	}

    nwords = ((int)rol->dabufp-(int)dabufp1)/4+1;

    *rol->dabufp ++ = LSWAP((0x11<<27)+nwords); /*block trailer*/

    BANKCLOSE;

#endif




#ifndef VXWORKS
TIMERL_STOP(100000/block_level,1000+rol->pid);
#endif




    /* read boards configurations */
    if(syncFlag==1 || EVENT_NUMBER==1)
    {
      BANKOPEN(0xe10E,3,rol->pid);
      chptr = chptr0 =(char *)rol->dabufp;
      nbytes = 0;

      /* add one 'return' to make evio2xml output nicer */
      *chptr++ = '\n';
      nbytes ++;

vmeBusLock();
      len = tiUploadAll(chptr, 10000);
vmeBusUnlock();
      /*printf("\nTI len=%d\n",len);
      printf(">%s<\n",chptr);*/
      chptr += len;
      nbytes += len;


#ifdef USE_FADC250
      if(nfadc>0)
      {
vmeBusLock();
        len = fadc250UploadAll(chptr, 12000);
vmeBusUnlock();
        /*printf("\nFADC len=%d\n",len);
        printf("%s\n",chptr);*/
        chptr += len;
        nbytes += len;
	  }
#endif

#ifdef USE_V1190_HIDE
	  if(ntdcs>0)
	  {
vmeBusLock();
        len = tdc1190UploadAll(chptr, 10000);
vmeBusUnlock();
        /*printf("\nTDC len=%d\n",len);
        printf("%s\n",chptr);*/
        chptr += len;
        nbytes += len;
	  }
#endif

#ifdef USE_DSC2
	  if(ndsc2>0)
	  {
vmeBusLock();
        len = dsc2UploadAll(chptr, 10000);
vmeBusUnlock();
        /*printf("\nDSC2 len=%d\n",len);
        printf("%s\n",chptr);*/
        chptr += len;
        nbytes += len;
	  }
#endif

#ifdef USE_VSCM
	  if(nvscm1>0)
	  {
vmeBusLock();
        len = vscmUploadAll(chptr, 65535);
vmeBusUnlock();
        /*printf("\nVSCM len=%d\n",len);
        printf("%s\n",chptr);*/
        chptr += len;
        nbytes += len;
	  }
#endif


#ifdef USE_SSP
      if(nssp>0)
      {
vmeBusLock();
        len = sspUploadAll(chptr, 10000);
vmeBusUnlock();
        /*printf("\nSSP len=%d\n",len);
        printf("%s\n",chptr);*/
        chptr += len;
        nbytes += len;
	  }
#endif


	  /* temporary for crates with GTP */
      if(rol->pid==37||rol->pid==39)
	  {
#define TEXT_STR  1000
        char *roc;
        int  ii, kk, stt = 0;
        char result[TEXT_STR];      /* string for messages from tcpClientCmd */
        char exename[200];          /* command to be executed by tcpServer */

        if(rol->pid==37) roc = "hps1gtp";
        else             roc = "hps2gtp";

        sprintf(exename,"gtpUploadAllPrint()");

        /*printf("gtptest1: roc >%s< exename >%s<\n",roc,exename);*/

        memset(result,0,TEXT_STR);
        tcpClientCmd(roc, exename, result);

        len = strlen(result) - 2; /* 'result' has 2 extra chars in the end we do not want ????? */
        /*printf("gtptest1: len=%d, >%s<",len,result);*/

        strncpy(chptr,result,len);
        chptr += len;
        nbytes += len;
	  }


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












	


    /* read scaler(s) */
    if(syncFlag==1 || EVENT_NUMBER==1)
    {
#ifdef USE_DSC2
	  /*printf("ndsc2_daq=%d\n",ndsc2_daq);*/
	  if(ndsc2_daq>0)
	  {
        BANKOPEN(0xe115,1,rol->pid);
        for(jj=0; jj<ndsc2_daq; jj++)
        {
          slot = dsc2Slot_daq(jj);
vmeBusLock();
          /* in following argument 4 set to 0xFF means latch and read everything, 0x3F - do not latch and read everything */
          nwords = dsc2ReadScalers(slot, tdcbuf, 0x10000, 0xFF, 1);
          /*printf("nwords=%d, nwords = 0x%08x 0x%08x 0x%08x 0x%08x\n",nwords,tdcbuf[0],tdcbuf[1],tdcbuf[2],tdcbuf[3]);*/
vmeBusUnlock();

#ifdef SSIPC
	      {
            int status, mm;
            unsigned int dd[72];
            for(mm=0; mm<72; mm++) dd[mm] = tdcbuf[mm];
            status = epics_msg_send("hallb_dsc2_hps2_slot2","uint",72,dd);
	      }
#endif
          /* unlike other boards, dcs2 scaler readout already swapped in 'dsc2ReadScalers', so swap it back, because
          hps2.c expects big-endian format*/
          for(ii=0; ii<nwords; ii++) *rol->dabufp ++ = LSWAP(tdcbuf[ii]);
        }
        BANKCLOSE;
	  }

#endif
	}


#ifndef TI_SLAVE
    /* print livetite */
    if(syncFlag==1)
	{
      int livetime, live_percent;
vmeBusLock();
      tiLatchTimers();
      livetime = tiLive(0);
vmeBusUnlock();
      live_percent = livetime/10;
	  printf("============= Livetime=%3d percent\n",live_percent);
#ifdef SSIPC
	  {
        int status;
        status = epics_msg_send("hallb_livetime","int",1,&live_percent);
	  }
#endif
	}
#endif


    /* for physics sync event, make sure all board buffers are empty */
    if(syncFlag==1)
    {
      int nblocks;
      nblocks = tiGetNumberOfBlocksInBuffer();
      /*printf(" Blocks ready for readout: %d\n\n",nblocks);*/

      if(nblocks)
	  {
        printf("SYNC ERROR: TI nblocks = %d\n",nblocks);fflush(stdout);
        sleep(10);
	  }
	}




#if 0
    /* for physics sync event, make sure all board buffers are empty */
    if(syncFlag==1)
    {
      int scan_flag = 0;
      unsigned short slot, nev16;

	  /*
	  if(scan_flag) logMsg("SYNC: scan_flag=0x%08x\n",scan_flag,2,3,4,5,6);
	  */
      ind2 = bosMopen_(jw,"PTRN",rol->pid,1,1);
      jw[ind2+1] = SYNC_FLAG + scan_flag;
      rol->dabufp += bosMclose_(jw,ind2,1,1);
    }
#endif


  }

  /* close event */
  CECLOSE;

  /*
  nusertrig ++;
  printf("usrtrig called %d times\n",nusertrig);fflush(stdout);
  */
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
  /*
  ndone ++;
  printf("_done called %d times\n",ndone);fflush(stdout);
  */
  /* from parser */
  poolEmpty = 0; /* global Done, Buffers have been freed */

  /* Acknowledge tir register */
  CDOACK(TIPRIMARY,TIR_SOURCE,0);

  return;
}

static void
__status()
{
  return;
}  

#else

void
fadc1_dummy()
{
  return;
}

#endif
