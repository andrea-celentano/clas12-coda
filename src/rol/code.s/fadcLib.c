/********************************************************************************
 *  fadcLib.c  -  Driver library for JLAB config and readout of JLAB 250MHz FLASH
 *                  ADC using a VxWorks >=5.4 or Linux >=2.6.18 based Single 
 *                  Board computer. 
 *
 *  Author: David Abbott & Bryan Moffit
 *          Jefferson Lab Data Acquisition Group
 *          June 2007
 *
 *  Revision  2.0 - Initial Revision for FADC V2
 *                    - Supports up to 20 FADC boards in a Crate
 *                    - Programmed I/O and Block reads
 *
 *  SVN: $Rev: 529 $
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

#define HPS

/* Include ADC definitions */
#include "fadcLib.h"

#ifdef VXWORKS
#define FALOCK
#define FAUNLOCK
#define FASDCLOCK
#define FASDCUNLOCK
#else
/* Mutex to guard flexio read/writes */
pthread_mutex_t   faMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t   fasdcMutex = PTHREAD_MUTEX_INITIALIZER;
#define FALOCK      if(pthread_mutex_lock(&faMutex)<0) perror("pthread_mutex_lock");
#define FAUNLOCK    if(pthread_mutex_unlock(&faMutex)<0) perror("pthread_mutex_unlock");
#define FASDCLOCK   if(pthread_mutex_lock(&fasdcMutex)<0) perror("pthread_mutex_lock");
#define FASDCUNLOCK if(pthread_mutex_unlock(&fasdcMutex)<0) perror("pthread_mutex_unlock");
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
IMPORT  STATUS sysVmeDmaDone(int, int);
IMPORT  STATUS sysVmeDmaSend(UINT32, UINT32, int, BOOL);

#define EIEIO    __asm__ volatile ("eieio")
#define SYNC     __asm__ volatile ("sync")
#endif

/* Define Interrupts variables */
BOOL              fadcIntRunning  = FALSE;                    /* running flag */
int               fadcIntID       = -1;                       /* id number of ADC generating interrupts */
LOCAL VOIDFUNCPTR fadcIntRoutine  = NULL;                     /* user interrupt service routine */
LOCAL int         fadcIntArg      = 0;                        /* arg to user routine */
LOCAL UINT32      fadcIntLevel    = FA_VME_INT_LEVEL;         /* default VME interrupt level */
LOCAL UINT32      fadcIntVec      = FA_VME_INT_VEC;           /* default interrupt Vector */

/* Define global variables */
int nfadc = 0;                                       /* Number of FADCs in Crate */
int fadcA32Base   = 0x08000000;                      /* Minimum VME A32 Address for use by FADCs */
int fadcA32Offset = 0x08000000;                      /* Difference in CPU A32 Base - VME A32 Base */
int fadcA24Offset = 0x0;                             /* Difference in CPU A24 Base - VME A24 Base */
int fadcA16Offset = 0x0;                             /* Difference in CPU A16 Base - VME A16 Base */
volatile struct fadc_struct *FAp[(FA_MAX_BOARDS+1)]; /* pointers to FADC memory map */
volatile struct fadc_sdc_struct *FASDCp;             /* pointer to FADC Signal distribution card */
volatile unsigned int *FApd[(FA_MAX_BOARDS+1)];      /* pointers to FADC FIFO memory */
volatile unsigned int *FApmb;                        /* pointer to Multblock window */
int fadcID[FA_MAX_BOARDS];                           /* array of slot numbers for FADCs */
unsigned int fadcAddrList[FA_MAX_BOARDS];            /* array of a24 addresses for FADCs */
int fadcRev[(FA_MAX_BOARDS+1)];                      /* Board Revision Info for each module */
unsigned short fadcChanDisable[(FA_MAX_BOARDS+1)];   /* Disabled Channel Mask for each Module*/
int fadcInited=0;                                    /* >0 if Library has been Initialized before */
int fadcMaxSlot=0;                                   /* Highest Slot hold an FADC */
int fadcMinSlot=0;                                   /* Lowest Slot holding an FADC */
int fadcSource=0;                                    /* Signal source for FADC system control*/
int fadcBlockLevel=0;                                /* Block Level for ADCs */
int fadcIntCount = 0;                                /* Count of interrupts from FADC */
int fadcUseSDC=0;                                    /* If > 0 then Use Signal Distribution board */
struct fadc_data_struct fadc_data;
int fadcBlockError=0;       /* Whether (1) or not (0) Block Transfer had an error */

/* Include Firmware Tools */
#include "cinclude/fadcFirmwareTools.c"


/*******************************************************************************
 *
 * faInit - Initialize JLAB FADC Library. 
 *
 *
 *   iFlag: 16 bit integer
 *       Low 6 bits - Specifies the default Signal distribution (clock,trigger) 
 *                    sources for the board (INTernal, FrontPanel, VXS, VME(Soft))
 *       bit    0:  defines Sync Reset source
 *                     0  VME (Software Sync-Reset)
 *                     1  Front Panel/VXS/P2 (Depends on Clk/Trig source selection)
 *       bits 3-1:  defines Trigger source
 *               0 0 0  VME (Software Triggers)
 *               0 0 1  Front Panel Input
 *               0 1 0  VXS (P0) 
 *               1 0 0  Internal Trigger Logic (HITSUM FPGA)
 *               (all others Undefined - default to Internal)
 *       bits 5-4:  defines Clock Source
 *           0 0  Internal 250MHz Clock
 *           0 1  Front Panel 
 *           1 0  VXS (P0)
 *           1 1  P2 Connector (Blackplane)
 *
 *       Common Modes of Operation:
 *           Value = 0  CLK (Int)  TRIG (Soft)   SYNC (Soft)    (Debug/Test Mode)
 *                   2  CLK (Int)  TRIG (FP)     SYNC (Soft)    (Single Board
 *                   3  CLK (Int)  TRIG (FP)     SYNC (FP)         Modes)
 *                0x10  CLK (FP)   TRIG (Soft)   SYNC (Soft)
 *                0x13  CLK (FP)   TRIG (FP)     SYNC (FP)      (VME SDC Mode)
 *                0x20  CLK (VXS)  TRIG (Soft)   SYNC (Soft)
 *                0x25  CLK (VXS)  TRIG (VXS)    SYNC (VXS)     (VXS SD Mode)
 *
 *
 *      High 10bits - A16 Base address of FADC Signal Distribution Module
 *                    This board can control up to 7 FADC Boards.
 *                    Clock Source must be set to Front Panel (bit4 = 1)
 *
 *      bit 16:  Exit before board initialization
 *             0 Initialize FADC (default behavior)
 *             1 Skip initialization (just setup register map pointers)
 *
 *      bit 17:  Use fadcAddrList instead of addr and addr_inc
 *               for VME addresses.
 *             0 Initialize with addr and addr_inc
 *             1 Use fadcAddrList 
 *      
 *
 * RETURNS: OK, or ERROR if the address is invalid or a board is not present.
 */

