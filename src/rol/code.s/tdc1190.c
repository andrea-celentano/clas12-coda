#if defined(VXWORKS) || defined(Linux_vme)

/*****************************************************************************
 *
 *  tdc1190.c - Driver library for readout of C.A.E.N. Model 1190
 *                  TDC using a VxWorks 5.2 or later based Single Board computer.
 *
 *  Author: David Abbott 
 *          Jefferson Lab Data Acquisition Group
 *          August 2002
 *
 *  Revision  1.0 - Initial Revision
 *                    - Supports up to 20 CAEN Model 1190s in a Crate
 *                    - Programmed I/O reads
 *                    - Interrupts from a Single 1190
 *
 *  adjusted for CLAS by Sergey Boyarinov - May 2003-present
 *
 *  Ported to Linux by Bryan Moffit - Jan 2010
 *
 *    - Linked List DMA supported
 *       vxworks: requires modified tsi148 DMA library 
 *
 * SVN: $Rev: 416 $
 *
 */


#include <stdio.h>
#include <string.h>
#include <pthread.h>
#ifdef VXWORKS
#include <vxWorks.h>
#include <logLib.h>
#include <taskLib.h>
#include <intLib.h>
#include <iv.h>
#include <vxLib.h>
#else
#include "jvme.h"
#endif

/*
#ifdef Linux_vme
#ifndef VOIDFUNCPTR
typedef void (*VOIDFUNCPTR) ();
#endif
#endif
*/

#include "tdc1190.h"


/* Prototype of Common Initialization of Modules */
STATUS tdc1190CommonInit(int itdc, UINT32 laddr);

/* Macro to check id and c1190p */
#define CHECKID(id) { \
    if((id<0) || (c1190p[id] == NULL)) { \
      logMsg("%s: ERROR : TDC id %d not initialized \n", \
	     (int)__FUNCTION__,id,3,4,5,6);	\
      return ERROR;	\
    }  \
    if(use1190[id]==0) \
    { \
	  logMsg("%s: TDC id %d flagged to NOT be used.\n", \
	       (int)__FUNCTION__,id,3,4,5,6); \
	  return ERROR; \
    } \
  }

#ifdef VXWORKS
/* Define external Functions */
IMPORT  STATUS sysBusToLocalAdrs(int, char *, char **);
IMPORT  STATUS intDisconnect(int);
IMPORT  STATUS sysIntEnable(int);
IMPORT  STATUS sysIntDisable(int);
#endif

#ifdef VXWORKS
#define EIEIO    __asm__ volatile ("eieio")
#define SynC     __asm__ volatile ("sync")
#else
#define EIEIO    
#define SynC     
#endif

/* Mutex to hold of reads and writes from competing threads */
pthread_mutex_t c1190_mutex = PTHREAD_MUTEX_INITIALIZER;
#define LOCK_1190 {				\
    if(pthread_mutex_lock(&c1190_mutex)<0)	\
      perror("pthread_mutex_lock");	\
  }
#define UNLOCK_1190 {				\
    if(pthread_mutex_unlock(&c1190_mutex)<0)	\
      perror("pthread_mutex_unlock");	\
  }

/* Define global variables */

unsigned int tdcAddrOffset = 0;                /* Difference in CPU (USERSPACE) Base */

int Nc1190 = 0;                              /* Number of TDCs in crate */

volatile struct v1190_struct *c1190p[V1190_MAX_MODULES];    /* pointers to TDC memory map */
static unsigned int c1190vme[V1190_MAX_MODULES]; /* jumper addresses for Dma */

static int use1190[V1190_MAX_MODULES] = {           /* Switch to turn on (1) or off (0) specific TDCs */
  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0};

volatile struct v1190_struct *c1190MCSTp;    /* pointer to TDC MCST memory map */
int tdcMaxSlot=-1;                           /* Highest element in use1190 hold an TDC */
int tdcMinSlot=-1;                           /* Lowest element in use1190 holding an TDC */

/* Define Interrupts variables */
BOOL              c1190IntRunning = FALSE; /* running flag */
int               c1190IntID      = -1; /* id number of TDC generating interrupts */
LOCAL VOIDFUNCPTR c1190IntRoutine = NULL; /* user interrupt service routine */
LOCAL int         c1190IntArg     = 0; /* arg to user routine */
LOCAL int         c1190IntEvCount = 0; /* Number of Events to generate Interrupt */
LOCAL UINT32      c1190IntLevel   = V1190_VME_INT_LEVEL; /* default VME interrupt level */
LOCAL UINT32      c1190IntVec     = V1190_INT_VEC; /* default interrupt Vector */
int               c1190IntCount   = 0; /* Count of interrupts from TDC */


/*Sergey*/

static int active;

/* window parameters */
static int window_width  = 4600;
static int window_offset = -4000;
static int window_extra  = 25;
static int window_reject = 50;

/* readout options */
static int cblt_not, a24_a32, sngl_blt_mblt, berr_fifo, sst_rate;
static int blt_Events, n_Hits, almostFullLevel, outProgControl;

/* variable filled by DmaStart and used by DmaDone */
static int nbytes_save[21];
/*Sergey*/

/*S.P.*/
static TDC1290_CONF tdc[NBOARD];
/*S.P.*/


/*******************************************************************************
 *
 * tdc1190Init - Initialize tdc1190 Library. 
 *
 *
 * RETURNS: the number of tdc boards detected, or ERROR if the address is invalid
 *   or board is not present.
 */

int
tdc1190Init(UINT32 addr, UINT32 addr_inc, int ntdc, int iFlag)
{
  int ii, jj, itdc, res=0, errFlag = 0;
  unsigned short rdata=0;
  int boardID = 0;
  unsigned int laddr;
  volatile struct v1190_ROM_struct *rp;
  char boardmodel[2][6] = {"v1190","v1290"};

  /* Check for valid address */
  if(addr==0) 
  {
    printf("tdc1190Init: ERROR: Must specify a Bus (VME-based A32/A24) address for TDC 0\n");
    return(ERROR);
  }
  else if(addr < 0x00ffffff) /* A24 Addressing */
  {
    if((addr_inc==0)||(ntdc==0)) ntdc = 1; /* assume only one TDC to initialize */

    /* get the TDCs address */
#ifdef VXWORKS
    res = sysBusToLocalAdrs(0x39,(char *)addr,(char **)&laddr);
    if (res != 0) 
	{
	  printf("tdc1190Init: ERROR in sysBusToLocalAdrs(0x39,0x%x,&laddr) \n",addr);
	  return(ERROR);
	}
    tdcAddrOffset = (unsigned int)laddr-(unsigned int)addr;
#else
    res = vmeBusToLocalAdrs(0x39,(char *)addr,(char **)&laddr);
    if (res != 0) 
	{
	  printf("tdc1190Init: ERROR in vmeBusToLocalAdrs(0x39,0x%x,&laddr) \n",addr);
	  return(ERROR);
	}
    tdcAddrOffset = (unsigned int)laddr-(unsigned int)addr;
#endif
  }
  else /* A32 Addressing */
  {
    if((addr_inc==0)||(ntdc==0)) ntdc = 1; /* assume only one TDC to initialize */

    /* get the TDC address */
#ifdef VXWORKS
    res = sysBusToLocalAdrs(0x09,(char *)addr,(char **)&laddr);
    if (res != 0) 
	{
	  printf("tdc1190Init: ERROR in sysBusToLocalAdrs(0x09,0x%x,&laddr) \n",addr);
	  return(ERROR);
	}
    tdcAddrOffset = (unsigned int)laddr-(unsigned int)addr;
#else
    res = vmeBusToLocalAdrs(0x09,(char *)addr,(char **)&laddr);
    if (res != 0) 
	{
	  printf("tdc1190Init: ERROR in vmeBusToLocalAdrs(0x09,0x%x,&laddr) \n",addr);
	  return(ERROR);
	}
    tdcAddrOffset = (unsigned int)laddr-(unsigned int)addr;
#endif
  }

  /*sergey: ii -> Nc1190 in two places below*/
  Nc1190 = 0;
  for(ii=0; ii<ntdc; ii++) 
  {
    if(tdc1190CommonInit(Nc1190/*ii*/,laddr + ii*addr_inc) == OK)
	{
      c1190vme[Nc1190/*ii*/] = /*0x08000000 +*/ addr + ii*addr_inc;
      Nc1190++;
      itdc = Nc1190-1;
      printf("Initialized TDC %s with ID=%d, DMA address 0x%08x, USER address 0x%08x in SLOT=%d\n",
		boardmodel[use1190[itdc]-1],itdc,(UINT32)c1190vme[itdc],(UINT32)c1190p[itdc],tdc1190Slot(itdc));
	}
	else
	{
      /*break;*/
	}
  }

  /* Disable/Clear all TDCs */
  for (ii=0; ii<Nc1190; ii++) 
  {
    if(!use1190[ii]) continue;
      
    /* Determine the first TDC for MCST or CBLT */
    if (tdcMinSlot==-1) tdcMinSlot = ii;

    /* Determine the last TDC for MCST or CBLT */
    tdcMaxSlot = ii;

    vmeWrite16(&(c1190p[ii]->moduleReset),1);
    vmeWrite16(&(c1190p[ii]->clear),1);
  }

  printf("Fount %d tdc1190 or/and tdc1290 boards\n",Nc1190);

  return(Nc1190);
}


/*******************************************************************************
 *
 * tdc1190CommonInit - Called by tdc1190Init or tdc1190InitList.
 *                     Initialization of individual modules, checking
 *                     for a valid VME address and BOARD_ID and 
 *                     Firmware Version.
 *
 * RETURNS: OK, or ERROR if the address is invalid or board is not present.
 */

STATUS
tdc1190CommonInit(int itdc, UINT32 laddr)
{
  int ii, res=0, errFlag = 0;
  unsigned short rdata=0;
  int boardID = 0;
  volatile struct v1190_ROM_struct *rp;

  c1190p[itdc] = (struct v1190_struct *)(laddr);

  /* Check if Board exists at that address */
#ifdef VXWORKS
  res = vxMemProbe((char *) &(c1190p[itdc]->firmwareRev),0,2,(char *)&rdata);
#else
  res = vmeMemProbe((char *) &(c1190p[itdc]->firmwareRev),2,(char *)&rdata);
#endif
  if(res < 0 )
  /*       if(res < 0 || rdata==0xffff)  */
  {
    printf("tdc1190CommonInit: ERROR: No addressable board at addr=0x%x\n",
        (UINT32) c1190p[itdc]);
    c1190p[itdc] = NULL;
    use1190[itdc] = 0;
    errFlag = 1;
    return ERROR;
  } 
  else 
  {
    /* Check if this is a Model 1190/1290 */
    rp = (struct v1190_ROM_struct *)((UINT32)c1190p[itdc] + V1190_ROM_OFFSET);
    boardID = ((vmeRead16(&(rp->board2))&(0xff))<<16) +
	  ((vmeRead16(&(rp->board1))&(0xff))<<8) +
	  (vmeRead16(&(rp->board0))&(0xff));
    if(((boardID&0xffff) != V1190_BOARD_ID) && ((boardID&0xffff) != V1290_BOARD_ID)) 
	{
	  printf("tdc1190CommonInit: ERROR: Board ID does not match: 0x%x \n",boardID);
	  use1190[itdc] = 0;
	  return ERROR;
	}
    else
	{
      printf(">>> Found board with BOARD_ID=0x%08x\n",boardID);
	}

    /* Check if this is the firmware we expect V1190_FIRMWARE_REV or V1190_FIRMWARE_REV+1 */
    if( (rdata != V1190_FIRMWARE_REV) && (rdata != (V1190_FIRMWARE_REV+1)) )
	{
	  printf("WARN: Firmware does not match: 0x%08x (expected 0x%08x) (laddr=0x%08x)\n",
			 rdata,V1190_FIRMWARE_REV, laddr);
      return ERROR;
	} 
  }
  if((boardID&0xffff) == V1190_BOARD_ID)  use1190[itdc] = 1;  /* 1 for v1190 */
  else                                    use1190[itdc] = 2;  /* 2 for v1290 */

  return OK;
}



/*Sergey start*/

/* set global variables which will be used to program all boards in crate */
void
tdc1190SetDefaultWindowWidth(int width)
{
  window_width = width;
  return;
}

void
tdc1190SetDefaultWindowOffset(int offset)
{
  window_offset = offset;
  return;
}


STATUS
tdc1190Config(char *fname)
{
  return(tdc1290Config(fname));
}



