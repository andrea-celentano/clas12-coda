/****************************************************************************** 
 * 
 *  sspLib.c    -  Driver library for configuration of JLAB Subsystem Processor 
 *                 (SSP) using a VxWorks 5.4 or later, or Linux based Single  
 *                 Board computer. 
 * 
 *                 Currently Supports SSP Type = 1 (Hall D) 
 * 
 *  Authors: Ben Raydo 
 *           Jefferson Lab Fast Electronics Group 
 *           August 2013 
 * 
 *           Bryan Moffit 
 *           Jefferson Lab Data Acquisition Group 
 *           September 2013 
 * 
 * 
 */ 
/*
#if defined(VXWORKS) || defined(Linux_vme)
*/
#if defined(Linux_vme)

 
#ifdef VXWORKS 
#include <vxWorks.h> 
/*#include "vxCompat.h"*/
#include <logLib.h> 
#include <taskLib.h> 
#include <intLib.h> 
#include <iv.h> 
#include <semLib.h> 
#include <vxLib.h> 
#else 
#include <unistd.h> 
#include <stddef.h> 
#include "jvme.h" 
#endif 
#include <pthread.h> 
#include <stdio.h> 
#include <string.h> 
 
#include "sspLib.h"
#include "xxxConfig.h"

#ifdef VXWORKS 
#define SYNC()		{ __asm__ volatile("eieio"); __asm__ volatile("sync"); } 
#endif 
 
/* Global Variables */ 
static int active;

static int nSSP;                                 /* Number of SSPs found with sspInit(..) */ 
volatile SSP_regs *pSSP[MAX_VME_SLOTS+1];        /* pointers to SSP memory map */ 
volatile unsigned int *SSPpf[MAX_VME_SLOTS + 1]; /* pointers to VSCM FIFO memory */
volatile unsigned int *SSPpmb;                   /* pointer to Multiblock Window */
int sspSL[MAX_VME_SLOTS+1];                      /* array of slot numbers for SSPs */ 
unsigned int sspAddrList[MAX_VME_SLOTS+1];       /* array of a24 addresses for SSPs */ 

static int sspA32Base   = 0x11000000/*0x08800000*/;                   /* Minimum VME A32 Address for use by FADCs */
static int sspA32Offset = 0x08000000;                   /* Difference in CPU A32 Base - VME A32 Base */
static int sspA24Offset=0;                              /* Difference in Local A24 Base and VME A24 Base */ 

static int minSlot = 21;
static int maxSlot = 1;

/* Mutex to guard read/writes */ 
pthread_mutex_t   sspMutex = PTHREAD_MUTEX_INITIALIZER; 
#define SSPLOCK      if(pthread_mutex_lock(&sspMutex)<0) perror("pthread_mutex_lock"); 
#define SSPUNLOCK    if(pthread_mutex_unlock(&sspMutex)<0) perror("pthread_mutex_unlock"); 
 
/* Static routine prototypes */ 
static void sspSelectSpi(int id, int sel); 
static void sspFlashGetId(int id, unsigned char *rsp); 
static void sspReloadFirmware(int id); 
static unsigned char sspFlashGetStatus(int id); 
static unsigned char sspTransferSpi(int id, unsigned char data); 




/* static variables to be set by sspConfig() */
typedef struct
{
	int xmin;
	int xmax;
	int prescale;
} singles_prescale;

static int block_level = 1;
static int bus_error = 1;
static int window_width  = 100;
static int window_offset = 825;
static int trigger_latency = 500;

static int singles_emin[2] = {0, 0};
static int singles_emax[2] = {8191, 8191};
static int singles_nmin[2] = {0, 0};
static int singles_emin_en[2] = {1, 1};
static int singles_emax_en[2] = {1, 1};
static int singles_nmin_en[2] = {1, 1};
static singles_prescale singles_prescalers[2][7] = {
		{						// Singles 0
			{-31, 31, 0},	// Region 0
			{-31, -31, 0},	// Region 1
			{-31, -31, 0},	// Region 2
			{-31, -31, 0},	// Region 3
			{-31, -31, 0},	// Region 4
			{-31, -31, 0},	// Region 5
			{-31, -31, 0}	// Region 6
		},
		{						// Singles 1
			{-31, 31, 0},	// Region 0
			{-31, -31, 0},	// Region 1
			{-31, -31, 0},	// Region 2
			{-31, -31, 0},	// Region 3
			{-31, -31, 0},	// Region 4
			{-31, -31, 0},	// Region 5
			{-31, -31, 0}	// Region 6
		}
	};

static int cosmic_timecoincidence = 10;
static int cosmic_pattern = 0xFE;

static int pairs_timecoincidence[2] = {2, 2};
static int pairs_emin[2] = {0, 0};
static int pairs_emax[2] = {8191, 8191};
static int pairs_nmin[2] = {0, 0};
static int pairs_summax[2] = {8191, 8191};
static int pairs_summin[2] = {0, 0};
static int pairs_summax_en[2] = {1, 1};
static int pairs_diffmax[2] = {8191, 8191};
static int pairs_diffmax_en[2] = {1, 1};
static int pairs_coplanartolerance[2] = {180, 180};
static int pairs_coplanartolerance_en[2] = {1, 1};
static float pairs_edfactor[2] = {1.0f, 1.0f};
static int pairs_edmin[2] = {0, 0};
static int pairs_ed_en[2] = {1, 1};

static int pulser_freq = 0;

static int ssp_io_mux[SD_SRC_NUM];

/* readout options */
static int a24_a32, sngl_blt_mblt, sst_rate;

static SSP_CONF sspconf[NBOARD];
 
static unsigned int  
sspReadReg(volatile unsigned int *addr) 
{ 
#ifdef VXWORKS 
  unsigned int result = *addr; 
  SYNC(); 
  return result; 
#else 
  return vmeRead32(addr); 
#endif 
} 
 
static void  
sspWriteReg(volatile unsigned int *addr, unsigned int val) 
{ 
#ifdef VXWORKS 
  unsigned int *addr0 = (unsigned int *)( ((unsigned int)addr) & 0xFFFF0000); 
  *addr = val; 
  *addr0 = 0;	// nasty hack for 5500 cpus that have kernel write optimizations enabled (ensures no sequential address writes exist in write queue) 
  SYNC(); 
#else 
  vmeWrite32(addr, val); 
#endif 
} 
 
/************************************************************ 
 * SSP Main 
 ************************************************************/ 
 
/******************************************************************************* 
 * 
 * sspInit(unsigned int addr, int iFlag) 
 *    addr: vme a24 base address 
 *    iFlag: 
 *        bits 1:0 - Mode 
 *           0 - disabled 
 *               clk src = 0 
 *               sync src = 0 
 *               trig src = 0 
 *           1 - local/P2LVDS 
 *               clk src = LOCAL 
 *               sync src = P2LVDSIN0 
 *               trig src = P2LVDSIN1 
 *           2 - local/FPLVDS 
 *               clk src = LOCAL 
 *               sync src = FPLVDSIN0 
 *               trig src = FPLVDSIN1 
 *           3 - vxs 
 *               clk src = VXS SWB (SD) 
 *               sync src = VXS SWB (SD) 
 *               trig src = VXS SWB (SD) 
 * 
 *        bit 12 - Skip initialization the clock/syncReset/trigger source 
 *                 Setup (keeping the current values the same) 
 * 
 *        bit 13 - Ignore version compatibility between firware and library 
 * 
 *        bit 14 - Exit before board initialization (just map structure pointer) 
 * 
 *        bit 15 -  Use sspAddrList instead of addr and addr_inc 
 *                  for VME addresses 
 * 
 *        bits 23:16 - Fiber Enable 
 *           see TRG_CTRL_FIBER_ENx definitions in ssp.h 
 *        bits 31:24 - GTP data source 
 *           see TRG_CTRL_GTPSRC_* definitions in ssp.h 
 * 
 * Note: sspInit should only be called once the clock source is stable and 
 *       remains so. If the clock source disappears or changes source sspInit() 
 *       must be called again to properly initialize the ssp. 
 * 
 */ 
 
int  
sspInit(unsigned int addr, unsigned int addr_inc, int nfind, int iFlag) 
{
  int useList=0, noBoardInit=0, noFirmwareCheck=0;; 
  unsigned int rdata, laddr, laddr_inc, boardID, a32addr; 
  int issp=0, islot=0, res; 
  int result=OK; 
  volatile SSP_regs *ssp; 
  unsigned int firmwareInfo=0, sspVersion=0; 
  int ii;
 
  /* Check if we're skipping the firmware check */ 
  if(iFlag & SSP_INIT_SKIP_FIRMWARE_CHECK) 
  { 
    printf("%s: noFirmwareCheck\n",__FUNCTION__); 
    noFirmwareCheck=1; 
  } 
 
  /* Check if we're skipping initialization, and just mapping the structure pointer */ 
  if(iFlag & SSP_INIT_NO_INIT) 
  { 
    printf("%s: noBoardInit\n",__FUNCTION__); 
    noBoardInit=1; 
  } 
 
  /* Check if we're initializing using a list */ 
  if(iFlag & SSP_INIT_USE_ADDRLIST) 
  { 
    printf("%s: useList\n",__FUNCTION__); 
    useList=1; 
  } 
 
  /* Check for valid address */ 
  if( (addr==0) && (useList==0) ) 
  { 
    useList=1; 
    nfind=16; 
 
    /* Loop through JLab Standard GEOADDR to VME addresses to make a list */ 
    for(islot=3; islot<11; islot++) /* First 8 */
	{
	  sspAddrList[islot-3] = (islot<<19); 
    }
   
    /* Skip Switch Slots */ 
       
    for(islot=13; islot<21; islot++) /* Last 8 */ 
	{
	  sspAddrList[islot-5] = (islot<<19); 
	}
  } 
  else if(addr > 0x00ffffff)  
  { /* A32 Addressing */ 
    printf("%s: ERROR: A32 Addressing not allowed for SSP configuration space\n", __FUNCTION__); 
    return(ERROR); 
  } 
  else 
  { /* A24 Addressing for ONE SSP */ 
    if( ((addr_inc==0)||(nfind==0)) && (useList==0) ) nfind = 1; /* assume only one SSP to initialize */ 
  } 
 
  /* Get the SSP address */ 
#ifdef VXWORKS 
  res = sysBusToLocalAdrs(0x39,(char *)addr,(char **)&laddr); 
#else 
  res = vmeBusToLocalAdrs(0x39,(char *)addr,(char **)&laddr); 
#endif 
 
  if (res != 0)
  { 
#ifdef VXWORKS 
    printf("%s: ERROR in sysBusToLocalAdrs(0x39,0x%x,&laddr) \n", 
	     __FUNCTION__,addr); 
#else 
    printf("%s: ERROR in vmeBusToLocalAdrs(0x39,0x%x,&laddr) \n", 
	     __FUNCTION__,addr); 
#endif 
    return(ERROR); 
  } 
  sspA24Offset = laddr - addr; 
 



  nSSP = 0;
  for(issp=0; issp<nfind; issp++)
  { 
    if(useList==1) 
    { 
	  laddr_inc = sspAddrList[issp] + sspA24Offset; 
	} 
    else 
	{ 
	  laddr_inc = laddr + issp*addr_inc; 
	} 
 
    ssp = (volatile SSP_regs *)laddr_inc; 


      /* Check if Board exists at that address */ 
#ifdef VXWORKS 
    res = vxMemProbe((char *) &(ssp->Cfg.BoardId),VX_READ,4,(char *)&rdata); 
#else 
    res = vmeMemProbe((char *) &(ssp->Cfg.BoardId),4,(char *)&rdata); 
#endif 
 
    if(res < 0)  
	{ 
#ifdef VXWORKS 
	  printf("%s: ERROR: No addressable board at addr=0x%x\n", 
		 __FUNCTION__,(UINT32) ssp); 
#else 
	  printf("%s: ERROR: No addressable board at VME (Local) addr=0x%x (0x%x)\n", 
		 __FUNCTION__, 
		 (UINT32) laddr_inc-sspA24Offset, (UINT32) ssp); 
#endif 
	} 
    else  
	{ 
	  /* Check that it is a ssp */ 
	  if(rdata != SSP_CFG_BOARDID) 
	  { 
#ifdef DEBUG
	    printf(" WARN: For board at 0x%x, Invalid Board ID: 0x%x\n", (UINT32) laddr_inc-sspA24Offset, rdata); 
#endif
	    continue; 
	  } 
	  else  
	  {
	    /* Check if this is board has a valid slot number */ 
	    boardID =  (sspReadReg(&ssp->Cfg.FirmwareRev)&SSP_CFG_SLOTID_MASK)>>24; 
	    if((boardID <= 0)||(boardID >21))  
		{ 
		  printf(" WARN: Board Slot ID is not in range: %d (this module ignored)\n" ,boardID); 
		  continue; 
		} 
	    else 
		{ 
		  pSSP[boardID] = (volatile SSP_regs *)(laddr_inc); 
		  sspSL[nSSP] = boardID; 

 
		  /* Get the Firmware Information and print out some details */ 
		  firmwareInfo = sspReadReg(&pSSP[boardID]->Cfg.FirmwareRev); 
		  if(firmwareInfo>0) 
		  { 
		    printf("  Slot %2d: Type %d \tFirmware (major.minor): %d.%d\n", 
			     boardID, 
			     (firmwareInfo & SSP_CFG_SSPTYPE_MASK)>>16,  
			     (firmwareInfo & SSP_CFG_FIRMWAREREV_MAJOR_MASK)>>8,  
			     (firmwareInfo & SSP_CFG_FIRMWAREREV_MINOR_MASK)); 
		    sspVersion = firmwareInfo&0xFFF; 
		    if(sspVersion < SSP_SUPPORTED_FIRMWARE) 
			{ 
			  if(noFirmwareCheck) 
			  { 
			    printf("   WARN: Firmware version (0x%x) not supported by this driver.\n", 
				     sspVersion); 
			    printf("          Supported version = 0x%x (IGNORED)\n", 
				     SSP_SUPPORTED_FIRMWARE); 
			  } 
			  else 
			  { 
			    printf("   ERROR: Firmware version (0x%x) not supported by this driver.\n", 
				     sspVersion); 
			    printf("          Supported version = 0x%x\n", 
				     SSP_SUPPORTED_FIRMWARE); 
			    pSSP[boardID] = NULL; 
			    continue; 
			  } 
			} 
		  } 
		  else 
		  { 
		    printf("  Slot %2d:  ERROR: Invalid firmware 0x%08x\n", 
			     boardID,firmwareInfo); 
		    pSSP[boardID] = NULL; 
		    continue; 
		  } 
  
		  printf("Initialized SSP %2d  Slot # %2d at address 0x%08x (0x%08x)\n", 
			 nSSP, sspSL[nSSP],(UINT32) pSSP[(sspSL[nSSP])], 
				 (UINT32) pSSP[(sspSL[nSSP])]-sspA24Offset); 
		} 
	  } 
	  nSSP++; 
	} 
  }


  /* Program an A32 access address for SSP's FIFO */
  for(ii=0; ii<nSSP; ii++) 
  {
    a32addr = sspA32Base + (ii * SSP_MAX_FIFO);

    vmeWrite32(&pSSP[sspSL[ii]]->EB.AD32, ((a32addr >> 16) & 0xFF80) | 0x0001);

	printf("sspInit: a32addr=0x%08x, write to AD32 register 0x%08x\n",a32addr,((a32addr >> 16) & 0xFF80) | 0x0001);

#ifdef VXWORKS
    res = sysBusToLocalAdrs(0x09,(char *)a32addr,(char **)&laddr);
    if (res != 0) 
	{
	  printf("sspInit: ERROR in sysBusToLocalAdrs(0x09,0x%x,&laddr) \n",a32addr);
	  return(ERROR);
	}
#else
    res = vmeBusToLocalAdrs(0x09,(char *)a32addr,(char **)&laddr);
    if (res != 0) 
	{
	  printf("sspInit: ERROR in vmeBusToLocalAdrs(0x09,0x%x,&laddr) \n",a32addr);
	  return(ERROR);
	}
#endif

    SSPpf[sspSL[ii]] = (unsigned int *)(laddr);
    sspA32Offset = laddr - a32addr;
    printf("sspInit: laddr(am=0x09) = 0x%08x, sspA32Offset=0x%08x\n",laddr, sspA32Offset);

	printf("SSP %2d  Slot # %2d at address 0x%08x (0x%08x) assigned A32 address 0x%08x (0x%08x)\n", 
			 ii, sspSL[ii],(UINT32) pSSP[(sspSL[ii])], 
		   (UINT32) pSSP[(sspSL[ii])]-sspA24Offset,
           (unsigned int)SSPpf[sspSL[ii]], (unsigned int)SSPpf[sspSL[ii]]-sspA32Offset);



    /***************************************************/
    /* following will be reconfigured in sspConfig() ) */

    /* the number of events per block */
    sspSetBlockLevel(sspSL[ii], block_level);

    /* Enable Bus Error */
    if(bus_error) sspEnableBusError(sspSL[ii]);
    else          sspDisableBusError(sspSL[ii]);

    /* window size and position */
	sspSetWindowWidth(sspSL[ii], window_width);
    sspSetWindowOffset(sspSL[ii], window_offset);
  }





  /*
   * If more than 1 SSP in crate then setup the Muliblock Address
   * window. This must be the same on each board in the crate
   */
  if (nSSP > 1)
  {
    /* set MB base above individual board base */
    a32addr = sspA32Base + (nSSP * SSP_MAX_FIFO);
#ifdef VXWORKS
    res = sysBusToLocalAdrs(0x09, (char *)a32addr, (char **)&laddr);
    if (res != 0)
    {
      printf("ERROR: %s: in sysBusToLocalAdrs(0x09,0x%x,&laddr) \n",__func__, a32addr);
      return(ERROR);
    }
#else
    res = vmeBusToLocalAdrs(0x09, (char *)a32addr, (char **)&laddr);
    if (res != 0)
    {
	  printf("ERROR: %s: in vmeBusToLocalAdrs(0x09,0x%x,&laddr) \n",__func__, a32addr);
	  return(ERROR);
    }
#endif
    SSPpmb = (unsigned int *)laddr;  /* Set a pointer to the FIFO */
	for (ii = 0; ii < nSSP; ii++)
    {
  	  /* Write the register and enable */
      vmeWrite32((volatile unsigned int *)&(pSSP[sspSL[ii]]->EB.Adr32M), \
                    ((a32addr + SSP_MAX_A32MB_SIZE) >> 7) | \
                    (a32addr >> 23) | (1 << 25));
	}
    vmeWrite32((volatile unsigned int *)&(pSSP[minSlot]->EB.Adr32M), \
                vmeRead32((volatile unsigned int *)&(pSSP[minSlot]->EB.Adr32M)) \
                            | (1 << 26));
    vmeWrite32((volatile unsigned int *)&(pSSP[maxSlot]->EB.Adr32M), \
                vmeRead32((volatile unsigned int *)&(pSSP[maxSlot]->EB.Adr32M)) \
                            | (1 << 27));
  }








  /* Setup initial configuration */ 
  if(noBoardInit==0) 
  { 
    for(issp=0; issp<nSSP; issp++) 
	{ 
	  result = sspSetMode(sspSlot(issp),iFlag,0);
	  if(result != OK)
	  {
	    return ERROR;
	  }

      /* soft reset (resets EB and fifo) */
      vmeWrite32(&pSSP[sspSL[issp]]->Cfg.Reset, 1);
      taskDelay(2);
      vmeWrite32(&pSSP[sspSL[issp]]->Cfg.Reset, 0);

	}	 
  } 


  printf("sspInit: found %d SSPs\n",nSSP);

  return(nSSP);
} 
 


