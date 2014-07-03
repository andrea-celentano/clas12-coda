
/* gtpLib.c */

#if defined(NIOS)

#include <sys/mman.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "gtpLib.h"

static Gtp_regs *pGtpRegPtr;
static int fdGtpMem = 0; 

static fd_set read_fds, write_fds, except_fds;
struct timeval timeout;
struct pollfd pollfdGtp;

unsigned int
gtpRead32(volatile unsigned int *addr)
{
  unsigned int rval;
  rval = *addr;
  return rval;
}

void
gtpWrite32(volatile unsigned int *addr, unsigned int val)
{
  *addr = val;
}

Gtp_regs *
gtpGetRegsPtr()
{
  int *pGtp = NULL;

  if(fdGtpMem > 0)
  {
  	close(fdGtpMem);
	fdGtpMem = 0;
  }

  fdGtpMem = open("/dev/uio0", O_RDWR);
  if(fdGtpMem < 1)
  {
    printf("ERROR: %s: failed to open /dev/uio0\n", __func__);
    return NULL;
  }

  /* Set isr data poll mode */ 
  pollfdGtp.fd = fdGtpMem;
  pollfdGtp.events = POLLIN;
	
  /* Set timeout */
  timeout.tv_sec = 0;
  timeout.tv_usec = 10000;


  pGtp = (int *)mmap(NULL, 0x10000, PROT_READ | PROT_WRITE, MAP_SHARED, fdGtpMem, (off_t)0);
  if(pGtp == MAP_FAILED)
  {
    printf("ERROR: %s: mmap /dev/uio0 Gtp Mem failed\n", __func__);
    pGtp = NULL;
    return NULL;
  }

  printf("INFO: %s: GTP mapped @ CPU address 0x%08X\n", __func__, (unsigned int)pGtp);

  return (Gtp_regs *)pGtp;
}

/*
  void gtpSetClock(int mode)
    mode: 0=disable, 1=vxs, 2=internal
*/
void
gtpSetClock(int mode)
{
  printf("INFO: %s: clock source = %d\n", __func__, mode);
  if(mode == 1)
  {
    gtpWrite32(&pGtpRegPtr->Clk.Ctrl, GTP_CLK_CTRL_P0	| GTP_CLK_CTRL_RESET);
    gtpWrite32(&pGtpRegPtr->Clk.Ctrl, GTP_CLK_CTRL_P0);
  }
  else if(mode == 2)
  {
    gtpWrite32(&pGtpRegPtr->Clk.Ctrl, GTP_CLK_CTRL_INT | GTP_CLK_CTRL_RESET);
    gtpWrite32(&pGtpRegPtr->Clk.Ctrl, GTP_CLK_CTRL_INT);
  }
  else
    gtpWrite32(&pGtpRegPtr->Clk.Ctrl, GTP_CLK_CTRL_RESET);

  usleep(10000);
  gtpGetClock();
}

void
gtpGetClock()
{
  int v;

  printf("INFO: %s\n", __func__);

  v = gtpRead32(&pGtpRegPtr->Clk.Ctrl);
  if((v & 0x3) == 1)
    printf("Gtp clock source = VXS\n");
  else if((v & 0x3) == 2)
    printf("Gtp clock source = Internal\n");
  else
  {
    printf("Gtp clock source = Disabled\n");
    return;
  }

  v = gtpRead32(&pGtpRegPtr->Clk.Status);
  if(v & 0x1) printf("  GCLK PLL Locked");
  else        printf("  ERROR: GCLK PLL not Locked");

  if(v & 0x2) printf("  ATX L PLL Locked");
  else        printf("  ERROR: ATX L PLL not Locked");

  if(v & 0x4) printf("  ATX R PLL Locked");
  else        printf("  ERROR: ATX R PLL not Locked");
}

/*
  void gtpSetSync(int mode)
    mode: GTP_SD_SRC_SEL_*
*/
void
gtpSetSync(int mode)
{
  printf("INFO: %s: sync source = %d\n", __func__, mode);
  gtpWrite32(&pGtpRegPtr->Sd.SrcSel[GTP_SD_SRC_SYNC], mode);
  gtpGetSync();
}