STATUS 
faInit (UINT32 addr, UINT32 addr_inc, int nadc, int iFlag)
{
  int ii, res, errFlag = 0;
  int boardID = 0;
  int maxSlot = 1;
  int minSlot = 21;
  int trigSrc=0, clkSrc=0, srSrc=0;
  unsigned int rdata, laddr, laddr_inc, a32addr, a16addr=0;
  volatile struct fadc_struct *fa;
  unsigned short sdata;
  int noBoardInit=0;
  int useList=0;

  /* Check if we have already Initialized boards before */
  if((fadcInited>0) && (fadcID[0] != 0)) 
    {
      /* Hard Reset of all FADC boards in the Crate */
      for(ii=0;ii<nfadc;ii++) 
	{
	  vmeWrite32(&(FAp[fadcID[ii]]->csr),FA_CSR_HARD_RESET);
	}
      taskDelay(5);
    }
  
  /* Check if we're initializing using a list */
  useList=(iFlag&(1<<17))>>17;

  /* Check for valid address */
  if(addr==0) 
    {
      printf("faInit: ERROR: Must specify a Bus (VME-based A24) address for FADC 0\n");
      return(ERROR);
    }
  else if(addr > 0x00ffffff) 
    { /* A24 Addressing */
      printf("faInit: ERROR: A32 Addressing not allowed for FADC configuration space\n");
      return(ERROR);
    }
  else
    { /* A24 Addressing */
      if( ((addr_inc==0)||(nadc==0)) && (useList==0) )
	nadc = 1; /* assume only one FADC to initialize */

      /* get the FADC address */
#ifdef VXWORKS
      res = sysBusToLocalAdrs(0x39,(char *)addr,(char **)&laddr);
#else
      res = vmeBusToLocalAdrs(0x39,(char *)addr,(char **)&laddr);
#endif
      if (res != 0) 
	{
#ifdef VXWORKS
	  printf("faInit: ERROR in sysBusToLocalAdrs(0x39,0x%x,&laddr) \n",addr);
#else
	  printf("faInit: ERROR in vmeBusToLocalAdrs(0x39,0x%x,&laddr) \n",addr);
#endif
	  return(ERROR);
	}
      fadcA24Offset = laddr - addr;
    }

  /* Init Some Global variables */
  fadcSource = iFlag&FA_SOURCE_MASK;
  fadcInited = nfadc = 0;
  fadcUseSDC = 0;
  bzero((char *)fadcChanDisable,sizeof(fadcChanDisable));
  bzero((char *)fadcID,sizeof(fadcID));

  for (ii=0;ii<nadc;ii++) 
    {
      if(useList==1)
	{
	  laddr_inc = fadcAddrList[ii] + fadcA24Offset;
	}
      else
	{
	  laddr_inc = laddr +ii*addr_inc;
	}
      fa = (struct fadc_struct *)laddr_inc;
      /* Check if Board exists at that address */
#ifdef VXWORKS
      res = vxMemProbe((char *) &(fa->version),VX_READ,4,(char *)&rdata);
#else
      res = vmeMemProbe((char *) &(fa->version),4,(char *)&rdata);
#endif
      if(res < 0) 
	{
#ifdef VXWORKS
	  printf("faInit: ERROR: No addressable board at addr=0x%x\n",(UINT32) fa);
#else
	  printf("faInit: ERROR: No addressable board at VME (Local) addr=0x%x (0x%x)\n",
		 (UINT32) laddr_inc-fadcA24Offset, (UINT32) fa);
#endif
	  errFlag = 1;
/* 	  break; */
	}
      else 
	{
	  /* Check that it is an FA board */
	  if((rdata&FA_BOARD_MASK) != FA_BOARD_ID) 
	    {
	      printf(" ERROR: For board at 0x%x, Invalid Board ID: 0x%x\n",
		     (UINT32) fa, rdata);
/* 	      return(ERROR); */
	    }
	  /* Check if this is board has a valid slot number */
	  boardID =  ((vmeRead32(&(fa->intr)))&FA_SLOT_ID_MASK)>>16;
	  if( ((rdata&FA_VERSION_MASK)==0x01) | ((rdata&FA_VERSION_MASK)==0x07)
	      | ((rdata&FA_VERSION_MASK)==0x08) | ((rdata&FA_VERSION_MASK)==0x09) )
	    {
	      /* FIXME: Flip bits for Firmware version 0x0201, 0x0207, 0x0208. */
	      boardID = (~boardID) &0x1F;
	      printf("%s: using boardID bit flip (0x%x)\n",__FUNCTION__,boardID);
	    }
	  if((boardID <= 0)||(boardID >21)) 
	    {
	      printf(" ERROR: Board Slot ID is not in range: %d\n",boardID);
	      continue;
/* 	      return(ERROR); */
	    }
	  FAp[boardID] = (struct fadc_struct *)(laddr_inc);
	  fadcRev[boardID] = rdata&FA_VERSION_MASK;
/* 	} */
	  fadcID[nfadc] = boardID;
	  if(boardID >= maxSlot) maxSlot = boardID;
	  if(boardID <= minSlot) minSlot = boardID;
	  
	  printf("Initialized FADC %2d  Slot # %2d at address 0x%08x (0x%08x) \n",
		 nfadc,fadcID[nfadc],(UINT32) FAp[(fadcID[nfadc])],
		 (UINT32) FAp[(fadcID[nfadc])]-fadcA24Offset);
	  nfadc++;
/* 	  printf("Initialized FADC %2d  Slot # %2d at address 0x%08x \n", */
/* 		 ii,fadcID[ii],(UINT32) FAp[(fadcID[ii])]); */
	}
    }

  /* Check if we are to exit when pointers are setup */
  noBoardInit=iFlag&(1<<16);

  /* Check if we are using a JLAB FADC Signal Distribution Card (SDC)
     NOTE the SDC board only supports 7 FADCs - so if there are
     more than 7 FADCs in the crate they can only be controlled by daisychaining 
     multiple SDCs together - or by using a VXS Crate with SD switch card 
  */
  a16addr = iFlag&FA_SDC_ADR_MASK;
  if(a16addr) 
    {
#ifdef VXWORKS
      res = sysBusToLocalAdrs(0x29,(char *)a16addr,(char **)&laddr);
      if (res != 0) 
	{
	  printf("faInit: ERROR in sysBusToLocalAdrs(0x29,0x%x,&laddr) \n",a16addr);
	  return(ERROR);
	}

      res = vxMemProbe((char *) laddr,VX_READ,2,(char *)&sdata);
#else
      res = vmeBusToLocalAdrs(0x29,(char *)a16addr,(char **)&laddr);
      if (res != 0) 
	{
	  printf("faInit: ERROR in vmeBusToLocalAdrs(0x29,0x%x,&laddr) \n",a16addr);
	  return(ERROR);
	}
      res = vmeMemProbe((char *) laddr,2,(char *)&sdata);
#endif
      if(res < 0) 
	{
	  printf("faInit: ERROR: No addressable SDC board at addr=0x%x\n",(UINT32) laddr);
	} 
      else 
	{
	  fadcA16Offset = laddr-a16addr;
	  FASDCp = (struct fadc_sdc_struct *) laddr;
	  if(!noBoardInit)
	    vmeWrite16(&(FASDCp->ctrl),FASDC_CSR_INIT);   /* Reset the Module */

	  if(nfadc>7) 
	    {
	      printf("WARN: A Single JLAB FADC Signal Distribution Module only supports 7 FADCs\n");
	      printf("WARN: You must use multiple SDCs to support more FADCs - this must be configured in hardware\n");
	    }
#ifdef VXWORKS
	  printf("Using JLAB FADC Signal Distribution Module at address 0x%x\n",
		 (UINT32) FASDCp); 
#else
	  printf("Using JLAB FADC Signal Distribution Module at VME (Local) address 0x%x (0x%x)\n",
		 (UINT32)a16addr, (UINT32) FASDCp); 
#endif
	  fadcUseSDC=1;
	}
      if(fadcSource == FA_SOURCE_SDC) 
	{  /* Check if SDC will be used */
	  fadcUseSDC = 1;
	  printf("faInit: JLAB FADC Signal Distribution Card is Assumed in Use\n");
	  printf("faInit: Front Panel Inputs will be enabled. \n");
	}
      else
	{
	  fadcUseSDC = 0;
	  printf("faInit: JLAB FADC Signal Distribution Card will not be Used\n");
	}
    }

  /* Hard Reset of all FADC boards in the Crate */
  for(ii=0;ii<nfadc;ii++) 
    {
      if(!noBoardInit)
	vmeWrite32(&(FAp[fadcID[ii]]->reset),FA_RESET_ALL);
    }
  taskDelay(60); 

  /* Initialize Interrupt variables */
  fadcIntID = -1;
  fadcIntRunning = FALSE;
  fadcIntLevel = FA_VME_INT_LEVEL;
  fadcIntVec = FA_VME_INT_VEC;
  fadcIntRoutine = NULL;
  fadcIntArg = 0;

  /* Calculate the A32 Offset for use in Block Transfers */
#ifdef VXWORKS
  res = sysBusToLocalAdrs(0x09,(char *)fadcA32Base,(char **)&laddr);
  if (res != 0) 
    {
      printf("faInit: ERROR in sysBusToLocalAdrs(0x09,0x%x,&laddr) \n",fadcA32Base);
      return(ERROR);
    } 
  else 
    {
      fadcA32Offset = laddr - fadcA32Base;
    }
#else
  res = vmeBusToLocalAdrs(0x09,(char *)fadcA32Base,(char **)&laddr);
  if (res != 0) 
    {
      printf("faInit: ERROR in vmeBusToLocalAdrs(0x09,0x%x,&laddr) \n",fadcA32Base);
      return(ERROR);
    } 
  else 
    {
      fadcA32Offset = laddr - fadcA32Base;
    }
#endif

  if(!noBoardInit)
    {
      /* what are the Trigger Sync Reset and Clock sources */
      if (fadcSource == FA_SOURCE_VXS)
	{
	  printf("faInit: Enabling FADC for VXS Clock ");
	  clkSrc  = FA_REF_CLK_P0;
	  switch (iFlag&0xf) 
	    {
	    case 0: case 1:
	      printf("and Software Triggers (Soft Sync Reset)\n");
	      trigSrc = FA_TRIG_VME | FA_ENABLE_SOFT_TRIG;
	      srSrc   = FA_SRESET_VME | FA_ENABLE_SOFT_SRESET;
	      break;
	    case 2:
	      printf("and Front Panel Triggers (Soft Sync Reset)\n");
	      trigSrc = FA_TRIG_FP_ISYNC;
	      srSrc   = FA_SRESET_VME | FA_ENABLE_SOFT_SRESET;
	      break;
	    case 3:
	      printf("and Front Panel Triggers (FP Sync Reset)\n");
	      trigSrc = FA_TRIG_FP_ISYNC;
	      srSrc   = FA_SRESET_FP_ISYNC;
	      break;
	    case 4: case 6:
	      printf("and VXS Triggers (Soft Sync Reset)\n");
	      trigSrc = FA_TRIG_P0_ISYNC;
	      srSrc   = FA_SRESET_VME | FA_ENABLE_SOFT_SRESET;
	      break;
	    case 5: case 7:
	      printf("and VXS Triggers (VXS Sync Reset)\n");
	      trigSrc = FA_TRIG_P0_ISYNC;
	      srSrc   = FA_SRESET_P0_ISYNC;
	      break;
	    case 8: case 10: case 12: case 14:
	      printf("and Internal Trigger Logic (Soft Sync Reset)\n");
	      trigSrc = FA_TRIG_INTERNAL;
	      srSrc   = FA_SRESET_VME | FA_ENABLE_SOFT_SRESET;
	      break;
	    case 9: case 11: case 13: case 15:
	      printf("and Internal Trigger Logic (VXS Sync Reset)\n");
	      trigSrc = FA_TRIG_INTERNAL;
	      srSrc   = FA_SRESET_FP_ISYNC;
	      break;
	    }
	}
      else if (fadcSource == FA_SOURCE_SDC) 
	{
	  printf("faInit: Enabling FADC for SDC Clock (Front Panel) ");
	  clkSrc  = FA_REF_CLK_FP;
	  switch (iFlag&0xf) 
	    {
	    case 0: case 1:
	      printf("and Software Triggers (Soft Sync Reset)\n");
	      trigSrc = FA_TRIG_VME | FA_ENABLE_SOFT_TRIG;
	      srSrc   = FA_SRESET_VME | FA_ENABLE_SOFT_SRESET;
	      break;
	    case 2: case 4: case 6:
	      printf("and Front Panel Triggers (Soft Sync Reset)\n");
	      trigSrc = FA_TRIG_FP_ISYNC;
	      srSrc   = FA_SRESET_VME | FA_ENABLE_SOFT_SRESET;
	      break;
	    case 3: case 5: case 7:
	      printf("and Front Panel Triggers (FP Sync Reset)\n");
	      trigSrc = FA_TRIG_FP_ISYNC;
	      srSrc   = FA_SRESET_FP_ISYNC;
	      break;
	    case 8: case 10: case 12: case 14:
	      printf("and Internal Trigger Logic (Soft Sync Reset)\n");
	      trigSrc = FA_TRIG_INTERNAL;
	      srSrc   = FA_SRESET_VME | FA_ENABLE_SOFT_SRESET;
	      break;
	    case 9: case 11: case 13: case 15:
	      printf("and Internal Trigger Logic (Front Panel Sync Reset)\n");
	      trigSrc = FA_TRIG_INTERNAL;
	      srSrc   = FA_SRESET_FP_ISYNC;
	      break;
	    }
	  faSDC_Config(0,0);
	}
      else 
	{  /* Use internal Clk */
	  printf("faInit: Enabling FADC Internal Clock, ");
	  clkSrc = FA_REF_CLK_INTERNAL;
	  switch (iFlag&0xf) 
	    {
	    case 0: case 1:
	      printf("and Software Triggers (Soft Sync Reset)\n");
	      trigSrc = FA_TRIG_VME | FA_ENABLE_SOFT_TRIG;
	      srSrc   = FA_SRESET_VME | FA_ENABLE_SOFT_SRESET ;
	      break;
	    case 2:
	      printf("and Front Panel Triggers (Soft Sync Reset)\n");
	      trigSrc = FA_TRIG_FP_ISYNC;
	      srSrc   = FA_SRESET_VME | FA_ENABLE_SOFT_SRESET;
	      break;
	    case 3:
	      printf("and Front Panel Triggers (FP Sync Reset)\n");
	      trigSrc = FA_TRIG_FP_ISYNC;
	      srSrc   = FA_SRESET_FP_ISYNC;
	      break;
	    case 4: case 6:
	      printf("and VXS Triggers (Soft Sync Reset)\n");
	      trigSrc = FA_TRIG_P0_ISYNC;
	      srSrc   = FA_SRESET_VME | FA_ENABLE_SOFT_SRESET;
	      break;
	    case 5: case 7:
	      printf("and VXS Triggers (VXS Sync Reset)\n");
	      trigSrc = FA_TRIG_P0_ISYNC;
	      srSrc   = FA_SRESET_P0_ISYNC;
	      break;
	    case 8: case 10: case 12: case 14:
	      printf("and Internal Trigger Logic (Soft Sync Reset)\n");
	      trigSrc = FA_TRIG_INTERNAL;
	      srSrc   = FA_SRESET_VME | FA_ENABLE_SOFT_SRESET;
	      break;
	    case 9: case 11: case 13: case 15:
	      printf("and Internal Trigger Logic (Front Panel Sync Reset)\n");
	      trigSrc = FA_TRIG_INTERNAL;
	      srSrc   = FA_SRESET_FP_ISYNC;
	      break;
	    }
	}
    }

  /* Enable Clock source - Internal Clk enabled by default */ 
  for(ii=0;ii<nfadc;ii++) 
    {
      vmeWrite32(&(FAp[fadcID[ii]]->ctrl1),(clkSrc | FA_ENABLE_INTERNAL_CLK)) ;
    }
  taskDelay(20);

  /* Hard Reset FPGAs and FIFOs */
  for(ii=0;ii<nfadc;ii++) 
    {
      vmeWrite32(&(FAp[fadcID[ii]]->reset),
		(FA_RESET_ADC_FPGA1 | FA_RESET_ADC_FIFO1 |
		 FA_RESET_DAC | FA_RESET_EXT_RAM_PT));
/* #ifdef USEMGTCTRL */
      /* Release reset on MGTs */
      vmeWrite32(&(FAp[fadcID[ii]]->mgt_ctrl),FA_MGT_RESET);
      vmeWrite32(&(FAp[fadcID[ii]]->mgt_ctrl),FA_RELEASE_MGT_RESET);
      vmeWrite32(&(FAp[fadcID[ii]]->mgt_ctrl),FA_MGT_RESET);
/* #endif */
    }
  taskDelay(5);

  /* Write configuration registers with default/defined Sources */
  for(ii=0;ii<nfadc;ii++) 
    {
    
      /* Program an A32 access address for this FADC's FIFO */
      a32addr = fadcA32Base + ii*FA_MAX_A32_MEM;
#ifdef VXWORKS
      res = sysBusToLocalAdrs(0x09,(char *)a32addr,(char **)&laddr);
      if (res != 0) 
	{
	  printf("faInit: ERROR in sysBusToLocalAdrs(0x09,0x%x,&laddr) \n",a32addr);
	  return(ERROR);
	}
#else
      res = vmeBusToLocalAdrs(0x09,(char *)a32addr,(char **)&laddr);
      if (res != 0) 
	{
	  printf("faInit: ERROR in vmeBusToLocalAdrs(0x09,0x%x,&laddr) \n",a32addr);
	  return(ERROR);
	}
#endif
      FApd[fadcID[ii]] = (unsigned int *)(laddr);  /* Set a pointer to the FIFO */
      if(!noBoardInit)
	{
	  vmeWrite32(&(FAp[fadcID[ii]]->adr32),(a32addr>>16) + 1);  /* Write the register and enable */
	
	  /* Set Default Block Level to 1 */
	  vmeWrite32(&(FAp[fadcID[ii]]->blk_level),1);
	}
      fadcBlockLevel=1;

      /* Setup Trigger and Sync Reset sources */
      if(!noBoardInit)
	{
	  vmeWrite32(&(FAp[fadcID[ii]]->ctrl1),
		    vmeRead32(&(FAp[fadcID[ii]]->ctrl1)) | 
		    (srSrc | trigSrc) );
	}
    }

  /* If there are more than 1 FADC in the crate then setup the Muliblock Address
     window. This must be the same on each board in the crate */
  if(nfadc > 1) 
    {
      a32addr = fadcA32Base + (nfadc+1)*FA_MAX_A32_MEM; /* set MB base above individual board base */
#ifdef VXWORKS
      res = sysBusToLocalAdrs(0x09,(char *)a32addr,(char **)&laddr);
      if (res != 0) 
	{
	  printf("faInit: ERROR in sysBusToLocalAdrs(0x09,0x%x,&laddr) \n",a32addr);
	  return(ERROR);
	}
#else
      res = vmeBusToLocalAdrs(0x09,(char *)a32addr,(char **)&laddr);
      if (res != 0) 
	{
	  printf("faInit: ERROR in vmeBusToLocalAdrs(0x09,0x%x,&laddr) \n",a32addr);
	  return(ERROR);
	}
#endif
      FApmb = (unsigned int *)(laddr);  /* Set a pointer to the FIFO */
      if(!noBoardInit)
	{
	  for (ii=0;ii<nfadc;ii++) 
	    {
	      /* Write the register and enable */
	      vmeWrite32(&(FAp[fadcID[ii]]->adr_mb),
			(a32addr+FA_MAX_A32MB_SIZE) + (a32addr>>16) + FA_A32_ENABLE);
	    }
	}    
      /* Set First Board and Last Board */
      fadcMaxSlot = maxSlot;
      fadcMinSlot = minSlot;
      if(!noBoardInit)
	{
	  vmeWrite32(&(FAp[minSlot]->ctrl1),
		    vmeRead32(&(FAp[minSlot]->ctrl1)) | FA_FIRST_BOARD);
	  vmeWrite32(&(FAp[maxSlot]->ctrl1),
		    vmeRead32(&(FAp[maxSlot]->ctrl1)) | FA_LAST_BOARD);
	}    
    }

  fadcInited = nfadc;
  if(errFlag > 0) 
    {
      printf("faInit: ERROR: Unable to initialize all FADC Modules\n");
      if(nfadc > 0)
	printf("faInit: %d FADC(s) successfully initialized\n",nfadc );
      return(ERROR);
    } 
  else 
    {
      return(OK);
    }
}

/*******************************************************************************
 *
 * faSetClockSource - Set the clock source
 *
 *   This routine should be used in the case that the source clock
 *   is NOT set in faInit (and defaults to Internal).  Such is the case
 *   when clocks are synchronized in a many crate system.  The clock source
 *   of the FADC should ONLY be set AFTER those clocks have been set and
 *   synchronized.
 *
 *   clkSrc: 2 bit integer
 *       bits 1-0:  defines Clock Source
 *           0 0  Internal 250MHz Clock
 *           0 1  Front Panel 
 *           1 0  VXS (P0)
 *           1 1  VXS (P0)
 *
 */

int
faSetClockSource(int id, int clkSrc)
{
  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      printf("faStatus: ERROR : ADC in slot %d is not initialized \n",id);
      return;
    }

  if(clkSrc>0x3)
    {
      printf("%s: ERROR: Invalid Clock Source specified (0x%x)\n",
	     __FUNCTION__,clkSrc);
      return ERROR;
    }

  /* Enable Clock source - Internal Clk enabled by default */ 
  vmeWrite32(&(FAp[id]->ctrl1),
	     (vmeRead32(&FAp[id]->ctrl1) & ~(FA_REF_CLK_MASK)) |
	     (clkSrc | FA_ENABLE_INTERNAL_CLK)) ;
  taskDelay(20);

  switch(clkSrc)
    {
    case FA_REF_CLK_INTERNAL:
      printf("%s: FADC id %d clock source set to INTERNAL\n",
	     __FUNCTION__,id);
      break;

    case FA_REF_CLK_FP:
      printf("%s: FADC id %d clock source set to FRONT PANEL\n",
	     __FUNCTION__,id);
      break;

    case FA_REF_CLK_P0:
      printf("%s: FADC id %d clock source set to VXS (P0)\n",
	     __FUNCTION__,id);
      break;

    case FA_REF_CLK_MASK:
      printf("%s: FADC id %d clock source set to VXS (P0)\n",
	     __FUNCTION__,id);
      break;
    }

  return OK;
}