void 
sspCheckAddresses(int id) 
{ 
  unsigned int offset=0, expected=0, base=0; 
  int iser=0; 
 
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return; 
    } 
   
  printf("%s:\n\t ---------- Checking SSP address space ---------- \n",__FUNCTION__); 
 
  base = (unsigned int) &pSSP[id]->Cfg; 
 
  offset = ((unsigned int) &pSSP[id]->Clk) - base; 
  expected = 0x100; 
  if(offset != expected) 
    printf("%s: ERROR pSSP[%d]->Clk not at offset = 0x%x (@ 0x%x)\n", 
	   __FUNCTION__,id,expected,offset); 
 
  offset = ((unsigned int) &pSSP[id]->Sd) - base; 
  expected = 0x200; 
  if(offset != expected) 
    printf("%s: ERROR pSSP[%d]->Sd not at offset = 0x%x (@ 0x%x)\n", 
	   __FUNCTION__,id,expected,offset); 
 
  offset = ((unsigned int) &pSSP[id]->Trigger) - base; 
  expected = 0x2100; 
  if(offset != expected) 
    printf("%s: ERROR pSSP[%d]->Trigger not at offset = 0x%x (@ 0x%x)\n", 
	   __FUNCTION__,id,expected,offset); 
 
  for(iser=0; iser<10; iser++) 
    { 
      offset = ((unsigned int) &pSSP[id]->Ser[iser]) - base; 
      expected = 0x1000 + iser*0x100; 
      if(offset != expected) 
	printf("%s: ERROR pSSP[%d]->Ser[%d] not at offset = 0x%x (@ 0x%x)\n", 
	       __FUNCTION__,id,iser,expected,offset); 
    } 
 
} 
 
/******************************************************************************* 
 * 
 * sspSlot - Convert an index into a slot number, where the index is 
 *           the element of an array of SSPs in the order in which they were 
 *           initialized. 
 * 
 * RETURNS: Slot number if Successfull, otherwise ERROR. 
 * 
 */ 
 

int
sspGetGeoAddress(int id)
{
  int slot;

  /*
  slot =  (sspReadReg(&ssp->Cfg.FirmwareRev)&SSP_CFG_SLOTID_MASK)>>24;
  */
  return(slot);
}


int 
sspSlot(unsigned int id) 
{ 
  if(id>=nSSP) 
    { 
      printf("%s: ERROR: Index (%d) >= SSPs initialized (%d).\n", 
	     __FUNCTION__,id,nSSP); 
      return ERROR; 
    } 
 
  return(sspSL[id]); 
} 

int 
sspId(unsigned int slot) 
{
  int id;

  for(id=0; id<nSSP; id++)
    if(sspSL[id] == slot)
      return(id);

  printf("%s: ERROR: SSP in slot %d does not exist or not initialized.\n",__FUNCTION__,slot);
  return(ERROR);
} 
 
/******************************************************************************* 
 * 
 * sspSetMode(int id, int iFlag) 
 *       id: SSP Slot number 
 *    iFlag: 
 *        bits 1:0 - Mode 
 *           0 - disabled 
 *               clk src = 0 
 *               sync src = 0 
 *               trig src = 0 
 *           1 - local/P2LVDS 
 *               clk src = LOCAL 
 *               sync src = P2LVDSIN0 
 *               trig src = P2LVDSIN1 
 *           2 - local/FPLVDS 
 *               clk src = LOCAL 
 *               sync src = FPLVDSIN0 
 *               trig src = FPLVDSIN1 
 *           3 - vxs 
 *               clk src = VXS SWB (SD) 
 *               sync src = VXS SWB (SD) 
 *               trig src = VXS SWB (SD) 
 * 
 *        bits 23:16 - Fiber Enable 
 *           see TRG_CTRL_FIBER_ENx definitions in ssp.h 
 *        bits 31:24 - GTP data source 
 *           see TRG_CTRL_GTPSRC_* definitions in ssp.h 
 * 
 *  RETURNS: OK if successful, otherwise ERROR. 
 * 
 */ 
 
int 
sspSetMode(int id, int iFlag, int pflag) 
{ 
  int result, clksrc, syncsrc, trigsrc; 
 
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return ERROR; 
    } 
 
  switch((iFlag>>0) & SSP_INIT_MODE_MASK) 
  { 
    case SSP_INIT_MODE_DISABLED: 
      clksrc = SSP_CLKSRC_DISABLED; 
      syncsrc = SD_SRC_SEL_0; 
      trigsrc = SD_SRC_SEL_0; 
      break; 
 
    case SSP_INIT_MODE_P2: 
      clksrc = SSP_CLKSRC_LOCAL; 
      syncsrc = SD_SRC_SEL_P2LVDSIN0; 
      trigsrc = SD_SRC_SEL_P2LVDSIN1;			 
      break; 
 
    case SSP_INIT_MODE_FP: 
      clksrc = SSP_CLKSRC_LOCAL; 
      syncsrc = SD_SRC_SEL_LVDSIN0; 
      trigsrc = SD_SRC_SEL_LVDSIN1;			 
      break; 
 
    case SSP_INIT_MODE_VXS: 
      clksrc = SSP_CLKSRC_SWB; 
      syncsrc = SD_SRC_SEL_SYNC; 
      trigsrc = SD_SRC_SEL_TRIG1; 
      break; 
  } 
	 
  if((iFlag & SSP_INIT_SKIP_SOURCE_SETUP)==0) 
  { 
      /* Setup Clock Source */ 
      result = sspSetClkSrc(id, clksrc); 
      if(result != OK) 
	return ERROR; 
       
      /* Setup Sync Source */ 
      result = sspSetIOSrc(id, SD_SRC_SYNC, syncsrc); 
      if(result != OK) 
	return ERROR; 
       
      /* Setup Trig Source */ 
      result = sspSetIOSrc(id, SD_SRC_TRIG, trigsrc); 
      if(result != OK) 
	return ERROR; 
  } 

		 
  sspPortEnable(id, (iFlag & SSP_INIT_FIBER_ENABLE_MASK)>>16, pflag); 

  return OK; 
} 
 
int 
sspStatus(int id, int rflag) 
{ 
  int showregs=0; 
  int i=0; 
  unsigned int fiberEnabledMask=0; 
  unsigned int SSPBase=0; 
  SSP_regs st; 
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 
 
  if(rflag & SSP_STATUS_SHOWREGS) 
    showregs=1; 
 
 
  SSPLOCK; 
  SSPBase             = (unsigned int)pSSP[id]; 
  st.Cfg.BoardId      = sspReadReg(&pSSP[id]->Cfg.BoardId); 
  st.Cfg.FirmwareRev  = sspReadReg(&pSSP[id]->Cfg.FirmwareRev); 
  st.Clk.Ctrl         = sspReadReg(&pSSP[id]->Clk.Ctrl); 
  st.Clk.Status       = sspReadReg(&pSSP[id]->Clk.Status); 
  for(i=0; i < SD_SRC_NUM; i++) 
    st.Sd.SrcSel[i]   = sspReadReg(&pSSP[id]->Sd.SrcSel[i]); 
  for(i=0; i < SSP_SER_NUM; i++) 
  { 
    st.Ser[i].Ctrl    = sspReadReg(&pSSP[id]->Ser[i].Ctrl); 
    st.Ser[i].Status = sspReadReg(&pSSP[id]->Ser[i].Status); 
    if((st.Ser[i].Ctrl & SSP_SER_CTRL_POWERDN)==0) 
      fiberEnabledMask |= (1<<i); 
  } 
  SSPUNLOCK; 
 
#ifdef VXWORKS 
  printf("\nSTATUS for SSP in slot %d at base address 0x%x \n", 
	 id, (UINT32) pSSP[id]); 
#else 
  printf("\nSTATUS for SSP in slot %d at VME (Local) base address 0x%x (0x%x)\n", 
	 id, (UINT32) pSSP[id] - sspA24Offset, (UINT32) pSSP[id]); 
#endif 
  printf("--------------------------------------------------------------------------------\n"); 
 
  if(showregs)
  { 
    printf("\n"); 
    printf(" Registers (offset):\n"); 
    printf("  Cfg.BoardID    (0x%04x) = 0x%08x\t", (unsigned int)(&pSSP[id]->Cfg.BoardId) - SSPBase, st.Cfg.BoardId); 
    printf("  Cfg.FirmwareRev(0x%04x) = 0x%08x\n", (unsigned int)(&pSSP[id]->Cfg.FirmwareRev) - SSPBase, st.Cfg.FirmwareRev); 
    printf("  Clk.Ctrl       (0x%04x) = 0x%08x\t", (unsigned int)(&pSSP[id]->Clk.Ctrl) - SSPBase, st.Clk.Ctrl); 
    printf("  Clk.Status     (0x%04x) = 0x%08x\n", (unsigned int)(&pSSP[id]->Clk.Status) - SSPBase, st.Clk.Status); 
 
    for(i=0; i < SD_SRC_NUM; i=i+2) 
	 { 
	   printf("  Sd.SrcSel[%2d]  (0x%04x) = 0x%08x\t", i, (unsigned int)(&pSSP[id]->Sd.SrcSel[i]) - SSPBase, st.Sd.SrcSel[i]); 
	   printf("  Sd.SrcSel[%2d]  (0x%04x) = 0x%08x\n", i+1, (unsigned int)(&pSSP[id]->Sd.SrcSel[i+1]) - SSPBase, st.Sd.SrcSel[i+1]); 
	 } 
    for(i=0; i < SSP_SER_NUM; i=i+2) 
	 { 
	   printf("  Ser[%2d].Ctrl   (0x%04x) = 0x%08x\t", i, (unsigned int)(&pSSP[id]->Ser[i].Ctrl) - SSPBase, st.Ser[i].Ctrl); 
	   printf("  Ser[%2d].Ctrl   (0x%04x) = 0x%08x\n", i+1, (unsigned int)(&pSSP[id]->Ser[i+1].Ctrl) - SSPBase, st.Ser[i+1].Ctrl); 
	 } 
  } 
  printf("\n"); 
 
  printf(" Board Firmware Rev/ID = 0x%04x\n", 
	 st.Cfg.FirmwareRev&0x0000FFFF); 
 
  printf("\n Signal Sources: \n"); 
  printf("   Ref Clock : %s - %s\n", 
	 ((st.Clk.Ctrl & CLK_CTRL_SERDES_MASK)>>24)<SSP_CLKSRC_NUM ? 
	 ssp_clksrc_name[(st.Clk.Ctrl & CLK_CTRL_SERDES_MASK)>>24] : 
	 "unknown", 
	 (st.Clk.Status & CLK_STATUS_GCLKLOCKED) ? 
	 "PLL Locked" : 
	 "*** PLL NOT Locked ***"); 
 
  printf("   Trig1     : %s\n", 
	 (st.Sd.SrcSel[SD_SRC_TRIG]<SD_SRC_SEL_NUM) ? 
	 ssp_signal_names[st.Sd.SrcSel[SD_SRC_TRIG]] : 
	 "unknown"); 
 
  printf("   SyncReset : %s\n", 
	 (st.Sd.SrcSel[SD_SRC_SYNC]<SD_SRC_SEL_NUM) ? 
	 ssp_signal_names[st.Sd.SrcSel[SD_SRC_SYNC]] : 
	 "unknown"); 
 
  printf("\n"); 
 
 
  if(fiberEnabledMask) 
  { 
    printf(" Fiber Ports Enabled (0x%x) =\n",fiberEnabledMask); 
    for(i=0; i <= SSP_SER_NUM; i++) 
    {
      if(i>SSP_SER_FIBER7) continue; /* skip VXS */
      if(fiberEnabledMask & (1<<i)) 
        printf("   %-10s: -%-12s-\n", 
		    ssp_serdes_names[i], 
          (st.Ser[i].Status & SSP_SER_STATUS_CHUP) ? "CHANNEL UP" : "CHANNEL DN"); 
	 } 
  } 
  else 
  { 
    printf(" No Fiber Ports Enabled\n"); 
  } 
 
  printf("\n"); 
  printf(" I/O Configuration: \n"); 
  sspPrintIOSrc(id,2); 
  printf("\n"); 
     
  printf("--------------------------------------------------------------------------------\n"); 
  printf("\n"); 
 
  return OK; 
} 
 
void 
sspGStatus(int rflag) 
{ 
  int showregs=0; 
  int issp=0, id=0, i=0, iport=0; 
  SSP_regs st[20]; 
  int portUsedInTrigger[20][SSP_SER_NUM]; 
 
  if(rflag & SSP_STATUS_SHOWREGS) 
    showregs=1; 
 
  SSPLOCK; 
  for(issp=0; issp<nSSP; issp++) 
  { 
    id = sspSlot(issp); 
    for(i=0; i < SSP_SER_NUM; i++) 
	 { 
	   st[id].Ser[i].Ctrl       = sspReadReg(&pSSP[id]->Ser[i].Ctrl); 
	   st[id].Ser[i].Status     = sspReadReg(&pSSP[id]->Ser[i].Status); 
	   st[id].Ser[i].MonStatus  = sspReadReg(&pSSP[id]->Ser[i].MonStatus); 
	   st[id].Ser[i].CrateId    = sspReadReg(&pSSP[id]->Ser[i].CrateId) & SER_CRATEID_MASK; 
	   st[id].Ser[i].ErrTile0   = sspReadReg(&pSSP[id]->Ser[i].ErrTile0); 
	   st[id].Ser[i].ErrTile1   = sspReadReg(&pSSP[id]->Ser[i].ErrTile1); 
	 } 
  } 
  SSPUNLOCK; 
 
  printf("\n"); 
 
  printf("                            SSP Port Status Summary\n\n"); 
  printf("                Channel  Used in    Rcv Trig    Trig          Lane bit errors\n"); 
  printf("Sl- P    ID     Status   Trigger      Data     Latency      0     1     2     3\n"); 
  printf("--------------------------------------------------------------------------------\n"); 
  for(issp=0; issp<nSSP; issp++) 
  { 
    id = sspSlot(issp); 
    for(iport=0; iport<8; iport++) 
	 { 
	   /* Slot and port number */ 
	   printf("%2d-%2d ",id, iport+1); 
 
	   /* Crate ID */ 
	   printf("%5d      ",st[id].Ser[iport].CrateId); 
 
	   /* Channel Status */ 
	   printf("%s      ", 
		 (st[id].Ser[iport].Status & SSP_SER_STATUS_CHUP) ?    
		 " UP ":"DOWN"); 
 
	  /* Receive trigger data */ 
	  printf("%s      ", 
		 (st[id].Ser[iport].Status & SSP_SER_STATUS_SRCRDYN) ?    
		 " UP ":"DOWN"); 
	   
	  /* Trigger latency */ 
	  printf("%5d   ", 
		 (st[id].Ser[iport].MonStatus)>>16); 
 
	  /* Lane bit errors */ 
	  printf("%5d ", 
		 st[id].Ser[iport].ErrTile0&0xFFFF); 
	  printf("%5d ", 
		 st[id].Ser[iport].ErrTile0>>16); 
	  printf("%5d ", 
		 st[id].Ser[iport].ErrTile1&0xFFFF); 
	  printf("%5d ", 
		 st[id].Ser[iport].ErrTile1>>16); 
	   printf("\n"); 
	 } 
  } 
  printf("--------------------------------------------------------------------------------\n"); 
  printf("\n"); 
  printf("\n"); 
 
} 
 
 
/************************************************************ 
 * SSP CLK Functions 
 ************************************************************/ 
 
/************************************************************ 
 * int sspSetClkSrc(int src) 
 *    src options: 
 *        SSP_CLKSRC_DISABLED 
 *        SSP_CLKSRC_SWB 
 *        SSP_CLKSRC_P2 
 *        SSP_CLKSRC_LOCAL 
*/ 
 
int  
sspSetClkSrc(int id, int src) 
{ 
  unsigned int clksrc; 
 
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return ERROR; 
    } 
	 
  if((src < SSP_CLKSRC_DISABLED) || (src > SSP_CLKSRC_LOCAL)) 
    { 
      printf("%s: ERROR: invalid clock source: %d [unknown]\n",  
	     __FUNCTION__,src); 
      return ERROR; 
    } 
	 
  clksrc = (src<<24) | (src<<26); 
 
  SSPLOCK; 
  sspWriteReg(&pSSP[id]->Clk.Ctrl, CLK_CTRL_GCLKRST | clksrc ); 
  taskDelay(1); 
  sspWriteReg(&pSSP[id]->Clk.Ctrl, clksrc); 
  taskDelay(1); 
  SSPUNLOCK; 
	 
  if(sspGetClkStatus(id) == ERROR) 
    { 
      printf("%s: ERROR: PLL not locked - no clock at source: %d [%s]\n", 
	     __FUNCTION__, 
	     src, ssp_clksrc_name[src]); 
      return ERROR; 
    }	 
	 
  printf("%s:  Clock source successfully set to: %d [%s]\n",  
	 __FUNCTION__, 
	 src, ssp_clksrc_name[src]); 
  return OK; 
} 
 
int  
sspGetClkStatus(int id) 
{ 
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return ERROR; 
    } 
 
  SSPLOCK; 
  if(!(sspReadReg(&pSSP[id]->Clk.Status) & CLK_STATUS_GCLKLOCKED)) 
    { 
      printf("%s: ERROR: PLL not locked\n", 
	     __FUNCTION__); 
      SSPUNLOCK; 
      return ERROR; 
    }	 
	 
  printf("%s: PLL locked\n", 
	 __FUNCTION__); 
  SSPUNLOCK; 
 
  return OK; 
} 
 
