
#if defined(VXWORKS) || defined(Linux_vme)

/***************************************************************************
                          main.c  -  Scaler Readout
                             -------------------
    begin                : Thu Sep 14 15:52:00 UTC 2002
    copyright            : (C) 2002 by Tanest Chinwanawich
    email                : tanest@jlab.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#ifdef VXWORKS
#include <vxWorks.h>
#include <sysLib.h>
#include <taskLib.h>
#include <intLib.h>
#include <logLib.h>
#include <vxLib.h>
#else
#include <unistd.h>
#endif

#include <jvme.h>
#include "sis3801.h"

static int Nsis = 0;                            /* Number of SISs in Crate */
static volatile SIS3801 *sisp[SIS_MAX_SLOTS+1]; /* pointers to SIS A24 memory map */
static unsigned int sisA24Offset = 0;           /* Offset between VME A24 and Local address space */

#ifdef VXWORKSPPC
IMPORT STATUS intDisconnect(int);
#endif

/* Mutex for thread safe read/writes */
pthread_mutex_t   sis3801Mutex = PTHREAD_MUTEX_INITIALIZER;
#define SLOCK   if(pthread_mutex_lock(&sis3801Mutex)<0) perror("pthread_mutex_lock");
#define SUNLOCK if(pthread_mutex_unlock(&sis3801Mutex)<0) perror("pthread_mutex_unlock");

LOCAL BOOL scalIntRunning = FALSE;
LOCAL VOIDFUNCPTR scalIntRoutine = NULL;
LOCAL int scalIntArg = (int) NULL;
LOCAL UINT32 scalIntLevel = SCAL_VME_INT_LEVEL;
LOCAL UINT32 scalIntVec = SCAL_INT_VEC;


unsigned long addr;              /* Local address of scaler */
volatile struct fifo_scaler *pf = NULL; /* Pointer to scaler reg map */
volatile UINT32 scalData[32];

/* Set of helper functions */

int
sis3801CheckAddresses()
{
  int rval = OK;
  unsigned long offset=0, expected=0, base=0;
  struct fifo_scaler fbase;

  printf("%s:\n\t ---------- Checking sis3801 address space ---------- \n",
	 __FUNCTION__);

  base = (unsigned long) &fbase;

  offset = ((unsigned long) &fbase.write_to_fifo) - base;
  expected = 0x10;
  if(offset != expected)
    {
      printf("%s: ERROR ->write_to_fifo not at offset = 0x%lx (@ 0x%lx)\n",
	     __FUNCTION__,expected,offset);
      rval = ERROR;
    }

  offset = ((unsigned long) &fbase.enable_ref1) - base;
  expected = 0x50;
  if(offset != expected)
    {
      printf("%s: ERROR ->enable_ref1 not at offset = 0x%lx (@ 0x%lx)\n",
	     __FUNCTION__,expected,offset);
      rval = ERROR;
    }

  offset = ((unsigned long) &fbase.fifo[0]) - base;
  expected = 0x100;
  if(offset != expected)
    {
      printf("%s: ERROR ->fifo[0] not at offset = 0x%lx (@ 0x%lx)\n",
	     __FUNCTION__,expected,offset);
      rval = ERROR;
    }

  return rval;
}


int
sis3801readfifo(unsigned long addr)
{
  int rval = 0;
  struct fifo_scaler *ptr = (struct fifo_scaler *) addr;
  SLOCK;
  rval = vmeRead32(&ptr->fifo[0]);
  SUNLOCK;
  return rval;
}

void
sis3801writefifo(unsigned long addr, int value)
{
  struct fifo_scaler *ptr = (struct fifo_scaler *) addr;
  SLOCK;
  vmeWrite32(&ptr->write_to_fifo, value);
  SUNLOCK;
}

int
sis3801status(unsigned long addr)
{
  int rval = 0;
  struct fifo_scaler *ptr = (struct fifo_scaler *) addr;
  SLOCK;
  rval = vmeRead32(&ptr->csr);
  SUNLOCK;
  return rval;
}

void
sis3801control(unsigned long addr, int value)
{
  struct fifo_scaler *ptr = (struct fifo_scaler *) addr;
  SLOCK;
  vmeWrite32(&ptr->csr, value);
  SUNLOCK;
}

