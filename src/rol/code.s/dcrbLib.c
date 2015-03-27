/******************************************************************************
*
*  dcrbLib.h  - Driver library header file for readout of the dcrb boards
*
*  Author: Ber Raydo
*          Jefferson Lab Data Acquisition Group
*
*/

#if defined(VXWORKS) || defined(Linux_vme)

#ifdef VXWORKS
#include <vxWorks.h>
/*#include "vxCompat.h"*/
#else
#include <stddef.h>
#include <pthread.h>
#include "jvme.h"
#endif
#include <stdio.h>
#include <string.h>
#ifdef VXWORKS
#include <logLib.h>
#include <taskLib.h>
#include <intLib.h>
#include <iv.h>
#include <semLib.h>
#include <vxLib.h>
#else
#include <unistd.h>
#endif


/* Include DCRB definitions */
#include "dcrbLib.h"

#include "xxxConfig.h"
static int active;

#ifdef VXWORKS
#define DCRBLOCK
#define DCRBUNLOCK
#else
/* Mutex to guard flexio read/writes */
pthread_mutex_t   dcrbMutex = PTHREAD_MUTEX_INITIALIZER;
#define DCRBLOCK      if(pthread_mutex_lock(&dcrbMutex)<0) perror("pthread_mutex_lock");
#define DCRBUNLOCK    if(pthread_mutex_unlock(&dcrbMutex)<0) perror("pthread_mutex_unlock");
#endif

#define LSWAP(x)        ((((x) & 0x000000ff) << 24) | \
                         (((x) & 0x0000ff00) <<  8) | \
                         (((x) & 0x00ff0000) >>  8) | \
                         (((x) & 0xff000000) >> 24))

/* Define external Functions */
#ifdef VXWORKS
IMPORT  STATUS sysBusToLocalAdrs(int, char *, char **);
IMPORT  STATUS intDisconnect(int);
IMPORT  STATUS sysIntEnable(int);
IMPORT  STATUS sysIntDisable(int);
/*
IMPORT  STATUS sysVmeDmaDone(int, int);
IMPORT  STATUS sysVmeDmaSend(UINT32, UINT32, int, BOOL);
*/

#define EIEIO    __asm__ volatile ("eieio")
#define SYNC     __asm__ volatile ("sync")
#endif

/* Define Interrupts variables */
BOOL              dcrbIntRunning  = FALSE;                    /* running flag */
int               dcrbIntID       = -1;                       /* id number of DCRB generating interrupts */
LOCAL VOIDFUNCPTR dcrbIntRoutine  = NULL;                     /* user interrupt service routine */
LOCAL int         dcrbIntArg      = 0;                        /* arg to user routine */
LOCAL UINT32      dcrbIntLevel    = DCRB_VME_INT_LEVEL;         /* default VME interrupt level */
LOCAL UINT32      dcrbIntVec      = DCRB_VME_INT_VEC;           /* default interrupt Vector */

/* Define global variables */
int ndcrb = 0;                                       /* Number of DCRBs in Crate */
int dcrbA32Base   = 0x09000000;                      /* Minimum VME A32 Address for use by DCRBs */
int dcrbA32Offset = 0x08000000;                      /* Difference in CPU A32 Base - VME A32 Base */
int dcrbA24Offset = 0x0;                             /* Difference in CPU A24 Base - VME A24 Base */
volatile struct dcrb_struct *DCRBp[(DCRB_MAX_BOARDS+1)]; /* pointers to DCRB memory map */
volatile unsigned int *DCRBpd[(DCRB_MAX_BOARDS+1)];      /* pointers to DCRB FIFO memory */
volatile unsigned int *DCRBpmb;                        /* pointer to Multblock window */
int dcrbID[DCRB_MAX_BOARDS];                           /* array of slot numbers for DCRBs */
unsigned int dcrbAddrList[DCRB_MAX_BOARDS];            /* array of a24 addresses for DCRBs */
int dcrbRev[(DCRB_MAX_BOARDS+1)];                      /* Board Revision Info for each module */
int dcrbInited=0;                                    /* >0 if Library has been Initialized before */
int dcrbMaxSlot=0;                                   /* Highest Slot hold an DCRB */
int dcrbMinSlot=0;                                   /* Lowest Slot holding an DCRB */
int dcrbSource=0;                                    /* Signal source for DCRB system control*/
int dcrbBlockLevel=0;                                /* Block Level for DCRBs */
int dcrbIntCount = 0;                                /* Count of interrupts from DCRB */
struct dcrb_data_struct dcrb_data;


int
dcrbSlot(unsigned int id)
{
  if(id>=ndcrb)
  {
    printf("%s: ERROR: Index (%d) >= DCRBs initialized (%d).\n",__FUNCTION__,id,ndcrb);
    return(-1);
  }

  return(dcrbID[id]);
}

int
dcrbId(unsigned int slot)
{
  int id;

  for(id=0; id<ndcrb; id++)
  {
    if(dcrbID[id]==slot)
	{
      return(id);
	}
  }

  printf("%s: ERROR: DCRB in slot %d does not exist or not initialized.\n",__FUNCTION__,slot);
  return(-1);
}

/*******************************************************************************
 *
 * dcrbInit - Initialize JLAB DCRB Library. 
 *
 *
 *   iFlag: 16 bit integer
 *       Low 6 bits - Specifies the default Signal distribution (clock,trigger) 
 *                    sources for the board (INTernal, FrontPanel, VXS, VME(Soft))
 *       bit    0:  defines Sync Reset source
 *                     0  VME (Software Sync-Reset)
 *                     1  VXS (P0)
 *       bit    1:  defines Trigger source
 *                     0  VME (Software Triggers)
 *                     1  VXS (P0) 
 *       bits   2:  defines Clock Source
 *                     0  Internal 250MHz Clock
 *                     1  VXS (P0)
 *
 *       Common Modes of Operation:
 *           Value = 0  CLK (Int)  TRIG (Soft)        SYNC (Soft)    (Debug/Test Mode)
 *                   1  CLK (Int)  TRIG (VXS-HighRes) SYNC (Soft)    (Single Board Modes)
 *                   2  CLK (Int)  TRIG (VXS)         SYNC (Soft)    
 *                   3  CLK (Int)  TRIG (VXS-HighRes) SYNC (VXS)
 *                   4  CLK (VXS)  TRIG (Soft)        SYNC (Soft)
 *                   5  CLK (VXS)  TRIG (VXS-HighRes) SYNC (VXS)     (VXS SD Mode)
 *                   7  CLK (VXS)  TRIG (VXS)         SYNC (VXS)     (VXS SD Mode)
 *
 *      bit 16:  Exit before board initialization
 *             0 Initialize DCRB (default behavior)
 *             1 Skip initialization (just setup register map pointers)
 *
 *      bit 17:  Use dcrbAddrList instead of addr and addr_inc
 *               for VME addresses.
 *             0 Initialize with addr and addr_inc
 *             1 Use dcrbAddrList 
 *      
 *
 * RETURNS: ndcrb, or 0 if no boards found, or ERROR
 */