int 
sspGetClkSrc(int id, int pflag) 
{ 
  int rval=0; 
 
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return ERROR; 
    } 
 
  SSPLOCK; 
  rval = (sspReadReg(&pSSP[id]->Clk.Ctrl) & CLK_CTRL_SERDES_MASK)>>24; 
  SSPUNLOCK; 
	 
  if(pflag) 
    { 
      printf("%s: Clock Source = %d [%s]\n", 
	     __FUNCTION__, 
	     rval, 
	     (rval<SSP_CLKSRC_NUM) ? 
	     ssp_clksrc_name[rval] : 
	     "unknown"); 
    } 
 
  return rval; 
} 
 
/************************************************************ 
 * SSP SD.IO Functions 
 ************************************************************/ 
 
int  
sspSetIOSrc(int id, int ioport, int signal) 
{ 
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return ERROR; 
    } 
 
  if((ioport < 0) || (ioport >= SD_SRC_NUM)) 
    { 
      printf("%s: ERROR: invalid ioport (%d)\n",  
	     __FUNCTION__, 
	     ioport); 
      return ERROR; 
    } 
	 
  if((signal < 0) || (signal >= SD_SRC_SEL_NUM)) 
    { 
      printf("%s: ERROR: invalid signal source (%d)\n",  
	     __FUNCTION__, 
	     signal); 
      return ERROR; 
    } 
		 
  SSPLOCK; 
  sspWriteReg(&pSSP[id]->Sd.SrcSel[ioport], signal); 
  SSPUNLOCK; 
 
  return OK; 
} 
 
int 
sspGetIOSrc(int id, int ioport, int pflag) 
{ 
  int rval=0; 
 
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return ERROR; 
    } 
 
  if((ioport < 0) || (ioport >= SD_SRC_NUM)) 
    { 
      printf("%s: ERROR: invalid ioport (%d)\n",  
	     __FUNCTION__,ioport); 
      return ERROR; 
    } 
 
  SSPLOCK; 
  rval = sspReadReg(&pSSP[id]->Sd.SrcSel[ioport]) & SD_SRC_SEL_MASK; 
  SSPUNLOCK; 
 
  if(pflag) 
    { 
      if(rval < SD_SRC_SEL_NUM) 
	printf("%s:   %15s mapped to: %s\n",  
	       __FUNCTION__, 
	       ssp_ioport_names[ioport], ssp_signal_names[rval]); 
      else 
	printf("%s:   %15s mapped to: unknown\n",  
	       __FUNCTION__, 
	       ssp_ioport_names[ioport]); 
    } 
 
  return rval; 
} 
 
void  
sspPrintIOSrc(int id, int pflag) 
{ 
  int i; 
  unsigned int val; 
	 
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return;  
    } 
	 
  if(pflag!=2) 
    printf(" %s: \n",__FUNCTION__); 
  SSPLOCK; 
  for(i = 0; i < SD_SRC_NUM; i++) 
    { 
      val = sspReadReg(&pSSP[id]->Sd.SrcSel[i]) & SD_SRC_SEL_MASK; 
      if(val < SD_SRC_SEL_NUM) 
	printf("   %15s mapped to: %s\n",  
	       ssp_ioport_names[i], ssp_signal_names[val]); 
      else 
	printf("   %15s mapped to: unknown\n", 
	       ssp_ioport_names[i]); 
    } 
  SSPUNLOCK; 
 
} 
 
/************************************************************ 
 * SSP Trigger Functions 
 ************************************************************/ 
 
/************************************************************ 
 * SSP SD.PULSER Functions 
 ************************************************************/ 
 
int  
sspPulserStatus(int id) 
{ 
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return ERROR; 
    } 
 
  SSPLOCK; 
  if(sspReadReg(&pSSP[id]->Sd.PulserDone) & SD_PULSER_DONE) 
    { 
      SSPUNLOCK; 
      return 1;	// pulser has finished sending npulses 
    } 
 
  SSPUNLOCK; 
  return 0;		// pulser is active 
} 
 
void  
sspPulserStart(int id) 
{ 
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return; 
    } 
 
  SSPLOCK; 
  sspWriteReg(&pSSP[id]->Sd.PulserStart, 0); 
  SSPUNLOCK; 
} 
 
 
/************************************************************ 
 * int sspPulserSetup(float freq, float duty, unsigned npulses) 
 *    freq: 
 *        0.01 to 25E6 pulser frequency in Hz 
 *    duty: 
 *        0 to 1 pulser duty cycle 
 *    npulses: 
 *        0: pulser disabled 
 *        1 to 0xFFFFFFFE: pulser fires this number of times before being disabled.  
 *                         Must write to Sd.PulserStart to start pulser in this mode 
 *        0xFFFFFFFF: pulser fires forever 
 */ 
 
void  
sspPulserSetup(int id, float freq, float duty, unsigned int npulses) 
{ 
  unsigned int per, low; 
	 
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return; 
    } 
 
  if(freq < SD_PULSER_FREQ_MIN) 
    { 
      printf("%s: ERROR: Frequency input (%f) too low. Setting to minimum...\n",  
	     __FUNCTION__,freq); 
      freq = SD_PULSER_FREQ_MIN; 
    } 
	 
  if(freq > SD_PULSER_FREQ_MAX) 
    { 
      printf("%s: ERROR: Frequency input (%f) too high. Setting to maximum...\n",  
	     __FUNCTION__,freq); 
      freq = SD_PULSER_FREQ_MAX; 
    } 
	 
  if((duty < 0.0) || (duty > 1.0)) 
    { 
      printf("%s: ERROR: Invalid duty cycle %f. Setting to 0.5\n",  
	     __FUNCTION__,duty); 
      duty = 0.5; 
    } 
 
  SSPLOCK;	 
  /* Set to disable pulser output during setup */
  sspWriteReg(&pSSP[id]->Sd.PulserLowCycles, 0xFFFFFFFF); 
  sspWriteReg(&pSSP[id]->Sd.PulserPeriod, 0);
  sspWriteReg(&pSSP[id]->Sd.PulserNPulses, npulses);
  /* Hack to let npulses saturate in case burst mode is needed with no glitches */
  taskDelay(60);
  
  // Setup period register... 
  per = /*SYSCLK_FREQ*/GCLK_FREQ / freq; 
  if(!per) 
    per = 1; 
  sspWriteReg(&pSSP[id]->Sd.PulserPeriod, per);
	 
  // Setup duty cycle register...	 
  low = per * duty; 
  if(!low) 
    low = 1; 
  sspWriteReg(&pSSP[id]->Sd.PulserLowCycles, low); 
	 
	 
  printf("%s: Actual frequency = %f, duty = %f\n",  
	 __FUNCTION__, 
	 (float)/*SYSCLK_FREQ*/GCLK_FREQ/(float)per, (float)low/(float)per); 
  SSPUNLOCK; 

  sspPulserStart(id);
} 

float sspGetPulserFreq(int id)
{
  int val;
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return; 
  } 
  
  SSPLOCK;
  val = sspReadReg(&pSSP[id]->Sd.PulserPeriod);
  SSPUNLOCK;
  
  if(!val)
	  return 0.0f;
  
  return (float)GCLK_FREQ/(float)val;
}

/************************************************************ 
 * SSP SERDES Functions 
 ************************************************************/ 
 
/************************************************************ 
 * int sspPortEnable(int mask) 
 *    mask bits (set bit enables serdes): 
 *       SSP_SER_FIBER0				0 
 *       SSP_SER_FIBER1				1 
 *       SSP_SER_FIBER2				2 
 *       SSP_SER_FIBER3				3 
 *       SSP_SER_FIBER4				4 
 *       SSP_SER_FIBER5				5 
 *       SSP_SER_FIBER6				6 
 *       SSP_SER_FIBER7				7 
 *       SSP_SER_VXS0				8 
 *       SSP_SER_VXSGTP				9 
 */ 
 
void 
sspPortEnable(int id, int mask, int pflag) 
{ 
  int i, j; 
	 
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return; 
  } 
 
  if(pflag) 
    printf("%s - \n",__FUNCTION__); 
  SSPLOCK;
  for(i = 0; i < SSP_SER_NUM; i++) 
  { 
    if(mask & (1<<i)) 
    { 
      if(pflag) 
        printf("   Enabling channel: %s...\n", ssp_serdes_names[i]); 
 
      for(j = 0; j < 10; j++)
		{
        /* if the port already has its channel up, skip it */ 
        if((sspReadReg(&pSSP[id]->Ser[i].Status) & SSP_SER_STATUS_CHUP) == 0) 
        { 
	       sspWriteReg(&pSSP[id]->Ser[i].Ctrl, SSP_SER_CTRL_LINKRST | 
            SSP_SER_CTRL_GTXRST | 
            SSP_SER_CTRL_POWERDN); 
 
          sspWriteReg(&pSSP[id]->Ser[i].Ctrl, SSP_SER_CTRL_LINKRST | 
            SSP_SER_CTRL_GTXRST); 
		 
          sspWriteReg(&pSSP[id]->Ser[i].Ctrl, SSP_SER_CTRL_LINKRST); 
		 
          sspWriteReg(&pSSP[id]->Ser[i].Ctrl, 0); 
          sspWriteReg(&pSSP[id]->Ser[i].Ctrl, SSP_SER_CTRL_ERRCNT_EN);
		  
          sleep(1);
		  }
      }
      if((sspReadReg(&pSSP[id]->Ser[i].Status) & SSP_SER_STATUS_CHUP) == 0)
        printf("%s: ERROR: SSP in slot %d FIBER %d NOT UP\n", __FUNCTION__,id,i);
    } 
    else 
    { 
      if(pflag) 
        printf("   Disabling channel: %s...\n", ssp_serdes_names[i]); 
      sspWriteReg(&pSSP[id]->Ser[i].Ctrl, SSP_SER_CTRL_LINKRST | 
        SSP_SER_CTRL_GTXRST | 
        SSP_SER_CTRL_POWERDN); 
    } 
  } 
  SSPUNLOCK; 
  sspPortResetErrorCount(id, mask); 
  sspPortPrintStatus(id, mask); 
} 
 
void  
sspPortResetErrorCount(int id, int mask) 
{ 
  int i; 
  unsigned int val; 
	 
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return; 
    } 
 
  SSPLOCK; 
  for(i = 0; i < SSP_SER_NUM; i++) 
    { 
      if(mask & (1<<i)) 
	{ 
	  val = sspReadReg(&pSSP[id]->Ser[i].Ctrl); 
	  sspWriteReg(&pSSP[id]->Ser[i].Ctrl, val | SSP_SER_CTRL_ERRCNT_RST); 
	  sspWriteReg(&pSSP[id]->Ser[i].Ctrl, val & ~SSP_SER_CTRL_ERRCNT_RST); 
	} 
    } 
  SSPUNLOCK; 
} 
 
int  
sspPortGetErrorCount(int id, int port, int lane) 
{ 
  int val; 
  unsigned int result = 0xFFFF; 
 
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return ERROR; 
    } 
 
  if((port < 0) || (port >= SSP_SER_NUM)) 
    { 
      printf("%s: ERROR: Invalid port (%d)\n",  
	     __FUNCTION__, 
	     port); 
      return 0xFFFF; 
    } 
 
  SSPLOCK;	 
  val = sspReadReg(&pSSP[id]->Ser[port].Ctrl); 
  sspWriteReg(&pSSP[id]->Ser[port].Ctrl, val & ~SSP_SER_CTRL_ERRCNT_EN); 
	 
  if(lane == 0) 
    result = (sspReadReg(&pSSP[id]->Ser[port].ErrTile0)>>0) & 0xFFFF; 
  else if(lane == 1) 
    result = (sspReadReg(&pSSP[id]->Ser[port].ErrTile0)>>16) & 0xFFFF; 
  else if(lane == 2) 
    result = (sspReadReg(&pSSP[id]->Ser[port].ErrTile1)>>0) & 0xFFFF; 
  else if(lane == 3) 
    result = (sspReadReg(&pSSP[id]->Ser[port].ErrTile1)>>16) & 0xFFFF; 
		 
  sspWriteReg(&pSSP[id]->Ser[port].Ctrl, val | SSP_SER_CTRL_ERRCNT_EN); 
  SSPUNLOCK; 
 
  return result; 
} 
 
void  
sspPortPrintStatus(int id, int mask) 
{ 
  int i; 
  unsigned int ctrl, status; 
 
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return; 
    } 
 
 
  printf("%s - \n", 
	 __FUNCTION__); 
  if(mask) 
    { 
      for(i = 0; i < SSP_SER_NUM; i++) 
	{ 
	  if(mask & (1<<i)) 
	    { 
	      SSPLOCK;			 
	      ctrl = sspReadReg(&pSSP[id]->Ser[i].Ctrl); 
	      status = sspReadReg(&pSSP[id]->Ser[i].Status); 
	      SSPUNLOCK; 
	      printf("   Status(ctrl=0x%08X,status=0x%08X) for channel: %s\n",  
		     ctrl, status, ssp_serdes_names[i]); 
	      printf("    %-10s: %4u", "POWER_DOWN", (ctrl>>0) & 0x1); 
	      printf("    %-10s: %4u", "GT_RESET", (ctrl>>1) & 0x1); 
	      printf("    %-10s: %4u\n", "RESET", (ctrl>>9) & 0x1); 
	      printf("    %-10s: %4u", "HARD_ERR0", (status>>0) & 0x1); 
	      printf("    %-10s: %4u", "HARD_ERR1", (status>>1) & 0x1); 
	      printf("    %-10s: %4u", "HARD_ERR2", (status>>2) & 0x1); 
	      printf("    %-10s: %4u\n", "HARD_ERR3", (status>>3) & 0x1); 
	      printf("    %-10s: %4u", "LANE_UP0", (status>>4) & 0x1); 
	      printf("    %-10s: %4u", "LANE_UP1", (status>>5) & 0x1); 
	      printf("    %-10s: %4u", "LANE_UP2", (status>>6) & 0x1); 
	      printf("    %-10s: %4u\n", "LANE_UP3", (status>>7) & 0x1); 
	      printf("    %-10s: %4u", "CHANNEL_UP", (status>>12) & 0x1); 
	      printf("    %-10s: %4u", "TX_LOCK", (status>>13) & 0x1); 
	      printf("    %-10s: %4u\n", "RXSRCRDYN", (status>>14) & 0x1);			 
	      printf("    %-10s: %4u", "BITERRORS0", sspPortGetErrorCount(id, i, 0)); 
	      printf("    %-10s: %4u", "BITERRORS1", sspPortGetErrorCount(id, i, 1)); 
	      printf("    %-10s: %4u", "BITERRORS2", sspPortGetErrorCount(id, i, 2)); 
	      printf("    %-10s: %4u\n", "BITERRORS3", sspPortGetErrorCount(id, i, 3)); 
	      SSPLOCK;			 
	      printf("    %-10s: %4uns\n", "LATENCY", sspReadReg(&pSSP[id]->Ser[i].MonStatus)>>16); 
	      SSPUNLOCK;			 
	    } 
	} 
    } 
  else 
    { 
      printf("   No ports configured\n"); 
    } 
 
} 
 
int 
sspGetConnectedFiberMask(int id) 
{ 
  int rval=0; 
  int iport=0; 
 
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return ERROR; 
    } 
 
  SSPLOCK; 
  for(iport=SSP_SER_FIBER0; iport<=SSP_SER_FIBER7; iport++) 
    { 
      if(vmeRead32(&pSSP[id]->Ser[iport].Status) & SSP_SER_STATUS_CHUP) 
	rval |= (1<<iport); 
    } 
  SSPUNLOCK; 
 
  return rval; 
} 
 
int 
sspGetCrateID(int id, int port) 
{ 
  int crateid=0; 
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return ERROR; 
    } 
 
  if((port<SSP_SER_FIBER0) || (port>SSP_SER_FIBER7)) 
    { 
      printf("%s: ERROR: Invalid port (%d)\n", 
	     __FUNCTION__, 
	     port); 
      return ERROR; 
    } 
 
  SSPLOCK; 
  crateid = sspReadReg(&pSSP[id]->Ser[port].CrateId) & SER_CRATEID_MASK; 
  SSPUNLOCK; 
   
  return crateid; 
} 
 
void 
sspSerdesEnable(int id, int mask, int pflag) 
{ 
  sspPortEnable(id,mask,pflag); 
} 
 
void  
sspSerdesResetErrorCount(int id, int mask) 
{ 
  sspPortResetErrorCount(id,mask); 
} 
 
int  
sspSerdesGetErrorCount(int id, int ser, int lane) 
{ 
  return sspPortGetErrorCount(id,ser,lane); 
} 
 
void  
sspSerdesPrintStatus(int id, int mask) 
{ 
  sspPortPrintStatus(id,mask); 
} 
 
 
/************************************************************ 
 * SSP SD.SCALERS Functions 
 ************************************************************/ 
 
void  
sspPrintScalers(int id) 
{ 
  double ref, rate; 
  int i; 
  unsigned int scalers[SD_SCALER_NUM]; 
 
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return; 
    } 
 
  SSPLOCK; 
  sspWriteReg(&pSSP[id]->Sd.ScalerLatch, 0); 
	 
  for(i = 0; i < SD_SCALER_NUM; i++) 
    scalers[i] = sspReadReg(&pSSP[id]->Sd.Scalers[i]); 
	 
  SSPUNLOCK; 
 
  printf("%s - \n", 
	 __FUNCTION__); 
  if(!scalers[SD_SCALER_SYSCLK]) 
    { 
      printf("Error: sspPrintScalers() reference time is 0. Reported rates will not be normalized.\n"); 
      ref = 1.0; 
    } 
  else 
    { 
      ref = (double)scalers[SD_SCALER_SYSCLK] / (double)SYSCLK_FREQ; 
    } 
	 
  for(i = 0; i < SD_SCALER_NUM; i++) 
    { 
      rate = (double)scalers[i]; 
      rate = rate / ref; 
      if(scalers[i] == 0xFFFFFFFF) 
	printf("   %-25s %10u,%.3fHz [OVERFLOW]\n", ssp_scaler_name[i], scalers[i], rate); 
      else 
	printf("   %-25s %10u,%.3fHz\n", ssp_scaler_name[i], scalers[i], rate); 
    } 
} 
 
/************************************************************ 
 * SSP SSPCFG Firmware Functions 
 ************************************************************/ 
 