STATUS
tdc1190Config_obsolete(int options)
{
  int ii, ifull;
  unsigned short value, value0, value1, array0[32], array1[32], data16;
  unsigned short dat16[21][10];
  short tmp[5], tdata;
  unsigned int ww, wm, wr;
  int wo;

  /* get DMA settings */
  usrVmeDmaGetConfig(&a24_a32, &sngl_blt_mblt, &sst_rate);

  /* unpack options */
  berr_fifo = (options)&0xFF;
  cblt_not = 0x0;

  if(sngl_blt_mblt==0x01)
  {
    printf("  D32 single word readout\n");
  }
  else if(sngl_blt_mblt==0x02)
  {
    printf("  D32 DMA (BLT) readout\n");
  }
  else if(sngl_blt_mblt==0x03)
  {
    printf("  D64 DMA (MBLT) readout\n");
  }
  else if(sngl_blt_mblt==0x04)
  {
    printf("  D64 DMA (2eVME) readout\n");
  }
  else if(sngl_blt_mblt==0x05)
  {
    printf("  D64 DMA (2eSST) readout\n");
  }
  else
  {
    printf("  unknown readout mode, use D32 single word readout\n");
    sngl_blt_mblt = 0x01;
  }

  if(berr_fifo==0x00)
  {
    printf("  DMA Nwords method: use VME BUS error\n");
  }
  else if(berr_fifo==0x01)
  {
    printf("  DMA Nwords method: use event fifo\n");
  }
  else
  {
    printf("  unknown DMA Nwords method, use VME BUS error\n");
    berr_fifo = 0x00;
  }

  printf("start v1190/v1290 initialization\n");

  /* general initialization */
  /*for(ii=0; ii<Nc1190; ii++) tdc1190Init(ii,0,1,0);*/
  for(ii=0; ii<Nc1190; ii++) tdc1190Clear(ii);
  for(ii=0; ii<Nc1190; ii++) tdc1190Reset(ii);

  /* Program TDC for trigger matching mode */
  for(ii=0; ii<Nc1190; ii++)
  {
    tdc1190SetTriggerMatchingMode(ii);
  }


  /*tdc1190ReadAcquisitionMode(ii,&data16);*/
  for(ii=0; ii<Nc1190; ii++)
  {
    tdc1190WriteMicro(ii,0x0200);
  }
  for(ii=0; ii<Nc1190; ii++)
  {
    tdc1190ReadMicro(ii,&value,1);
    value &= 0x1;
    if(value==0)
      printf("Acquisition Mode: Continuous Storage\n");
    else
      printf("Acquisition Mode: Trigger Matching\n");
  }





  /* Set Trigger Window Width (ns) */
  ww = window_width;
  for(ii=0; ii<Nc1190; ii++)
  {
    /*tdc1190SetWindowWidth(ii, ww);*/
    tdc1190WriteMicro(ii,0x1000);
  }
  for(ii=0; ii<Nc1190; ii++)
  {
    if(ww < 25)         value = 1;
    else if(ww > 51175) value = 0x7FF;
    else                value = ww/25;
    printf("Set Window Width to %d ns\n",value*25);
    tdc1190WriteMicro(ii,value);
  }





  /* Set Trigger Window Offset (ns) */
  wo = window_offset;;
  for(ii=0; ii<Nc1190; ii++)
  {
    /*tdc1190SetWindowOffset(ii, wo);*/
    tdc1190WriteMicro(ii,0x1100);
  }
  for(ii=0; ii<Nc1190; ii++)
  {
    if(wo < -800000) tdata = -32000;
    else             tdata = wo/25;
    printf("Set Window Offset to %d ns\n",tdata*25);
    tdc1190WriteMicro(ii,tdata);
  }



  /* extra search margin (after window) (ns) */
  wm = 25;
  for(ii=0; ii<Nc1190; ii++)
  {
    /*tdc1190SetExtraSearchMargin(ii, wm);*/
    tdc1190WriteMicro(ii,0x1200);
  }
  for(ii=0; ii<Nc1190; ii++)
  {
    if(wm < 0)         value = 0;
    else if(wm > 1250) value = 50;
    else               value = wm/25;
    printf("Set Extra Search Margin to %d ns\n",value*25);
    tdc1190WriteMicro(ii,value);
  }



  /* reject margin (before window) (ns) */
  wr = 50;
  for(ii=0; ii<Nc1190; ii++)
  {
    /*tdc1190SetRejectMargin(ii, wr);*/
    tdc1190WriteMicro(ii,0x1300);
  }
  for(ii=0; ii<Nc1190; ii++)
  {
    if(wr < 0)         value = 0;
    else if(wr > 1250) value = 50;
    else               value = wr/25;
    printf("Set Reject Margin to %d ns\n",value*25);
    tdc1190WriteMicro(ii,value);
  }



  /* Enable subtraction of trigger time */
  for(ii=0; ii<Nc1190; ii++)
  {
    tdc1190EnableTriggerTimeSubtraction(ii);
  }




  /*tdc1190ReadTriggerConfiguration(ii);*/
  for(ii=0; ii<Nc1190; ii++)
  {
    tdc1190WriteMicro(ii,0x1600);
  }
  for(ii=0; ii<Nc1190; ii++)
  {
    /*tdc1190ReadMicro(ii,tmp,5);*/
    tdc1190ReadMicro(ii,&tmp[0],1);
    tdc1190ReadMicro(ii,&tmp[1],1);
    tdc1190ReadMicro(ii,&tmp[2],1);
    tdc1190ReadMicro(ii,&tmp[3],1);
    tdc1190ReadMicro(ii,&tmp[4],1);
    printf("  Window Width              = %6d ns\n",tmp[0]*25);
    printf("  Window Offset             = %6d ns\n",tmp[1]*25);
    printf("  Extra Seach Margin        = %6d ns\n",tmp[2]*25);
    printf("  Reject Margin             = %6d ns\n",tmp[3]*25);
    printf("  Trigger Time Subtruction = %6d\n",tmp[4]);
  }




  /* edge detection: */
  /* 0-pair, 1-trailing, 2-leading, 3-both leading and trailing */
  value1=2;
  for(ii=0; ii<Nc1190; ii++)
  {
    /*tdc1190SetEdgeDetectionConfig(ii, value1);*/
    value = value1 & 0x3;
    printf("SetEdgeDetectionConfig: ");
    if(value==0x0)
      printf("set pair mode\n");
    else if(value==0x1)
      printf("set trailing edge only\n");
    else if(value==0x2)
      printf("set leading edge only\n");
    else
      printf("set both leading and trailing edges\n");
    tdc1190WriteMicro(ii,0x2200);
  }
  for(ii=0; ii<Nc1190; ii++)
  {
    tdc1190WriteMicro(ii,value);
  }




    /* get edge detection configuration */
  for(ii=0; ii<Nc1190; ii++)
  {
    /*tdc1190ReadEdgeDetectionConfig(ii,&data16);*/
    tdc1190WriteMicro(ii,0x2300);
  }
  for(ii=0; ii<Nc1190; ii++)
  {
    tdc1190ReadMicro(ii,&value,1);
    value &= 0x3;
    printf("ReadEdgeDetectionConfig: value=%d (0x%x)\n",value,value);
    if(value==0)
      printf("ReadEdgeDetectionConfig: paired measurement\n");
    else if(value==1)
      printf("ReadEdgeDetectionConfig: trailing edge\n");
    else if(value==2)
      printf("ReadEdgeDetectionConfig: leading edge\n");
    else if(value==3)
      printf("ReadEdgeDetectionConfig: both leading and trailing\n");
    else
    {
      printf("ReadEdgeDetectionConfig: ERROR, value=%d (0x%x)\n",value,value);
    }
  }



  /********************/
  /* readout settings */

  if(berr_fifo == 0x01)
  {
    /* enable event fifo */
    for(ii=0; ii<Nc1190; ii++)
    {
      tdc1190EventFifo(ii,1);
    }
  }
  else
  {
    /* disable event fifo */
    for(ii=0; ii<Nc1190; ii++)
    {
      tdc1190EventFifo(ii,0);
    }
  }

  for(ii=0; ii<Nc1190; ii++)
  {
    ifull = tdc1190StatusFull(ii);
    printf("ifull=%d\n",ifull);

    printf("end v1190\n");
  }


  /* if using event fifo, disable bus error on BLT finish */
  if(berr_fifo == 0x01)
  {
    printf("disable VME BUS error for event fifo readout\n");
    for(ii=0; ii<Nc1190; ii++)
    {
      tdc1190BusError(ii,0);
	}
  }
  else
  {
    printf("enable VME BUS error\n");
    for(ii=0; ii<Nc1190; ii++)
    {
      tdc1190BusError(ii,1);
    }
  }

  /* enable 64-bit alignment */
  /* NOTE: 64-bit alignment must be enabled for any DMA readout,
  not only for MBLT(D64) but for BLT(D32) as well; some DMA engines
  (for example the one on mv5100) will cut off last 32-bit word if
  it is not even, for example if event size is equal to 137 D32 words,
  only 136 D32 words will be transfered */
  /* in addition universe library contains check for 64-bit alignment
	 in dmastart procedure, it will return error if ... */
  for(ii=0; ii<Nc1190; ii++)
  {
    tdc1190Align64(ii,1);
  }

  /* set BLT Event Number Register */
  for(ii=0; ii<Nc1190; ii++)
  {
    tdata = 1;
    tdc1190SetBLTEventNumber(ii, tdata);
    tdata = tdc1190GetBLTEventNumber(ii);
    printf("BLT Event Number set to %d\n",tdata);
  }


  /* set maximum number of hits per event */
  for(ii=0; ii<Nc1190; ii++)
  {
    tdata = 64;
    tdc1190SetMaxNumberOfHitsPerEvent(ii, tdata);
    tdata = tdc1190GetMaxNumberOfHitsPerEvent(ii);
  }
  

  /* reset MCST flag in every board in case if it was set before */
  for(ii=0; ii<Nc1190; ii++)
  {
    tdc1190ResetMCST(ii);
  }

  /* set 'almost full' level and program output connector to signal on it */
  for(ii=0; ii<Nc1190; ii++)
  {
    tdc1190SetAlmostFullLevel(ii, 16384/*8*//*32735*/); /* default is 64 words, max is 32735 words */
    tdata = tdc1190GetAlmostFullLevel(ii);

    tdc1190SetOutProg(ii,2);/*0-data ready,1-full,2-almost full,3-err*/
    tdata = tdc1190GetOutProg(ii);
  }

  /*
  fflush(stdout);
  taskDelay(10);
  */

  return;
}

/*Sergey end*/



int
tdc1190Type(int id)
{
  if(id>=0 && id<Nc1190) return(use1190[id]);

  printf("%s: ERROR: TDC with id %d does not exist.\n",__FUNCTION__,id);
  return(ERROR);
}

/*****************************************************************************/
/*****************************************************************************/
/*S.P.start*/

int
tdc1190Slot(int id)
{
  if(id>=0 && id<Nc1190)
    return(tdc1190GetGeoAddress(id));

  printf("%s: ERROR: TDC with id %d does not exist.\n",__FUNCTION__,id);
  return(ERROR);
}

int
tdc1190Id(int slot)
{
  int id;

  for(id=0; id<Nc1190; id++)
    if(tdc1190GetGeoAddress(id)==slot)
      return(id);

  printf("%s: ERROR: TDC in slot %d does not exist or not initialized.\n",__FUNCTION__,slot);
  return(ERROR);
}

int
tdc119GetBoardID(int id)
{
  int rval;
  volatile struct v1190_ROM_struct *rp;

  CHECKID(id);
  rp = (struct v1190_ROM_struct *)((UINT32)c1190p[id] + V1190_ROM_OFFSET);
  LOCK_1190;
  rval = ((vmeRead16(&(rp->board1))&(0xff))<<8) | (vmeRead16(&(rp->board0))&(0xff));
  UNLOCK_1190;

  return(rval);
}

int
tdc119GetBoardRev(int id)
{
  int rval;
  volatile struct v1190_ROM_struct *rp;

  CHECKID(id);
  rp = (struct v1190_ROM_struct *)((UINT32)c1190p[id] + V1190_ROM_OFFSET);
  LOCK_1190;
  /*
  printf("  rp->revis3 addr=0x%08x  val=0x%04x \n", &(rp->revis3), *(&(rp->revis3)));
  printf("  rp->revis2 addr=0x%08x  val=0x%04x \n", &(rp->revis2), *(&(rp->revis2)));
  printf("  rp->revis1 addr=0x%08x  val=0x%04x \n", &(rp->revis1), *(&(rp->revis1)));
  printf("  rp->revis0 addr=0x%08x  val=0x%04x \n", &(rp->revis0), *(&(rp->revis0)));
  */
  rval = ((vmeRead16(&(rp->revis3))&(0xff))<<24) |
	 ((vmeRead16(&(rp->revis2))&(0xff))<<16) |
	 ((vmeRead16(&(rp->revis1))&(0xff))<<8)  |
	  (vmeRead16(&(rp->revis0))&(0xff));
  UNLOCK_1190;

  return(rval);
}

int
tdc119GetSerialNumber(int id)
{
  int rval;
  volatile struct v1190_ROM_struct *rp;

  CHECKID(id);
  rp = (struct v1190_ROM_struct *)((UINT32)c1190p[id] + V1190_ROM_OFFSET);
  LOCK_1190;
  /*
  printf("  rp->sernum1 addr=0x%08x  val=0x%04x \n", &(rp->sernum1), *(&(rp->sernum1)));
  printf("  rp->sernum2 addr=0x%08x  val=0x%04x \n", &(rp->sernum2), *(&(rp->sernum2)));
  */
  rval = ((vmeRead16(&(rp->sernum1))&(0xff))<<8) + (vmeRead16(&(rp->sernum2))&(0xff));
  UNLOCK_1190;

  return(rval);
}

#define SSWAP(x)        ((((x) & 0x00ff) << 8) | \
                         (((x) & 0xff00) >> 8))
/*
to see all ROM from checksum to sernum2:
      tcpClient rocbcal1 "tdc1190PrintROM(0x4000,34)"
*/
void
tdc1190PrintROM(UINT32 addr_add, int loop)
{
  volatile unsigned short *addr;
  unsigned short rval;
  int jj;

  if (loop<=0) loop=0;

  addr = (unsigned short *)((UINT32)c1190p[0] + addr_add);

  LOCK_1190;

  for(jj=0; jj<loop; jj++)
  {
    rval = *addr;
    rval = SSWAP(rval);
    printf("  addr=0x%08x  rval=0x%02x \n", addr, rval);
    addr += 2;
  }
  UNLOCK_1190;
}

/* channel_mask[0] - low 16 channels */
STATUS
tdc1290EnableChannels(int id, UINT16 channel_mask[2])
{
  CHECKID(id);
  tdc1190WriteMicro(id,0x4400);
  tdc1190WriteMicro(id,channel_mask[0]);
  tdc1190WriteMicro(id,channel_mask[1]);
  return(OK);
}

STATUS
tdc1290GetChannels(int id, UINT16 channels[2])
{
  int i;
  CHECKID(id);
  tdc1190WriteMicro(id,0x4500);
  for(i=0; i<2; i++) tdc1190ReadMicro(id,&channels[i],1);
  return(OK);
}

/*****************************************************************************/

void
tdc1290InitGlobals()
{
  int ii, jj;

  cblt_not      = 0;
  berr_fifo     = 1;

  blt_Events      = 1;
  n_Hits          = 64;
  almostFullLevel = 16384;
  outProgControl  = 2;

  window_width  = 750;
  window_offset = -1750;
  window_extra  = 25;
  window_reject = 50;

  for(ii=0; ii<NBOARD; ii++)
  {
    tdc[ii].group = 0;
    tdc[ii].edge  = 2;
    for(jj=0; jj<8; jj++) tdc[ii].mask[jj] = 0xffff;
  }

  /* obtain CPU DMA setting information */
  usrVmeDmaGetConfig(&a24_a32, &sngl_blt_mblt, &sst_rate);
  /*
  a24_a32       = 2;
  sngl_blt_mblt = 3;
  sst_rate      = 0;
  */

  return;
}


#define SCAN_MSK \
	args = sscanf (str_tmp, "%*s %d %d %d %d %d %d %d %d   \
                                     %d %d %d %d %d %d %d %d", \
		       &msk[ 0], &msk[ 1], &msk[ 2], &msk[ 3], \
		       &msk[ 4], &msk[ 5], &msk[ 6], &msk[ 7], \
		       &msk[ 8], &msk[ 9], &msk[10], &msk[11], \
		       &msk[12], &msk[13], &msk[14], &msk[15])


