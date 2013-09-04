


#define DMA_TO_BIGBUF /*if want to dma directly to the big buffers*/

/* if event rate goes higher then 10kHz, with randon triggers we have wrong
slot number reported in GLOBAL HEADER and/or GLOBAL TRAILER words; to work
around that problem temporary patches were applied - until fixed (Sergey) */
#define SLOTWORKAROUND

/* tid1.c - first readout list for VME crates with new TID */

#define USE_FADC250




#undef DEBUG


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/time.h>
typedef      long long       hrtime_t;

#include "circbuf.h"

/* from fputil.h */
#define SYNC_FLAG 0x20000000

/* readout list name */
#define ROL_NAME__ "TID1"

/* polling mode if needed */
#define POLLING_MODE

/* main TI board */
#define TID_ADDR   (21<<19)  /* if 0 - default will be used, assuming slot 21*/


/* name used by loader */
#ifdef TID_MASTER
#define INIT_NAME tid1_master__init
#define TID_READOUT TID_READOUT_EXT_POLL /* Poll for available data, front panel triggers */
#else
#ifdef TID_SLAVE
#define INIT_NAME tid1_slave__init
#define TID_READOUT TID_READOUT_TS_POLL /* Poll for available data, triggers from master over fiber */
#else
#define INIT_NAME tid1__init
#define TID_READOUT TID_READOUT_EXT_POLL /* Poll for available data, front panel triggers */
#endif
#endif

#include "rol.h"

void usrtrig(unsigned int EVTYPE, unsigned int EVSOURCE);
void usrtrig_done();

#include "TIDPRIMARY_source.h"



/* user code */

#include "uthbook.h"
#include "coda.h"
#include "tt.h"
#include "scaler7201.h"
#include "adc792.h"
#include "tdc1190.h"

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








void
tsleep(int n)
{
#ifdef VXWORKS
  taskDelay ((sysClkRateGet() / NTICKS) * n);
#else
#endif
}



extern struct TID_A24RegStruct *TIDp;

void
tidtest1()
{
  if(TIDp==NULL) {printf("NULL\n");return;}
  printf("0x%08x(%d) 0x%08x(%d)\n",
		 vmeRead32(&TIDp->fiberLatencyMeasurement),vmeRead32(&TIDp->fiberLatencyMeasurement),
		 vmeRead32(&TIDp->fiberSyncAlignment),vmeRead32(&TIDp->fiberSyncAlignment));
}



#ifdef USE_FADC250

/* on dvcs2: goto '$CODA/src/rol' amd type 'make urols' */
/* FX TAGGED HERE */
/*1-raw,2-pulse,3-pulse integral*/
#ifdef TID_MASTER
static int fadc_mode = 1;
#else
static int fadc_mode = 3;
#endif

/*ctp cluster coincidence window*/
static unsigned int cluster_window = 1; /*0: 4 ns, 1: 8ns, ..., max 32ns - coincidence window on FADC hits */

/*FADC trigger reporting sum scale factor*/
static unsigned int fa_sumScaleFactor = 7; /*lsb will be: 0-20fC, 1-40fC, 2-80, 3-160, ...: range is 5 bits (0-31); Scale Factor for the total sum reported to the CTP i.e. Sum/(2^sumScaleFactor) (<15)*/

void
changeFadcMode()
{
  if(fadc_mode==3)
  {
    fadc_mode=1;
    printf("set fadc_mode to %d (raw window mode)\n",fadc_mode);
  }
  else
  {
    fadc_mode=3;
    printf("set fadc_mode to %d (pulse integral mode)\n",fadc_mode);
  }
}

void
changeClusterWindow()
{
  cluster_window += 1;
  if(cluster_window>7) cluster_window=0;
  printf("set CTP cluster_window to %d (%d ns)\n",cluster_window,(cluster_window+1)*4);
}

void
changeScaleFactor()
{
  fa_sumScaleFactor += 1;
  if(fa_sumScaleFactor>15) fa_sumScaleFactor=0;
  printf("set FADC trigger reporting sum scale factor to %d (%d fC)\n",fa_sumScaleFactor,(fa_sumScaleFactor+1)*20);
}

#include "fadcLib.h"

#define DIST_ADDR  0xEA00	  /*  base address of FADC signal distribution board  (A16)  */

#define FADC_WINDOW_LAT      825/*850*/  /* Trigger Window Latency */
#define FADC_WINDOW_WIDTH    100/*100*/  /* Trigger Window Width */
#define FADC_DAC_LEVEL      3300  /* Internal DAC Level */
#define FADC_THRESHOLD       10  /* Threshold for data readout */


