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

#ifdef VXWORKS
#define SYNC()		{ __asm__ volatile("eieio"); __asm__ volatile("sync"); }
#endif

/* Global Variables */
int nSSP=0;                                /* Number of SSPs found with sspInit(..) */
volatile SSP_regs *pSSP[MAX_VME_SLOTS+1];  /* pointers to SSP memory map */
int sspID[MAX_VME_SLOTS+1];                /* array of slot numbers for SSPs */
unsigned int sspAddrList[MAX_VME_SLOTS+1]; /* array of a24 addresses for SSPs */
int sspA24Offset=0;                        /* Difference in Local A24 Base and VME A24 Base */

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
  unsigned int rdata, laddr, laddr_inc, boardID;
  int issp=0, islot=0, res;
  int result=OK;
  volatile SSP_regs *ssp;
  unsigned int firmwareInfo=0, sspVersion=0;

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
	sspAddrList[islot-3] = (islot<<19);
      
      /* Skip Switch Slots */
      
      for(islot=13; islot<21; islot++) /* Last 8 */
	sspAddrList[islot-5] = (islot<<19);

    }
  else if(addr > 0x00ffffff) 
    { /* A32 Addressing */
      printf("%s: ERROR: A32 Addressing not allowed for SSP configuration space\n",
	     __FUNCTION__);
      return(ERROR);
    }
  else
    { /* A24 Addressing for ONE SSP */
      if( ((addr_inc==0)||(nfind==0)) && (useList==0) )
	nfind = 1; /* assume only one SSP to initialize */
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

  for (issp=0;issp<nfind;issp++) 
    {
      if(useList==1)
	{
	  laddr_inc = sspAddrList[issp] + sspA24Offset;
	}
      else
	{
	  laddr_inc = laddr +issp*addr_inc;
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
	      printf(" WARN: For board at 0x%x, Invalid Board ID: 0x%x\n",
		     (UINT32) laddr_inc-sspA24Offset, rdata);
	      continue;
	    }
	  else 
	    {

	      /* Check if this is board has a valid slot number */
	      boardID =  (sspReadReg(&ssp->Cfg.FirmwareRev)&SSP_CFG_SLOTID_MASK)>>24;
	      if((boardID <= 0)||(boardID >21)) 
		{
		  printf(" WARN: Board Slot ID is not in range: %d (this module ignored)\n"
			 ,boardID);
		  continue;
		}
	      else
		{
		  pSSP[boardID] = (volatile SSP_regs *)(laddr_inc);
		  sspID[nSSP] = boardID;

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


		  printf("Initialized SSP %2d  Slot # %2d at address 0x%08x (0x%08x) \n",
			 nSSP, sspID[nSSP],(UINT32) pSSP[(sspID[nSSP])],
			 (UINT32) pSSP[(sspID[nSSP])]-sspA24Offset);
		}
	    }
	  nSSP++;
	}
    }

  /* Setup initial configuration */
  if(noBoardInit==0)
    {
      for(issp=0; issp<nSSP; issp++)
	{
	  result = sspSetMode(sspSlot(issp),iFlag,0);
	  if(result != OK)
	    return ERROR;
	}	
    }

  return OK;
}

