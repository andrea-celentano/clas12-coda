
/* svt1.c */

#if defined(VXWORKS) || defined(Linux_vme)

#define USE_VSCM 

/* clas12 svt - linux version */

#ifndef VXWORKS
#define DMA_TO_BIGBUF /*if want to dma directly to the big buffers*/
#endif

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

#include "circbuf.h"

/* from fputil.h */
#define SYNC_FLAG 0x20000000

/* readout list name */
#define ROL_NAME__ "SVT1"

/* polling mode if needed */
#define POLLING_MODE

/* main TI board */
#define TI_ADDR   (21<<19)  /* if 0 - default will be used, assuming slot 21*/


/* name used by loader */
#define INIT_NAME svt1__init
#define TI_READOUT TI_READOUT_EXT_POLL /* Poll for available data, front panel triggers */

#include "rol.h"

void usrtrig(unsigned int EVTYPE, unsigned int EVSOURCE);
void usrtrig_done();

#include "TIPRIMARY_source.h"


/* user code */

#include "uthbook.h"
#include "coda.h"
#include "tt.h"

#include "vscmLib.h"


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


/*
int
blablasvt()
{
  char *filename;
  int id, beg_chip, end_chip, beg_chan, end_chan, start_thr, chan_mult;

  fssrGainScanM(id, filename,
              beg_chip, end_chip,
              beg_chan, end_chan,
				start_thr, chan_mult);
}
*/


static unsigned int vscmSlotMask = 0; /* bit=slot (starting from 0) */
static int nvscm1 = 0; /* the number of vscm boards found by vscmInit() */
static int VSCM_ROFLAG = 1;  /* 0-noDMA, 1-board-by-board DMA, 2-chainedDMA */


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



/* for the calculation of maximum data words in the block transfer */
static unsigned int MAXFADCWORDS = 100000;
static unsigned int MAXTIDWORDS  = 0;


static void
__download()
{
  int ii, islot, i1, i2, i3;
#ifdef POLLING_MODE
  rol->poll = 1;
#else
  rol->poll = 0;
#endif

  printf("\n>>>>>>>>>>>>>>> ROCID=%d, CLASSID=%d <<<<<<<<<<<<<<<<\n\n",rol->pid,rol->classid);

  printf("rol1: downloading DDL table ...\n");
  clonbanks_();
  printf("rol1: ... done.\n");

  /**/
  CTRIGINIT;

  /* initialize OS windows and TI board */
#ifdef VXWORKS
  CDOINIT(TIPRIMARY);
#else
  CDOINIT(TIPRIMARY,TIR_SOURCE);
#endif



  /* rewrite some TIPRIMARY settings*/

  block_level = 1;
  tiSetBlockLevel(block_level);
tiSetBlockBufferLevel(1);


goto a123;
/*tiLoadTriggerTableSVT();*/
  tiLoadTriggerTable(0);
  tiSetTriggerSource(TI_TRIGGER_TSINPUTS);  
  tiDisableTSInput(TI_TSINPUT_ALL);
  tiEnableTSInput( TI_TSINPUT_2 );
 a123:



  usrVmeDmaSetConfig(2,5,1); /*A32,2eSST,267MB/s*/
  /*usrVmeDmaSetConfig(2,5,0);*/ /*A32,2eSST,160MB/s*/
  /*usrVmeDmaSetConfig(2,3,0);*/ /*A32,MBLT*/
  /*usrVmeDmaSetConfig(2,2,0);*/ /*A32,BLT*/

  tdcbuf = (unsigned int *)i2_from_rol1;


#ifdef USE_VSCM
  printf("\n\n\n");
#ifndef VXWORKS
  vmeSetQuietFlag(1); /* skip the errors associated with BUS Errors */
#endif
  nvscm1 = vscmInit((unsigned int)(3<<19),(1<<19),20,0);
#ifndef VXWORKS
  vmeSetQuietFlag(0); /* Turn the error statements back on */
#endif
  printf("\n\n\n");

  if(VSCM_ROFLAG==2 && nvscm1==1) VSCM_ROFLAG = 1; /*no chainedDMA if one board only*/
  /*
  if(VSCM_ROFLAG==2) faEnableMultiBlock(1);
  else faDisableMultiBlock();
  */

  vscmSlotMask=0;
  for(ii=0; ii<nvscm1; ii++)
  {
    islot = vscmSlot(ii);      /* Grab the current module's slot number */
    vscmSlotMask |= (1<<islot); /* Add it to the mask */
    vscmSetBlockLevel(islot, block_level);
  }
#endif


  /***************************************
   *   SD SETUP
   ***************************************/
  sdInit(1);   /* Initialize the SD library */
  sdSetActiveVmeSlots(vscmSlotMask); /* Use the vscmSlotMask to configure the SD */
  printf("SD slot mask = 0x%08x\n",vscmSlotMask);
  sdStatus();


  logMsg("INFO: User Download Executed\n",1,2,3,4,5,6);
}