int
tdc1290ReadConfigFile(char *filename)
{
  FILE   *fd;
  char   fname[FNLEN] = { "" };  /* config file name */
  int    ii, jj, ch, gr = 0;
  char   str_tmp[STRLEN], keyword[ROCLEN];
  char   host[ROCLEN], ROC_name[ROCLEN];
  char   str2[2];
  int    args, i1, msk[NCHAN];
  int    slot, chan;
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
      sprintf(fname, "%s/tdc1190/%s", clonparms, filename);
	}

    if((fd=fopen(fname,"r")) == NULL)
    {
      printf("\nReadConfigFile: Can't open config file >%s<\n",fname);
      return(-1);
    }
  }
  else /* filename does not specified */
  {
    /* obtain our hostname */
    gethostname(host,ROCLEN);
    sprintf(fname, "%s/tdc1190/%s.cnf", clonparms, host);
    if((fd=fopen(fname,"r")) == NULL)
    {
      sprintf(fname, "%s/tdc1190/%s.cnf", clonparms, expid);
      if((fd=fopen(fname,"r")) == NULL)
      {
        printf("\nReadConfigFile: Can't open config file >%s<\n",fname);
        return(-2);
	  }
	}

  }
  printf("\nReadConfigFile: Using configuration file >%s<\n",fname);

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
      if((strcmp(keyword,"TDC1190_CRATE")==0)||(strcmp(keyword,"TDC1290_CRATE")==0))
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

      else if(active && ((strcmp(keyword,"TDC1190_ALLSLOTS")==0)||(strcmp(keyword,"TDC1290_ALLSLOTS")==0)))
      {
	    gr++;
	    for(ii=0; ii<NBOARD; ii++)  tdc[ii].group = gr;
      }

      else if(active && ((strcmp(keyword,"TDC1190_SLOT")==0)||(strcmp(keyword,"TDC1290_SLOT")==0)||
			   (strcmp(keyword,"TDC1190_SLOTS")==0)||(strcmp(keyword,"TDC1290_SLOTS")==0)))
      {
	    gr++;
	    SCAN_MSK;
	    printf("\nReadConfigFile: gr = %d     args = %d \n",gr,args);

	    for(ii=0; ii<args; ii++)
	    {
	      slot = msk[ii];
	      if(slot<1 || slot>21)
	      {
	        printf("\nReadConfigFile: Wrong slot number %d, %s\n",slot,str_tmp);
	        return(-4);
	      }
	      tdc[slot].group = gr;
	    }
      }

      else if(active && ((strcmp(keyword,"TDC1190_A24_A32")==0)||(strcmp(keyword,"TDC1290_A24_A32")==0)))
      {
        sscanf (str_tmp, "%*s %d %d", &i1);
	    if(i1!=1 && i1!=2)
	    {
	      printf("\nReadConfigFile: Wrong DMA addrType %d, %s\n",i1,str_tmp);
	      return(-5);
	    }
	    a24_a32 = i1;
      }

      else if(active && ((strcmp(keyword,"TDC1190_SNGL_BLT")==0)||(strcmp(keyword,"TDC1290_SNGL_BLT")==0)))
      {
        sscanf (str_tmp, "%*s %d %d", &i1);
	    if(i1<1 && i1>5)
	    {
	      printf("\nReadConfigFile: Wrong DMA dataType %d, %s\n",i1,str_tmp);
	      return(-6);
	    }
	    sngl_blt_mblt = i1;
      }

      else if(active && ((strcmp(keyword,"TDC1190_SST_RATE")==0)||(strcmp(keyword,"TDC1290_SST_RATE")==0)))
      {
        sscanf (str_tmp, "%*s %d %d", &i1);
	    if(i1!=0 && i1!=1)
	    {
	      printf("\nReadConfigFile: Wrong DMA sstMode %d, %s\n",i1,str_tmp);
	      return(-7);
	    }
	    sst_rate = i1;
      }

      else if(active && ((strcmp(keyword,"TDC1190_BERR_FIFO")==0)||(strcmp(keyword,"TDC1290_BERR_FIFO")==0)))
      {
        sscanf (str_tmp, "%*s %d %d", &i1);
	    if(i1!=0 && i1!=1)
	    {
	      printf("\nReadConfigFile: Wrong DMA Nwords method %d, %s\n",i1,str_tmp);
	      return(-8);
	    }
	    berr_fifo = i1;
      }

      else if(active && ((strcmp(keyword,"TDC1190_BLT_EVENTS")==0)||(strcmp(keyword,"TDC1290_BLT_EVENTS")==0)))
      {
        sscanf (str_tmp, "%*s %d %d", &i1);
	    blt_Events= i1;
      }

      else if(active && ((strcmp(keyword,"TDC1190_N_HITS")==0)||(strcmp(keyword,"TDC1290_N_HITS")==0)))
      {
        sscanf (str_tmp, "%*s %d %d", &i1);
	    n_Hits= i1;
      }

      else if(active && ((strcmp(keyword,"TDC1190_ALMOSTFULL")==0)||(strcmp(keyword,"TDC1290_ALMOSTFULL")==0)))
      {
        sscanf (str_tmp, "%*s %d %d", &i1);
	    almostFullLevel= i1;
      }

      else if(active && ((strcmp(keyword,"TDC1190_OUT_PROG")==0)||(strcmp(keyword,"TDC1290_OUT_PROG")==0)))
      {
        sscanf (str_tmp, "%*s %d %d", &i1);
	    outProgControl= i1;
      }

	  else if(active && ((strcmp(keyword,"TDC1190_W_WIDTH")==0)||(strcmp(keyword,"TDC1290_W_WIDTH")==0)))
      {
        sscanf (str_tmp, "%*s %d %d", &i1);
	    window_width= i1;
      }

      else if(active && ((strcmp(keyword,"TDC1190_W_OFFSET")==0)||(strcmp(keyword,"TDC1290_W_OFFSET")==0)))
      {
        sscanf (str_tmp, "%*s %d %d", &i1);
	    window_offset= i1;
      }

      else if(active && ((strcmp(keyword,"TDC1190_W_EXTRA")==0)||(strcmp(keyword,"TDC1290_W_EXTRA")==0)))
      {
        sscanf (str_tmp, "%*s %d %d", &i1);
	    window_extra= i1;
      }

      else if(active && ((strcmp(keyword,"TDC1190_W_REJECT")==0)||(strcmp(keyword,"TDC1290_W_REJECT")==0)))
      {
        sscanf (str_tmp, "%*s %d %d", &i1);
	    window_reject= i1;
      }

      else if(active && ((strcmp(keyword,"TDC1190_EDGE")==0)||(strcmp(keyword,"TDC1290_EDGE")==0)))
      {
        sscanf (str_tmp, "%*s %d %d", &i1);
	    for(ii=0; ii<NBOARD; ii++)  if(tdc[ii].group == gr)  tdc[ii].edge = i1 & 0x3;
      }

      else if(active && ((strncmp(keyword,"TDC1190_MASK",12) == 0)||(strncmp(keyword,"TDC1290_MASK",12) == 0)))
      {
	    SCAN_MSK;
	    ui1 = 0;
	    for(jj=0; jj<NCHAN; jj++)
	    {
	      if((msk[jj] < 0) || (msk[jj] > 1))
	      {
	        printf("\nReadConfigFile: Wrong mask bit value, %d\n\n",msk[jj]); return(-9);
	      }
	      ui1 |= (msk[jj]<<jj);
	    }

        str2[0] = keyword[12];
        str2[1] = '\0';
        jj = atoi(str2) - 1;
        if(jj<0 || jj>7)
		{
          printf("ERROR: mask jj = %d, must be between 0 and 7\n",jj);
          return(-9);
		}
        printf("mask jj = %d\n",jj);
	    for(ii=0; ii<NBOARD; ii++)
	      if(tdc[ii].group == gr)
	        tdc[ii].mask[jj] = ui1;
	  }
      else
      {
        ; /* unknown key - do nothing */
		/*
        printf("ReadConfigFile: Unknown Field or Missed Field in\n");
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






int
tdc1290DownloadAll()
{
  int ii;

  for(ii=0; ii<Nc1190; ii++) tdc1190Clear(ii);
  for(ii=0; ii<Nc1190; ii++) tdc1190Reset(ii);
  for(ii=0; ii<Nc1190; ii++)
  {
    /* program TDC for trigger matching mode */
    tdc1190SetTriggerMatchingMode(ii);

    /* set Trigger Window Width (ns) */
    tdc1190SetWindowWidth(ii, window_width);

    /* set Trigger Window Offset (ns) */
    tdc1190SetWindowOffset(ii, window_offset);

    /* set Extra Search Margin (after window) (ns) */
    tdc1190SetExtraSearchMargin(ii, window_extra);

    /* set Reject Margin (before window) (ns) */
    tdc1190SetRejectMargin(ii, window_reject);

    /* enable subtraction of trigger time */
    tdc1190EnableTriggerTimeSubtraction(ii);

    /* set Edge Detection */
    tdc1190SetEdgeDetectionConfig(ii, tdc[ii].edge);

    /* set Channels Enable Mask */
    tdc1290EnableChannels(ii, tdc[ii].mask);

    /* set Readout settings */
    tdc1190EventFifo(ii, berr_fifo);
    tdc1190BusError(ii, (~berr_fifo)&0x1);

    /* enable 64-bit alignment */
    /*  NOTE: 64-bit alignment must be enabled for any DMA readout,
       not only for MBLT(D64) but for BLT(D32) as well; some DMA engines
       (for example the one on mv5100) will cut off last 32-bit word if
       it is not even, for example if event size is equal to 137 D32 words,
       only 136 D32 words will be transfered */
    /* in addition universe library contains check for 64-bit alignment
       in dmastart procedure, it will return error if ... */
    tdc1190Align64(ii, 1);

    /* set BLT Event Number Register */
    tdc1190SetBLTEventNumber(ii, blt_Events);

    /* set Max Number of Hits per Event */
    tdc1190SetMaxNumberOfHitsPerEvent(ii, n_Hits);

    /* reset MCST flag in every board in case if it was set before */
    tdc1190ResetMCST(ii);

    /* set 'almost full' level */
    tdc1190SetAlmostFullLevel(ii, almostFullLevel);

    /* program output connector to signal on it */
    tdc1190SetOutProg(ii, outProgControl);
  }

  return(0);
}


/*
cd "/usr/local/clas12/release/0.1/coda/src/rol/VXWORKS_ppc/lib"
ld < librol.so
tdc1190Init(0x11300000,0x80000,20,0)
tdc1190Config("")
tdc1190Mon(0)
*/

/* tdc1190Init() have to be called before this function */
int  
tdc1290Config(char *fname)
{
  int res;

  /* set defaults */
  tdc1290InitGlobals();

  /* read config file */
  if( (res = tdc1290ReadConfigFile(fname)) < 0 ) return(res);

  /* download to all boards */
  tdc1290DownloadAll();

  return(0);
}

void
tdc1190Mon(int slot)
{
  tdc1290Mon(slot);
}

void
tdc1290Mon(int slot)
{
  int id, ii, start, end, res;
  UINT16 channels[2];
  usrVmeDmaGetConfig(&a24_a32, &sngl_blt_mblt, &sst_rate);

  printf("\nCPU DMA settings:\n");
  if(a24_a32==0) printf("  A16 address mode\n");
  else if(a24_a32==1) printf("  A24 address mode\n");
  else if(a24_a32==2) printf("  A32 address mode\n");
  else printf("  unknown address mode\n");

  tdc1290PrintDMAdataType(sngl_blt_mblt);

  if(sst_rate==0) printf("  SST160 rate\n");
  else if(sst_rate==1) printf("  SST267 rate\n");
  else printf("  unknown SST rate\n");

  printf("\n");

  if(slot==0)
  {
    start = 0;
    end   = Nc1190;
  }
  else if((id = tdc1190Id(slot)) >= 0)
  {
    start = id;
    end   = start + 1;
  }
  else
  {
    return;
  }

  for(ii=start; ii<end; ii++)
  {
    printf("\ntdc CAEN 1290 number %d in slot %d\n",ii,tdc1190Slot(ii));
    printf("  Board Type/ID       = 0x%04x\n", tdc119GetBoardID(ii));
    printf("  Board Revision      = 0x%08x\n", tdc119GetBoardRev(ii));
    printf("  Board Serial Number = %04d\n",  tdc119GetSerialNumber(ii));

    tdc1290GetChannels(ii,channels[2]);
    printf("  Channel Mask        = 0x%04x%04x\n", channels[1],channels[0]);
    printf("\n");

    tdc1290PrintDMABerrFifo(berr_fifo);
    printf("\n");

    tdc1190ReadTriggerConfiguration(ii);
    printf("\n");
    tdc1190ReadEdgeDetectionConfig(ii);

    res = tdc1190ReadAcquisitionMode(ii);
    if(res==0)
      printf(" Acquisition Mode = Continuous Storage\n");
    else
      printf(" Acquisition Mode = Trigger Matching\n");

    res = tdc1190StatusFull(ii);
    if(res==0)
      printf(" Output Buffer status = Not Full\n");
    else
      printf(" Output Buffer status = Full\n");

    printf(" BLT Event Number = %d\n",tdc1190GetBLTEventNumber(ii));
    printf(" Max Number of Hits per Event = %d\n",tdc1190GetMaxNumberOfHitsPerEvent(ii));
    tdc1190GetAlmostFullLevel(ii);
  }
}








#define ADD_TO_STRING \
  len1 = strlen(str); \
  len2 = strlen(sss); \
  if((len1+len2) < length) strcat(str,sss); \
  else \
  { \
    str[len1+1] = ' '; \
    str[len1+2] = ' '; \
    str[len1+3] = ' '; \
    len1 = ((len1+3)/4)*4; \
    return(len1); \
  }

/* upload setting from all found DSC2s */
int
tdc1290UploadAll(char *string, int length)
{
  int slot, i, ii, jj, kk, nnn, len1, len2;
  char *str, sss[1024];
  unsigned int tmp, val[NCHAN], val1[NCHAN];
  unsigned short sval[NCHAN];
  unsigned short bypMask;
  unsigned short channels[8];

  str = string;
  str[0] = '\0';
  for(kk=0; kk<Nc1190; kk++)
  {
    slot = tdc1190Slot(kk);

    sprintf(sss,"TDC1190_SLOT %d\n",slot);
    ADD_TO_STRING;

    sprintf(sss,"TDC1190_BERR_FIFO %d\n",tdc1190GetEventFifo(kk));
    ADD_TO_STRING;
	
    sprintf(sss,"TDC1190_BLT_EVENTS %d\n",tdc1190GetBLTEventNumber(kk));
    ADD_TO_STRING;

    sprintf(sss,"TDC1190_N_HITS %d\n",tdc1190GetMaxNumberOfHitsPerEvent(kk));
    ADD_TO_STRING;

    sprintf(sss,"TDC1190_ALMOSTFULL %d\n",tdc1190GetAlmostFullLevel(kk));
    ADD_TO_STRING;

    sprintf(sss,"TDC1190_OUT_PROG %d\n",tdc1190GetOutProg(kk));
    ADD_TO_STRING;

    sprintf(sss,"TDC1190_W_WIDTH %d # not fron hardware\n",window_width);
    ADD_TO_STRING;

    sprintf(sss,"TDC1190_W_OFFSET %d # not fron hardware\n",window_offset);
    ADD_TO_STRING;

    sprintf(sss,"TDC1190_W_EXTRA %d # not fron hardware\n",window_extra);
    ADD_TO_STRING;

    sprintf(sss,"TDC1190_W_REJECT %d # not fron hardware\n",window_reject);
    ADD_TO_STRING;

    sprintf(sss,"TDC1190_EDGE %d\n",tdc1190ReadEdgeDetectionConfig(kk));
    ADD_TO_STRING;

    tdc1190GetChannels(kk, channels);
    if(use1190[kk] == 1) nnn = 8;
    else                 nnn = 2;
    for(ii=0; ii<nnn; ii++)
	{
      sprintf(sss,"TDC1190_MASK%1d",ii+1);
      ADD_TO_STRING;
      for(jj=0; jj<16; jj++)
	  {
        sprintf(sss," %d",(channels[ii]>>jj)&0x1);
        ADD_TO_STRING;
	  }
      sprintf(sss,"\n");
      ADD_TO_STRING;
	}
  }

  len1 = strlen(str);
  str[len1+1] = ' ';
  str[len1+2] = ' ';
  str[len1+3] = ' ';
  len1 = ((len1+3)/4)*4;

  return(len1);
}






int
tdc1190UploadAll(char *string, int length)
{
  return(tdc1290UploadAll(string, length));
}





void
tdc1290PrintDMAdataType(int dataType)
{
  if(dataType==0x01)
  {
    printf("  D32 single word readout\n");
  }
  else if(dataType==0x02)
  {
    printf("  D32 DMA (BLT) readout\n");
  }
  else if(dataType==0x03)
  {
    printf("  D64 DMA (MBLT) readout\n");
  }
  else if(dataType==0x04)
  {
    printf("  D64 DMA (2eVME) readout\n");
  }
  else if(dataType==0x05)
  {
    printf("  D64 DMA (2eSST) readout\n");
  }
  else
  {
    printf("  unknown readout mode !!!\n");
  }
}

void
tdc1290PrintDMABerrFifo(int method)
{
  if(method==0x00)
  {
    printf("  DMA Nwords method: use VME BUS error\n");
  }
  else if(method==0x01)
  {
    printf("  DMA Nwords method: use event FIFO\n");
  }
  else
  {
    printf("  unknown DMA Nwords method, use VME BUS error\n");
  }

}

/*S.P.end*/
/*****************************************************************************/
/*****************************************************************************/




/*******************************************************************************
 *
 * tdc1190Status - Gives Status info on specified TDC
 *
 *
 * RETURNS: None
 */

