
/* dcrb2.c - first readout list for DCRB (Intel) */

#if defined(VXWORKS) || defined(Linux_vme)

#ifndef VXWORKS
#define DMA_TO_BIGBUF /*if want to dma directly to the big buffers*/
#endif

/* if event rate goes higher then 10kHz, with random triggers we have wrong
slot number reported in GLOBAL HEADER and/or GLOBAL TRAILER words; to work
around that problem temporary patches were applied - until fixed (Sergey) */
#define SLOTWORKAROUND


#define USE_V1190

  
#undef DEBUG

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#ifndef VXWORKS
#include <sys/types.h>
#include <sys/time.h>
/*
typedef      long long       hrtime_t;
*/
#endif

#include "circbuf.h"

/* from fputil.h */
#define SYNC_FLAG 0x20000000

/* readout list name */
#define ROL_NAME__ "DCRB2"

/* polling mode if needed */
#ifndef VXWORKS
#define POLLING_MODE
#endif

/* main TI board */
#define TI_ADDR   (21<<19)  /* if 0 - default will be used, assuming slot 21*/


/* name used by loader */
#define INIT_NAME dcrb2__init

#ifdef VXWORKS
#define TI_READOUT TI_READOUT_EXT_INT  /* Interrupt for available data, front panel triggers */
#else
#define TI_READOUT TI_READOUT_EXT_POLL /* Poll for available data, front panel triggers */
#endif

#include "rol.h"

void usrtrig(unsigned int EVTYPE, unsigned int EVSOURCE);
void usrtrig_done();


#include "TIPRIMARY_source.h"


/* user code */

#include "uthbook.h"
#include "coda.h"
#include "tt.h"

#include "dcrbLib.h"

extern int ndcrb;                     /* Number of DCRBs in Crate */
extern int dcrbID[DC_MAX_BOARDS];     /* slot numbers */
static int DCRB_SLOT, islot;
static int DCRB_ROFLAG = 1;           /* 0-noDMA, 1-board-by-board DMA, 2-chainedDMA */

static char rcname[5];

#define NBOARDS 22    /* maximum number of VME boards: we have 21 boards, but numbering starts from 1 */
#define MY_MAX_EVENT_LENGTH 3000/*3200*/ /* max words per board */
static unsigned int *tdcbuf;


#ifdef USE_V1190
#include "tdc1190.h"
#ifdef SLOTWORKAROUND
static int slotnums[NBOARDS];
#endif
static unsigned int NBsubtract = 9;
static int error_flag[NBOARDS];
static int ntdcs;
#endif


/*#ifdef DMA_TO_BIGBUF*/
/* must be 'rol' members, like dabufp */
#ifndef VXWORKS
extern unsigned int dabufp_usermembase;
extern unsigned int dabufp_physmembase;
#endif
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




/*TEMPORARY!!!*/
void
svt2getdmabuffer()
{
  return;
}



void
tsleep(int n)
{
#ifdef VXWORKS
  taskDelay ((sysClkRateGet() / NTICKS) * n);
#else
#endif
}



/*
extern struct TI_A24RegStruct *TIp;
void
titest1()
{
  if(TIp==NULL) {printf("NULL\n");return;}
  printf("0x%08x(%d) 0x%08x(%d)\n",
		 vmeRead32(&TIp->fiberLatencyMeasurement),vmeRead32(&TIp->fiberLatencyMeasurement),
		 vmeRead32(&TIp->fiberAlignment),vmeRead32(&TIp->fiberAlignment));
}
*/


#define DIST_ADDR  0xEA00	  /*  base address of FADC signal distribution board  (A16)  */


/* for the calculation of maximum data words in the block transfer */
static unsigned int MAXDCRBWORDS = 10000;
static unsigned int MAXTIDWORDS  = 0;

static unsigned int dcrbSlotMask; /* bit=slot (starting from 0) */