void
sspCheckAddresses(int id)
{
  unsigned int offset=0, expected=0, base=0;
  int iser=0;

  if(id==0) id=sspID[0];
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

  offset = ((unsigned int) &pSSP[id]->Trg) - base;
  expected = 0x400;
  if(offset != expected)
    printf("%s: ERROR pSSP[%d]->Trg not at offset = 0x%x (@ 0x%x)\n",
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
sspSlot(unsigned int i)
{
  if(i>=nSSP)
    {
      printf("%s: ERROR: Index (%d) >= SSPs initialized (%d).\n",
	     __FUNCTION__,i,nSSP);
      return ERROR;
    }

  return sspID[i];
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

  if(id==0) id=sspID[0];
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
	
  if(sspTriggerSetup(id, (iFlag & SSP_INIT_FIBER_ENABLE_MASK)>>16, 
		     (iFlag & SSP_INIT_GTP_FIBER_ENABLE_MASK)>>24, pflag) != OK)
    return ERROR;

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
  if(id==0) id=sspID[0];
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
  st.Trg.Ctrl         = sspReadReg(&pSSP[id]->Trg.Ctrl);
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
      printf("  Trg.Ctrl       (0x%04x) = 0x%08x\n", (unsigned int)(&pSSP[id]->Trg.Ctrl) - SSPBase, st.Trg.Ctrl);
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
      for(i=0; i <= SSP_SER_FIBER7; i++)
	{
	  if(fiberEnabledMask & (1<<i))
	    printf("   %-10s: -%-12s-\n",
		   ssp_serdes_names[i],
                   (st.Ser[i].Status & SSP_SER_STATUS_CHUP) ?   
		   "CHANNEL UP" :
		   "CHANNEL DN");
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
      st[id].Trg.Ctrl         = sspReadReg(&pSSP[id]->Trg.Ctrl);
      for(i=0; i < SSP_SER_NUM; i++)
	{
	  st[id].Ser[i].Ctrl       = sspReadReg(&pSSP[id]->Ser[i].Ctrl);
	  st[id].Ser[i].Status     = sspReadReg(&pSSP[id]->Ser[i].Status);
	  st[id].Ser[i].MonStatus  = sspReadReg(&pSSP[id]->Ser[i].MonStatus);
	  st[id].Ser[i].CrateId    = sspReadReg(&pSSP[id]->Ser[i].CrateId) & SER_CRATEID_MASK;
	  st[id].Ser[i].ErrTile0   = sspReadReg(&pSSP[id]->Ser[i].ErrTile0);
	  st[id].Ser[i].ErrTile1   = sspReadReg(&pSSP[id]->Ser[i].ErrTile1);
	  portUsedInTrigger[id][i] = (st[id].Trg.Ctrl & (1<<i))>>i;
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

	  /* Used in trigger */
	  printf("%s        ",
		 (portUsedInTrigger[id][iport]) ?   
		 " NO":"YES");

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

  if(id==0) id=sspID[0];
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
  if(id==0) id=sspID[0];
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

  if(id==0) id=sspID[0];
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
  if(id==0) id=sspID[0];
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

  if(id==0) id=sspID[0];
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
	
  if(id==0) id=sspID[0];
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
 * void sspTriggerSetup(int fiber_mask, int gtp_src)
 *   fiber_mask (bits determine which fiber ports are used in trigger):
 *    TRG_CTRL_FIBER_EN0			0x00000001
 *    TRG_CTRL_FIBER_EN1			0x00000002
 *    TRG_CTRL_FIBER_EN2			0x00000004
 *    TRG_CTRL_FIBER_EN3			0x00000008
 *    TRG_CTRL_FIBER_EN4			0x00000010
 *    TRG_CTRL_FIBER_EN5			0x00000020
 *    TRG_CTRL_FIBER_EN6			0x00000040
 *    TRG_CTRL_FIBER_EN7			0x00000080
 *
 *   gtp_src (value determines what data the gtp will receive):
 *    TRG_CTRL_GTPSRC_FIBER0	0
 *    TRG_CTRL_GTPSRC_FIBER1	1
 *    TRG_CTRL_GTPSRC_FIBER2	2
 *    TRG_CTRL_GTPSRC_FIBER3	3
 *    TRG_CTRL_GTPSRC_FIBER4	4
 *    TRG_CTRL_GTPSRC_FIBER5	5
 *    TRG_CTRL_GTPSRC_FIBER6	6
 *    TRG_CTRL_GTPSRC_FIBER7	7
 *    TRG_CTRL_GTPSRC_SUM	8
 */

int 
sspTriggerSetup(int id, int fiber_mask, int gtp_src, int pflag)
{
  int i;
	
  if(id==0) id=sspID[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL))
    {
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id);
      return ERROR;
    }

  SSPLOCK;
  sspWriteReg(&pSSP[id]->Trg.Ctrl, fiber_mask | (gtp_src<<8));
  SSPUNLOCK;

  if(pflag)
    {
      printf("%s -\n",__FUNCTION__);
      printf("   enabled fiber ports: ");
      
      for(i = 0; i < 8; i++)
	{
	  if(fiber_mask & (1<<i))
	    printf("%d ", i);
	}
      printf("\n");
      
      if((gtp_src < 0) || (gtp_src >= TRG_CTRL_GTPSRC_NUM))
	{
	  printf("   gtp data source: unknown\n");
	  return ERROR;
	}
      else
	printf("   gtp data source: %s\n", ssp_gtpsrc_names[gtp_src]);
    }	
  return OK;
}

/************************************************************
 * SSP SD.PULSER Functions
 ************************************************************/

int 
sspPulserStatus(int id)
{
  if(id==0) id=sspID[0];
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
  if(id==0) id=sspID[0];
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
 * int sspSetClkSrc(float freq, float duty, unsigned npulses)
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
sspPulserSetup(int id, float freq, float duty, unsigned npulses)
{
  unsigned int per, low;
	
  if(id==0) id=sspID[0];
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
  // Setup period register...
  per = SYSCLK_FREQ / freq;
  if(!per)
    per = 1;
  sspWriteReg(&pSSP[id]->Sd.PulserPeriod, per);
	
  // Setup duty cycle register...	
  low = per * duty;
  if(!low)
    low = 1;
  sspWriteReg(&pSSP[id]->Sd.PulserLowCycles, low);
	
  sspWriteReg(&pSSP[id]->Sd.PulserNPulses, npulses);
	
  printf("%s: Actual frequency = %f, duty = %f\n", 
	 __FUNCTION__,
	 (float)SYSCLK_FREQ/(float)per, (float)low/(float)per);
  SSPUNLOCK;
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
  int i;
	
  if(id==0) id=sspID[0];
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
	    }
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
	
  if(id==0) id=sspID[0];
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

  if(id==0) id=sspID[0];
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

  if(id==0) id=sspID[0];
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

  if(id==0) id=sspID[0];
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
  if(id==0) id=sspID[0];
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

  if(id==0) id=sspID[0];
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
  if(id==0) id=sspID[0];
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
	
  if(id==0) id=sspID[0];
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
  if(id==0) id=sspID[0];
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
	
  if(id==0) id=sspID[0];
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
  if(id==0) id=sspID[0];
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
	
  if(id==0) id=sspID[0];
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

  if(id==0) id=sspID[0];
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

  if(id==0) id=sspID[0];
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
	
  if(id==0) id=sspID[0];
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

  if(id==0) id=sspID[0];
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

  if(id==0) id=sspID[0];
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

/* sspHps_SetLatency() - set trigger latency in 4ns ticks */
int sspHps_SetLatency(int id, int latency)
{
  if(id==0) id=sspID[0];
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
  sspWriteReg(&pSSP[id]->HpsSingles.Latency, latency);
  sspWriteReg(&pSSP[id]->HpsPairs.Latency, latency);
  SSPUNLOCK;

  return OK;
}

/* sspHps_SinglesEnableTop() - enable/disable top calorimeter cluster trigger */
int sspHps_SetSinglesEnableTop(int id, int en)
{
  int val;

  if(id==0) id=sspID[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL))
    {
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id);
      return ERROR;
    }

  SSPLOCK;
  if(en)
    val = sspReadReg(&pSSP[id]->HpsSingles.Ctrl) | 0x00000001;
  else
    val = sspReadReg(&pSSP[id]->HpsSingles.Ctrl) & 0xFFFFFFFE;
  sspWriteReg(&pSSP[id]->HpsSingles.Ctrl, val);
  SSPUNLOCK;

  return OK;
}

/* sspHps_SinglesEnableBot() - enable/disable bottom calorimeter cluster trigger */
int sspHps_SetSinglesEnableBot(int id, int en)
{
  int val;

  if(id==0) id=sspID[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL))
    {
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id);
      return ERROR;
    }

  SSPLOCK;
  if(en)
    val = sspReadReg(&pSSP[id]->HpsSingles.Ctrl) | 0x00000002;
  else
    val = sspReadReg(&pSSP[id]->HpsSingles.Ctrl) & 0xFFFFFFFD;
  sspWriteReg(&pSSP[id]->HpsSingles.Ctrl, val);
  SSPUNLOCK;

  return OK;
}