STATUS
tdc1190Status(int id)
{
  int lock;
  int drdy=0, afull=0, bfull=0;
  int berr=0, testmode=0;
  int trigMatch=0, headers=0, tdcerror[4];
  UINT16 statReg, cntlReg, afullLevel, bltEvents;
  UINT16 iLvl, iVec, evStored;
  UINT16 evCount, res=0;
  char   tdcname[8] = { "v1190" };

  CHECKID(id);
  if(use1190[id]==2) sprintf(tdcname, "%s", "v1290");

  /* read various registers */
  LOCK_1190;
  statReg = vmeRead16(&(c1190p[id]->status))&V1190_STATUS_MASK;
  cntlReg = vmeRead16(&(c1190p[id]->control))&V1190_CONTROL_MASK;
  afullLevel = vmeRead16(&(c1190p[id]->almostFullLevel));
  bltEvents = vmeRead16(&(c1190p[id]->bltEventNumber))&V1190_BLTEVNUM_MASK;
  
  iLvl = vmeRead16(&(c1190p[id]->intLevel))&V1190_INTLEVEL_MASK;
  iVec = vmeRead16(&(c1190p[id]->intVector))&V1190_INTVECTOR_MASK;
  evCount  = vmeRead32(&(c1190p[id]->evCount))&V1190_EVCOUNT_MASK;
  evStored = vmeRead16(&(c1190p[id]->evStored));
  UNLOCK_1190;

  drdy  = statReg&V1190_STATUS_DATA_READY;
  afull = statReg&V1190_STATUS_ALMOST_FULL;
  bfull = statReg&V1190_STATUS_FULL;

  trigMatch = statReg&V1190_STATUS_TRIG_MATCH;
  headers  = statReg&V1190_STATUS_HEADER_ENABLE;
  tdcerror[0] = (statReg&V1190_STATUS_ERROR_0)>>6;
  tdcerror[1] = (statReg&V1190_STATUS_ERROR_1)>>7;
  tdcerror[2] = (statReg&V1190_STATUS_ERROR_2)>>8;
  tdcerror[3] = (statReg&V1190_STATUS_ERROR_3)>>9;

  berr = cntlReg&V1190_BUSERROR_ENABLE;
  testmode = cntlReg&V1190_TESTMODE;

  /* print out status info */

#ifdef VXWORKS
  printf("STATUS for %s TDC at base address 0x%x\n",tdcname,(UINT32)c1190p[id]);
#else
  printf("STATUS for %s TDC at VME (USER) base address 0x%x (0x%x)\n",
	 tdcname, (UINT32)c1190p[id] - tdcAddrOffset, (UINT32)c1190p[id]);
#endif
  printf("---------------------------------------------- \n");

  if(iLvl>0) {
    printf(" Interrupts Enabled - Request level = %d words\n",afullLevel);
    printf(" VME Interrupt Level: %d   Vector: 0x%x \n",iLvl,iVec);
  } else {
    printf(" Interrupts Disabled\n");
  }
  printf("\n");

  printf("  Data Status \n");
  printf("    Events to transfer via BLT  = 0x%04x\n",bltEvents);
  if(bfull) {
    printf("    Status        = 0x%04x (Buffer Full)\n",statReg);
  } else if(afull) {
    printf("    Status        = 0x%04x (Almost Full)\n",statReg);
  }else if(drdy) {
    printf("    Status        = 0x%04x (Data Ready )\n",statReg);
  }else {
    printf("    Status        = 0x%04x (No Data)\n",statReg);
  }
  
  printf("    Events Stored = %d\n",evStored);
  if(evCount == 0xffff)
    printf("    Total Events  = (No Events taken)\n");
  else
    printf("    Total Events  = %d\n",evCount);
  printf("    Almost Full   = %d\n",afullLevel);

  
  printf("\n");

  printf("  TDC Mode/Status \n"); 

  res = tdc1190GetEdgeResolution(id);
  printf("    Resolution      : %d ps\n",res);

  if(testmode)
    printf("    Test Mode       : On\n");

  if(trigMatch)
    printf("    Trigger Matching: On\n");
  else
    printf("    Trigger Matching: Off\n");
  
  if(headers)
    printf("    TDC Headers/EOB : On\n");
  else
    printf("    TDC Headers/EOB : Off\n");

  if(berr)
    printf("    Bus Errors      : On\n");
  else
    printf("    Bus Errors      : Off\n");

  printf("    TDC Errors      : 0:%d   1:%d   2:%d   3:%d\n",
	 tdcerror[0],tdcerror[1],tdcerror[2],tdcerror[3]);
  printf("\n");

  return OK;
}



/*******************************************************************************
 *
 *  tdc1190ReadMicro - Read Microcontroller Register
 *
 *  returns  OK or ERROR
 */

STATUS
tdc1190ReadMicro(int id, UINT16 *data, int nwords)
{
  int ii, jj, kk=0;
  UINT16 mstatus;

  CHECKID(id);

  LOCK_1190;
 retry:

  mstatus = vmeRead16(&(c1190p[id]->microHandshake)) & V1190_MICRO_READOK;
  if(kk > 10)
    {
      printf("-> ReadMicro: mstatus=%d (0x%x)\n",mstatus,
	     vmeRead16(&(c1190p[id]->microHandshake)));
    }
  if(mstatus)
    {
      for(ii=0; ii<nwords; ii++)
	{
	  jj=0;
	  while(!(vmeRead16(&(c1190p[id]->microHandshake)) & V1190_MICRO_READOK))
	    {
	      jj++;
	      if(jj>20)
		{
		  logMsg("tdc1190ReadMicro: ERROR1: Read Status not OK (read %d)\n",
			 ii,0,0,0,0,0);
		  UNLOCK_1190;
		  return(ERROR);
		}
	    }
	  data[ii] = vmeRead16(&(c1190p[id]->microReg));
	}
    }
  else
    {
      kk++;
      if(kk>=20)
	{
	  logMsg("tdc1190ReadMicro: ERROR2: Read Status not OK\n",0,0,0,0,0,0);
	  UNLOCK_1190;
	  return(ERROR);
	}
      else
	{
	  taskDelay(10);
	  goto retry;
	}
    }

  if(kk > 10) printf("-> ReadMicro: kk=%d\n",kk);
  UNLOCK_1190;
  return(OK);
}



/*******************************************************************************
 *
 *  tdc1190WriteMicro - Write to Microcontroller Register
 *
 *   returns  OK or ERROR
 */
STATUS
tdc1190WriteMicro(int id, UINT16 data)
{
  int kk=0;
  volatile UINT16 mstatus;

  CHECKID(id);

  LOCK_1190;
 retry:

  mstatus = vmeRead16(&(c1190p[id]->microHandshake)) & V1190_MICRO_WRITEOK;

  if(mstatus)
    {
      vmeWrite16(&(c1190p[id]->microReg),data);
    }
  else
    {
      kk++;
      mstatus=0;
      if(kk>=20)
	{
	  logMsg("tdc1190WriteMicro: ERROR: Write Status not OK\n",0,0,0,0,0,0);
	  UNLOCK_1190;
	  return(ERROR);
	}
      else
	{
	  taskDelay(10);
	  goto retry;
	}
    }

  UNLOCK_1190;
  if(kk > 10) printf("-> WriteMicro: kk=%d\n",kk);
  return(OK);
}

/*******************************************************************************
 *
 * tdc1190PrintEvent - Print an event from TDC to standard out. 
 *
 *
 * RETURNS: Number of Data words read from the TDC (including Header/Trailer).
 */

int
tdc1190PrintEvent(int id, int pflag)
{
  int ii, jj, nWords, evID, bunchID, evCount, headFlag, trigMatch;
  UINT32 gheader, gtrailer, theader, ttrailer, tmpData, dCnt;
  int tdcID, chanID, dataVal, tEdge;

  CHECKID(id);


  LOCK_1190;
  /* Check if there is a valid event */
  if(vmeRead16(&(c1190p[id]->status))&V1190_STATUS_DATA_READY) {
    dCnt = 0;
    headFlag  = vmeRead16(&(c1190p[id]->status))&V1190_STATUS_HEADER_ENABLE;
    trigMatch = vmeRead16(&(c1190p[id]->status))&V1190_STATUS_TRIG_MATCH;

    if(trigMatch) {  /* If trigger match mode then print individual event */

      /* Read Global Header - Get event count */
      gheader = vmeRead32(&(c1190p[id]->data[0]));
      if((gheader&V1190_DATA_TYPE_MASK) != V1190_GLOBAL_HEADER_DATA)
	{
	  logMsg("tdc1190PrintEvent: ERROR: Invalid Global Header Word 0x%08x\n",
		 gheader,2,3,4,5,6);
	  UNLOCK_1190;
	  return(ERROR);
	}
      else
	{
	  logMsg("  TDC DATA for Module at address 0x%08x\n",
		 (UINT32)c1190p[id],2,3,4,5,6);
	  evCount = (gheader&V1190_GHEAD_EVCOUNT_MASK)>>5;
	  dCnt++;
	  logMsg("  Global Header: 0x%08x   Event Count = %d \n",
		 gheader,evCount,3,4,5,6);
	}

      /* Loop over four TDC chips and get data for each */
      for(ii=0; ii<4; ii++)
	{
	  /* Read TDC Header - Get event ID, Bunch ID */
	  theader = vmeRead32(&(c1190p[id]->data[0]));
	  if((theader&V1190_DATA_TYPE_MASK) != V1190_TDC_HEADER_DATA)
	    {
	      logMsg("ERROR: Invalid TDC Header Word 0x%08x for TDC %d\n",
		     theader,ii,3,4,5,6);
	      UNLOCK_1190;
	      return(ERROR);
	    }
	  else
	    {
	      evID = (theader&V1190_TDCHEAD_EVID_MASK)>>12;
	      bunchID = (theader&V1190_TDCHEAD_BUNCHID_MASK);
	      dCnt++;
	      logMsg("    TDC %d Header: 0x%08x   EventID = %d  Bunch ID = %d ",
		     ii,theader,evID,bunchID,5,6);
	    }
	  jj=0;
	  tmpData = vmeRead32(&(c1190p[id]->data[0]));
	  dCnt++;
	  while((tmpData&V1190_DATA_TYPE_MASK) != V1190_TDC_EOB_DATA)
	    {
/* 	      if((jj % 5) == 0) printf("\n     "); */
	      logMsg("ch %d:   0x%08x (0x%08x)\n",
		     (tmpData&0x3e00000)>>21,(tmpData&0x1ffff),
		     (tmpData),4,5,6);
	      jj++;
	      tmpData = vmeRead32(&(c1190p[id]->data[jj]));
	    }
	  /* reached EOB for TDC */
	  logMsg("\n",1,2,3,4,5,6);
	  ttrailer = tmpData;
	  if((ttrailer&V1190_DATA_TYPE_MASK) != V1190_TDC_EOB_DATA)
	    {
	      logMsg("ERROR: Invalid TDC EOB Word 0x%08x for TDC %d\n",
		     ttrailer,ii,3,4,5,6);
	      UNLOCK_1190;
	      return(ERROR);
	    }
	  else
	    {
	      nWords = (ttrailer&V1190_TDCEOB_WORDCOUNT_MASK);
	      dCnt++;
	      logMsg("    TDC %d EOB   : 0x%08x   Word Count = %d \n",
		     ii,ttrailer,nWords,4,5,6);
	    }
	}

      /* next data word should be Global EOB */
      gtrailer = vmeRead32(&(c1190p[id]->data[dCnt]));
      if((gtrailer&V1190_DATA_TYPE_MASK) != V1190_GLOBAL_EOB_DATA)
	{
	  logMsg("tdc1190PrintEvent: ERROR: Invalid Global EOB Word 0x%08x\n",
		 gtrailer,2,3,4,5,6);
	  UNLOCK_1190;
	  return(ERROR);
	}
      else
	{
	  nWords = (gtrailer&V1190_GEOB_WORDCOUNT_MASK)>>5;
	  dCnt++;
	  logMsg("  Global EOB   : 0x%08x   Total Word Count = %d \n",
		 gtrailer,nWords,3,4,5,6);
	}
    }
    else /* Continuous Storage mode */
      {
	tmpData = vmeRead32(&(c1190p[id]->data[dCnt]));
	logMsg("  TDC Continuous Storage DATA\n",1,2,3,4,5,6);
	while((tmpData&V1190_DATA_TYPE_MASK) != V1190_FILLER_DATA)
	  {
	    tdcID  = (tmpData&V1190_TDC_MASK)>>24;
	    chanID = (tmpData&V1190_CHANNEL_MASK)>>19;
	    tEdge = (tmpData&V1190_EDGE_MASK)>>19;
	    dataVal = (tmpData&V1190_DATA_MASK);
	    logMsg("    %d   %d   %d    %d\n",tdcID, chanID, tEdge, dataVal,5,6);
	    dCnt++;
	    tmpData = vmeRead32(&(c1190p[id]->data[dCnt]));
	  }
	printf("\n");
      }
    UNLOCK_1190;
    return(dCnt);
  }
  else
    {
      logMsg("tdc1190PrintEvent: No data available for readout!\n",1,2,3,4,5,6);
      UNLOCK_1190;
      return(0);
    }
  UNLOCK_1190;
}

/*******************************************************************************
 *
 * tdc1190StatusFull - Return the status of the Output Buffer Full register
 *
 * RETURNS: 1 if Full, 0 if Not Full, -1 if TDC is disabled
 *
 */

int
tdc1190StatusFull(int id)
{
  int res;

  CHECKID(id);

  if(use1190[id]==0) 
    return -1; 

  LOCK_1190;
  res = vmeRead16(&(c1190p[id]->status)) & V1190_STATUS_FULL;
  UNLOCK_1190;

  return(res);
}

/*******************************************************************************
 *
 * tdc1190StatusAlmostFull - Return the status of the Output Buffer 
 *                           Almost Full register
 *
 * RETURNS: 1 if Almost Full level is met, 0 if not, -1 if TDC is disabled
 *
 */

int
tdc1190StatusAlmostFull(int id)
{
  int res;

  CHECKID(id);

  if(use1190[id]==0) return(-1); 

  LOCK_1190;
  res = vmeRead16(&(c1190p[id]->status)) & V1190_STATUS_ALMOST_FULL;
  UNLOCK_1190;

  return(res);
}


/******************************************************************************
 *
 * tdc1190Dready - data readyness
 *
 *
 * RETURNS: 0(No Data) or the number of events
 */

int
tdc1190Dready(int id)
{
  UINT16 stat=0, nevents;
  
  CHECKID(id);

  if(use1190[id]==0) return(ERROR); 

  LOCK_1190;
  stat = vmeRead16(&(c1190p[id]->status)) & V1190_STATUS_DATA_READY;
  if(stat)
  {
    nevents = vmeRead16(&(c1190p[id]->evStored));
    UNLOCK_1190;
    return(nevents);
  }
  else
  {
    /*logMsg("stat=%d\n",stat,2,3,4,5,6);*/
    UNLOCK_1190;
    return(0);
  }
}

/******************************************************************************
 *
 * tdc1190EventFifo - Set the status of the event fifo
 *                flag = 1 : enable event fifo
 *                       0 : disable event fifo
 *
 *
 * RETURNS: 0 if successful, ERROR otherwise.
 */

int
tdc1190EventFifo(int id, UINT32 flag)
{
  unsigned short reg;

  CHECKID(id);

  if(use1190[id]==0) 
    return ERROR; 

  if(flag<0||flag>1)
  {
    printf("%s: ERROR: Invalid flag = %d",__FUNCTION__,flag);
    return ERROR;
  }

  LOCK_1190;
  reg = vmeRead16(&(c1190p[id]->control));
  printf("%s before: 0x%04x\n",__FUNCTION__,reg);
  if(flag == 1)
  {
    reg = reg | V1190_EVENT_FIFO_ENABLE;
    vmeWrite16(&(c1190p[id]->control),reg);
  }
  else
  {
    reg = reg & ~V1190_EVENT_FIFO_ENABLE;
    vmeWrite16(&(c1190p[id]->control),reg);
  }
  printf("%s  after: 0x%04x\n",__FUNCTION__,vmeRead16(&(c1190p[id]->control)));
  UNLOCK_1190;

  return(0);
}

int
tdc1190GetEventFifo(int id)
{
  unsigned short reg;

  CHECKID(id);

  if(use1190[id]==0) 
    return ERROR; 

  LOCK_1190;
  reg = vmeRead16(&(c1190p[id]->control));
  UNLOCK_1190;

  return(reg & V1190_EVENT_FIFO_ENABLE);
}



/******************************************************************************
 *
 * tdc1190ResetMSCT - Reset the MSCT/CBLT register
 *                    This effectively disables MSCT/CBLT
 *
 *
 * RETURNS: 0 if successful, ERROR otherwise.
 */

int
tdc1190ResetMCST(int id)
{
  CHECKID(id);

  if(use1190[id]==0) 
    return ERROR; 

  LOCK_1190;
  vmeWrite16(&(c1190p[id]->mcstCtrl),0);
  UNLOCK_1190;

  return(0);
}