void
sis3801mask(unsigned long addr, int value)
{
  struct fifo_scaler *ptr = (struct fifo_scaler *) addr;
  SLOCK;
  vmeWrite32(&ptr->copy_disable, value);
  SUNLOCK;
}

void
sis3801clear(unsigned long addr)
{
  struct fifo_scaler *ptr = (struct fifo_scaler *) addr;
  SLOCK;
  vmeWrite32(&ptr->clear, 0x1);
  SUNLOCK;
}

void
sis3801nextclock(unsigned long addr)
{
  struct fifo_scaler *ptr = (struct fifo_scaler *) addr;
  SLOCK;
  vmeWrite32(&ptr->next, 0x1);
  SUNLOCK;
}

void
sis3801enablenextlogic(unsigned long addr)
{
  struct fifo_scaler *ptr = (struct fifo_scaler *) addr;
  SLOCK;
  vmeWrite32(&ptr->enable, 0x1);
  SUNLOCK;
}

void
sis3801disablenextlogic(unsigned long addr)
{
  struct fifo_scaler *ptr = (struct fifo_scaler *) addr;
  SLOCK;
  vmeWrite32(&ptr->disable, 0x1);
  SUNLOCK;
}

void
sis3801reset(unsigned long addr)
{
  struct fifo_scaler *ptr = (struct fifo_scaler *) addr;
  SLOCK;
  vmeWrite32(&ptr->reset, 0x1);
  SUNLOCK;
}

void
sis3801testclock(unsigned long addr)
{
  struct fifo_scaler *ptr = (struct fifo_scaler *) addr;
  SLOCK;
  vmeWrite32(&ptr->test, 0x1);
  SUNLOCK;
}

void
sis3801setlneprescaler(unsigned long addr, unsigned int prescale)
{
  struct fifo_scaler *ptr = (struct fifo_scaler *) addr;
  if(prescale > 0xffffff)
    {
      printf("%s: ERROR: Invalid prescale (%d)\n",
	     __func__, prescale);
      return;
    }

  SLOCK;
  vmeWrite32(&ptr->prescale, prescale);
  SUNLOCK;
}

void
sis3801Uledon(unsigned long addr)
{
  struct fifo_scaler *ptr = (struct fifo_scaler *) addr;
  SLOCK;
  vmeWrite32(&ptr->csr, 0x1);
  SUNLOCK;
}

void
sis3801Uledoff(unsigned long addr)
{
  struct fifo_scaler *ptr = (struct fifo_scaler *) addr;
  SLOCK;
  vmeWrite32(&ptr->csr, 0x100);
  SUNLOCK;
}

int
sis3801almostread(unsigned long addr, unsigned int *value)
{
  unsigned int *outbuf;
  outbuf = value;
  while (!(sis3801status(addr) & FIFO_ALMOST_EMPTY))
    *outbuf++ = sis3801readfifo(addr);
  return ((int) ((int)outbuf - (int)value));
}

int
sis3801read(unsigned long addr, unsigned int *value)
{
  unsigned int *outbuf;
  int len;
  
  outbuf = value;
  len = *value;

  while ((sis3801status(addr) & FIFO_EMPTY) == 0)
    {
      *outbuf++ = sis3801readfifo(addr);
      if(--len == 0)
	break;
    }
  return (int)((int)outbuf - (int)value);

}

/**************************************************************************/

LOCAL void
scalInt(void)
{
  int mask;

  SLOCK;
  mask = vmeRead32(&pf->csr) & ENABLE_IRQ;
  vmeWrite32(&pf->csr, DISABLE_IRQ);
  SUNLOCK;

  if  (scalIntRoutine != NULL)
    (*scalIntRoutine) (scalIntArg);

  SLOCK;
  vmeWrite32(&pf->csr, mask);
  SUNLOCK;
}

/*************************************************************************/

STATUS
scalIntInit(UINT32 addr, UINT32 level, UINT32 vector)
{
  if (scalIntRunning)
    {
      printf("scalIntInit:ERROR call scalIntDiable() first\n");
      return (ERROR);
    }

#ifdef  VXWORKSPPC
  intEnable(scalIntVec);
  sysIntEnable(scalIntLevel);
#endif

  memset((char *) scalData, 0, sizeof(scalData));

  return (OK);
}

/*************************************************************************/