static void  
sspSelectSpi(int id, int sel) 
{ 
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return; 
    } 
 
  if(sel) 
    sspWriteReg(&pSSP[id]->Cfg.SpiCtrl, SSPCFG_SPI_NCSCLR); 
  else 
    sspWriteReg(&pSSP[id]->Cfg.SpiCtrl, SSPCFG_SPI_NCSSET); 
} 
 
static unsigned char  
sspTransferSpi(int id, unsigned char data) 
{ 
  int i; 
  unsigned int val; 
	 
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return ERROR; 
    } 
 
  sspWriteReg(&pSSP[id]->Cfg.SpiCtrl, data | SSPCFG_SPI_START); 
 
  for(i = 0; i < 1000; i++) 
    { 
      val = sspReadReg(&pSSP[id]->Cfg.SpiStatus); 
      if(val & SSPCFG_SPI_DONE) 
	break; 
    } 
  if(i == 1000) 
    printf("%s: ERROR: Timeout!!!\n", 
	   __FUNCTION__); 
	 
  return val & 0xFF; 
} 
 
static void  
sspFlashGetId(int id, unsigned char *rsp) 
{ 
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return; 
    } 
 
  sspSelectSpi(id,1); 
  sspTransferSpi(id,SPI_CMD_GETID); 
  rsp[0] = sspTransferSpi(id,0xFF); 
  rsp[1] = sspTransferSpi(id,0xFF); 
  rsp[2] = sspTransferSpi(id,0xFF); 
  sspSelectSpi(id,0); 
} 
 
static unsigned char  
sspFlashGetStatus(int id) 
{ 
  unsigned char rsp; 
	 
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return ERROR; 
    } 
 
  sspSelectSpi(id,1); 
  sspTransferSpi(id,SPI_CMD_GETSTATUS); 
  rsp = sspTransferSpi(id,0xFF); 
  sspSelectSpi(id,0); 
	 
  return rsp; 
} 
 
static void  
sspReloadFirmware(int id) 
{ 
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return; 
    } 
 
  printf("%s: ERROR: Not implemented yet. Issue power cycle or VME SYSRESET to reload firmware.\n", 
	 __FUNCTION__); 
  /* 
    int i; 
    unsigned short reloadSequence[] = { 
    0xFFFF, 0xAA99, 0x5566, 0x3261, 
    0x0000, 0x3281, 0x0B00, 0x32A1, 
    0x0000, 0x32C1, 0x0B00, 0x30A1, 
    0x000E, 0x2000 
    }; 
 
    VSCM_WriteReg((unsigned int)&pVSCM_BASE->ICap, 0x40000 | 0x00000); 
    VSCM_WriteReg((unsigned int)&pVSCM_BASE->ICap, 0x40000 | 0x20000); 
    for(i = 0; i < sizeof(reloadSequence)/sizeof(reloadSequence[0]); i++) 
    { 
    VSCM_WriteReg((unsigned int)&pVSCM_BASE->ICap, 0x00000 | reloadSequence[i]); 
    VSCM_WriteReg((unsigned int)&pVSCM_BASE->ICap, 0x20000 | reloadSequence[i]); 
    } 
    for(i = 0; i < 10; i++) 
    { 
    VSCM_WriteReg((unsigned int)&pVSCM_BASE->ICap, 0x40000 | 0x00000); 
    VSCM_WriteReg((unsigned int)&pVSCM_BASE->ICap, 0x40000 | 0x20000); 
    } 
    taskDelay(120);
  */ 
} 
 
 
int  
sspFirmwareUpdateVerify(int id, const char *filename) 
{ 
  int result; 
	 
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return ERROR; 
    } 
 
  printf("Updating firmware..."); 
  result = sspFirmwareUpdate(id, filename); 
  if(result != OK) 
    { 
      printf("failed.\n"); 
      return result; 
    } 
  else 
    printf("succeeded."); 
	 
  printf("\nVerifying..."); 
  result = sspFirmwareVerify(id, filename); 
  if(result != OK) 
    { 
      printf("failed.\n"); 
      return result; 
    } 
  else 
    printf("ok.\n"); 
 
  sspReloadFirmware(id); 
		 
  return OK; 
} 
 
int  
sspFirmwareUpdate(int id, const char *filename) 
{ 
  FILE *f; 
  int i; 
  unsigned int page = 0; 
  unsigned char buf[1056], rspId[3]; 
 
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return ERROR; 
    } 
 
  SSPLOCK; 
  sspSelectSpi(id,0); 
  sspFlashGetId(id, rspId); 
	 
  printf("Flash: Mfg=0x%02X, Type=0x%02X, Capacity=0x%02X\n", rspId[0], rspId[1], rspId[2]); 
 
  if( (rspId[0] == SPI_MFG_ATMEL) && 
      (rspId[1] == (SPI_DEVID_AT45DB642D>>8)) && 
      (rspId[2] == (SPI_DEVID_AT45DB642D&0xFF)) ) 
    { 
      f = fopen(filename, "rb"); 
      if(!f) 
	{ 
	  printf("%s: ERROR: invalid file %s\n", __FUNCTION__, filename); 
	  return ERROR; 
	  SSPUNLOCK; 
	} 
	 
      memset(buf, 0xff, 1056); 
      while(fread(buf, 1, 1056, f) > 0) 
	{ 
	  sspSelectSpi(id,1);	// write buffer 1 
	  sspTransferSpi(id,SPI_CMD_WRBUF1); 
	  sspTransferSpi(id,0x00); 
	  sspTransferSpi(id,0x00); 
	  sspTransferSpi(id,0x00); 
	  for(i = 0; i < 1056; i++) 
	    sspTransferSpi(id,buf[i]); 
	  sspSelectSpi(id,0); 
 
	  sspSelectSpi(id,1);	// buffer 1 to flash w/page erase 
	  sspTransferSpi(id,SPI_CMD_PGBUF1ERASE); 
	  sspTransferSpi(id,(page>>5) & 0xFF); 
	  sspTransferSpi(id,(page<<3) & 0xFF); 
	  sspTransferSpi(id,0x00); 
	  sspSelectSpi(id,0); 
			 
	  i = 0; 
	  while(1) 
	    { 
	      if(sspFlashGetStatus(id) & 0x80) 
		break; 
	      if(i == 40000)	// 40ms maximum page program time 
		{ 
		  fclose(f); 
		  printf("%s: ERROR: failed to program flash\n", __FUNCTION__); 
		  SSPUNLOCK; 
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
      printf("%s: ERROR: failed to identify flash id 0x%02X 0x%02X 0x%02X\n",  
	     __FUNCTION__, (int)rspId[0], (int)rspId[1], (int)rspId[2]); 
      SSPUNLOCK; 
      return ERROR; 
    } 
 
  SSPUNLOCK; 
  return OK; 
} 
 
int  
sspFirmwareRead(int id, const char *filename) 
{ 
  FILE *f; 
  int i; 
  unsigned char rspId[3]; 
 
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return ERROR; 
    } 
 
  SSPLOCK; 
  sspSelectSpi(id,0); 
  sspFlashGetId(id, rspId); 
	 
  printf("Flash: Mfg=0x%02X, Type=0x%02X, Capacity=0x%02X\n", rspId[0], rspId[1], rspId[2]); 
 
  if( (rspId[0] == SPI_MFG_ATMEL) && 
      (rspId[1] == (SPI_DEVID_AT45DB642D>>8)) && 
      (rspId[2] == (SPI_DEVID_AT45DB642D&0xFF)) ) 
    { 
      f = fopen(filename, "wb"); 
      if(!f) 
	{ 
	  printf("%s: ERROR: invalid file %s\n", __FUNCTION__, filename); 
	  SSPUNLOCK; 
	  return ERROR; 
	} 
		 
      sspSelectSpi(id,1); 
      sspTransferSpi(id,SPI_CMD_RD);	// continuous array read 
      sspTransferSpi(id,0); 
      sspTransferSpi(id,0); 
      sspTransferSpi(id,0); 
		 
      for(i = 0; i < SPI_BYTE_LENGTH; i++) 
	{ 
	  fputc(sspTransferSpi(id,0xFF), f); 
	  if(!(i% 65536)) 
	    { 
	      printf("."); 
	      taskDelay(1); 
	    } 
	} 
			 
      sspSelectSpi(id,0); 
      fclose(f); 
    } 
  else 
    { 
      printf("%s: ERROR: failed to identify flash id 0x%02X 0x%02X 0x%02X\n",  
	     __FUNCTION__, (int)rspId[0], (int)rspId[1], (int)rspId[2]); 
      SSPUNLOCK; 
      return ERROR; 
    } 
 
  SSPUNLOCK; 
  return OK; 
} 
 
int  
sspFirmwareVerify(int id, const char *filename) 
{ 
  FILE *f; 
  int i,len; 
  unsigned int addr = 0; 
  unsigned char buf[256]; 
  unsigned char rspId[3], val; 
	 
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return ERROR; 
    } 
 
  SSPLOCK; 
  sspSelectSpi(id,0); 
  sspFlashGetId(id, rspId); 
	 
  printf("Flash: Mfg=0x%02X, Type=0x%02X, Capacity=0x%02X\n", rspId[0], rspId[1], rspId[2]); 
 
  if( (rspId[0] == SPI_MFG_ATMEL) && 
      (rspId[1] == (SPI_DEVID_AT45DB642D>>8)) && 
      (rspId[2] == (SPI_DEVID_AT45DB642D&0xFF)) ) 
    { 
      f = fopen(filename, "rb"); 
      if(!f) 
	{ 
	  printf("%s: ERROR: invalid file %s\n", __FUNCTION__, filename); 
	  SSPUNLOCK; 
	  return ERROR; 
	} 
		 
      sspSelectSpi(id,1); 
      sspTransferSpi(id,SPI_CMD_RD);	// continuous array read 
      sspTransferSpi(id,0); 
      sspTransferSpi(id,0); 
      sspTransferSpi(id,0); 
 
      while((len = fread(buf, 1, 256, f)) > 0) 
	{ 
	  for(i = 0; i < len; i++) 
	    { 
	      val = sspTransferSpi(id,0xFF); 
	      if(buf[i] != val) 
		{ 
		  sspSelectSpi(id,0); 
		  fclose(f);					 
		  printf("%s: ERROR: failed verify at addess 0x%08X[%02X,%02X]\n",  
			 __FUNCTION__, addr+i, buf[i], val); 
		  SSPUNLOCK; 
		  return ERROR; 
		} 
	    } 
	  addr+=256; 
	  if(!(addr & 0xFFFF)) 
	    printf(".");					 
	} 
      sspSelectSpi(id,0); 
      fclose(f); 
    } 
  else 
    { 
      printf("%s: ERROR: failed to identify flash id 0x%02X 0x%02X 0x%02X\n",  
	     __FUNCTION__, (int)rspId[0], (int)rspId[1], (int)rspId[2]); 
      SSPUNLOCK; 
      return ERROR; 
    } 
 
  SSPUNLOCK; 
  return OK; 
} 
 
int  
sspGetSerialNumber(int id, char *mfg, int *sn) 
{ 
  int i; 
 
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return ERROR; 
    } 
 
  SSPLOCK; 
  // need to parse this and extract MFG string and Serial int	 
  sspSelectSpi(id,0); 
  sspSelectSpi(id,1); 
  sspTransferSpi(id,SPI_CMD_RD); 
  sspTransferSpi(id,0xFF); 
  sspTransferSpi(id,0xF8); 
  sspTransferSpi(id,0x00); 
	 
  for(i = 0; i < 256; i++) 
    { 
      if(!(i & 0xF)) 
	printf("\n0x%04X: ", i); 
      printf("%02X ", sspTransferSpi(id,0xFF)); 
    } 
	 
  sspSelectSpi(id,0); 
  SSPUNLOCK; 
		 
  return OK; 
} 
 
unsigned int 
sspGetFirmwareVersion(int id) 
{ 
  unsigned int rval=0; 
 
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return ERROR; 
    } 
 
  SSPLOCK; 
  rval = sspReadReg(&pSSP[id]->Cfg.FirmwareRev) & SSP_CFG_FIRMWAREREV_MASK; 
  SSPUNLOCK; 
 
  return rval; 
} 

/* HPS Routines */

/***************************************
           set parameters
***************************************/