/******************************************************************************
 *
 * tdc1190BusError - Enable/Disable Bus Errors (to finish a block transfer,
 *                   or on an empty buffer read)
 *                    flag = 0 : Disable
 *                           1 : Enable
 *
 * RETURNS: 0 if successful, ERROR otherwise.
 */

int
tdc1190BusError(int id, UINT32 flag)
{
  CHECKID(id);

  if(use1190[id]==0) 
    return ERROR; 

  if(flag>1)
  {
    printf("%s: ERROR: Invalid flag = %d",__FUNCTION__,flag);
    return ERROR;
  }

  LOCK_1190;
  if(flag == 1)
  {
    printf("  set BUSerror\n");
    vmeWrite16(&(c1190p[id]->control),
		 vmeRead16(&(c1190p[id]->control)) | V1190_BUSERROR_ENABLE);
  }
  else if(flag == 0)
  {
    printf("  reset BUSerror\n");
    vmeWrite16(&(c1190p[id]->control),
		 vmeRead16(&(c1190p[id]->control)) & ~V1190_BUSERROR_ENABLE);
  }
  UNLOCK_1190;

  return(0);
}

int
tdc1190GetBusError(int id)
{
  int reg;

  CHECKID(id);

  if(use1190[id]==0) 
    return ERROR; 

  LOCK_1190;
  reg = vmeRead16(&(c1190p[id]->control));
  UNLOCK_1190;

  return(reg & V1190_BUSERROR_ENABLE);
}

/******************************************************************************
 *
 * tdc1190Align64 - Enable/Disable 64 bit alignment for block transfers
 *                   flag = 0 : Disable
 *                          1 : Enable
 *
 * RETURNS: 0 if successful, ERROR otherwise.
 */

int
tdc1190Align64(int id, UINT32 flag)
{

  CHECKID(id);

  if(use1190[id]==0) 
    return ERROR; 

  if(flag>1)
    {
      printf("%s: ERROR: Invalid flag = %d",__FUNCTION__,flag);
      return ERROR;
    }

  LOCK_1190;
  printf("%s before: 0x%04x\n",__FUNCTION__,
	 vmeRead16(&(c1190p[id]->control)));

  if(flag == 1)
    vmeWrite16(&(c1190p[id]->control),
	       vmeRead16(&(c1190p[id]->control)) | V1190_ALIGN64);

  else if(flag == 0)
    vmeWrite16(&(c1190p[id]->control),
	       vmeRead16(&(c1190p[id]->control)) & ~V1190_ALIGN64);

  else
    printf("%s: unknown flag=%d\n",__FUNCTION__,flag);

  printf("%s  after: 0x%04x\n",__FUNCTION__,
	 vmeRead16(&(c1190p[id]->control)));
  UNLOCK_1190;


  return(0);
}





/*Sergey start: my mid-  and top-level functions*/

int
tdc1190ReadBoard(int itdc, UINT32 *tdata)
{
  volatile UINT32 *data;
  volatile UINT32 *fifo;
  UINT32 *output = tdata - 1;
  int fifodata, ndata, nev, ii;

  UINT32 addr = (unsigned int) c1190p[itdc];
  data = (UINT32 *) addr;
  fifo = (UINT32 *) (addr+0x1038);

  if(berr_fifo == 0x01)
  {
    /* get event length in words */
    /* do not need it here but must read fifo as soon as it is enabled,
       otherwise 'full' condition will happens */

    nev = tdc1190Dready(itdc);
    if(nev > blt_Events) nev = blt_Events;
    for(ii=0; ii<nev; ii++)
	{
      fifodata = (vmeRead32(&(c1190p[itdc]->fifo))&0xffff);
	}
  }

  do
  {
    *(++output) = vmeRead32(&(c1190p[itdc]->data[0]));
  } while( ((*output)&V1190_DATA_TYPE_MASK) != V1190_GLOBAL_EOB_DATA );

  return(((int)(output-tdata))+1);
}


int
tdc1190ReadBoardDmaStart(int ib, UINT32 *tdata)
{
  volatile UINT32 *vmeAdr;
  volatile UINT32 *fifo;
  int mdata, fifodata, res;
  int i, ii, nbytes, nev;
  int ndata_save, extra_save;

  UINT32 addr = (unsigned int) c1190p[ib];
  fifo = (UINT32 *) (addr+0x1038);

  if(berr_fifo == 0x01)
  {
    /* get event length in words */
    nev = tdc1190Dready(ib);
    if(nev > blt_Events) nev = blt_Events;
    ndata_save = 0;
    for(ii=0; ii<nev; ii++)
	{
      fifodata = (vmeRead32(&(c1190p[ib]->fifo))&0xffff);
      ndata_save += fifodata&0xffff;
	}
    /*
    logMsg("tdc1190ReadBoardDmaStart: INFO: event fifo reports %d words\n",
           ndata_save,0,0,0,0,0);
	*/
  }
  else
  {
    ndata_save = V1190_MAX_WORDS_PER_BOARD;
    mdata = 0;
    /*
    logMsg("tdc1190ReadBoardDmaStart: INFO: trying to DMA %d words\n",
           ndata_save,0,0,0,0,0);
    */
  }

  /*usrVmeDmaReset();*/

  if(berr_fifo == 0x01)
  {
    if(sngl_blt_mblt >= 0x04) /* 128 bit alignment */
	{
      extra_save = (4-(ndata_save%4));
	  /*
      logMsg("111: tdc1190ReadBoardDmaStart: ndata_save=%d extra_save=%d\n",
        ndata_save,extra_save,3,4,5,6);
	  */
      if(extra_save==4) extra_save=0;
	}
	else /* 64 bit alignment */
	{
      if( (ndata_save%2) != 0 ) extra_save = 1;
      else                      extra_save = 0;
	}
	/*
    logMsg("tdc1190ReadBoardDmaStart: ndata_save=%d extra_save=%d\n",
      ndata_save,extra_save,3,4,5,6);
	*/
    nbytes_save[ib] = nbytes = ((ndata_save+extra_save)<<2);
  }
  else
  {
    nbytes_save[ib] = nbytes = ndata_save<<2;
  }

  /*
printf("tdc1190ReadBoardDmaStart[%d]: c1190vme=0x%08x, tdata=0x%08x, nbytes=%d\n",
		 ib,c1190vme[ib],tdata, nbytes);
  */
  res = usrVme2MemDmaStart( (UINT32 *)c1190vme[ib], (UINT32 *)tdata, nbytes);

  if(res < 0)
  {
    logMsg("tdc1190ReadEventDmaStart: ERROR: usrVme2MemDmaStart returned %d\n",
           res,0,0,0,0,0);
  }
  /*else
  {
    logMsg("tdc1190ReadBoardDmaStart: usrVme2MemDmaStart returned %d\n",
      res,0,0,0,0,0);
  }
  */

  return(res);
}

/* returns the number of words transfered, or error */
int
tdc1190ReadBoardDmaDone(int ib)
{
  int mbytes, res;

  /* check if transfer is completed; returns nbytes or ERROR  */
  if((res = usrVme2MemDmaDone()) < 0)
  {
    logMsg("tdc1190ReadBoardDmaDone: ERROR: usrVme2MemDmaDone returned = %d\n",
           res,2,3,4,5,6);
    return(-1);
  }

  if(berr_fifo == 0x01) /*fifo readout*/
  {
    mbytes = nbytes_save[ib] - res;
    /*logMsg("%s: nbytes_save=%d res=%d -> mbytes=%d\n",__FUNCTION__,nbytes_save[id],res,mbytes,5,6);*/
    if(mbytes>0)
    {
      logMsg("%s: WRONG: nbytes_save[%d]=%d, res=%d => mbytes=%d\n",(int)__FUNCTION__,
          ib,nbytes_save[ib],res,mbytes,6);
      return(-2);
    }
  }

  return(res>>2); /* return length in words */
}


/* generic readout for v1190/v1290 TDC boards */
/* time profiling data for 2 boards 550 bytes event size */
int
tdc1190ReadStart(INT32 *tdcbuf, INT32 *rlenbuf)
{
  int ii, jj, nev, itdcbuf, itmp1;
  int nn[21];
  unsigned short tdata;
  int notready;

/* whole routine: 42 usec */

/* part1: 6 usec */

  if(Nc1190==0)
  {
    logMsg("tdc1190ReadStart: ERROR: Nc1190=%d\n",Nc1190,2,3,4,5,6);
  }

  notready = 0;
  for(jj=0; jj<Nc1190; jj++)
  {
    /* check if board is full */
    if(tdc1190StatusFull(jj))
    {
      logMsg("ERROR: [%2d] board is full - clear (Nc1190=%d)\n",
        jj,Nc1190,3,4,5,6);
      /* clear board, otherwise cannot use it any more ! */
      tdc1190Clear(jj);
    }

    /* check the number of events */
    nn[jj] = nev = tdc1190Dready(jj);

    /* if trigger matching window 'straddling' trigger, tdc1190Dready returns 0
      (V1190_STATUS_DATA_READY bit is zero),but next call returns 1 - do not know why ..*/
    if(nev == 0) nn[jj] = nev = tdc1190Dready(jj);

    if(nev == 0)
    {
      logMsg("tdc1190ReadStart: [%2d] not ready ! (nev=%d)\n",jj,tdc1190Dready(jj),3,4,5,6);
      notready = 1;
    }

    /* should never have more then 100 events in one block 
    if(nev > 100)      
    {
	  logMsg("tdc1190ReadStart: ERROR: [%2d] nev=%d\n",jj,nev,3,4,5,6);
	}
	*/
  }

  if(notready) return(ERROR);


  /* part2: 36 usec */

  /* readout */
  notready = 0;
  itdcbuf = 0;
  for(jj=0; jj<Nc1190; jj++)
  {

    if(sngl_blt_mblt == 0x01) /*no DMA*/
    {
      rlenbuf[jj] = tdc1190ReadBoard(jj,&tdcbuf[itdcbuf]);
    }
    else
    {
      /* 18usec x 2boards = 36Usec */
      tdc1190ReadBoardDmaStart(jj,&tdcbuf[itdcbuf]);
      rlenbuf[jj] = tdc1190ReadBoardDmaDone(jj);
    }

    if(rlenbuf[jj] <= 0)
    {
      logMsg("[%2d] ERROR: tdc1190ReadEvent[Dma] returns %d\n",
        jj,rlenbuf[jj],3,4,5,6);
      notready = 1;
    }
    else
    {
      itdcbuf += rlenbuf[jj];
    }

  }

  if(notready) return(ERROR);

  return(OK);
}


/* use DMA list */
int
tdc1190ReadListStart(INT32 *tdcbuf, INT32 *rlenbuf)
{
  int fifodata;
  int ii, jj, nev;
  int itdcbuf;
  static int *destination[V1190_MAX_MODULES];
  int ndata_save, extra_save;

  /*
TIMER_VAR;
  */
  if(Nc1190<2)
  {
    logMsg("tdc1190ReadListStart: ERROR: Nc1190=%d\n",Nc1190,2,3,4,5,6);
  }

  for(jj=0; jj<Nc1190; jj++)
  {
    /* check if board is full */
    if(tdc1190StatusFull(jj))
    {
      logMsg("ERROR: [%2d] board is full - clear (Nc1190=%d)\n",
        jj,Nc1190,3,4,5,6);
      /* clear board, otherwise cannot use it any more ! */
      tdc1190Clear(jj);
    }

    /* check the number of events */
    nev = tdc1190Dready(jj);

    /* if trigger matching window 'straddling' trigger, tdc1190Dready returns 0 (V1190_STATUS_DATA_READY bit is zero),
    but next call returns 1 - do not know why ..*/
    if(nev == 0) nev = tdc1190Dready(jj);

    if(nev == 0)
    {
      logMsg("tdc1190ReadListStart: [%2d] not ready !\n",jj,2,3,4,5,6);
    }

    /* Trigger Supervisor has 6 event buffer, but we can get 7
	   if 'parallel' readout is in use 
    if(nev > 7)
    {
	  logMsg("tdc1190ReadListStart: ERROR: [%2d] nev=%d\n",jj,nev,3,4,5,6);
	}
*/
  }


  if(sngl_blt_mblt == 0x01)
  {
    logMsg("ERROR: DMA method must be used: sngl_blt_mblt=%d\n",
        sngl_blt_mblt,2,3,4,5,6);
    return(ERROR);
  }



  /* readout settings */
  if(berr_fifo == 0x01) /* use FIFO reaout */
  {
    itdcbuf = 0;
    for(jj=0; jj<Nc1190; jj++)
    {

      /* get event length in words */
      nev = tdc1190Dready(jj);
      if(nev > blt_Events) nev = blt_Events;
      ndata_save = 0;
      for(ii=0; ii<nev; ii++)
	  {
        fifodata = (vmeRead32(&(c1190p[jj]->fifo))&0xffff);
        ndata_save += (fifodata & 0xffff);
	  }

      if(sngl_blt_mblt >= 0x04) /* 128 bit alignment */
	  {
        extra_save = (4-(ndata_save%4));
        if(extra_save==4) extra_save=0;
	  }
	  else /* 64 bit alignment */
	  {
        if( (ndata_save%2) != 0 ) extra_save = 1;
        else                      extra_save = 0;
	  }

      nbytes_save[jj] = (ndata_save+extra_save)<<2;
      rlenbuf[jj] = ndata_save+extra_save;

      destination[jj] = &tdcbuf[itdcbuf];

      itdcbuf += rlenbuf[jj];

	/*
logMsg("[%d] ask=%d (%d bytes), got=%d (0x%08x to 0x%08x)\n",
 jj,ndata_save+extra_save,nbytes_save[jj],rlenbuf[jj],(unsigned int)c1190p[jj],destination[jj]);
	*/

    }
  }
  else /* use BERR readout */
  {
    itdcbuf = 0;
    for(jj=0; jj<Nc1190; jj++)
    {
      ndata_save = V1190_MAX_WORDS_PER_BOARD;
      extra_save = 0;

      nbytes_save[jj] = (ndata_save)<<2;
      rlenbuf[jj] = ndata_save;

      destination[jj] = &tdcbuf[itdcbuf];

      itdcbuf += rlenbuf[jj];
	} 
  }


  /*
{
  TIMER_NORMALIZE;
  TIMER_START;
}
*/

  /* set DMA list and start DMA */
  usrVme2MemDmaListSet(c1190vme, destination, nbytes_save, Nc1190);
  usrVmeDmaListStart();

  /*
{
  TIMER_STOP(100000,-1);
}
*/
  return(OK);
}

int
tdc1190ReadDone()
{
  int mbytes, res, ii, nbytes_saved = 0;

  /* check if transfer is completed; returns nbytes or ERROR  */
  if((res = usrVme2MemDmaDone()) < 0)
  {
    logMsg("tdc1190ReadDone: ERROR: usrVme2MemDmaDone returned = %d\n",
           res,2,3,4,5,6);
    return(-1);
  }

  if(berr_fifo == 0x01) /*fifo readout*/
  {
    for(ii=0; ii<Nc1190; ii++) nbytes_saved += nbytes_save[ii];
    mbytes = nbytes_saved - res;
    /*logMsg("%s: nbytes_saved=%d res=%d -> mbytes=%d\n",__FUNCTION__,nbytes_saved,res,mbytes,5,6);*/
    if(mbytes>0)
    {
      logMsg("%s: WRONG: nbytes_saved=%d, res=%d => mbytes=%d\n",(int)__FUNCTION__,
          nbytes_saved,res,mbytes,5,6);
      return(-2);
    }
  }

  return(res>>2); /* return length in words */
}

/*Sergey end: my mid-  and top-level functions*/




/*******************************************************************************
 *
 * tdc1190ReadEvent - Read event from TDC to specified address. 
 *
 * // FIXME: May just remove this routine..
 *
 * RETURNS: Number of Data words read from the TDC (including Header/Trailer).
 */
/* FIXME: Make sure endian-ness is correct */