void
faStatus(int id, int sflag)
{ 
  int ii;
  unsigned int a32Base, ambMin, ambMax, vers, bid, brev;
  unsigned int csr, ctrl1, ctrl2, count, bcount, blevel, intr, addr32, addrMB;
  unsigned int adcStat[3], adcConf[3],
    PTW, PL, NSB, NSA, NP, adcChanDisabled, playbackMode;
  unsigned int adc_enabled, adc_version, adc_option;
  unsigned int trigCnt, itrigCnt, ramWords;
  unsigned int mgtStatus;
  unsigned int berr_count=0;

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      printf("faStatus: ERROR : ADC in slot %d is not initialized \n",id);
      return;
    }

  FALOCK;
  vers   =  vmeRead32(&FAp[id]->version);
  bid    = ((vers)&FA_BOARD_MASK)>>16;
  brev   = (vers)&FA_VERSION_MASK;

  csr    = (vmeRead32(&(FAp[id]->csr)))&FA_CSR_MASK;
  ctrl1  = (vmeRead32(&(FAp[id]->ctrl1)))&FA_CONTROL_MASK;
  ctrl2  = (vmeRead32(&(FAp[id]->ctrl2)))&FA_CONTROL2_MASK;
  count  = (vmeRead32(&(FAp[id]->ev_count)))&FA_EVENT_COUNT_MASK;
  bcount = (vmeRead32(&(FAp[id]->blk_count)))&FA_BLOCK_COUNT_MASK;
  blevel  = (vmeRead32(&(FAp[id]->blk_level)))&FA_BLOCK_LEVEL_MASK;
  ramWords = (vmeRead32(&(FAp[id]->ram_word_count)))&FA_RAM_DATA_MASK;
  trigCnt = vmeRead32(&(FAp[id]->trig_scal));
  itrigCnt = vmeRead32(&(FAp[id]->internal_trig_scal));
  intr   = vmeRead32(&(FAp[id]->intr));
  addr32 = vmeRead32(&(FAp[id]->adr32));
  a32Base = (addr32&FA_A32_ADDR_MASK)<<16;
  addrMB = vmeRead32(&(FAp[id]->adr_mb));
  ambMin =  (addrMB&FA_AMB_MIN_MASK)<<16;
  ambMax =  (addrMB&FA_AMB_MAX_MASK);
  berr_count = vmeRead32(&(FAp[id]->berr_module_scal));

  for(ii=0;ii<3;ii++) 
    {
      adcStat[ii] = (vmeRead32(&(FAp[id]->adc_status[ii]))&0xFFFF);
      adcConf[ii] = (vmeRead32(&(FAp[id]->adc_config[ii]))&0xFFFF);
    }    
  PTW =  (vmeRead32(&(FAp[id]->adc_ptw))&0xFFFF)*FA_ADC_NS_PER_CLK;
  PL  =  (vmeRead32(&(FAp[id]->adc_pl))&0xFFFF)*FA_ADC_NS_PER_CLK;
  NSB =  (vmeRead32(&(FAp[id]->adc_nsb))&0xFFFF)*FA_ADC_NS_PER_CLK;
  NSA =  (vmeRead32(&(FAp[id]->adc_nsa))&0xFFFF)*FA_ADC_NS_PER_CLK;
  adc_version = adcStat[0]&FA_ADC_VERSION_MASK;
  adc_option  = (adcConf[0]&FA_ADC_PROC_MASK) + 1;
  NP          = (adcConf[0]&FA_ADC_PEAK_MASK)>>4;
  adc_enabled = (adcConf[0]&FA_ADC_PROC_ENABLE);
  playbackMode = (adcConf[0]&FA_ADC_PLAYBACK_MODE)>>7;
  adcChanDisabled = (adcConf[1]&FA_ADC_CHAN_MASK);

  mgtStatus = vmeRead32(&(FAp[id]->mgt_status));

  FAUNLOCK;

#ifdef VXWORKS
  printf("\nSTATUS for FADC in slot %d at base address 0x%x \n",
	 id, (UINT32) FAp[id]);
#else
  printf("\nSTATUS for FADC in slot %d at VME (Local) base address 0x%x (0x%x)\n",
	 id, (UINT32) FAp[id] - fadcA24Offset, (UINT32) FAp[id]);
#endif
  printf("---------------------------------------------------------------------- \n");

  printf(" Board Firmware Rev/ID = 0x%04x : ADC Processing Rev = 0x%04x\n",
	 (vers)&0xffff, adc_version);
  if(addrMB&FA_AMB_ENABLE) 
    {
      printf(" Alternate VME Addressing: Multiblock Enabled\n");
      if(addr32&FA_A32_ENABLE)
	printf("   A32 Enabled at VME (Local) base 0x%08x (0x%08x)\n",a32Base,(UINT32) FApd[id]);
      else
	printf("   A32 Disabled\n");
    
      printf("   Multiblock VME Address Range 0x%08x - 0x%08x\n",ambMin,ambMax);
    }
  else
    {
      printf(" Alternate VME Addressing: Multiblock Disabled\n");
      if(addr32&FA_A32_ENABLE)
	printf("   A32 Enabled at VME (Local) base 0x%08x (0x%08x)\n",a32Base,(UINT32) FApd[id]);
      else
	printf("   A32 Disabled\n");
    }

  if(ctrl1&FA_INT_ENABLE_MASK) 
    {
      printf("\n  Interrupts ENABLED: ");
      if(ctrl1&FA_ENABLE_BLKLVL_INT) printf(" on Block Level(%d)",blevel);
    
      printf("\n");
      printf("  Interrupt Reg: 0x%08x\n",intr);
      printf("  VME INT Vector = 0x%x  Level = %d\n",(intr&FA_INT_VEC_MASK),((intr&FA_INT_LEVEL_MASK)>>8));
    }

  printf("\n Signal Sources: \n");

  if((ctrl1&FA_REF_CLK_MASK)==FA_REF_CLK_INTERNAL) 
    {
      printf("   Ref Clock : Internal\n");
    }
  else if((ctrl1&FA_REF_CLK_MASK)==FA_REF_CLK_P0) 
    {
      printf("   Ref Clock : VXS\n");
    }
  else if((ctrl1&FA_REF_CLK_MASK)==FA_REF_CLK_FP) 
    {
      printf("   Ref Clock : Front Panel\n");
    }
  else
    {
      printf("   Ref Clock : %d (Undefined)\n",(ctrl1&FA_REF_CLK_MASK));
    }

  switch(ctrl1&FA_TRIG_MASK) 
    {
    case FA_TRIG_INTERNAL:
      printf("   Trig Src  : Internal\n");
      break;
    case FA_TRIG_VME:
      printf("   Trig Src  : VME (Software)\n");
      break;
    case FA_TRIG_P0_ISYNC:
      printf("   Trig Src  : VXS (Async)\n");
      break;
    case FA_TRIG_P0:
      printf("   Trig Src  : VXS (Sync)\n");
      break;
    case FA_TRIG_FP_ISYNC:
      printf("   Trig Src  : Front Panel (Async)\n");
      break;
    case FA_TRIG_FP:
      printf("   Trig Src  : Front Panel (Sync)\n");
    }  

  switch(ctrl1&FA_SRESET_MASK) 
    {
    case FA_SRESET_VME:
      printf("   Sync Reset: VME (Software)\n");
      break;
    case FA_SRESET_P0_ISYNC:
      printf("   Sync Reset: VXS (Async)\n");
      break;
    case FA_SRESET_P0:
      printf("   Sync Reset: VXS (Sync)\n");
      break;
    case FA_SRESET_FP_ISYNC:
      printf("   Sync Reset: Front Panel (Async)\n");
      break;
    case FA_SRESET_FP:
      printf("   Sync Reset: Front Panel (Sync)\n");
    }  

  if(fadcUseSDC) 
    {
      printf("   SDC       : In Use\n");
    }


  printf("\n Configuration: \n");

  if(ctrl1&FA_ENABLE_INTERNAL_CLK)
    printf("   Internal Clock ON\n");
  else
    printf("   Internal Clock OFF\n");

  if(ctrl1&FA_ENABLE_BERR)
    printf("   Bus Error ENABLED\n");
  else
    printf("   Bus Error DISABLED\n");


  if(ctrl1&FA_ENABLE_MULTIBLOCK) 
    {
      int tP0, tP2;
      tP0 = ctrl1&FA_MB_TOKEN_VIA_P0;
      tP2 = ctrl1&FA_MB_TOKEN_VIA_P2;

      if(tP0) 
	{
	  if(ctrl1&FA_FIRST_BOARD)
	    printf("   MultiBlock transfer ENABLED (First Board - token via VXS)\n");
	  else if(ctrl1&FA_LAST_BOARD)
	    printf("   MultiBlock transfer ENABLED (Last Board  - token via VXS)\n");
	  else
	    printf("   MultiBlock transfer ENABLED (Token via VXS)\n");
	  /* #ifdef VERSION1 */
	}
      else if(tP2)
	{
	  if(ctrl1&FA_FIRST_BOARD)
	    printf("   MultiBlock transfer ENABLED (First Board - token via P2)\n");
	  else if(ctrl1&FA_LAST_BOARD)
	    printf("   MultiBlock transfer ENABLED (Last Board  - token via P2)\n");
	  else
	    printf("   MultiBlock transfer ENABLED (Token via P2)\n");
	  /* #endif */
	}
      else
	{
	  printf("   MultiBlock transfer ENABLED (**NO Tokens enabled**)\n");
	}
    } 
  else 
    {
      printf("   MultiBlock transfer DISABLED\n");
    }

  if(ctrl1&FA_ENABLE_SOFT_TRIG)
    printf("   Software Triggers   ENABLED\n");
  if(ctrl1&FA_ENABLE_SOFT_SRESET)
    printf("   Software Sync Reset ENABLED\n");


  printf("\n ADC Processing Configuration: \n");
  printf("   Channel Disable Mask = 0x%04x\n",adcChanDisabled);
  if(adc_enabled)
    printf("   Mode = %d  (ENABLED)\n",adc_option);
  else
    printf("   Mode = %d  (Disabled)\n",adc_option);
  printf("   Lookback (PL)    = %d ns   Time Window (PTW) = %d ns\n",PL,PTW);
  printf("   Time Before Peak = %d ns   Time After Peak   = %d ns\n",NSB,NSA);
  printf("   Max Peak Count   = %d \n",NP);
  printf("   Playback Mode    = %d \n",playbackMode);

  printf("\n");
  if(csr&FA_CSR_ERROR_MASK) 
    {
      printf("  CSR       Register = 0x%08x - **Error Condition**\n",csr);
    }
  else 
    {
      printf("  CSR       Register = 0x%08x\n",csr);
    }

  printf("  Control 1 Register = 0x%08x \n",ctrl1);


  if((ctrl2&FA_CTRL_ENABLE_MASK)==FA_CTRL_ENABLED) 
    {
      printf("  Control 2 Register = 0x%08x - Enabled for triggers\n",ctrl2);
    }
  else
    {
      printf("  Control 2 Register = 0x%08x - Disabled\n",ctrl2);
    }

  printf("  Internal Triggers (Live) = %d\n",itrigCnt);
  printf("  Trigger   Scaler         = %d\n",trigCnt);


  if(csr&FA_CSR_BLOCK_READY) 
    {
      printf("  Blocks in FIFO           = %d  (Block level = %d) - Block Available\n",bcount,blevel);
      printf("  RAM Level (Bytes)        = %d \n",(ramWords*8)); 
    }
  else if (csr&FA_CSR_EVENT_AVAILABLE) 
    {
      printf("  Events in FIFO           = %d  (Block level = %d) - Data Available\n",count,blevel);
      printf("  RAM Level (Bytes)        = %d \n",(ramWords*8)); 
    }
  else
    {
      printf("  Events in FIFO           = %d  (Block level = %d)\n",count,blevel);
    }

  printf("  MGT Status Register      = 0x%08x ",mgtStatus);
  if(mgtStatus & (FA_MGT_GTX1_HARD_ERROR | FA_MGT_GTX1_SOFT_ERROR |
		  FA_MGT_GTX2_HARD_ERROR | FA_MGT_GTX2_SOFT_ERROR))
    printf(" - **Error Condition**\n");
  else
    printf("\n");
	 
  printf("  BERR count (from module) = %d\n",berr_count);

}

void 
faGStatus(int sflag)
{
  int ii;

  for (ii=0;ii<nfadc;ii++) 
    {
      faStatus(fadcID[ii],sflag);
    }

}



/***********************
 *
 *  faSetProcMode - Setup ADC processing modes.
 *
 *   VERSION2: bank is ignored
 */
int
faSetProcMode(int id, int pmode, unsigned int PL, unsigned int PTW, 
	      unsigned int NSB, unsigned int NSA, unsigned int NP, int bank)
{
  
  int err=0;
  unsigned int ptw_last_adr, ptw_max_buf;


  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faSetProcMode: ERROR : FADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return(ERROR);
    }

  if((pmode<=0)||(pmode>8)) 
    {
      printf("faSetProcMode: ERROR: Processing mode (%d) out of range (pmode= 1-8)\n",pmode);
      return(ERROR);
    }
  else
    {
      if((pmode>3)&&(pmode<8)) 
	{
	  printf("faSetProcMode: ERROR: Processing mode (%d) not implemented \n",pmode);
	}
    }
  
  if(NP>4) 
    {
      printf("faSetProcMode: ERROR: Invalid Peak count %d (must be 0-4)\n",NP);
      return(ERROR);
    }

  /*Defaults */
  if((PL==0)||(PL>FA_ADC_MAX_PL))  PL  = FA_ADC_DEFAULT_PL;
  if((PTW==0)||(PTW>FA_ADC_MAX_PTW)) PTW = FA_ADC_DEFAULT_PTW;
  if((NSB==0)||(NSB>FA_ADC_MAX_NSB)) NSB = FA_ADC_DEFAULT_NSB;
  if((NSA==0)||(NSA>FA_ADC_MAX_NSA)) NSA = FA_ADC_DEFAULT_NSA;
  if((NP==0)&&(pmode!=FA_ADC_PROC_MODE_WINDOW))  NP = FA_ADC_DEFAULT_NP;

  /* Consistancy check */
  if(PTW > PL) 
    {
      err++;
      printf("faSetProcMode: ERROR: Window must be <= Latency\n"); 
    }
  if(((NSB+NSA)%2)==0) 
    {
      err++;
      printf("faSetProcMode: ERROR: NSB+NSA must be an odd number\n"); 
    }

  /* Calculate Proc parameters */
  ptw_max_buf  = (unsigned int) (2016/(PTW + 8));
  ptw_last_adr = ptw_max_buf * (PTW + 8) - 1;

  /* Current firmware (version<=0x0208) requires a call to faSetNormalMode
     before enabling the window registers */
  faSetNormalMode(id,0);

  FALOCK;
  /* Disable ADC processing while writing window info */
  vmeWrite32(&(FAp[id]->adc_config[0]),
	    ((pmode-1) | (NP<<4) ) );
  vmeWrite32(&(FAp[id]->adc_config[1]), fadcChanDisable[id]);
  vmeWrite32(&(FAp[id]->adc_pl),PL);
  vmeWrite32(&(FAp[id]->adc_ptw),PTW);
  vmeWrite32(&(FAp[id]->adc_nsb),NSB);
  vmeWrite32(&(FAp[id]->adc_nsa),NSA);
  vmeWrite32(&(FAp[id]->ptw_max_buf),
	    ptw_max_buf);
  vmeWrite32(&(FAp[id]->ptw_last_adr),
	    ptw_last_adr);
  /* Enable ADC processing */
  vmeWrite32(&(FAp[id]->adc_config[0]),
	    ((pmode-1) | (NP<<4) | FA_ADC_PROC_ENABLE) );

  FAUNLOCK;

  return(OK);
}

