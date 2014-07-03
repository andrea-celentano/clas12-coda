
#ifdef Linux_vme

#include <sys/mman.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "CrateMsgTypes.h"
#include "jvme.h"


#define SCALER_THREAD



#ifdef SCALER_THREAD

#include "dsc2Lib.h"


/********************************************************************************************************
 scalers stuff - allows organize scalers readout by this server with defined interval and report it to
 tcp clients such as CODA or EPICS
********************************************************************************************************/

#ifdef VXWORKS

#define SCALER_LOCK   semTake(vmescalers_lock, WAIT_FOREVER)
#define SCALER_UNLOCK semGive(vmescalers_lock)

#else

#define SCALER_LOCK   pthread_mutex_lock(&vmescalers_lock)
#define SCALER_UNLOCK pthread_mutex_unlock(&vmescalers_lock)

#endif

#define DIST_ADDR  0xEA00	  /*  base address of FADC signal distribution board  (A16)  */

#define MAXTYPES    2   /* the number of different board types */
#define MAXBOARDS  21   /* max number od boards per crate */
#define MAXWORDS  256   /* max number of scaler words per board */

static int vmeScalersReadInterval = 1;
#ifdef VXWORKS
static SEM_ID vmescalers_lock;
#else
static pthread_mutex_t vmescalers_lock;
#endif


static int ndsc2, rflag, rmode;
static int nfadc;
static unsigned int  vmescalerslen[MAXBOARDS];  /*scalers space (the number of words) */
static unsigned int *vmescalers[MAXBOARDS];     /*scalers memory space address*/
static unsigned int i2_from_rol1;

static unsigned int *tdcbuf;


/* parameter 'time' in seconds */
static int
vmeSetScalersReadInterval(int time)
{
  if(time < 0)        vmeScalersReadInterval = 0; /* 0 means no reads */
  else if(time > 100) vmeScalersReadInterval = 100;
  else                vmeScalersReadInterval = time;

  printf("vmeSetScalersReadInterval: vmeScalersReadInterval set to %d\n",vmeScalersReadInterval);

  return(vmeScalersReadInterval);
}

static int
vmeGetScalersReadInterval()
{
  return(vmeScalersReadInterval);
}


static int
vmeScalersRead()
{
  int itype, id, ii, nw, slot;

  SCALER_LOCK;

  for(itype=0; itype<1; itype++)
  {
    /* read dsc2 scalers */
    for(id=0; id<ndsc2; id++)
    {
      slot = dsc2Slot(id);
      nw = dsc2ReadScalers(slot/*id*/, tdcbuf, MAXWORDS, rflag, 1/*rmode*/);
      vmescalerslen[slot] = nw;
      for(ii=0; ii<nw; ii++) vmescalers[slot][ii] = tdcbuf[ii];
	  /*
      printf("slot %d return %d scaler words\n",slot,nw);
      for(ii=0; ii<nw; ii++) printf(" [%3d] 0x%08x\n",ii,vmescalers[slot][ii]);
	  */
    }
  }

  SCALER_UNLOCK;

  return(0);
}


static int
vmeScalersReadFromMemory()
{
  SCALER_LOCK;
  SCALER_UNLOCK;

  return(0);
}


/* following function called from boot script for every board */
static void vmeReadTask();
static int
vmeScalersReadoutStart()
{
#ifndef VXWORKS
  pthread_t id;
#endif  

#ifdef VXWORKS
  vmescalers_lock = semBCreate(SEM_Q_FIFO, SEM_FULL);
  if(vmescalers_lock == NULL)
  {
    printf("vmeScalersReadoutStart ERROR: could not allocate a semaphore\n");
    return(-1);
  }
#else
  pthread_mutex_init(&vmescalers_lock, NULL);
#endif

#ifdef VXWORKS
  if(taskSpawn("vmeREAD", 250, 0, 100000,(FUNCPTR)vmeReadTask,
	   0,0,0,0,0,0,0,0,0,0) == ERROR)
  {
    printf("ERROR vmeScalersReadoutStart: cannot start thread\n");
    perror("taskSpawn"); 
    return(-2);
  }
  else
  {
    printf("vmeScalersReadoutStart: 'vmeREAD' task started\n");
  }
#else
  {
    pthread_attr_t attr;

    pthread_attr_init(&attr); /* initialize attr with default attributes */
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    pthread_create(&id, &attr, vmeReadTask, NULL);
  }
#endif

  return(0);
}