int
tdc1190ReadEvent(int id, UINT32 *tdata)
{
/*   int ii, nWords, evID; */
  UINT32 header, trailer, dCnt, tmpData;
  UINT16 contReg, statReg;
  int fifodata, nev, ii;

  CHECKID(id);

  LOCK_1190;
  /* Check if there is a valid event */
  if(vmeRead16(&(c1190p[id]->evStored))==0) 
    {
      logMsg("tdc1190ReadEvent: Data Buffer is EMPTY!\n",0,0,0,0,0,0);
      UNLOCK_1190;
      return(0);
    }

  statReg = vmeRead16(&(c1190p[id]->status));
  contReg = vmeRead16(&(c1190p[id]->control));
  if(statReg & V1190_STATUS_DATA_READY) 
    {
      dCnt = 0;

    /* Check to see if the Event FIFO is enabled, 
	 if so, read its register */
    if(contReg & V1190_EVENT_FIFO_ENABLE)
	{
      nev = tdc1190Dready(id);
      if(nev > blt_Events) nev = blt_Events;
      for(ii=0; ii<nev; ii++)
	  {
	    fifodata = vmeRead32(&(c1190p[id]->fifo));
	  }
	}

	/* Read Header  */
	header = vmeRead32(&(c1190p[id]->data[dCnt]));
      if((header&V1190_DATA_TYPE_MASK) != V1190_TDC_HEADER_DATA) 
	{
	  logMsg("tdc1190ReadEvent: ERROR: Invalid Header Word 0x%08x\n",header,0,0,0,0,0);
	  UNLOCK_1190;
	  return(-1);
	}
      else
	{
	  tdata[dCnt] = header;
	  dCnt++;
	  tmpData = vmeRead32(&(c1190p[id]->data[0]));
	  while((tmpData&V1190_DATA_TYPE_MASK) != V1190_TDC_EOB_DATA )
	    {
	      tdata[dCnt] = tmpData;
	      dCnt++;
	      tmpData = vmeRead32(&(c1190p[id]->data[0]));
	    }
	}

      /* reached EOB for TDC */
      trailer = tmpData;
      if((trailer&V1190_DATA_TYPE_MASK) != V1190_TDC_EOB_DATA) {
	logMsg("tdc1190ReadEvent: ERROR: Invalid Trailer Word 0x%08x\n",trailer,0,0,0,0,0);
	UNLOCK_1190;
	return(-1);
      }else{
	tdata[dCnt] = trailer;
	dCnt++;
      }
      UNLOCK_1190;
      return (dCnt);
      
    }else{
    logMsg("tdc1190ReadEvent: Data Not ready for readout!\n",0,0,0,0,0,0);
    UNLOCK_1190;
    return(0);
  }

}


/******************************************************************************
 *
 * tdc1190ReadData - available data into a buffer. 
 *
 *
 * RETURNS: Number of Data words read from the TDC (including Header/Trailer).
 */
/* FIXME: Make sure endian-ness is correct */

int
tdc1190ReadData(int id, UINT32 *tdata, int maxWords)
{
  int ii, jj, nWords, evID, bunchID, evCount, headFlag, trigMatch;
  UINT32 gheader, gtrailer, theader, ttrailer, tmpData, dCnt;

  CHECKID(id);

  if(maxWords==0) maxWords = 1024;

  /* Check if there is a valid event */
  LOCK_1190;

  if(vmeRead16(&(c1190p[id]->status)) & V1190_STATUS_DATA_READY)
    {
      dCnt = 0;
      headFlag  = vmeRead16(&(c1190p[id]->status)) & V1190_STATUS_HEADER_ENABLE;
      trigMatch = vmeRead16(&(c1190p[id]->status)) & V1190_STATUS_TRIG_MATCH;

      if(trigMatch) /* If trigger match mode then read individual event */
	{
	  /* Read Global Header - Get event count */
	  gheader = vmeRead32(&(c1190p[id]->data[0]));
	  if((gheader & V1190_DATA_TYPE_MASK) != V1190_GLOBAL_HEADER_DATA)
	    {
	      logMsg("tdc1190ReadData: ERROR: Invalid Global Header Word 0x%08x\n",
		     gheader,2,3,4,5,6);
	      UNLOCK_1190;
	      return(ERROR);
	    }
	  else
	    {
	      tdata[dCnt] = gheader;
	      evCount = (gheader & V1190_GHEAD_EVCOUNT_MASK) >> 5;
	      dCnt++;
	    }

	  /* Loop over four TDC chips and get data for each */
	  for(ii=0; ii<4; ii++)
	    {
	      /* Read TDC Header - Get event ID, Bunch ID */
	      theader = vmeRead32(&(c1190p[id]->data[0]));
	      if((theader&V1190_DATA_TYPE_MASK) != V1190_TDC_HEADER_DATA)
		{
		  logMsg("ERROR: Invalid TDC Header Word 0x%08x for TDC %d\n",
			 theader,ii,3,4,5,6);
		  UNLOCK_1190;
		  return(ERROR);
		}
	      else
		{
		  tdata[dCnt] = theader;
		  evID = (theader&V1190_TDCHEAD_EVID_MASK)>>12;
		  bunchID = (theader&V1190_TDCHEAD_BUNCHID_MASK);
		  dCnt++;
		}
	      jj=0;
	      tmpData = vmeRead32(&(c1190p[id]->data[0]));
	      while((tmpData&V1190_DATA_TYPE_MASK) != V1190_TDC_EOB_DATA)
		{
		  tdata[dCnt] = tmpData;
		  jj++;
		  dCnt++;
		  tmpData = vmeRead32(&(c1190p[id]->data[jj]));
		}

	      /* reached EOB for TDC */
	      ttrailer = tmpData;
	      if((ttrailer&V1190_DATA_TYPE_MASK) != V1190_TDC_EOB_DATA)
		{
		  logMsg("ERROR: Invalid TDC EOB Word 0x%08x for TDC %d\n",
			 ttrailer,ii,3,4,5,6);
		  UNLOCK_1190;
		  return(ERROR);
		}
	      else
		{
		  tdata[dCnt] = ttrailer;
		  nWords = (ttrailer&V1190_TDCEOB_WORDCOUNT_MASK);
		  dCnt++;
		}
	    }

	  /* next data word should be Global EOB */
	  gtrailer = vmeRead32(&(c1190p[id]->data[dCnt]));
	  if((gtrailer&V1190_DATA_TYPE_MASK) != V1190_GLOBAL_EOB_DATA)
	    {
	      logMsg("tdc1190ReadData: ERROR: Invalid Global EOB Word 0x%08x\n",
		     gtrailer,2,3,4,5,6);
	      UNLOCK_1190;
	      return(ERROR);
	    }
	  else
	    {
	      tdata[dCnt] = gtrailer;
	      nWords = (gtrailer&V1190_GEOB_WORDCOUNT_MASK)>>5;
	      dCnt++;
	    }
	}
      else /* Continuous Storage mode */
	{
	  tmpData = vmeRead32(&(c1190p[id]->data[dCnt]));
	  while(((tmpData&V1190_DATA_TYPE_MASK) != V1190_FILLER_DATA) &&
		(dCnt<maxWords))
	    {
	      tdata[dCnt]=tmpData;
	      dCnt++;
	      tmpData = vmeRead32(&(c1190p[id]->data[dCnt]));
	    }
	}
      UNLOCK_1190;
      return(dCnt);
    }
  else
    {
      logMsg("tdc1190ReadData: No data available for readout!\n",1,2,3,4,5,6);
      UNLOCK_1190;
      return(0);
    }
}

/******************************************************************************
 * ACQUISITION MODE OPCODES Routines
 *
 *  All routines return OK or ERROR,
 *   unless otherwise specified.
 * 
 * tdc1190SetTriggerMatchingMode
 * tdc1190SetContinuousStorageMode
 * tdc1190ReadAcquisitionMode         - Returns 0: Continuous storage mode
 *                                              1: Trigger matching mode
 * tdc1190SetKeepToken
 * tdc1190ClearKeepToken
 * tdc1190LoadDefaultConfiguration
 * tdc1190SaveUserConfiguration
 * tdc1190LoadUserConfiguration
 * tdc1190SetAutoLoadUserConfiguration
 * tdc1190SetAutoLoadDefaultConfiguration
 *
*/

STATUS
tdc1190SetTriggerMatchingMode(int id)
{
  STATUS status;
  CHECKID(id);
  status = tdc1190WriteMicro(id,0x0000);
  tdc1190Clear(id);
  return status;
}

STATUS
tdc1190SetContinuousStorageMode(int id)
{
  STATUS status;
  CHECKID(id);
  status = tdc1190WriteMicro(id,0x0100);
  tdc1190Clear(id);
  return status;
}

int
tdc1190ReadAcquisitionMode(int id)
{
  UINT16 tdata;
  int status;
  int rval;

  CHECKID(id);
  tdc1190WriteMicro(id,0x0200);
  status = tdc1190ReadMicro(id,&tdata,1);
  if(status==ERROR)
    {
      rval = ERROR;
    }
  else 
    {
      rval = tdata&0x1;
    }

  return rval;
}

STATUS
tdc1190SetKeepToken(int id)
{
  CHECKID(id);
  return tdc1190WriteMicro(id,0x0300);
}

STATUS
tdc1190ClearKeepToken(int id)
{
  CHECKID(id);
  return tdc1190WriteMicro(id,0x0400);
}

STATUS
tdc1190LoadDefaultConfiguration(int id)
{
  CHECKID(id);
  return tdc1190WriteMicro(id,0x0500);
}

STATUS
tdc1190SaveUserConfiguration(int id)
{
  CHECKID(id);
  return tdc1190WriteMicro(id,0x0600);
}

STATUS 
tdc1190LoadUserConfiguration(int id)
{
  CHECKID(id);
  return tdc1190WriteMicro(id,0x0700);
}

STATUS
tdc1190SetAutoLoadUserConfiguration(int id)
{
  CHECKID(id);
  return tdc1190WriteMicro(id,0x0800);
}

STATUS
tdc1190SetAutoLoadDefaultConfiguration(int id)
{
  CHECKID(id);
  return tdc1190WriteMicro(id,0x0900);
}




/******************************************************************************
 * TRIGGER MODE OPCODES Routines
 *
 *  All routines return OK or ERROR,
 *   unless otherwise specified.
 *  All parameters in tdc1190Set* are in steps of 25ns
 * 
 * tdc1190SetWindowWidth                - Set window width [25ns,51175ns]
 * tdc1190SetWindowOffset               - Set window offset > -800000ns
 * tdc1190SetExtraSearchMargin          - Set window extra search margin [0,1250ns]
 * tdc1190SetRejectMargin               - Set reject margin [0,1250ns]
 * tdc1190EnableTriggerTimeSubtraction
 * tdc1190DisableTriggerTimeSubtraction
 * tdc1190ReadTriggerConfiguration
 *
 */

STATUS
tdc1190SetWindowWidth(int id, UINT32 window_width)
{
  UINT16 tdata;

  CHECKID(id);
  tdc1190WriteMicro(id,0x1000);

  if(window_width < 25)         
    tdata = 1;
  else if (window_width > 51175) 
    tdata = 0x7FF;
  else
    tdata = window_width/25;

  printf("%s(%d): Set Window Width to %d ns\n",__FUNCTION__,id,tdata*25);

  return tdc1190WriteMicro(id,tdata);
}

STATUS
tdc1190SetWindowOffset(int id, INT32 window_offset)
{
  INT16 tdata;

  CHECKID(id);
  tdc1190WriteMicro(id,0x1100);

  if(window_offset < -800000) 
    tdata = -32000;
  else             
    tdata = window_offset/25;

  printf("%s(%d): Set Window Offset to %d ns\n",__FUNCTION__,id,tdata*25);

  return tdc1190WriteMicro(id,tdata);

}

STATUS
tdc1190SetExtraSearchMargin(int id, UINT32 window_extra)
{
  UINT16 tdata;

  CHECKID(id);
  tdc1190WriteMicro(id,0x1200);

  if(window_extra < 0)         
    tdata = 0;
  else if (window_extra > 1250) 
    tdata = 50;
  else               
    tdata = window_extra/25;
  printf("%s(%d): Set Extra Search Margin to %d ns\n",__FUNCTION__,id,tdata*25);

  return tdc1190WriteMicro(id,tdata);

}

STATUS
tdc1190SetRejectMargin(int id, UINT32 window_reject)
{
  UINT16 tdata;

  CHECKID(id);
  tdc1190WriteMicro(id,0x1300);

  if(window_reject < 0)
    tdata = 0;
  else if (window_reject > 1250) 
    tdata = 50;
  else               
    tdata = window_reject/25;

  printf("%s(%d): Set Reject Margin to %d ns\n",__FUNCTION__,id,tdata*25);

  return tdc1190WriteMicro(id,tdata);

}

STATUS
tdc1190EnableTriggerTimeSubtraction(int id)
{
  CHECKID(id);
  return tdc1190WriteMicro(id,0x1400);
}

STATUS
tdc1190DisableTriggerTimeSubtraction(int id)
{
  CHECKID(id);
  return tdc1190WriteMicro(id,0x1500);
}

STATUS
tdc1190ReadTriggerConfiguration(int id)
{
  INT16 tmp[5] = {0,0,0,0,0};
  CHECKID(id);

  EIEIO;
  SynC;
  tdc1190WriteMicro(id,0x1600);
  EIEIO;
  SynC;

  /*tdc1190ReadMicro(id,tmp,5);*/
  tdc1190ReadMicro(id,&tmp[0],1);
  tdc1190ReadMicro(id,&tmp[1],1);
  tdc1190ReadMicro(id,&tmp[2],1);
  tdc1190ReadMicro(id,&tmp[3],1);
  tdc1190ReadMicro(id,&tmp[4],1);

  EIEIO;
  SynC;

  printf(" %s(%d):\n",__FUNCTION__,id);
  printf("  Window Width              = %6d ns\n",tmp[0]*25);
  printf("  Window Offset             = %6d ns\n",tmp[1]*25);
  printf("  Extra Seach Margin        = %6d ns\n",tmp[2]*25);
  printf("  Reject Margin             = %6d ns\n",tmp[3]*25);
  printf("  Trigger Time Subtraction  = %6d\n",tmp[4]);

  return(OK);
}

/******************************************************************************
 * EDGE DETECTION AND RESOLUTION OPCODES Routines
 *
 *  All routines return OK or ERROR,
 *   unless otherwise specified.
 *
 * tdc1190SetEdgeDetectionConfig   
 * tdc1190ReadEdgeDetectionConfig - Returns:
 *                                    0 : Pair mode
 *                                    1 : Trailing Edge
 *                                    2 : Leading Edge
 *                                    3 : Both leading and trailing edge
 * tdc1190SetEdgeResolution
 * tdc1190GetEdgeResolution       - Returns edge resolution (ns)
 * tdc1190SetPairResolution 
 * tdc1190GetPairResolution       - Returns 2nd and 3rd argument (ns)
 * tdc1190SetDoubleHitResolution
 * tdc1190GetDoubleHitResolution  - Returns double hit resolution (ns)
 *
*/

STATUS
tdc1190SetEdgeDetectionConfig(int id, UINT16 setedge)
{
  UINT16 tdata=0;

  CHECKID(id);

  printf("%s(%d): ",__FUNCTION__,id);
  if(setedge==0)
    {
      tdata=0x0;
      printf("set pair mode\n");
    }
  else if(setedge==1)
    {
      tdata=0x1;
      printf("set trailing edge only\n");
    }
  else if(setedge==2)
    {
      tdata=0x2;
      printf("set leading edge only\n");
    }
  else
    {
      printf("set both leading and trailing edges\n");
      tdata=0x3;
    }

  tdc1190WriteMicro(id,0x2200);
  return tdc1190WriteMicro(id,tdata);

}