STATUS scalIntConnect(VOIDFUNCPTR routine, int arg)
{
  int status;
  scalIntRoutine = routine;
  scalIntArg = arg;

#ifdef VXWORKSPPC
  /* Disconnect any current interrupts */
  status = intDisconnect(scalIntVec);
  if(status != OK)
    {
      printf("%s: Error disconnecting Interrupt\n",__func__);
      return ERROR;
    }
#endif

#ifdef VXWORKSPPC
  intConnect((VOIDFUNCPTR *)scalIntVec,scalInt,scalIntArg);
#else
  status = vmeIntConnect (scalIntVec, scalIntLevel,
			  scalInt, scalIntArg);
  if (status != OK)
    {
      printf("%s: vmeIntConnect failed with status = 0x%08x\n",
	     __func__,status);
      return(ERROR);
    }
#endif
  
  return (OK);
}

int
scalIntDisconnect()
{
  int status;
#ifdef VXWORKSPPC
  /* Disconnect any current interrupts */
  sysIntDisable(scalIntLevel);
  status = intDisconnect(scalIntVec);
#else
  status = vmeIntDisconnect(scalIntLevel);
#endif

  if (status != OK)
    {
      printf("%s: Error disconnecting Interrupt\n",__func__);
      return ERROR;
    }
  return OK;
}

/*************************************************************************/

STATUS
scalIntEnable(UINT32 mask)
{
  if ((mask == 0) || (mask > 0xf))
    {
      printf("VME Interrupt mask=0x%x is out of range\n", mask);
      return (ERROR);
    }
  else
    {
      if (pf)
	{
	  SLOCK;
	  vmeWrite32(&pf->csr, DISABLE_IRQ);
	  vmeWrite32(&pf->irq, (0x800) | (scalIntLevel << 8) | scalIntVec);
	  vmeWrite32(&pf->csr, mask << 20);
	  SUNLOCK;
	}
      else
	{
	  printf("scalInt not Initialized. call scalIntInit();\n");

	  return (ERROR);
	}
    }
  return (OK);

}

/*************************************************************************/

STATUS
scalIntDisable(void)
{
  if (pf)
    {
      SLOCK;
      vmeWrite32(&pf->csr, DISABLE_IRQ);
      scalIntRunning = FALSE;
      SUNLOCK;
    }
  else
    {
      printf("scalInt not Initialized. Call scalIntInit()!\n");
      return (ERROR);
    }

  return (OK);
}

/************************************************************************/

int
store(unsigned long addr, int mask)
{
  logMsg("store data\n", 1, 2, 3, 4, 5, 6);
  sis3801reset(addr);
  sis3801clear(addr);
  sis3801enablenextlogic(addr);
  sis3801nextclock(addr);
  sis3801control(addr, ENABLE_TEST);
  sis3801status(addr);
  sis3801nextclock(addr);
  sis3801testclock(addr);
  sis3801control(addr, DISABLE_TEST);
  sis3801Uledon(addr);

  return (0);
}


/****************************/
/* level 2 scaler functions */
/****************************/

static char lookup[30][10] = {
  "[S1R1 AX]", "[S1R2 AX]", "[S1R3 AX]", "[S2R1 AX]", "[S2R2 AX]",
    "[S2R3 AX]",
  "[S3R1 AX]", "[S3R2 AX]", "[S3R3 AX]", "[S4R1 AX]", "[S4R2 AX]",
    "[S4R3 AX]",
  "[S5R1 AX]", "[S5R2 AX]", "[S5R3 AX]", "[S6R1 AX]", "[S6R2 AX]",
    "[S6R3 AX]",
  "[S1R2 ST]", "[S1R3 ST]", "[S2R2 ST]", "[S2R3 ST]", "[S3R2 ST]",
    "[S3R3 ST]",
  "[S4R2 ST]", "[S4R3 ST]", "[S5R2 ST]", "[S5R3 ST]", "[S6R2 ST]", "[S6R3 ST]"
};




int
l2_init()
{
  int res;
#ifdef VXWORKS
  res = sysBusToLocalAdrs(0x39, (char *)L2SCALER, (char **) &addr);
#else
  res = vmeBusToLocalAdrs(0x39, (char *)L2SCALER, (char **) &addr);
#endif

  if(res != 0)
    {
      printf("%s: ERROR: cannot obtain local address from 0x%08x\n",
	     __func__, L2SCALER);
      return ERROR;
    }

  printf("l2_init: scaler address = 0x%08x (0x%lx)\n",
	 L2SCALER, addr);

  pf = (volatile struct fifo_scaler *)addr;
  
  return OK;
}