/* sspHps_SinglesEmin() - set minimum cluster energy for calorimeter single cluster trigger (units: MeV) */
int sspHps_SetSinglesEmin(int id, int min)
{
  if(id==0) id=sspID[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL))
    {
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id);
      return ERROR;
    }

  if( (min < 0) || (min > 8191) )
  {
    printf("%s: ERROR: emin is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  sspWriteReg(&pSSP[id]->HpsSingles.ClusterEmin, min);
  SSPUNLOCK;

  return OK;
}

/* sspHps_SinglesEmax() - set maximum cluster energy for calorimeter single cluster trigger (units: MeV) */
int sspHps_SetSinglesEmax(int id, int max)
{
  if(id==0) id=sspID[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL))
    {
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id);
      return ERROR;
    }

  if( (max < 0) || (max > 8191) )
  {
    printf("%s: ERROR: emax is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  sspWriteReg(&pSSP[id]->HpsSingles.ClusterEmax, max);
  SSPUNLOCK;

  return OK;
}

/* sspHps_SinglesNHitsmin() - set minimum cluster hits for calorimeter single cluster trigger (units: counts) */
int sspHps_SetSinglesNHitsmin(int id, int min)
{
  if(id==0) id=sspID[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL))
    {
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id);
      return ERROR;
    }

  if( (min < 0) || (min > 9) )
  {
    printf("%s: ERROR: emin is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  sspWriteReg(&pSSP[id]->HpsSingles.ClusterNHitsmin, min);
  SSPUNLOCK;

  return OK;
}

/* sspHps_PairsTimeCoincidence() - set cluster pair coincidence time window (units: +/-4ns) */
int sspHps_SetPairsTimeCoincidence(int id, int ticks)
{
  if(id==0) id=sspID[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL))
    {
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id);
      return ERROR;
    }

  if( (ticks < 0) || (ticks > 15) )
  {
    printf("%s: ERROR: ticks is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  sspWriteReg(&pSSP[id]->HpsPairs.ClusterTimeCoincidence, ticks);
  SSPUNLOCK;

  return OK;
}

/* sspHps_PairsSummax() - set cluster pair sum maximum (units: MeV) */
int sspHps_SetPairsSummax(int id, int max)
{
  if(id==0) id=sspID[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL))
    {
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id);
      return ERROR;
    }

  if( (max < 0) || (max > 8191) )
  {
    printf("%s: ERROR: ticks is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  sspWriteReg(&pSSP[id]->HpsPairs.ClusterSummax, max);
  SSPUNLOCK;

  return OK;
}

/* sspHps_PairsDiffmax() - set cluster pair difference maximum (units: MeV) */
int sspHps_SetPairsDiffmax(int id, int max)
{
  if(id==0) id=sspID[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL))
    {
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id);
      return ERROR;
    }

  if( (max < 0) || (max > 8191) )
  {
    printf("%s: ERROR: ticks is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  sspWriteReg(&pSSP[id]->HpsPairs.ClusterDiffmax, max);
  SSPUNLOCK;

  return OK;
}


/* sspHps_PairsEmin() - set cluster pair cluster energy minimum (units: MeV) */
int sspHps_SetPairsEmin(int id, int min)
{
  if(id==0) id=sspID[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL))
    {
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id);
      return ERROR;
    }

  if( (min < 0) || (min > 8191) )
  {
    printf("%s: ERROR: emin is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  sspWriteReg(&pSSP[id]->HpsPairs.ClusterEmin, min);
  SSPUNLOCK;

  return OK;
}

/* sspHps_PairsEmin() - set cluster pair cluster energy maximum (units: MeV) */
int sspHps_SetPairsEmax(int id, int max)
{
  if(id==0) id=sspID[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL))
    {
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id);
      return ERROR;
    }

  if( (max < 0) || (max > 8191) )
  {
    printf("%s: ERROR: emax is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  sspWriteReg(&pSSP[id]->HpsPairs.ClusterEmax, max);
  SSPUNLOCK;

  return OK;
}

/* sspHps_PairsNHitsmin() - set minimum cluster hits for calorimeter pair cluster trigger (units: counts) */
int sspHps_SetPairsNHitsmin(int id, int min)
{
  if(id==0) id=sspID[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL))
    {
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id);
      return ERROR;
    }

  if( (min < 0) || (min > 9) )
  {
    printf("%s: ERROR: emin is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  sspWriteReg(&pSSP[id]->HpsPairs.ClusterNHitsmin, min);
  SSPUNLOCK;

  return OK;
}

/* sspHps_PairsCoplanarTolerance() - set cluster pair-beam coplanarity tolerance (units: degrees) */
int sspHps_SetPairsCoplanarTolerance(int id, int tol)
{
  if(id==0) id=sspID[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL))
    {
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id);
      return ERROR;
    }

  if( (tol < 0) || (tol > 255) )
  {
    printf("%s: ERROR: tol is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  sspWriteReg(&pSSP[id]->HpsPairs.ClusterCoplanarTol, tol);
  SSPUNLOCK;

  return OK;
}

/* sspHps_PairsEDFactor() - set cluster pair radial distance->energy factor (units: MeV/cm) */
int sspHps_SetPairsEDFactor(int id, int f)
{
  if(id==0) id=sspID[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL))
    {
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id);
      return ERROR;
    }

  if( (f < 0) || (f > 8191) )
  {
    printf("%s: ERROR: f is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  sspWriteReg(&pSSP[id]->HpsPairs.ClusterEDFactor, f);
  SSPUNLOCK;

  return OK;
}

int sspHps_SetPairsEDmin(int id, int min)
{
  if(id==0) id=sspID[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL))
    {
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id);
      return ERROR;
    }

  if( (min < 0) || (min > 8191) )
  {
    printf("%s: ERROR: min is outside acceptable range.\n",__FUNCTION__);
    return ERROR;
  }

  SSPLOCK;
  sspWriteReg(&pSSP[id]->HpsPairs.ClusterEDmin, min);
  SSPUNLOCK;

  return OK;
}