void
gtpGetSync()
{
  const char *src_names[] = {
    "GTP_SD_SRC_SEL_0",
    "GTP_SD_SRC_SEL_1",
    "GTP_SD_SRC_SEL_SYNC",
    "GTP_SD_SRC_SEL_TRIG1",
    "GTP_SD_SRC_SEL_TRIG2",
    "GTP_SD_SRC_SEL_LVDSIN",
    "GTP_SD_SRC_SEL_PULSER",
    "GTP_SD_SRC_SEL_BUSY",
    "GTP_SD_SRC_SEL_TRIGGER"
  };

  int v;

  printf("INFO: %s\n", __func__);

  v = gtpRead32(&pGtpRegPtr->Sd.SrcSel[GTP_SD_SRC_SYNC]) & 0x3F;
  printf("Gtp sync source = ");
  if(v < 4)
    printf("%s\n", src_names[v]);
  else if(v < 9)
    printf("%s%d\n", src_names[5], v-5);
  else if((v >= 9) && (v <= 10))
    printf("%s\n", src_names[v-9+6]);
  else
    printf("%s%d\n", src_names[8], v-32);
}

void
gtpPayloadTriggerStatus()
{
  int i, payload, status, errors, status2;
  const char *port_names[] = {
    "Slot10-PP1", "Slot13-PP2", "Slot9-PP3", "Slot14-PP4", "Slot8-PP5", "Slot15-PP6", "Slot7-PP7", "Slot16-PP8",
    "Slot6-PP9", "Slot17-PP10", "Slot5-PP11", "Slot18-PP12", "Slot4-PP13", "Slot19-PP14", "Slot3-PP15", "Slot20-PP16"
  };

  int slot_to_payload[] = {
    0,0,15,13,11,9,7,5,3,1,0,0,2,4,6,8,10,12,14,16
  };

  printf("gtpPayloadTriggerStatus()\n");
  printf("%-11s %-9s %-12s %-11s\n", "Port", "ChannelUp", "Latency(ns)", "Errors");

  for(i = 0; i < sizeof(slot_to_payload)/sizeof(slot_to_payload[0]); i++)
  {
    payload = slot_to_payload[i];
    if(payload)
    {
      status = gtpRead32(&pGtpRegPtr->Ser[payload-1].Status);
      errors = gtpRead32(&pGtpRegPtr->Ser[payload-1].ErrTile0);
      status2 = gtpRead32(&pGtpRegPtr->Ser[payload-1].Status2);
      printf("%-11s %-9d %-12d %-5d %-5d\n", port_names[payload-1], (status & 0x1000)>>12, (status2 & 0xFFFF)*4, errors & 0xFFFF, errors>>16);
    }
  }
}

void
gtpFiberTriggerStatus()
{
  int i, payload, status, errors, errors2;
  printf("gtpFiberTriggerStatus()\n");
  printf("%-11s %-9s %-22s\n", "Port", "ChannelUp", "Errors");

  status = gtpRead32(&pGtpRegPtr->QsfpSer.Status);
  errors = gtpRead32(&pGtpRegPtr->QsfpSer.ErrTile0);
  errors2 = gtpRead32(&pGtpRegPtr->QsfpSer.ErrTile1);
  printf("%-11s %-9d %-5d %-5d %-5d %-5d\n", "QSFP", (status & 0x1000)>>12, errors & 0xFFFF, errors>>16, errors2 & 0xFFFF, errors2>>16);
}

void
gtpFiberTriggerEnable()
{
  int i;

  printf("gtpFiberTriggerEnable\n");

  /* release resets */
  gtpWrite32(&pGtpRegPtr->QsfpSer.Ctrl, 0x0);

  /* delay to allow channel to lock */
  usleep(20000);

  /* enable error counters */
  gtpWrite32(&pGtpRegPtr->QsfpSer.Ctrl, 0x800);

  /* print status of ports */
  gtpPayloadTriggerStatus();
}

void
gtpFiberTriggerReset()
{
  int i;

  printf("gtpFiberTriggerReset payload\n");

  gtpWrite32(&pGtpRegPtr->QsfpSer.Ctrl, 0x401);
}