int
sis3801Init(unsigned int addr, unsigned int addr_inc, int nsis, int iFlag)
{
  unsigned int laddr, laddr_inc, errFlag, fwrev;
  unsigned int boardID;
  volatile struct sis3801_struct *sis;  
  int ii, res;
#ifdef VXWORKS
  res = sysBusToLocalAdrs(0x39, (char *)addr, (char **) &laddr);
#else
  res = vmeBusToLocalAdrs(0x39, (char *)addr, (char **) &laddr);
#endif
  if (res != 0) 
  {
#ifdef VXWORKS
    printf("%s: ERROR in sysBusToLocalAdrs(0x%x,0x%x,&laddr) \n",
	     __FUNCTION__,0x39,addr);
#else
    printf("%s: ERROR in vmeBusToLocalAdrs(0x%x,0x%x,&laddr) \n",
	     __FUNCTION__,0x39,addr);
#endif
    return(ERROR);
  }
  sisA24Offset = laddr - addr;



  Nsis = 0;
  for(ii=0; ii<SIS_MAX_SLOTS; ii++) 
  {
    /*printf("ii = %d\n",ii);*/
	laddr_inc = laddr + ii*addr_inc;

    sis = (struct dsc_struct *)laddr_inc;


    /* Check if Board exists at that address */
#ifdef VXWORKS
    res = vxMemProbe((char *) &(sis->irq),VX_READ,4,(char *)&boardID);
#else
    res = vmeMemProbe((char *) &(sis->irq),4,(char *)&boardID);
#endif
    if(res < 0) 
	{
#ifdef DEBUG
	  printf("%s: ERROR: No addressable board at A24 Address 0x%x\n",
		 __FUNCTION__,(UINT32) sis - dscA24Offset);
#endif
	  errFlag = 1;
	  continue;
	}
    else
	{
      boardID = (boardID>>16)&0xFFFF;
      printf("boardID=0x%04x\n",boardID);
	}

	/*
jlabgefMemProbe: Clearing VME BERR/2eST (0x800cf900) at VME address 0x100404
jlabgefMemProbe: Clearing VME BERR/2eST (0x800cf900) at VME address 0x180404
jlabgefMemProbe: Clearing VME BERR/2eST (0x800cf900) at VME address 0x200404
jlabgefMemProbe: Clearing VME BERR/2eST (0x800cf900) at VME address 0x280404
jlabgefMemProbe: Clearing VME BERR/2eST (0x800cf900) at VME address 0x300404
Initialized dsc2 ID 0 slot 7 at VME (USER) address 0x380000 (0xac278000).
Initialized dsc2 ID 1 slot 8 at VME (USER) address 0x400000 (0xac2f8000).
	*/

    /* Check if this is a SIS3801 */
    if(boardID != SIS3801_BOARD_ID) 
	{
#ifdef DEBUG
	  printf("%s: ERROR: Board ID at addr=0x%x does not match: 0x%08x \n",
		 __FUNCTION__,(UINT32) sis - dscA24Offset, boardID);
#endif
	  errFlag = 1;
	  continue;
	}






    sisp[Nsis] = (struct sis3801_struct*)laddr_inc;
    printf("Initialized sis3801 ID %d at VME (USER) address 0x%x (0x%x).\n",
		   Nsis, (UINT32) sisp[Nsis] - sisA24Offset, (UINT32) sisp[Nsis]);
    Nsis++;
    if(Nsis>=nsis) break;
  }




  return(Nsis);
}





int
l2_readscaler(unsigned int counts[NL2CH])
{
  int j;

  sis3801reset(addr);
  sis3801clear(addr);
  sis3801enablenextlogic(addr);
  sis3801nextclock(addr);

  /*sis3801control(addr, ENABLE_TEST); */
  /*sis3801control(addr, DISABLE_TEST); */
  sis3801Uledon(addr);

  while (sis3801status(addr) & FIFO_EMPTY)
    {
#ifdef VXWORKS
      taskDelay(sysClkRateGet() * 1);	/* 1 sec delay */
#else
      sleep(1);
#endif
      sis3801nextclock(addr);
    }

  for (j = 0; j < NL2CH; j++)
    counts[j] = sis3801readfifo(addr);
  sis3801clear(addr);

  return (0);
}