static int
vmeScalersReadoutStop()
{
  /* TODO: END TASK HERE !!!!!!!!!!!!!!!!!!!!!!!! */

  free(vmescalers);

#ifdef VXWORKS
  semGive(vmescalers_lock);
  semDelete(vmescalers_lock);
#else
  pthread_mutex_unlock(&vmescalers_lock);
  pthread_mutex_destroy(&vmescalers_lock);
#endif

  return(0);
}


/* function for forcing synch. event every 10 seconds; use command
      taskSpawn "FORCE_SYNC",119,0,6000,ts2syncTask
   in the boot script */
static void
vmeReadTask()
{
  int id, iFlag;
  int ii, jj;

#ifdef VXWORKS
  extern unsigned long sysClkRateGet();
#endif

  for(ii=0; ii<MAXBOARDS; ii++)
  {
    if( (vmescalers[ii] = (unsigned int *) calloc(MAXWORDS,4)) <= 0)
    {
      printf("ERROR in ScalerThread: cannot allocate memory\n");
      return;
    }
    printf("ScalerThread: Allocated 0x%08x words for slot %d at address 0x%08x\n",MAXWORDS,ii,vmescalers[ii]);
  }



  /* dma */
  usrVmeDmaSetConfig(2,3,0); /*A32,MBLT*/

  tdcbuf = (unsigned int *)i2_from_rol1;


  dsc2Init(0x100000,0x80000,16,0);
  dsc2Config("");
  ndsc2 = dsc2GetNdsc();
  printf("vmeReadTask: found %d dsc2 boards\n",ndsc2);
  rflag = 0xFF; /* latch and read everything */
  rmode = 0; /* not-dma readout */


goto skip123;
  iFlag = 0;  /* base address */
  iFlag = (DIST_ADDR)<<10;
  iFlag |= (1<<0);    /* Sync Source: VXS */
  iFlag |= (1<<2);    /* Trigger Source: VXS */
  /*iFlag |= (1<<5);*/    /* Clock Source: VXS */
  iFlag |= (0<<5);  /* Internal Clock Source */
  faInit((unsigned int)(3<<19),(1<<19),16,iFlag);
  nfadc = faGetNfadc();
skip123:


  while(1)
  {
    if(vmeScalersReadInterval==0) /* if interval==0, wait 1 sec and check again */
    {
#ifdef VXWORKS
      taskDelay(sysClkRateGet());
#else
      sleep(1);
#endif
    }
    else
    {
#ifdef VXWORKS
      taskDelay(sysClkRateGet()*vmeScalersReadInterval);
#else
      sleep(vmeScalersReadInterval);
#endif
      vmeScalersRead();
	  /*printf("vmeReadTask: reading scalers ...\n");*/
    }
  }

  vmeScalersReadoutStop();

  return;
}


/********************************************************************************************************
 end of scalers stuff
********************************************************************************************************/


#endif

void sig_handler(int signo);

ServerCBFunctions gCrateServerCBFcn;

int swap32(int val)
{
  return DW_SWAP(val);
}

short swap16(short val)
{
  return HW_SWAP(val);
}

int
Vme_Read16(Cmd_Read16 *pCmd_Read16, Cmd_Read16_Rsp *pCmd_Read16_Rsp)
{
	short *pRd;
	short *pWr = (short *)((long)pCmd_Read16_Rsp->vals);
	int c = pCmd_Read16->cnt;
	int size = 4+2*c;

	vmeBusToLocalAdrs(0x39, (char *)((long)pCmd_Read16->addr),(char **)&pRd);

	pCmd_Read16_Rsp->cnt = c;	
	
	vmeBusLock();
	if(pCmd_Read16->flags & CRATE_MSG_FLAGS_ADRINC)
		while(c--) *pWr++ = swap16(*pRd++);
	else
		while(c--) *pWr++ = swap16(*pRd);
	vmeBusUnlock();

	return size;
}

int
Vme_Write16(Cmd_Write16 *pCmd_Write16)
{
	short *pRd = pCmd_Write16->vals;
	short *pWr;
	int c = pCmd_Write16->cnt;
	
	vmeBusToLocalAdrs(0x39, (char *)((long)pCmd_Write16->addr),(char **)&pWr);

	vmeBusLock();
	if(pCmd_Write16->flags & CRATE_MSG_FLAGS_ADRINC)
		while(c--) *pWr++ = swap16(*pRd++);
	else
		while(c--) *pWr = swap16(*pRd++);
	vmeBusUnlock();

	return 0;
}