#ifdef TID_MASTER
int fadc_window_lat = FADC_WINDOW_LAT;
#else
int fadc_window_lat = /*955*/FADC_WINDOW_LAT;
#endif



unsigned int fadcSlotMask   = 0;    /* bit=slot (starting from 0) */
extern   int fadcA32Base;           /* This will need to be reset from it's default
                                     * so that it does not overlap with the TID */
extern   int nfadc;                 /* Number of FADC250s verified with the library */
extern   int fadcID[FA_MAX_BOARDS]; /* Array of slot numbers, discovered by the library */
static int NFADC;                   /* The Maximum number of tries the library will
                                     * use before giving up finding FADC250s */
static int FA_SLOT;                 /* We'll use this over and over again to provide
				                     * us access to the current FADC slot number */ 

static int FADC_ROFLAG           = 2;  /* 0-noDMA, 1-board-by-board DMA, 2-chainedDMA */

/* for the calculation of maximum data words in the block transfer */
static unsigned int MAXFADCWORDS = 0;
static unsigned int MAXTIDWORDS  = 0;


void
abc123(int par)
{
  int islot;
  fadc_window_lat = par;
  for(islot=0; islot<NFADC; islot++) 
  {
    FA_SLOT = fadcID[islot];      /* Grab the current module's slot number */
    faSetProcMode(FA_SLOT,1,fadc_window_lat,FADC_WINDOW_WIDTH,3,6,1,0);
  }
  return;
}


/**/

/* IC lookup tables */

/*tdc's in slots 19 and 20*/
int ic_tdc_high[2][128] = {
0*256
};

int ic_tdc_low[2][128] = {
0*256
};



/*adc's in slots 3-10 and 13-18*/

unsigned int ic_adc_ped[19][16];
unsigned short ic_adc_thres[19][16];
unsigned int offset;


int
abcReadPeds(int rocid)
{
  FILE *fd = NULL;
  char *dir = NULL;
  char *expid = NULL;
  char fname[1024];
  int slot, chan, ped, status;
  float sigma;


  printf("Opening pedestal file\n");
  if((dir=getenv("CLAS")) == NULL)
  {
    printf("ERROR: environment variable CLAS is not defined - exit\n");
    return(-1);
  }
  if((expid=getenv("EXPID")) == NULL)
  {
    printf("ERROR: environment variable EXPID is not defined - exit\n");
    return(-1);
  }
  sprintf(fname,"%s/parms/peds/%s/fadc%02d.ped",dir,expid,rocid);
  fd = fopen(fname,"r");
  if(fd==NULL)
  {
    printf("ttfa: ERROR: cannot open pedestal file >%s<\n",fname);
  }
  else
  {
    printf("ttfa: pedestal file >%s< is opened for writing\n",fname);


    for(slot=0; slot<19; slot++)
    {
      for(chan=0; chan<16; chan++)
      {
        ic_adc_ped[slot][chan] = 0;
        ic_adc_thres[slot][chan] = 0;
      }
    }

    while( (status=fscanf(fd,"%d %d %d %f %d\n",&slot,&chan,&ped,&sigma,&offset)) > 0 )
    {
      printf("status=%d -> slot=%2d chan=%2d ped=%5d sigma=%6.3f\n",status,slot,chan,ped,sigma);
      if(slot>=0&&slot<19&&chan>=0&&chan<16)
	  {
        /* set trigger threshold */
        ic_adc_ped[slot][chan] = ped + offset;

        /* set readout threshold as pedestal + some offset */
        ic_adc_thres[slot][chan] = ped + 50; /* was 50 until May 8 12:30pm*/
	  }
      else printf("bad slot=%d or chan=%d\n",slot,chan);
    }

    if(status==EOF) printf("EOF reached\n");
    else            printf("fscanf() returned error %d\n",status);

    fclose(fd);
    printf("ttfa: pedestal file >%s< is closed\n",fname);
  }

  return(0);
}

#endif