int 
dcrbInit(UINT32 addr, UINT32 addr_inc, int ndc, int iFlag)
{
  int ii, res, errFlag = 0;
  int boardID = 0;
  int maxSlot = 1;
  int minSlot = 21;
  int trigSrc=0, clkSrc=0, srSrc=0;
  unsigned int rdata, laddr, laddr_inc, a32addr;
  volatile struct dcrb_struct *dcrb;
  unsigned short sdata;
  int noBoardInit=0;
  int useList=0;

  /* Check if we have already Initialized boards before */
  if((dcrbInited>0) && (dcrbID[0] != 0)) 
  {
    /* Hard Reset of all DCRB boards in the Crate */
    for(ii=0;ii<ndcrb;ii++) 
	{
	  dcrbHardReset(dcrbID[ii]);
	}
    taskDelay(120);
  }
  
  /* Check if we're initializing using a list */
  useList=(iFlag&(1<<17))>>17;

  /* Check for valid address */
  if(addr==0) 
    {
      printf("dcrbInit: ERROR: Must specify a Bus (VME-based A24) address for DCRB 0\n");
      return(ERROR);
    }
  else if(addr > 0x00ffffff) 
    { /* A24 Addressing */
      printf("dcrbInit: ERROR: A32 Addressing not allowed for DCRB configuration space\n");
      return(ERROR);
    }
  else
    { /* A24 Addressing */
      if( ((addr_inc==0)||(ndc==0)) && (useList==0) )
	ndc = 1; /* assume only one DCRB to initialize */

      /* get the DCRB address */
#ifdef VXWORKS
      res = sysBusToLocalAdrs(0x39,(char *)addr,(char **)&laddr);
#else
      res = vmeBusToLocalAdrs(0x39,(char *)addr,(char **)&laddr);
#endif
      if (res != 0) 
	{
#ifdef VXWORKS
	  printf("dcrbInit: ERROR in sysBusToLocalAdrs(0x39,0x%x,&laddr) \n",addr);
#else
	  printf("dcrbInit: ERROR in vmeBusToLocalAdrs(0x39,0x%x,&laddr) \n",addr);
#endif
	  return(ERROR);
	}
      dcrbA24Offset = laddr - addr;
    }

  /* Init Some Global variables */
  dcrbSource = iFlag&DCRB_SOURCE_MASK;
  dcrbInited = ndcrb = 0;
  bzero((char *)dcrbID,sizeof(dcrbID));

  printf("ndc=%d\n",ndc);fflush(stdout);
  for (ii=0; ii<ndc; ii++) 
  {
      if(useList==1)
	{
	  laddr_inc = dcrbAddrList[ii] + dcrbA24Offset;
	}
      else
	{
	  laddr_inc = laddr +ii*addr_inc;
	}
      dcrb = (struct dcrb_struct *)laddr_inc;
      /* Check if Board exists at that address */
#ifdef VXWORKS
      res = vxMemProbe((char *) &(dcrb->BoardID),VX_READ,4,(char *)&rdata);
#else
      res = vmeMemProbe((char *) &(dcrb->BoardID),4,(char *)&rdata);
#endif
      if(res < 0) 
	{
      printf("NO ADDRESSABLE BOARD !!!\n");fflush(stdout);
#ifdef VXWORKS
	  printf("dcrbInit: ERROR: No addressable board at addr=0x%x\n",(UINT32) dcrb);
#else
	  printf("dcrbInit: ERROR: No addressable board at VME (Local) addr=0x%x (0x%x)\n",
		 (UINT32) laddr_inc-dcrbA24Offset, (UINT32) dcrb);
#endif
	  /*errFlag = 1;*/
/* 	  break; */
	}
      else 
	{
	  /* Check that it is an DCRB board */
	  if((rdata&DCRB_BOARD_MASK) != DCRB_BOARD_ID) 
	  {
	    printf(" ERROR: For board at 0x%x, Invalid Board ID: 0x%x\n",
		     (UINT32) dcrb, rdata);
        continue;
	  }
	  /* Check if this is board has a valid slot number */
	  boardID =  ((vmeRead32(&(dcrb->BlockConfig)))&DCRB_SLOT_ID_MASK)>>11;
	  if((boardID <= 0)||(boardID >21)) 
	  {
	    printf(" ERROR: Board Slot ID is not in range: %d\n",boardID);
	    continue;
	  }
	  DCRBp[boardID] = (struct dcrb_struct *)(laddr_inc);
	  dcrbRev[boardID] = vmeRead32(&(dcrb->FirmwareRev));
/* 	} */
	  dcrbID[ndcrb] = boardID;
	  if(boardID >= maxSlot) maxSlot = boardID;
	  if(boardID <= minSlot) minSlot = boardID;
	  
	  printf("Initialized DCRB %2d  Slot # %2d at address 0x%08x (0x%08x) \n",
		 ndcrb,dcrbID[ndcrb],(UINT32) DCRBp[(dcrbID[ndcrb])],
		 (UINT32) DCRBp[(dcrbID[ndcrb])]-dcrbA24Offset);
	  ndcrb++;
/* 	  printf("Initialized DCRB %2d  Slot # %2d at address 0x%08x \n", */
/* 		 ii,dcrbID[ii],(UINT32) DCRBp[(dcrbID[ii])]); */
	}
  }
  printf("Finished boards initialization, ndcrb=%d\n",ndcrb);fflush(stdout);


  /* Check if we are to exit when pointers are setup */
  noBoardInit=iFlag&(1<<16);

  /* Hard Reset of all DCRB boards in the Crate */
  for(ii=0;ii<ndcrb;ii++) 
  {
    if(!noBoardInit)
	{
      printf("HardReseting slot %d ..\n",dcrbID[ii]);fflush(stdout);
      /*dcrbHardReset(dcrbID[ii]);*/
      printf(".. done.\n");fflush(stdout);
	}
  }
  printf("Finished hard reset\n");fflush(stdout);
  taskDelay(120); 

  /* Initialize Interrupt variables */
  dcrbIntID = -1;
  dcrbIntRunning = FALSE;
  dcrbIntLevel = DCRB_VME_INT_LEVEL;
  dcrbIntVec = DCRB_VME_INT_VEC;
  dcrbIntRoutine = NULL;
  dcrbIntArg = 0;

  /* Calculate the A32 Offset for use in Block Transfers */
#ifdef VXWORKS
  res = sysBusToLocalAdrs(0x09,(char *)dcrbA32Base,(char **)&laddr);
  if (res != 0) 
    {
      printf("dcrbInit: ERROR in sysBusToLocalAdrs(0x09,0x%x,&laddr) \n",dcrbA32Base);
      return(ERROR);
    } 
  else 
    {
      dcrbA32Offset = laddr - dcrbA32Base;
    }
#else
  res = vmeBusToLocalAdrs(0x09,(char *)dcrbA32Base,(char **)&laddr);
  if (res != 0) 
    {
      printf("dcrbInit: ERROR in vmeBusToLocalAdrs(0x09,0x%x,&laddr) \n",dcrbA32Base);fflush(stdout);
      return(ERROR);
    } 
  else 
    {
      dcrbA32Offset = laddr - dcrbA32Base;
    }
#endif

  printf("111\n");fflush(stdout);

    if(!noBoardInit)
    {
	  switch(iFlag&0x7)
	    {
		case 0:
			printf("dcrbInit: Enabling DCRB for Internal Clock ");
			printf("and Software Triggers (Soft Sync Reset)\n");
			clkSrc  = DCRB_REF_CLK_INTERNAL;
			trigSrc = 0;
			break;
		case 1: case 2:
			printf("dcrbInit: Enabling DCRB for Internal Clock ");
			printf("and VXS Triggers (Soft Sync Reset)\n");
			clkSrc  = DCRB_REF_CLK_INTERNAL;
			trigSrc = DCRB_TRIG_P0;
			break;
		case 3:
			printf("dcrbInit: Enabling DCRB for Internal Clock ");
			printf("and VXS-HighRes Triggers (Soft Sync Reset)\n");
			clkSrc  = DCRB_REF_CLK_INTERNAL;
			trigSrc = DCRB_TRIG_P0 | DCRB_SYNC_P0;
			break;
		case 4:
			printf("dcrbInit: Enabling DCRB for VXS Clock ");
			printf("and Software Triggers (Soft Sync Reset)\n");
			clkSrc  = DCRB_REF_CLK_P0;
			trigSrc = 0;
			break;
		case 5:
			printf("dcrbInit: Enabling DCRB for VXS Clock ");
			printf("and VXS Triggers (Soft Sync Reset)\n");
			clkSrc  = DCRB_REF_CLK_P0;
			trigSrc = DCRB_TRIG_P0;
			break;
		case 6: case 7:
			printf("dcrbInit: Enabling DCRB for VXS Clock ");
			printf("and VXS Triggers (VXS Sync Reset)\n");
			clkSrc  = DCRB_REF_CLK_P0;
			trigSrc = DCRB_TRIG_P0 | DCRB_SYNC_P0;
			break;
	    }
    }

  /* Enable Clock source - Internal Clk enabled by default */ 
  for(ii=0; ii<ndcrb; ii++) 
  {
    vmeWrite32(&(DCRBp[dcrbID[ii]]->ClockConfig),clkSrc | DCRB_REF_CLK_RESET);
	vmeWrite32(&(DCRBp[dcrbID[ii]]->ClockConfig),clkSrc);
    printf("Board %d (slot %d) ClockConfig=0x%08x\n",ii,dcrbID[ii],vmeRead32(&(DCRBp[dcrbID[ii]]->ClockConfig)));
  }
  taskDelay(1);

  /* Hard Reset FPGAs and FIFOs */
  for(ii=0;ii<ndcrb;ii++) 
    {
      vmeWrite32(&(DCRBp[dcrbID[ii]]->Reset),0);
    }
  taskDelay(1);

  /* Write configuration registers with default/defined Sources */
  for(ii=0;ii<ndcrb;ii++) 
  {
    
      /* Program an A32 access address for this DCRB's FIFO */
      a32addr = dcrbA32Base + ii*DCRB_MAX_A32_MEM;
#ifdef VXWORKS
      res = sysBusToLocalAdrs(0x09,(char *)a32addr,(char **)&laddr);
      if (res != 0) 
	{
	  printf("dcrbInit: ERROR in sysBusToLocalAdrs(0x09,0x%x,&laddr) \n",a32addr);
	  return(ERROR);
	}
#else
      res = vmeBusToLocalAdrs(0x09,(char *)a32addr,(char **)&laddr);
      if (res != 0) 
	{
	  printf("dcrbInit: ERROR in vmeBusToLocalAdrs(0x09,0x%x,&laddr) \n",a32addr);
	  return(ERROR);
	}
#endif
      DCRBpd[dcrbID[ii]] = (unsigned int *)(laddr);  /* Set a pointer to the FIFO */
      if(!noBoardInit)
	{
	  vmeWrite32(&(DCRBp[dcrbID[ii]]->ADR32),(a32addr>>16) + DCRB_A32_ENABLE);  /* Write the register and enable */
	
	  /* Set Default Block Level to 1 */
	  vmeWrite32(&(DCRBp[dcrbID[ii]]->BlockConfig),1);

      /* berr for every board; TODO: multiblock needs it on last board only !!!*/
      vmeWrite32(&(DCRBp[dcrbID[ii]]->ReadoutConfig), DCRB_ENABLE_BERR);

	}
      dcrbBlockLevel=1;

      /* Setup Trigger and Sync Reset sources */
      if(!noBoardInit)
	{
	  vmeWrite32(&(DCRBp[dcrbID[ii]]->TriggerSource),trigSrc);
	}
  }

  /* If there are more than 1 DCRB in the crate then setup the Muliblock Address
     window. This must be the same on each board in the crate */
  if(ndcrb > 1) 
  {
      a32addr = dcrbA32Base + (ndcrb+1)*DCRB_MAX_A32_MEM; /* set MB base above individual board base */
#ifdef VXWORKS
      res = sysBusToLocalAdrs(0x09,(char *)a32addr,(char **)&laddr);
      if (res != 0) 
	{
	  printf("dcrbInit: ERROR in sysBusToLocalAdrs(0x09,0x%x,&laddr) \n",a32addr);
	  return(ERROR);
	}
#else
      res = vmeBusToLocalAdrs(0x09,(char *)a32addr,(char **)&laddr);
      if (res != 0) 
	{
	  printf("dcrbInit: ERROR in vmeBusToLocalAdrs(0x09,0x%x,&laddr) \n",a32addr);
	  return(ERROR);
	}
#endif
      DCRBpmb = (unsigned int *)(laddr);  /* Set a pointer to the FIFO */


goto skipmultiblock;
    if(!noBoardInit)
	{
	  for (ii=0;ii<ndcrb;ii++) 
	    {
	      /* Write the register and enable */
	      vmeWrite32(&(DCRBp[dcrbID[ii]]->ADR32M),
			((a32addr+DCRB_MAX_A32MB_SIZE)>>8) + (a32addr>>23) + DCRB_AMB_ENABLE);
	    }
	}    
      /* Set First Board and Last Board */
      dcrbMaxSlot = maxSlot;
      dcrbMinSlot = minSlot;
    if(!noBoardInit)
	{
	  vmeWrite32(&(DCRBp[minSlot]->ADR32M),
		    vmeRead32(&(DCRBp[minSlot]->ADR32M)) | DCRB_FIRST_BOARD);
	  vmeWrite32(&(DCRBp[maxSlot]->ADR32M),
		    vmeRead32(&(DCRBp[maxSlot]->ADR32M)) | DCRB_LAST_BOARD);
	}
skipmultiblock:
	;    
  }

  dcrbInited = ndcrb;
  if(errFlag > 0) 
  {
    printf("dcrbInit: ERROR: Unable to initialize all DCRB Modules\n");
    return(ERROR);
  } 
  else 
  {
    if(ndcrb > 0) printf("dcrbInit: %d DCRB(s) successfully initialized\n",ndcrb );
    return(ndcrb);
  }
}






int
dcrbHardReset(int id)
{
	int i;
	unsigned short reloadSequence[] = {
		0xFFFF, 0xAA99, 0x5566, 0x3261,
		0x0000, 0x3281, 0x0B00, 0x32A1,
		0x0000, 0x32C1, 0x0B00, 0x30A1,
		0x000E, 0x2000
	};

	vmeWrite32(&(DCRBp[id]->ICap), DCRB_ICAP_CE);
	vmeWrite32(&(DCRBp[id]->ICap), DCRB_ICAP_CE | DCRB_ICAP_CLK);
	for(i = 0; i < sizeof(reloadSequence)/sizeof(reloadSequence[0]); i++)
	{
		vmeWrite32(&(DCRBp[id]->ICap), reloadSequence[i]);
		vmeWrite32(&(DCRBp[id]->ICap), DCRB_ICAP_CLK | reloadSequence[i]);
	}
	for(i = 0; i < 10; i++)
	{
		vmeWrite32(&(DCRBp[id]->ICap), DCRB_ICAP_CE);
		vmeWrite32(&(DCRBp[id]->ICap), DCRB_ICAP_CE | DCRB_ICAP_CLK);
	}
	return(OK);
}

/*******************************************************************************
 *
 * dcrbSetClockSource - Set the clock source
 *
 *   This routine should be used in the case that the source clock
 *   is NOT set in dcrbInit (and defaults to Internal).  Such is the case
 *   when clocks are synchronized in a many crate system.  The clock source
 *   of the DCRB should ONLY be set AFTER those clocks have been set and
 *   synchronized.
 *
 *   clkSrc: 1 bit integer
 *       bit 0:  defines Clock Source
 *           0  Internal 250MHz Clock
 *           1  VXS (P0)
 *
 */

int
dcrbSetClockSource(int id, int clkSrc)
{
  if(id==0) id=dcrbID[0];

  if((id<=0) || (id>21) || (DCRBp[id] == NULL)) 
    {
      printf("dcrbStatus: ERROR : DCRB in slot %d is not initialized \n",id);
      return;
    }

  if(clkSrc>0x1)
    {
      printf("%s: ERROR: Invalid Clock Source specified (0x%x)\n",
	     __FUNCTION__,clkSrc);
      return ERROR;
    }

  /* Enable Clock source - Internal Clk enabled by default */ 
  vmeWrite32(&(DCRBp[dcrbID[id]]->ClockConfig),clkSrc | DCRB_REF_CLK_RESET);
  vmeWrite32(&(DCRBp[dcrbID[id]]->ClockConfig),clkSrc);
  taskDelay(20);

  switch(clkSrc)
    {
    case DCRB_REF_CLK_INTERNAL:
      printf("%s: DCRB id %d clock source set to INTERNAL\n",
	     __FUNCTION__,id);
      break;

    case DCRB_REF_CLK_P0:
      printf("%s: DCRB id %d clock source set to VXS (P0)\n",
	     __FUNCTION__,id);
      break;
    }

  return OK;
}