/*************************************************************************/

int
l2_status()
{
  unsigned int counts[NL2CH];
  int j;

  l2_readscaler(counts);

  for (j = 0; j < NL2CH; j++)
    {
      printf("[%2d] %9.9s %8d Hz\n", j + 1, lookup[j], counts[j]);
    }

  return (0);
}

int
l2_status_dead()
{
  unsigned int counts[NL2CH];
  int j, mean;

  /* DISABLE THIS FUNCTION AS SOON AS NOBODY WANT TO FIX HARDWARE PROBLEM */
  /*return(0);*/

  l2_readscaler(counts);

  mean = 0;
  for (j = 0; j < NL2CH; j++)
    {
      mean += counts[j];
    }
  mean = mean / NL2CH;
  /*printf("mean=%d\n",mean); */

  /* report dead channels only if count high enough */
  if (mean > 10000)
    {
      for (j = 0; j < NL2CH; j++)
	{
	  if (counts[j] == 0)
	    printf("%9.9s\n", lookup[j]);
	}
    }

  return (0);
}

int
l2_status_hot()
{
  unsigned int counts[NL2CH];
  int j, mean, nalive;

  l2_readscaler(counts);

  nalive = mean = 0;
  for (j = 0; j < NL2CH; j++)
    {
      if (counts[j] != 0)
	{
	  nalive++;
	  mean += counts[j];
	}
    }

  if (nalive > 0)
    {
      mean = mean / nalive;
      /*printf("nalive=%d mean=%d\n",nalive,mean); */
      for (j = 0; j < NL2CH; j++)
	{
	  if (counts[j] != 0)
	    {
	      if (counts[j] > mean * 2)	/* how much higher then 'mean' value channel */
		{		/* must count to be considered as 'hot'      */
		  printf("%9.9s\n", lookup[j]);
		}
	    }
	}
    }

  return (0);
}


/**************************************************************************/
/***  SP: additional functions to make remote reset for ADB crates  *******/
/**************************************************************************/

#define FLEX_ADDR      0x0EE0

struct flex_struct *flex_io;

int
l2_flex_init()
{
  unsigned long laddr;
  int res;

#ifdef VXWORKS
  res = sysBusToLocalAdrs(0x29, (char *) FLEX_ADDR, (char **) &laddr);
#else
  res = vmeBusToLocalAdrs(0x29, (char *) FLEX_ADDR, (char **) &laddr);
#endif

  if(res != 0)
    {
      printf("%s: ERROR: cannot obtain local address from 0x%08x\n",
	     __func__, FLEX_ADDR);
      return ERROR;
    }

  flex_io = (struct flex_struct *) laddr;


  vmeWrite16(&flex_io->csr1, 0x8000);	/* reset FLEX_IO */
  printf("l2_flex_init:  FLEX_IO address = 0x%08x (0x%lx)\n",
	 FLEX_ADDR, laddr);

  return OK;
}

int
l2_ADB_reset(char *ADB_name)
{
  int j;

  if (strcmp(ADB_name, NULL) == 0)
    {
      /* reserved to reset all bad ADB crates in ons shot */
      printf("l2_ADB_reset:  ADB_name = %9.9s \n", ADB_name);
      return (200);
    }

  j = l2_ADB_name2id(ADB_name, 0);	/* get correspondence ADB<->id */
  if (j == -1)
    {
      return (-1);
    }
  else if (j < 16)
    {
      SLOCK;
      vmeWrite16(&flex_io->port1_data, 1 << j);	/* enable  Direct VME output */
#ifdef VXWORKS
      taskDelay((sysClkRateGet() / 1000) * 15);	/* delay = 1msec * 15        */
#else
      usleep(15000);
#endif
      vmeWrite16(&flex_io->port1_data, 0);	/* disable Direct VME output */
      SUNLOCK;
    }
  else
    {
      SLOCK;
      vmeWrite16(&flex_io->port2_data, 1 << (j - 16));	/* enable  Direct VME output */
#ifdef VXWORKS
      taskDelay((sysClkRateGet() / 1000) * 15);	/* delay = 1msec * 15        */
#else
      usleep(15000);
#endif
      vmeWrite16(&flex_io->port2_data, 0);	/* disable Direct VME output */
      SUNLOCK;
    }

  printf("l2_ADB_reset:  ADB crate \"%s\" was reseted \n", ADB_name);
  return (j);
}