static void
__download()
{
  int i1, i2, i3;
#ifdef USE_FADC250

  int islot, isl, ichan;
  unsigned short iflag;
  int iFlag = 0;
  int ich;


  /*ctp params*/
  unsigned int cluster_threshold = 0; /* 8 bits: (MeV) cluster energy sum threshold */

  /*ssp params*/
  unsigned int ssp_addr_a24 = 0xBE0000;

  unsigned int coincidence_window = 15; /*4: +/-16ns time window for cluster pairs */

  unsigned int energy_sum_max = 255; /*255: no cut (cut on pair)*/
  unsigned int pair_energy_difference_max = 255; /*255: no cut*/
  unsigned int pair_energy_difference_threshold = 255; /*255: no cut*/
  unsigned int energy_max = 255; /*255: no cut (cut on individual cluster)*/
  unsigned int energy_min = 10/*10*/; /*not cut (cut on individual cluster)*/
  unsigned int cluster_delay = 128; /*128: 512ns delay critical parameter for internal algorithm to work*/
  unsigned int trigger_latency = 511; /*2us delay for sync->trigger out of SSP...real latency to FADC trigger input will be about 3.2us*/

  /*
  0:    will trigger on a cluster pair
  0x10: will trigger on a cluster found in top half of calorimeter
  0x20: will trigger on a cluster found in bottom half of calorimeter
  0x30: will trigger on a cluster found in top OR bottom half of calorimeter
  */
  unsigned int trigger_cut_enable_mask = 0x30;


#endif
#ifdef POLLING_MODE
  rol->poll = 1;
#else
  rol->poll = 0;
#endif

  printf(">>>>>>>>>>>>>>>>>>>>>>>>>> ROCID = %d <<<<<<<<<<<<<<<<\n",rol->pid);

  printf("rol1: downloading DDL table ...\n");
  clonbanks_();
  printf("rol1: ... done.\n");

  /**/
  CTRIGINIT;

  /* initialize OS windows and TI board */
  CDOINIT(TIDPRIMARY,TIR_SOURCE);


  usrVmeDmaSetConfig(2,5,1); /*A32,2eSST,267MB/s*/
  /*usrVmeDmaSetConfig(2,5,0);*/ /*A32,2eSST,160MB/s*/
  /*usrVmeDmaSetConfig(2,3,0);*/ /*A32,MBLT*/

  tdcbuf = (unsigned int *)i2_from_rol1;



  /******************/
  /* USER code here */

#ifdef USE_FADC250

#ifdef TID_SLAVE
  abcReadPeds(2);
#else
  abcReadPeds(1);
#endif


  /* Here, we assume that the addresses of each board were set according
   to their geographical address (slot number):
   * Slot  3:  (3<<19) = 0x00180000
   * Slot  4:  (4<<19) = 0x00200000
   * ...
   * Slot 20: (20<<19) = 0x00A00000
   */

  NFADC = 16+2;   /* 16 slots + 2 (for the switch slots) */
  fadcA32Base=0x09000000; /*NOTE: starting from that address, memory chunks size=FA_MAX_A32_MEM(=0x800000)
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

ANY OTHER BOARDS IN A32 SPACE MUST BE USING ADDRESSES FROM 0x11000000 AND ABOVE !!!
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

  vmeSetQuietFlag(1); /* skip the errors associated with BUS Errors */

  faInit((unsigned int)(3<<19),(1<<19),NFADC,iFlag); /* start from 0x00180000, increment 0x00080000 */

  NFADC=nfadc;        /* Redefine our NFADC with what was found from the driver */
  vmeSetQuietFlag(0); /* Turn the error statements back on */
  
  /* Calculate the maximum number of words per block transfer (assuming Pulse mode)
   *   MAX = NFADC * block_level * (EvHeader + TrigTime*2 + Pulse*2*chan) 
   *         + 2*32 (words for byte alignment) 
   */
  MAXFADCWORDS = NFADC * block_level * (1+2+FADC_WINDOW_WIDTH*16) + 2*32;
  
  printf("**************************************************\n");
  printf("* Calculated MAX FADC words per block = %d\n",MAXFADCWORDS);
  printf("**************************************************\n");

  /* Check these numbers, compared to our buffer size.. */
  if( (MAXFADCWORDS+MAXTIDWORDS)*4 > MAX_EVENT_LENGTH )
  {
    printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    printf(" WARNING.  Event buffer size is smaller than the expected data size\n");
    printf("     Increase the size of MAX_EVENT_LENGTH and recompile!\n");
    printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
  }


  if(NFADC==1) FADC_ROFLAG = 1; /*no chainedDMA if one board only*/

  if(FADC_ROFLAG==2) faEnableMultiBlock(1);
  else faDisableMultiBlock();

  /* Additional Configuration for each module */
  fadcSlotMask=0;
  for(islot=0; islot<NFADC; islot++) 
  {
    FA_SLOT = fadcID[islot];      /* Grab the current module's slot number */
    fadcSlotMask |= (1<<FA_SLOT); /* Add it to the mask */
	printf("=======================> fadcSlotMask=0x%08x\n",fadcSlotMask);


	/*hps1
  Payload port lanes up:  1  2  3  4  5  6  7  8  9 10 11 12 13 
  Payload port lanes down: 14 15 16 
  Payload port channels up:  1  2  3  4  7  8  9 10 11 12 13 
  Payload port channels down:  5  6 14 15 16 
	*/
	/*hps2
  Payload port lanes up:  1  2  3  4  5  6  7  8  9 10 11 12 
  Payload port lanes down: 13 14 15 16 
  Payload port channels up:  1  2  3  4  7  8  9 10 11 12 
  Payload port channels down:  5  6 13 14 15 16 
	*/


    /* Set the internal DAC level */
    faSetDAC(FA_SLOT,FADC_DAC_LEVEL,0);
	faPrintDAC(FA_SLOT);


    /*  Setup option 1 processing - RAW Window Data     <-- */
    /*        option 2            - RAW Pulse Data */
    /*        option 3            - Integral Pulse Data */
    /*  Setup 200 nsec latency (PL  = 50)  */
    /*  Setup  80 nsec Window  (PTW = 20) */
    /*  Setup Pulse widths of 36ns (NSB(3)+NSA(6) = 9)  */
    /*  Setup up to 1 pulse processed */
    /*  Setup for both ADC banks(0 - all channels 0-15) */
    /* Integral Pulse Data */

	/*
       faSetProcMode(
           int id           - 
           int pmode        - 
           unsigned int PL  - 
           unsigned int PTW - 
	       unsigned int NSB - the number of samples before
           unsigned int NSA - the number of samples after
           unsigned int NP  - the maxinum number of pulses will be reported (<=4)
           int bank         - 
                    )
	*/

	if(fadc_mode==1)
	{
      /* Set the threshold for data readout */
      faSetThreshold(FA_SLOT,FADC_THRESHOLD,0);
	  /* fadc_mode=1: raw window mode */
      faSetProcMode(FA_SLOT,1,fadc_window_lat,FADC_WINDOW_WIDTH,3,6,1,0);
	}
    else
	{
      /*fadc_mode=2-pulse modes, fadc_mode=3-integral mode*/
      faSetThresholdAll(FA_SLOT,ic_adc_thres[FA_SLOT]);
      faSetProcMode(FA_SLOT,fadc_mode,fadc_window_lat,FADC_WINDOW_WIDTH,5,30,3,0);
	}

    /* Bus errors to terminate block transfers (preferred) */
    faEnableBusError(FA_SLOT);


    /*set threshold for pulse integration in trigger*/
    faWriteHPSConfig(FA_SLOT, 1000, 0x4 | (0x6<<4));


    /* Set the Block level */
    faSetBlockLevel(FA_SLOT, block_level);







	/*****************/
	/*trigger-related*/

    /* Set the individual channel pedestals for the data that is sent to the CTP */
    for(ichan=0; ichan<16; ichan++)
	{
	  faSetChannelPedestal(FA_SLOT,ichan,ic_adc_ped[FA_SLOT][ichan]);
	}


    {
      /* trigger thresholds: was 80; 50 from May 8 11:30am*/
      unsigned int fa_tot = 80; /* 1V / 4096chan = 0.25mV/chan -> 20 means 5mV: Time Over Threshold value for integration window Value (<4095)*/


      unsigned int fa_maxIntTime = 7; /*The Maximum Integration time (in 16 ns steps) (<8) */


      faSetHPSParameters(FA_SLOT, fa_tot, fa_maxIntTime, fa_sumScaleFactor);
    }
	
	/*****************/
	/*****************/

  }


  /* read back and print trigger pedestals */
  printf("\n\nTrigegr pedestals readback\n");
  for(islot=0; islot<NFADC; islot++) 
  {
    FA_SLOT = fadcID[islot];
    for(ichan=0; ichan<16; ichan++)
	{
      printf("  slot=%2d chan=%2d ped=%5d\n",FA_SLOT,ichan,faGetChannelPedestal(FA_SLOT, ichan));
    }
  }
  printf("\n\n");




  /*  Enable FADC - new place */
  for(islot=0; islot<NFADC; islot++) 
  {
    FA_SLOT = fadcID[islot];
    faChanDisable(FA_SLOT,0x0);
  }


  /*
  goto a567;
  */
#ifdef TID_MASTER
  /*before 14_may_2012_1pm*/
  /*faChanDisable(18,0x20);*/ /* 15 2 */
  /*faChanDisable(17,0x4);*/ /* 8 2 */

  /*after 14_may_2012_2:30pm*/
faChanDisable(17,0x4);
faChanDisable(16,0x0020);
#else
  /* manual disable hot channels */
  /*before 14_may_2012_1pm*/
  /*faChanDisable(16,0x8000);*/ /* x=*4, y=-3 */
  /*faChanDisable(18,0x0208);*/ /* x=14, y=-4 */
  /*faChanDisable( 5,0x0431);*/ /* x=12, y=-4 */
  /*faChanDisable(17,0x0007);*/ /* x=7, y=-4 and x=8, y=-2 */
  /*faChanDisable(6,0x77f8);*/ /*x=6, y=-4 and around (seems whole connector) */

  /*after 14_may_2012_2:30pm*/
faChanDisable(17,0x0001);
faChanDisable(6,0x37f8);
faChanDisable(16,0x8000);

  /*after 17 may 2012 11:00pm*/
faChanDisable(19,0x0128);

#endif
 a567:


  /***************************************
   *   SD SETUP
   ***************************************/
  sdInit();   /* Initialize the SD library */
  sdSetActiveVmeSlots(fadcSlotMask); /* Use the fadcSlotMask to configure the SD */
  sdStatus();


  /*****************
   *   CTP SETUP
   *****************/
  ctpInit();

  ctpSetVmeSlotEnableMask(fadcSlotMask);



  /*debugging
#ifdef TID_MASTER
  ctpSetVmeSlotEnableMask(0x20); slot 5 only
#else
  ctpSetVmeSlotEnableMask(0x2000); slot 13 only
#endif
  debugging */


  ctpSetClusterMinimumThreshold(cluster_threshold);
  ctpSetClusterWindow(cluster_window);

  ctpStatus();

  int iwait=0;
  int allchanup=0;
  while(allchanup  != (0x7) )
  {
    iwait++;
    allchanup = ctpGetAllChanUp();
    if(iwait>1000)
	{
	  printf("iwait timeout   allchup - 0x%x\n",allchanup);
	  break;
	}
  }



#ifndef TID_SLAVE

  /*****************
   *   SSP SETUP
   *****************/

  sspInit_HPS(ssp_addr_a24);
  sspSetCoincidence(coincidence_window);
  sspSetEnergySumMaximum(energy_sum_max);
  sspSetPairEnergyDifferenceMaximum(pair_energy_difference_max);
  sspSetPairEnergyDistanceThreshold(pair_energy_difference_threshold);
  sspSetEnergyMaximum(energy_max);
  sspSetEnergyMinimum(energy_min);
  sspSetClusterDelay(cluster_delay);
  sspSetTriggerLatency(trigger_latency);
  sspSetTriggerCutEnableMask(trigger_cut_enable_mask);

#endif


#endif


  /*
  vmeServer("hps1");
  */


  logMsg("INFO: User Download Executed\n",1,2,3,4,5,6);
}

