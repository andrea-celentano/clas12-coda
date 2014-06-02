
/* tdc1.c - first readout list for CAEN TDCs */

#if defined(VXWORKS) || defined(Linux_vme)

/*
coda_roc_gef -s clastest -o "tdcecal1 ROC" -i
*/



#undef DMA_TO_BIGBUF /*if want to dma directly to the big buffers*/

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
#define ROL_NAME__ "TDC1"

/* polling mode if needed */


/*sergey*/
/*#ifndef VXWORKS*/
#define POLLING_MODE
/*#endif*/

/* main TI board */
#define TI_ADDR   (21<<19)  /* if 0 - default will be used, assuming slot 21*/


/* name used by loader */
#ifdef TI_MASTER
#define INIT_NAME tdc1_master__init
#define TI_READOUT TI_READOUT_EXT_INT  /* Interrupt for available data, front panel triggers */
#else
#ifdef TI_SLAVE
#define INIT_NAME tdc1_slave__init
#define TI_READOUT TI_READOUT_TS_POLL /* Poll for available data, triggers from master over fiber */
#else
#define INIT_NAME tdc1__init
#define TI_READOUT TI_READOUT_EXT_POLL /* Poll for available data, front panel triggers */
#endif
#endif


#include "rol.h"

void usrtrig(unsigned int EVTYPE, unsigned int EVSOURCE);
void usrtrig_done();


#include "TIPRIMARY_source.h"


/* user code */

#include "uthbook.h"
#include "coda.h"
#include "tt.h"


static char rcname[5];

#define NBOARDS 22    /* maximum number of VME boards: we have 21 boards, but numbering starts from 1 */
#define MY_MAX_EVENT_LENGTH 3000/*3200*/ /* max words per board */
static unsigned int *tdcbuf;


#ifdef USE_V1190
#include "tdc1190.h"
#ifdef SLOTWORKAROUND
static int slotnums[NBOARDS];
#endif
static int tdctypebyslot[NBOARDS];
static int error_flag[NBOARDS];
static int ntdcs;


static unsigned int NBsubtract = 9; /*same for v1190 and v1290 ?*/

#endif

#include "dsc2Lib.h"

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

#define DIST_ADDR  0xEA00	  /*  base address of FADC signal distribution board  (A16)  */