int
tdc1190ReadEdgeDetectionConfig(int id)
{
  UINT16 tdata;

  CHECKID(id);

  tdc1190WriteMicro(id,0x2300);
  tdc1190ReadMicro(id,&tdata,1);

  tdata &= 0x3;
  if(tdata<3)
    {
      printf(" %s(%d): Mode =",__FUNCTION__,id);
      if(tdata==0)
	printf(" pair mode\n");
      else if(tdata==1)
	printf(" trailing edge\n");
      else if(tdata==2)
	printf(" leading edge\n");
      else if(tdata==3)
	printf(" both leading and trailing\n");
    }
  else
    {
      printf("%s(%d) ERROR: tdata=%d (0x%x)\n",__FUNCTION__,id,tdata,tdata);
      return(ERROR);
    }

  return(tdata);
}

STATUS
tdc1190SetEdgeResolution(int id, UINT16 edge_res)
{
  UINT16 tdata;

  CHECKID(id);

  if(edge_res==800)      
    tdata = 0;
  else if(edge_res==200) 
    tdata = 1;
  else if(edge_res==100) 
    tdata = 2;
  else
    {
      printf("%s(%d): ERROR: Invalid edge_res=%d, must be 100, 200 or 800 ps\n",
	     __FUNCTION__,id,edge_res);
      return(ERROR);
    }

  tdc1190WriteMicro(id,0x2400);
  tdc1190WriteMicro(id,tdata);
  printf("%s(%d): Set Edge Resolution to %d ps\n",__FUNCTION__,id,edge_res);

  return(OK);
}

int
tdc1190GetEdgeResolution(int id)
{
  UINT16 tdata=0;
  int rval;

  CHECKID(id);

  tdc1190WriteMicro(id,0x2600);
  tdc1190ReadMicro(id,&tdata,1);

  tdata &= 0x3;
  if(tdata==0)
    rval = 800;
  else if(tdata==1)
    rval = 200;
  else if(tdata==2)
    rval = 100;
  else
    {
      if(use1190[id]==2) rval = 25;
      else {
	printf("%s(%d): ERROR: tdata=%d (0x%x)\n",__FUNCTION__,id,tdata,tdata);
	return(ERROR);
      }
    }
  /*
  printf("%s(%d): Edge Resolution is %d ps\n",__FUNCTION__,id,rval);
  */

  return(rval);
}

STATUS
tdc1190SetPairResolution(int id, UINT32 leading_res, UINT32 pulse_width_res)
{
  UINT16 tdata;

  CHECKID(id);

  if(leading_res==100)            tdata = 0x0;
  else if(leading_res==200)       tdata = 0x1;
  else if(leading_res==400)       tdata = 0x2;
  else if(leading_res==800)       tdata = 0x3;
  else if(leading_res==1600)      tdata = 0x4;
  else if(leading_res==3120)      tdata = 0x5;
  else if(leading_res==6250)      tdata = 0x6;
  else if(leading_res==12500)     tdata = 0x7;
  else
    {
      printf("%s(%d): ERROR: Invalid leading_res=%d\n",__FUNCTION__,id,
	     leading_res);
      return(ERROR);
    }

  if(pulse_width_res==100)         tdata += 0x0;
  else if(pulse_width_res==200)    tdata += 0x100;
  else if(pulse_width_res==400)    tdata += 0x200;
  else if(pulse_width_res==800)    tdata += 0x300;
  else if(pulse_width_res==1600)   tdata += 0x400;
  else if(pulse_width_res==3200)   tdata += 0x500;
  else if(pulse_width_res==6250)   tdata += 0x600;
  else if(pulse_width_res==12500)  tdata += 0x700;
  else if(pulse_width_res==25000)  tdata += 0x800;
  else if(pulse_width_res==50000)  tdata += 0x900;
  else if(pulse_width_res==100000) tdata += 0xA00;
  else if(pulse_width_res==200000) tdata += 0xB00;
  else if(pulse_width_res==400000) tdata += 0xC00;
  else if(pulse_width_res==800000) tdata += 0xD00;
  else
    {
      printf("%s(%d): ERROR: Invalid pulse_width_res=%d\n",__FUNCTION__,id,
	     pulse_width_res);
      return(ERROR);
    }

  tdc1190WriteMicro(id,0x2500);
  tdc1190WriteMicro(id,tdata);
  printf("%s(%d): Set Pair Resolution to %d(leading time), %d(width) ps\n",
	 __FUNCTION__,id,leading_res,pulse_width_res);

  return(OK);
}

STATUS
tdc1190GetPairResolution(int id, UINT32 *leading_res, UINT32 *pulse_width_res)
{
  UINT16 tdata, tdata1, tdata2;

  CHECKID(id);

  tdc1190WriteMicro(id,0x2600);
  tdc1190ReadMicro(id,&tdata,1);

  tdata1 = tdata & 0x7;
  if(tdata1 == 0x0)      *leading_res = 100;
  else if(tdata1 == 0x1) *leading_res = 200;
  else if(tdata1 == 0x2) *leading_res = 400;
  else if(tdata1 == 0x3) *leading_res = 800;
  else if(tdata1 == 0x4) *leading_res = 1600;
  else if(tdata1 == 0x5) *leading_res = 3120;
  else if(tdata1 == 0x6) *leading_res = 6250;
  else if(tdata1 == 0x7) *leading_res = 12500;
  else
    {
      printf("ERROR: GetPairResolution: tdata1=%d\n",tdata1);
      return(ERROR);
    }

  tdata2 = tdata & 0xF00;
  if(tdata2 == 0x0)        *pulse_width_res = 100;
  else if(tdata2 == 0x100) *pulse_width_res = 200;
  else if(tdata2 == 0x200) *pulse_width_res = 400;
  else if(tdata2 == 0x300) *pulse_width_res = 800;
  else if(tdata2 == 0x400) *pulse_width_res = 1600;
  else if(tdata2 == 0x500) *pulse_width_res = 3200;
  else if(tdata2 == 0x600) *pulse_width_res = 6250;
  else if(tdata2 == 0x700) *pulse_width_res = 12500;
  else if(tdata2 == 0x800) *pulse_width_res = 25000;
  else if(tdata2 == 0x900) *pulse_width_res = 50000;
  else if(tdata2 == 0xA00) *pulse_width_res = 100000;
  else if(tdata2 == 0xB00) *pulse_width_res = 200000;
  else if(tdata2 == 0xC00) *pulse_width_res = 400000;
  else if(tdata2 == 0xD00) *pulse_width_res = 800000;
  else
    {
      printf("ERROR: GetPairResolution: tdata2=%d\n",tdata2);
      return(ERROR);
    }
  printf("Pair Resolution is %d(leading time), %d(width) ps\n",*leading_res,*pulse_width_res);

  return(OK);
}

STATUS
tdc1190SetDoubleHitResolution(int id, UINT16 doublehit_res)
{
  UINT16 tdata;

  CHECKID(id);

  if(doublehit_res==5)        tdata = 0;
  else if(doublehit_res==10)  tdata = 1;
  else if(doublehit_res==30)  tdata = 2;
  else if(doublehit_res==100) tdata = 3;
  else
    {
      printf("%s(%d): ERROR: Invalid doublehit_res=%d, must be 5, 10, 30 or 100 ns\n",
	     __FUNCTION__,id,doublehit_res);
      return(ERROR);
    }

  tdc1190WriteMicro(id,0x2800);
  tdc1190WriteMicro(id,tdata);
  printf("%s(%d): Set Double Hit Resolution to %d ns\n",__FUNCTION__,id,doublehit_res);

  return(OK);
}

int
tdc1190GetDoubleHitResolution(int id)
{
  UINT16 tdata;
  int rval;

  CHECKID(id);

  tdc1190WriteMicro(id,0x2900);
  tdc1190ReadMicro(id,&tdata,1);

  tdata &= 0x3;
  if(tdata==0)      rval = 5;
  else if(tdata==1) rval = 10;
  else if(tdata==2) rval = 30;
  else if(tdata==3) rval = 100;
  else
    {
      printf("%s(%d): ERROR: tdata=%d (0x%x)\n",__FUNCTION__,id,tdata,tdata);
      return(ERROR);
    }
  printf("w%s(%d): Double Hit Resolution is %d ns\n",__FUNCTION__,id,rval);

  return(rval);
}

/******************************************************************************
 * TDC READOUT OPCODES Routines
 *
 *  All routines return OK or ERROR,
 *   unless otherwise specified.
 *
 * tdc1190EnableTDCHeaderAndTrailer
 * tdc1190DisableTDCHeaderAndTrailer
 * tdc1190GetTDCHeaderAndTrailer        - Returns 
 *                                         0 : header/trailer disabled
 *                                         1 : header/trailer enabled
 * tdc1190SetMaxNumberOfHitsPerEvent
 * tdc1190GetMaxNumberOfHitsPerEvent    - Returns maximum number of hits/event
 * tdc1190EnableTDCErrorMark
 * tdc1190DisableTDCErrorMark
 * tdc1190EnableTDCErrorBypass
 * tdc1190DisableTDCErrorBypass
 * tdc1190SetTDCErrorType
 * tdc1190GetTDCErrorType               - Returns mask of enabled error types
 * tdc1190SetEffectiveSizeOfReadoutFIFO
 * tdc1190GetEffectiveSizeOfReadoutFIFO - Returns number of words in readout FIFO
 *
 */

STATUS
tdc1190EnableTDCHeaderAndTrailer(int id)
{
  CHECKID(id);
  return tdc1190WriteMicro(id,0x3000);
}

STATUS
tdc1190DisableTDCHeaderAndTrailer(int id)
{
  CHECKID(id);
  return tdc1190WriteMicro(id,0x3100);
}

int
tdc1190GetTDCHeaderAndTrailer(int id)
{
  UINT16 tdata;

  CHECKID(id);

  tdc1190WriteMicro(id,0x3200);
  tdc1190ReadMicro(id,&tdata,1);

  tdata &= 0x1;
  if(tdata==0)
    printf("%s(%d): TDC Header/Trailer disabled\n",__FUNCTION__,id);
  else if(tdata==1)
    printf("%s(%d): TDC Header/Trailer enabled\n",__FUNCTION__,id);

  return(tdata);
}

STATUS
tdc1190SetMaxNumberOfHitsPerEvent(int id, UINT32 nhits)
{
  UINT16 tdata;

  CHECKID(id);

  tdc1190WriteMicro(id,0x3300);

  if(nhits==0)        tdata = 0;
  else if(nhits==1)   tdata = 1;
  else if(nhits==2)   tdata = 2;
  else if(nhits==4)   tdata = 3;
  else if(nhits==8)   tdata = 4;
  else if(nhits==16)  tdata = 5;
  else if(nhits==32)  tdata = 6;
  else if(nhits==64)  tdata = 7;
  else if(nhits==128) tdata = 8;
  else                tdata = 9;

  if(tdata==9)
    printf("%s(%d): Set Unlimited Number Of Hits Per Event\n",__FUNCTION__,id);
  else
    printf("%s(%d): Set Maximum Number Of Hits Per Event to %d\n",__FUNCTION__,id,nhits);

  return tdc1190WriteMicro(id,tdata);

}

int
tdc1190GetMaxNumberOfHitsPerEvent(int id)
{
  UINT16 tdata;
  int rval;

  CHECKID(id);

  tdc1190WriteMicro(id,0x3400);
  tdc1190ReadMicro(id,&tdata,1);

  tdata &= 0xF;
  if(tdata==0)      rval = 0;
  else if(tdata==1) rval = 1;
  else if(tdata==2) rval = 2;
  else if(tdata==3) rval = 4;
  else if(tdata==4) rval = 8;
  else if(tdata==5) rval = 16;
  else if(tdata==6) rval = 32;
  else if(tdata==7) rval = 64;
  else if(tdata==8) rval = 128;
  else if(tdata==9) rval = 9999;
  else
    {
      printf("%s(%d): ERROR: tdata=%d (0x%x)\n",__FUNCTION__,id,tdata,tdata);
      return(ERROR);
    }

  return(rval);
}

STATUS
tdc1190EnableTDCErrorMark(int id)
{
  CHECKID(id);
  return tdc1190WriteMicro(id,0x3500);
}

STATUS
tdc1190DisableTDCErrorMark(int id)
{
  CHECKID(id);
  return tdc1190WriteMicro(id,0x3600);
}

STATUS
tdc1190EnableTDCErrorBypass(int id)
{
  CHECKID(id);
  return tdc1190WriteMicro(id,0x3700);
}

STATUS
tdc1190DisableTDCErrorBypass(int id)
{
  CHECKID(id);
  return tdc1190WriteMicro(id,0x3800);
}

STATUS
tdc1190SetTDCErrorType(int id, UINT32 mask)
{
  UINT16 tdata;

  CHECKID(id);

  tdata = mask & 0x7FF;
  tdc1190WriteMicro(id,0x3900);
  return tdc1190WriteMicro(id,tdata);

}

int
tdc1190GetTDCErrorType(int id)
{
  UINT16 tdata;

  CHECKID(id);

  tdc1190WriteMicro(id,0x3A00);
  tdc1190ReadMicro(id,&tdata,1);

  tdata &= 0x7FF;
  if(tdata & 0x1)
    printf("%s(%d): Vernier error\n",__FUNCTION__,id);
  else if(tdata & 0x2)
    printf("%s(%d): Coarse error\n",__FUNCTION__,id);
  else if(tdata & 0x4)
    printf("%s(%d): Channel select error\n",__FUNCTION__,id);
  else if(tdata & 0x8)
    printf("%s(%d): L1 buffer parity error\n",__FUNCTION__,id);
  else if(tdata & 0x10)
    printf("%s(%d): Trigger fifo parity error\n",__FUNCTION__,id);
  else if(tdata & 0x20)
    printf("%s(%d): Trigger matching error\n",__FUNCTION__,id);
  else if(tdata & 0x40)
    printf("%s(%d): Readout fifo parity error\n",__FUNCTION__,id);
  else if(tdata & 0x80)
    printf("%s(%d): Readout state error\n",__FUNCTION__,id);
  else if(tdata & 0x100)
    printf("%s(%d): Set up parity error\n",__FUNCTION__,id);
  else if(tdata & 0x200)
    printf("%s(%d): Control parity error\n",__FUNCTION__,id);
  else if(tdata & 0x400)
    printf("%s(%d): Jtag instruction parity error\n",__FUNCTION__,id);

  return(tdata);
}

STATUS
tdc1190SetEffectiveSizeOfReadoutFIFO(int id, UINT32 nwords)
{
  UINT16 tdata;

  CHECKID(id);

  tdc1190WriteMicro(id,0x3B00);

  if(nwords==2)        tdata = 0;
  else if(nwords==4)   tdata = 1;
  else if(nwords==8)   tdata = 2;
  else if(nwords==16)  tdata = 3;
  else if(nwords==32)  tdata = 4;
  else if(nwords==64)  tdata = 5;
  else if(nwords==128) tdata = 6;
  else if(nwords==256) tdata = 7;
  else                 tdata = 7;

  printf("%s(%d): Set Effective Size Of Readout FIFO to %d\n",__FUNCTION__,id,nwords);

  return tdc1190WriteMicro(id,tdata);

}

int
tdc1190GetEffectiveSizeOfReadoutFIFO(int id)
{
  UINT16 tdata;

  CHECKID(id);

  tdc1190WriteMicro(id,0x3C00);
  tdc1190ReadMicro(id,&tdata,1);

  tdata &= 0x7;
  if(tdata==0)
    printf("%s(%d): 2 words\n",__FUNCTION__,id);
  else if(tdata==1)
    printf("%s(%d): 4 words\n",__FUNCTION__,id);
  else if(tdata==2)
    printf("%s(%d): 8 words\n",__FUNCTION__,id);
  else if(tdata==3)
    printf("%s(%d): 16 words\n",__FUNCTION__,id);
  else if(tdata==4)
    printf("%s(%d): 32 words\n",__FUNCTION__,id);
  else if(tdata==5)
    printf("%s(%d): 64 words\n",__FUNCTION__,id);
  else if(tdata==6)
    printf("%s(%d): 128 words\n",__FUNCTION__,id);
  else if(tdata==7)
    printf("%s(%d): 256 words\n",__FUNCTION__,id);

  return(tdata);
}