void
gtpPayloadTriggerEnable(int mask)
{
  int i;

  printf("gtpPayloadTriggerEnable payload 0x%04X\n", mask);

  /* release resets */
  for(i = 0; i < 16; i++)
  {
    if(!(mask & (1<<i)))
      continue;
    gtpWrite32(&pGtpRegPtr->Ser[i].Ctrl, 0x0);
  }

  /* delay to allow channel to lock */
  usleep(20000);

  /* enable error counters */
  for(i = 0; i < 16; i++)
  {
    if(!(mask & (1<<i)))
      continue;
    gtpWrite32(&pGtpRegPtr->Ser[i].Ctrl, 0x800);
  }

  /* enable payloads in trigger processing */
  gtpWrite32(&pGtpRegPtr->Trg.Ctrl, mask);

  /* print status of ports */
  gtpPayloadTriggerStatus();
}

void
gtpPayloadTriggerReset(int mask)
{
  int i;

  printf("gtpPayloadTriggerReset payload 0x%04X\n", mask);

  for(i = 0; i < 16; i++)
  {
    if(!(mask & (1<<i)))
      continue;
    gtpWrite32(&pGtpRegPtr->Ser[i].Ctrl, 0x401);
  }
}

/*
  void gtpSetHpsParameters(int pulseCoincidenceTicks, int pulseClusterThreshold)
    pulseCoincidenceTicks: 0-7=number of +/-4ns ticks to combine hits into a cluster
    pulseClusterThreshold: 0-8191=minimum threshold(MeV) to form a cluster
*/
void
gtpSetHpsParameters(int pulseCoincidenceTicks, int pulseClusterThreshold)
{
  printf("INFO: %s: ClusterPulseCoincidence window = +/-%dns, ClusterPulseThreshold = %dMeV\n", __func__, pulseCoincidenceTicks, 4*pulseClusterThreshold);
  gtpWrite32(&pGtpRegPtr->Trg.ClusterPulseCoincidence, pulseCoincidenceTicks);
  gtpWrite32(&pGtpRegPtr->Trg.ClusterPulseThreshold, pulseClusterThreshold);

  gtpGetHpsParameters();
}

void
gtpGetHpsParameters()
{
  int v;

  printf("INFO: %s\n", __func__);

  v = gtpRead32(&pGtpRegPtr->Trg.ClusterPulseCoincidence);
  printf("ClusterPulseCoincidence window = +/-%dns\n", v * 4);

  v = gtpRead32(&pGtpRegPtr->Trg.ClusterPulseThreshold);
  printf("ClusterPulseThreshold = %dMeV\n", v);
}

/*******************************************************************************
 *
 *  gtpIntAck
 *  - Acknowledge an interrupt or latched trigger.  This "should" effectively 
 *  release the "Busy" state of the TI-GTP.
 *  Execute a user defined routine, if it is defined.  Otherwise, use
 *  a default prescription.
 *
 */

void
gtpIntAck()
{
/*  int resetbits=0;*/
  if(pGtpRegPtr == NULL) {
    printf("gtpIntAck: ERROR: GTP not initialized\n");
    return;
  }

  /*printf("gtpIntAck reached\n");*/

  /* Send ack from GTP->TI over TiGtp link */
  gtpWrite32(&pGtpRegPtr->TiGtp.LinkCtrl, 0x2);
#if 0
  if (tiAckRoutine != NULL)
    {
      /* Execute user defined Acknowlege, if it was defined */
      TILOCK;
      (*tiAckRoutine) (tiAckArg);
      TIUNLOCK;
    }
  else
    {
      TILOCK;
      tiDoAck = 1;
      tiAckCount++;
      resetbits = TI_RESET_BUSYACK;

      if(!tiReadoutEnabled)
	{
	  /* Readout Acknowledge and decrease the number of available blocks by 1 */
	  resetbits |= TI_RESET_BLOCK_READOUT;
	}
      
      if(tiDoSyncResetRequest)
	{
	  resetbits |= TI_RESET_SYNCRESET_REQUEST;
	  tiDoSyncResetRequest=0;
	}

      vmeWrite32(&TIp->reset, resetbits);
      TIUNLOCK;
    }
#endif
}