static void
__download()
{
  int i1, i2, i3, id, ii;
#ifdef POLLING_MODE
  rol->poll = 1;
#else
  rol->poll = 0;
#endif

  printf(">>>>>>>>>>>>>>>>>>>>>>>>>> ROCID = %d <<<<<<<<<<<<<<<<\n",rol->pid);

  printf("rol1: downloading DDL table ...\n");
  clonbanks_();
  printf("rol1: ... done.\n");


/*???
tidClockReset();
sleep(1);
*/

  /**/
  CTRIGINIT;

  /* initialize OS windows and TI board */
#ifdef VXWORKS
  /*CDOINIT(TIPRIMARY);*/
  CDOINIT(TIPRIMARY);
#else
  CDOINIT(TIPRIMARY,TIR_SOURCE);
#endif



  /* rewrite some TIPRIMARY settings*/
  

 tiSetBusySource(TI_BUSY_LOOPBACK | TI_BUSY_SWA,1);
tiSetBlockBufferLevel(1);



goto a123;
  tiLoadTriggerTable(0);
  tiSetTriggerSource(TI_TRIGGER_TSINPUTS);  
  tiDisableTSInput(TI_TSINPUT_ALL);
  tiEnableTSInput( TI_TSINPUT_2 );
 a123:



  /*usrVmeDmaSetConfig(2,5,1);*/ /*A32,2eSST,267MB/s*/
  /*usrVmeDmaSetConfig(2,5,0);*/ /*A32,2eSST,160MB/s*/
  usrVmeDmaSetConfig(2,3,0); /*A32,MBLT*/
  /*usrVmeDmaSetConfig(2,2,0);*/ /*A32,BLT*/

  tdcbuf = (unsigned int *)i2_from_rol1;

  dcInit((4<<19), 0x80000, 16, 7); /* 7 boards from slot 4, 7 boards from slot 13 */
  printf("dcinit finished\n");fflush(stdout);

  dcGSetDAC(20/*10*/); /* threshold in mV */
  dcGSetCalMask(0,0x3f);
  dcGSetProcMode(4000/*2000*/,4000/*2000*/,32);

  if(ndcrb==1) DCRB_ROFLAG = 1; /*no chainedDMA if one board only*/
  if(DCRB_ROFLAG==2) dcEnableMultiBlock(1);
  else dcDisableMultiBlock();

  /* Additional Configuration for each module */
  dcrbSlotMask=0;
  for(islot=0; islot<ndcrb; islot++) 
  {
    DCRB_SLOT = dcrbID[islot];      /* Grab the current module's slot number */
    dcrbSlotMask |= (1<<DCRB_SLOT); /* Add it to the mask */
	printf("=======================> dcrbSlotMask=0x%08x\n",dcrbSlotMask);

  }


  /* DCRB stuff */

  for(id=0; id<ndcrb; id++) 
  {
    DCRB_SLOT = dcrbID[id];
    dcrbTriggerPulseWidth(DCRB_SLOT, 8000);
	dcrbLinkReset(DCRB_SLOT);

    /* will try to reset 5 times */
    for(ii=0; ii<5; ii++)
	{
      if(dcrbLinkStatus(DCRB_SLOT)) break;
	  printf("Reseting link at slot %d\n",DCRB_SLOT);
      dcrbLinkReset(DCRB_SLOT);
	}
  }


  for(id=0; id<ndcrb; id++)
  {
    DCRB_SLOT = dcrbID[id];
    if(dcrbLinkStatus(DCRB_SLOT)) printf("Link at slot %d is UP\n",DCRB_SLOT);
    else printf("Link at slot %d is DOWN\n",DCRB_SLOT);
  }


  /***************************************
   *   SD SETUP
   ***************************************/
  sdInit();   /* Initialize the SD library */
  /*sdSetActiveVmeSlots(fadcSlotMask);*/ /* Use the fadcSlotMask to configure the SD */
  sdStatus();



#ifdef USE_V1190
  /* have to call tdc1190Init() before any other calls */
  ntdcs = tdc1190Init(0x11210000,0x10000,20,0);
  if(ntdcs>0)
  {
    tdc1190SetDefaultWindowWidth(3000);
    tdc1190SetDefaultWindowOffset(-2000);
    tdc1190Config(1); /*0-BERR, 1-FIFO*/
  }
#endif





  logMsg("INFO: User Download Executed\n",1,2,3,4,5,6);
}