void
faGSetProcMode(int pmode, unsigned int PL, unsigned int PTW, 
	       unsigned int NSB, unsigned int NSA, unsigned int NP, int bank)
{
  int ii, res;

  for (ii=0;ii<nfadc;ii++) {
    res = faSetProcMode(fadcID[ii],pmode,PL,PTW,NSB,NSA,NP,bank);
    if(res<0) printf("ERROR: slot %d, in faSetProcMode()\n",fadcID[ii]);
  }
}

/*
 * faWaitForAdcReady()
 *   - Static routine, to wait for the ADC processing chip ready bit
 *     before proceeding with further programming
 *
 */
static void
faWaitForAdcReady(int id)
{
  int iwait=0;

  while((iwait<100) && (vmeRead32(&FAp[id]->adc_status[0])&0x8000)==0)
    {
      iwait++;
    }

  if(iwait==100)
    printf("%s: ERROR: Wait timeout.\n",__FUNCTION__);

}

/* faSetNormalMode 
 *    - Configure the ADC Processing in "Normal Mode"
 *      This is temporary until the firmware is confirmed to be stable
 *
 */
void
faSetNormalMode(int id, int opt)
{
  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) {
    logMsg("faSetProcMode: ERROR : FADC in slot %d is not initialized \n",id,0,0,0,0,0);
    return;
  }

  FALOCK;
  faWaitForAdcReady(id);
  vmeWrite32(&FAp[id]->adc_config[3], 0x0F02);
  faWaitForAdcReady(id);
  vmeWrite32(&FAp[id]->adc_config[2], 0x40);
  faWaitForAdcReady(id);
  vmeWrite32(&FAp[id]->adc_config[2], 0xC0);

  faWaitForAdcReady(id);
  vmeWrite32(&FAp[id]->adc_config[3], 0x179F);
  faWaitForAdcReady(id);
  vmeWrite32(&FAp[id]->adc_config[2], 0x40);
  faWaitForAdcReady(id);
  vmeWrite32(&FAp[id]->adc_config[2], 0xC0);
	    
  /* 01dec2011 This portion commented out... would change the input gain */
/*   faWaitForAdcReady(id); */
/*   vmeWrite32(&FAp[id]->adc_config[3], 0x1811); */
/*   faWaitForAdcReady(id); */
/*   vmeWrite32(&FAp[id]->adc_config[2], 0x40); */
/*   faWaitForAdcReady(id); */
/*   vmeWrite32(&FAp[id]->adc_config[2], 0xC0);	 */
	    
  faWaitForAdcReady(id);
  vmeWrite32(&FAp[id]->adc_config[3], 0xFF01);		/* transfer register values */
  faWaitForAdcReady(id);
  vmeWrite32(&FAp[id]->adc_config[2], 0x40);
  faWaitForAdcReady(id);
  vmeWrite32(&FAp[id]->adc_config[2], 0xC0);
	    
  printf("%s: ---- FADC %2d ADC chips initialized ----\n",
	 __FUNCTION__,id);
	        
  faWaitForAdcReady(id);
  vmeWrite32(&FAp[id]->adc_config[3], 0x0D00);
  faWaitForAdcReady(id);
  vmeWrite32(&FAp[id]->adc_config[2], 0x40);
  faWaitForAdcReady(id);
  vmeWrite32(&FAp[id]->adc_config[2], 0xC0);
	    
  faWaitForAdcReady(id);
  vmeWrite32(&FAp[id]->adc_config[3], 0xFF01);		/* transfer register values */
  faWaitForAdcReady(id);
  vmeWrite32(&FAp[id]->adc_config[2], 0x40);
  faWaitForAdcReady(id);
  vmeWrite32(&FAp[id]->adc_config[2], 0xC0);

  FAUNLOCK;


}

/***********************
 *
 *  faSetPPG - Setup FADC Progammable Pulse Generator
 *
 *
 */
int
faSetPPG(int id, int pmode, unsigned short *sdata, int nsamples)
{
  
  int ii, diff;
  unsigned short rval;


  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faSetPPG: ERROR : FADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return(ERROR);
    }

  if(sdata == NULL) 
    {
      printf("faSetPPG: ERROR: Invalid Pointer to sample data\n");
      return(ERROR);
    }

  /*Defaults */
  if((nsamples <= 0)||(nsamples>FA_PPG_MAX_SAMPLES)) nsamples = FA_PPG_MAX_SAMPLES;
  diff = FA_PPG_MAX_SAMPLES - nsamples;

  FALOCK;
  for(ii=0;ii<(nsamples-2);ii++) 
    {
      vmeWrite32(&FAp[id]->adc_test_data, (sdata[ii]|FA_PPG_WRITE_VALUE));
      rval = vmeRead32(&FAp[id]->adc_test_data);
      if( (rval&FA_PPG_SAMPLE_MASK) != sdata[ii])
	printf("faSetPPG: ERROR: Write error %x != %x (ii=%d)\n",rval, sdata[ii],ii);

    }

  vmeWrite32(&FAp[id]->adc_test_data, (sdata[(nsamples-2)]&FA_PPG_SAMPLE_MASK));
  rval = vmeRead32(&FAp[id]->adc_test_data);
  if(rval != sdata[(nsamples-2)])
	printf("faSetPPG: ERROR: Write error %x != %x\n",
	       rval, sdata[nsamples-2]);
  vmeWrite32(&FAp[id]->adc_test_data, (sdata[(nsamples-1)]&FA_PPG_SAMPLE_MASK));
  rval = vmeRead32(&FAp[id]->adc_test_data);
  if(rval != sdata[(nsamples-1)])
	printf("faSetPPG: ERROR: Write error %x != %x\n",
	       rval, sdata[nsamples-1]);
    
/*   vmeWrite32(&FAp[id]->adc_test_data, (sdata[(nsamples-2)]&FA_PPG_SAMPLE_MASK)); */
/*   vmeWrite32(&FAp[id]->adc_test_data, (sdata[(nsamples-1)]&FA_PPG_SAMPLE_MASK)); */
    
  FAUNLOCK;
  
  return(OK);
}

void
faPPGEnable(int id)
{
  unsigned short val1;

  if(id==0) id=fadcID[0];
  
  FALOCK;
  val1 = (vmeRead32(&FAp[id]->adc_config[0])&0xFFFF);
  val1 |= (FA_PPG_ENABLE | 0xff00); 
  vmeWrite32(&FAp[id]->adc_config[0], val1);
  FAUNLOCK;
  
}

void
faPPGDisable(int id)
{
  unsigned short val1;

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faPPGDisable: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }

  FALOCK;
  val1 = (vmeRead32(&FAp[id]->adc_config[0])&0xFFFF);
  val1 &= ~FA_PPG_ENABLE;
  val1 &= ~(0xff00);
  vmeWrite32(&FAp[id]->adc_config[0], val1);
  FAUNLOCK;

}


#ifdef VERSION1
/*************************************************************************************
 *
 *  faItrigBurstConfig - Setup Internal Trigger Burst control Parameters
 * 
 *   ntrig        = max triggers (1-128) allowed in Burst Window 
 *   burst_window = size (in clock ticks 4ns/tick) of Burst window (1 - 4 microsec)
 *   busy_period  = size (in clocks) of busy period to wait after max triggers reached
 *                   (0 - 262 microsec)
 */
int
faItrigBurstConfig(int id, unsigned int ntrig, 
		   unsigned int burst_window, unsigned int busy_period)
{


  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) {
    logMsg("faItrigBurstConfig: ERROR : FADC in slot %d is not initialized \n",id,0,0,0,0,0);
    return(ERROR);
  }

  /* Set Defaults */
  if((ntrig==0)||(ntrig>128))                    ntrig        = 4;
  if((burst_window<0x100)||(burst_window>0x3ff)) burst_window = 0x200;
  if((busy_period==0)||(busy_period>0xffff))     busy_period  = 0x800;

  FALOCK;
  vmeWrite32(&(FAp[id]->itrig_burst_count),ntrig);
  vmeWrite32(&(FAp[id]->itrig_burst_ctrl),((busy_period)<<16) | burst_window);
  FAUNLOCK;

  return(OK);
}
#endif

/*
 * Set Internal trigger pulse width and deadtime between triggers 
 *   Range for each :   4ns <-> 1020ns
 *
 *    Units are in clock ticks (4ns/tick)
 */
unsigned int
faItrigControl(int id, unsigned short itrig_width, unsigned short itrig_dt)
{
  unsigned int retval=0;

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) {
    logMsg("faItrigControl: ERROR : FADC in slot %d is not initialized \n",id,0,0,0,0,0);
    return(0xffffffff);
  }

  /* If both parameters = 0 then just return the current value */
  FALOCK;
  if((itrig_width==0)&&(itrig_dt==0)) {
    retval = vmeRead32(&(FAp[id]->itrig_cfg));
  }else{
    if((itrig_width==0)||(itrig_width>255))    itrig_width = 0xc; /* default 48ns */
    if((itrig_dt==0)||(itrig_dt>255))          itrig_dt    = 0xa; /* default 40ns */

    vmeWrite32(&(FAp[id]->itrig_cfg),(itrig_width<<16)|itrig_dt);
    retval = vmeRead32(&(FAp[id]->itrig_cfg));
  }
  FAUNLOCK;

  return(retval);
}



/**************************************************************************************
 *
 *  faReadBlock - General Data readout routine
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
faReadBlock(int id, volatile UINT32 *data, int nwrds, int rflag)
{
  int ii, blknum, evnum1;
  int stat, retVal, xferCount, rmode, async;
  int dCnt, berr=0;
  int dummy=0;
  volatile unsigned int *laddr;
  unsigned int bhead, ehead, val;
  unsigned int vmeAdr, csr;

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faReadBlock: ERROR : FADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return(ERROR);
    }

  if(data==NULL) 
    {
      logMsg("faReadBlock: ERROR: Invalid Destination address\n",0,0,0,0,0,0);
      return(ERROR);
    }

  fadcBlockError=0;
  if(nwrds <= 0) nwrds= (FA_MAX_ADC_CHANNELS*FA_MAX_DATA_PER_CHANNEL) + 8;
  rmode = rflag&0x0f;
  async = rflag&0x80;
  
  if(rmode >= 1) 
    { /* Block Transfers */
    
      /*Assume that the DMA programming is already setup. */
      /* Don't Bother checking if there is valid data - that should be done prior
	 to calling the read routine */

      /* Check for 8 byte boundary for address - insert dummy word (Slot 0 FADC Dummy DATA)*/
      if((unsigned long) (data)&0x7) 
	{
#ifdef VXWORKS
	  *data = FA_DUMMY_DATA;
#else
	  *data = LSWAP(FA_DUMMY_DATA);
#endif
	  dummy = 1;
	  laddr = (data + 1);
	} 
      else 
	{
	  dummy = 0;
	  laddr = data;
	}

      FALOCK;
      if(rmode == 2) 
	{ /* Multiblock Mode */
	  if((vmeRead32(&(FAp[id]->ctrl1))&FA_FIRST_BOARD)==0) 
	    {
	      logMsg("faReadBlock: ERROR: FADC in slot %d is not First Board\n",id,0,0,0,0,0);
	      FAUNLOCK;
	      return(ERROR);
	    }
	  vmeAdr = (unsigned int)(FApmb) - fadcA32Offset;
	}
      else
	{
	  vmeAdr = (unsigned int)(FApd[id]) - fadcA32Offset;
	}



	/*sergey
#ifdef VXWORKS
      retVal = sysVmeDmaSend((UINT32)laddr, vmeAdr, (nwrds<<2), 0);
#else
      retVal = vmeDmaSend((UINT32)laddr, vmeAdr, (nwrds<<2));
#endif
	*/
    retVal = usrVme2MemDmaStart(vmeAdr, (UINT32)laddr, (nwrds<<2));



      if(retVal |= 0) 
	{
	  logMsg("faReadBlock: ERROR in DMA transfer Initialization 0x%x\n",retVal,0,0,0,0,0);
	  FAUNLOCK;
	  return(retVal);
	}

      if(async) 
	{ /* Asynchonous mode - return immediately - don't wait for done!! */
	  FAUNLOCK;
	  return(OK);
	}
      else
	{
	  /* Wait until Done or Error */



	  /*sergey
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
	  /* Check to see that Bus error was generated by FADC */
	  if(rmode == 2) 
	    {
	      csr = vmeRead32(&(FAp[fadcMaxSlot]->csr));  /* from Last FADC */
	      stat = (csr)&FA_CSR_BERR_STATUS;  /* from Last FADC */
	    }
	  else
	    {
	      csr = vmeRead32(&(FAp[id]->csr));  /* from Last FADC */
	      stat = (csr)&FA_CSR_BERR_STATUS;  /* from Last FADC */
	    }
	  if((retVal>0) && (stat)) 
	    {

/*sergey
#ifdef VXWORKS
	      xferCount = (nwrds - (retVal>>2) + dummy);
#else
*/

	      xferCount = ((retVal>>2) + dummy);  /* Number of Longwords transfered */
	      /* 	xferCount = (retVal + dummy);  /\* Number of Longwords transfered *\/ */

/*sergey
#endif
*/

	      FAUNLOCK;
	      return(xferCount); /* Return number of data words transfered */
	    }
	  else
	    {
/*sergey
#ifdef VXWORKS
	      xferCount = (nwrds - (retVal>>2) + dummy);
#else
*/
	      xferCount = ((retVal>>2) + dummy);  /* Number of Longwords transfered */

/*sergey
#endif
*/

	      logMsg("faReadBlock: DMA transfer terminated by unknown BUS Error (csr=0x%x xferCount=%d id=%d)\n",
		     csr,xferCount,id,0,0,0);
	      FAUNLOCK;
	      fadcBlockError=1;
	      return(xferCount);
	      /* 	return(ERROR); */
	    }
	} 
      else if (retVal == 0)
	{ /* Block Error finished without Bus Error */
/*sergey
#ifdef VXWORKS
	  logMsg("faReadBlock: WARN: DMA transfer terminated by word count 0x%x\n",nwrds,0,0,0,0,0);
#else
*/
	  logMsg("faReadBlock: WARN: DMA transfer returned zero word count 0x%x\n",nwrds,0,0,0,0,0);
/*sergey
#endif
*/
	  FAUNLOCK;
	  fadcBlockError=1;
	  return(nwrds);
	} 
      else 
	{  /* Error in DMA */
/*sergey
#ifdef VXWORKS
	  logMsg("faReadBlock: ERROR: sysVmeDmaDone returned an Error\n",0,0,0,0,0,0);
#else
*/
	  logMsg("faReadBlock: ERROR: vmeDmaDone returned an Error\n",0,0,0,0,0,0);
/*sergey
#endif
*/
	  FAUNLOCK;
	  fadcBlockError=1;
	  return(retVal>>2);
	}

    } 
  else 
    {  /*Programmed IO */

      /* Check if Bus Errors are enabled. If so then disable for Prog I/O reading */
      FALOCK;
      berr = vmeRead32(&(FAp[id]->ctrl1))&FA_ENABLE_BERR;
      if(berr)
	vmeWrite32(&(FAp[id]->ctrl1),vmeRead32(&(FAp[id]->ctrl1)) & ~FA_ENABLE_BERR);

      dCnt = 0;
      /* Read Block Header - should be first word */
      bhead = (unsigned int) *FApd[id]; 
#ifndef VXWORKS
      bhead = LSWAP(bhead);
#endif
      if((bhead&FA_DATA_TYPE_DEFINE)&&((bhead&FA_DATA_TYPE_MASK) == FA_DATA_BLOCK_HEADER)) {
	blknum = bhead&FA_DATA_BLKNUM_MASK;
	ehead = (unsigned int) *FApd[id];
#ifndef VXWORKS
	ehead = LSWAP(ehead);
#endif
	evnum1 = ehead&FA_DATA_TRIGNUM_MASK;
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
	  if( (vmeRead32(&(FAp[id]->ev_count)) & FA_EVENT_COUNT_MASK) == 0) 
	    {
	      logMsg("faReadBlock: FIFO Empty (0x%08x)\n",bhead,0,0,0,0,0);
	      FAUNLOCK;
	      return(0);
	    } 
	  else 
	    {
	      logMsg("faReadBlock: ERROR: Invalid Header Word 0x%08x\n",bhead,0,0,0,0,0);
	      FAUNLOCK;
	      return(ERROR);
	    }
	}

      ii=0;
      while(ii<nwrds) 
	{
	  val = (unsigned int) *FApd[id];
	  data[ii+2] = val;
#ifndef VXWORKS
	  val = LSWAP(val);
#endif
	  if( (val&FA_DATA_TYPE_DEFINE) 
	      && ((val&FA_DATA_TYPE_MASK) == FA_DATA_BLOCK_TRAILER) )
	    break;
	  ii++;
	}
      ii++;
      dCnt += ii;


      if(berr)
	vmeWrite32(&(FAp[id]->ctrl1),
		  vmeRead32(&(FAp[id]->ctrl1)) | FA_ENABLE_BERR);

      FAUNLOCK;
      return(dCnt);
    }

  FAUNLOCK;
  return(OK);
}

