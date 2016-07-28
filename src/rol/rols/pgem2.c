
/* pgem2.c - PRAD master VME crate first readout list */

#if defined(VXWORKS) || defined(Linux_vme)

#define NEW

/* PRad Trigger Table */
#define TRG_LGSUM TI_TSINPUT_1
#define TRG_TOTALSUM TI_TSINPUT_2
#define TRG_LMSLED   TI_TSINPUT_3
#define TRG_LMSALPHA TI_TSINPUT_4
#define TRG_TAGGERE  TI_TSINPUT_5
#define TRG_SCIN     TI_TSINPUT_6

#define TRG_PRODUCTION (TRG_LGSUM | TRG_TOTALSUM)
//#define TRG_PRODUCTION TRG_SCIN

#undef SSIPC

static int nusertrig, ndone;

#define USE_PRAD
#define EVENT_CYCLE
#undef USE_V767
#define USE_DSC2
#define USE_V1190
#undef V1190_TEST

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
#define ROL_NAME__ "PGEM1"

/* polling mode if needed */
#define POLLING_MODE

/* main TI board */
#define TI_ADDR   0x100000  /* slot 2 */


/* name used by loader */

#ifdef TI_MASTER
#define INIT_NAME pgem2_master__init
#define TI_READOUT TI_READOUT_EXT_POLL /* Poll for available data, front panel triggers */
#else
#ifdef TI_SLAVE
#define INIT_NAME pgem2_slave__init
#define TI_READOUT TI_READOUT_TS_POLL /* Poll for available data, triggers from master over fiber */
#else
#define INIT_NAME pgem2__init
#define TI_READOUT TI_READOUT_EXT_POLL /* Poll for available data, front panel triggers */
#endif
#endif

#include "rol.h"
#include "TIPRIMARY_source.h"



#ifdef USE_PRAD

#include "dsc2Lib.h"
static int ndsc2=0, ndsc2_daq=0;

#ifdef USE_V767
#include "v767Lib.h"

/* V767_ADDR1 is the HW address for the leftmost v767 in the crate */
unsigned int V767_ADDR1 = 0x380000;
/* V767_OFF is the increment for each subsequent v767 (ie. next module would be 0x180000) */
unsigned int V767_OFF = 0x080000;
#endif

#ifdef USE_V1190

#include "tdc1190.h"
unsigned int V1190_ADDR1 = 0x380000;
unsigned int V1190_OFF = 0x080000;
unsigned int datascan;
int ntdcs;
unsigned int rlenbuf[24];
#endif

#endif

void usrtrig(unsigned int EVTYPE, unsigned int EVSOURCE);
void usrtrig_done();




/* user code */


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

static int livetime, live_percent;

static unsigned int trigger_type = TRG_LMSLED;
static unsigned int lms_phase = 1;

#ifdef EVENT_CYCLE
unsigned int event_count = 0;
#endif

/* PRad LMS Wheel */
/* input: number of phase shift of lms wheel*/
/* lms wheel phase: 1 -6 */
/* 1, 3, 5, open; 2, 4, 6, close. */
/* phase change will take about 1-2 seconds */
/* ti OT#3 is connected to gate generator to send the signal */
void lms_phase_shift()
{
    tiSetOutputPort(0, 0, 1, 0);
    taskDelay(10);
    tiSetOutputPort(0, 0, 0, 0);

    // update lms phase
    ++lms_phase;
    if(lms_phase > 6)
      lms_phase = 1;
    printf("Changing lms phase to %d. \n", lms_phase);
    printf("***Check camera and make sure the phase is correct.***\n");
}

void
lms_phase_change(int n)
{
  // control LMS wheel
  int lms_loop;
  for(lms_loop = 0; lms_loop < n; ++lms_loop)
  {
    lms_phase_shift();
    // LMS wheel change will take about 1-2 secs
    taskDelay(200);
  }
}

void
ti_usetrg(int n)
{
    tiDisableTSInput(TI_TSINPUT_ALL);
    taskDelay(10);
    tiEnableTSInput(n);
    trigger_type = n;
    printf("Changed trigger to 0x%x.\n", n);
    taskDelay(50);
}


void
tsleep(int n)
{
#ifdef VXWORKS
  taskDelay ((sysClkRateGet() / NTICKS) * n);
#else
#endif
}



extern struct TI_A24RegStruct *TIp;
static int ti_slave_fiber_port = 1;