int sspHps_SetPairsEnableSum(int id, int en)
{
  int val;

  if(id==0) id=sspID[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL))
    {
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id);
      return ERROR;
    }

  SSPLOCK;
  if(en)
    val = sspReadReg(&pSSP[id]->HpsPairs.Ctrl) | 0x00000001;
  else
    val = sspReadReg(&pSSP[id]->HpsPairs.Ctrl) & 0xFFFFFFFE;
  sspWriteReg(&pSSP[id]->HpsPairs.Ctrl, val);
  SSPUNLOCK;

  return OK;
}

int sspHps_SetPairsEnableDiff(int id, int en)
{
  int val;

  if(id==0) id=sspID[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL))
    {
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id);
      return ERROR;
    }

  SSPLOCK;
  if(en)
    val = sspReadReg(&pSSP[id]->HpsPairs.Ctrl) | 0x00000002;
  else
    val = sspReadReg(&pSSP[id]->HpsPairs.Ctrl) & 0xFFFFFFFD;
  sspWriteReg(&pSSP[id]->HpsPairs.Ctrl, val);
  SSPUNLOCK;

  return OK;
}

int sspHps_SetPairsEnableCoplanar(int id, int en)
{
  int val;

  if(id==0) id=sspID[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL))
    {
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id);
      return ERROR;
    }

  SSPLOCK;
  if(en)
    val = sspReadReg(&pSSP[id]->HpsPairs.Ctrl) | 0x00000004;
  else
    val = sspReadReg(&pSSP[id]->HpsPairs.Ctrl) & 0xFFFFFFFB;
  sspWriteReg(&pSSP[id]->HpsPairs.Ctrl, val);
  SSPUNLOCK;

  return OK;
}