void
dcrbStatus(int id, int sflag)
{ 
  int ii;
  unsigned int vers, bid;
  unsigned int busyFifo, busyTrig, errorFifo;
  unsigned int adr32m, adr32;
  unsigned int lookBack, windowWidth, blockConfig, tdcConfig, testPulseConfig;
  unsigned int clkConfig, dacConfig, trigBusyThreshold, trigSrc;
  unsigned int intr, geo;
  unsigned int fifoWordCnt, fifoEventCnt, fifoBlockCnt;
  unsigned int intWordCnt, intEventCnt, intBlockCnt;
  unsigned int readoutCfg;
  unsigned int chDisable[3];

  if(id==0) id=dcrbID[0];

  if((id<=0) || (id>21) || (DCRBp[id] == NULL)) 
    {
      printf("dcrbStatus: ERROR : DCRB in slot %d is not initialized \n",id);
      return;
    }

  DCRBLOCK;
  vers   = vmeRead32(&DCRBp[id]->FirmwareRev);
  bid    = vmeRead32(&DCRBp[id]->BoardID);
  busyFifo = vmeRead32(&DCRBp[id]->GrpBusyFifo);
  busyTrig = vmeRead32(&DCRBp[id]->GrpBusyTrig);
  errorFifo = vmeRead32(&DCRBp[id]->GrpErrorFifo);
  adr32m = vmeRead32(&DCRBp[id]->ADR32M);
  lookBack = vmeRead32(&DCRBp[id]->LookBack);
  windowWidth = vmeRead32(&DCRBp[id]->WindowWidth);
  blockConfig = vmeRead32(&DCRBp[id]->BlockConfig);
  tdcConfig = vmeRead32(&DCRBp[id]->TDCConfig);
  clkConfig = vmeRead32(&DCRBp[id]->ClockConfig);
  testPulseConfig = vmeRead32(&DCRBp[id]->TestPulseConfig);
  dacConfig = vmeRead32(&DCRBp[id]->DACConfig);
  trigBusyThreshold = vmeRead32(&DCRBp[id]->TriggerBusyThreshold);
  trigSrc = vmeRead32(&DCRBp[id]->TriggerSource);
  adr32 = vmeRead32(&DCRBp[id]->ADR32);
  intr = vmeRead32(&DCRBp[id]->Interrupt);
  geo = vmeRead32(&DCRBp[id]->Geo);
  fifoWordCnt = vmeRead32(&DCRBp[id]->FifoWordCnt);
  fifoEventCnt = vmeRead32(&DCRBp[id]->FifoEventCnt);
  fifoBlockCnt = vmeRead32(&DCRBp[id]->FifoBlockCnt);
  readoutCfg = vmeRead32(&DCRBp[id]->ReadoutConfig);
  intWordCnt = vmeRead32(&DCRBp[id]->IntWordCnt);
  intEventCnt = vmeRead32(&DCRBp[id]->IntEventCnt);
  intBlockCnt = vmeRead32(&DCRBp[id]->IntBlockCnt);
  
  chDisable[0] = vmeRead32(&DCRBp[id]->ChDisable[0]);
  chDisable[1] = vmeRead32(&DCRBp[id]->ChDisable[1]);
  chDisable[2] = vmeRead32(&DCRBp[id]->ChDisable[2]);
  DCRBUNLOCK;

#ifdef VXWORKS
  printf("\nSTATUS for DCRB in slot %d at base address 0x%x \n",
	 id, (UINT32) DCRBp[id]);
#else
  printf("\nSTATUS for DCRB in slot %d at VME (Local) base address 0x%x (0x%x)\n",
	 id, (UINT32) DCRBp[id] - dcrbA24Offset, (UINT32) DCRBp[id]);
#endif
  printf("---------------------------------------------------------------------- \n");

  printf(" Board Firmware Rev/ID = 0x%04x / 0x%08x\n", (vers)&0xffff, bid);
  if(adr32m&DCRB_AMB_ENABLE) 
    {
      printf(" Alternate VME Addressing: Multiblock Enabled\n");
      if(adr32&DCRB_A32_ENABLE)
	printf("   A32 Enabled at VME (Local) base 0x%08x (0x%08x)\n",(adr32&0xFF80)<<16,(UINT32) DCRBpd[id]);
      else
	printf("   A32 Disabled\n");
    
      printf("   Multiblock VME Address Range 0x%08x - 0x%08x\n",(adr32m&0x1FF)<<23,(adr32m&0x1FF0000)<<7);
    }
  else
    {
      printf(" Alternate VME Addressing: Multiblock Disabled\n");
      if(adr32&DCRB_A32_ENABLE)
	printf("   A32 Enabled at VME (Local) base 0x%08x (0x%08x)\n",(adr32&0xFF80)<<16,(UINT32) DCRBpd[id]);
      else
	printf("   A32 Disabled\n");
    }

  if(intr&DCRB_INT_ENABLE_MASK) 
    {
      printf("\n  Interrupts ENABLED: ");
      printf(" on Block Count(%d), Event Count(%d), Word Count(%d)", intBlockCnt, intEventCnt, intWordCnt);
      printf("\n");
      printf("  Interrupt Reg: 0x%08x\n",intr);
      printf("  VME INT Vector = 0x%x  Level = %d\n",(intr&DCRB_INT_VEC_MASK),((intr&DCRB_INT_LEVEL_MASK)>>8));
    }

  printf("\n Signal Sources: \n");
  if((clkConfig&DCRB_REF_CLK_MASK)==DCRB_REF_CLK_INTERNAL)
    {
      printf("   Ref Clock : Internal\n");
    }
  else if((clkConfig&DCRB_REF_CLK_MASK)==DCRB_REF_CLK_P0)
    {
      printf("   Ref Clock : VXS\n");
    }
  else
    {
      printf("   Ref Clock : %d (Undefined)\n",(clkConfig&DCRB_REF_CLK_MASK));
    }

  switch(trigSrc&DCRB_TRIG_MASK) 
    {
    case 0:
      printf("   Trig Src  : VME (Software)\n");
      break;
    case DCRB_TRIG_P0_SYNC:
      printf("   Trig Src  : VXS\n");
      break;
    case DCRB_TRIG_P0:
      printf("   Trig Src  : VXS-HighRes\n");
      break;
    }  

  switch(trigSrc&DCRB_SYNC_MASK) 
    {
    case 0:
      printf("   Sync Reset: VME (Software)\n");
      break;
    case DCRB_SYNC_P0:
      printf("   Sync Reset: VXS (Sync)\n");
      break;
    }  

  printf("\n Configuration: \n");

  if(readoutCfg&DCRB_ENABLE_BERR)
    printf("   Bus Error ENABLED\n");
  else
    printf("   Bus Error DISABLED\n");


  if(adr32m&DCRB_ENABLE_MULTIBLOCK) 
    {
	  if(adr32m&DCRB_FIRST_BOARD)
	    printf("   MultiBlock transfer ENABLED (First Board - token via VXS)\n");
	  else if(adr32m&DCRB_LAST_BOARD)
	    printf("   MultiBlock transfer ENABLED (Last Board  - token via VXS)\n");
	  else
	    printf("   MultiBlock transfer ENABLED (Token via VXS)\n");
	  /* #ifdef VERSION1 */
    } 
  else 
    {
      printf("   MultiBlock transfer DISABLED\n");
    }

  printf("\n TDC Processing Configuration: \n");
  printf("   Channel Disable Mask[31- 0] = 0x%08x\n",chDisable[0]);
  printf("   Channel Disable Mask[63-32] = 0x%08x\n",chDisable[1]);
  printf("   Channel Disable Mask[95-64] = 0x%08x\n",chDisable[2]);
  printf("   Lookback (PL)    = %d ns   Time Window (PTW) = %d ns\n",lookBack,windowWidth);
  printf("   Channel deadtime = %d ns\n", tdcConfig*8);

  printf("\n");
  printf("   Busy Fifo Mask  = 0x%06x\n", busyFifo);
  printf("   Busy Trig Mask  = 0x%06x\n", busyTrig);
  printf("   Error Fifo Mask = 0x%06x\n", errorFifo);

  printf("   Blocks in FIFO  = %u  (Block level = %d)\n", fifoBlockCnt, blockConfig & 0x7ff);
  printf("   Events in FIFO  = %u\n", fifoEventCnt);
  printf("   Words in FIFO   = %u = %u\n", fifoWordCnt);
  printf("\n");
}

void 
dcrbGStatus(int sflag)
{
  int ii;

  for (ii=0;ii<ndcrb;ii++) 
    {
      dcrbStatus(dcrbID[ii],sflag);
    }

}



/***********************
 *
 *  dcrbSetProcMode - Setup DCRB processing modes.
 *
 */
int
dcrbSetProcMode(int id, unsigned int lookBack, unsigned int windowWidth, unsigned int deadTime)
{
  
  int err=0;
  unsigned int ptw_last_adr, ptw_max_buf;


  if(id==0) id=dcrbID[0];

  if((id<=0) || (id>21) || (DCRBp[id] == NULL)) 
    {
      logMsg("dcrbSetProcMode: ERROR : DCRB in slot %d is not initialized \n",id,0,0,0,0,0);
      return(ERROR);
    }

	
	#define DCRB_MAX_LOOKBACK    65535
#define DCRB_MAX_WINDOW      65535

  /*Defaults */
  if((lookBack==0)||(lookBack>DCRB_MAX_LOOKBACK))  lookBack  = DCRB_DEFAULT_LOOKBACK;
  if((windowWidth==0)||(windowWidth>DCRB_MAX_WINDOW)) windowWidth = DCRB_DEFAULT_WINDOW;
  if((deadTime==0)||(deadTime>DCRB_MAX_DEADTIME)) deadTime = DCRB_MAX_DEADTIME;

  /* Consistancy check */
  if(windowWidth > lookBack) 
    {
      err++;
      printf("dcrbSetProcMode: ERROR: Window must be <= Latency\n"); 
    }

  DCRBLOCK;
  vmeWrite32(&DCRBp[id]->LookBack, lookBack);
  vmeWrite32(&DCRBp[id]->WindowWidth, windowWidth);
  vmeWrite32(&DCRBp[id]->TDCConfig, deadTime);
  DCRBUNLOCK;

  return(OK);
}

void
dcrbGSetProcMode(unsigned int lookBack, unsigned int windowWidth, unsigned int deadTime)
{
  int ii, res;

  for (ii=0;ii<ndcrb;ii++) {
    res = dcrbSetProcMode(dcrbID[ii],lookBack,windowWidth, deadTime);
    if(res<0) printf("ERROR: slot %d, in dcrbSetProcMode()\n",dcrbID[ii]);
  }
}

/**************************************************************************************
 *
 *  dcrbReadBlock - General Data readout routine
 *
 *    id    - Slot number of module to read
 *    data  - local memory address to place data
 *    nwrds - Max number of words to transfer
 *    rflag - Readout Flag
 *              0 - programmed I/O from the specified board
 *              1 - DMA transfer using Universe/Tempe DMA Engine 
 *                    (DMA VME transfer Mode must be setup prior)
 *              2 - Multiblock DMA transfer (Multiblock must be enabled
 *                     and daisychain in place or SD being used)
 */