void
titest1()
{
  if(TIp==NULL) {printf("NULL\n");return;}
  printf("0x%08x(%d) 0x%08x(%d)\n",
		 vmeRead32(&TIp->fiberLatencyMeasurement),vmeRead32(&TIp->fiberLatencyMeasurement),
		 vmeRead32(&TIp->fiberAlignment),vmeRead32(&TIp->fiberAlignment));
}

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
  int i1, i2, i3;
  char *ch, tmp[64];

#ifdef POLLING_MODE
  rol->poll = 1;
#else
  rol->poll = 0;
#endif

  printf("\n>>>>>>>>>>>>>>> ROCID=%d, CLASSID=%d <<<<<<<<<<<<<<<<\n",rol->pid,rol->classid);
  printf("CONFFILE >%s<\n\n",rol->confFile);
  printf("LAST COMPILED: %s %s\n", __DATE__, __TIME__);

  printf("USRSTRING >%s<\n\n",rol->usrString);

  /* if slave, get fiber port number from user string */
#ifdef TI_SLAVE
  ti_slave_fiber_port = 1; /* default */

  ch = strstr(rol->usrString,"fp=");
  if(ch != NULL)
  {
    strcpy(tmp,ch+strlen("fp="));
    printf("tmp >>>>>>>>>>>>>>>>>>>>>%s<<<<<<<<<<<<<<<<<<<<<\n",tmp);
    ti_slave_fiber_port = atoi(tmp);
    printf("ti_slave_fiber_port =%d\n",ti_slave_fiber_port);
    tiSetFiberIn_preInit(ti_slave_fiber_port);
  }
#endif

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
  tiSetTriggerWindow(14);	// (7+1)*4ns trigger it coincidence time to form trigger type
  /*
  tiSetTriggerHoldoff(1,20,1);
  tiSetTriggerHoldoff(2,0,1);
  tiSetTriggerHoldoff(3,0,1);
  tiSetTriggerHoldoff(4,0,1);
  */
//  tiAddSlave(7);
//  tiAddSlave(8);
  
vmeBusUnlock();
#endif


  /*********************************************************/
  /*********************************************************/

#ifdef USE_PRAD
  /* (width + 3)*4 ns, 300 ns width */
vmeBusLock();
  tiSetPromptTriggerWidth(60);
  int ii;
  for(ii = 1; ii <= 6; ++ii)
    tiSetTSInputDelay(ii, 0);
vmeBusUnlock();
#endif


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




#ifdef USE_PRAD

vmeBusLock();

 dsc2Init(0x400000,0x80000,5,0/*1<<19*/);
  ndsc2 = dsc2GetNdsc();
  printf("!!!!!!!!!!! ndsc2=%d\n",ndsc2);
  if(ndsc2>0)
  {
    DSC2_READ_CONF_FILE;
    ndsc2_daq = dsc2GetNdsc_daq();
    printf("!!!!!!!!!!! ndsc2_daq=%d\n",ndsc2_daq);
  }
 printf("Set Gate Source... \n");
 dsc2SetGateSource(9, 1, 1);
 //dsc2SetGateSource(9, 2, 3);
/*
  dsc2Init(0x480000,0x80000,5,1<<19);
  int dsc_ch, dsc_sl;
  for(dsc_ch = 0; dsc_ch < 6; ++dsc_ch)
      dsc2SetThreshold(0, dsc_ch, 100, TDCTRG);
  for(dsc_sl = 1; dsc_sl < 5; ++dsc_sl) {
      dsc2SetDelay(dsc_sl, 0, 3);
      for(dsc_ch = 0; dsc_ch < 16; ++dsc_ch)
          dsc2SetThreshold(dsc_sl, dsc_ch, 10, TDCTRG);
  }
  dsc2SetChannelMask(0,0xffff,TDCTRG);
  dsc2SetChannelORMask(0,0xffff,TDCTRG);
  dsc2Status(0,0);
*/
vmeBusUnlock();

#ifdef USE_V767
  /*****************
   *  V767 SETUP
   *****************/
/*vmeBusLock();*/
  v767Init(V767_ADDR1,V767_OFF,1,0);
  // turn off test mode
  v767WriteMicro(0, 0x0200);
  /* select stop trigger matching, substraction of trigger time,
     all channels on, autoload on, and save configuration */
//  v767SetAcqMode(0,0);
//  v767SetDReadyMode(0,0); 
  /*     v767BusErrEnable(0); */
  /*     v767SetBLKEndMode(0, 1);  */
  /*     v767AutoloadDisable(itdc); */
  /*     v767AddrRotary(itdc); */