static void
__download()
{
  int ii, slot, jj, i1, i2, i3;
#ifdef POLLING_MODE
  rol->poll = 1;
#else
  rol->poll = 0;
#endif

  printf("\n>>>>>>>>>>>>>>> ROCID=%d, CLASSID=%d <<<<<<<<<<<<<<<<\n",rol->pid,rol->classid);
  printf("CONFFILE >%s<\n\n",rol->confFile);

  
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



/* rewrite some TIPRIMARY settings
tiSetBlockBufferLevel(1);
*/


/* enable BUSY from TDC FANOUT
tiSetBusySource(TI_BUSY_LOOPBACK | TI_BUSY_P2,1);
*/

  /* add tdc fanout as busy source (overwrite TI_BUSY_SWB set by tiInit) */
  /* if ti is busy, it will not send trigger enable over fiber, since it is the same fiber
	 and busy has higher priority */
  tiSetBusySource(TI_BUSY_P2,1);

#ifndef TI_SLAVE
  tiSetBusySource(TI_BUSY_LOOPBACK,0);
#endif

  /*usrVmeDmaSetConfig(2,5,1);*/ /*A32,2eSST,267MB/s*/
  /*usrVmeDmaSetConfig(2,5,0);*/ /*A32,2eSST,160MB/s*/
  usrVmeDmaSetConfig(2,3,0); /*A32,MBLT*/
  /*usrVmeDmaSetConfig(2,2,0);*/ /*A32,BLT*/


  /*bad for ti !!! */
  /*usrVmeDmaSetConfig(1,2,0);*/ /*A24,BLT*/



  tdcbuf = (unsigned int *)i2_from_rol1;


#ifdef USE_V1190
  
  dsc2Init(0x100000,0x80000,16,0);
  dsc2Config("");

  ntdcs = tdc1190Init(0x11100000,0x80000,20,0);
  tdc1190Config("");

  for(ii=0; ii<ntdcs; ii++)
  {
    slot = tdc1190Slot(ii);
    tdctypebyslot[slot] = tdc1190Type(ii);
    printf(">>> id=%d slot=%d type=%d\n",ii,slot,tdctypebyslot[slot]);
  }


#endif






#ifndef TI_SLAVE
  /*pulser*/
  v851Init(0xd000,0);
  /*v851SetDelay(1,10,1,0);*/
  v851_start(100);
  /*pulser*/
#endif




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



  sleep(1);
  tiStatus(1);
  sleep(2);

  tiIntDisable();


#ifdef USE_V1190

  /* dsc2 configuration */
  dsc2Config("");

#ifdef SLOTWORKAROUND
  for(ii=0; ii<ntdcs; ii++)
  {
    slot = tdc1190GetGeoAddress(ii);
	slotnums[ii] = slot;
    printf("[%d] slot %d\n",ii,slotnums[ii]);
  }
#endif

  for(ii=0; ii<ntdcs; ii++)
  {
    tdc1190Clear(ii);
    error_flag[ii] = 0;
  }
#endif


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
  
#ifdef USE_V1190
  for(ii=0; ii<ntdcs; ii++)
  {
    tdc1190Clear(ii);
    error_flag[ii] = 0;
  }
  taskDelay(100);
#endif

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
  int zerochan[22], reftdc, nwtdc;
#ifdef SLOTWORKAROUND
  unsigned long tdcslot_h, tdcslot_t, remember_h;
#endif
#endif
  /*
  printf("EVTYPE=%d syncFlag=%d\n",EVTYPE,syncFlag);
  */
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


	len = tiReadBlock(tdcbuf,900>>2,0); /* 1-DMA, 0-noDMA*/
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

    /* Turn off all output ports */
    tiSetOutputPort(0,0,0,0);

	/* TI stuff */
    /*************/


#ifndef VXWORKS
TIMERL_START;
#endif


#ifdef USE_V1190
	if(ntdcs>0)
	{
	  
      tdc1190ReadStart(tdcbuf, rlenbuf);
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
	}

    for(jj=0; jj<22; jj++) zerochan[jj] = -1;


    /* analysis */
    reftdc = 0;
    nwtdc = 0;

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
      logMsg("jj=%d, rlen=%d\n",jj,rlen,3,4,5,6);
#endif

	  /*	  
#ifdef DEBUG
      level = tdc1190GetAlmostFullLevel(jj);
      ii = tdc1190StatusAlmostFull(jj);
      logMsg("jj=%d, rlen=%d, almostfull=%d level=%d\n",jj,rlen,ii,level,5,6);
#endif
	  */	  

      if(rlen <= 0) continue;

      tdc = &tdcbuf[itdcbuf];
      itdcbuf += rlen;

#ifndef VXWORKS
      for(ii=0; ii<rlen; ii++) tdc[ii] = LSWAP(tdc[ii]);
#endif

      for(ii=0; ii<rlen; ii++)
      {
		
        if( ((tdc[ii]>>27)&0x1F) == 0)
        {
          nbcount ++;

/*#ifdef DEBUG*/
          tdcedge = ((tdc[ii]>>26)&0x1);
		  /*printf("tdcslot=%d tdctypebyslot=%d\n",tdcslot,tdctypebyslot[tdcslot]);*/
          if( tdctypebyslot[tdcslot] == 2 ) /*1290*/
          {
            tdcchan = ((tdc[ii]>>21)&0x1F);
            tdcval = (tdc[ii]&0x1FFFFF);
          }
          else /*1190*/
          {
            tdcchan = ((tdc[ii]>>19)&0x7F);
            tdcval = (tdc[ii]&0x7FFFF);
			/*if(tdcchan==63) {reftdc=1; printf("[ev %d] befor: 0x%08x\n",*(rol->nevents),tdcval);}*/
            tdcval = tdcval<<2; /*report 1190 value with same resolution as 1290*/
			/*if(tdcchan==63) printf("[ev %d] after: 0x%08x (0x%08x)\n",*(rol->nevents),tdcval,tdcval&0x7FFFF);*/
		  }

		  /*
          *rol->dabufp ++ = (tdc[ii] | (tdcslot<<27));
		  */
          /* output data in 'standard' format: slot[27-31]  edge[26]  chan[19-25]  tdc[0-18] */
          *rol->dabufp ++ = (tdcslot<<27) + (tdcedge<<26) + (tdcchan<<19) + (tdcval&0x7FFFF);

		  if(0/*tdcchan<2*/)
          {
            logMsg(" DATA: type=%1d slot=%02d tdc=%02d ch=%03d edge=%01d value=%08d\n",
				   tdctypebyslot[tdcslot],tdcslot,(int)tdc14,(int)tdcchan,(int)tdcedge,(int)tdcval);
		  }
/*#endif*/
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

          nwtdc = (tdc[ii]>>5)&0x3FFF;
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
		  if( !(error_flag[tdcslot]%100) ) /* do not print for every event */
          {
            unsigned int ddd, lock, ntdc;
			
            logMsg(" ERR: event# %7d, slot# %2d, tdc# %02d, error_flags=0x%08x, err=0x%04x, lock=0x%04x\n",
              tdceventcount,tdcslot,(int)tdc14,(int)tdcerrorflags,ddd,lock);
			
		  }
          error_flag[tdcslot] ++;
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

	/*
    printf("nwtdc=%d nbcount=%d\n",nwtdc,nbcount);
    if(reftdc==0) printf("[%d] ERROR: no reference !!!\n",*(rol->nevents));
	*/

#ifndef VXWORKS
TIMERL_STOP(100000,1000+rol->pid);
#endif



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
      logMsg(": %d %d 0x%x 0x%x\n",blen,ind,(UINT32)dabufp1,(UINT32)dabufp2,0,0);
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
tdc1_dummy()
{
  return;
}

#endif