int
faReadBlockStatus(int id, volatile UINT32 *data, int nwrds, int rflag)
{

  int stat, retVal, xferCount, rmode, async;
  int dummy=0;
  unsigned int csr=0;
  
  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faReadBlockStatus: ERROR : FADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return(ERROR);
    }

  if(nwrds <= 0) nwrds= (FA_MAX_ADC_CHANNELS*FA_MAX_DATA_PER_CHANNEL) + 8;
  rmode = rflag&0x0f;
  async = rflag&0x80;

  /* Check for 8 byte boundary for address - insert dummy word (Slot 0 FADC Dummy DATA)*/
  if((unsigned long) (data)&0x7) 
    {
      dummy = 1;
    } 
  else 
    {
      dummy = 0;
    }


/*sergey
#ifdef VXWORKS
  retVal = sysVmeDmaDone(10000,1);
#else
  retVal = vmeDmaDone();
#endif
*/
retVal = usrVme2MemDmaDone();


  FALOCK;
  if(retVal > 0) 
    {
      /* Check to see that Bus error was generated by FADC */
      if(rmode == 2) 
	{
	  csr = vmeRead32(&(FAp[fadcMaxSlot]->csr));  /* from Last FADC */
	  stat = (csr)&FA_CSR_BERR_STATUS;  /* from Last FADC */
	}
      else
	{
	  stat = vmeRead32(&(FAp[id]->csr))&FA_CSR_BERR_STATUS;  /* from FADC id */
	}
      if((retVal>0) && (stat)) 
	{
	  xferCount = (/*sergey: nwrds -*/ (retVal>>2) + dummy);  /* Number of Longwords transfered */
	  FAUNLOCK;
	  return(xferCount); /* Return number of data words transfered */
	}
      else
	{
	  xferCount = (/*sergey: nwrds -*/ (retVal>>2) + dummy);  /* Number of Longwords transfered */
	  logMsg("faReadBlockStatus: DMA transfer terminated by unknown BUS Error (csr=0x%x nwrds=%d)\n",csr,xferCount,0,0,0,0);
	  FAUNLOCK;
	  return(ERROR);
	}
    } 
  else if (retVal == 0)
    { /* Block Error finished without Bus Error */
      logMsg("faReadBlockStatus: WARN: DMA transfer terminated by word count 0x%x\n",nwrds,0,0,0,0,0);
      FAUNLOCK;
      return(nwrds);
    } 
  else 
    {  /* Error in DMA */
      logMsg("faReadBlockStatus: ERROR: sysVmeDmaDone returned an Error\n",0,0,0,0,0,0);
      FAUNLOCK;
      return(retVal);
    }
  
}

int
faPrintBlock(int id, int rflag)
{

  int ii, blknum, evnum1;
  int nwrds=32768, dCnt, berr=0;
  unsigned int data, bhead, ehead;

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      printf("faPrintEvent: ERROR : FADC in slot %d is not initialized \n",id);
      return(ERROR);
    }

  /* Check if data available */
  FALOCK;
  if((vmeRead32(&(FAp[id]->ev_count))&FA_EVENT_COUNT_MASK)==0) 
    {
      printf("faPrintEvent: ERROR: FIFO Empty\n");
      FAUNLOCK;
      return(0);
    }

  /* Check if Bus Errors are enabled. If so then disable for reading */
  berr = vmeRead32(&(FAp[id]->ctrl1))&FA_ENABLE_BERR;
  if(berr)
    vmeWrite32(&(FAp[id]->ctrl1),
	      vmeRead32(&(FAp[id]->ctrl1)) & ~FA_ENABLE_BERR);
  
  dCnt = 0;
  /* Read Block Header - should be first word */
  bhead = (unsigned int) *FApd[id];
#ifndef VXWORKS
  bhead = LSWAP(bhead);
#endif
  if( (bhead&FA_DATA_TYPE_DEFINE)&&((bhead&FA_DATA_TYPE_MASK) == FA_DATA_BLOCK_HEADER)) 
    {
      blknum = bhead&FA_DATA_BLKNUM_MASK;
      ehead = (unsigned int) *FApd[id];
#ifndef VXWORKS
      ehead = LSWAP(ehead);
#endif
      evnum1 = ehead&FA_DATA_TRIGNUM_MASK;
      printf("%4d: ",dCnt+1); 
      faDataDecode(bhead);
      dCnt++;
      printf("%4d: ",dCnt+1); 
      faDataDecode(ehead);
      dCnt++;
    }
  else
    {
      /* We got bad data - Check if there is any data at all */
      if((vmeRead32(&(FAp[id]->ev_count))&FA_EVENT_COUNT_MASK)==0) 
	{
	  logMsg("faPrintBlock: FIFO Empty (0x%08x)\n",bhead,0,0,0,0,0);
	  FAUNLOCK;
	  return(0);
	} 
      else 
	{
	  logMsg("faPrintBlock: ERROR: Invalid Header Word 0x%08x\n",bhead,0,0,0,0,0);
	  FAUNLOCK;
	  return(ERROR);
	}
    }
  
  ii=0;
  while(ii<nwrds) 
    {
      data = (unsigned int) *FApd[id];
#ifndef VXWORKS
      data = LSWAP(data);
#endif
      printf("%4d: ",dCnt+1+ii); 
      faDataDecode(data);
      if((data&FA_DATA_TYPE_DEFINE)&&((data&FA_DATA_TYPE_MASK) == FA_DATA_BLOCK_TRAILER))
	break;
      if((data&FA_DATA_TYPE_DEFINE)&&((data&FA_DATA_TYPE_MASK) == FA_DATA_INVALID))
	break;
      ii++;
    }
  ii++;
  dCnt += ii;


  if(berr)
    vmeWrite32(&(FAp[id]->ctrl1),
	      vmeRead32( &(FAp[id]->ctrl1)) | FA_ENABLE_BERR );
  
  FAUNLOCK;
  return(dCnt);
  
}




/*****************************************************************************/

unsigned int
faReadCSR(int id)
{
  unsigned int rval;

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faReadCSR: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return(0);
    }
  
  FALOCK;
  rval = vmeRead32(&(FAp[id]->csr));
  FAUNLOCK;
  
  return(rval);
}


void
faClear(int id)
{

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faClear: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }
  FALOCK;
  vmeWrite32(&(FAp[id]->csr),FA_CSR_SOFT_RESET);
  FAUNLOCK;
}
void
faGClear()
{

  int ii, id;

  FALOCK;
  for(ii=0;ii<nfadc;ii++) 
    {
      id = fadcID[ii];
      if((id<=0) || (id>21) || (FAp[id] == NULL)) 
	{
	  logMsg("faGClear: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
	}
      else
	{
	  vmeWrite32(&(FAp[id]->csr),FA_CSR_SOFT_RESET);
	}
    }
  FAUNLOCK;

}

void
faClearError(int id)
{

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faClearErr: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }
  
  FALOCK;
  vmeWrite32(&(FAp[id]->csr),FA_CSR_ERROR_CLEAR);
  FAUNLOCK;

}
void
faGClearError()
{

  int ii, id;

  FALOCK;
  for(ii=0;ii<nfadc;ii++) 
    {
      id = fadcID[ii];
      if((id<=0) || (id>21) || (FAp[id] == NULL)) 
	{
	  logMsg("faGClearErr: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
	}
      else
	{
	  vmeWrite32(&(FAp[id]->csr),FA_CSR_ERROR_CLEAR);
	}
    }
  FAUNLOCK;

}


void
faReset(int id, int iFlag)
{
  unsigned int a32addr, addrMB;

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faReset: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }

  FALOCK;
  if(iFlag==0)
    {
      a32addr = vmeRead32(&(FAp[id]->adr32));
      addrMB  = vmeRead32(&(FAp[id]->adr_mb));
    }

  vmeWrite32(&(FAp[id]->csr),FA_CSR_HARD_RESET);
  taskDelay(2);

  if(iFlag==0)
    {
      vmeWrite32(&(FAp[id]->adr32),a32addr);
      vmeWrite32(&(FAp[id]->adr_mb),addrMB);
    }
  FAUNLOCK;

}

void
faSoftReset(int id)
{
  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faReset: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }

  FALOCK;
  vmeWrite32(&(FAp[id]->csr),FA_CSR_SOFT_RESET);
  FAUNLOCK;
  
}

/* #ifdef VERSION1 */
void
faResetToken(int id)
{

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faResetToken: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }
  
  FALOCK;
  vmeWrite32(&(FAp[id]->reset),FA_RESET_TOKEN);
  FAUNLOCK;
}
/* #endif */

void
faSetCalib(int id, unsigned short sdelay, unsigned short tdelay)
{
  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faSetCalib: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }
  
  FALOCK;
  vmeWrite32(&(FAp[id]->delay),(sdelay<<16) | tdelay);
  FAUNLOCK;

}

void
faChanDisable(int id, unsigned short cmask)
{

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faChanDisable: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }

  fadcChanDisable[id] = cmask;  /* Set Global Variable */

  FALOCK;
  /* Write New Disable Mask */
  vmeWrite32(&(FAp[id]->adc_config[1]), cmask);
  FAUNLOCK;

}

/* For VERSION2, bank is ignored */
void
faEnable(int id, int eflag, int bank)
{

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faEnable: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }

  FALOCK;

  if(eflag) 
    {  /* Enable Internal Trigger logic as well*/
      vmeWrite32(&(FAp[id]->ctrl2),
		FA_CTRL_GO | FA_CTRL_ENABLE_TRIG | FA_CTRL_ENABLE_SRESET | 
		FA_CTRL_ENABLE_INT_TRIG );
    }
  else
    {
      vmeWrite32(&(FAp[id]->ctrl2),
		FA_CTRL_GO | FA_CTRL_ENABLE_TRIG | FA_CTRL_ENABLE_SRESET);
    }
  FAUNLOCK;
}

void
faGEnable(int eflag, int bank)
{
  int ii;

  for(ii=0;ii<nfadc;ii++)
    faEnable(fadcID[ii],eflag,bank);

  if(fadcUseSDC)
    faSDC_Enable(1);

}

void
faDisable(int id, int eflag)
{

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faDisable: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }

  FALOCK;
  if(eflag)
    vmeWrite32(&(FAp[id]->ctrl2),0);   /* Turn FIFO Transfer off as well */
  else
    vmeWrite32(&(FAp[id]->ctrl2),FA_CTRL_GO);
  FAUNLOCK;
}

void
faGDisable(int eflag)
{
  int ii;

  if(fadcUseSDC)
    faSDC_Disable();

  for(ii=0;ii<nfadc;ii++)
    faDisable(fadcID[ii],eflag);

}



void
faTrig(int id)
{

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faTrig: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }

  FALOCK;
  if( vmeRead32(&(FAp[id]->ctrl1)) & (FA_ENABLE_SOFT_TRIG) )
    vmeWrite32(&(FAp[id]->csr), FA_CSR_TRIGGER);
  else
    logMsg("faTrig: ERROR: Software Triggers not enabled",0,0,0,0,0,0);
  FAUNLOCK;
}
void
faGTrig()
{
  int ii;

  for(ii=0;ii<nfadc;ii++)
    faTrig(fadcID[ii]);
}



void
faSync(int id)
{

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faSync: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }

  FALOCK;
  if(vmeRead32(&(FAp[id]->ctrl1))&(FA_ENABLE_SOFT_SRESET))
    vmeWrite32(&(FAp[id]->csr), FA_CSR_SYNC);
  else
    logMsg("faSync: ERROR: Software Sync Resets not enabled\n",0,0,0,0,0,0);
  FAUNLOCK;
}



/* Return Event/Block count for ADC in slot id */
int
faDready(int id, int dflag)
{
  unsigned int dcnt=0;

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faDready: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return(ERROR);
    }
  
  FALOCK;
  if(dflag)
    dcnt = vmeRead32(&(FAp[id]->blk_count))&FA_BLOCK_COUNT_MASK;
  else
    dcnt = vmeRead32(&(FAp[id]->ev_count))&FA_EVENT_COUNT_MASK;
  FAUNLOCK;

  
  return(dcnt);
}

/* Return a Block Ready status for ADC. If Block Level is =1 then return Event Ready status */
int
faBready(int id)
{
  int stat=0;

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faBready: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return(ERROR);
    }
  
  FALOCK;
  /*if(fadcBlockLevel==1) 
    {
      stat = (vmeRead32(&(FAp[id]->csr))) & FA_CSR_BLOCK_LEVEL_FLAG;
    }
  else
  {*/
      stat = (vmeRead32(&(FAp[id]->csr))) & FA_CSR_BLOCK_READY;
	  /*}*/
  FAUNLOCK;

  if(stat)
    return(1);
  else
    return(0);
}

unsigned int
faGBready()
{
  int ii, id, stat=0;
  unsigned int dmask=0;
  
  FALOCK;
  for(ii=0;ii<nfadc;ii++) 
    {
      id = fadcID[ii];
      
	  /*if(fadcBlockLevel==1) 
	{
	  stat = vmeRead32(&(FAp[id]->csr))&FA_CSR_BLOCK_LEVEL_FLAG;
	}
      else
	  {*/
	  stat = vmeRead32(&(FAp[id]->csr))&FA_CSR_BLOCK_READY;
	  /*}*/
      
      if(stat)
	dmask |= (1<<id);
    }
  FAUNLOCK;
  
  return(dmask);
}