//  v767SetEdgeDetectionMode(0,1);
  /*     v767OverlapTrigDisable(itdc); */
  v767TriggerConfig(0,2000,1000,0);
  /*     v767ChannelMask(itdc,channelmask);  */
  v767Status(0, 0, 0);
//  v767TriggerConfig(0,0,0,0); /* Print current trigger config */
  printf("v767 8\n");fflush(stdout);
/*vmeBusUnlock();*/
  /*     v767SaveConfig(itdc); */
#endif

#ifdef USE_V1190
vmeBusLock();
    tdc1190Init(V1190_ADDR1, V1190_OFF, 1, 0);
    TDC_READ_CONF_FILE;
vmeBusUnlock();
#endif

#endif /* USE_PRAD */




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



#ifdef USE_PRAD

#ifdef USE_V767
vmeBusLock();
  v767Clear(0);
  v767Status(0, 0, 0);
vmeBusUnlock();
#endif

#ifdef USE_V1190
vmeBusLock();
  tdc1190Clear(0);
  tdc1190Status(0);
vmeBusUnlock();
#endif
  /*if(nfadc>0)*/
  {
    printf("Set BUSY from FP for FASTBUS\n");
vmeBusLock();
    tiSetBusySource(TI_BUSY_FP, 0);
vmeBusUnlock();
  }
#endif


  /* USER code here */
  /******************/




vmeBusLock();
  tiIntDisable();
vmeBusUnlock();


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

  /*
  printf("holdoff rule 1 set to %d\n",tiGetTriggerHoldoff(1));
  printf("holdoff rule 2 set to %d\n",tiGetTriggerHoldoff(2));
  */

#endif

/* set block level in all boards where it is needed;
   it will overwrite any previous block level settings */

#ifdef TI_SLAVE /* assume that for master and standalone TIs block level is set from config file */
vmeBusLock();
  tiSetBlockLevel(block_level);
vmeBusUnlock();
#endif

#ifdef USE_V1190
  tdc1190SetBLTEventNumber(0, block_level);
#endif

vmeBusLock();
  tdc1190Status(0);
vmeBusUnlock();

#ifdef EVENT_CYCLE
  event_count = 0;
#endif

vmeBusLock();
 printf("TISTATUS 1\n");
  tiStatus(1);
vmeBusUnlock();

vmeBusLock();
  ti_usetrg(0);
  tiSetTriggerSource(TI_TRIGGER_PULSER);
//  ti_usetrg(TRG_TOTALSUM);
//  tiSetInputPrescale(5, 3);
vmeBusUnlock();

vmeBusLock();
 printf("TISTATUS 2\n");
  tiStatus(1);
vmeBusUnlock();

  printf("INFO: Prestart1 Executed\n");fflush(stdout);

  *(rol->nevents) = 0;
  rol->recNb = 0;

  return;
}       


static void
__go()
{
  int ii, jj, id, slot;

  logMsg("INFO: Entering Go 1\n",1,2,3,4,5,6);

#ifndef TI_SLAVE
  /* set sync event interval (in blocks) */
vmeBusLock();
 tiSetSyncEventInterval(100000/*10000*//*block_level*/);

 printf("TI: Sync Event Block-Interval = %d\n",
	tiGetSyncEventInterval());

 vmeBusUnlock();
#endif

 

#ifdef USE_PRAD
  /* Enable modules, if needed, here */
#ifdef USE_V767
vmeBusLock();
  v767Clear(0);
vmeBusUnlock();
#endif

#ifdef USE_V1190
vmeBusLock();
  tdc1190Clear(0);
vmeBusUnlock();
#endif
  /* DCS2
  dsc2LatchScalers(0,1);
  */
#endif

  tiStatus(1);

  /* always clear exceptions */
  jlabgefClearException(1);

  nusertrig = 0;
  ndone = 0;

  CDOENABLE(TIPRIMARY,TIR_SOURCE,0); /* bryan has (,1,1) ... */

  tiSoftTrig(1,1,0x448C,1);
  logMsg("INFO: Go 1 Executed\n",1,2,3,4,5,6);
}


static void
__end()
{
  int iwait=0;
  int blocksLeft=0;
  int id;

  tiSoftTrig(1,0,0x1123,1);


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



vmeBusLock();
  tiStatus(1);
vmeBusUnlock();

#ifdef EVENT_CYCLE
  //reset lms phase
  printf("INFO: trying to reset lms phase.");
vmeBusLock();
  lms_phase_change((6 + 1 - lms_phase)%6);
vmeBusUnlock();
#endif

  printf("INFO: End1 Executed\n\n\n");fflush(stdout);

  return;
}