/******************************************************************************
 * CHANNEL ENABLE OPCODES Routines
 *
 *  All routines return OK or ERROR,
 *   unless otherwise specified.
 *
 * tdc1190EnableChannel
 * tdc1190DisableChannel
 * tdc1190EnableAllChannels
 * tdc1190DisableAllChannels
 * tdc1190EnableChannels
 * tdc1190GetChannels        - Returns mask of enabled channels
 *
*/


STATUS
tdc1190EnableChannel(int id, UINT16 channel)
{
  int opcode = 0x4000 + (channel & 0x7F);

  CHECKID(id);

  return tdc1190WriteMicro(id,opcode);
}

STATUS
tdc1190DisableChannel(int id, UINT16 channel)
{
  int opcode = 0x4100 + (channel & 0x7F);

  CHECKID(id);
  return tdc1190WriteMicro(id,opcode);
}

STATUS
tdc1190EnableAllChannels(int id)
{
  CHECKID(id);
  return tdc1190WriteMicro(id,0x4200);
}

STATUS
tdc1190DisableAllChannels(int id)
{
  CHECKID(id);
  tdc1190WriteMicro(id,0x4300);
  return(OK);
}

STATUS
tdc1190EnableChannels(int id, UINT16 channels[8])
{
  int i;
  CHECKID(id);

  tdc1190WriteMicro(id,0x4400);
  for(i=0; i<8; i++) tdc1190WriteMicro(id,channels[i]);
  return(OK);
}

STATUS
tdc1190GetChannels(int id, UINT16 channels[8])
{
  int i;
  CHECKID(id);

  tdc1190WriteMicro(id,0x4500);
  for(i=0; i<8; i++) tdc1190ReadMicro(id,&channels[i],1);
  return(OK);
}



/******************************************************************************
 * ADVANCED OPCODES Routines
 *
 *  All routines return OK or ERROR,
 *   unless otherwise specified.
 *
 * tdc1190GetTDCError   - Returns internal error from selected TDC
 * tdc1190GetTDCDLLLock - Returns 
 *                         0 : DLL not in LOCK
 *                         1 : DLL in LOCK
 * 
*/


int
tdc1190GetTDCError(int id, UINT16 ntdc)
{
  int opcode = 0x7400 + (ntdc & 0x3);
  unsigned short tmp;
  int rval;

  CHECKID(id);

  tdc1190WriteMicro(id,opcode);
  tdc1190ReadMicro(id,&tmp,1);

  rval = tmp & 0x7FF;

  return(rval);
}

int
tdc1190GetTDCDLLLock(int id, UINT16 ntdc)
{
  int opcode = 0x7500 + (ntdc & 0x3);
  unsigned short tmp;
  int rval;

  CHECKID(id);

  tdc1190WriteMicro(id,opcode);
  tdc1190ReadMicro(id,&tmp,1);

  rval = tmp & 0x1;

  return(rval);
}

/******************************************************************************
 *
 * tdc1190InitMSCT     - Initialize MSCT for all initialized boards
 *                       vmeaddress is set to the MSCT address
 *                       Returns OK, if successful.  ERROR, otherwise.
 *
 */

STATUS
tdc1190InitMCST(UINT32 *vmeaddress)
{
  unsigned int baseadrs;
  int ii;

  baseadrs = (V1190_A32_ADDR >> 24) & 0xFF;
  baseadrs += (V1190_A32_MCST_OFFSET >> 24);

#ifdef VXWORKS
  sysBusToLocalAdrs(0x09,(char *)(baseadrs<<24),(char **)vmeaddress);
#else
  vmeBusToLocalAdrs(0x09,(char *)(baseadrs<<24),(char **)vmeaddress);
#endif
  c1190MCSTp = (struct v1190_struct *)(vmeaddress);

  printf("tdc1190InitMCST: MCST VME (USER) base address 0x%x (0x%x):\n",
	 (baseadrs<<24), (UINT32)c1190MCSTp);

  /* Loop through use1190/c1190p array to set mcstBaseAddr
     and set First/intermediate/Last boards */
  for(ii=0; ii<V1190_MAX_MODULES; ii++)
    {
      if(use1190[ii] == 0)
	continue;

      vmeWrite16(&(c1190p[ii]->mcstBaseAddr),(baseadrs & V1190_MCSTBASEADDR_MASK));

      if(ii==tdcMinSlot)
	{
	  vmeWrite16(&(c1190p[ii]->mcstCtrl),2); /* active first */
	  printf("\tFirst  board at 0x%08x\n",(UINT32)c1190p[ii]);
	}
      else if(ii==tdcMaxSlot)
	{
	  vmeWrite16(&(c1190p[ii]->mcstCtrl),1); /* active last */
	  printf("\tLast   board at 0x%08x\n",(UINT32)c1190p[ii]);
	}
      else
	{
	  vmeWrite16(&(c1190p[ii]->mcstCtrl),3); /* active intermediate */
	  printf("\tMiddle board at 0x%08x\n",(UINT32)c1190p[ii]);
	}
    }

  return(OK);
}


/******************************************************************************
 *
 * tdc1190Trig         - Issue Software trigger to TDC
 * tdc1190Clear        - Clear TDC
 * tdc1190Reset        - Clear/Reset TDC
 * tdc1190GetEventCounter - Return number of triggers since last clear/reset.
 * tdc1190GetEventStored  - Return number of events currently stored in 
 *                          output buffer
 * tdc1190SetAlmostFullLevel - Set value of number of words in output buffer
 *                             when an IRQ is generated (if enabled) and
 *                             correspond bit in Status reg is set.
 * tdc1190SetOutProg - Set the function for the output on the control connector
 *                     0 : Data Ready
 *                     1 : Full
 *                     2 : Almost Full
 *                     3 : Error
 * tdc1190GetOutProg - Get value set for the output on the control connector.
 * tdc1190TestMode   - Enable (off=0) or disable (off=1) Test mode
 * tdc1190Test       - Set a value (tval) into the Testreg register
 * tdc1190SetGeoAddress - Set the GEO Address (contained in header/trailer words)
 * tdc1190GetGeoAddress - Get the GEO Address (contained in header/trailer words)
 *
 */

STATUS
tdc1190Trig(int id)
{
  CHECKID(id);
  LOCK_1190;
  vmeWrite16(&(c1190p[id]->trigger),1);
  UNLOCK_1190;
  return OK;
}

STATUS
tdc1190Clear(int id)
{
  CHECKID(id);
  LOCK_1190;
  vmeWrite16(&(c1190p[id]->clear),1);
  UNLOCK_1190;
  return OK;
}

STATUS
tdc1190Reset(int id)
{
  CHECKID(id);
  LOCK_1190;
  vmeWrite16(&(c1190p[id]->moduleReset),1);
  UNLOCK_1190;
  return OK;
}

int
tdc1190GetEventCounter(int id)
{
  int rval;

  CHECKID(id);
  LOCK_1190;
  rval = vmeRead32(&(c1190p[id]->evCount));
  UNLOCK_1190;
  return rval;
}

int
tdc1190GetEventStored(int id)
{
  int rval;

  CHECKID(id);
  LOCK_1190;
  rval = vmeRead16(&(c1190p[id]->evStored));
  UNLOCK_1190;
  return rval;
}

STATUS
tdc1190SetAlmostFullLevel(int id, UINT16 nwords)
{
  CHECKID(id);
  LOCK_1190;
  vmeWrite16(&(c1190p[id]->almostFullLevel),(nwords & 0x7FFF));
  UNLOCK_1190;
  return OK;
}

int
tdc1190GetAlmostFullLevel(int id)
{
  int rval;

  CHECKID(id);
  LOCK_1190;
  rval = vmeRead16(&(c1190p[id]->almostFullLevel));
  printf(" Almost Full Level set to %d (0x%04x) words\n",rval,rval);
  UNLOCK_1190;
  return rval;
}


STATUS
tdc1190SetOutProg(int id, UINT16 code)
{
  CHECKID(id);
  LOCK_1190;
  vmeWrite16(&(c1190p[id]->outProgControl),code & 0x3);
  UNLOCK_1190;
  return OK;
}

int
tdc1190GetOutProg(int id)
{
  int rval;

  CHECKID(id);
  LOCK_1190;
  rval = vmeRead16(&(c1190p[id]->outProgControl));
  UNLOCK_1190;
  return rval;
}

STATUS
tdc1190TestMode(int id, int off)
{
  CHECKID(id);
  /* FIXME: Test mode enabled/disabled through OPCODE */

  if(off) 
    vmeWrite16(&(c1190p[id]->control),
	       vmeRead16(&(c1190p[id]->control)) & (~V1190_TESTMODE&0xf));
  else
    vmeWrite16(&(c1190p[id]->control),
	       vmeRead16(&(c1190p[id]->control)) | (V1190_TESTMODE));

  return OK;
}

STATUS
tdc1190Test(int id, UINT16 tval)
{
  /* FIXME: Test mode enabled/disabled through OPCODE */
  UINT16 testMode=0;

  CHECKID(id);

  testMode = vmeRead16(&(c1190p[id]->control))&V1190_TESTMODE;
  if(testMode)
    {
      if(tval==0)
	vmeWrite32(&(c1190p[id]->testReg),0x11223344);
      else
	vmeWrite32(&(c1190p[id]->testReg),tval);
      /* FIXME: Maybe the below is better than above... I dont know... fix it later */
      /* 	(UINT16 *)(c1190p[id]->testReg) = tval; */
    }
  else
    {
      printf("tdc1190Test: ERROR: TestMode not enabled.\n");
    }
     
  return OK;
}


STATUS
tdc1190SetGeoAddress(int id, UINT16 geoaddr)
{
  CHECKID(id);
  LOCK_1190;
  vmeWrite16(&(c1190p[id]->geoAddr),geoaddr & 0x1F);
  UNLOCK_1190;
  return OK;
}

int
tdc1190GetGeoAddress(int id)
{
  int rval;

  CHECKID(id);
  LOCK_1190;
  rval = vmeRead16(&(c1190p[id]->geoAddr)) & 0x1F;
  UNLOCK_1190;
  return rval;
}


STATUS
tdc1190SetBLTEventNumber(int id, UINT16 nevents)
{
  CHECKID(id);
  blt_Events = nevents & 0xFF;
  LOCK_1190;
  vmeWrite16(&(c1190p[id]->bltEventNumber), blt_Events);
  UNLOCK_1190;
  printf("tdc1190SetBLTEventNumber: set blt event number to %d\n",blt_Events);
  return OK;
}

int
tdc1190GetBLTEventNumber(int id)
{
  int rval;

  CHECKID(id);
  LOCK_1190;
  rval = vmeRead16(&(c1190p[id]->bltEventNumber));
  UNLOCK_1190;
  return rval;
}




/***********************/
/* INTERRUPT FUNCTIONS */
/***********************/


/*******************************************************************************
 *
 * tdc1190Int - default interrupt handler
 *
 * This rountine handles the v1190 TDC interrupt.  A user routine is
 * called, if one was connected by tdc1190IntConnect().
 *
 * RETURNS: N/A
 *
 */

LOCAL void 
tdc1190Int (void)
{

  UINT32 nevt=0;
  
  /* Disable interrupts */
#ifdef VXWORKS
  sysIntDisable(c1190IntLevel);
#else
  vmeBusLock();
#endif

  c1190IntCount++;
 
  if (c1190IntRoutine != NULL)  {     /* call user routine */
    (*c1190IntRoutine) (c1190IntArg);
  }else{
    if((c1190IntID<0) || (c1190p[c1190IntID] == NULL)) {
      logMsg("c1190Int: ERROR : TDC id %d not initialized \n",c1190IntID,0,0,0,0,0);
      return;
    }

    logMsg("c1190Int: Processed %d events\n",nevt,0,0,0,0,0);

  }

  /* Enable interrupts */
#ifdef VXWORKS
  sysIntEnable(c1190IntLevel);
#else
  vmeBusUnlock();
#endif

}



/*******************************************************************************
 *
 * c1190IntConnect - connect a user routine to the v1190 TDC interrupt
 *
 * This routine specifies the user interrupt routine to be called at each
 * interrupt. 
 *
 * RETURNS: OK, or ERROR if Interrupts are enabled
 */

STATUS 
tdc1190IntConnect (VOIDFUNCPTR routine, int arg, UINT16 level, UINT16 vector)
{

  if(c1190IntRunning) {
    printf("c1190IntConnect: ERROR : Interrupts already Initialized for TDC id %d\n",
	   c1190IntID);
    return(ERROR);
  }
  
  c1190IntRoutine = routine;
  c1190IntArg = arg;

  /* Check for user defined VME interrupt level and vector */
  if(level == 0) {
    c1190IntLevel = V1190_VME_INT_LEVEL; /* use default */
  }else if (level > 7) {
    printf("c1190IntConnect: ERROR: Invalid VME interrupt level (%d). Must be (1-7)\n",level);
    return(ERROR);
  } else {
    c1190IntLevel = level;
  }

  if(vector == 0) {
    c1190IntVec = V1190_INT_VEC;  /* use default */
  }else if ((vector < 32)||(vector>255)) {
    printf("c1190IntConnect: ERROR: Invalid interrupt vector (%d). Must be (32<vector<255)\n",vector);
    return(ERROR);
  }else{
    c1190IntVec = vector;
  }
      
  /* Connect the ISR */
#ifdef VXWORKSPPC
  if((intDisconnect((int)INUM_TO_IVEC(c1190IntVec)) != 0)) {
    printf("c1190IntConnect: ERROR disconnecting Interrupt\n");
    return(ERROR);
  }
  if((intConnect(INUM_TO_IVEC(c1190IntVec),tdc1190Int,0)) != 0) {
    printf("c1190IntConnect: ERROR in intConnect()\n");
    return(ERROR);
  }
#else
  if(vmeIntConnect(c1190IntVec,c1190IntLevel,tdc1190Int,c1190IntArg) != OK)
    {
      printf("%s: ERROR: vmeIntConnect failed\n");
      return ERROR;
    }
#endif

  return (OK);
}


/*******************************************************************************
 *
 * c1190IntEnable - Enable interrupts from specified TDC
 *
 * Enables interrupts for a specified TDC.
 * 
 * RETURNS OK or ERROR if TDC is not available or parameter is out of range
 */

STATUS 
tdc1190IntEnable (int id, UINT16 evCnt)
{

  if(c1190IntRunning) {
    printf("c1190IntEnable: ERROR : Interrupts already initialized for TDC id %d\n",
	   c1190IntID);
    return(ERROR);
  }

  if((id<0) || (c1190p[id] == NULL)) {
    printf("c1190IntEnable: ERROR : TDC id %d not initialized \n",id);
    return(ERROR);
  }else{
    c1190IntID = id;
  }
  
  
#ifdef VXWORKS
  sysIntEnable(c1190IntLevel);   /* Enable VME interrupts */
#endif  

  /* Zero Counter and set Running Flag */
  c1190IntEvCount = evCnt;
  c1190IntCount = 0;
  c1190IntRunning = TRUE;
  
  return(OK);
}


/*******************************************************************************
 *
 * c1190IntDisable - disable the TDC interrupts
 *
 * RETURNS: OK, or ERROR if not initialized
 */

STATUS 
tdc1190IntDisable (int iflag)
{

  if((c1190IntID<0) || (c1190p[c1190IntID] == NULL)) {
    logMsg("c1190IntDisable: ERROR : TDC id %d not initialized \n",c1190IntID,0,0,0,0,0);
    return(ERROR);
  }

#ifdef VXWORKS
  sysIntDisable(c1190IntLevel);   /* Disable VME interrupts */
#endif

  /* Tell tasks that Interrupts have been disabled */
  if(iflag > 0) {
    c1190IntRunning = FALSE;
  }
  
  return (OK);
}


#else /* dummy version*/

void
tdc1190_dummy()
{
  return;
}

#endif