int
l2_ADB_reset_VMEpulse(char *ADB_name)
{
  int j;

  if (strcmp(ADB_name, NULL) == 0)
    {
      /* reserved to reset all bad ADB crates in ons shot */
      printf("l2_ADB_reset:  ADB_name = %9.9s \n", ADB_name);
      return (200);
    }

  j = l2_ADB_name2id(ADB_name, 0);	/* get correspondence ADB<->id */
  if (j == -1)
    {
      return (-1);
    }
  else if (j < 16)
    {
      SLOCK;
      vmeWrite16(&flex_io->port1_data, 1 << j);	/* enable one reset channel */
      vmeWrite16(&flex_io->csr1, 0x101);	/* send reset by one VME Pulse */
      SUNLOCK;
    }
  else
    {
      SLOCK;
      vmeWrite16(&flex_io->port2_data, 1 << (j - 16));	/* enable one reset channel */
      vmeWrite16(&flex_io->csr2, 0x101);	/* send reset by one VME Pulse */
      SUNLOCK;
    }

  printf("l2_ADB_reset:  ADB crate \"%s\" was reseted \n", ADB_name);
  return (j);
}

int
l2_ADB_name2id(char *ADB_name, int prn)
{
  char tmp[80];
  int j, res;

  sprintf(tmp, "[%s]", ADB_name);

  for (j = 0; j < NL2CH; j++)
    {
      if (strcmp(lookup[j], tmp) == 0)
	{
	  res = j;
	  j += NL2CH;
	  if (prn == 1)
	    printf("l2_ADB_name2id:  ADB_name = %9.9s -> ADB_id = %d \n",
		   lookup[res], res);
	}
    }

  if (j == NL2CH)
    {
      printf("l2_ADB_name2id:  Wrong ADB_name \"%s\" \n", ADB_name);
      return (-1);
    }

  return (res);
}

/* This variable will probably need a mutex */
static int dead_reset_STOP;

void
l2_status_dead_reset_STOP()
{
  dead_reset_STOP = 1;
}

int
l2_trigger_ERROR_set()
{
  char dddd[80];
  /*
     getFromHost("date",0,0,dddd);
   */
  getFromHost
    ("error_msg l2_check clonalarm l2_check trigger 2 ERROR 1 \"Level 2 problem\"",
     0, 0, dddd);
  printf(" dddd=>%s<\n", dddd);

  return OK;
}

int
l2_trigger_ERROR_clear()
{
  char dddd[80];
  getFromHost
    ("error_msg l2_check clonalarm l2_check trigger 0 INFO 1 \"Level 2 ok\"",
     0, 0, dddd);
  printf(" dddd=>%s<\n", dddd);

  return OK;
}


static void
ACTUAL_RESET(int j)
{
  if (j < 16)
    {
      SLOCK;
      vmeWrite16(&flex_io->port1_data, 1<<j);         /* enable  Direct VME output */
#ifdef VXWORKS
      taskDelay((sysClkRateGet()/1000)*15); /* delay = 1msec * 15        */
#else
      usleep(15000);
#endif
      vmeWrite16(&flex_io->port1_data, 0);              /* disable Direct VME output */
      SUNLOCK;
    }
  else
    {
      SLOCK;
      vmeWrite16(&flex_io->port2_data, 1<<(j-16));    /* enable  Direct VME output */
#ifdef VXWORKS
      taskDelay((sysClkRateGet()/1000)*15); /* delay = 1msec * 15        */
#else
      usleep(15000);
#endif
      vmeWrite16(&flex_io->port2_data, 0);              /* disable Direct VME output */
      SUNLOCK;
    }
}

int
l2_reset()
{
  int j;

  for (j = 0; j < NL2CH; j++)
    {
      ACTUAL_RESET(j);
    }
  return (0);
}