int
Vme_Read32(Cmd_Read32 *pCmd_Read32, Cmd_Read32_Rsp *pCmd_Read32_Rsp)
{
	int *pRd;
	int *pWr = pCmd_Read32_Rsp->vals;
	int c = pCmd_Read32->cnt;
	int size = 4+4*c;

	vmeBusToLocalAdrs(0x39, (char *)((long)pCmd_Read32->addr),(char **)&pRd);

	pCmd_Read32_Rsp->cnt = c;	

	vmeBusLock();
	if(pCmd_Read32->flags & CRATE_MSG_FLAGS_ADRINC)
		while(c--) *pWr++ = swap32(*pRd++);
	else
		while(c--) *pWr++ = swap32(*pRd);
	vmeBusUnlock();

	return size;
}

int
Vme_Write32(Cmd_Write32 *pCmd_Write32)
{
	int *pRd = pCmd_Write32->vals;
	int *pWr;
	int c = pCmd_Write32->cnt;
	
	vmeBusToLocalAdrs(0x39, (char *)((long)pCmd_Write32->addr),(char **)&pWr);

	vmeBusLock();
	if(pCmd_Write32->flags & CRATE_MSG_FLAGS_ADRINC)
		while(c--) *pWr++ = swap32(*pRd++);
	else
		while(c--) *pWr = swap32(*pRd++);
	vmeBusUnlock();

	return 0;
}


int
Vme_ReadScalers(Cmd_ReadScalers *pCmd_ReadScalers, Cmd_ReadScalers_Rsp *pCmd_ReadScalers_Rsp)
{
	int *pWr = pCmd_ReadScalers_Rsp->vals;
	int c = pCmd_ReadScalers->cnt;
	int size = 4+4*c;
    int nw, slot, ii;

	/*printf("Vme_ReadScalers reached\n");fflush(stdout);*/
    nw = pCmd_ReadScalers->cnt;
    slot = pCmd_ReadScalers->slot;
	/*printf("input: cnt=%d slot=%d\n",nw,slot);*/

    pCmd_ReadScalers_Rsp->cnt = vmescalerslen[slot];
    for(ii=0; ii<pCmd_ReadScalers_Rsp->cnt; ii++) pWr[ii] = vmescalers[slot][ii];

	return(size);
}


int
Vme_Delay(Cmd_Delay *pCmd_Delay)
{
	usleep(1000*pCmd_Delay->ms);
	
	return 0;
}




int
main(int argc, char *argv[])
{
  int stat;
#ifdef SCALER_THREAD
  pthread_t gScalerThread;
#endif
	
  if(signal(SIGINT, sig_handler) == SIG_ERR)
  {
	perror("signal");
	exit(0);
  }

  vmeSetQuietFlag(1);
  stat = vmeOpenDefaultWindows();
  if(stat != OK) goto CLOSE;

#ifdef SCALER_THREAD


  {
    int i1, i2, i3;

    usrVmeDmaInit();

    usrVmeDmaMemory(&i1, &i2, &i3);
    i2_from_rol1 = i2;
    printf("tiprimarytinit: i2_from_rol1 = 0x%08x\n",i2_from_rol1);
    i2_from_rol1 = (i2_from_rol1 & 0xFFFFFFF0);
    printf("tiprimarytinit: i2_from_rol1 = 0x%08x\n",i2_from_rol1);
    i2_from_rol1 = i2_from_rol1 + 0x10;
    printf("tiprimarytinit: i2_from_rol1 = 0x%08x\n",i2_from_rol1);
  }

  vmeScalersReadoutStart(); /* pthread_create inside */

  sleep(2);

#endif


//	stat = crateShmCreate();
//	if(stat != OK)
//		goto CLOSE;

// Run socket server
  gCrateServerCBFcn.Read16 = Vme_Read16;
  gCrateServerCBFcn.Write16 = Vme_Write16;
  gCrateServerCBFcn.Read32 = Vme_Read32;
  gCrateServerCBFcn.Write32 = Vme_Write32;
  gCrateServerCBFcn.Delay = Vme_Delay;
  gCrateServerCBFcn.ReadScalers = Vme_ReadScalers;
	
  printf("Starting CrateMsgServer...");
  CrateMsgServerStart(&gCrateServerCBFcn, CRATEMSG_LISTEN_PORT);
  printf("Done.\n");


  while(1) sleep(1);

	
CLOSE:
  vmeCloseDefaultWindows();
	
  return 0;
}

void
closeup()
{
	vmeCloseDefaultWindows();
	printf("DiagGUI server closed...\n");
}

void
sig_handler(int signo)
{
	printf("%s: signo = %d\n",__FUNCTION__,signo);
	switch(signo)
	{
		case SIGINT:
			closeup();
			exit(1);  /* exit if CRTL/C is issued */
	}
}

#else

main()
{
  return;
}

#endif