static void
__prestart()
{
  int ii, i1, i2, i3;
  int islot, isl, ichan, id;
  unsigned short iflag;
  int iFlag = 0;
  int ich;
  unsigned short aa = 0;
  unsigned short bb;
  unsigned short thr = 400;
  unsigned short slot, channel, pattern[8];

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



#ifdef USE_V1190
#ifdef SLOTWORKAROUND
  for(ii=0; ii<ntdcs; ii++)
  {
    slot = tdc1190GetGeoAddress(ii);
	slotnums[ii] = slot;
    printf("[%d] slot %d\n",ii,slotnums[ii]);
    tdc1190SetEdgeDetectionConfig(ii, 2/*3*/); /*1-trailing edge, 2-leading, 3-both*/
  }
#endif
#endif

  tiIntDisable();

  /* master and standalone crates, NOT slave */
#ifndef TI_SLAVE
  printf("Disable all inputs\n");
  sleep(1);
  tiSyncReset();
  sleep(2);
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


  tiStatus();

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



#ifdef USE_V1190
  for(ii=0; ii<ntdcs; ii++)
  {
    tdc1190Clear(ii);
    error_flag[ii] = 0;
  }
#endif


  for(ii=0; ii<ndcrb; ii++)
  {
    DCRB_SLOT = dcrbID[ii];
    dcClear(DCRB_SLOT);
  }


/* disable BUSY
tiSetBusySource(TI_BUSY_LOOPBACK,1);
*/



  
  CDOENABLE(TIPRIMARY,TIR_SOURCE,0);
  

  logMsg("INFO: Go 1 Executed\n",1,2,3,4,5,6);
}