static void
__prestart()
{
  int ii, i1, i2, i3;
#ifdef USE_FADC250
  int islot, isl, ichan;
  unsigned short iflag;
  int iFlag = 0;
  int ich;
  unsigned short aa = 0;
  unsigned short bb;
  unsigned short thr = 400;
#endif

  /* Clear some global variables etc for a clean start */
  *(rol->nevents) = 0;

#ifdef POLLING_MODE
  CTRIGRSS(TIDPRIMARY, TIR_SOURCE, usrtrig, usrtrig_done);
#else
  CTRIGRSA(TIDPRIMARY, TIR_SOURCE, usrtrig, usrtrig_done);
#endif

  sprintf(rcname,"RC%02d",rol->pid);
  printf("rcname >%4.4s<\n",rcname);




/*!!!!!!!!!!!!!!!!!!!!!!!!!!*/
#ifdef TID_MASTER

  printf("rocMask = 0x%08x\n",rocMask);fflush(stdout);
  printf("rocMask = 0x%08x\n",rocMask);fflush(stdout);
  printf("rocMask = 0x%08x\n",rocMask);fflush(stdout);
  printf("rocMask = 0x%08x\n",rocMask);fflush(stdout);
  printf("rocMask = 0x%08x\n",rocMask);fflush(stdout);
  printf("rocMask = 0x%08x\n",rocMask);fflush(stdout);

  /* assume that id=2(hps2) is on fiber 5, id=3(hpstracker) is on fiber 6, id=4(svt) is on fiber 7 */ 
  
  if(rocMask & 0x04)
  {
    printf("enable fiber 5\n");fflush(stdout);
    printf("enable fiber 5\n");fflush(stdout);
    printf("enable fiber 5\n");fflush(stdout);
    printf("enable fiber 5\n");fflush(stdout);
    printf("enable fiber 5\n");fflush(stdout);
    printf("enable fiber 5\n");fflush(stdout);
    tidAddSlave(5);
  }
  if(rocMask & 0x08)
  {
    printf("enable fiber 6\n");fflush(stdout);
    printf("enable fiber 6\n");fflush(stdout);
    printf("enable fiber 6\n");fflush(stdout);
    printf("enable fiber 6\n");fflush(stdout);
    printf("enable fiber 6\n");fflush(stdout);
    printf("enable fiber 6\n");fflush(stdout);
    tidAddSlave(6);
  }
  if(rocMask & 0x10)
  {
    printf("enable fiber 7\n");fflush(stdout);
    printf("enable fiber 7\n");fflush(stdout);
    printf("enable fiber 7\n");fflush(stdout);
    printf("enable fiber 7\n");fflush(stdout);
    printf("enable fiber 7\n");fflush(stdout);
    printf("enable fiber 7\n");fflush(stdout);
    tidAddSlave(7);
  }
  

#endif




/* sergey: old 1*/
/*If the TI Master, send a Clock and Trig Link Reset */
#ifndef TID_SLAVE
  tidClockReset();
  sleep(2);
  tidTrigLinkReset();
  sleep(2);
#endif



#ifdef USE_FADC250

  /* Program/Init VME Modules Here */
  for(islot=0; islot<NFADC; islot++)
  {
    FA_SLOT = fadcID[islot];
    faSetClockSource(FA_SLOT,2);
  }

  sleep(1);

  for(islot=0; islot<NFADC; islot++)
  {
    FA_SLOT = fadcID[islot];
    faSoftReset(FA_SLOT);
    faResetToken(FA_SLOT);
    faResetTriggerCount(FA_SLOT);
    faStatus(FA_SLOT,0);
    faPrintThreshold(FA_SLOT);
  }

  tidStatus();

  /*  Enable FADC */
  for(islot=0; islot<NFADC; islot++) 
  {
    FA_SLOT = fadcID[islot];
    /*faSetMGTTestMode(FA_SLOT,0);*/
    faSetBlockLevel(FA_SLOT,block_level); /* done above !!??*/
    /*faChanDisable(FA_SLOT,0xffff);enabled in download*/
    faEnable(FA_SLOT,0,0);
  }


  sleep(2);

  /* do following after the FADC clocks have been set to P0 */
  ctpFiberLinkReset();


  ctpStatus();

#endif

  /* USER code here */
  /******************/

  tidIntDisable();

  /*sergey: old 2*/
#ifndef TID_SLAVE
  printf("Disable all inputs\n");
  sleep(1);
  tidSyncReset();
  sleep(2);

  printf("holdoff rule 1 set to %d\n",tidGetTriggerHoldoff(1));
  printf("holdoff rule 2 set to %d\n",tidGetTriggerHoldoff(2));

#endif


#ifdef TID_MASTER
  sspSetTriggerWidth(8000); /*7000-28us, 8000-32us, 10000-40us, 12000-48us, 16000-64us*/
  /*6250=25us svt runs, 6000=24us brakes SVT completely, need power recycle; 7000-chash on high rate (10kHz)*/ 

  sspClearEventFifo();
  sspSetEventPatternDelay(210); /*to place ssp trigger info into readout window*/
  sspPrintEventPatternDelay();
#endif


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
  int islot;

  CDODISABLE(TIDPRIMARY,TIR_SOURCE,0);

  /* Before disconnecting... wait for blocks to be emptied */
  blocksLeft = tidBReady();
  if(blocksLeft)
  {
    printf("... end:  Blocks left on the TID (%d)\n",blocksLeft);
    while(iwait < 100)
	{
	  if(blocksLeft <= 0) break;
	  blocksLeft = tidBReady();
	  iwait++;
	}
    printf("... end:  Blocks left on the TID (%d)\n",blocksLeft);
  }


#ifdef USE_FADC250
  /* FADC Disable */
  for(islot=0;islot<NFADC;islot++) 
  {
    FA_SLOT = fadcID[islot];
    faDisable(FA_SLOT,0);
    faStatus(FA_SLOT,0);
  }
  sdStatus();
#endif

  tidStatus();

  logMsg("INFO: User End Executed\n",1,2,3,4,5,6);

  return;
}