int
dcrbReadBlock(int id, volatile UINT32 *data, int nwrds, int rflag)
{
  int ii, blknum, evnum1;
  int stat, retVal, xferCount, rmode, async;
  int dCnt, berr=0;
  int dummy=0;
  volatile unsigned int *laddr;
  unsigned int bhead, ehead, val;
  unsigned int vmeAdr, csr;

  if(id==0) id=dcrbID[0];

  if((id<=0) || (id>21) || (DCRBp[id] == NULL)) 
    {
      logMsg("dcrbReadBlock: ERROR : DCRB in slot %d is not initialized \n",id,0,0,0,0,0);
      return(ERROR);
    }

  if(data==NULL) 
    {
      logMsg("dcrbReadBlock: ERROR: Invalid Destination address\n",0,0,0,0,0,0);
      return(ERROR);
    }

  if(nwrds <= 0) nwrds= (DCRB_MAX_TDC_CHANNELS*DCRB_MAX_DATA_PER_CHANNEL) + 8;
  rmode = rflag&0x0f;
  async = rflag&0x80;
  
  if(rmode >= 1) 
    { /* Block Transfers */
    
      /*Assume that the DMA programming is already setup. */
      /* Don't Bother checking if there is valid data - that should be done prior
	 to calling the read routine */

      /* Check for 8 byte boundary for address - insert dummy word (Slot 0 DCRB Dummy DATA)*/
      if((unsigned long) (data)&0x7) 
	{
#ifdef VXWORKS
	  *data = DCRB_DUMMY_DATA;
#else
	  *data = LSWAP(DCRB_DUMMY_DATA);
#endif
	  dummy = 1;
	  laddr = (data + 1);
	} 
      else 
	{
	  dummy = 0;
	  laddr = data;
	}

      DCRBLOCK;
      if(rmode == 2) 
	{ /* Multiblock Mode */
	  if((vmeRead32(&(DCRBp[id]->ADR32M))&DCRB_FIRST_BOARD)==0) 
	    {
	      logMsg("dcrbReadBlock: ERROR: DCRB in slot %d is not First Board\n",id,0,0,0,0,0);
	      DCRBUNLOCK;
	      return(ERROR);
	    }
	  vmeAdr = (unsigned int)(DCRBpmb) - dcrbA32Offset;
	}
      else
	{
	  vmeAdr = (unsigned int)(DCRBpd[id]) - dcrbA32Offset;
	}
	  /*
#ifdef VXWORKS
      retVal = sysVmeDmaSend((UINT32)laddr, vmeAdr, (nwrds<<2), 0);
#else
      retVal = vmeDmaSend((UINT32)laddr, vmeAdr, (nwrds<<2));
#endif
	  */
    retVal = usrVme2MemDmaStart(vmeAdr, (UINT32)laddr, (nwrds<<2));
      if(retVal |= 0) 
	{
	  logMsg("dcrbReadBlock: ERROR in DMA transfer Initialization 0x%x\n",retVal,0,0,0,0,0);
	  DCRBUNLOCK;
	  return(retVal);
	}

      if(async) 
	{ /* Asynchonous mode - return immediately - don't wait for done!! */
	  DCRBUNLOCK;
	  return(OK);
	}
      else
	{
	  /* Wait until Done or Error */
	  /*
#ifdef VXWORKS
	  retVal = sysVmeDmaDone(10000,1);
#else
	  retVal = vmeDmaDone();
#endif
	  */
      retVal = usrVme2MemDmaDone();
	}

      if(retVal > 0) 
	{
	/*
	  / * Check to see that Bus error was generated by FADC * /
	  if(rmode == 2) 
	    {
	      csr = vmeRead32(&(FAp[fadcMaxSlot]->csr));  / * from Last FADC * /
	      stat = (csr)&FA_CSR_BERR_STATUS;  / * from Last FADC * /
	    }
	  else
	    {
	      csr = vmeRead32(&(FAp[id]->csr));  / * from Last FADC * /
	      stat = (csr)&FA_CSR_BERR_STATUS;  / * from Last FADC * /
	    }
		*/
	  if((retVal>0))/* && (stat))*/ 
	    {
#ifdef VXWORKS
	      xferCount = (/*nwrds -*/ (retVal>>2) + dummy);  /* Number of Longwords transfered */
#else
	      xferCount = ((retVal>>2) + dummy);  /* Number of Longwords transfered */
	      /* 	xferCount = (retVal + dummy);  /\* Number of Longwords transfered *\/ */
#endif
	      DCRBUNLOCK;
	      return(xferCount); /* Return number of data words transfered */
	    }
	  else
	    {
#ifdef VXWORKS
	      xferCount = (/*nwrds -*/ (retVal>>2) + dummy);  /* Number of Longwords transfered */
#else
	      xferCount = ((retVal>>2) + dummy);  /* Number of Longwords transfered */
#endif
	      logMsg("dcrbReadBlock: DMA transfer terminated by unknown BUS Error (csr=0x%x xferCount=%d)\n",csr,xferCount,0,0,0,0);
	      DCRBUNLOCK;
	      return(xferCount);
	      /* 	return(ERROR); */
	    }
	} 
      else if (retVal == 0)
	{ /* Block Error finished without Bus Error */
#ifdef VXWORKS
	  logMsg("dcrbReadBlock: WARN: DMA transfer terminated by word count 0x%x\n",nwrds,0,0,0,0,0);
#else
	  logMsg("dcrbReadBlock: WARN: DMA transfer returned zero word count 0x%x\n",nwrds,0,0,0,0,0);
#endif
	  DCRBUNLOCK;
	  return(/*nwrds*/retVal>>2);
	} 
      else 
	{  /* Error in DMA */
#ifdef VXWORKS
	  logMsg("dcrbReadBlock: ERROR: sysVmeDmaDone returned an Error\n",0,0,0,0,0,0);
#else
	  logMsg("dcrbReadBlock: ERROR: vmeDmaDone returned an Error\n",0,0,0,0,0,0);
#endif
	  DCRBUNLOCK;
	  return(retVal>>2);
	}

    } 
  else 
    {  /*Programmed IO */

      /* Check if Bus Errors are enabled. If so then disable for Prog I/O reading */
      DCRBLOCK;
      berr = vmeRead32(&(DCRBp[id]->ReadoutConfig))&DCRB_ENABLE_BERR;
      if(berr)
	    vmeWrite32(&(DCRBp[id]->ReadoutConfig),vmeRead32(&(DCRBp[id]->ReadoutConfig)) & ~DCRB_ENABLE_BERR);

      dCnt = 0;
      /* Read Block Header - should be first word */
      bhead = (unsigned int) *DCRBpd[id]; 
#ifndef VXWORKS
      bhead = LSWAP(bhead);
#endif
      if((bhead&DCRB_DATA_TYPE_DEFINE)&&((bhead&DCRB_DATA_TYPE_MASK) == DCRB_DATA_BLOCK_HEADER)) {
	blknum = bhead&DCRB_DATA_BLKNUM_MASK;
	ehead = (unsigned int) *DCRBpd[id];
#ifndef VXWORKS
	ehead = LSWAP(ehead);
#endif
	evnum1 = ehead&DCRB_DATA_TRIGNUM_MASK;
#ifdef VXWORKS
	data[dCnt] = bhead;
#else
	data[dCnt] = LSWAP(bhead); /* Swap back to little-endian */
#endif
	dCnt++;
#ifdef VXWORKS
	data[dCnt] = ehead;
#else
	data[dCnt] = LSWAP(ehead); /* Swap back to little-endian */
#endif
	dCnt++;
      }
      else
	{
	  /* We got bad data - Check if there is any data at all */
	  if( vmeRead32(&(DCRBp[id]->FifoWordCnt)) == 0) 
	    {
	      logMsg("dcrbReadBlock: FIFO Empty (0x%08x)\n",bhead,0,0,0,0,0);
	      DCRBUNLOCK;
	      return(0);
	    } 
	  else 
	    {
	      logMsg("dcrbReadBlock: ERROR: Invalid Header Word 0x%08x\n",bhead,0,0,0,0,0);
	      DCRBUNLOCK;
	      return(ERROR);
	    }
	}

      ii=0;
      while(ii<nwrds) 
	{
	  val = (unsigned int) *DCRBpd[id];
	  data[ii+2] = val;
#ifndef VXWORKS
	  val = LSWAP(val);
#endif
	  if( (val&DCRB_DATA_TYPE_DEFINE) 
	      && ((val&DCRB_DATA_TYPE_MASK) == DCRB_DATA_BLOCK_TRAILER) )
	    break;
	  ii++;
	}
      ii++;
      dCnt += ii;


      if(berr)
	vmeWrite32(&(DCRBp[id]->ReadoutConfig),
		  vmeRead32(&(DCRBp[id]->ReadoutConfig)) | DCRB_ENABLE_BERR);

      DCRBUNLOCK;
      return(dCnt);
    }

  DCRBUNLOCK;
  return(OK);
}

int
dcrbReadBlockStatus(int id, volatile UINT32 *data, int nwrds, int rflag)
{

  int stat, retVal, xferCount, rmode, async;
  int dummy=0;
  unsigned int csr=0;
  
  if(id==0) id=dcrbID[0];

  if((id<=0) || (id>21) || (DCRBp[id] == NULL)) 
    {
      logMsg("dcrbReadBlockStatus: ERROR : DCRB in slot %d is not initialized \n",id,0,0,0,0,0);
      return(ERROR);
    }

  if(nwrds <= 0) nwrds= (DCRB_MAX_TDC_CHANNELS*DCRB_MAX_DATA_PER_CHANNEL) + 8;
  rmode = rflag&0x0f;
  async = rflag&0x80;

  /* Check for 8 byte boundary for address - insert dummy word (Slot 0 DCRB Dummy DATA)*/
  if((unsigned long) (data)&0x7) 
    {
      dummy = 1;
    } 
  else 
    {
      dummy = 0;
    }
  /*
#ifdef VXWORKS
  retVal = sysVmeDmaDone(10000,1);
#else
  retVal = vmeDmaDone();
#endif
  */
      retVal = usrVme2MemDmaDone();
  DCRBLOCK;
  if(retVal > 0) 
    {
	/*
      / * Check to see that Bus error was generated by FADC * /
      if(rmode == 2) 
	{
	  csr = vmeRead32(&(FAp[fadcMaxSlot]->csr));  / * from Last FADC * /
	  stat = (csr)&FA_CSR_BERR_STATUS;  / * from Last FADC * /
	}
      else
	{
	  stat = vmeRead32(&(FAp[id]->csr))&FA_CSR_BERR_STATUS;  / * from FADC id * /
	}
	*/
      if((retVal>0) && (stat)) 
	{
	  xferCount = (/*nwrds -*/ (retVal>>2) + dummy);  /* Number of Longwords transfered */
	  DCRBUNLOCK;
	  return(xferCount); /* Return number of data words transfered */
	}
      else
	{
	  xferCount = (/*nwrds -*/ (retVal>>2) + dummy);  /* Number of Longwords transfered */
	  logMsg("dcrbReadBlockStatus: DMA transfer terminated by unknown BUS Error (csr=0x%x nwrds=%d)\n",csr,xferCount,0,0,0,0);
	  DCRBUNLOCK;
	  return(ERROR);
	}
    } 
  else if (retVal == 0)
    { /* Block Error finished without Bus Error */
      logMsg("dcrbReadBlockStatus: WARN: DMA transfer terminated by word count 0x%x\n",nwrds,0,0,0,0,0);
      DCRBUNLOCK;
      return(/*nwrds*/retVal>>2);
    } 
  else 
    {  /* Error in DMA */
      logMsg("dcrbReadBlockStatus: ERROR: sysVmeDmaDone returned an Error\n",0,0,0,0,0,0);
      DCRBUNLOCK;
      return(retVal);
    }
  
}

int
dcrbPrintBlock(int id, int rflag)
{

  int ii, blknum, evnum1;
  int nwrds=32768, dCnt, berr=0;
  unsigned int data, bhead, ehead;

  if(id==0) id=dcrbID[0];

  if((id<=0) || (id>21) || (DCRBp[id] == NULL)) 
    {
      printf("dcrbPrintEvent: ERROR : DCRB in slot %d is not initialized \n",id);
      return(ERROR);
    }

  /* Check if data available */
  DCRBLOCK;
  if(vmeRead32(&(DCRBp[id]->FifoWordCnt))==0) 
    {
      printf("dcrbPrintEvent: ERROR: FIFO Empty\n");
      DCRBUNLOCK;
      return(0);
    }

  /* Check if Bus Errors are enabled. If so then disable for reading */
  berr = vmeRead32(&(DCRBp[id]->ReadoutConfig))&DCRB_ENABLE_BERR;
  if(berr)
    vmeWrite32(&(DCRBp[id]->ReadoutConfig),vmeRead32(&(DCRBp[id]->ReadoutConfig)) & ~DCRB_ENABLE_BERR);

  dCnt = 0;
  /* Read Block Header - should be first word */
  bhead = (unsigned int) *DCRBpd[id];
#ifndef VXWORKS
  bhead = LSWAP(bhead);
#endif
  if( (bhead&DCRB_DATA_TYPE_DEFINE)&&((bhead&DCRB_DATA_TYPE_MASK) == DCRB_DATA_BLOCK_HEADER)) 
    {
      blknum = bhead&DCRB_DATA_BLKNUM_MASK;
      ehead = (unsigned int) *DCRBpd[id];
#ifndef VXWORKS
      ehead = LSWAP(ehead);
#endif
      evnum1 = ehead&DCRB_DATA_TRIGNUM_MASK;
      printf("%4d: ",dCnt+1); 
      dcrbDataDecode(bhead);
      dCnt++;
      printf("%4d: ",dCnt+1); 
      dcrbDataDecode(ehead);
      dCnt++;
    }
  else
    {
      /* We got bad data - Check if there is any data at all */
      if(vmeRead32(&(DCRBp[id]->FifoWordCnt))==0) 
	{
	  logMsg("dcrbPrintBlock: FIFO Empty (0x%08x)\n",bhead,0,0,0,0,0);
	  DCRBUNLOCK;
	  return(0);
	} 
      else 
	{
	  logMsg("dcrbPrintBlock: ERROR: Invalid Header Word 0x%08x\n",bhead,0,0,0,0,0);
	  DCRBUNLOCK;
	  return(ERROR);
	}
    }
  
  ii=0;
  while(ii<nwrds) 
    {
      data = (unsigned int) *DCRBpd[id];
#ifndef VXWORKS
      data = LSWAP(data);
#endif
      printf("%4d: ",dCnt+1+ii); 
      dcrbDataDecode(data);
      if((data&DCRB_DATA_TYPE_DEFINE)&&((data&DCRB_DATA_TYPE_MASK) == DCRB_DATA_BLOCK_TRAILER))
	break;
      if((data&DCRB_DATA_TYPE_DEFINE)&&((data&DCRB_DATA_TYPE_MASK) == DCRB_DATA_INVALID))
	break;
      ii++;
    }
  ii++;
  dCnt += ii;


  if(berr)
    vmeWrite32(&(DCRBp[id]->ReadoutConfig),
	      vmeRead32( &(DCRBp[id]->ReadoutConfig)) | DCRB_ENABLE_BERR );
  
  DCRBUNLOCK;
  return(dCnt);
  
}




/*****************************************************************************/