static void
__prestart()
{
  int ii, i1, i2, i3;
  int islot, isl, ichan;
  unsigned short iflag;
  int iFlag = 0;
  int ich;
  unsigned short aa = 0;
  unsigned short bb;
  unsigned short thr = 400;

  /* Clear some global variables etc for a clean start */
  *(rol->nevents) = 0;

  tiEnableVXSSignals();

#ifdef POLLING_MODE
  CTRIGRSS(TIPRIMARY, TIR_SOURCE, usrtrig, usrtrig_done);
#else
  CTRIGRSA(TIPRIMARY, TIR_SOURCE, usrtrig, usrtrig_done);
#endif

  sprintf(rcname,"RC%02d",rol->pid);
  printf("rcname >%4.4s<\n",rcname);


#ifdef USE_VSCM
  printf("vscmPrestart ...\n"); fflush(stdout);
  vscmPrestart("VSCMConfig.txt");
  printf("vscmPrestart done\n"); fflush(stdout);
#endif



  tiIntDisable();

  /* master and standalone crates, NOT slave */
#ifndef TI_SLAVE
  sleep(1);
  tiSyncReset(1);
  sleep(1);

  printf("holdoff rule 1 set to %d\n",tiGetTriggerHoldoff(1));
  printf("holdoff rule 2 set to %d\n",tiGetTriggerHoldoff(2));

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

  CDODISABLE(TIPRIMARY,TIR_SOURCE,0);

  /* Before disconnecting... wait for blocks to be emptied */
  blocksLeft = tiBReady();
  if(blocksLeft)
  {
    printf("... end:  Blocks left on the TI (%d)\n",blocksLeft);
    while(iwait < 100)
	{
	  if(blocksLeft <= 0) break;
	  blocksLeft = tiBReady();
	  iwait++;
	}
    printf("... end:  Blocks left on the TI (%d)\n",blocksLeft);
  }


  tiStatus(1);

  logMsg("INFO: User End Executed\n",1,2,3,4,5,6);

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
  int ii, jj, islot;

  logMsg("INFO: Entering Go 1\n",1,2,3,4,5,6);





  /* disable BUSY 
tiSetBusySource(TI_BUSY_LOOPBACK,1);
*/  




#ifdef USE_VSCM
  for(ii=0; ii<nvscm1; ii++)
  {
    for(jj=0; jj<8; jj++)
    {
      fssrSCR(vscmSlot(ii), jj);
	}
  }

  /* Reset the Token */
  if(VSCM_ROFLAG==2)
  {
	for(ii=0; ii<nvscm1; ii++)
    {
	  islot = vscmSlot(ii);
      vscmResetToken(islot);
	}
  }

	for(ii=0; ii<nvscm1; ii++)
    {
	  islot = vscmSlot(ii);
	  vscmStat(islot);
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

  
  CDOENABLE(TIPRIMARY,TIR_SOURCE,0);
  

  logMsg("INFO: Go 1 Executed\n",1,2,3,4,5,6);
}

void
usrtrig(unsigned int EVTYPE, unsigned int EVSOURCE)
{
  int *jw, ind, ind2, islot, i, ii, jj, iii, blen, len, rlen, itdcbuf, dCnt, stat, gbready, itime;
  unsigned int mask, *tdcbuf_save, *tdc;
  unsigned int *dabufp1, *dabufp2;
#ifndef VXWORKS
  TIMERL_VAR;
#endif
#ifdef DMA_TO_BIGBUF
  unsigned int pMemBase, uMemBase, mSize;
#endif

  /*printf("EVTYPE=%d syncFlag=%d\n",EVTYPE,syncFlag);*/

  rol->dabufp = (int *) 0;

  CEOPEN(EVTYPE, BT_BANKS);

#ifdef USE_BOS
  jw = rol->dabufp; 
  jw[-2] = 1;
#endif

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

#ifdef USE_BOS
    /* for EVIO format, will dump raw data */
    if((ind = bosMopen_(jw, rcname, 0, 1, 0)) <=0)
    {
      printf("bosMopen_ Error: %d\n",ind);
    }
    rol->dabufp += NHEAD;
#endif

    tdcbuf_save = tdcbuf;

    /*************/
	/* TI stuff */

    /* Set high, the first output port */
    tiSetOutputPort(1,0,0,0);

    /* Grab the data from the TI */
    len = tiReadBlock(tdcbuf,900>>2,1);
    if(len<=0)
    {
      printf("TIreadout : No data or error, len = %d\n",len);
    }
    else
    {
	  ;
#ifdef DEBUG
      printf("ti: len=%d\n",len);
      for(jj=0; jj<len; jj++) printf("ti[%2d] 0x%08x\n",jj,LSWAP(tdcbuf[jj]));
#endif
      /* *rol->dabufp++ = LSWAP(tdcbuf[jj]);*/

    }

    /* Turn off all output ports */
    tiSetOutputPort(0,0,0,0);

	/* TI stuff */
    /*************/


    /* Configure Block Type... temp fix for 2eSST trouble with token passing */
    tdcbuf = tdcbuf_save;
    dCnt=0;
    if(nvscm1 != 0)
    {

#ifndef VXWORKS
TIMERL_START;
#endif

/*
 taskDelay(100);
*/

/*COMMENT OUT FOLLOWING 'FOR' LOOP FOR SPEED UP !!!*/
/*
stat=1;
goto a1233;
*/
#ifdef DEBUG
      printf("Calling vscmGBReady ...\n");fflush(stdout);
#endif
      for(itime=0; itime<100000; itime++) 
	  {
	    gbready = vscmGBReady();
	    stat = (gbready == vscmSlotMask);
	    if (stat>0) 
	    {
	      break;
	    }
#ifdef DEBUG
		else
		{
          printf("NOT READY: gbready=0x%08x, expect 0x%08x\n",gbready,vscmSlotMask);
		}
#endif
	  }
a1233:	  

#ifdef DEBUG
	  
	  /* print fifo info */
      printf("FIFOs info:\n");fflush(stdout);
      for(jj=0; jj<nvscm1; jj++)
	  {
        vscmStat(vscmSlot(jj));
	  }
      printf("mask=0x%08x gbready=0x%08x stat=%d\n",vscmSlotMask,gbready,stat);
	  
#endif
	  
      if(stat>0)
	  {
        if(VSCM_ROFLAG==2)
        {
          islot = vscmSlot(0);
#ifdef DMA_TO_BIGBUF
          uMemBase = dabufp_usermembase;
          pMemBase = dabufp_physmembase;
          mSize = 0x100000;
          usrChangeVmeDmaMemory(pMemBase, uMemBase, mSize);
 
          usrVmeDmaMemory(&pMemBase, &uMemBase, &mSize);
 

 
 	      dCnt = vscmReadBlock(islot,rol->dabufp,500000/*MAXFADCWORDS*/,VSCM_ROFLAG);
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
	      dCnt = vscmReadBlock(islot,tdcbuf,500000/*MAXFADCWORDS*/,VSCM_ROFLAG);

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


	        len = vscmReadBlock(vscmSlot(jj),rol->dabufp,1000/*MAXFADCWORDS*/,VSCM_ROFLAG);
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

	        len = vscmReadBlock(vscmSlot(jj),&tdcbuf[dCnt],1000/*MAXFADCWORDS*/,VSCM_ROFLAG);
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
	      printf("VSCM: No data or error.  dCnt = %d (slots from %d)\n",dCnt,islot);
          dCnt=0;
	    }
	    else
	    {
#ifndef DMA_TO_BIGBUF
          for(jj=0; jj<dCnt; jj++) *rol->dabufp++ = tdcbuf[jj];
#endif
	    }
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
	      islot = vscmSlot(ii);
	      vscmResetToken(islot);
	    }
	  }

#ifndef VXWORKS
TIMERL_STOP(100000,1000+rol->pid);
#endif


	}





#ifdef USE_BOS
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
      logMsg("1ERROR: event too long, blen=%d, ind=%d\n",blen,ind,0,0,0,0);
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
#endif

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
svt1_dummy()
{
  return;
}

#endif