static void
__pause()
{
  CDODISABLE(TIDPRIMARY,TIR_SOURCE,0);
  logMsg("INFO: Pause Executed\n",1,2,3,4,5,6);
  
} /*end pause */

static void
__go()
{
  int jj, islot;

  logMsg("INFO: Entering Go 1\n",1,2,3,4,5,6);


#ifdef USE_FADC250


  /*  Enable FADC - old place
  for(islot=0; islot<NFADC; islot++) 
  {
    FA_SLOT = fadcID[islot];
    faChanDisable(FA_SLOT,0x0);
  }
  sleep(1);
  */

  
  /*  Send Sync Reset to FADC */
  /*faSDC_Sync();*/
#endif

  CDOENABLE(TIDPRIMARY,TIR_SOURCE,0); /* bryan has (,1,1) ... */

  logMsg("INFO: Go 1 Executed\n",1,2,3,4,5,6);
}

#define NSSP 8

void
usrtrig(unsigned int EVTYPE, unsigned int EVSOURCE)
{
  int *jw, ind, ind2, i, ii, jj, jjj, iii, blen, len, rlen, itdcbuf, nbytes;
  unsigned int *tdcbuf_save, *tdc;
  unsigned int *dabufp1, *dabufp2, sspbuf[NSSP];
  TIMERL_VAR;
#ifdef USE_FADC250
  unsigned int datascan, mask;
  int nwords, njjloops;
  unsigned short *dabufp16, *dabufp16_save;
  int islot;
  int dCnt, idata;
  int stat, itime, gbready;
#endif
#ifdef DMA_TO_BIGBUF
    unsigned int pMemBase, uMemBase, mSize;
#endif

  /*printf("EVTYPE=%d syncFlag=%d\n",EVTYPE,syncFlag);*/

  rol->dabufp = (int *) 0;

  CEOPEN(EVTYPE, BT_BANKS/*BT_UI4*/);

  jw = rol->dabufp; 

  /***/

  /* at that moment only second CODA world defined  */
  /* first CODA world (length) undefined, so set it */
  /*jw[ILEN] = 1;*/ jw[-2] = 1;

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
    if((ind = bosMopen_(jw, rcname, 0, 1, 0)) <=0)
    {
      printf("bosMopen_ Error: %d\n",ind);
    }
    rol->dabufp += NHEAD;

    tdcbuf_save = tdcbuf;

    /*************/
	/* TID stuff */

    /* Set high, the first output port */
    tidSetOutputPort(1,0,0,0);

    /* Grab the data from the TID */
    len = tidReadBlock(tdcbuf,900>>2,1);
    if(len<=0)
    {
      /*printf("TIDreadout : No data or error, len = %d\n",len)*/;
    }
    else
    {
	  ;
	  /*
      printf("tid: len=%d\n",len);
      for(jj=0; jj<len; jj++) printf("tid[%2d] 0x%08x\n",jj,LSWAP(tdcbuf[jj]));
	  */
      /* *rol->dabufp++ = LSWAP(tdcbuf[jj]);*/

    }

    /* Turn off all output ports */
    tidSetOutputPort(0,0,0,0);

	/* TID stuff */
    /*************/





#ifdef USE_FADC250

    /* Configure Block Type... temp fix for 2eSST trouble with token passing */
    tdcbuf = tdcbuf_save;
    dCnt=0;
    if(NFADC != 0)
    {

TIMERL_START;


/*COMMENT OUT FOLLOWING 'FOR' LOOP FOR SPEED UP !!!*/
      for(itime=0; itime<100000; itime++) 
	  {
	    gbready = faGBready();
	    stat = (gbready == fadcSlotMask);
	    if (stat>0) 
	    {
	      break;
	    }
	  }










#ifdef TID_MASTER
	nwords = sspReadFifo(sspbuf);	
    sspbuf[7] = time(0);
	/*
    printf("sspbuf[%2d]: 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
      nwords,sspbuf[0],sspbuf[1],sspbuf[2],sspbuf[3],sspbuf[4],sspbuf[5],sspbuf[6]);
	*/
#endif




      if(stat>0)
	  {
        FA_SLOT = fadcID[0];
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
 

#ifdef TID_MASTER
          for(jjj=0; jjj<NSSP; jjj++) *rol->dabufp++ = sspbuf[jjj];
#endif

 
 /*25us->*/
 	      dCnt = faReadBlock(FA_SLOT,rol->dabufp,500000/*MAXFADCWORDS*/,FADC_ROFLAG);
          rol->dabufp += dCnt;
 /*->25us*/
		  /*  
 		  printf("dCnt=%d, data: 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",dCnt,
 		  rol->dabufp[0],rol->dabufp[1],rol->dabufp[2],rol->dabufp[3],rol->dabufp[4],rol->dabufp[5]);
 		  */
          usrRestoreVmeDmaMemory();
          usrVmeDmaMemory(&pMemBase, &uMemBase, &mSize);
 	      /*printf("restored: 0x%08x 0x%08x 0x%08x\n",pMemBase,uMemBase,mSize);*/
#else
	      dCnt = faReadBlock(FA_SLOT,tdcbuf,500000/*MAXFADCWORDS*/,FADC_ROFLAG);
#endif
        }
        else
		{
          for(jj=0; jj<NFADC; jj++)
		  {
#ifdef DMA_TO_BIGBUF

            uMemBase = dabufp_usermembase;
            pMemBase = dabufp_physmembase;
            mSize = 0x100000;
            usrChangeVmeDmaMemory(pMemBase, uMemBase, mSize);

#ifdef TID_MASTER
            /* report ssp data before first fadc board */
            if(jj==0) for(jjj=0; jjj<NSSP; jjj++) *rol->dabufp++ = sspbuf[jjj];
#endif
	        len = faReadBlock(fadcID[jj],rol->dabufp,10000/*MAXFADCWORDS*/,FADC_ROFLAG);
            rol->dabufp += len;
            dCnt += len;

            usrRestoreVmeDmaMemory();
#else
	        len = faReadBlock(fadcID[jj],&tdcbuf[dCnt],10000/*MAXFADCWORDS*/,FADC_ROFLAG);
            dCnt += len;
#endif
#ifdef DEBUG
            printf("[%d] len=%d dCnt=%d\n",jj,len,dCnt);
#endif
		  }
	    }

	    if(dCnt<=0)
	    {
	      printf("FADCs: No data or error.  dCnt = %d (slots from %d)\n",dCnt,FA_SLOT);
          dCnt=0;
	    }
	    else
	    {
#ifndef DMA_TO_BIGBUF
#ifdef TID_MASTER
          for(jjj=0; jjj<NSSP; jjj++) *rol->dabufp++ = sspbuf[jjj];
#endif
          for(jj=0; jj<dCnt; jj++) *rol->dabufp++ = tdcbuf[jj];
#endif
	    }
	  }
      else 
	  {
	    printf ("FADCs: no events   stat=%d  intcount = %d   gbready = 0x%08x  fadcSlotMask = 0x%08x\n",
		  stat,tidGetIntCount(),gbready,fadcSlotMask);
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
	    for(islot=0; islot<NFADC; islot++)
	    {
	      FA_SLOT = fadcID[islot];
	      faResetToken(FA_SLOT);
	    }
/*->2us*/
	  }

TIMERL_STOP(100000/block_level,1000+rol->pid);


    }

#endif




    blen = rol->dabufp - (int *)&jw[ind+1];
#ifdef DEBUG
    printf("rol1trig: len=%d\n",blen);
#endif
    if(blen == 0) /* no data - return pointer to the initial position */
    {
      rol->dabufp -= NHEAD;
	  /* NEED TO CLOSE BANK !!?? */
    }
    else if(blen >= (MAX_EVENT_LENGTH/4))
    {
      logMsg("1ERROR: event too long, blen=%d, ind=%d (MAX_EVENT_LENGTH/4=%d)\n",blen,ind,MAX_EVENT_LENGTH/4,0,0,0);
      logMsg(": %d %d 0x%x 0x%x\n",blen,ind,dabufp1,dabufp2,0,0);
      tsleep(1); /* 1 = 0.01 sec */
    }
    else if(bosMclose_(jw,ind,1,blen) <= 0)
    {
      logMsg("2ERROR in bosMclose_ - space is not enough !!!\n",1,2,3,4,5,6);
    }


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



  }

  /* close event */
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

  /* Acknowledge tir register */
  CDOACK(TIDPRIMARY,TIR_SOURCE,0);

  return;
}

static void
__status()
{
  return;
}  