/* return Scan mask for all initialized FADCs */
unsigned int
faScanMask()
{
  int ifadc, id, dmask=0;

  for(ifadc=0; ifadc<nfadc; ifadc++)
    {
      id = fadcID[ifadc];
      dmask |= (1<<id);
    }

  return(dmask);
}


/* if val>0 then set the busy level, if val=0 then read it back.
   if bflag>0 then force the module Busy */
int
faBusyLevel(int id, unsigned int val, int bflag)
{
  unsigned int blreg=0;

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faBusyLevel: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return(ERROR);
    }
  if(val>FA_BUSY_LEVEL_MASK)
    return(ERROR);
  
  /* if Val > 0 then set the Level else leave it alone*/
  FALOCK;
  if(val) 
    {
      if(bflag)
	vmeWrite32(&(FAp[id]->busy_level),(val | FA_FORCE_BUSY));
      else
	vmeWrite32(&(FAp[id]->busy_level),val);
    }
  else
    {
      blreg = vmeRead32(&(FAp[id]->busy_level));
      if(bflag)
	vmeWrite32(&(FAp[id]->busy_level),(blreg | FA_FORCE_BUSY));
    }
  FAUNLOCK;

  return((blreg&FA_BUSY_LEVEL_MASK));
}

int
faBusy(int id)
{
  unsigned int blreg=0;
  unsigned int dreg=0;

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faBusy: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return(ERROR);
    }
  
  FALOCK;
  blreg = vmeRead32(&(FAp[id]->busy_level))&FA_BUSY_LEVEL_MASK;
  dreg  = vmeRead32(&(FAp[id]->ram_word_count))&FA_RAM_DATA_MASK;
  FAUNLOCK;

  if(dreg>=blreg)
    return(1);
  else
    return(0);
}


void
faEnableSoftTrig(int id)
{

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faEnableSoftTrig: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }
  
  /* Clear the source */
  FALOCK;
  vmeWrite32(&(FAp[id]->ctrl1),
	    vmeRead32(&(FAp[id]->ctrl1)) & ~FA_TRIG_MASK );
  /* Set Source and Enable*/
  vmeWrite32(&(FAp[id]->ctrl1),
	    vmeRead32(&(FAp[id]->ctrl1)) | (FA_TRIG_VME | FA_ENABLE_SOFT_TRIG) );
  FAUNLOCK;
}
void
faGEnableSoftTrig()
{
  int ii, id;

  for(ii=0;ii<nfadc;ii++) 
    {
      id = fadcID[ii];
      faEnableSoftTrig(id);
    }
  
}


void
faDisableSoftTrig(int id)
{

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faDisableSoftTrig: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }
  
  FALOCK;
  vmeWrite32(&(FAp[id]->ctrl1), 
	    vmeRead32(&(FAp[id]->ctrl1)) & ~FA_ENABLE_SOFT_TRIG );
  FAUNLOCK;

}

void
faEnableSoftSync(int id)
{

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faEnableSoftSync: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }
  
  /* Clear the source */
  FALOCK;
  vmeWrite32(&(FAp[id]->ctrl1),
	    vmeRead32(&(FAp[id]->ctrl1)) & ~FA_SRESET_MASK);
  /* Set Source and Enable*/
  vmeWrite32(&(FAp[id]->ctrl1),
	    vmeRead32(&(FAp[id]->ctrl1)) | (FA_SRESET_VME | FA_ENABLE_SOFT_SRESET));
  FAUNLOCK;
}

void
faDisableSoftSync(int id)
{

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faDisableSoftSync: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }
  
  FALOCK;
  vmeWrite32(&(FAp[id]->ctrl1),
	    vmeRead32(&(FAp[id]->ctrl1)) & ~FA_ENABLE_SOFT_SRESET);
  FAUNLOCK;

}

void
faEnableClk(int id)
{

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faEnableClk: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }
  
  FALOCK;
  vmeWrite32(&(FAp[id]->ctrl1),
	    vmeRead32(&(FAp[id]->ctrl1)) | (FA_REF_CLK_INTERNAL|FA_ENABLE_INTERNAL_CLK) );
  FAUNLOCK;

}

void
faDisableClk(int id)
{

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faDisableClk: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }
  
  FALOCK;
  vmeWrite32(&(FAp[id]->ctrl1),
	    vmeRead32(&(FAp[id]->ctrl1)) & ~FA_ENABLE_INTERNAL_CLK );
  FAUNLOCK;

}

/*************************************************************************************
 *
 *  faEnableTriggerOut - Enable trigger out for front panel or p0
 * 
 *   output = 0 for FP trigger out
 *            1 for P0 trigger out
 *            2 for FP and P0 trigger out
 */

void
faEnableTriggerOut(int id, int output)
{
  int bitset=0;
  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faEnableBusError: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }

  if(output>2)
    {
      logMsg("faEnableTriggerOut: ERROR: output (%d) out of range.  Must be less than 3",
	     output,2,3,4,5,6);
      return;

    }

  switch(output)
    {
    case 0:
      bitset = FA_ENABLE_TRIG_OUT_FP;
      break;
    case 1:
      bitset = FA_ENABLE_TRIG_OUT_P0;
      break;
    case 2:
      bitset = FA_ENABLE_TRIG_OUT_FP | FA_ENABLE_TRIG_OUT_P0;
      break;
	
    }
  FALOCK;
  vmeWrite32(&(FAp[id]->ctrl1),
	    vmeRead32(&(FAp[id]->ctrl1)) | bitset );
  FAUNLOCK;



}

void
faEnableBusError(int id)
{

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faEnableBusError: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }
  
  FALOCK;
  vmeWrite32(&(FAp[id]->ctrl1),
	    vmeRead32(&(FAp[id]->ctrl1)) | FA_ENABLE_BERR );
  FAUNLOCK;

}
void
faGEnableBusError()
{
  int ii;

  FALOCK;
  for(ii=0;ii<nfadc;ii++) 
    {
      vmeWrite32(&(FAp[fadcID[ii]]->ctrl1),
		vmeRead32(&(FAp[fadcID[ii]]->ctrl1)) | FA_ENABLE_BERR );
    }
  FAUNLOCK;
  
}


void
faDisableBusError(int id)
{

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faDisableBusError: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }
  
  FALOCK;
  vmeWrite32(&(FAp[id]->ctrl1),
	    vmeRead32(&(FAp[id]->ctrl1)) & ~FA_ENABLE_BERR );
  FAUNLOCK;

}


void
faEnableMultiBlock(int tflag)
{
  int ii, id;
  unsigned int mode;

  if((nfadc <= 1) || (FAp[fadcID[0]] == NULL)) 
    {
      logMsg("faEnableMultiBlock: ERROR : Cannot Enable MultiBlock mode \n",0,0,0,0,0,0);
      return;
    }

  /* if token = 0 then send via P2 else via VXS */
  if(tflag)
    mode = (FA_ENABLE_MULTIBLOCK | FA_MB_TOKEN_VIA_P0);
  else
    mode = (FA_ENABLE_MULTIBLOCK | FA_MB_TOKEN_VIA_P2);
    
  for(ii=0;ii<nfadc;ii++) 
    {
      id = fadcID[ii];
      FALOCK;
      vmeWrite32(&(FAp[id]->ctrl1),
		vmeRead32(&(FAp[id]->ctrl1)) | mode );
      FAUNLOCK;
      faDisableBusError(id);
      if(id == fadcMinSlot) 
	{
	  FALOCK;
	  vmeWrite32(&(FAp[id]->ctrl1),
		    vmeRead32(&(FAp[id]->ctrl1)) | FA_FIRST_BOARD );
	  FAUNLOCK;
	}
      if(id == fadcMaxSlot) 
	{
	  FALOCK;
	  vmeWrite32(&(FAp[id]->ctrl1),
		    vmeRead32(&(FAp[id]->ctrl1)) | FA_LAST_BOARD );
	  FAUNLOCK;
	  faEnableBusError(id);   /* Enable Bus Error only on Last Board */
	}
    }

}

void
faDisableMultiBlock()
{
  int ii;

  if((nfadc <= 1) || (FAp[fadcID[0]] == NULL)) 
    {
      logMsg("faDisableMultiBlock: ERROR : Cannot Disable MultiBlock Mode\n",0,0,0,0,0,0);
      return;
    }
  
  FALOCK;
  for(ii=0;ii<nfadc;ii++)
    vmeWrite32(&(FAp[fadcID[ii]]->ctrl1),
	      vmeRead32(&(FAp[fadcID[ii]]->ctrl1)) & ~FA_ENABLE_MULTIBLOCK );
  FAUNLOCK;

}



int
faSetBlockLevel(int id, int level)
{
  int rval;

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faSetBlockLevel: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return(ERROR);
    }
  
  if(level<=0) level = 1;
  FALOCK;
  vmeWrite32(&(FAp[id]->blk_level), level);
  fadcBlockLevel = level;
  rval = vmeRead32(&(FAp[id]->blk_level)) & FA_BLOCK_LEVEL_MASK;
  FAUNLOCK;

  return(rval);

}
void
faGSetBlockLevel(int level)
{
  int ii;

  if(level<=0) level = 1;
  FALOCK;
  for(ii=0;ii<nfadc;ii++)
    vmeWrite32(&(FAp[fadcID[ii]]->blk_level), level);
  FAUNLOCK;

  fadcBlockLevel = level;
}

int
faSetClkSource(int id, int source)
{
  int rval;

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faSetClkSource: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return(ERROR);
    }
  
  FALOCK;
  vmeWrite32(&(FAp[id]->ctrl1),
	    vmeRead32(&(FAp[id]->ctrl1)) & ~FA_REF_CLK_SEL_MASK );
  if((source<0)||(source>7)) source = FA_REF_CLK_INTERNAL;
  vmeWrite32(&(FAp[id]->ctrl1),
	    vmeRead32(&(FAp[id]->ctrl1)) | source );
  rval = vmeRead32(&(FAp[id]->ctrl1)) & FA_REF_CLK_SEL_MASK;
  FAUNLOCK;


  return(rval);

}

int
faSetTrigSource(int id, int source)
{
  int rval;

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faSetTrigSource: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return(ERROR);
    }
  
  FALOCK;
  vmeWrite32(&(FAp[id]->ctrl1),
	    vmeRead32(&(FAp[id]->ctrl1)) & ~FA_TRIG_SEL_MASK );
  if((source<0)||(source>7)) source = FA_TRIG_FP_ISYNC;
  vmeWrite32(&(FAp[id]->ctrl1),
	    vmeRead32(&(FAp[id]->ctrl1)) | source );
  rval = vmeRead32(&(FAp[id]->ctrl1)) & FA_TRIG_SEL_MASK;
  FAUNLOCK;

  return(rval);

}

int
faSetSyncSource(int id, int source)
{
  int rval;

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faSetSyncSource: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return(ERROR);
    }
  
  FALOCK;
  vmeWrite32(&(FAp[id]->ctrl1),
	    vmeRead32(&(FAp[id]->ctrl1)) & ~FA_SRESET_SEL_MASK );
  if((source<0)||(source>7)) source = FA_SRESET_FP_ISYNC;
  vmeWrite32(&(FAp[id]->ctrl1),
	    vmeRead32(&(FAp[id]->ctrl1)) | source );
  rval = vmeRead32(&(FAp[id]->ctrl1)) & FA_SRESET_SEL_MASK;
  FAUNLOCK;

  return(rval);

}

/* Enable Front Panel Inputs (and Disable software triggers/syncs 
   but leave the clock source alone */
void
faEnableFP(int id)
{

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faEnableFP: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }
  
  FALOCK;
  vmeWrite32(&(FAp[id]->ctrl1),
	    vmeRead32(&(FAp[id]->ctrl1)) & 
	    ~(FA_TRIG_SEL_MASK | FA_SRESET_SEL_MASK | FA_ENABLE_SOFT_SRESET | FA_ENABLE_SOFT_TRIG));
  vmeWrite32(&(FAp[id]->ctrl1),
	    vmeRead32(&(FAp[id]->ctrl1)) | (FA_TRIG_FP_ISYNC | FA_SRESET_FP_ISYNC));
  FAUNLOCK;

}

/* Set trigger output options
 *   trigout bits:  
 *      0  0  1  Live Internal Trigger to Output
 *      0  1  0  Enable Front Panel Trigger Output
 *      1  0  0  Enable VXS Trigger Output
 *
 * RETURNS: OK, or ERROR if unsuccessful
 */

int
faSetTrigOut(int id, int trigout)
{
  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      printf("faSetTrigOut: ERROR : ADC in slot %d is not initialized \n",id);
      return ERROR;
    }

  if(trigout<0 || trigout > 7)
    {
      printf("faSetTrigOut: ERROR : Invalid trigout value (%d) \n",trigout);
      return ERROR;
    }

  FALOCK;
  vmeWrite32(&(FAp[id]->ctrl1),
	    (vmeRead32(&(FAp[id]->ctrl1)) & ~FA_TRIGOUT_MASK) |
	    trigout<<12);
  FAUNLOCK;

  return OK;
}

int
faResetTriggerCount(int id)
{
  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faResetTriggerCount: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return(ERROR);
    }

  FALOCK;
  vmeWrite32(&FAp[id]->trig_scal,FA_TRIG_SCAL_RESET);
  FAUNLOCK;

  return OK;
}

int
faSetThreshold(int id, unsigned short tvalue, unsigned short chmask)
{

  int ii, doWrite=0;
  unsigned int lovalue=0, hivalue=0;

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faSetThreshold: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return(ERROR);
    }

  if(chmask==0) chmask = 0xffff;  /* Set All channels the same */

  FALOCK;
  for(ii=0;ii<FA_MAX_ADC_CHANNELS;ii++) 
    {
      if(ii%2==0)
	{
	  lovalue = (vmeRead16(&FAp[id]->adc_thres[ii]));
	  hivalue = (vmeRead16(&FAp[id]->adc_thres[ii+1]));

	  if((1<<ii)&chmask)
	    {
	      lovalue = tvalue;
	      doWrite=1;
	    }
	  if((1<<(ii+1))&chmask)
	    {
	      hivalue = tvalue;
	      doWrite=1;
	    }

	  if(doWrite)
	    vmeWrite32((unsigned int *)&(FAp[id]->adc_thres[ii]),
		       lovalue<<16 | hivalue);

	  lovalue = 0; 
	  hivalue = 0;
	  doWrite=0;
	}
    }
  FAUNLOCK;

  return(OK);
}