/* sspHps_SetLatency() - set trigger latency in 4ns ticks */ 
int sspHps_SetLatency(int id, int latency)
{
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return ERROR; 
    } 

  if( (latency < 0) || (latency > 1023) )
  {
    printf("%s: ERROR: latency is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }
 
  SSPLOCK; 
  sspWriteReg(&pSSP[id]->Trigger.Latency, latency);
  SSPUNLOCK; 
 
  return OK; 
}

/* sspHps_SetSinglesEmin() - set calorimeter single cluster trigger parameters
     n    - singles trigger 0 or 1
     emin - minimum cluster energy (units: MeV)
*/
int sspHps_SetSinglesEmin(int id, int n, int emin)
{
  if(id==0) id=sspSL[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return ERROR; 
    } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  if( (emin < 0) || (emin > 8191) )
  {
    printf("%s: ERROR: emin is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  sspWriteReg(&pSSP[id]->HpsSingles[n].ClusterEmin, emin);
  SSPUNLOCK; 
 
  return OK; 
}

/* sspHps_SetSinglesEmax() - set calorimeter single cluster trigger parameters
     n    - singles trigger 0 or 1
     emax - minimum cluster energy (units: MeV)
*/
int sspHps_SetSinglesEmax(int id, int n, int emax)
{
  if(id==0) id=sspSL[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return ERROR; 
    } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  if( (emax < 0) || (emax > 8191) )
  {
    printf("%s: ERROR: emax is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  sspWriteReg(&pSSP[id]->HpsSingles[n].ClusterEmax, emax);
  SSPUNLOCK; 
 
  return OK; 
}

/* sspHps_SetSinglesNHitsmin() - set calorimeter single cluster trigger parameters
     n    - singles trigger 0 or 1
     nmin - minimum cluster hits
*/
int sspHps_SetSinglesNHitsmin(int id, int n, int nmin)
{
  if(id==0) id=sspSL[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return ERROR; 
    } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  if( (nmin < 0) || (nmin > 15) )
  {
    printf("%s: ERROR: nmin is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  sspWriteReg(&pSSP[id]->HpsSingles[n].ClusterNHitsmin, nmin);
  SSPUNLOCK; 
 
  return OK; 
}

/* sspHps_SetSinglesEnableEmin() - set calorimeter single cluster trigger parameters
     n    - singles trigger 0 or 1
     en   - '0' disables , '1' enables trigger
*/
int sspHps_SetSinglesEnableEmin(int id, int n, int en)
{
  int val;
  
  if(id==0) id=sspSL[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
   } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  val = sspReadReg(&pSSP[id]->HpsSingles[n].Ctrl);
  if(en)
    val |= 0x00000001;
  else
    val &= 0xFFFFFFFE;
  sspWriteReg(&pSSP[id]->HpsSingles[n].Ctrl, val);
  SSPUNLOCK; 
 
  return OK; 
}

/* sspHps_SetSinglesEnableEmax() - set calorimeter single cluster trigger parameters
     n    - singles trigger 0 or 1
     en   - '0' disables , '1' enables trigger
*/
int sspHps_SetSinglesEnableEmax(int id, int n, int en)
{
  int val;
  
  if(id==0) id=sspSL[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  val = sspReadReg(&pSSP[id]->HpsSingles[n].Ctrl);
  if(en)
    val |= 0x00000002;
  else
    val &= 0xFFFFFFFD;
  sspWriteReg(&pSSP[id]->HpsSingles[n].Ctrl, val);
  SSPUNLOCK; 
 
  return OK; 
}

/* sspHps_SetSinglesEnableNmin() - set calorimeter single cluster trigger parameters
     n    - singles trigger 0 or 1
     en   - '0' disables , '1' enables trigger
*/
int sspHps_SetSinglesEnableNmin(int id, int n, int en)
{
  int val;
  
  if(id==0) id=sspSL[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  val = sspReadReg(&pSSP[id]->HpsSingles[n].Ctrl);
  if(en)
    val |= 0x00000004;
  else
    val &= 0xFFFFFFFB;
  sspWriteReg(&pSSP[id]->HpsSingles[n].Ctrl, val);
  SSPUNLOCK; 
 
  return OK; 
}

/* sspHps_SetSinglePrescale() - set singles regional prescaler
     n        - singles trigger 0 or 1
     region   - region number: 0 to 6
     xmin     - region definition minimum cluster X: -31 to 31
     xmax     - region definition maximum cluster X: -31 to 31
     prescale - region prescale value: 0 to 65535
                0 - no prescaling (accepts everything)
                1 - skip 1, accept 1
                2 - skip 2, accept 1
                ...
     Note: All region decisions are OR'd together.
           All regions (xmin,xmax) should be defined mutually exclusive
*/
int sspHps_SetSinglePrescale(int id, int n, int region, int xmin, int xmax, int prescale)
{
  if(id==0) id=sspSL[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  }

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  if( (region < 0) || (region > 6) )
  {
    printf("%s: ERROR: region is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  if( (xmin < 31) || (xmin > 31) )
  {
    printf("%s: ERROR: xmin is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  if( (xmax < 31) || (xmax > 31) )
  {
    printf("%s: ERROR: xmax is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  if( (prescale < 0) || (prescale > 65535) )
  {
    printf("%s: ERROR: xmax is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }


  SSPLOCK;
  vmeWrite32(&pSSP[id]->HpsSingles[n].Prescale[region],
				 ((prescale & 0xFFFF)<<0) | ((xmin & 0x3F)<<16) | ((xmax & 0x3F)<<24)
    );
  SSPUNLOCK; 
 
  return OK; 
}

/* sspHps_SetCosmicTimeCoincidence() - set cosmic scintillator coincidence time
     ticks - coincidence time (units: +/-4ns)
*/
int sspHps_SetCosmicTimeCoincidence(int id, int ticks)
{
  int ctrl = 0;
  
  if(id==0) id=sspSL[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 

  if( (ticks < 0) || (ticks > 255) )
  {
    printf("%s: ERROR: ticks is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  sspWriteReg(&pSSP[id]->HpsCosmic.TimeCoincidence, ticks);
  SSPUNLOCK; 
 
  return OK; 
}

/* sspHps_SetCosmicCoincidencePattern() - set cosmic scintillator coincidence pattern
     pattern  - 3:1 LUT definition for scintillator coincidence pattern that is accepted/rejected
              Scintillator channels are the last 3 channels (14-16) of FADC in slot 20
              pattern = 0xFE will trigger on any hit channels
              pattern = 0x80 will trigger when all 3 channels hit
              pattern = 0x88 will trigger when channels 14&15 are hit
              pattern = 0xE8 will trigger when any 2 channels hit
*/
int sspHps_SetCosmicCoincidencePattern(int id, int pattern)
{
  int ctrl = 0;
  
  if(id==0) id=sspSL[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return ERROR; 
    } 

  if( (pattern < 0) || (pattern > 65535) )
  {
    printf("%s: ERROR: pattern is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  sspWriteReg(&pSSP[id]->HpsCosmic.TriggerPattern, pattern);
  SSPUNLOCK; 
 
  return OK; 
}

int sspHps_SetPairsEnableSum(int id, int n, int en)
{
  int val;

  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  if(en)
    val = sspReadReg(&pSSP[id]->HpsPairs[n].Ctrl) | 0x00000001;
  else
    val = sspReadReg(&pSSP[id]->HpsPairs[n].Ctrl) & 0xFFFFFFFE;
  sspWriteReg(&pSSP[id]->HpsPairs[n].Ctrl, val); 
  SSPUNLOCK; 
 
  return OK;
}

int sspHps_SetPairsEnableDiff(int id, int n, int en)
{
  int val;

  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  if(en)
    val = sspReadReg(&pSSP[id]->HpsPairs[n].Ctrl) | 0x00000002;
  else
    val = sspReadReg(&pSSP[id]->HpsPairs[n].Ctrl) & 0xFFFFFFFD;
  sspWriteReg(&pSSP[id]->HpsPairs[n].Ctrl, val); 
  SSPUNLOCK; 
 
  return OK;
}

int sspHps_SetPairsEnableCoplanar(int id, int n, int en)
{
  int val;

  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  if(en)
    val = sspReadReg(&pSSP[id]->HpsPairs[n].Ctrl) | 0x00000004;
  else
    val = sspReadReg(&pSSP[id]->HpsPairs[n].Ctrl) & 0xFFFFFFFB;
  sspWriteReg(&pSSP[id]->HpsPairs[n].Ctrl, val); 
  SSPUNLOCK; 
 
  return OK;
}

int sspHps_SetPairsEnableED(int id, int n, int en)
{
  int val;

  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  if(en)
    val = sspReadReg(&pSSP[id]->HpsPairs[n].Ctrl) | 0x00000008;
  else
    val = sspReadReg(&pSSP[id]->HpsPairs[n].Ctrl) & 0xFFFFFFF7;
  sspWriteReg(&pSSP[id]->HpsPairs[n].Ctrl, val); 
  SSPUNLOCK; 
 
  return OK;
}

/* sspHps_PairsTimeCoincidence() - set cluster pair coincidence time window (units: +/-4ns) */
int sspHps_SetPairsTimeCoincidence(int id, int n, int ticks)
{
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  if( (ticks < 0) || (ticks > 15) )
  {
    printf("%s: ERROR: ticks is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  sspWriteReg(&pSSP[id]->HpsPairs[n].ClusterTimeCoincidence, ticks); 
  SSPUNLOCK; 
 
  return OK; 
}

/* sspHps_PairsSummax() - set cluster pair sum maximum (units: MeV) */
int sspHps_SetPairsSummax(int id, int n, int max)
{
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  if( (max < 0) || (max > 8191) )
  {
    printf("%s: ERROR: ticks is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  sspWriteReg(&pSSP[id]->HpsPairs[n].ClusterSummax, max); 
  SSPUNLOCK; 
 
  return OK; 
}

/* sspHps_PairsSummin() - set cluster pair sum minimum (units: MeV) */
int sspHps_SetPairsSummin(int id, int n, int min)
{
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  if( (min < 0) || (min > 8191) )
  {
    printf("%s: ERROR: ticks is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  sspWriteReg(&pSSP[id]->HpsPairs[n].ClusterSummin, min); 
  SSPUNLOCK; 
 
  return OK; 
}

/* sspHps_PairsDiffmax() - set cluster pair difference maximum (units: MeV) */
int sspHps_SetPairsDiffmax(int id, int n, int max)
{
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  if( (max < 0) || (max > 8191) )
  {
    printf("%s: ERROR: ticks is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  sspWriteReg(&pSSP[id]->HpsPairs[n].ClusterDiffmax, max); 
  SSPUNLOCK; 
 
  return OK;
}


/* sspHps_PairsEmin() - set cluster pair cluster energy minimum (units: MeV) */
int sspHps_SetPairsEmin(int id, int n, int min)
{
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  if( (min < 0) || (min > 8191) )
  {
    printf("%s: ERROR: emin is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  sspWriteReg(&pSSP[id]->HpsPairs[n].ClusterEmin, min); 
  SSPUNLOCK; 
 
  return OK;
}

/* sspHps_PairsEmin() - set cluster pair cluster energy maximum (units: MeV) */
int sspHps_SetPairsEmax(int id, int n, int max)
{
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  if( (max < 0) || (max > 8191) )
  {
    printf("%s: ERROR: emax is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  sspWriteReg(&pSSP[id]->HpsPairs[n].ClusterEmax, max); 
  SSPUNLOCK; 
 
  return OK;
}

/* sspHps_PairsNHitsmin() - set minimum cluster hits for calorimeter pair cluster trigger (units: counts) */
int sspHps_SetPairsNHitsmin(int id, int n, int min)
{
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  {
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  }

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  if( (min < 0) || (min > 9) )
  {
    printf("%s: ERROR: emin is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  sspWriteReg(&pSSP[id]->HpsPairs[n].ClusterNHitsmin, min); 
  SSPUNLOCK; 
 
  return OK; 
}

/* sspHps_PairsCoplanarTolerance() - set cluster pair-beam coplanarity tolerance (units: degrees) */
int sspHps_SetPairsCoplanarTolerance(int id, int n, int tol)
{
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  if( (tol < 0) || (tol > 255) )
  {
    printf("%s: ERROR: tol is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  sspWriteReg(&pSSP[id]->HpsPairs[n].ClusterCoplanarTol, tol); 
  SSPUNLOCK; 
 
  return OK;
}

/* sspHps_PairsEDFactor() - set cluster pair radial distance->energy factor (units: MeV/mm) */
int sspHps_SetPairsEDFactor(int id, int n, float f)
{
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  if( (f < 0.0) || (f > 511.0) )
  {
    printf("%s: ERROR: f is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  /* convert to fixed point 8.4 format */
  f = f * 16.0f;

  SSPLOCK;
  sspWriteReg(&pSSP[id]->HpsPairs[n].ClusterEDFactor, (int)f); 
  SSPUNLOCK; 
 
  return OK;
}

int sspHps_SetPairsEDmin(int id, int n, int min)
{
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  {
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  if( (min < 0) || (min > 8191) )
  {
    printf("%s: ERROR: min is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  sspWriteReg(&pSSP[id]->HpsPairs[n].ClusterEDmin, min); 
  SSPUNLOCK; 
 
  return OK;
}

/***************************************
           get parameters
***************************************/

int sspHps_GetLatency(int id)
{
  int val;
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return ERROR; 
    } 

  SSPLOCK; 
  val = sspReadReg(&pSSP[id]->Trigger.Latency);
  SSPUNLOCK; 
 
  return val;
}

int sspHps_GetSinglesEmin(int id, int n)
{
  int val;
  if(id==0) id=sspSL[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return ERROR; 
    } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  val = sspReadReg(&pSSP[id]->HpsSingles[n].ClusterEmin);
  SSPUNLOCK; 
 
  return val;
}

int sspHps_GetSinglesEmax(int id, int n)
{
  int val;
  if(id==0) id=sspSL[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return ERROR; 
    } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }
  
  SSPLOCK;
  val = sspReadReg(&pSSP[id]->HpsSingles[n].ClusterEmax);
  SSPUNLOCK; 
 
  return val;
}

int sspHps_GetSinglesNHitsmin(int id, int n)
{
  int val;
  if(id==0) id=sspSL[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return ERROR; 
    } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  val = sspReadReg(&pSSP[id]->HpsSingles[n].ClusterNHitsmin);
  SSPUNLOCK; 
 
  return val;
}

int sspHps_GetSinglesEnableEmin(int id, int n)
{
  int val;
  if(id==0) id=sspSL[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
   } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  val = (sspReadReg(&pSSP[id]->HpsSingles[n].Ctrl) & 0x00000001) >> 0;
  SSPUNLOCK; 
 
  return val;
}

int sspHps_GetSinglesEnableEmax(int id, int n)
{
  int val;  
  if(id==0) id=sspSL[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  val = (sspReadReg(&pSSP[id]->HpsSingles[n].Ctrl) & 0x00000002) >>1;
  SSPUNLOCK; 
 
  return val;
}

int sspHps_GetSinglesEnableNmin(int id, int n)
{
  int val;  
  if(id==0) id=sspSL[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  val = (sspReadReg(&pSSP[id]->HpsSingles[n].Ctrl) & 0x00000004) >> 2;
  SSPUNLOCK; 
 
  return val;
}

int sspHps_GetSinglePrescaleXmin(int id, int n, int region)
{
  int val;
  if(id==0) id=sspSL[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  }
  
  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  if( (region < 0) || (region > 6) )
  {
    printf("%s: ERROR: region is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  val = vmeRead32(&pSSP[id]->HpsSingles[n].Prescale[region]);
  SSPUNLOCK;
  
  val = (val>>16) & 0x3F;
  if(val & 0x20) val |= 0xFFFFFFC;
  
  return val;
}

int sspHps_GetSinglePrescaleXmax(int id, int n, int region)
{
  int val;
  if(id==0) id=sspSL[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  }

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  if( (region < 0) || (region > 6) )
  {
    printf("%s: ERROR: region is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  val = vmeRead32(&pSSP[id]->HpsSingles[n].Prescale[region]);
  SSPUNLOCK;

  val = (val>>24) & 0x3F;
  if(val & 0x20) val |= 0xFFFFFFC;
  
  return val;
}

int sspHps_GetSinglePrescalePrescale(int id, int n, int region)
{
  int val;
  if(id==0) id=sspSL[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  }

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  if( (region < 0) || (region > 6) )
  {
    printf("%s: ERROR: region is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  val = vmeRead32(&pSSP[id]->HpsSingles[n].Prescale[region]);
  SSPUNLOCK;
  
  return val & 0xFFFF;
}

int sspHps_GetCosmicTimeCoincidence(int id)
{
  int val;
  if(id==0) id=sspSL[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 

  SSPLOCK;
  val = sspReadReg(&pSSP[id]->HpsCosmic.TimeCoincidence);
  SSPUNLOCK; 
 
  return val;
}

int sspHps_GetCosmicCoincidencePattern(int id)
{
  int val;
  if(id==0) id=sspSL[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
    { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return ERROR; 
    } 

  SSPLOCK;
  val = sspReadReg(&pSSP[id]->HpsCosmic.TriggerPattern);
  SSPUNLOCK; 
 
  return val; 
}

int sspHps_GetPairsEnableSum(int id, int n)
{
  int val;
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  val = (sspReadReg(&pSSP[id]->HpsPairs[n].Ctrl) & 0x00000001) >> 0;
  SSPUNLOCK; 
 
  return val;
}

int sspHps_GetPairsEnableDiff(int id, int n)
{
  int val;
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  val = (sspReadReg(&pSSP[id]->HpsPairs[n].Ctrl) & 0x00000002) >> 1;
  SSPUNLOCK; 
 
  return val;
}

int sspHps_GetPairsEnableCoplanar(int id, int n)
{
  int val;
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  val = (sspReadReg(&pSSP[id]->HpsPairs[n].Ctrl) & 0x00000004) >> 2;
  SSPUNLOCK; 
 
  return val;
}

int sspHps_GetPairsEnableED(int id, int n)
{
  int val;
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  val = (sspReadReg(&pSSP[id]->HpsPairs[n].Ctrl) & 0x00000008) >> 3;
  SSPUNLOCK; 
 
  return val;
}

int sspHps_GetPairsTimeCoincidence(int id, int n)
{
  int val;
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  val = sspReadReg(&pSSP[id]->HpsPairs[n].ClusterTimeCoincidence); 
  SSPUNLOCK; 
 
  return val; 
}

int sspHps_GetPairsSummax(int id, int n)
{
  int val;
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  val = sspReadReg(&pSSP[id]->HpsPairs[n].ClusterSummax);
  SSPUNLOCK; 
 
  return val; 
}

int sspHps_GetPairsSummin(int id, int n)
{
  int val;
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  val = sspReadReg(&pSSP[id]->HpsPairs[n].ClusterSummin);
  SSPUNLOCK; 
 
  return val; 
}

int sspHps_GetPairsDiffmax(int id, int n)
{
  int val;
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  val = sspReadReg(&pSSP[id]->HpsPairs[n].ClusterDiffmax);
  SSPUNLOCK; 
 
  return val;
}

int sspHps_GetPairsEmin(int id, int n)
{
  int val;
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  val = sspReadReg(&pSSP[id]->HpsPairs[n].ClusterEmin);
  SSPUNLOCK; 
 
  return val;
}

int sspHps_GetPairsEmax(int id, int n)
{
  int val;
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  val = sspReadReg(&pSSP[id]->HpsPairs[n].ClusterEmax); 
  SSPUNLOCK; 
 
  return val;
}

int sspHps_GetPairsNHitsmin(int id, int n)
{
  int val;
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  {
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  }

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  val = sspReadReg(&pSSP[id]->HpsPairs[n].ClusterNHitsmin);
  SSPUNLOCK; 
 
  return val; 
}

int sspHps_GetPairsCoplanarTolerance(int id, int n)
{
  int val;
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  val = sspReadReg(&pSSP[id]->HpsPairs[n].ClusterCoplanarTol); 
  SSPUNLOCK; 
 
  return val;
}

float sspHps_GetPairsEDFactor(int id, int n)
{
  float val;
  
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  val = (float)sspReadReg(&pSSP[id]->HpsPairs[n].ClusterEDFactor) / 16.0f;
  SSPUNLOCK; 
 
  return val;
}

int sspHps_GetPairsEDmin(int id, int n)
{
  int val;
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  {
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return ERROR; 
  } 

  if( (n < 0) || (n > 1) )
  {
    printf("%s: ERROR: n is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  val = sspReadReg(&pSSP[id]->HpsPairs[n].ClusterEDmin); 
  SSPUNLOCK; 
 
  return val;
}

void sspPrintHpsScalers(int id) 
{
  double ref, rate; 
  int i; 
  unsigned int scalers[SD_SCALER_NUM];
  unsigned int hpsscalers[24];
  const char *hpsscalers_name[24] = {
    "PairTrig0.PairPass",
    "PairTrig0.SumPass",
    "PairTrig0.DiffPass",
    "PairTrig0.EDPass",
    "PairTrig0.CoplanarPass",
    "PairTrig0.TriggerPass",
    "PairTrig1.PairPass",
    "PairTrig1.SumPass",
    "PairTrig1.DiffPass",
    "PairTrig1.EDPass",
    "PairTrig1.CoplanarPass",
    "PairTrig1.TriggerPass",
    "SingleTrig0.SinglesPass",
    "SingleTrig0.SinglesTot",
    "SingleTrig1.SinglesPass",
    "SingleTrig1.SinglesTot",
    "Cosmic.Top0",
    "Cosmic.Top1",
    "Cosmic.Top2",
    "Cosmic.TopTrig",
    "Cosmic.Bot0",
    "Cosmic.Bot1",
    "Cosmic.Bot2",
    "Cosmic.BotTrig"
    };
 
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  {
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return; 
  }
 
  SSPLOCK; 
  sspWriteReg(&pSSP[id]->Sd.ScalerLatch, 0); 
 
  for(i = 0; i < SD_SCALER_NUM; i++) 
    scalers[i] = sspReadReg(&pSSP[id]->Sd.Scalers[i]); 

  hpsscalers[0] = sspReadReg(&pSSP[id]->HpsPairs[0].ScalerPairsPass);
  hpsscalers[1] = sspReadReg(&pSSP[id]->HpsPairs[0].ScalerSumPass);
  hpsscalers[2] = sspReadReg(&pSSP[id]->HpsPairs[0].ScalerDiffPass);
  hpsscalers[3] = sspReadReg(&pSSP[id]->HpsPairs[0].ScalerEDPass);
  hpsscalers[4] = sspReadReg(&pSSP[id]->HpsPairs[0].ScalerCoplanarPass);
  hpsscalers[5] = sspReadReg(&pSSP[id]->HpsPairs[0].ScalerTriggerPass);
  hpsscalers[6] = sspReadReg(&pSSP[id]->HpsPairs[1].ScalerPairsPass);
  hpsscalers[7] = sspReadReg(&pSSP[id]->HpsPairs[1].ScalerSumPass);
  hpsscalers[8] = sspReadReg(&pSSP[id]->HpsPairs[1].ScalerDiffPass);
  hpsscalers[9] = sspReadReg(&pSSP[id]->HpsPairs[1].ScalerEDPass);
  hpsscalers[10] = sspReadReg(&pSSP[id]->HpsPairs[1].ScalerCoplanarPass);
  hpsscalers[11] = sspReadReg(&pSSP[id]->HpsPairs[1].ScalerTriggerPass);
  hpsscalers[12] = sspReadReg(&pSSP[id]->HpsSingles[0].ScalerSinglesPass);
  hpsscalers[13] = sspReadReg(&pSSP[id]->HpsSingles[0].ScalerSinglesTot);
  hpsscalers[14] = sspReadReg(&pSSP[id]->HpsSingles[1].ScalerSinglesPass);
  hpsscalers[15] = sspReadReg(&pSSP[id]->HpsSingles[1].ScalerSinglesTot);
  hpsscalers[16] = sspReadReg(&pSSP[id]->HpsCosmic.ScalerScintillatorTop[0]);
  hpsscalers[17] = sspReadReg(&pSSP[id]->HpsCosmic.ScalerScintillatorTop[1]);
  hpsscalers[18] = sspReadReg(&pSSP[id]->HpsCosmic.ScalerScintillatorTop[2]);
  hpsscalers[19] = sspReadReg(&pSSP[id]->HpsCosmic.ScalerCosmicTop);
  hpsscalers[20] = sspReadReg(&pSSP[id]->HpsCosmic.ScalerScintillatorBot[0]);
  hpsscalers[21] = sspReadReg(&pSSP[id]->HpsCosmic.ScalerScintillatorBot[1]);
  hpsscalers[22] = sspReadReg(&pSSP[id]->HpsCosmic.ScalerScintillatorBot[2]);
  hpsscalers[23] = sspReadReg(&pSSP[id]->HpsCosmic.ScalerCosmicBot);

  SSPUNLOCK; 
 
  printf("%s - \n", __FUNCTION__); 
  if(!scalers[SD_SCALER_SYSCLK]) 
  {
    printf("Error: sspPrintScalers() reference time is 0. Reported rates will not be normalized.\n"); 
    ref = 1.0; 
  } 
  else 
  { 
    ref = (double)scalers[SD_SCALER_SYSCLK] / (double)SYSCLK_FREQ; 
  } 
	 
  for(i = 0; i < SD_SCALER_NUM; i++) 
  { 
    rate = (double)scalers[i]; 
    rate = rate / ref; 
    if(scalers[i] == 0xFFFFFFFF) 
	   printf("   %-25s %10u,%.3fHz [OVERFLOW]\n", ssp_scaler_name[i], scalers[i], rate); 
    else 
	   printf("   %-25s %10u,%.3fHz\n", ssp_scaler_name[i], scalers[i], rate); 
  }

  for(i = 0; i < 24; i++) 
  { 
    rate = (double)hpsscalers[i]; 
    rate = rate / ref; 
    if(hpsscalers[i] == 0xFFFFFFFF) 
	   printf("   %-25s %10u,%.3fHz [OVERFLOW]\n", hpsscalers[i], hpsscalers[i], rate); 
    else 
	   printf("   %-25s %10u,%.3fHz\n", hpsscalers[i], hpsscalers[i], rate); 
  }
}

void sspPrintHpsConfig(int id)
{
  int i, j;
  int triggerLatency, singlesEmin[2], singlesEmax[2], singlesNmin[2],
      singlesEminEn[2], singlesEmaxEn[2], singlesNminEn[2],
      singlesPrescaleXmin[2][7], singlesPrescaleXmax[2][7], singlesPrescalePrescale[2][7], 
      cosmicTimeCoincidence, cosmicPatternCoincidence,
      pairsSumEn[2], pairsDiffEn[2], pairsCoplanarEn[2], pairsEDEn[2],
      pairsTimeCoincidence[2], pairsSummax[2],
      pairsSummin[2], pairsDiffmax[2], pairsEmin[2], pairsEmax[2],
      pairsNHitsmin[2], pairsCoplanarTolerance[2], pairsEDmin[2];
  float pairsEDFactor[2];
		
  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
    printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
    return; 
  } 

  printf("*** HpsConfig ***\n");
  
  triggerLatency = sspHps_GetLatency(id);
  
  for(i = 0; i < 2; i++)
  {
    singlesEmin[i] = sspHps_GetSinglesEmin(id, i);
    singlesEmax[i] = sspHps_GetSinglesEmax(id, i);
    singlesNmin[i] = sspHps_GetSinglesNHitsmin(id, i);
    singlesEminEn[i] = sspHps_GetSinglesEnableEmin(id, i);
    singlesEmaxEn[i] = sspHps_GetSinglesEnableEmax(id, i);
    singlesNminEn[i] = sspHps_GetSinglesEnableNmin(id, i);
  
    for(j = 0; j < 7; j++)
    {
      singlesPrescaleXmin[i][j] = sspHps_GetSinglePrescaleXmin(id, i, j);
      singlesPrescaleXmax[i][j] = sspHps_GetSinglePrescaleXmax(id, i, j);
      singlesPrescalePrescale[i][j] = sspHps_GetSinglePrescalePrescale(id, i, j);
    }
  }

  cosmicTimeCoincidence = sspHps_GetCosmicTimeCoincidence(id);
  cosmicPatternCoincidence = sspHps_GetCosmicCoincidencePattern(id);
  
  for(i = 0; i < 2; i++)
  {
  pairsSumEn[i] = sspHps_GetPairsEnableSum(id, i);
  pairsDiffEn[i] = sspHps_GetPairsEnableDiff(id, i);
  pairsCoplanarEn[i] = sspHps_GetPairsEnableCoplanar(id, i);
  pairsEDEn[i] = sspHps_GetPairsEnableED(id, i);
  pairsTimeCoincidence[i] = sspHps_GetPairsTimeCoincidence(id, i);
  pairsSummax[i] = sspHps_GetPairsSummax(id, i);
  pairsSummin[i] = sspHps_GetPairsSummin(id, i);
  pairsDiffmax[i] = sspHps_GetPairsDiffmax(id, i);
  pairsEmin[i] = sspHps_GetPairsEmin(id, i);
  pairsEmax[i] = sspHps_GetPairsEmax(id, i);
  pairsNHitsmin[i] = sspHps_GetPairsNHitsmin(id, i);
  pairsCoplanarTolerance[i] = sspHps_GetPairsCoplanarTolerance(id, i);
  pairsEDFactor[i] = sspHps_GetPairsEDFactor(id, i);
  pairsEDmin[i] = sspHps_GetPairsEDmin(id, i);
  }

  for(i = 0; i < 2; i++)
  {
  printf("   *** Singles %d Configuration ***\n", i);
  printf("     Emin = %dMeV, Enabled = %d\n", singlesEmin[i], singlesEminEn[i]);
  printf("     Emax = %dMeV, Enabled = %d\n", singlesEmax[i], singlesEmaxEn[i]);
  printf("     NHitsmin = %d, Enabled = %d\n", singlesNmin[i], singlesNminEn[i]);
  for(j = 0; j < 7; j++)
  printf("     Prescale region %d: xmin = %d, xmax = %d, prescale = %d\n", j,
			singlesPrescaleXmin[i][j], singlesPrescaleXmax[i][j], singlesPrescalePrescale[i][j]);  
  printf("\n");
  }
  
  for(i = 0; i < 2; i++)
  {
  printf("   *** Pairs %d Configuration ***\n", i);
  printf("     Emin = %dMeV\n", pairsEmin[i]);
  printf("     Emax = %dMeV\n", pairsEmax[i]);
  printf("     NHitsmin = %d\n", pairsNHitsmin[i]);
  printf("     TimeCoincidence = %d(+/-%dns)\n", pairsTimeCoincidence[i], pairsTimeCoincidence[i]*4);
  printf("     SumMax = %dMeV, SumMin = %dMeV, Enabled = %d\n", pairsSummax[i], pairsSummin[i], pairsSumEn[i]);
  printf("     DiffMax = %dMeV, Enabled = %d\n", pairsDiffmax[i], pairsDiffEn[i]);
  printf("     Coplanarity = +/-%ddegrees, Enabled = %d\n", pairsCoplanarTolerance[i], pairsCoplanarEn[i]);
  printf("     EDFactor = %.3fMeV/mm, EDmin = %dMeV, Enabled = %d\n", pairsEDFactor[i], pairsEDmin[i], pairsEDEn[i]);
  printf("\n");
  }

  printf("   *** Cosmic Configuration ***\n");
  printf("     TimeCoincidence = %d(%dns)\n", cosmicTimeCoincidence, cosmicTimeCoincidence*4);
  printf("     PatternCoincidence = 0x%02X\n", cosmicPatternCoincidence);
  printf("\n");
  
  printf("   *** Trigger Configuration ***\n");
  printf("     Latency = %d(%dns)\n", triggerLatency, triggerLatency*4);
  
  printf("   *** Trigger Output Configuration ***\n");
  for(i = SD_SRC_P2_LVDSOUT0; i <= SD_SRC_P2_LVDSOUT7; i++)
  {
  printf("     %s = %s\n", ssp_ioport_names[i], ssp_signal_names[sspGetIOSrc(id, i, 0)]);
  }
  
  printf("\n");
}


/* Global arrays of strings of names of ports/signals */ 
 
const char *ssp_ioport_names[SD_SRC_NUM] =  
  { 
    "LVDSOUT0", 
    "LVDSOUT1", 
    "LVDSOUT2", 
    "LVDSOUT3", 
    "LVDSOUT4", 
    "GPIO0", 
    "GPIO1", 
    "P2_LVDSOUT0 (TS#1)", 
    "P2_LVDSOUT1 (TS#2)", 
    "P2_LVDSOUT2 (TS#3)", 
    "P2_LVDSOUT3 (TS#4)", 
    "P2_LVDSOUT4 (TS#5)", 
    "P2_LVDSOUT5 (TS#6)", 
    "P2_LVDSOUT6", 
    "P2_LVDSOUT7", 
    "TRIG", 
    "SYNC" 
  }; 
	 
const char *ssp_signal_names[SD_SRC_SEL_NUM] =  
  { 
    "SD_SRC_SEL_0", 
    "SD_SRC_SEL_1", 
    "SD_SRC_SEL_SYNC", 
    "SD_SRC_SEL_TRIG1", 
    "SD_SRC_SEL_TRIG2", 
    "SD_SRC_SEL_LVDSIN0", 
    "SD_SRC_SEL_LVDSIN1", 
    "SD_SRC_SEL_LVDSIN2", 
    "SD_SRC_SEL_LVDSIN3", 
    "SD_SRC_SEL_LVDSIN4", 
    "SD_SRC_SEL_P2LVDSIN0", 
    "SD_SRC_SEL_P2LVDSIN1", 
    "SD_SRC_SEL_P2LVDSIN2", 
    "SD_SRC_SEL_P2LVDSIN3", 
    "SD_SRC_SEL_P2LVDSIN4", 
    "SD_SRC_SEL_P2LVDSIN5", 
    "SD_SRC_SEL_P2LVDSIN6", 
    "SD_SRC_SEL_P2LVDSIN7", 
    "SD_SRC_SEL_PULSER", 
    "SD_SRC_SEL_BUSY", 
    "SD_SRC_SEL_TRIGGER0 (HPS SINGLES 0)", 
    "SD_SRC_SEL_TRIGGER1 (HPS SINGLES 1)", 
    "SD_SRC_SEL_TRIGGER2 (HPS PAIRS 0)", 
    "SD_SRC_SEL_TRIGGER3 (HPS PAIRS 1)", 
    "SD_SRC_SEL_TRIGGER4 (HPS LED)", 
    "SD_SRC_SEL_TRIGGER5 (HPS COSMIC)", 
    "SD_SRC_SEL_TRIGGER6", 
    "SD_SRC_SEL_TRIGGER7" 
  }; 
 
const char *ssp_gtpsrc_names[TRG_CTRL_GTPSRC_NUM] =  
  { 
    "TRG_CTRL_GTPSRC_FIBER0", 
    "TRG_CTRL_GTPSRC_FIBER1", 
    "TRG_CTRL_GTPSRC_FIBER2", 
    "TRG_CTRL_GTPSRC_FIBER3", 
    "TRG_CTRL_GTPSRC_FIBER4", 
    "TRG_CTRL_GTPSRC_FIBER5", 
    "TRG_CTRL_GTPSRC_FIBER6", 
    "TRG_CTRL_GTPSRC_FIBER7", 
    "TRG_CTRL_GTPSRC_SUM" 
  }; 
 
const char *ssp_scaler_name[SD_SCALER_NUM] =  
  { 
    "SD_SCALER_SYSCLK", 
    "SD_SCALER_GCLK", 
    "SD_SCALER_SYNC", 
    "SD_SCALER_TRIG1", 
    "SD_SCALER_TRIG2", 
    "SD_SCALER_GPIO0", 
    "SD_SCALER_GPIO1", 
    "SD_SCALER_LVDSIN0", 
    "SD_SCALER_LVDSIN1", 
    "SD_SCALER_LVDSIN2", 
    "SD_SCALER_LVDSIN3", 
    "SD_SCALER_LVDSIN4", 
    "SD_SCALER_LVDSOUT0", 
    "SD_SCALER_LVDSOUT1", 
    "SD_SCALER_LVDSOUT2", 
    "SD_SCALER_LVDSOUT3", 
    "SD_SCALER_LVDSOUT4", 
    "SD_SCALER_BUSY", 
    "SD_SCALER_BUSYCYCLES", 
    "SD_SCALER_P2_LVDSIN0", 
    "SD_SCALER_P2_LVDSIN1", 
    "SD_SCALER_P2_LVDSIN2", 
    "SD_SCALER_P2_LVDSIN3", 
    "SD_SCALER_P2_LVDSIN4", 
    "SD_SCALER_P2_LVDSIN5", 
    "SD_SCALER_P2_LVDSIN6", 
    "SD_SCALER_P2_LVDSIN7", 
    "SD_SCALER_P2_LVDSOUT0", 
    "SD_SCALER_P2_LVDSOUT1", 
    "SD_SCALER_P2_LVDSOUT2", 
    "SD_SCALER_P2_LVDSOUT3", 
    "SD_SCALER_P2_LVDSOUT4", 
    "SD_SCALER_P2_LVDSOUT5", 
    "SD_SCALER_P2_LVDSOUT6", 
    "SD_SCALER_P2_LVDSOUT7" 
  }; 
 
const char *ssp_clksrc_name[SSP_CLKSRC_NUM] =  
  { 
    "DISABLED",  
    "SWB",  
    "P2",  
    "LOCAL" 
  }; 
 
const char *ssp_serdes_names[SSP_SER_NUM] =  
  { 
    "SSP_SER_FIBER0", 
    "SSP_SER_FIBER1",
	/* 
    "SSP_SER_FIBER2", 
    "SSP_SER_FIBER3", 
    "SSP_SER_FIBER4", 
    "SSP_SER_FIBER5", 
    "SSP_SER_FIBER6", 
    "SSP_SER_FIBER7", 
    "SSP_SER_VXS0", 
    "SSP_SER_VXSGTP" 
	*/
  }; 



/*******************************************************************************
 *
 * sspReadBlock - General Data readout routine
 *
 *    id    - SSP to read from
 *    data  - local memory address to place data
 *    nwrds - Max number of words to transfer
 *    rflag - Readout Flag
 *              0 - programmed I/O from the specified board
 *              1 - DMA transfer using Universe/Tempe DMA Engine 
 *                    (DMA VME transfer Mode must be setup prior)
 *              2 - Multiblock DMA transfer (Multiblock must be enabled
 *                     and daisychain in place or SD being used)
 *
 * RETURNS: Number of words transferred to data if successful, ERROR otherwise
 *
 */

int
sspReadBlock(int id, unsigned int *data, int nwrds, int rflag)
{
  int retVal;
  volatile unsigned int *laddr;
  unsigned int val;
  unsigned int vmeAdr;

  if(id==0) id=sspSL[0]; 
  if((id<=0) || (id>21) || (pSSP[id]==NULL)) 
  { 
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id); 
      return -1; 
  } 

  if (SSPpf[id] == NULL) {
  logMsg("ERROR: %s: SSP A32 not initialized\n", __func__);
    return -1;
  }

  if (data == NULL)
  {
    logMsg("ERROR: %s: Invalid Destination address\n", __func__);
    return -1;
  }

  SSPLOCK;

  /* Block transfer */
  if (rflag >= 1)
  {
    /* Assume that the DMA programming is already setup. 
    Don't Bother checking if there is valid data - that should be done prior
    to calling the read routine */

    laddr = data;
	/*
	printf("DMA1: 0x%08x 0x%08x 0x%08x\n",
		   vmeRead32(&pSSP[id]->EB.FifoBlockCnt),
		   vmeRead32(&pSSP[id]->EB.FifoWordCnt),
		   vmeRead32(&pSSP[id]->EB.FifoEventCnt));
	*/
    vmeAdr = (unsigned int)(SSPpf[id]) - sspA32Offset;

	/*
    printf("sspReadBlock: vmeAdr=0x%08x (0x%08x - 0x%08x)\n",vmeAdr,(unsigned int)(SSPpf[id]),sspA32Offset);fflush(stdout);
	*/

    retVal = usrVme2MemDmaStart(vmeAdr, (unsigned int *)laddr, (nwrds << 2));

    if (retVal |= 0)
    {
      logMsg("ERROR: %s: DMA transfer Init @ 0x%x Failed\n", __func__, retVal);
      SSPUNLOCK;
      return retVal;
    }

    retVal = usrVme2MemDmaDone();

    if (retVal > 0) {
      int xferCount = (retVal >> 2);

      SSPUNLOCK;
      return xferCount;
    }
    else if (retVal == 0) {
      logMsg("WARN: %s: DMA transfer terminated by word count 0x%x\n", \
              __func__, nwrds);
      SSPUNLOCK;
      return 0;
    }
    /* Error in DMA */
    else {
      logMsg("ERROR: %s: DmaDone returned an Error\n", __func__);
      SSPUNLOCK;
      return 0;
    }
  }
  /* Programmed IO */
  else
  {
    int dCnt = 0;
    int ii = 0;

    while (ii < nwrds)
    {
      val = *SSPpf[id];
#ifndef VXWORKS
      val = LSWAP(val);
#endif
/*
      if (val == TI_EMPTY_FIFO)
        break;
#ifndef VXWORKS
      val = LSWAP(val);
#endif
*/
      data[ii] = val;
      ii++;
    }
    ii++;
    dCnt += ii;

    SSPUNLOCK;
    return dCnt;
  }

  SSPUNLOCK;
  return(0);
}






int
sspBReady(int slot)
{
  unsigned int rval;

  SSPLOCK;
  rval = vmeRead32(&pSSP[slot]->EB.FifoBlockCnt);
  SSPUNLOCK;

  return (rval > 0) ? 1 : 0;
}

unsigned int
sspGBReady()
{
  unsigned int mask = 0;
  int i, stat;

  /*SSPLOCK;*/
  for (i = 0; i < nSSP; i++) {
    stat = sspBReady(sspSL[i]);
    if (stat)
      mask |= (1 << sspSL[i]);
  }
  /*SSPUNLOCK;*/

  return(mask);
}



























/*****************************************************************************/

void
sspInitGlobals()
{
  int ii, jj;

  block_level = 1;
  bus_error = 1;
  window_width  = 100;
  window_offset = 825;

  trigger_latency = 500;

  for(ii = 0; ii < 2; ii++)
  {
    singles_emin[ii] = 0;
    singles_emax[ii] = 8191;
    singles_nmin[ii] = 0;
    singles_emin_en[ii] = 1;
    singles_emax_en[ii] = 1;
    singles_nmin_en[ii] = 1;

	 // Enable Region 0 to cover whole ECAL with no prescaling
    singles_prescalers[ii][0].xmin = -31;
	 singles_prescalers[ii][0].xmax = 31;
	 singles_prescalers[ii][0].prescale = 0;
	 
	 // Disable Regions 1-6
	 for(jj = 1; jj < 7; jj++)
	 {
      singles_prescalers[ii][jj].xmin = -31;
	   singles_prescalers[ii][jj].xmax = -31;
	   singles_prescalers[ii][jj].prescale = 0;
	 }
  }

  cosmic_timecoincidence = 10;
  cosmic_pattern = 0xFE;

  pulser_freq = 0;

  for(ii = 0; ii < 2; ii++)
  {
    pairs_timecoincidence[ii] = 2;
    pairs_emin[ii] = 0;
    pairs_emax[ii] = 8191;
    pairs_nmin[ii] = 0;
    pairs_summax[ii] = 8191;
    pairs_summin[ii] = 0;
    pairs_summax_en[ii] = 1;
    pairs_diffmax[ii] = 8191;
    pairs_diffmax_en[ii] = 1;
    pairs_coplanartolerance[ii] = 180;
    pairs_coplanartolerance_en[ii] = 1;
    pairs_edfactor[ii] = 1;
    pairs_edmin[ii] = 0;
    pairs_ed_en[ii] = 1;
  }

  for(ii = 0; ii < SD_SRC_NUM; ii++)
    ssp_io_mux[ii] = SD_SRC_SEL_0;

  ssp_io_mux[SD_SRC_TRIG] = SD_SRC_SEL_TRIG1;
  ssp_io_mux[SD_SRC_SYNC] = SD_SRC_SEL_SYNC;

  /*
  for(ii=0; ii<NBOARD; ii++)
  {
    sspconf[ii].group = 0;
    for(jj=0; jj<8; jj++) sspconf[ii].mask[jj] = 0xffff;
  }
  */

  /* obtain CPU DMA setting information */
  usrVmeDmaGetConfig(&a24_a32, &sngl_blt_mblt, &sst_rate);

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
sspReadConfigFile(char *filename)
{
  FILE   *fd;
  char   fname[FNLEN] = { "" };  /* config file name */
  int    ii, jj, ch, gr = 0;
  char   str_tmp[STRLEN], keyword[ROCLEN];
  char   host[ROCLEN], ROC_name[ROCLEN];
  char   str2[2];
  int    args, i1, i2, i3, i4, i5, msk[NCHAN];
  float  f1;
  int    slot, chan;
  unsigned int  ui1, ui2;
  char *getenv();
  char *clonparms;
  char *expid;

  gethostname(host,ROCLEN);  /* obtain our hostname */
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
      sprintf(fname, "%s/ssp/%s", clonparms, filename);
	}

    if((fd=fopen(fname,"r")) == NULL)
    {
      printf("\nsspReadConfigFile: Can't open config file >%s<\n",fname);
      return(-1);
    }
  }
  else /* filename does not specified */
  {
    sprintf(fname, "%s/ssp/%s.cnf", clonparms, host);
    if((fd=fopen(fname,"r")) == NULL)
    {
      sprintf(fname, "%s/ssp/%s.cnf", clonparms, expid);
      if((fd=fopen(fname,"r")) == NULL)
      {
        printf("\nsspReadConfigFile: Can't open config file >%s<\n",fname);
        return(-2);
	  }
	}

  }
  printf("\nsspReadConfigFile: Using configuration file >%s<\n",fname);

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
      if(strcmp(keyword,"SSP_CRATE") == 0)
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

      else if(active && (strcmp(keyword,"SSP_ALLSLOTS")==0))
      {
	    gr++;
	    for(ii=0; ii<NBOARD; ii++)  sspconf[ii].group = gr;
      }

      else if(active && ((strcmp(keyword,"SSP_SLOT")==0) || (strcmp(keyword,"SSP_SLOTS")==0)) )
      {
	    gr++;
	    SCAN_MSK;
	    printf("\nsspReadConfigFile: gr = %d     args = %d \n",gr,args);

	    for(ii=0; ii<args; ii++)
	    {
	      slot = msk[ii];
	      if(slot<1 || slot>21)
	      {
	        printf("\nsspReadConfigFile: Wrong slot number %d, %s\n",slot,str_tmp);
	        return(-4);
	      }
	      sspconf[slot].group = gr;
	    }
      }

      else if(active && (strcmp(keyword,"SSP_BERR")==0))
      {
        sscanf (str_tmp, "%*s %d %d", &i1);
	    if(i1!=0 && i1!=1)
	    {
	      printf("\nsspReadConfigFile: Wrong DMA bus error %d, %s\n",i1,str_tmp);
	      return(-8);
	    }
	    bus_error = i1;
      }

      else if(active && (strcmp(keyword,"SSP_BLOCK_LEVEL")==0))
      {
        sscanf (str_tmp, "%*s %d %d", &i1);
	    block_level= i1;
      }

      else if(active && (strcmp(keyword,"SSP_W_WIDTH")==0))
      {
        sscanf (str_tmp, "%*s %d %d", &i1);
	    window_width= i1;
      }

      else if(active && (strcmp(keyword,"SSP_W_OFFSET")==0))
      {
        sscanf (str_tmp, "%*s %d %d", &i1);
	    window_offset= i1;
      }

      else if(active && (strcmp(keyword,"SSP_HPS_SINGLES_EMIN")==0))
      {
        sscanf (str_tmp, "%*s %d %d %d", &i1, &i2, &i3);		  
		if((i1 < 0) || (i1 > 1))
		{
	      printf("\nsspReadConfigFile: Wrong index number %d, %s\n",slot,str_tmp);
	      return(-4);
		}
        singles_emin[i1] = i2;
		singles_emin_en[i1] = i3;
      }

      else if(active && (strcmp(keyword,"SSP_HPS_SINGLES_EMAX")==0))
      {
        sscanf (str_tmp, "%*s %d %d %d", &i1, &i2, &i3);		  
		if((i1 < 0) || (i1 > 1))
		{
	      printf("\nsspReadConfigFile: Wrong index number %d, %s\n",slot,str_tmp);
	      return(-4);
		}
        singles_emax[i1] = i2;
		singles_emax_en[i1] = i3;
      }

      else if(active && (strcmp(keyword,"SSP_HPS_SINGLES_NMIN")==0))
      {
        sscanf (str_tmp, "%*s %d %d %d", &i1, &i2, &i3);		  
		if((i1 < 0) || (i1 > 1))
		{
	      printf("\nsspReadConfigFile: Wrong index number %d, %s\n",slot,str_tmp);
	      return(-4);
		}
        singles_nmin[i1] = i2;
		singles_nmin_en[i1] = i3;
      }
      
      else if(active && (strcmp(keyword, "SSP_HPS_SINGLES_PRESCALE")==0))
      {
		  scanf (str_tmp, "%*s %d %d %d %d %d", &i1, &i2, &i3, &i4, &i5);
        if((i1 < 0) || (i1 > 1))
        {
          printf("\nsspReadConfigFile: Wrong index number %d, %s\n",slot,str_tmp);
          return(-4);
        }
        if((i2 < 0) || (i2 > 6))
        {
          printf("\nsspReadConfigFile: Wrong region number %d, %s\n",slot,str_tmp);
          return(-4);
        }
        if((i3 < -31) || (i3 > 31))
        {
          printf("\nsspReadConfigFile: Wrong xmin number %d, %s\n",slot,str_tmp);
          return(-4);
        }
        if((i4 < -31) || (i4 > 31))
        {
          printf("\nsspReadConfigFile: Wrong xmax number %d, %s\n",slot,str_tmp);
          return(-4);
        }
        if((i5 < 0) || (i5 > 65535))
        {
          printf("\nsspReadConfigFile: Wrong prescale number %d, %s\n",slot,str_tmp);
          return(-4);
        }
        singles_prescalers[i1][i2].xmin = i3;
        singles_prescalers[i1][i2].xmax = i4;
        singles_prescalers[i1][i2].prescale = i5;
      }
      
      else if(active && (strcmp(keyword,"SSP_HPS_LATENCY")==0))
      {
        sscanf (str_tmp, "%*s %d", &i1);
        trigger_latency = i1;
      }

      else if(active && (strcmp(keyword,"SSP_HPS_COSMIC_TIMECOINCIDENCE")==0))
      {
        sscanf (str_tmp, "%*s %d", &i1);
        cosmic_timecoincidence = i1;
      }

      else if(active && (strcmp(keyword,"SSP_HPS_COSMIC_PATTERNCOINCIDENCE")==0))
      {
        sscanf (str_tmp, "%*s %d", &i1);
        cosmic_pattern = i1;
      }

      else if(active && (strcmp(keyword,"SSP_HPS_PAIRS_TIMECOINCIDENCE")==0))
      {
        sscanf (str_tmp, "%*s %d %d", &i1, &i2);
		if((i1 < 0) || (i1 > 1))
		{
	      printf("\nsspReadConfigFile: Wrong index number %d, %s\n",slot,str_tmp);
	      return(-4);
		}
        pairs_timecoincidence[i1] = i2;
      }

      else if(active && (strcmp(keyword,"SSP_HPS_PAIRS_EMIN")==0))
      {
        sscanf (str_tmp, "%*s %d %d", &i1, &i2);		  
		if((i1 < 0) || (i1 > 1))
		{
	      printf("\nsspReadConfigFile: Wrong index number %d, %s\n",slot,str_tmp);
	      return(-4);
		}
        pairs_emin[i1] = i2;
      }

      else if(active && (strcmp(keyword,"SSP_HPS_PAIRS_EMAX")==0))
      {
        sscanf (str_tmp, "%*s %d %d %d", &i1, &i2);		  
		if((i1 < 0) || (i1 > 1))
		{
	      printf("\nsspReadConfigFile: Wrong index number %d, %s\n",slot,str_tmp);
	      return(-4);
		}
        pairs_emax[i1] = i2;
      }

      else if(active && (strcmp(keyword,"SSP_HPS_PAIRS_NMIN")==0))
      {
        sscanf (str_tmp, "%*s %d %d", &i1, &i2);		  
		if((i1 < 0) || (i1 > 1))
		{
	      printf("\nsspReadConfigFile: Wrong index number %d, %s\n",slot,str_tmp);
	      return(-4);
		}
        pairs_nmin[i1] = i2;
      }

      else if(active && (strcmp(keyword,"SSP_HPS_PAIRS_SUMMAX")==0))
      {
        sscanf (str_tmp, "%*s %d %d %d", &i1, &i2, &i3);
		if((i1 < 0) || (i1 > 1))
		{
	      printf("\nsspReadConfigFile: Wrong index number %d, %s\n",slot,str_tmp);
	      return(-4);
	    }
        pairs_summax[i1] = i2;
	    pairs_summin[i1] = 0;
	    pairs_summax_en[i1] = i3;
      }

      else if(active && (strcmp(keyword,"SSP_HPS_PAIRS_SUMMAX_MIN")==0))
      {
        sscanf (str_tmp, "%*s %d %d %d %d", &i1, &i2, &i3, &i4);
		if((i1 < 0) || (i1 > 1))
		{
	      printf("\nsspReadConfigFile: Wrong index number %d, %s\n",slot,str_tmp);
	      return(-4);
	    }
        pairs_summax[i1] = i2;
	    pairs_summin[i1] = i3;
	    pairs_summax_en[i1] = i4;
      }

      else if(active && (strcmp(keyword,"SSP_HPS_PAIRS_DIFFMAX")==0))
      {
        sscanf (str_tmp, "%*s %d %d %d", &i1, &i2, &i3);
	    if((i1 < 0) || (i1 > 1))
		{
	      printf("\nsspReadConfigFile: Wrong index number %d, %s\n",slot,str_tmp);
	      return(-4);
  	    }
        pairs_diffmax[i1] = i2;
		pairs_diffmax_en[i1] = i3;
      }

      else if(active && (strcmp(keyword,"SSP_HPS_PAIRS_COPLANARITY")==0))
      {
        sscanf (str_tmp, "%*s %d %d %d", &i1, &i2, &i3);
		if((i1 < 0) || (i1 > 1))
		{
	      printf("\nsspReadConfigFile: Wrong index number %d, %s\n",slot,str_tmp);
	      return(-4);
		}
        pairs_coplanartolerance[i1] = i2;
		pairs_coplanartolerance_en[i1] = i3;
      }

      else if(active && (strcmp(keyword,"SSP_HPS_PAIRS_ENERGYDIST")==0))
      {
        sscanf (str_tmp, "%*s %d %f %d %d", &i1, &f1, &i3, &i4);
		if((i1 < 0) || (i1 > 1))
		{
	      printf("\nsspReadConfigFile: Wrong index number %d, %s\n",slot,str_tmp);
	      return(-4);
		}
        pairs_edfactor[i1] = f1;
		pairs_edmin[i1] = i3;
		pairs_ed_en[i1] = i4;
      }

      else if(active && (strcmp(keyword,"SSP_HPS_PULSER")==0))
      {
        sscanf (str_tmp, "%*s %d", &i1);
        pulser_freq = i1;
      }

      else if(active && (strcmp(keyword,"SSP_HPS_SET_IO_SRC")==0))
      {
        sscanf (str_tmp, "%*s %d %d", &i1, &i2);
        if((i1 < 0) || (i1 >= SD_SRC_NUM))
        {
          printf("\nsspReadConfigFile: Wrong src number %d, %s\n",slot,str_tmp);
          return(-4);
        }
        if((i2 < 0) || (i2 >= SD_SRC_SEL_NUM))
        {
          printf("\nsspReadConfigFile: Wrong sel number %d, %s\n",slot,str_tmp);
          return(-4);
        }
        ssp_io_mux[i1] = i2;
      }

      else
      {
        ; /* unknown key - do nothing */
		/*
        printf("sspReadConfigFile: Unknown Field or Missed Field in\n");
        printf("   %s \n", fname);
        printf("   str_tmp=%s", str_tmp);
        printf("   keyword=%s \n\n", keyword);
        return(-10);
		*/
      }

    }
  } /* end of while */

  fclose(fd);

  gr--;
  return(gr);
}



/*
cd "/usr/local/clas12/release/0.1/coda/src/rol/VXWORKS_ppc/lib"
ld < librol.so
sspInit(0,0,0,0x0)
sspConfig("")
sspMon(0)
*/


/* the number of events per block */
int
sspSetBlockLevel(int id, int block_level)
{
  if(id==0) id=sspSL[0]; 
  vmeWrite32(&pSSP[id]->EB.BlockCfg, block_level);
  return(0);
}

int
sspGetBlockLevel(int id)
{
  int ret;
  if(id==0) id=sspSL[0];
  ret = vmeRead32(&pSSP[id]->EB.BlockCfg);
  printf("sspGetBlockLevel returns %d\n",ret),fflush(stdout);
  return(ret);
}

/* Enable Bus Error */
int
sspEnableBusError(int id)
{
  if(id==0) id=sspSL[0]; 
  vmeWrite32(&pSSP[id]->EB.ReadoutCfg, 1);
  return(0);
}

int
sspDisableBusError(int id)
{
  if(id==0) id=sspSL[0]; 
  vmeWrite32(&pSSP[id]->EB.ReadoutCfg, 0);
  return(0);
}

int
sspGetBusError(int id)
{
  int ret;
  if(id==0) id=sspSL[0];
  ret = vmeRead32(&pSSP[id]->EB.ReadoutCfg);
  printf("sspGetBusError returns %d\n",ret),fflush(stdout);
  return(ret);
}


/* window size */
int
sspSetWindowWidth(int id, int window_width)
{
  if(id==0) id=sspSL[0]; 
  vmeWrite32(&pSSP[id]->EB.WindowWidth, window_width);
  return(0);
}

int
sspGetWindowWidth(int id)
{
  int ret;
  if(id==0) id=sspSL[0];
  ret = vmeRead32(&pSSP[id]->EB.WindowWidth);
  printf("sspGetWindowWidth returns %d\n",ret),fflush(stdout);
  return(ret);
}

/* window position */
int
sspSetWindowOffset(int id, int window_offset)
{
  if(id==0) id=sspSL[0]; 
  vmeWrite32(&pSSP[id]->EB.Lookback, window_offset);
  return(0);
}

int
sspGetWindowOffset(int id)
{
  int ret;
  if(id==0) id=sspSL[0];
  ret = vmeRead32(&pSSP[id]->EB.Lookback);
  printf("sspGetWindowOffset returns %d\n",ret),fflush(stdout);
  return(ret);
}

/* CONFIG */


int
sspDownloadAll()
{
  int ii, jj, kk;

  /*for(ii=0; ii<nssp; ii++) sspReset(ii);*/
  for(ii=0; ii<nSSP; ii++)
  {
    /* the number of events per block */
    sspSetBlockLevel(sspSL[ii], block_level);

    /* Enable Bus Error */
    if(bus_error) sspEnableBusError(sspSL[ii]);
    else          sspDisableBusError(sspSL[ii]);

    /* window size and position */
	 sspSetWindowWidth(sspSL[ii], window_width);
    sspSetWindowOffset(sspSL[ii], window_offset);
	 
	 if(pulser_freq)
    {
    	sspPulserSetup(sspSL[ii], (float)pulser_freq, 0.5, 0xFFFFFFFF);
      sspPulserStart(sspSL[ii]);
    }
	 else
    	sspPulserSetup(sspSL[ii], (float)0.0, 0, 0);

	 sspHps_SetLatency(sspSL[ii], trigger_latency);
	 
	 sspHps_SetCosmicTimeCoincidence(sspSL[ii], cosmic_timecoincidence);
	 sspHps_SetCosmicCoincidencePattern(sspSL[ii], cosmic_pattern);

    for(jj=0; jj<2; jj++)
	 {
      sspHps_SetSinglesEmin(sspSL[ii], jj, singles_emin[jj]);
      sspHps_SetSinglesEmax(sspSL[ii], jj, singles_emax[jj]);
      sspHps_SetSinglesNHitsmin(sspSL[ii], jj, singles_nmin[jj]);
      sspHps_SetSinglesEnableEmin(sspSL[ii], jj, singles_emin_en[jj]);
      sspHps_SetSinglesEnableEmax(sspSL[ii], jj, singles_emax_en[jj]);
      sspHps_SetSinglesEnableNmin(sspSL[ii], jj, singles_nmin_en[jj]);

      /* setup hps singles prescalers */
      for(kk=0; kk<7; kk++)
      {
        sspHps_SetSinglePrescale(sspSL[ii], jj, kk, 
											singles_prescalers[jj][kk].xmin,
											singles_prescalers[jj][kk].xmax,
											singles_prescalers[jj][kk].prescale);
      }
		
      sspHps_SetPairsEnableSum(sspSL[ii], jj, pairs_summax_en[jj]);
      sspHps_SetPairsEnableDiff(sspSL[ii], jj, pairs_diffmax_en[jj]);
      sspHps_SetPairsEnableCoplanar(sspSL[ii], jj, pairs_coplanartolerance_en[jj]);
      sspHps_SetPairsEnableED(sspSL[ii], jj, pairs_ed_en[jj]);
      sspHps_SetPairsTimeCoincidence(sspSL[ii], jj, pairs_timecoincidence[jj]);
      sspHps_SetPairsSummax(sspSL[ii], jj, pairs_summax[jj]);
		sspHps_SetPairsSummin(sspSL[ii], jj, pairs_summin[jj]);
      sspHps_SetPairsDiffmax(sspSL[ii], jj, pairs_diffmax[jj]);
      sspHps_SetPairsEmin(sspSL[ii], jj, pairs_emin[jj]);
      sspHps_SetPairsEmax(sspSL[ii], jj, pairs_emax[jj]);
      sspHps_SetPairsNHitsmin(sspSL[ii], jj, pairs_nmin[jj]);
      sspHps_SetPairsCoplanarTolerance(sspSL[ii], jj, pairs_coplanartolerance[jj]);
      sspHps_SetPairsEDFactor(sspSL[ii], jj, pairs_edfactor[jj]);
      sspHps_SetPairsEDmin(sspSL[ii], jj, pairs_edmin[jj]);
	 }

    for(jj=0; jj<SD_SRC_NUM; jj++)
      sspSetIOSrc(sspSL[ii], jj, ssp_io_mux[jj]);
  }

  return(0);
}



/* sspInit() have to be called before this function */
int  
sspConfig(char *fname)
{
  int res;
  char *string; /*dummy, will not be used*/

  if(strlen(fname) > 0) /* filename specified  - upload initial settings from the hardware */
  {
    sspUploadAll(string, 0);
  }
  else /* filename not specified  - set defaults */
  {
    sspInitGlobals();
  }

  /* read config file */
  if( (res = sspReadConfigFile(fname)) < 0 ) return(res);

  /* download to all boards */
  sspDownloadAll();

  return(0);
}

void
sspMon(int slot)
{
  int id, ii, start, end, res;
  usrVmeDmaGetConfig(&a24_a32, &sngl_blt_mblt, &sst_rate);

  printf("\nCPU DMA settings:\n");
  if(a24_a32==0) printf("  A16 address mode\n");
  else if(a24_a32==1) printf("  A24 address mode\n");
  else if(a24_a32==2) printf("  A32 address mode\n");
  else printf("  unknown address mode\n");

  if(sngl_blt_mblt==0x01) printf("  D32 single word readout\n");
  else if(sngl_blt_mblt==0x02) printf("  D32 DMA (BLT) readout\n");
  else if(sngl_blt_mblt==0x03) printf("  D64 DMA (MBLT) readout\n");
  else if(sngl_blt_mblt==0x04) printf("  D64 DMA (2eVME) readout\n");
  else if(sngl_blt_mblt==0x05) printf("  D64 DMA (2eSST) readout\n");
  else printf("  unknown readout mode !!!\n");

  if(sst_rate==0) printf("  SST160 rate\n");
  else if(sst_rate==1) printf("  SST267 rate\n");
  else printf("  unknown SST rate\n");

  printf("\n");

  if(slot==0)
  {
    start = 0;
    end   = nSSP;
  }
  else if((id = sspId(slot)) >= 0)
  {
    start = id;
    end   = start + 1;
  }
  else
  {
    printf("ERROR 1\n");
    return;
  }

  printf("SSP board ID's: start=%d, end=%d\n",start,end);fflush(stdout);

  for(ii=start; ii<end; ii++)
  {
    printf("[%d] slot %d -> block_level=%d, berr=%d, w_width=%d, w_offset=%d\n",ii,sspSL[ii],
		   sspGetBlockLevel(sspSL[ii]),sspGetBusError(sspSL[ii]),sspGetWindowWidth(sspSL[ii]),
		   sspGetWindowOffset(sspSL[ii]));
			
    sspPrintHpsConfig(ii);
  }

}




/* upload setting from all found DSC2s */
int
sspUploadAll(char *string, int length)
{
  int slot, i, j, kk, ifiber, len1, len2;
  char *str, sss[1024];
  unsigned int tmp, connectedfibers;
  unsigned short sval;
  unsigned short bypMask;
  unsigned short channels[8];

  for(kk=0; kk<nSSP; kk++)
  {
    slot = sspSlot(kk);
    bus_error = sspGetBusError(slot);
    block_level = sspGetBlockLevel(slot);
    window_width = sspGetWindowWidth(slot);
    window_offset = sspGetWindowOffset(slot);
    for(i=0; i<2; i++)
    {
      singles_emin[i] = sspHps_GetSinglesEmin(slot,i);
      singles_emin_en[i] = sspHps_GetSinglesEnableEmin(slot, i);
      singles_emax[i] = sspHps_GetSinglesEmax(slot,i);
      singles_emax_en[i] = sspHps_GetSinglesEnableEmax(slot, i);
      singles_nmin[i] = sspHps_GetSinglesNHitsmin(slot,i);
      singles_nmin_en[i] = sspHps_GetSinglesEnableNmin(slot, i);
		
		for(j=0; j<7;j++)
		{
		  singles_prescalers[i][j].xmin = sspHps_GetSinglePrescaleXmin(slot, i, j);
        singles_prescalers[i][j].xmax = sspHps_GetSinglePrescaleXmax(slot, i, j);
        singles_prescalers[i][j].prescale = sspHps_GetSinglePrescalePrescale(slot, i, j);
		}
    }

    trigger_latency =sspHps_GetLatency(slot);
    cosmic_timecoincidence = sspHps_GetCosmicTimeCoincidence(slot);
    cosmic_pattern = sspHps_GetCosmicCoincidencePattern(slot);

    for(i=0; i<2; i++)
    {
      pairs_timecoincidence[i] = sspHps_GetPairsTimeCoincidence(slot,i);
      pairs_emin[i] = sspHps_GetPairsEmin(slot,i);
      pairs_emax[i]= sspHps_GetPairsEmax(slot,i);
      pairs_nmin[i] = sspHps_GetPairsNHitsmin(slot,i);
      pairs_summax[i] = sspHps_GetPairsSummax(slot,i);
      pairs_summin[i] = sspHps_GetPairsSummin(slot,i);
      pairs_summax_en[i] = sspHps_GetPairsEnableSum(slot, i);
      pairs_diffmax[i] = sspHps_GetPairsDiffmax(slot,i);
      pairs_diffmax_en[i] = sspHps_GetPairsEnableDiff(slot,i);
      pairs_coplanartolerance[i] = sspHps_GetPairsCoplanarTolerance(slot,i);
      pairs_coplanartolerance_en[i] = sspHps_GetPairsEnableCoplanar(slot,i);
      pairs_edfactor[i] = sspHps_GetPairsEDFactor(slot,i);
      pairs_edmin[i] = sspHps_GetPairsEDmin(slot,i);
      pairs_ed_en[i] = sspHps_GetPairsEnableED(slot,i);
	}

	pulser_freq = (int)sspGetPulserFreq(slot);

    for(i = SD_SRC_P2_LVDSOUT0; i <= SD_SRC_P2_LVDSOUT7; i++)
	{
	  ssp_io_mux[i] = sspGetIOSrc(slot, i, 0);
	}
  }


  if(length)
  {
    str = string;
    str[0] = '\0';
    for(kk=0; kk<nSSP; kk++)
    {
      slot = sspSlot(kk);

      sprintf(sss,"SSP_SLOT %d\n",slot); ADD_TO_STRING;

      sprintf(sss,"SSP_BERR %d\n",bus_error); ADD_TO_STRING;
      sprintf(sss,"SSP_BLOCK_LEVEL %d\n",block_level); ADD_TO_STRING;
      sprintf(sss,"SSP_W_WIDTH %d\n",window_width); ADD_TO_STRING;
      sprintf(sss,"SSP_W_OFFSET %d\n",window_offset); ADD_TO_STRING;
	 
	  for(i = 0; i < 2; i++)
	  {
        sprintf(sss,"SSP_HPS_SINGLES_EMIN %d %d %d\n",i,singles_emin[i],singles_emin_en[i]); ADD_TO_STRING;
        sprintf(sss,"SSP_HPS_SINGLES_EMAX %d %d %d\n",i,singles_emax[i],singles_emax_en[i]); ADD_TO_STRING;
        sprintf(sss,"SSP_HPS_SINGLES_NMIN %d %d %d\n",i,singles_nmin[i],singles_nmin_en[i]); ADD_TO_STRING;
		  
        for(j=0; j<7;j++)
        {
          sprintf(sss,"SSP_HPS_SINGLES_PRESCALE %d %d %d %d %d\n",i,j,
						singles_prescalers[i][j].xmin,
						singles_prescalers[i][j].xmax,
						singles_prescalers[i][j].prescale);
          ADD_TO_STRING;
		  }
	  }

      sprintf(sss,"SSP_HPS_LATENCY %d\n",trigger_latency); ADD_TO_STRING;
      sprintf(sss,"SSP_HPS_COSMIC_TIMECOINCIDENCE %d\n",cosmic_timecoincidence); ADD_TO_STRING;
      sprintf(sss,"SSP_HPS_COSMIC_PATTERNCOINCIDENCE %d\n",cosmic_pattern); ADD_TO_STRING;

	  for(i = 0; i < 2; i++)
	  {
	    sprintf(sss,"SSP_HPS_PAIRS_TIMECOINCIDENCE %d %d\n",i,pairs_timecoincidence[i]); ADD_TO_STRING;
        sprintf(sss,"SSP_HPS_PAIRS_EMIN %d %d\n",i,pairs_emin[i]); ADD_TO_STRING;
        sprintf(sss,"SSP_HPS_PAIRS_EMAX %d %d\n",i,pairs_emax[i]); ADD_TO_STRING;
        sprintf(sss,"SSP_HPS_PAIRS_NMIN %d %d\n",i,pairs_nmin[i]); ADD_TO_STRING;
        sprintf(sss,"SSP_HPS_PAIRS_SUMMAX_MIN %d %d %d %d\n",i,pairs_summax[i],pairs_summin[i],pairs_summax_en[i]); ADD_TO_STRING;
        sprintf(sss,"SSP_HPS_PAIRS_DIFFMAX %d %d %d\n",i,pairs_diffmax[i],pairs_diffmax_en[i]); ADD_TO_STRING;
        sprintf(sss,"SSP_HPS_PAIRS_COPLANARITY %d %d %d\n",i,pairs_coplanartolerance[i],pairs_coplanartolerance_en[i]); ADD_TO_STRING;
        sprintf(sss,"SSP_HPS_PAIRS_ENERGYDIST %d %7.3f %d %d\n",i,pairs_edfactor[i],pairs_edmin[i],pairs_ed_en[i]); ADD_TO_STRING;
	  }
      sprintf(sss,"SSP_HPS_PULSER %d\n",pulser_freq); ADD_TO_STRING;
      for(i = SD_SRC_P2_LVDSOUT0; i <= SD_SRC_P2_LVDSOUT7; i++)
	  {
        sprintf(sss,"SSP_HPS_SET_IO_SRC %d %d\n",i,ssp_io_mux[i]); ADD_TO_STRING;
	  }
    }

    CLOSE_STRING;
  }

}

#endif

/*
#
# ssp config file
#
# this file contains settings for SSP
#
# format:
# ~~~~~~~
# SSP_CRATE             rocbcal1   <- ROC/crate name, usually IP name
#
# SSP_ALLSLOTS             <- just keyword - all settings after this line will be implemented
#                                                for all slots, till SSP_SLOTS will be met
#
# SSP_SLOTS     3  8  19   <- slot_numbers - in which next settings will be implemented
#                                                till file ends or next SSP_SLOTS will be met
#
# SSP_BERR  1   <- DMA Nwords method readout: 1 - enable VME BUS error
#                                             0 - disable VME BUS error
#
# SSP_BLOCK_LEVEL 1      <- to set block level
#
# SSP_W_WIDTH    N
#    N: 0 to 1023 - Trigger Window Width, in 4ns ticks
#
# SSP_W_OFFSET   N
#    N: 0 to 1023 - Trigger Window Offset, in 4ns ticks
#
# SSP_HPS_LATENCY  LATENCY
#    LATENCY: 0 to 511 - sets trigger latency (in 4ns ticks)
#
# SSP_HPS_SINGLES:
# ----------------
# SSP_HPS_SINGLES_EMIN  N  EMIN   EN
#    N:    0 or 1 - selects the hps singles trigger bit
#    EMIN: 0 to 8191MeV - cluster energies below this threshold are cut
#    EN:   1 - enables, 0 - disables
#
# SSP_HPS_SINGLES_EMAX  N  EMAX   EN
#    N:    0 or 1 - selects the hps singles trigger bit
#    EMAX: 0 to 8191MeV - cluster energies above this threshold are cut
#    EN:   1 - enables, 0 - disables
#
# SSP_HPS_SINGLES_NMIN  N  NMIN   EN
#    N:    0 or 1 - selects the hps singles trigger bit
#    NMIN: 0 to 9 - cluster with #hits below this threshold are cut
#    EN:   1 - enables, 0 - disables
#
# SSP_HPS_SINGLES_PRESCALE N REGION XMIN XMAX PRESCALE
#    N:        0 or 1 - selects the hps singles trigger bit
#    REGION:   0 to 6 - selects which region index to define
#    XMIN:    -31 to 31  - chooses the ECAL x coordinate to start region definition (inclusive)
#    XMAX:    -31 to 31  - chooses the ECAL x coordinate to end region definition  (inclusive)
#    PRESCALE: 0 to 65535 - set the prescale value for the region
#
#    Notes:
#         1) To disable prescaling, set a region to cover the whole ECAL (i.e. xmin = -31, max = 31) and use PRESCALE = 0
#         2) When using prescaling, define regions to be mutually exclusive to make the actual prescaling simple to understand
#         3) The prescalers for all regions are OR'd together to form the actual singles trigger bit
#         4) By default (if not defining anything) Region 0 is defined as the whole ECAL with prescaling disabled
#            and Regions 1-6 are defined outside the ECAL range (i.e. disabled)
#
# SSP_HPS_COSMICS:
# ----------------
# SSP_HPS_COSMIC_TIMECOINCIDENCE  T
#    T:    0 to 255 - sets cosmic hit coincidence time (in 4ns ticks)
#
# SSP_HPS_COSMIC_PATTERNCOINCIDENCE  pattern
#    pattern  - 3:1 LUT definition for scintillator coincidence pattern that is accepted/rejected
#                Scintillator channels are the last 3 channels (14-16) of FADC in slot 20
#                pattern = 254 will trigger on any hit channels
#                pattern = 128 will trigger when all 3 channels hit
#                pattern = 136 will trigger when channels 14&15 are hit
#                pattern = 232 will trigger when any 2 channels hit
#
# SSP_HPS_PAIRS:
# ----------------
# SSP_HPS_PAIRS_TIMECOINCIDENCE  N   T
#    N:    0 or 1 - selects the hps pair trigger bit
#    T:    0 to 15 - sets cluster pair hit coincidence time (in +/-4ns ticks)
#
# SSP_HPS_PAIRS_EMIN  N   EMIN
#    N:    0 or 1 - selects the hps pair trigger bit
#    EMIN: 0 to 8191MeV - clusters energies below this threshold are cut
#
# SSP_HPS_PAIRS_EMAX  N   EMAX
#    N:    0 or 1 - selects the hps pair trigger bit
#    EMAX: 0 to 8191MeV - clusters energies above this threshold are cut
#
# SSP_HPS_PAIRS_NMIN  N   NMIN
#    N:    0 or 1 - selects the hps pair trigger bit
#    NMIN: 0 to 9 - cluster with #hits below this threshold are cut
#
# SSP_HPS_PAIRS_SUMMAX  N   MAX   EN
#    N:    0 or 1 - selects the hps pair trigger bit
#    MAX:  0 to 8191MeV - cluster pairs energy sums above this threshold are cut
#    EN:   1 - enables, 0 - disables
#
# SSP_HPS_PAIRS_DIFFMAX  N  MAX   EN 
#    N:    0 or 1 - selects the hps pair trigger bit
#    MAX:  0 to 8191MeV - cluster pairs energy differences above this threshold are cut
#    EN:   1 - enables, 0 - disables
#
# SSP_HPS_PAIRS_COPLANARITY  N   ANGLE  EN
#    N:    0 or 1 - selects the hps pair trigger bit
#    ANGLE: 0 to 180 - cluster pair coplanarity with photon beam must be < +/-ANGLE (in degress)
#    EN:   1 - enables, 0 - disables
#
# SSP_HPS_PAIRS_ENERGYDIST  N   F  MIN  EN
#    N:    0 or 1 - selects the hps pair trigger bit
#    F:    0 to 8191 - conversion factor in MeV/mm to compensate cluster energy by radial distance from photon beam
#    MIN:  0 to 8191 - minimum threshold in MeV to accept cluster pair.
#          For each pair, the cluster with the smallest energy, Emin, is used in this cut.
#          That cluster's minimum distance to photon beam is computed, R, and tested in the equation:
#              MIN < Emin + F * R
#          When the above is true the cluster pair passes this cut, otherwise it is cut.
#    EN:   1 - enables, 0 - disables
#
# SSP_HPS_PULSER   F
#    F:     Pulser frequency (0 to 125MHz)
#
# SSP_HPS_SET_IO_SRC   OUTPUT_PIN     SIGNAL_SEL
#    OUTPUT_PIN selection:
#       LVDSOUT0        0
#       LVDSOUT1        1
#       LVDSOUT2        2
#       LVDSOUT3        3
#       LVDSOUT4        4
#       GPIO0           5
#       GPIO1           6
#       P2_LVDSOUT0     7
#       P2_LVDSOUT1     8
#       P2_LVDSOUT2     9
#       P2_LVDSOUT3     10
#       P2_LVDSOUT4     11
#       P2_LVDSOUT5     12
#       P2_LVDSOUT6     13
#       P2_LVDSOUT7     14
#       TRIG            15
#       SYNC            16
#
#    SIGNAL_SEL selection:
#       0          0
#       1          1
#       SYNC       2
#       TRIG1      3
#       TRIG2      4
#       LVDSIN0    5
#       LVDSIN1    6
#       LVDSIN2    7
#       LVDSIN3    8
#       LVDSIN4    9
#       P2LVDSIN0  10
#       P2LVDSIN1  11
#       P2LVDSIN2  12
#       P2LVDSIN3  13
#       P2LVDSIN4  14
#       P2LVDSIN5  15
#       P2LVDSIN6  16
#       P2LVDSIN7  17
#       PULSER     18
#       BUSY       19
#       TRIGGER0   20   (HPS SINGLES 0)
#       TRIGGER1   21   (HPS SINGLES 1)
#       TRIGGER2   22   (HPS PAIRS 0)
#       TRIGGER3   23   (HPS PAIRS 1)
#       TRIGGER4   24   (HPS LED)
#       TRIGGER5   25   (HPS COSMIC)
#       TRIGGER6   26
#       TRIGGER7   27
*/