void
gtpTiGtpLinkReset()
{
  int result;
  unsigned int val;

  if(pGtpRegPtr == NULL) {
    printf("gtpTiGtpLinkReset: ERROR: GTP not initialized\n");
    return;
  }

  gtpWrite32(&pGtpRegPtr->TiGtp.LinkReset, 0x7);
  gtpWrite32(&pGtpRegPtr->TiGtp.LinkReset, 0x5);
  usleep(20000);

  gtpWrite32(&pGtpRegPtr->TiGtp.LinkReset, 0x4);
  gtpWrite32(&pGtpRegPtr->TiGtp.LinkReset, 0x0);
  usleep(20000);

  /* will be called again in Go() transition */
  gtpTiGtpFifoReset();

  result = gtpRead32(&pGtpRegPtr->TiGtp.LinkStatus);
  if(!(result & 0x10000))
  {
    printf("gtpTiGtpLinkReset: ERROR: TiGtp not up!!!\n");
    return;
  }
}

void
gtpTiGtpFifoReset()
{
  gtpWrite32(&pGtpRegPtr->TiGtp.FifoCtrl, 0x1);
  gtpWrite32(&pGtpRegPtr->TiGtp.FifoCtrl, 0x0);
  
  gtpClearIntData();
}

int
gtpReadBlock(volatile unsigned int *data, int nwrds, int rflag)
{
  int len, ii, status1, status2;

  if(pGtpRegPtr == NULL)
  {
    printf("gtpReadBlock: ERROR: GTP not initialized\n");
    return;
  }

  status1 = gtpRead32(&pGtpRegPtr->TiGtp.FifoLenStatus);
  status2 = gtpRead32(&pGtpRegPtr->TiGtp.FifoDataStatus);
  len = gtpRead32(&pGtpRegPtr->TiGtp.FifoLen);
len++; /* TEMPORARY !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
  if(len > nwrds)
  {
    printf("gtpReadBlock ERROR: output buffer length %d words is not enough (need %d words)\n",nwrds,len);
    len = nwrds;
  }

  /*printf("statuses: 0x%08x 0x%08x (%d %d)\n",status1,status2,status1,status2);*/
  for(ii=0; ii<len; ii++)
  {
    *data++ = gtpRead32(&pGtpRegPtr->TiGtp.FifoData);
  }

  return(len);
}

unsigned int
gtpBReady()
{
  int result;

  if(pGtpRegPtr == NULL) {
    printf("gtpBReady: ERROR: GTP not initialized\n");
    return;
  }

  if( gtpRead32(&pGtpRegPtr->TiGtp.FifoLenStatus) & 0x1FF) return(1);

  return(0);
}

void
gtpClearIntData()
{
  int i, isr_num;
  for(i = 0; i < 1000; i++)
  {
    pollfdGtp.revents = 0;
    poll(&pollfdGtp, 1, 0);
    if(pollfdGtp.revents & POLLIN)
      read(fdGtpMem, &isr_num, sizeof(isr_num));
	 else
      break;
  }
}

int
gtpWaitForInt()
{
  int isr_num;

  /* Wait for input to become ready or until the time out(10ms) */
  pollfdGtp.revents = 0;
  poll(&pollfdGtp, 1, 10);
  if(pollfdGtp.revents & POLLIN)
  {
    read(fdGtpMem, &isr_num, sizeof(isr_num)); /*need it ???*/
    return(1); /* fd is ready for reading */
  }
  return(0);/* timeout or error */
}

void
gtpEnableInt(int en)
{
  write(fdGtpMem, &en, 4);
}

int
gtpInit(int flag)
{
  int v;

  pGtpRegPtr = gtpGetRegsPtr();
  if(!pGtpRegPtr) return -1;

  v = gtpRead32(&pGtpRegPtr->Cfg.BoardId);
  if(v != GTP_BOARDID)
  {
     printf("ERROR: %s: invalid board id = 0x%08X\n", v);
     return -1;
  }
  printf("Gtp board id = 0x%08X\n", v);

  v = gtpRead32(&pGtpRegPtr->Cfg.FirmwareRev);
  printf("Gtp firmware revision = %d.%d, type = ", (v>>0)&0xFF, (v>>8)&0xFF);
  if((v>>16) == GTP_TYPE_HALLD)
    printf("HallD\n");
  else if((v>>16) == GTP_TYPE_HPS)
    printf("HPS\n");
  else
  {
    printf("***** Unknown *****\nERROR: %s: unknown GTP firmware type\n", __func__);
    return -1;
  }
  
  gtpEnableInt(0);
}

#else /* dummy version*/

void
gtpLib_dummy()
{
  return;
}

#endif