int
l2_status_dead_reset()
{
  FILE *l2log;
  char date[80], tmp[80];
  unsigned int counts[NL2CH];
  int N_reset[NL2CH];
  int j, mean, err_flag, D_prn = 0, N_dead = 0;

  dead_reset_STOP = 0;
  for (j = 0; j < NL2CH; j++)
    N_reset[j] = 0;

  while (dead_reset_STOP == 0)
    {
      l2log =
	fopen("/nfs/usr/clas/logs/run_log/l2_status_dead_reset.log", "a+");

      getFromHost("date", 0, 0, date);
      l2_readscaler(counts);

      mean = 0;
      for (j = 0; j < NL2CH; j++)
	{
	  mean += counts[j];
	}
      mean = mean / NL2CH;

      if (mean > 30000)		/* report dead channels only if count high enough */
	{
	  for (j = 0; j < NL2CH; j++)
	    {
	      if ((counts[j] == 0) && (N_reset[j] < 10))
		{
		  N_reset[j]++;
		  N_dead++;

		  ACTUAL_RESET(j);

		  if (D_prn == 0)
		    {
		      fprintf(l2log,
			      "------------------------------------------------ \n\n");
		      fprintf(l2log, "%s \n", date);
		      D_prn = 1;
		    }
		  fprintf(l2log,
			  " reset signal was sent to ADB crate %9.9s \n",
			  lookup[j]);
		}
	      else if ((counts[j] != 0) && (N_reset[j] != 0))
		{
		  N_reset[j] = 0;
		  if (D_prn == 0)
		    {
		      fprintf(l2log, "%s \n", date);
		      D_prn = 1;
		    }
		  fprintf(l2log,
			  " l2_status is OK now for ADB crate %9.9s \n",
			  lookup[j]);
		}
	    }

	  err_flag = 0;
	  for (j = 0; j < NL2CH; j++)
	    {
	      if (N_reset[j] >= 10)
		err_flag = 2 + j;
	      if (N_dead > 3)
		fprintf(l2log, "  mean=%d [%2d] %9.9s %8d Hz\n", mean, j + 1,
			lookup[j], counts[j]);
	    }

	  if (err_flag >= 2)
	    {
	      getFromHost
		("error_msg l2_check clonalarm l2_check trigger 2 ERROR 1 \"Level 2 problem\"",
		 0, 0, tmp);
	      fprintf(l2log, " alarm set UP due to N_reset=%d for %9.9s \n",
		      N_reset[err_flag - 2], lookup[err_flag - 2]);
	    }
	  else
	    {
	      getFromHost
		("error_msg l2_check clonalarm l2_check trigger 0 INFO 1 \"Level 2 OK\"",
		 0, 0, tmp);
	    }
	}

      D_prn = 0;
      N_dead = 0;
      fclose(l2log);
#ifdef VXWORKS
      taskDelay(sysClkRateGet() * 20);	/* 20 sec delay */
#else
      sleep(20);
#endif
    }

  return (0);
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/



int
readout1()
{
  unsigned int len, buffer[100];
  int i, j;
  char qui = 'z';

  sis3801reset(addr);
  sis3801clear(addr);
  sis3801enablenextlogic(addr);
  sis3801nextclock(addr);

  while (qui != 'q')
    {
      sis3801nextclock(addr);
      sis3801Uledon(addr);
      while (sis3801status(addr) & FIFO_EMPTY)
	{
	  taskDelay(1);
	}
      printf("Status: %08x\n", sis3801status(addr));
      for (j = 0; j < 32; j++)
	printf("%d FIFO: 0x%08x\n", j, sis3801readfifo(addr));
      len = sis3801read(addr, buffer);
      for (i = 0; i < len; i++)
	printf("value=%08x\n", buffer[i]);
      printf("... done.\n");
      /*sis3801clear(addr); */
      printf("Enter 'q' to quit or anyother to continue\n");
      scanf("%s", &qui);
    }

  return (0);
}

/***********************************************************************/

void
scalIntUser(int arg)
{
  int ii;
  unsigned int fifo_read;

  SLOCK;
  fifo_read = vmeRead32(&pf->csr);

  if (fifo_read & FIFO_EMPTY)
    {
      logMsg("scalIntUser: FIFO EMPTY\n", 0, 0, 0, 0, 0, 0);
      printf("data");
    }
  else
    {
      for (ii = 0; ii << arg; ii++)
	{
	  /*printf("data"); */
	  scalData[ii] = scalData[ii] + vmeRead32(&pf->fifo[0]);

	}
    }

  SUNLOCK;

}

/************************************************************************/
void
scalPrint()
{
  int ii;
  printf(" scaler data (Sums):\n");
  for (ii = 1; ii < 32; ii += 4)
    {
      printf("SCAL%02d: %10d SCAL%02d: %10d SCAL%02d: %10d SCAL%02d: %10d\n",
	     ii, scalData[ii - 1], ii + 1, scalData[ii], ii + 2,
	     scalData[ii + 1], ii + 3, scalData[ii + 2]);
    }
}

/*************************************************************************/
void
scalPulse(int count)
{
  int ii;

  SLOCK;
  if (pf)
    {
      if (vmeRead32(&pf->csr) & ENABLE_TEST)
	{
	  for (ii = 0; ii < count; ii++)
	    vmeWrite32(&pf->test, 1);
	}
      else
	{
	  printf("Mode_disable call test(mode)\n");
	}
    }
  else
    {
      printf("call init\n");
    }
  SUNLOCK;
}

/**************************************************************************/

int
l2_input(unsigned int counts[NL2CH])
{
  int j;
  int time;
  sis3801reset(addr);
  sis3801clear(addr);
  sis3801enablenextlogic(addr);
  sis3801nextclock(addr);
  sis3801Uledon(addr);
  printf("Enter time delay --> ");
  scanf("%d", &time);
  while (sis3801status(addr) & FIFO_EMPTY)
    {
#ifdef VXWORKS
      taskDelay(sysClkRateGet() * time);	/* 1 sec delay */
#else
      sleep(time);
#endif
      sis3801nextclock(addr);
    }

  for (j = 0; j < NL2CH; j++)
    counts[j] = sis3801readfifo(addr);
  sis3801clear(addr);

  return (0);
}

int
pp()
{
  printf("pepepe");
  return (0);
}

int
l2_delay()
{
  unsigned int counts[NL2CH];
  int j;
  printf("test123");
  l2_input(counts);
  for (j = 0; j < NL2CH; j++)
    {
      printf("[%2d] %9.9s %8d Hz\n", j, lookup[j], counts[j]);
    }

  return (0);
}

/**************************************************************************/

int
junk(unsigned int counts[NL2CH])
{
  int j;
  int time;
  sis3801reset(addr);
  sis3801clear(addr);
  sis3801enablenextlogic(addr);
  sis3801nextclock(addr);
  sis3801Uledon(addr);
  printf("Enter time delay --> ");
  scanf("%d", &time);
  while (sis3801status(addr) & FIFO_EMPTY)
    {
#ifdef VXWORKS
      taskDelay(sysClkRateGet() * time);	/* 1 sec delay */
#else
      sleep(time);
#endif
      sis3801nextclock(addr);
    }

  for (j = 0; j < NL2CH; j++)
    counts[j] = sis3801readfifo(addr);
  sis3801clear(addr);

  return (0);
}

int
hist()
{
  FILE *stream;
  FILE *log;
  unsigned int counts[NL2CH];
  int j;
  junk(counts);
  stream =
    fopen("/usr/local/clas/devel/source/vxworks/code/temp/snapshot.txt", "w");
  log = fopen("/usr/local/clas/devel/source/vxworks/code/temp/log.txt", "a+");
  for (j = 0; j < NL2CH; j++)
    {
      if (j == 0)
	{
	  fclose(stream);
	  stream =
	    fopen
	    ("/usr/local/clas/devel/source/vxworks/code/temp/snapshot.txt",
	     "a+");
	}

      printf("[%2d] %9.9s %8d Hz\n", j, lookup[j], counts[j]);
      if (stream == NULL)
	{
	  printf("Error creating file!\n");
	}
      else
	{
	  fprintf(stream, "[%2d] %9.9s %8d Hz\n", j, lookup[j], counts[j]);
	}
      /*fclose(stream); */
      if (log == NULL)
	{
	  printf("Error appending to hist.txt\n");
	}
      else
	{
	  fprintf(log, "[%2d] %9.9s %8d Hz\n", j, lookup[j], counts[j]);
	}
    }
  fclose(stream);
  fclose(log);
  return (0);
}

#else /* dummy version*/

void
sis3801_dummy()
{
  return;
}

#endif