int sspHps_SetPairsEnableED(int id, int en)
{
  int val;

  if(id==0) id=sspID[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL))
    {
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id);
      return ERROR;
    }

  SSPLOCK;
  if(en)
    val = sspReadReg(&pSSP[id]->HpsPairs.Ctrl) | 0x00000008;
  else
    val = sspReadReg(&pSSP[id]->HpsPairs.Ctrl) & 0xFFFFFFF7;
  sspWriteReg(&pSSP[id]->HpsPairs.Ctrl, val);
  SSPUNLOCK;

  return OK;
}

void sspPrintHpsScalers(int id)
{
  double ref, rate;
  int i;
  unsigned int scalers[SD_SCALER_NUM];
  unsigned int hpsscalers[8];
  const char *hpsscalers_name[8] = {
    "PairPass",
    "SumPass",
    "DiffPass",
    "EDPass",
    "CoplanarPass",
    "TriggerPass",
    "SinglesPass",
    "SinglesTot"
    };

  if(id==0) id=sspID[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL))
    {
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id);
      return;
    }

  SSPLOCK;
  sspWriteReg(&pSSP[id]->Sd.ScalerLatch, 0);

  for(i = 0; i < SD_SCALER_NUM; i++)
    scalers[i] = sspReadReg(&pSSP[id]->Sd.Scalers[i]);

  hpsscalers[0] = sspReadReg(&pSSP[id]->HpsPairs.ScalerPairsPass);
  hpsscalers[1] = sspReadReg(&pSSP[id]->HpsPairs.ScalerSumPass);
  hpsscalers[2] = sspReadReg(&pSSP[id]->HpsPairs.ScalerDiffPass);
  hpsscalers[3] = sspReadReg(&pSSP[id]->HpsPairs.ScalerEDPass);
  hpsscalers[4] = sspReadReg(&pSSP[id]->HpsPairs.ScalerCoplanarPass);
  hpsscalers[5] = sspReadReg(&pSSP[id]->HpsPairs.ScalerTriggerPass);
  hpsscalers[6] = sspReadReg(&pSSP[id]->HpsSingles.ScalerSinglesPass);
  hpsscalers[7] = sspReadReg(&pSSP[id]->HpsSingles.ScalerSinglesTot);

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

  for(i = 0; i < 8; i++)
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
  int singlesLatency, singlesTopEn, singlesBotEn, singlesEmin, singlesEmax, singlesNHitsmin,
      pairsLatency, pairsTimeCoincidence, pairsSummax, pairsDiffmax, pairsEmin, pairsEmax,
      pairsNHitsmin, pairsCoplanarTolerance, pairsEDFactor, pairsEDmin, pairsSumEn,
      pairsDiffEn, pairsCoplanarEn, pairsEDEn;

  if(id==0) id=sspID[0];
  if((id<=0) || (id>21) || (pSSP[id]==NULL))
    {
      printf("%s: ERROR: SSP in slot %d not initialized\n",__FUNCTION__,id);
      return;
    }

  printf("*** HpsConfig ***\n");

  SSPLOCK;
  singlesLatency = sspReadReg(&pSSP[id]->HpsSingles.Latency);
  singlesTopEn = sspReadReg(&pSSP[id]->HpsSingles.Ctrl) & 0x1;
  singlesBotEn = sspReadReg(&pSSP[id]->HpsSingles.Ctrl) & 0x2;
  singlesEmin = sspReadReg(&pSSP[id]->HpsSingles.ClusterEmin);
  singlesEmax = sspReadReg(&pSSP[id]->HpsSingles.ClusterEmax);
  singlesNHitsmin = sspReadReg(&pSSP[id]->HpsSingles.ClusterNHitsmin);
  pairsLatency = sspReadReg(&pSSP[id]->HpsPairs.Latency);
  pairsTimeCoincidence = sspReadReg(&pSSP[id]->HpsPairs.ClusterTimeCoincidence);
  pairsSummax = sspReadReg(&pSSP[id]->HpsPairs.ClusterSummax);
  pairsDiffmax = sspReadReg(&pSSP[id]->HpsPairs.ClusterDiffmax);
  pairsEmin = sspReadReg(&pSSP[id]->HpsPairs.ClusterEmin);
  pairsEmax = sspReadReg(&pSSP[id]->HpsPairs.ClusterEmax);
  pairsNHitsmin = sspReadReg(&pSSP[id]->HpsPairs.ClusterNHitsmin);
  pairsCoplanarTolerance = sspReadReg(&pSSP[id]->HpsPairs.ClusterCoplanarTol);
  pairsEDFactor = sspReadReg(&pSSP[id]->HpsPairs.ClusterEDFactor);
  pairsEDmin = sspReadReg(&pSSP[id]->HpsPairs.ClusterEDmin);
  pairsSumEn = sspReadReg(&pSSP[id]->HpsPairs.Ctrl) & 0x1;
  pairsDiffEn = sspReadReg(&pSSP[id]->HpsPairs.Ctrl) & 0x2;
  pairsCoplanarEn = sspReadReg(&pSSP[id]->HpsPairs.Ctrl) & 0x4;
  pairsEDEn = sspReadReg(&pSSP[id]->HpsPairs.Ctrl) & 0x8;
  SSPUNLOCK;

  printf("   *** Singles Configuration ***\n");
  printf("     Latency = %dns\n", singlesLatency);
  printf("     TopEnabled = %d, BotEnabled = %d\n", singlesTopEn, singlesBotEn);
  printf("     Emin = %dMeV, Emax = %dMeV\n", singlesEmin, singlesEmax);
  printf("     NHitsmin = %d\n", singlesNHitsmin);
  printf("   *** Pairs Configuration ***\n");
  printf("     Latency = %dns\n", pairsLatency);
  printf("     SumEnabled = %d, DiffEnabled = %d, CoplanarEnabled = %d, EDEnabled = %d\n", pairsSumEn, pairsDiffEn, pairsCoplanarEn, pairsEDEn);
  printf("     Coincidence = +/-%dns\n", pairsTimeCoincidence*4);
  printf("     SumMax = %dMeV\n", pairsSummax);
  printf("     DiffMax = %dMeV\n", pairsDiffmax);
  printf("     Emin = %dMeV, Emax = %dMeV\n", pairsEmin, pairsEmax);
  printf("     NHitsmin = %d\n", pairsNHitsmin);
  printf("     Coplanar Tolerance = +/-%ddegrees\n", pairsCoplanarTolerance);
  printf("     EDFactor = %dMeV/mm\n", pairsEDFactor);
  printf("     EDmin = %dMeV\n", pairsEDmin);
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
    "P2_LVDSOUT0",
    "P2_LVDSOUT1",
    "P2_LVDSOUT2",
    "P2_LVDSOUT3",
    "P2_LVDSOUT4",
    "P2_LVDSOUT5",
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
    "SD_SRC_SEL_TRIGGER0",
    "SD_SRC_SEL_TRIGGER1",
    "SD_SRC_SEL_TRIGGER2",
    "SD_SRC_SEL_TRIGGER3",
    "SD_SRC_SEL_TRIGGER4",
    "SD_SRC_SEL_TRIGGER5",
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
    "SSP_SER_FIBER2",
    "SSP_SER_FIBER3",
    "SSP_SER_FIBER4",
    "SSP_SER_FIBER5",
    "SSP_SER_FIBER6",
    "SSP_SER_FIBER7",
    "SSP_SER_VXS0",
    "SSP_SER_VXSGTP"
  };

#endif