/*sergey*/
int
faSetThresholdAll(int id, unsigned short tvalue[16])
{
  int ii;
  unsigned int wvalue=0;

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
  {
    logMsg("faSetThresholdAll: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
    return(ERROR);
  }

  FALOCK;
  for(ii=0; ii<FA_MAX_ADC_CHANNELS; ii++) 
  {
    if(ii%2==0)
	{
	  wvalue |= (tvalue[ii] << 16);
	  wvalue |= tvalue[ii+1];

	  printf("faSetThreshold: ch %d, wvalue=0x%08x\n",ii,wvalue);
	  vmeWrite32((unsigned int *)&(FAp[id]->adc_thres[ii]), wvalue);
	  wvalue=0;
	}
  }
  FAUNLOCK;

  return(OK);
}





int
faPrintThreshold(int id)
{
  int ii;
  unsigned short tval[FA_MAX_ADC_CHANNELS];

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faPrintThreshold: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return(ERROR);
    }

  FALOCK;
  for(ii=0;ii<FA_MAX_ADC_CHANNELS;ii++)
    {
      tval[ii] = vmeRead16(&(FAp[id]->adc_thres[ii]));
    }
  FAUNLOCK;


  printf(" Threshold Settings for FADC in slot %d:",id);
  for(ii=0;ii<FA_MAX_ADC_CHANNELS;ii++) 
    {
      if((ii%4)==0) 
	{
	  printf("\n");
	}
      printf("Chan %2d: %5d   ",(ii+1),tval[ii]);
    }
  printf("\n");
  

  return(OK);
}





/*sergey: set same pedestal for all channels, will change it later*/
int
faSetPedestal(int id, unsigned int wvalue)
{
  int ii;

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
  {
    logMsg("faSetPedestal: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
    return(ERROR);
  }

  FALOCK;
  for(ii=0; ii<FA_MAX_ADC_CHANNELS; ii++) 
  {
	vmeWrite32((unsigned int *)&(FAp[id]->adc_pedestal[ii]),wvalue);
  }
  FAUNLOCK;

  return(OK);
}
int
faPrintPedestal(int id)
{
  int ii;
  unsigned int tval[FA_MAX_ADC_CHANNELS];

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
  {
    logMsg("faPrintPedestal: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
    return(ERROR);
  }

  FALOCK;
  for(ii=0; ii<FA_MAX_ADC_CHANNELS;ii++)
  {
    tval[ii] = vmeRead32(&(FAp[id]->adc_pedestal[ii]));
  }
  FAUNLOCK;


  printf(" Pedestal Settings for FADC in slot %d:",id);
  for(ii=0;ii<FA_MAX_ADC_CHANNELS;ii++) 
  {
    if((ii%4)==0) 
	{
	  printf("\n");
	}
    printf("chan %2d: %3d   ",(ii+1),tval[ii]);
  }
  printf("\n");

  return(OK);
}









int
faSetDAC(int id, unsigned short dvalue, unsigned short chmask)
{
  int ii, doWrite=0;
  unsigned int lovalue=0, hivalue=0;
  
  if(id==0) id=fadcID[0];
  
  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faSetDAC: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return(ERROR);
    }
  
  if(chmask==0) chmask = 0xffff;  /* Set All channels the same */
  
  if(dvalue>0xfff) 
    {
      logMsg("faSetDAC: ERROR : DAC value (%d) out of range (0-255) \n",
	     dvalue,0,0,0,0,0);
      return(ERROR);
    }
  
  FALOCK;
  for(ii=0;ii<FA_MAX_ADC_CHANNELS;ii++)
    {

      if(ii%2==0)
	{
	  lovalue = (vmeRead16(&FAp[id]->dac[ii]));
	  hivalue = (vmeRead16(&FAp[id]->dac[ii+1]));

	  if((1<<ii)&chmask)
	    {
	      lovalue = dvalue&FA_DAC_VALUE_MASK;
	      doWrite=1;
	    }
	  if((1<<(ii+1))&chmask)
	    {
	      hivalue = (dvalue&FA_DAC_VALUE_MASK);
	      doWrite=1;
	    }

	  if(doWrite)
	    vmeWrite32((unsigned int *)&(FAp[id]->dac[ii]), 
		       lovalue<<16 | hivalue);

	  lovalue = 0; 
	  hivalue = 0;
	  doWrite=0;
	}

    }
  FAUNLOCK;

  return(OK);
}

void
faPrintDAC(int id)
{
  int ii;
  unsigned short dval[FA_MAX_ADC_CHANNELS];

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faPrintDAC: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }

  FALOCK;
  for(ii=0;ii<FA_MAX_ADC_CHANNELS;ii++)
    dval[ii] = vmeRead16(&(FAp[id]->dac[ii])) & FA_DAC_VALUE_MASK;
  FAUNLOCK;
  
  
  printf(" DAC Settings for FADC in slot %d:",id);
  for(ii=0;ii<FA_MAX_ADC_CHANNELS;ii++) 
    {
      if((ii%4)==0) printf("\n");
      printf("Chan %2d: %5d   ",(ii+1),dval[ii]);
    }
  printf("\n");
  
}

int
faSetChannelPedestal(int id, unsigned int chan, unsigned int ped)
{
  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faSetChannelPedestal: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return(ERROR);
    }

  if(chan>16)
    {
      logMsg("faSetChannelPedestal: ERROR : Channel (%d) out of range (0-15) \n",
	     chan,0,0,0,0,0);
      return(ERROR);
    }

  if(ped>0xffff) 
    {
      logMsg("faSetChannelPedestal: ERROR : PED value (%d) out of range (0-65535) \n",
	     ped,0,0,0,0,0);
      return(ERROR);
    }

  FALOCK;
  vmeWrite32(&FAp[id]->adc_pedestal[chan], ped);
  FAUNLOCK;

  return(OK);
}

int
faGetChannelPedestal(int id, unsigned int chan)
{
  unsigned int rval=0;

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faGetChannelPedestal: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return(ERROR);
    }

  if(chan>16)
    {
      logMsg("faGetChannelPedestal: ERROR : Channel (%d) out of range (0-15) \n",
	     chan,0,0,0,0,0);
      return(ERROR);
    }

  FALOCK;
  rval = vmeRead32(&FAp[id]->adc_pedestal[chan]) & FA_ADC_PEDESTAL_MASK;
  FAUNLOCK;

  return(rval);
}

int
faSetMGTTestMode(int id, unsigned int mode)
{
  unsigned int rval=0, wval=0;

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      printf("%s: ERROR : ADC in slot %d is not initialized \n",
	     __FUNCTION__,id);
      return(ERROR);
    }

#define USEMGTCTRL
#ifdef USEMGTCTRL
  FALOCK;
  if(mode)
    wval = (1<<1);
  else
    wval = 0;
  /* FIXME: Old way*/
/*   rval = vmeRead32(&FAp[id]->mgt_ctrl); */
/*   if(mode) */
/*     wval = (1<<1); */
/*   else */
/*     wval = rval & ~(1<<1); */

  vmeWrite32(&FAp[id]->mgt_ctrl,wval);
  FAUNLOCK;

  printf("%s: mode=%d  rval = 0x%08x    wval = 0x%08x\n",__FUNCTION__,
	 mode, rval, wval);
#else
  printf("%s: Function disabled\n",__FUNCTION__);
#endif

  return(OK);
}

/**************************************************************************************
 *
 *  faReadScalers - Scaler Data readout routine
 *        Readout the desired scalers (indicated by the channel mask), as well
 *        as the timer counter.  The timer counter will be the last word
 *        in the "data" array.
 *
 *    id     - Slot number of module to read
 *    data   - local memory address to place data
 *    chmask - Channel Mask (indicating which channels to read)
 *    rflag  - Readout Flag
 *            bit 0 - Latch Scalers before read
 *            bit 1 - Clear Scalers after read
 *
 *   RETURNS the number of 32bit words read, or ERROR if unsuccessful.
 */
int
faReadScalers(int id, volatile unsigned int *data, unsigned int chmask, int rflag)
{
  int doLatch=0, doClear=0, ichan=0;
  int dCnt=0;

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faReadScalers: ERROR : ADC in slot %d is not initialized \n",
	     id,0,0,0,0,0);
      return ERROR;
    }

  if(rflag & ~(FA_SCALER_CTRL_MASK))
    {
      logMsg("faReadScalers: WARN : rflag (0x%x) has undefined bits \n",
	     rflag,0,0,0,0,0);
    }

  doLatch = rflag&(1<<0);
  doClear = rflag&(1<<1);

  FALOCK;
  if(doLatch)
    vmeWrite32(&FAp[id]->scaler_ctrl,
	       FA_SCALER_CTRL_ENABLE | FA_SCALER_CTRL_LATCH);

  for(ichan=0; ichan<16; ichan++)
    {
      if( (1<<ichan) & chmask )
	{
	  data[dCnt] = vmeRead32(&FAp[id]->scaler[ichan]);
	  dCnt++;
	}
    }
  
  data[dCnt] =  vmeRead32(&FAp[id]->time_count);
  dCnt++;

  if(doClear)
    vmeWrite32(&FAp[id]->scaler_ctrl,
	       FA_SCALER_CTRL_ENABLE | FA_SCALER_CTRL_RESET);
  FAUNLOCK;

  return dCnt;

}

/**************************************************************************************
 *
 *  faPrintScalers - Scaler Print Out routine
 *        Print out the scalers as well as the timer counter.
 *
 *    id     - Slot number of module to read
 *    rflag  - Printout Flag
 *            bit 0 - Latch Scalers before read
 *            bit 1 - Clear Scalers after read
 *
 *   RETURNS ok if successful , or ERROR if unsuccessful.
 */
int
faPrintScalers(int id, int rflag)
{
  int doLatch=0, doClear=0, ichan=0;
  unsigned int data[16], time_count;

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faPrintScalers: ERROR : ADC in slot %d is not initialized \n",
	     id,0,0,0,0,0);
      return ERROR;
    }

  if(rflag & ~(FA_SCALER_CTRL_MASK))
    {
      logMsg("faPrintScalers: WARN : rflag (0x%x) has undefined bits \n",
	     rflag,0,0,0,0,0);
    }

  doLatch = rflag&(1<<0);
  doClear = rflag&(1<<1);

  FALOCK;
  if(doLatch)
    vmeWrite32(&FAp[id]->scaler_ctrl,
	       FA_SCALER_CTRL_ENABLE | FA_SCALER_CTRL_LATCH);

  for(ichan=0; ichan<16; ichan++)
    {
      data[ichan] = vmeRead32(&FAp[id]->scaler[ichan]);
    }
  
  time_count =  vmeRead32(&FAp[id]->time_count);

  if(doClear)
    vmeWrite32(&FAp[id]->scaler_ctrl,
	       FA_SCALER_CTRL_ENABLE | FA_SCALER_CTRL_RESET);
  FAUNLOCK;

  printf("%s: Scaler Counts\n",__FUNCTION__);
  for(ichan=0; ichan<16; ichan++)
    {
      if( (ichan%4) == 0 )
	printf("\n");

      printf("%2d: %10d ",ichan,data[ichan]);
    }
  printf("\n  timer: %10d\n",time_count);

  return OK;

}

int
faClearScalers(int id)
{
  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faClearScalers: ERROR : ADC in slot %d is not initialized \n",
	     id,0,0,0,0,0);
      return ERROR;
    }

  FALOCK;
  vmeWrite32(&FAp[id]->scaler_ctrl,
	   FA_SCALER_CTRL_ENABLE | FA_SCALER_CTRL_RESET);
  FAUNLOCK;

  return OK;
}


int
faLatchScalers(int id)
{
  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faLatchScalers: ERROR : ADC in slot %d is not initialized \n",
	     id,0,0,0,0,0);
      return ERROR;
    }

  FALOCK;
  vmeWrite32(&FAp[id]->scaler_ctrl,
	   FA_SCALER_CTRL_ENABLE | FA_SCALER_CTRL_LATCH);
  FAUNLOCK;

  return OK;
}

int
faEnableScalers(int id)
{
  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faEnableScalers: ERROR : ADC in slot %d is not initialized \n",
	     id,0,0,0,0,0);
      return ERROR;
    }

  FALOCK;
  vmeWrite32(&FAp[id]->scaler_ctrl,FA_SCALER_CTRL_ENABLE);
  FAUNLOCK;

  return OK;
}

int
faDisableScalers(int id)
{
  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faDisableScalers: ERROR : ADC in slot %d is not initialized \n",
	     id,0,0,0,0,0);
      return ERROR;
    }

  FALOCK;
  vmeWrite32(&FAp[id]->scaler_ctrl,~FA_SCALER_CTRL_ENABLE);
  FAUNLOCK;

  return OK;
}


/*********************************************
 *
 *  FADC Internal Trigger FADC Configuration and Control
 *  Routines.
 */
#include "cinclude/faItrig.c"



/* -------------------------------------------------------------------------------------

Utility routines
*/

void 
faPrintAuxScal(int id)
{
  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faPrintAuxScal: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }
  
  FALOCK;
  printf("Auxillary Scalers:\n");
  printf("       Word Count:         %d\n",
	 vmeRead32(&(FAp[id]->aux_scal[0])));
  printf("       Headers   :         %d\n",
	 vmeRead32(&(FAp[id]->aux_scal[2])));
  printf("       Trailers  :         %d\n",
	 vmeRead32(&(FAp[id]->aux_scal[4])));
  FAUNLOCK;

  return;
}

void 
faPrintFifoStatus(int id)
{ 
  unsigned int ibuf, bbuf, obuf, dflow;
  unsigned int wc[2],mt[2],full[2];
  unsigned int rdy[2];

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faPrintFifoStatus: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return;
    }
  
  FALOCK;
  dflow = vmeRead32(&(FAp[id]->dataflow_status));
  ibuf = vmeRead32(&(FAp[id]->status[0]))&0xdfffdfff;
  bbuf = vmeRead32(&(FAp[id]->status[1]))&0x1fff1fff;
  obuf = vmeRead32(&(FAp[id]->status[2]))&0x3fff3fff;
  FAUNLOCK;

  printf("%s: Fifo Buffers Status (DataFlow Status = 0x%08x\n",
	 __FUNCTION__,dflow);

  mt[1]  = full[1] = 0;
  wc[1]  = (ibuf&0x7ff0000)>>16;
  rdy[1] = (ibuf&0x80000000)>>31;
  if(ibuf&0x8000000)  full[1]=1;
  if(ibuf&0x10000000) mt[1]=1;

  printf("  Input Buffer : 0x%08x \n",ibuf);
  printf("    FPGA : wc=%d   Empty=%d Full=%d Ready=%d\n",wc[1],mt[1],full[1],rdy[1]);

  mt[0]=full[0]=0;
  wc[0]   =  bbuf&0x7ff;
  if(bbuf&0x800) full[0]=1;
  if(bbuf&0x1000) mt[0]=1;

  mt[1]=full[1]=0;
  wc[1]   = (bbuf&0x7ff0000)>>16;
  if(bbuf&0x8000000)  full[1]=1;
  if(bbuf&0x10000000) mt[1]=1;

  printf("  Build Buffer : 0x%08x \n",bbuf);
  printf("    BUF_A: wc=%d   Empty=%d Full=%d \n",wc[1],mt[1],full[1]);
  printf("    BUF_B: wc=%d   Empty=%d Full=%d \n",wc[0],mt[0],full[0]);

  mt[0]=full[0]=0;
  wc[0]   =  obuf&0xfff;
  if(obuf&0x1000) full[0]=1;
  if(obuf&0x2000) mt[0]=1;

  mt[1]=full[1]=0;
  wc[1]   = (obuf&0xfff0000)>>16;
  if(obuf&0x10000000)  full[1]=1;
  if(obuf&0x20000000) mt[1]=1;

  printf("  Output Buffer: 0x%08x \n",obuf);
  printf("    BUF_A: wc=%d   Empty=%d Full=%d \n",wc[1],mt[1],full[1]);
  printf("    BUF_B: wc=%d   Empty=%d Full=%d \n",wc[0],mt[0],full[0]);


  return;

}