void
usrtrig(unsigned int EVTYPE, unsigned int EVSOURCE)
{
  int *jw, ind, ind2, i, ii, jj, iii, ll, blen, len, rlen, dCnt, stat, itime, gbready;
  unsigned int *tdcbuf_save, mask;
  unsigned int *dabufp1, *dabufp2;
  unsigned short level;
#ifndef VXWORKS
  TIMERL_VAR;
#endif
#ifdef DMA_TO_BIGBUF
  unsigned int pMemBase, uMemBase, mSize;
#endif
#ifdef USE_V1190
  int nev, rlenbuf[22];
  unsigned long tdcslot, tdcchan, tdcval, tdc14, tdcedge, tdceventcount;
  unsigned long tdceventid, tdcbunchid, tdcwordcount, tdcerrorflags;
  unsigned int *tdc, *tdchead, nbsubtract;
  int nheaders, ntrailers, itdcbuf, njjloops, nbcount;
  int zerochan[22];
#ifdef SLOTWORKAROUND
  unsigned long tdcslot_h, tdcslot_t, remember_h;
#endif
#endif

  /*printf("EVTYPE=%d syncFlag=%d\n",EVTYPE,syncFlag);*/

  rol->dabufp = (int *) 0;

  CEOPEN(EVTYPE, BT_BANKS);

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
	/* TI stuff */

    /* Set high, the first output port
    tiSetOutputPort(1,0,0,0);
    */

    /* Grab the data from the TI */
    len = tiReadBlock(tdcbuf,900>>2,0);
    if(len<=0)
    {
      printf("TIreadout : No data or error, len = %d\n",len);
    }
#ifdef DEBUG
    else
    {
      printf("ti: len=%d\n",len);
      for(jj=0; jj<len; jj++) printf("ti[%2d] 0x%08x %d\n",jj,LSWAP(tdcbuf[jj]),LSWAP(tdcbuf[jj]));
    }
#endif

    /* Turn off all output ports
    tiSetOutputPort(0,0,0,0);
    */

	/* TI stuff */
    /*************/



#ifdef USE_V1190
	if(ntdcs>0)
	{
      tdc1190ReadStart(tdcbuf, rlenbuf);
      /*check if anything left in event buffer; if yos, print warning message and clear event buffer*/
      for(jj=0; jj<ntdcs; jj++)
      {
        nev = tdc1190Dready(jj);
        if(nev > 0)
		{
          printf("WARN: v1290[%2d] has %d events - clear it\n",jj,nev);
          tdc1190Clear(jj);
		}
	  }

      /*for(ii=0; ii<rlenbuf[0]; ii++) *rol->dabufp++ = tdcbuf[ii];*/
	  for(ii=0; ii<rlenbuf[0]; ii++) tdcbuf[ii] = LSWAP(tdcbuf[ii]);
	}

    for(jj=0; jj<22; jj++) zerochan[jj] = -1;
        /* analysis */
    itdcbuf = 0;
    nheaders = ntrailers = 0;

    njjloops = ntdcs;
	/*
for(i=0; i<200; i++) tmpgood[i] = tdcbuf[i];
	*/
    for(jj=0; jj<njjloops; jj++)
    {
      rlen = rlenbuf[jj];
	  
#ifdef DEBUG
	  
      level = tdc1190GetAlmostFullLevel(jj);
      ii = tdc1190StatusAlmostFull(jj);
      logMsg("jj=%d, rlen=%d, almostfull=%d level=%d\n",jj,rlen,ii,level,5,6);
	  
#endif
	  
      if(rlen <= 0) continue;

      tdc = &tdcbuf[itdcbuf];
      itdcbuf += rlen;

      for(ii=0; ii<rlen; ii++)
      {
		
        if( ((tdc[ii]>>27)&0x1F) == 0)
        {
          nbcount ++;
*rol->dabufp ++ = (tdc[ii] | (tdcslot<<27));

#ifdef DEBUG
          tdcedge = ((tdc[ii]>>26)&0x1);
          tdcchan = ((tdc[ii]>>19)&0x7F);
          tdcval = (tdc[ii]&0x7FFFF);
		  if(1/*tdcchan<2*/)
          {
            logMsg(" DATA: tdc=%02d ch=%03d edge=%01d value=%08d\n",
              (int)tdc14,(int)tdcchan,(int)tdcedge,(int)tdcval,5,6);
		  }
#endif
        }
        else if( ((tdc[ii]>>27)&0x1F) == 1)
        {
          tdc14 = ((tdc[ii]>>24)&0x3);
          tdceventid = ((tdc[ii]>12)&0xFFF);
          tdcbunchid = (tdc[ii]&0xFFF);
#ifdef DEBUG
          logMsg(" HEAD: tdc=%02d event_id=%05d bunch_id=%05d\n",
            (int)tdc14,(int)tdceventid,(int)tdcbunchid,4,5,6);
#endif
        }
        else if( ((tdc[ii]>>27)&0x1F) == 3)
        {
          tdc14 = ((tdc[ii]>>24)&0x3);
          tdceventid = ((tdc[ii]>12)&0xFFF);
          tdcwordcount = (tdc[ii]&0xFFF);
#ifdef DEBUG
          logMsg(" EOB:  tdc=%02d event_id=%05d word_count=%05d\n",
            (int)tdc14,(int)tdceventid,(int)tdcwordcount,4,5,6);
#endif
        }
        else if( ((tdc[ii]>>27)&0x1F) == 0x18)
        {
#ifdef DEBUG
          logMsg(" FILLER\n",1,2,3,4,5,6);
#endif
        }
        else if( ((tdc[ii]>>27)&0x1F) == 0x11)
        {
          nbsubtract ++;
#ifdef DEBUG
          logMsg(" TIME TAG: %08x\n",(int)(tdc[ii]&0x7ffffff),2,3,4,5,6);
#endif
        }
        else if( ((tdc[ii]>>27)&0x1F) == 8)
        {
          nbcount = 1; /* counter for the number of output words from board */
          nbsubtract = NBsubtract; /* # words to subtract including errors (5 for v1290N, 9 for others) */
          tdcslot = tdc[ii]&0x1F;
          tdceventcount = (tdc[ii]>>5)&0x3FFFFF;
          tdchead = (unsigned int *) rol->dabufp; /* remember pointer */
#ifdef SLOTWORKAROUND
          tdcslot_h = tdcslot;
	      remember_h = tdc[ii];
          if(slotnums[nheaders] != tdcslot) /* correct slot number */
		  {
            if( !((*(rol->nevents))%1000) ) logMsg("WARN: [%2d] slotnums=%d, tdcslot=%d -> use slotnums\n",
				   nheaders,slotnums[nheaders],tdcslot,4,5,6);
            tdcslot = slotnums[nheaders];
		  }
#endif
          *rol->dabufp ++ = tdcslot;
          nheaders++;

		  
#ifdef DEBUG
		  
          logMsg("GLOBAL HEADER: %d words, 0x%08x (slot=%d nevent=%d) -> header=0x%08x\n",
            rlen,tdc[ii],tdc[ii]&0x1F,(tdc[ii]>>5)&0xFFFFFF,*tdchead,6);
		  
#endif
		  
        }
        else if( ((tdc[ii]>>27)&0x1F) == 0x10)
        {
#ifdef SLOTWORKAROUND
          /* double check for slot number */
          tdcslot_t = tdc[ii]&0x1F;
          if(tdcslot_h>21||tdcslot_t>21||tdcslot_h!=tdcslot_t)
          {
            /*logMsg("WARN: slot from header=%d (0x%08x), from trailer=%d (0x%08x), must be %d\n",
			  tdcslot_h,remember_h,tdcslot_t,tdc[ii],tdcslot,6)*/;
          }
#endif
          /* put nwords in header; substract 4 TDC headers, 4 TDC EOBs,
          global trailer and error words if any */
          /**tdchead = (*tdchead) | (((tdc[ii]>>5)&0x3FFF) - nbsubtract);*/
          *tdchead |= ((((tdc[ii]>>5)&0x3FFF) - nbsubtract)<<5);
          if((((tdc[ii]>>5)&0x3FFF) - nbsubtract) != nbcount)
          {			
			
            logMsg("ERROR: word counters: %d != %d (subtract=%d)\n",
			  (((tdc[ii]>>5)&0x3FFF) - nbsubtract),nbcount,nbsubtract,4,5,6);
			
			/*
for(i=0; i<200; i++) tmpbad[i] = tdcbuf[i];
logMsg("tmpgood=0x%08x tmpbad=0x%08x\n",tmpgood,tmpbad,3,4,5,6);
			*/
          }
          ntrailers ++;

#ifdef DEBUG
          logMsg("GLOBAL TRAILER: 0x%08x (slot=%d nw=%d stat=%d) -> header=0x%08x\n",
            tdc[ii],tdc[ii]&0x1F,(tdc[ii]>>5)&0x3FFF,
            (tdc[ii]>>23)&0xF,*tdchead,6);
#endif
        }
        else if( ((tdc[ii]>>27)&0x1F) == 4)
        {
          nbsubtract ++;
          tdc14 = ((tdc[ii]>>24)&0x3);
          tdcerrorflags = (tdc[ii]&0x7FFF);
		  /*
#ifdef DEBUG
		  */
		  if(error_flag[tdcslot] == 0) /* print only once */
          {
            unsigned int ddd, lock, ntdc;
			
            logMsg(" ERR: event# %7d, slot# %2d, tdc# %02d, error_flags=0x%08x, err=0x%04x, lock=0x%04x\n",
              tdceventcount,tdcslot,(int)tdc14,(int)tdcerrorflags,ddd,lock);
			
		  }
          error_flag[tdcslot] = 1;
		  /*
#endif
		  */
        }
        else
        {
#ifdef DEBUG
          logMsg("ERROR: in TDC data format 0x%08x\n",
            (int)tdc[ii],2,3,4,5,6);
#endif
        }

      } /* loop over 'rlen' words */

    }


#endif




#ifndef VXWORKS
TIMERL_START;
#endif


    /* Configure Block Type... temp fix for 2eSST trouble with token passing */
    tdcbuf = tdcbuf_save;
    dCnt=0;
    if(ndcrb != 0)
    {



/*COMMENT OUT FOLLOWING 'FOR' LOOP FOR SPEED UP !!!*/
      for(itime=0; itime<100000; itime++) 
	  {
	    gbready = dcGBready();
	    stat = (gbready == dcrbSlotMask);
	    if (stat>0) 
	    {
	      break;
	    }
	  }




      if(stat>0)
	  {
        DCRB_SLOT = dcrbID[0];
        if(DCRB_ROFLAG==2)
        {
#ifdef DMA_TO_BIGBUF
          uMemBase = dabufp_usermembase;
          pMemBase = dabufp_physmembase;
          mSize = 0x100000;
          usrChangeVmeDmaMemory(pMemBase, uMemBase, mSize);
 
          usrVmeDmaMemory(&pMemBase, &uMemBase, &mSize);
 
 	      dCnt = faReadBlock(DCRB_SLOT,rol->dabufp,MAXDCRBWORDS,DCRB_ROFLAG);
          rol->dabufp += dCnt;
          usrRestoreVmeDmaMemory();
#else
	      dCnt = faReadBlock(DCRB_SLOT,tdcbuf,MAXDCRBWORDS,DCRB_ROFLAG);
#endif
        }
        else
		{
          for(jj=0; jj<ndcrb; jj++)
	      {
            DCRB_SLOT = dcrbID[jj];
	        /*dcPrintScalers(DCRB_SLOT);*/
#ifdef DMA_TO_BIGBUF
            uMemBase = dabufp_usermembase;
            pMemBase = dabufp_physmembase;
            mSize = 0x100000;
            usrChangeVmeDmaMemory(pMemBase, uMemBase, mSize);

            len = dcReadBlock(DCRB_SLOT, rol->dabufp, MAXDCRBWORDS, DCRB_ROFLAG);
#ifdef DEBUG
            printf("DCRB: slot=%d, nw=%d, data-> 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
			  DCRB_SLOT,len,LSWAP(rol->dabufp[0]),LSWAP(rol->dabufp[1]),LSWAP(rol->dabufp[2]),
			  LSWAP(rol->dabufp[3]),LSWAP(rol->dabufp[4]),LSWAP(rol->dabufp[5]),LSWAP(rol->dabufp[6]));
#endif
            rol->dabufp += len;
            dCnt += len;

            usrRestoreVmeDmaMemory();
#else
            len = dcReadBlock(DCRB_SLOT, &tdcbuf[dCnt], MAXDCRBWORDS, DCRB_ROFLAG);
#ifdef DEBUG
            printf("DCRB: slot=%d, nw=%d, data-> 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
			  DCRB_SLOT,len,LSWAP(tdcbuf[dCnt+0]),LSWAP(tdcbuf[dCnt+1]),LSWAP(tdcbuf[dCnt+2]),
              LSWAP(tdcbuf[dCnt+3]),LSWAP(tdcbuf[dCnt+4]),LSWAP(tdcbuf[dCnt+5]),LSWAP(tdcbuf[dCnt+6]));
#endif
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
	    for(islot=0; islot<ndcrb; islot++)
	    {
	      DCRB_SLOT = dcrbID[islot];
	      /*dcResetToken(DCRB_SLOT);not implemented yet !!!!!!!!!!*/
	    }
	  }



    }


#ifndef VXWORKS
TIMERL_STOP(100000,1000+rol->pid);
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
dcrb2_dummy()
{
  return;
}

#endif