static void
__pause()
{
  CDODISABLE(TIPRIMARY,TIR_SOURCE,0);
  logMsg("INFO: Pause Executed\n",1,2,3,4,5,6);
  
} /*end pause */





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

#ifdef USE_PRAD
  int islot;
  int stat, dCnt, idata;
  unsigned int sync_or_unbuff;
  unsigned int *dma_dabufp;
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
    BANKOPEN(0xe10A,1,rol->pid);
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

	  
      for(jj=0; jj<len; jj++) *rol->dabufp++ = tdcbuf[jj];
	  
    }
    *rol->dabufp++ = LSWAP((lms_phase<<16) | trigger_type);
/*
    *rol->dabufp++ = LSWAP(tiGetScalar(1,0));
    *rol->dabufp++ = LSWAP(tiGetScalar(2,0));
    *rol->dabufp++ = LSWAP(tiGetScalar(3,0));
    *rol->dabufp++ = LSWAP(tiGetScalar(4,0));
    *rol->dabufp++ = LSWAP(tiGetScalar(5,0));
    *rol->dabufp++ = LSWAP(tiGetScalar(6,0));
*/
    BANKCLOSE;

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







#ifdef USE_PRAD

#ifdef USE_V767

    BANKOPEN(0xe121,1,rol->pid);
    dma_dabufp = rol->dabufp;


vmeBusLock();
    stat = v767Dready(0);
vmeBusUnlock();

    jj = 0;
    while( (jj<100) && (stat==0) )
    {
vmeBusLock();
      stat = v767Dready(0);
vmeBusUnlock();
      jj++;
    }

    if(stat)
    {
vmeBusLock();
      dCnt = v767ReadEvent(0, dma_dabufp, 256);
vmeBusUnlock();
      if(dCnt <= 0) /*Error*/
	  {
        *dma_dabufp++ = LSWAP(0xed767bad);
        printf("v767(%d): No data or error.  dCnt = %d\n", ii, dCnt);
      }
      else
      {
        dma_dabufp += dCnt;
        /*printf("Read: %d\n", dCnt);*/
      }
    }
    else
    {
      printf("v767(%d): Timeout. Data not ready.\n", ii);
    }


    rol->dabufp = dma_dabufp;
    BANKCLOSE;

#endif

#ifdef USE_V1190
    int icheck = 0, maxcheck = 200, nev = 0;

    for(icheck = 0; icheck < maxcheck; icheck++)
    {
      nev = tdc1190Dready(0);
      if(nev > 0) break;
    }

    if(nev > 0) 
    {
      
      vmeBusLock();
      rlen = tdc1190ReadBoard(0, tdcbuf);
      vmeBusUnlock();
      
      if(rlen > 0) 
	  {
	  BANKOPEN(0xe121,1,rol->pid);

	    for(jj=0; jj<rlen; jj++)
	    {
	      *rol->dabufp++ = LSWAP(tdcbuf[jj]);
#ifdef V1190_TEST
	      if(!((*rol->nevents)%1000)) 
		  {
		    printf("Read %d words. \n", rlen);
		    if((tdcbuf[jj]&0xf8000000) == 0)
		      printf("channel %d, value %d\n", (tdcbuf[jj] >> 18)&0x7f, (tdcbuf[jj]&0x3ffff));
		  }
#endif
	    }
	    BANKCLOSE;
	  }
    }
    else
    {
      printf("%s: ERROR.  No data ready in TDC1190\n",
	     __FUNCTION__);
    }


#endif /* USE_V1190 */


#endif /* USE_FASTBUS */



#if 1
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

      nwords = 7; /* UPDATE THAT IF THE NUMBER OF WORDS CHANGED BELOW !!! */
      *rol->dabufp ++ = LSWAP((0x14<<27)+nwords); /*head data*/
      *rol->dabufp ++ = 0; /*version  number */
      *rol->dabufp ++ = LSWAP(RUN_NUMBER); /*run  number */
      *rol->dabufp ++ = LSWAP(event_number); /*event number */
      if(ii==(block_level-1))
      {
        *rol->dabufp ++ = LSWAP(time(0)); /*event unix time */
        *rol->dabufp ++ = LSWAP(EVTYPE); /*event type */
        *rol->dabufp ++ = LSWAP(livetime); /* livetime */
	  }
      else
	  {
        *rol->dabufp ++ = 0;
        *rol->dabufp ++ = 0;
        *rol->dabufp ++ = 0;
	  }
    }

    nwords = ((int)rol->dabufp-(int)dabufp1)/4+1;

    *rol->dabufp ++ = LSWAP((0x11<<27)+nwords); /*block trailer*/

    BANKCLOSE;