int
faLive(int id, int sflag)
{
  int ilt=0;
  unsigned int live;

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      logMsg("faLive: ERROR : ADC in slot %d is not initialized \n",id,0,0,0,0,0);
      return(ERROR);
    }

  /* Read Current Scaler values */
  FALOCK;
  live = vmeRead32(&(FAp[id]->internal_trig_scal));
  
  vmeWrite32(&(FAp[id]->internal_trig_scal), 0x80000000);
  FAUNLOCK;
    
  if(live == 0)   /* scaler is zero or disabled */
    return(0);
  ilt = live;
  
  return(ilt);
}


void 
faDataDecode(unsigned int data)
{
  int i_print = 1;
  static unsigned int type_last = 15;	/* initialize to type FILLER WORD */
  static unsigned int time_last = 0;

  if( data & 0x80000000 )		/* data type defining word */
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
    case 0:		/* BLOCK HEADER */
      fadc_data.slot_id_hd = (data & 0x7C00000) >> 22;
      fadc_data.n_evts = (data & 0x3FF800) >> 11;
      fadc_data.blk_num = (data & 0x7FF);
      if( i_print ) 
	printf("%8X - BLOCK HEADER - slot = %d   n_evts = %d   n_blk = %d\n",
	       data, fadc_data.slot_id_hd, fadc_data.n_evts, fadc_data.blk_num);
      break;
    case 1:		/* BLOCK TRAILER */
      fadc_data.slot_id_tr = (data & 0x7C00000) >> 22;
      fadc_data.n_words = (data & 0x3FFFFF);
      if( i_print ) 
	printf("%8X - BLOCK TRAILER - slot = %d   n_words = %d\n",
	       data, fadc_data.slot_id_tr, fadc_data.n_words);
      break;
    case 2:		/* EVENT HEADER */
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
    case 3:		/* TRIGGER TIME */
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
    case 4:		/* WINDOW RAW DATA */
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
    case 5:		/* WINDOW SUM */
      fadc_data.over = 0; 
      fadc_data.chan = (data & 0x7800000) >> 23;
      fadc_data.adc_sum = (data & 0x3FFFFF);
      if( data & 0x400000 )
	fadc_data.over = 1;
      if( i_print ) 
	printf("%8X - WINDOW SUM - chan = %d   over = %d   adc_sum = %08x\n",
	       data, fadc_data.chan, fadc_data.over, fadc_data.adc_sum);
      break;
    case 6:		/* PULSE RAW DATA */
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
    case 7:		/* PULSE INTEGRAL */
      fadc_data.chan = (data & 0x7800000) >> 23;
      fadc_data.pulse_num = (data & 0x600000) >> 21;
      fadc_data.quality = (data & 0x180000) >> 19;
      fadc_data.integral = (data & 0x7FFFF);
      if( i_print ) 
	printf("%8X - PULSE INTEGRAL - chan = %d   pulse # = %d   quality = %d   integral = %d\n", 
	       data, fadc_data.chan, fadc_data.pulse_num, 
	       fadc_data.quality, fadc_data.integral);
      break;
    case 8:		/* PULSE TIME */
      fadc_data.chan = (data & 0x7800000) >> 23;
      fadc_data.pulse_num = (data & 0x600000) >> 21;
      fadc_data.quality = (data & 0x180000) >> 19;
      fadc_data.time = (data & 0xFFFF);
      if( i_print ) 
	printf("%8X - PULSE TIME - chan = %d   pulse # = %d   quality = %d   time = %d\n", 
	       data, fadc_data.chan, fadc_data.pulse_num, 
	       fadc_data.quality, fadc_data.time);
      break;
    case 9:		/* STREAMING RAW DATA */
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
    case 10:		/* PULSE AMPLITUDE DATA */
      fadc_data.chan = (data & 0x7800000) >> 23;
      fadc_data.pulse_num = (data & 0x600000) >> 21;
      fadc_data.vmin = (data & 0x1FF000) >> 12;
      fadc_data.vpeak = (data & 0xFFF);
      if( i_print ) 
	printf("%8X - PULSE V - chan = %d   pulse # = %d   vmin = %d   vpeak = %d\n", 
	       data, fadc_data.chan, fadc_data.pulse_num, 
	       fadc_data.vmin, fadc_data.vpeak);
      break;

    case 11:		/* INTERNAL TRIGGER WORD */
      fadc_data.trig_type_int = data & 0x7;
      fadc_data.trig_state_int = (data & 0x8) >> 3;
      fadc_data.evt_num_int = (data & 0xFFF0) >> 4;
      fadc_data.err_status_int = (data & 0x10000) >> 16;
      if( i_print ) 
	printf("%8X - INTERNAL TRIGGER - type = %d   state = %d   num = %d   error = %d\n",
	       data, fadc_data.trig_type_int, fadc_data.trig_state_int, fadc_data.evt_num_int,
	       fadc_data.err_status_int);
    case 12:		/* UNDEFINED TYPE */
      if( i_print ) 
	printf("%8X - UNDEFINED TYPE = %d\n", data, fadc_data.type);
      break;
    case 13:		/* END OF EVENT */
      if( i_print ) 
	printf("%8X - END OF EVENT = %d\n", data, fadc_data.type);
      break;
    case 14:		/* DATA NOT VALID (no data available) */
      if( i_print ) 
	printf("%8X - DATA NOT VALID = %d\n", data, fadc_data.type);
      break;
    case 15:		/* FILLER WORD */
      if( i_print ) 
	printf("%8X - FILLER WORD = %d\n", data, fadc_data.type);
      break;
    }
	
  type_last = fadc_data.type;	/* save type of current data word */
		   
}        

#ifdef HPS
void
faWriteHPSConfig(int id, unsigned int config6, unsigned int config7)
{
  if(config6==0) config6 = 0x386;
  if(config7==0) config7 = 0xa4;

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      printf("%s: ERROR : ADC in slot %d is not initialized \n",__FUNCTION__,
	     id);
      return;
    }

  printf("%s: Writing 0x%x to FA%d config6\n",__FUNCTION__,
	 config6,id);
  printf("%s: Writing 0x%x to FA%d config7\n",__FUNCTION__,
	 config7,id);

  FALOCK;
  vmeWrite32(&(FAp[id]->config6),config6);
  vmeWrite32(&(FAp[id]->config7),config7);

  config6 = vmeRead32(&(FAp[id]->config6));
  config7 = vmeRead32(&(FAp[id]->config7));
  FAUNLOCK;

  printf("%s: Readback config6 = 0x%x\n",__FUNCTION__,config6&0xffff);
  printf("%s: Readback config7 = 0x%x\n",__FUNCTION__,config7&0xffff);

}

/**************************************************************************************
 *
 *  faSetHPSParameters - Set the parameters specific for HPS
 *        These parameters can be set at any time, but would be
 *        safest to set prior to a SYNC (by TI Master).
 *
 *    id     
 *           - Slot number of module to read
 *    tot    
 *           - Time Over Threshold value for integration window
 *             Value must be less than 4095
 *    maxIntTime 
 *           - The Maximum Integration time (in 16 ns steps)
 *             Must be less than 8
 *    sumScaleFactor 
 *           - Scale Factor for the total sum reported to the CTP
 *             i.e. Sum/(2^sumScaleFactor)
 *             Must be less than 15
 *
 *   RETURNS OK if successful, otherwise ERROR
 */

int
faSetHPSParameters(int id, unsigned int tot, 
		   unsigned int maxIntTime, unsigned int sumScaleFactor)
{
  unsigned int config6=0, config7=0;

  if(id==0) id=fadcID[0];

  if((id<=0) || (id>21) || (FAp[id] == NULL)) 
    {
      printf("%s: ERROR : ADC in slot %d is not initialized \n",__FUNCTION__,
	     id);
      return ERROR;
    }

  if(tot>0x1000)
    {
      printf("%s: ERROR : Invalid value for tot (%d)\n", 
	     __FUNCTION__,tot);
      return ERROR;
    }
  config6 = tot;

  if(maxIntTime>8)
    {
      printf("%s: ERROR: Invalid value for maxIntTime (%d).  Must be less than 8.\n",
	     __FUNCTION__, maxIntTime);
      return ERROR;
    }
  config7 = maxIntTime;

  if(sumScaleFactor>15)
    {
      printf("%s: ERROR: Invalid value for sumScaleFactor (%d).  Must be less than 12.\n",
	     __FUNCTION__, sumScaleFactor);
      return ERROR;
    }

  config7 |= ((sumScaleFactor)<<4);

  faWriteHPSConfig(id, config6, config7);

  return OK;
}
#endif /* HPS */

/***************************************************************************************
   JLAB FADC Signal Distribution Card (SDC) Routines

   cFlag:  controls the configuation of the SDC
          
          0:  Default Mode  Internal CLK, Sync External Trigger and Sync Reset
        > 0:  Pass through mode
                    
   bMask:  mask of Busy enables for the SDC - Do not Enable busy if there is no FADC

*/

int
faSDC_Config(unsigned short cFlag, unsigned short bMask)
{

  if(FASDCp == NULL) 
    {
      logMsg("faSDC_Config: ERROR : Cannot Configure FADC Signal Board \n",0,0,0,0,0,0);
      return(ERROR);
    }
  
  /* Reset the Board */
  FASDCLOCK;
  vmeWrite16(&(FASDCp->csr),FASDC_CSR_INIT);

  if(cFlag == 0) 
    {
      /* Default - Enable Internal Clock, Sync Trigger and Sync-Reset*/
      vmeWrite16(&(FASDCp->ctrl),(FASDC_CTRL_ENABLE_SOFT_TRIG | FASDC_CTRL_ENABLE_SOFT_SRESET));
    }
  else if(cFlag==1) 
    {
      /* Pass Through - */
      vmeWrite16(&(FASDCp->ctrl),(FASDC_CTRL_NOSYNC_TRIG | FASDC_CTRL_NOSYNC_SRESET | 
				 FASDC_CTRL_ENABLE_SOFT_TRIG | FASDC_CTRL_ENABLE_SOFT_SRESET));
    }
  else 
    { 
      /* Level Translator */
      vmeWrite16(&(FASDCp->ctrl),(FASDC_CTRL_NOSYNC_TRIG | FASDC_CTRL_NOSYNC_SRESET));
    }
  
  vmeWrite16(&(FASDCp->busy_enable),bMask);
  FASDCUNLOCK;
  
  return(OK);
}

void
faSDC_Status(int sFlag)
{

  unsigned short sdc[4];


  if(FASDCp == NULL) 
    {
      printf("faSDC_Status: ERROR : No FADC SDC available \n");
      return;
    }
  
  FASDCLOCK;
  sdc[0] = vmeRead16(&(FASDCp->csr));
  sdc[1] = vmeRead16(&(FASDCp->ctrl))&FASDC_CTRL_MASK;
  sdc[2] = vmeRead16(&(FASDCp->busy_enable))&FASDC_BUSY_MASK;
  sdc[3] = vmeRead16(&(FASDCp->busy_status));
  FASDCUNLOCK;


#ifdef VXWORKS
  printf("\nSTATUS for FADC Signal Distribution Card at base address 0x%x \n",(UINT32) FASDCp);
#else
  printf("\nSTATUS for FADC Signal Distribution Card at VME (Local) base address 0x%x (0x%x)\n",
	 (UINT32) FASDCp - fadcA16Offset, (UINT32) FASDCp);
#endif
  printf("---------------------------------------------------------------- \n");

  printf(" Board Firmware Rev/ID = 0x%02x\n",((sdc[0]&0xff00)>>8));
  printf(" Registers: \n");
  printf("   CSR         = 0x%04x     Control     = 0x%04x\n",sdc[0],sdc[1]);
  printf("   Busy Enable = 0x%04x     Busy Status = 0x%04x\n",sdc[2],sdc[3]);
  printf("\n");

  if((sdc[1]&FASDC_CTRL_CLK_EXT))
    printf(" Ref Clock : External\n");
  else
    printf(" Ref Clock : Internal\n");

  if((sdc[1]&FASDC_CTRL_ENABLE_SOFT_TRIG)) 
    {
      printf(" Software Triggers\n");
    }
  else
    {
      if((sdc[1]&FASDC_CTRL_NOSYNC_TRIG))
	printf(" External Triggers (Pass through)\n");
      else
	printf(" External Triggers (Sync with clock)\n");
    }

  if((sdc[1]&FASDC_CTRL_ENABLE_SOFT_SRESET)) 
    {
      printf(" Software Sync Reset\n");
    }
  else
    {
      if((sdc[1]&FASDC_CTRL_NOSYNC_SRESET))
	printf(" External Sync Reset (Pass through)\n");
      else
	printf(" External Sync Reset (Sync with clock)\n");
    }
  
}


void
faSDC_Enable(int nsync)
{

  if(FASDCp == NULL) 
    {
      logMsg("faSDC_Enable: ERROR : No FADC SDC available \n",0,0,0,0,0,0);
      return;
    }
  
  FASDCLOCK;
  if(nsync != 0) /* FP triggers only */
    vmeWrite16(&(FASDCp->ctrl),FASDC_CTRL_ENABLE_SOFT_SRESET);
  else      /* both FP triggers and sync reset */
    vmeWrite16(&(FASDCp->ctrl),0);
  FASDCUNLOCK;
}

void
faSDC_Disable()
{

  if(FASDCp == NULL) 
    {
      logMsg("faSDC_Disable: ERROR : No FADC SDC available \n",0,0,0,0,0,0);
      return;
    }
  
  FASDCLOCK;
  vmeWrite16(&(FASDCp->ctrl),(FASDC_CTRL_ENABLE_SOFT_TRIG | FASDC_CTRL_ENABLE_SOFT_SRESET));
  FASDCUNLOCK;
}



void
faSDC_Sync()
{

  if(FASDCp == NULL) 
    {
      logMsg("faSDC_Sync: ERROR : No FADC SDC available \n",0,0,0,0,0,0);
      return;
    }
  
  FASDCLOCK;
  vmeWrite16(&(FASDCp->csr),FASDC_CSR_SRESET);
  FASDCUNLOCK;
}

void
faSDC_Trig()
{
  if(FASDCp == NULL) 
    {
      logMsg("faSDC_Trig: ERROR : No FADC SDC available \n",0,0,0,0,0,0);
      return;
    }
  
  FASDCLOCK;
  vmeWrite16(&(FASDCp->csr),FASDC_CSR_TRIG);
  FASDCUNLOCK;
}

int
faSDC_Busy()
{
  int busy=0;

  if(FASDCp == NULL) 
    {
      logMsg("faSDC_Busy: ERROR : No FADC SDC available \n",0,0,0,0,0,0);
      return -1;
    }
  
  FASDCLOCK;
  busy = vmeRead16(&(FASDCp->csr))&FASDC_CSR_BUSY;
  FASDCUNLOCK;

  return(busy);
}


#else /* dummy version*/

void
fadcLib_dummy()
{
  return;
}

#endif