/* Reset Event Builder (FIFOs etc) */
void
dcrbClear(int id)
{
  if(id==0) id = dcrbID[0];

  if((id<=0) || (id>21) || (DCRBp[id] == NULL)) 
    {
      logMsg("dcrbClear: ERROR : DCRB in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }

  DCRBLOCK;
  vmeWrite32(&(DCRBp[id]->Reset),0);
  DCRBUNLOCK;
}


void
dcrbReset(int id, int iFlag)
{
  unsigned int a32addr, addrMB;

  if(id==0) id=dcrbID[0];

  if((id<=0) || (id>21) || (DCRBp[id] == NULL)) 
    {
      logMsg("dcrbReset: ERROR : DCRB in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }

  DCRBLOCK;
  if(iFlag==0)
    {
      a32addr = vmeRead32(&(DCRBp[id]->ADR32));
      addrMB  = vmeRead32(&(DCRBp[id]->ADR32M));
    }

  dcrbHardReset(id);
  taskDelay(120);

  if(iFlag==0)
    {
      vmeWrite32(&(DCRBp[id]->ADR32),a32addr);
      vmeWrite32(&(DCRBp[id]->ADR32M),addrMB);
    }
  DCRBUNLOCK;
}

void
dcrbChanDisable(int id, unsigned int cmask0, unsigned int cmask1, unsigned int cmask2)
{

  if(id==0) id=dcrbID[0];

  if((id<=0) || (id>21) || (DCRBp[id] == NULL)) 
    {
      logMsg("dcrbChanDisable: ERROR : DCRB in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }

  DCRBLOCK;
  /* Write New Disable Mask */
  vmeWrite32(&(DCRBp[id]->ChDisable[0]), cmask0);
  vmeWrite32(&(DCRBp[id]->ChDisable[1]), cmask1);
  vmeWrite32(&(DCRBp[id]->ChDisable[2]), cmask2);
  DCRBUNLOCK;

}

void
dcrbTrig(int id)
{
  if(id==0) id=dcrbID[0];

  if((id<=0) || (id>21) || (DCRBp[id] == NULL)) 
    {
      logMsg("dcrbTrig: ERROR : DCRB in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }

  DCRBLOCK;
  vmeWrite32(&(DCRBp[id]->TriggerSource), vmeRead32(&(DCRBp[id]->TriggerSource)) | DCRB_TRIG_VME);
  DCRBUNLOCK;
}

void
dcrbGTrig()
{
  int ii;

  for(ii=0;ii<ndcrb;ii++)
    dcrbTrig(dcrbID[ii]);
}



void
dcrbSync(int id)
{
  if(id==0) id=dcrbID[0];

  if((id<=0) || (id>21) || (DCRBp[id] == NULL)) 
    {
      logMsg("dcrbSync: ERROR : DCRB in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }

  DCRBLOCK;
  vmeWrite32(&(DCRBp[id]->TriggerSource), vmeRead32(&(DCRBp[id]->TriggerSource)) | DCRB_SYNC_VME);
  DCRBUNLOCK;
}



/* Return Event/Block count for DCRB in slot id */
int
dcrbDready(int id, int dflag)
{
  unsigned int dcnt=0;

  if(id==0) id=dcrbID[0];

  if((id<=0) || (id>21) || (DCRBp[id] == NULL)) 
    {
      logMsg("dcrbDready: ERROR : DCRB in slot %d is not initialized \n",id,0,0,0,0,0);
      return(ERROR);
    }
  DCRBLOCK;
  if(dflag)
    dcnt = vmeRead32(&(DCRBp[id]->FifoBlockCnt));
  else
    dcnt = vmeRead32(&(DCRBp[id]->FifoEventCnt));
  DCRBUNLOCK;

  
  return(dcnt);
}

/* Return a Block Ready status for DCRB. If Block Level is =1 then return Event Ready status */
int
dcrbBready(int id)
{
  int stat=0;

  if(id==0) id=dcrbID[0];

  if((id<=0) || (id>21) || (DCRBp[id] == NULL)) 
    {
      logMsg("dcrbBready: ERROR : DCRB in slot %d is not initialized \n",id,0,0,0,0,0);
      return(ERROR);
    }
  
  DCRBLOCK;
  stat = vmeRead32(&(DCRBp[id]->FifoBlockCnt));
  DCRBUNLOCK;

  if(stat)
    return(1);
  else
    return(0);
}

unsigned int
dcrbGBready()
{
  int ii, id, stat=0;
  unsigned int dmask=0;
  
  DCRBLOCK;
  for(ii=0;ii<ndcrb;ii++) 
    {
      id = dcrbID[ii];
      stat = vmeRead32(&(DCRBp[id]->FifoBlockCnt));
 
      if(stat)
	dmask |= (1<<id);
    }
  DCRBUNLOCK;
  
  return(dmask);
}

/* return Scan mask for all initialized DCRBs */
unsigned int
dcrbScanMask()
{
  int idcrb, id, dmask=0;

  for(idcrb=0; idcrb<ndcrb; idcrb++)
    {
      id = dcrbID[idcrb];
      dmask |= (1<<id);
    }

  return(dmask);
}

/* if val>0 then set the busy level, if val=0 then read it back.
   if bflag>0 then force the module Busy */
int
dcrbBusyLevel(int id, unsigned int val, int bflag)
{
  unsigned int blreg=0;

  if(id==0) id=dcrbID[0];

  if((id<=0) || (id>21) || (DCRBp[id] == NULL)) 
    {
      logMsg("dcrbBusyLevel: ERROR : DCRB in slot %d is not initialized \n",id,0,0,0,0,0);
      return(ERROR);
    }
  
  /* if Val > 0 then set the Level else leave it alone*/
  DCRBLOCK;

  vmeWrite32(&(DCRBp[id]->BusyEventCnt), 0xFFFFFFFF);
  if(bflag)
    vmeWrite32(&(DCRBp[id]->BusyBlockCnt), 0xFFFFFFFF);
  else
    vmeWrite32(&(DCRBp[id]->BusyBlockCnt), 0);

  if(val) 
	vmeWrite32(&(DCRBp[id]->BusyWordCnt),val);
  else
    blreg = vmeRead32(&(DCRBp[id]->BusyWordCnt));

  DCRBUNLOCK;

  return(blreg);
}

int
dcrbBusy(int id)
{
  unsigned int blreg=0;
  unsigned int dreg=0;

  if(id==0) id=dcrbID[0];

  if((id<=0) || (id>21) || (DCRBp[id] == NULL)) 
    {
      logMsg("dcrbBusy: ERROR : DCRB in slot %d is not initialized \n",id,0,0,0,0,0);
      return(ERROR);
    }
  
  DCRBLOCK;
  blreg = vmeRead32(&(DCRBp[id]->BusyWordCnt));
  dreg  = vmeRead32(&(DCRBp[id]->FifoWordCnt));
  DCRBUNLOCK;

  if(dreg>=blreg)
    return(1);
  else
    return(0);
}


void
dcrbEnableSoftTrig(int id)
{

  if(id==0) id=dcrbID[0];

  if((id<=0) || (id>21) || (DCRBp[id] == NULL)) 
    {
      logMsg("dcrbEnableSoftTrig: ERROR : DCRB in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }
  
  /* Clear the source */
  DCRBLOCK;
  vmeWrite32(&(DCRBp[id]->TriggerSource),
	    vmeRead32(&(DCRBp[id]->TriggerSource)) & ~DCRB_TRIG_MASK );
  /* Set Source and Enable*/
  vmeWrite32(&(DCRBp[id]->TriggerSource),
	    vmeRead32(&(DCRBp[id]->TriggerSource)) | DCRB_TRIG_VME );
  DCRBUNLOCK;
}
void
dcrbGEnableSoftTrig()
{
  int ii, id;

  for(ii=0;ii<ndcrb;ii++) 
    {
      id = dcrbID[ii];
      dcrbEnableSoftTrig(id);
    }
}

void
dcrbEnableSoftSync(int id)
{

  if(id==0) id=dcrbID[0];

  if((id<=0) || (id>21) || (DCRBp[id] == NULL)) 
    {
      logMsg("dcrbEnableSoftSync: ERROR : DCRB in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }
  
  /* Clear the source */
  DCRBLOCK;
  vmeWrite32(&(DCRBp[id]->TriggerSource),
	    vmeRead32(&(DCRBp[id]->TriggerSource)) & ~DCRB_SYNC_MASK);
  /* Set Source and Enable*/
  vmeWrite32(&(DCRBp[id]->TriggerSource),
	    vmeRead32(&(DCRBp[id]->TriggerSource)) | DCRB_SYNC_VME);
  DCRBUNLOCK;
}

void
dcrbEnableBusError(int id)
{

  if(id==0) id=dcrbID[0];

  if((id<=0) || (id>21) || (DCRBp[id] == NULL)) 
    {
      logMsg("dcrbEnableBusError: ERROR : DCRB in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }
  
  DCRBLOCK;
  vmeWrite32(&(DCRBp[id]->ReadoutConfig),
	    vmeRead32(&(DCRBp[id]->ReadoutConfig)) | DCRB_ENABLE_BERR );
  DCRBUNLOCK;
}

void
dcrbGEnableBusError()
{
  int ii;

  DCRBLOCK;
  for(ii=0;ii<ndcrb;ii++) 
    {
      vmeWrite32(&(DCRBp[dcrbID[ii]]->ReadoutConfig),
		vmeRead32(&(DCRBp[dcrbID[ii]]->ReadoutConfig)) | DCRB_ENABLE_BERR );
    }
  DCRBUNLOCK;
}


void
dcrbDisableBusError(int id)
{

  if(id==0) id=dcrbID[0];

  if((id<=0) || (id>21) || (DCRBp[id] == NULL)) 
    {
      logMsg("dcrbDisableBusError: ERROR : DCRB in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }
  
  DCRBLOCK;
  vmeWrite32(&(DCRBp[id]->ReadoutConfig),
	    vmeRead32(&(DCRBp[id]->ReadoutConfig)) & ~DCRB_ENABLE_BERR );
  DCRBUNLOCK;

}


void
dcrbEnableMultiBlock()
{
  int ii, id;

  if((ndcrb <= 1) || (DCRBp[dcrbID[0]] == NULL)) 
    {
      logMsg("dcrbEnableMultiBlock: ERROR : Cannot Enable MultiBlock mode \n",0,0,0,0,0,0);
      return;
    }
  
  for(ii=0;ii<ndcrb;ii++) 
    {
      id = dcrbID[ii];
      DCRBLOCK;
      vmeWrite32(&(DCRBp[id]->ADR32M),
		vmeRead32(&(DCRBp[id]->ADR32M)) | DCRB_ENABLE_MULTIBLOCK );
      DCRBUNLOCK;
      dcrbDisableBusError(id);
      if(id == dcrbMinSlot) 
	{
	  DCRBLOCK;
	  vmeWrite32(&(DCRBp[id]->ADR32M),
		    vmeRead32(&(DCRBp[id]->ADR32M)) | DCRB_FIRST_BOARD );
	  DCRBUNLOCK;
	}
      if(id == dcrbMaxSlot) 
	{
	  DCRBLOCK;
	  vmeWrite32(&(DCRBp[id]->ADR32M),
		    vmeRead32(&(DCRBp[id]->ADR32M)) | DCRB_LAST_BOARD );
	  DCRBUNLOCK;
	  dcrbEnableBusError(id);   /* Enable Bus Error only on Last Board */
	}
    }

}

void
dcrbDisableMultiBlock()
{
  int ii;

  if((ndcrb <= 1) || (DCRBp[dcrbID[0]] == NULL)) 
    {
      logMsg("dcrbDisableMultiBlock: ERROR : Cannot Disable MultiBlock Mode\n",0,0,0,0,0,0);
      return;
    }
  
  DCRBLOCK;
  for(ii=0;ii<ndcrb;ii++)
    vmeWrite32(&(DCRBp[dcrbID[ii]]->ADR32M),
	      vmeRead32(&(DCRBp[dcrbID[ii]]->ADR32M)) & ~DCRB_ENABLE_MULTIBLOCK );
  DCRBUNLOCK;
}



int
dcrbSetBlockLevel(int id, int level)
{
  int rval;

  if(id==0) id=dcrbID[0];

  if((id<=0) || (id>21) || (DCRBp[id] == NULL)) 
    {
      logMsg("dcrbSetBlockLevel: ERROR : DCRB in slot %d is not initialized \n",id,0,0,0,0,0);
      return(ERROR);
    }
  
  if(level<=0) level = 1;
  DCRBLOCK;
  vmeWrite32(&(DCRBp[id]->BlockConfig), level);
  dcrbBlockLevel = level;
  rval = vmeRead32(&(DCRBp[id]->BlockConfig)) & DCRB_BLOCK_LEVEL_MASK;
  DCRBUNLOCK;

  return(rval);

}
void
dcrbGSetBlockLevel(int level)
{
  int ii;

  if(level<=0) level = 1;
  DCRBLOCK;
  for(ii=0;ii<ndcrb;ii++)
    vmeWrite32(&(DCRBp[dcrbID[ii]]->BlockConfig), level);
  DCRBUNLOCK;

  dcrbBlockLevel = level;
}

int
dcrbSetDAC(int id, unsigned int dvalue)
{
  if(id==0) id=dcrbID[0];
  
  if((id<=0) || (id>21) || (DCRBp[id] == NULL)) 
    {
      logMsg("dcrbSetDAC: ERROR : DCRB in slot %d is not initialized \n",id,0,0,0,0,0);
      return(ERROR);
    }
  
  if(dvalue>DCRB_MAX_DAC_VAL) 
    {
      logMsg("dcrbSetDAC: ERROR : DCRB value (%d) out of range (0-%d) \n",
	     dvalue,DCRB_MAX_DAC_VAL,0,0,0,0);
      return(ERROR);
    }
  
  DCRBLOCK;
  vmeWrite32(&DCRBp[id]->DACConfig, (dvalue * 93600) / 1000);
  DCRBUNLOCK;

  return(OK);
}

int
dcrbGetDAC(int id)
{
  if(id==0) id=dcrbID[0];
  
  if((id<=0) || (id>21) || (DCRBp[id] == NULL)) 
  {
    logMsg("dcrbSetDAC: ERROR : DCRB in slot %d is not initialized \n",id,0,0,0,0,0);
    return(ERROR);
  }

  return(vmeRead32(&DCRBp[id]->DACConfig));
}

void
dcrbGSetDAC(unsigned int dvalue)
{
  int ii;
  for(ii=0;ii<ndcrb;ii++)
    dcrbSetDAC(dcrbID[ii], dvalue);
}

int
dcrbSetCalMask(int id, unsigned int dcrbe_mask, unsigned fce_mask)
{
  if(id==0) id=dcrbID[0];
  
  if((id<=0) || (id>21) || (DCRBp[id] == NULL)) 
    {
      logMsg("dcrbSetCalMask: ERROR : DCRB in slot %d is not initialized \n",id,0,0,0,0,0);
      return(ERROR);
    }
  
  if((dcrbe_mask>0x3f) || (fce_mask>0x3f))
    {
      logMsg("dcrbSetDAC: ERROR : DCRB dcrbe_mask/fce_mask (%d/%d) out of range\n",
	     dcrbe_mask,fce_mask,0,0,0,0);
      return(ERROR);
    }
  
  DCRBLOCK;
  vmeWrite32(&DCRBp[id]->TestPulseConfig, fce_mask | (dcrbe_mask<<6));
  DCRBUNLOCK;

  return(OK);
}

void
dcrbGSetCalMask(unsigned int dcrbe_mask, unsigned fce_mask)
{
  int ii;
  for(ii=0;ii<ndcrb;ii++)
    dcrbSetCalMask(dcrbID[ii], dcrbe_mask,fce_mask);
}

void dcrbSelectSpi(int id,int sel)
{
	if(sel)
		vmeWrite32(&DCRBp[id]->SpiFlash, 0x0);
	else
		vmeWrite32(&DCRBp[id]->SpiFlash, 0x4);
}

unsigned char dcrbTransferSpi(int id,unsigned char data)
{
	int i;
	unsigned char rsp = 0;
	for(i = 0; i < 8; i++)
	{
		vmeWrite32(&DCRBp[id]->SpiFlash, ((data>>7)&0x1));	
		rsp = (rsp<<1) | vmeRead32(&DCRBp[id]->SpiFlash) & 0x1;
		vmeWrite32(&DCRBp[id]->SpiFlash, 0x2|((data>>7)&0x1));
		data<<=1;
	}
	return rsp;
}

void dcrbFlashGetId(int id,unsigned char *rsp)
{
	dcrbSelectSpi(id,1);
	dcrbTransferSpi(id,FLASH_CMD_GETID);
	rsp[0] = dcrbTransferSpi(id,0xFF);
	rsp[1] = dcrbTransferSpi(id,0xFF);
	rsp[2] = dcrbTransferSpi(id,0xFF);
	dcrbSelectSpi(id,0);
}

unsigned char dcrbFlashGetStatus(int id, unsigned char cmd)
{
	unsigned char rsp;
	
	dcrbSelectSpi(id,1);
	dcrbTransferSpi(id,cmd);
	rsp = dcrbTransferSpi(id,0xFF);
	dcrbSelectSpi(id,0);
	
	return rsp;
}

int dcrbFirmwareUpdateVerify(int id, const char *filename)
{
	int i, result;

  if(id==0) id=dcrbID[0];
  
  if((id<=0) || (id>21) || (DCRBp[id] == NULL)) 
    {
      printf("dcrbFirmwareUpdateVerify: ERROR : DCRB in slot %d is not initialized \n",id);
      return(ERROR);
    }

	printf("Updating firmware...");
	result = dcrbFirmwareUpdate(id, filename);
	if(result != OK)
	{
		printf("failed.\n");
		return result;
	}
	else
		printf("succeeded.\n");
	
	printf("\nVerifying...");
	result = dcrbFirmwareVerify(id, filename);
	if(result != OK)
	{
		printf("failed.\n");
		return result;
	}
	else
		printf("ok.\n");

	dcrbHardReset(id);
	taskDelay(120);
		
	return OK;
}

void dcrbGFirmwareUpdateVerify(const char *filename)
{
	int ii;
	for(ii = 0; ii < ndcrb; ii++)
		dcrbFirmwareUpdateVerify(dcrbID[ii], filename);
}

int dcrbFirmwareUpdate(int id, const char *filename)
{
	FILE *f;
	int i, flashId = 0;
	unsigned int addr = 0, page = 0;
	unsigned char buf[1056], rspId[3];

  if(id==0) id=dcrbID[0];
  
  if((id<=0) || (id>21) || (DCRBp[id] == NULL)) 
    {
      printf("dcrbFirmwareUpdate: ERROR : DCRB in slot %d is not initialized \n",id);
      return(ERROR);
    }
	
	dcrbSelectSpi(id,0);
	dcrbFlashGetId(id,rspId);
	
	printf("Flash: Mfg=0x%02X, Type=0x%02X, Capacity=0x%02X\n", rspId[0], rspId[1], rspId[2]);

	if( (rspId[0] == FLASH_MFG_WINBOND) &&
	    (rspId[1] == (FLASH_DEVID_W25Q64>>8)) &&
		(rspId[2] == (FLASH_DEVID_W25Q64&0xFF)) )
	{
		f = fopen(filename, "rb");
		if(!f)
		{
			printf("%s: ERROR: dcrbFirmwareUpdate invalid file %s\n", __FUNCTION__, filename);
			return ERROR;
		}
	
		memset(buf, 0xff, 256);
		while(fread(buf, 1, 256, f) > 0)
		{
		  if(!(addr % 65536))		/* sector erase*/
			{
				dcrbSelectSpi(id,1);
				dcrbTransferSpi(id,FLASH_CMD_WREN);	/* write enable*/
				dcrbSelectSpi(id,0);

				dcrbSelectSpi(id,1);
				dcrbTransferSpi(id,FLASH_CMD_ERASE64K);	/* 64k sector erase*/
				dcrbTransferSpi(id,(addr>>16)&0xFF);
				dcrbTransferSpi(id,(addr>>8)&0xFF);
				dcrbTransferSpi(id,(addr)&0xFF);
				dcrbSelectSpi(id,0);

				printf(".");
				i = 0;
				while(1)
				{
					if(!(dcrbFlashGetStatus(id, FLASH_CMD_GETSTATUS) & 0x1))
						break;
					taskDelay(1);
					if(i == 60+6)	/* 1000ms maximum sector erase time*/
					{
						fclose(f);
						printf("%s: ERROR: dcrbFirmwareUpdate failed to erase flash\n", __FUNCTION__);
						return ERROR;
					}
					i++;
				}
			}

			dcrbSelectSpi(id,1);
			dcrbTransferSpi(id,FLASH_CMD_WREN);	/* write enable*/
			dcrbSelectSpi(id,0);

			dcrbSelectSpi(id,1);
			dcrbTransferSpi(id,FLASH_CMD_WRPAGE);	/* write page*/
			dcrbTransferSpi(id,(addr>>16)&0xFF);
			dcrbTransferSpi(id,(addr>>8)&0xFF);
			dcrbTransferSpi(id,(addr)&0xFF);
			for(i = 0; i < 256; i++)
				dcrbTransferSpi(id,buf[i]);
			dcrbSelectSpi(id,0);

			i = 0;
			while(1)
			{
			  if(!(dcrbFlashGetStatus(id, FLASH_CMD_GETSTATUS) & 0x1))	/* no faster than 1us per call*/
					break;
			  if(i == 3000)	/* 3ms maximum page program time*/
				{
					fclose(f);
					printf("%s: ERROR: dcrbFirmwareUpdate failed to program flash\n", __FUNCTION__);
					return ERROR;
				}
				i++;
			}
			memset(buf, 0xff, 256);
			addr+= 256;
		}
		fclose(f);
	}
	else if( (rspId[0] == FLASH_MFG_ATMEL) &&
	         (rspId[1] == (FLASH_DEVID_AT45DB642D>>8)) &&
		     (rspId[2] == (FLASH_DEVID_AT45DB642D&0xFF)) )
	{
		f = fopen(filename, "rb");
		if(!f)
		{
			printf("%s: ERROR: dcrbFirmwareUpdate invalid file %s\n", __FUNCTION__, filename);
			return ERROR;
		}
	
		memset(buf, 0xff, 1056);
		while(fread(buf, 1, 1056, f) > 0)
		{
		  dcrbSelectSpi(id,1);	/* write buffer 1 */
			dcrbTransferSpi(id,FLASH_CMD_WRBUF1);
			dcrbTransferSpi(id,0x00);
			dcrbTransferSpi(id,0x00);
			dcrbTransferSpi(id,0x00);
			for(i = 0; i < 1056; i++)
				dcrbTransferSpi(id,buf[i]);
			dcrbSelectSpi(id,0);

			dcrbSelectSpi(id,1);	/* buffer 1 to flash w/page erase */
			dcrbTransferSpi(id,FLASH_CMD_PGBUF1ERASE);
			dcrbTransferSpi(id, (page>>5) & 0xFF);
			dcrbTransferSpi(id, (page<<3) & 0xFF);
			dcrbTransferSpi(id, 0x00);
			dcrbSelectSpi(id,0);
			
			i = 0;
			while(1)
			{
				if(dcrbFlashGetStatus(id, FLASH_CMD_GETSTATUS2) & 0x80)
					break;
				if(i == 40000)	/* 40ms maximum page program time */
				{
					fclose(f);
					printf("%s: ERROR: dcrbFirmwareUpdate failed to program flash\n", __FUNCTION__);
					return ERROR;
				}
				i++;
			}			
			memset(buf, 0xff, 1056);
			page++;
		}
		fclose(f);
	}
	else
	{
		printf("%s: ERROR: dcrbFirmwareUpdate failed to identify flash id (or device not supported)\n", __FUNCTION__);
		return ERROR;
	}
	return OK;
}

int dcrbFirmwareRead(int id, const char *filename)
{
	FILE *f;
	int i,len, flashId = 0;
	unsigned int addr = 0;
	unsigned char buf[256];
	unsigned char rspId[3];

  if(id==0) id=dcrbID[0];
  
  if((id<=0) || (id>21) || (DCRBp[id] == NULL)) 
    {
      printf("dcrbFirmwareRead: ERROR : DCRB in slot %d is not initialized \n",id);
      return(ERROR);
    }
	
	dcrbSelectSpi(id,0);
	dcrbFlashGetId(id,rspId);
	
	printf("Flash: Mfg=0x%02X, Type=0x%02X, Capacity=0x%02X\n", rspId[0], rspId[1], rspId[2]);

	if( (rspId[0] == FLASH_MFG_WINBOND) &&
	    (rspId[1] == (FLASH_DEVID_W25Q64>>8)) &&
		(rspId[2] == (FLASH_DEVID_W25Q64&0xFF)) )
	{
		f = fopen(filename, "wb");
		if(!f)
		{
			printf("%s: ERROR: dcrbFirmwareRead invalid file %s\n", __FUNCTION__, filename);
			return ERROR;
		}
		
		dcrbSelectSpi(id,1);
		dcrbTransferSpi(id,FLASH_CMD_RD);	/* continuous array read */
		dcrbTransferSpi(id,(addr>>16)&0xFF);
		dcrbTransferSpi(id,(addr>>8)&0xFF);
		dcrbTransferSpi(id,(addr)&0xFF);
		
		for(i = 0; i < FLASH_BYTE_LENGTH; i++)
		{
			fputc(dcrbTransferSpi(id,0xFF), f);
			if(!(i% 65536))
			{
				printf(".");
				taskDelay(1);
			}
		}
			
		dcrbSelectSpi(id,0);
		fclose(f);
	}
	else if( (rspId[0] == FLASH_MFG_ATMEL) &&
	         (rspId[1] == (FLASH_DEVID_AT45DB642D>>8)) &&
		     (rspId[2] == (FLASH_DEVID_AT45DB642D&0xFF)) )
	{
		f = fopen(filename, "wb");
		if(!f)
		{
			printf("%s: ERROR: dcrbFirmwareRead invalid file %s\n", __FUNCTION__, filename);
			return ERROR;
		}
		
		dcrbSelectSpi(id,1);
		dcrbTransferSpi(id,FLASH_CMD_RD);	/* continuous array read */
		dcrbTransferSpi(id,(addr>>16)&0xFF);
		dcrbTransferSpi(id,(addr>>8)&0xFF);
		dcrbTransferSpi(id,(addr)&0xFF);
		
		for(i = 0; i < FLASH_BYTE_LENGTH; i++)
		{
			fputc(dcrbTransferSpi(id,0xFF), f);
			if(!(i% 65536))
			{
				printf(".");
				taskDelay(1);
			}
		}
			
		dcrbSelectSpi(id,0);
		fclose(f);
	}
	else
	{
		printf("%s: ERROR: dcrbFirmwareRead failed to identify flash id 0x%02X\n", __FUNCTION__, flashId);
		return ERROR;
	}
	return OK;
}

int dcrbFirmwareVerify(int id, const char *filename)
{
	FILE *f;
	int i,len, flashId = 0;
	unsigned int addr = 0;
	unsigned char buf[256];
	unsigned char rspId[3], val;
	
  if(id==0) id=dcrbID[0];
  
  if((id<=0) || (id>21) || (DCRBp[id] == NULL)) 
    {
      logMsg("dcrbFirmwareVerify: ERROR : DCRB in slot %d is not initialized \n",id,0,0,0,0,0);
      return(ERROR);
    }
	
	dcrbSelectSpi(id,0);
	dcrbFlashGetId(id,rspId);
	
	printf("Flash: Mfg=0x%02X, Type=0x%02X, Capacity=0x%02X\n", rspId[0], rspId[1], rspId[2]);

	if( (rspId[0] == FLASH_MFG_WINBOND) &&
	    (rspId[1] == (FLASH_DEVID_W25Q64>>8)) &&
		(rspId[2] == (FLASH_DEVID_W25Q64&0xFF)) )
	{
		f = fopen(filename, "rb");
		if(!f)
		{
			printf("%s: ERROR: dcrbFirmwareVerify invalid file %s\n", __FUNCTION__, filename);
			return ERROR;
		}
		
		dcrbSelectSpi(id,1);
		dcrbTransferSpi(id,FLASH_CMD_RD);	/* continuous array read */
		dcrbTransferSpi(id,(addr>>16)&0xFF);
		dcrbTransferSpi(id,(addr>>8)&0xFF);
		dcrbTransferSpi(id,(addr)&0xFF);

		while((len = fread(buf, 1, 256, f)) > 0)
		{
			for(i = 0; i < len; i++)
			{
				val = dcrbTransferSpi(id,0xFF);
				if(buf[i] != val)
				{
					dcrbSelectSpi(id,0);
					fclose(f);					
					printf("%s: ERROR: dcrbFirmwareVerify failed verify at addess 0x%08X[%02X,%02X]\n", __FUNCTION__, addr+i, buf[i], val);
					return ERROR;
				}
			}
			addr+=256;
			if(!(addr & 0xFFFF))
				printf(".");
		}
		dcrbSelectSpi(id,0);
		fclose(f);
	}
	else if( (rspId[0] == FLASH_MFG_ATMEL) &&
	         (rspId[1] == (FLASH_DEVID_AT45DB642D>>8)) &&
		     (rspId[2] == (FLASH_DEVID_AT45DB642D&0xFF)) )
	{
		f = fopen(filename, "rb");
		if(!f)
		{
			printf("%s: ERROR: dcrbFirmwareVerify invalid file %s\n", __FUNCTION__, filename);
			return ERROR;
		}
		
		dcrbSelectSpi(id,1);
		dcrbTransferSpi(id,FLASH_CMD_RD);	/* continuous array read */
		dcrbTransferSpi(id,(addr>>16)&0xFF);
		dcrbTransferSpi(id,(addr>>8)&0xFF);
		dcrbTransferSpi(id,(addr)&0xFF);

		while((len = fread(buf, 1, 256, f)) > 0)
		{
			for(i = 0; i < len; i++)
			{
				val = dcrbTransferSpi(id,0xFF);
				if(buf[i] != val)
				{
					dcrbSelectSpi(id,0);
					fclose(f);					
					printf("%s: ERROR: dcrbFirmwareVerify failed verify at addess 0x%08X[%02X,%02X]\n", __FUNCTION__, addr+i, buf[i], val);
					return ERROR;
				}
			}
			addr+=256;
			if(!(addr & 0xFFFF))
				printf(".");
		}
		dcrbSelectSpi(id,0);
		fclose(f);
	}
	else
	{
		printf("%s: ERROR: dcrbFirmwareVerify failed to identify flash id 0x%02X\n", __FUNCTION__, flashId);
		return ERROR;
	}
	return OK;
}

void 
dcrbDataDecode(unsigned int data)
{
	printf("0x%08X", data);

	if(data & 0x80000000)
	{
		int type = data & 0x78000000;
		switch(type)
		{
			case DCRB_DATA_BLOCK_HEADER:
				printf(" {BLKHDR} SLOTID: %d", (data>>22)&0x1f);
				printf(" NEVENTS: %d", (data>>11)&0x7ff);
				printf(" BLOCK: %d\n", (data>>0)&0x7ff);				
				break;
			case DCRB_DATA_BLOCK_TRAILER:
				printf(" {BLKTLR} SLOTID: %d", (data>>22)&0x1f);
				printf(" NWORDS: %d\n", (data>>0)&0x3fffff);
				break;
			case DCRB_DATA_EVENT_HEADER:
				printf(" {EVTHDR} EVENT: %d\n", (data>>0)&0x7ffffff);
				break;
			case DCRB_DATA_TRIGGER_TIME:
				printf(" {TRGTIME}\n");
				break;
			case DCRB_DATA_TDCEVT:
				printf(" {TDCEVT} CH: %d", (data>>16)&0x7f);
				printf(" TIME: %dns\n", (data>>0)&0xFFFF);
				break;
			case DCRB_DATA_INVALID:
				printf(" {***DNV***}\n");
				break;
			case DCRB_DATA_FILLER:
				printf(" {FILLER}\n");
				break;
			default:
				printf(" {***DATATYPE ERROR***}\n");
				break;
		}
	}
	else
		printf("\n");


/*
  int i_print = 1;
  static unsigned int type_last = 15;
  static unsigned int time_last = 0;

  if( data & 0x80000000 )
    {
      fadc_data.new_type = 1;
      fadc_data.type = (data & 0x78000000) >> 27;
    }
  else
    {
      fadc_data.new_type = 0;
      fadc_data.type = type_last;
    }
        
  switch( fadc_data.type )
    {
    case 0:
      fadc_data.slot_id_hd = (data & 0x7C00000) >> 22;
      fadc_data.n_evts = (data & 0x3FF800) >> 11;
      fadc_data.blk_num = (data & 0x7FF);
      if( i_print ) 
	printf("%8X - BLOCK HEADER - slot = %d   n_evts = %d   n_blk = %d\n",
	       data, fadc_data.slot_id_hd, fadc_data.n_evts, fadc_data.blk_num);
      break;
    case 1:
      fadc_data.slot_id_tr = (data & 0x7C00000) >> 22;
      fadc_data.n_words = (data & 0x3FFFFF);
      if( i_print ) 
	printf("%8X - BLOCK TRAILER - slot = %d   n_words = %d\n",
	       data, fadc_data.slot_id_tr, fadc_data.n_words);
      break;
    case 2:
      if( fadc_data.new_type )
	{
	  fadc_data.evt_num_1 = (data & 0x7FFFFFF);
	  if( i_print ) 
	    printf("%8X - EVENT HEADER 1 - evt_num = %d\n", data, fadc_data.evt_num_1);
	}    
      else
	{
	  fadc_data.evt_num_2 = (data & 0x7FFFFFF);
	  if( i_print ) 
	    printf("%8X - EVENT HEADER 2 - evt_num = %d\n", data, fadc_data.evt_num_2);
	}
      break;
    case 3:
      if( fadc_data.new_type )
	{
	  fadc_data.time_1 = (data & 0xFFFFFF);
	  if( i_print ) 
	    printf("%8X - TRIGGER TIME 1 - time = %08x\n", data, fadc_data.time_1);
	  fadc_data.time_now = 1;
	  time_last = 1;
	}    
      else
	{
	  if( time_last == 1 )
	    {
	      fadc_data.time_2 = (data & 0xFFFFFF);
	      if( i_print ) 
		printf("%8X - TRIGGER TIME 2 - time = %08x\n", data, fadc_data.time_2);
	      fadc_data.time_now = 2;
	    }    
	  else if( time_last == 2 )
	    {
	      fadc_data.time_3 = (data & 0xFFFFFF);
	      if( i_print ) 
		printf("%8X - TRIGGER TIME 3 - time = %08x\n", data, fadc_data.time_3);
	      fadc_data.time_now = 3;
	    }    
	  else if( time_last == 3 )
	    {
	      fadc_data.time_4 = (data & 0xFFFFFF);
	      if( i_print ) 
		printf("%8X - TRIGGER TIME 4 - time = %08x\n", data, fadc_data.time_4);
	      fadc_data.time_now = 4;
	    }    
	  else
	    if( i_print ) 
	      printf("%8X - TRIGGER TIME - (ERROR)\n", data);
	                
	  time_last = fadc_data.time_now;
	}    
      break;
    case 4:
      if( fadc_data.new_type )
	{
	  fadc_data.chan = (data & 0x7800000) >> 23;
	  fadc_data.width = (data & 0xFFF);
	  if( i_print ) 
	    printf("%8X - WINDOW RAW DATA - chan = %d   nsamples = %d\n", 
		   data, fadc_data.chan, fadc_data.width);
	}    
      else
	{
	  fadc_data.valid_1 = 1;
	  fadc_data.valid_2 = 1;
	  fadc_data.adc_1 = (data & 0x1FFF0000) >> 16;
	  if( data & 0x20000000 )
	    fadc_data.valid_1 = 0;
	  fadc_data.adc_2 = (data & 0x1FFF);
	  if( data & 0x2000 )
	    fadc_data.valid_2 = 0;
	  if( i_print ) 
	    printf("%8X - RAW SAMPLES - valid = %d  adc = %4d   valid = %d  adc = %4d\n", 
		   data, fadc_data.valid_1, fadc_data.adc_1, 
		   fadc_data.valid_2, fadc_data.adc_2);
	}    
      break;
    case 5:
      fadc_data.over = 0; 
      fadc_data.chan = (data & 0x7800000) >> 23;
      fadc_data.adc_sum = (data & 0x3FFFFF);
      if( data & 0x400000 )
	fadc_data.over = 1;
      if( i_print ) 
	printf("%8X - WINDOW SUM - chan = %d   over = %d   adc_sum = %08x\n",
	       data, fadc_data.chan, fadc_data.over, fadc_data.adc_sum);
      break;
    case 6:
      if( fadc_data.new_type )
	{
	  fadc_data.chan = (data & 0x7800000) >> 23;
	  fadc_data.pulse_num = (data & 0x600000) >> 21;
	  fadc_data.thres_bin = (data & 0x3FF);
	  if( i_print ) 
	    printf("%8X - PULSE RAW DATA - chan = %d   pulse # = %d   threshold bin = %d\n", 
		   data, fadc_data.chan, fadc_data.pulse_num, fadc_data.thres_bin);
	}    
      else
	{
	  fadc_data.valid_1 = 1;
	  fadc_data.valid_2 = 1;
	  fadc_data.adc_1 = (data & 0x1FFF0000) >> 16;
	  if( data & 0x20000000 )
	    fadc_data.valid_1 = 0;
	  fadc_data.adc_2 = (data & 0x1FFF);
	  if( data & 0x2000 )
	    fadc_data.valid_2 = 0;
	  if( i_print ) 
	    printf("%8X - PULSE RAW SAMPLES - valid = %d  adc = %d   valid = %d  adc = %d\n", 
		   data, fadc_data.valid_1, fadc_data.adc_1, 
		   fadc_data.valid_2, fadc_data.adc_2);
	}    
      break;
    case 7:
      fadc_data.chan = (data & 0x7800000) >> 23;
      fadc_data.pulse_num = (data & 0x600000) >> 21;
      fadc_data.quality = (data & 0x180000) >> 19;
      fadc_data.integral = (data & 0x7FFFF);
      if( i_print ) 
	printf("%8X - PULSE INTEGRAL - chan = %d   pulse # = %d   quality = %d   integral = %d\n", 
	       data, fadc_data.chan, fadc_data.pulse_num, 
	       fadc_data.quality, fadc_data.integral);
      break;
    case 8:
      fadc_data.chan = (data & 0x7800000) >> 23;
      fadc_data.pulse_num = (data & 0x600000) >> 21;
      fadc_data.quality = (data & 0x180000) >> 19;
      fadc_data.time = (data & 0xFFFF);
      if( i_print ) 
	printf("%8X - PULSE TIME - chan = %d   pulse # = %d   quality = %d   time = %d\n", 
	       data, fadc_data.chan, fadc_data.pulse_num, 
	       fadc_data.quality, fadc_data.time);
      break;
    case 9:
      if( fadc_data.new_type )
	{
	  fadc_data.chan_a = (data & 0x3C00000) >> 22;
	  fadc_data.source_a = (data & 0x4000000) >> 26;
	  fadc_data.chan_b = (data & 0x1E0000) >> 17;
	  fadc_data.source_b = (data & 0x200000) >> 21;
	  if( i_print ) 
	    printf("%8X - STREAMING RAW DATA - ena A = %d  chan A = %d   ena B = %d  chan B = %d\n", 
		   data, fadc_data.source_a, fadc_data.chan_a, 
		   fadc_data.source_b, fadc_data.chan_b);
	}    
      else
	{
	  fadc_data.valid_1 = 1;
	  fadc_data.valid_2 = 1;
	  fadc_data.adc_1 = (data & 0x1FFF0000) >> 16;
	  if( data & 0x20000000 )
	    fadc_data.valid_1 = 0;
	  fadc_data.adc_2 = (data & 0x1FFF);
	  if( data & 0x2000 )
	    fadc_data.valid_2 = 0;
	  fadc_data.group = (data & 0x40000000) >> 30;
	  if( fadc_data.group )
	    {
	      if( i_print ) 
		printf("%8X - RAW SAMPLES B - valid = %d  adc = %d   valid = %d  adc = %d\n", 
		       data, fadc_data.valid_1, fadc_data.adc_1, 
		       fadc_data.valid_2, fadc_data.adc_2);
	    }		 
	  else
	    if( i_print ) 
	      printf("%8X - RAW SAMPLES A - valid = %d  adc = %d   valid = %d  adc = %d\n", 
		     data, fadc_data.valid_1, fadc_data.adc_1, 
		     fadc_data.valid_2, fadc_data.adc_2);	            
	}    
      break;
    case 10:
      fadc_data.chan = (data & 0x7800000) >> 23;
      fadc_data.pulse_num = (data & 0x600000) >> 21;
      fadc_data.vmin = (data & 0x1FF000) >> 12;
      fadc_data.vpeak = (data & 0xFFF);
      if( i_print ) 
	printf("%8X - PULSE V - chan = %d   pulse # = %d   vmin = %d   vpeak = %d\n", 
	       data, fadc_data.chan, fadc_data.pulse_num, 
	       fadc_data.vmin, fadc_data.vpeak);
      break;

    case 11:
      fadc_data.trig_type_int = data & 0x7;
      fadc_data.trig_state_int = (data & 0x8) >> 3;
      fadc_data.evt_num_int = (data & 0xFFF0) >> 4;
      fadc_data.err_status_int = (data & 0x10000) >> 16;
      if( i_print ) 
	printf("%8X - INTERNAL TRIGGER - type = %d   state = %d   num = %d   error = %d\n",
	       data, fadc_data.trig_type_int, fadc_data.trig_state_int, fadc_data.evt_num_int,
	       fadc_data.err_status_int);
    case 12:
      if( i_print ) 
	printf("%8X - UNDEFINED TYPE = %d\n", data, fadc_data.type);
      break;
    case 13:
      if( i_print ) 
	printf("%8X - END OF EVENT = %d\n", data, fadc_data.type);
      break;
    case 14:
      if( i_print ) 
	printf("%8X - DATA NOT VALID = %d\n", data, fadc_data.type);
      break;
    case 15:
      if( i_print ) 
	printf("%8X - FILLER WORD = %d\n", data, fadc_data.type);
      break;
    }
	
  type_last = fadc_data.type;
*/

  return;
}


void
dcrbSoftTrig(int id)
{
  if(id==0) id=dcrbID[0];

  if((id<=0) || (id>21) || (DCRBp[id] == NULL)) 
  {
    printf("dcrbSoftTrig: ERROR : DCRB in slot %d is not initialized \n",id);
    return;
  }

  vmeWrite32(&(DCRBp[id]->TriggerSource),DCRB_TRIG_VME);

}


void
dcrbPrintScalers(int id)
{
  unsigned int scaler[6];

  if(id==0) id=dcrbID[0];

  if((id<=0) || (id>21) || (DCRBp[id] == NULL)) 
  {
    printf("dcrbPrintScalers: ERROR : DCRB in slot %d is not initialized \n",id);
    return;
  }

  vmeWrite32(&(DCRBp[id]->ScalerLatch), 0xFFFFFFFF);

  scaler[0] = vmeRead32(&(DCRBp[id]->ScalerBusy));
  scaler[1] = vmeRead32(&(DCRBp[id]->ScalerBusyCycles));
  scaler[2] = vmeRead32(&(DCRBp[id]->ScalerVmeClk));
  scaler[3] = vmeRead32(&(DCRBp[id]->ScalerSync));
  scaler[4] = vmeRead32(&(DCRBp[id]->ScalerTrig1));
  scaler[5] = vmeRead32(&(DCRBp[id]->ScalerTrig2));

  printf("\n Slot %d scalers\n",id);

  printf("ScalerBusy       %10u\n",scaler[0]);
  printf("ScalerBusyCycles %10u\n",scaler[1]);
  printf("ScalerVmeClk     %10u\n",scaler[2]);
  printf("ScalerSync       %10u\n",scaler[3]);
  printf("ScalerTrig1      %10u\n",scaler[4]);
  printf("ScalerTrig2      %10u\n",scaler[5]);

  printf("ScalerTDCs      %10u %10u %10u\n",
		 vmeRead32(&(DCRBp[id]->ScalerTDC[0])),
         vmeRead32(&(DCRBp[id]->ScalerTDC[1])),
         vmeRead32(&(DCRBp[id]->ScalerTDC[2])) );

  printf("\n Slot %d FIFO: nevents=%u nwords=%u\n",id,vmeRead32(&(DCRBp[id]->FifoEventCnt)),vmeRead32(&(DCRBp[id]->FifoWordCnt)));
}


/* width in ns */
int
dcrbTriggerPulseWidth(int id, unsigned int width)
{
  if(id==0) id=dcrbID[0];

  if((id<=0) || (id>21) || (DCRBp[id] == NULL)) 
  {
    printf("dcrbSoftTrig: ERROR : DCRB in slot %d is not initialized \n",id);
    return;
  }

  width /= 8;

  vmeWrite32(&(DCRBp[id]->TriggerCtrl),width);

  return(width);
}


/* returns 1 if link is UP, 0, otherwise */
int
dcrbLinkStatus(int id)
{
  if(id==0) id=dcrbID[0];

  if((id<=0) || (id>21) || (DCRBp[id] == NULL)) 
  {
    printf("dcrbSoftTrig: ERROR : DCRB in slot %d is not initialized \n",id);
    return;
  }

  return( ( (vmeRead32(&(DCRBp[id]->GtpStatus)))&0x1000 )>>12 );
}

int
dcrbLinkReset(int id)
{
  if(id==0) id=dcrbID[0];

  if((id<=0) || (id>21) || (DCRBp[id] == NULL)) 
  {
    printf("dcrbSoftTrig: ERROR : DCRB in slot %d is not initialized \n",id);
    return;
  }

  vmeWrite32(&(DCRBp[id]->GtpCtrl), 0x203);
  taskDelay(1);
  vmeWrite32(&(DCRBp[id]->GtpCtrl), 0x202);
  taskDelay(1);
  vmeWrite32(&(DCRBp[id]->GtpCtrl), 0x200);
  taskDelay(1);
  vmeWrite32(&(DCRBp[id]->GtpCtrl), 0);
  taskDelay(10);


  return(0);
}










/**** CONFIG ************************************/
typedef struct
{
  int threshold[DCRB_MAX_BOARDS];

} DCRB_CONFIG_STRUCT;

DCRB_CONFIG_STRUCT conf;

void
dcrbInitGlobals()
{
  int ii;

  for(ii=0; ii<DCRB_MAX_BOARDS; ii++) conf.threshold[ii] = 20;
 
  return;
}



#define NCHAN 8
#define SCAN_MSK \
	args = sscanf (str_tmp, "%*s %d %d %d %d %d %d %d %d   \
                                     %d %d %d %d %d %d %d %d", \
		       &msk[ 0], &msk[ 1], &msk[ 2], &msk[ 3], \
		       &msk[ 4], &msk[ 5], &msk[ 6], &msk[ 7], \
		       &msk[ 8], &msk[ 9], &msk[10], &msk[11], \
		       &msk[12], &msk[13], &msk[14], &msk[15])


int
dcrbReadConfigFile(char *filename)
{
  FILE   *fd;
  char   fname[FNLEN] = { "" };  /* config file name */
  int    ii, jj, ch;
  char   str_tmp[STRLEN], keyword[ROCLEN];
  char   host[ROCLEN], ROC_name[ROCLEN];
  char   str2[2];
  int    args, i1, i2, i3, i4, msk[NCHAN];
  float  f1;
  int    slot1, slot2, slot, chan;
  unsigned int  ui1, ui2;
  char *getenv();
  char *clonparms;
  char *expid;

  clonparms = getenv("CLON_PARMS");
  expid = getenv("EXPID");
  if(strlen(filename)!=0) /* filename specified */
  {
    if ( filename[0]=='/' || (filename[0]=='.' && filename[1]=='/') )
	{
      sprintf(fname, "%s", filename);
	}
    else
	{
      sprintf(fname, "%s/dcrb/%s", clonparms, filename);
	}

    if((fd=fopen(fname,"r")) == NULL)
    {
      printf("\ndcrbReadConfigFile: Can't open config file >%s<\n",fname);
      return(-1);
    }
  }
  else /* filename does not specified */
  {
    /* obtain our hostname */
    gethostname(host,ROCLEN);
    sprintf(fname, "%s/dcrb/%s.cnf", clonparms, host);
    if((fd=fopen(fname,"r")) == NULL)
    {
      sprintf(fname, "%s/dcrb/%s.cnf", clonparms, expid);
      if((fd=fopen(fname,"r")) == NULL)
      {
        printf("\ndcrbReadConfigFile: Can't open config file >%s<\n",fname);
        return(-2);
	  }
	}

  }
  printf("\ndcrbReadConfigFile: Using configuration file >%s<\n",fname);

  /* Parsing of config file */
  active = 0;
  while ((ch = getc(fd)) != EOF)
  {
    if ( ch == '#' || ch == ' ' || ch == '\t' )
    {
      while (getc(fd) != '\n') {}
    }
    else if( ch == '\n' ) {}
    else
    {
      ungetc(ch,fd);
      fgets(str_tmp, STRLEN, fd);
      sscanf (str_tmp, "%s %s", keyword, ROC_name);


      /* Start parsing real config inputs */
      if(strcmp(keyword,"DCRB_CRATE") == 0)
      {
	    if(strcmp(ROC_name,host) == 0)
        {
	      printf("\nReadConfigFile: crate = %s  host = %s - activated\n",ROC_name,host);
          active = 1;
        }
	    else if(strcmp(ROC_name,"all") == 0)
		{
	      printf("\nReadConfigFile: crate = %s  host = %s - activated\n",ROC_name,host);
          active = 1;
		}
        else
		{
	      printf("\nReadConfigFile: crate = %s  host = %s - disactivated\n",ROC_name,host);
          active = 0;
		}
      }

      else if(active && (strcmp(keyword,"DCRB_SLOT")==0))
      {
        sscanf (str_tmp, "%*s %s", str2);
        if(isdigit(str2[0]))
        {
          slot1 = atoi(str2);
          slot2 = slot1 + 1;
          if(slot1<2 && slot1>21)
          {
            printf("\nReadConfigFile: Wrong slot number %d\n\n",slot1);
            return(-1);
          }
        }
        else if(!strcmp(str2,"all"))
        {
          slot1 = 0;
          slot2 = DCRB_MAX_BOARDS;
        }
        else
        {
          printf("\nReadConfigFile: Wrong slot >%s<, must be 'all' or actual slot number\n\n",str2);
          return(-1);
        }
	  }

      else if(active && (strcmp(keyword,"DCRB_THRESHOLD")==0))
      {
        sscanf (str_tmp, "%*s %d", &i1);
		for(slot=slot1; slot<slot2; slot++) conf.threshold[slot] = i1;
      }

      else
      {
        ; /* unknown key - do nothing */
		/*
        printf("dcrbReadConfigFile: Unknown Field or Missed Field in\n");
        printf("   %s \n", fname);
        printf("   str_tmp=%s", str_tmp);
        printf("   keyword=%s \n\n", keyword);
        return(-10);
		*/
      }

    }
  } /* end of while */

  fclose(fd);

  return(0);
}

int
dcrbDownloadAll()
{
  int id, slot;

  for(id=0; id<ndcrb; id++)
  {
    slot = dcrbSlot(id);

    dcrbSetDAC(slot, conf.threshold[slot]);
  }

  return(0);
}

/* dcrbInit() have to be called before this function */
int  
dcrbConfig(char *fname)
{
  int res;
  char *string; /*dummy, will not be used*/

  if(strlen(fname) > 0) /* filename specified  - upload initial settings from the hardware */
  {
    dcrbUploadAll(string, 0);
  }
  else /* filename not specified  - set defaults */
  {
    dcrbInitGlobals();
  }

  /* read config file */
  if( (res = dcrbReadConfigFile(fname)) < 0 ) return(res);

  /* download to all boards */
  dcrbDownloadAll();

  return(0);
}

void
dcrbMon(int slot)
{
  int id;

}



/* upload setting from all found DSC2s */
int
dcrbUploadAll(char *string, int length)
{
  int id, slot, i, ii, jj, kk, ifiber, len1, len2;
  char *str, sss[1024];
  unsigned int tmp, connectedfibers;
  unsigned short sval;
  unsigned short bypMask;
  unsigned short channels[8];

  for(id=0; id<ndcrb; id++)
  {
    slot = dcrbSlot(id);

    conf.threshold[slot] = dcrbGetDAC(slot);
    printf("DCRB DAC slot=%d threshold=%d\n",slot,conf.threshold[slot]);
  }

  if(length)
  {
    str = string;
    str[0] = '\0';

    sprintf(sss,"DCRB_THRESHOLD %d\n", conf.threshold[slot]); ADD_TO_STRING;

    CLOSE_STRING;
  }

}

int
dcrbUploadAllPrint()
{
  char str[1025];
  dcrbUploadAll(str, 1024);
  printf("%s",str);
}













#else /* dummy version*/

void
dcrbLib_dummy()
{
  return;
}

#endif