#endif
#endif



#ifndef VXWORKS
TIMERL_STOP(100000/block_level,1000+rol->pid);
#endif





#if 1 /* enable/disable sync events processing */


    /* read boards configurations */
    if(syncFlag == 1 || EVENT_NUMBER==1)
    {
      printf("SYNC: read boards configurations\n");

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

      printf("SYNC: read boards configurations - done\n");
    }
#endif







#if 1
    /* read scaler(s) */
//    if(syncFlag==1 || EVENT_NUMBER==1)
    {
#ifdef USE_DSC2
	  printf("ndsc2_daq=%d\n",ndsc2_daq);fflush(stdout);
	  if(ndsc2_daq>0)
	  {
        BANKOPEN(0xe115,1,rol->pid);
        for(jj=0; jj<1/*ndsc2_daq*/; jj++)
        {
          slot = dsc2Slot_daq(jj);
          printf("dsc2: jj=%d slot=%d\n",jj,slot);
vmeBusLock();
          /* in following argument 4 set to 0xFF means latch and read everything, 0x3F - do not latch and read everything */
          nwords = dsc2ReadScalers(slot, tdcbuf, 0x10000, 0xFF, 1);
          //printf("dsc2: nwords=%d, nwords = 0x%08x 0x%08x 0x%08x 0x%08x\n",nwords,tdcbuf[0],tdcbuf[1],tdcbuf[2],tdcbuf[3]);
          if(nwords > 70)
          printf("dsc2: trigger counts = %08d %08d %08d %08d\n",tdcbuf[35],tdcbuf[36],tdcbuf[37],tdcbuf[38]);
vmeBusUnlock();

          /* unlike other boards, dcs2 scaler readout already swapped in 'dsc2ReadScalers', so swap it back, because
          hps2.c expects big-endian format*/
          for(ii=0; ii<nwords; ii++) *rol->dabufp ++ = LSWAP(tdcbuf[ii]);
        }
        BANKCLOSE;
	  }

#endif
	}
#endif



















#if 1

#ifndef TI_SLAVE

    /* print livetite */
    if(syncFlag==1)
	{
      printf("SYNC: livetime\n");fflush(stdout);
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
      printf("SYNC: livetime - done\n");
	}
#endif

#endif




#if 1

    /* for physics sync event, make sure all board buffers are empty */
    if(syncFlag==1)
    {
      printf("SYNC: make sure all board buffers are empty\n");fflush(stdout);

      int nblocks;
      nblocks = tiGetNumberOfBlocksInBuffer();
      /*printf(" Blocks ready for readout: %d\n\n",nblocks);*/

      if(nblocks)
	  {
        printf("SYNC ERROR: TI nblocks = %d\n",nblocks);fflush(stdout);
        sleep(10);
	  }
      
      /*check if anything left in TDC event buffer; if yes, print warning message and clear event buffer*/
      int nev = tdc1190Dready(0);
      if(nev > 0) 
	  {
	    printf("WARN: v1290[%2d] has %d events - clear it\n",jj,nev);
	    tdc1190Clear(0);
	  }
      
      printf("SYNC: make sure all board buffers are empty - done\n");
    }


#endif





  }

  /* close event */
  CECLOSE;

  /*
  nusertrig ++;
  printf("usrtrig called %d times\n",nusertrig);fflush(stdout);
  */
#ifdef EVENT_CYCLE

  /*printf("START EVENT CYCLE\n");fflush(stdout);*/

vmeBusLock();

  ++event_count;

  switch(trigger_type)
  {
  default:
  case TRG_LGSUM:
  case TRG_TOTALSUM:
  case TRG_TAGGERE:
  case TRG_SCIN:
    break;
  case TRG_LMSLED:
    if(event_count > 10000) {
      lms_phase_shift();
      ti_usetrg(TRG_LMSALPHA);
      event_count = 0;
    }
    break;
  case TRG_LMSALPHA:
    if(event_count > 10000) {
      ti_usetrg(TRG_PRODUCTION);
      event_count = 0;
    }
    break;
  }
vmeBusUnlock();

  /*printf("END EVENT CYCLE\n");fflush(stdout);*/

#endif

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
  tiSoftTrig(1,1,0x448C,1);


  return;
}

static void
__status()
{
  return;
}  

#else

void
pgem2_dummy()
{
  return;
}

#endif
