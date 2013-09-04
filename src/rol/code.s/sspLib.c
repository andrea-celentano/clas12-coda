#ifdef Linux_vme

/*sspLib.c */

#ifdef VXWORKS
#include <vxWorks.h>
#include <taskLib.h>
#else
#include <jvme.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "sspLib.h"

#define DEVEL

#ifdef VXWORKS
#define SYNC()		{ __asm__ volatile("eieio"); __asm__ volatile("sync"); }
#endif

/* Mutex to guard SSP read/writes */
pthread_mutex_t   sspMutex = PTHREAD_MUTEX_INITIALIZER;
#define SSPLOCK     if(pthread_mutex_lock(&sspMutex)<0) perror("pthread_mutex_lock");
#define SSPUNLOCK   if(pthread_mutex_unlock(&sspMutex)<0) perror("pthread_mutex_unlock");

#define SSP_BASE_ADDR		0xBE0000

SSP_regs *pSSP;


void 
sspInit(unsigned int addr_a24)
{
#ifdef VXWORKS
  sysBusToLocalAdrs(0x39, addr_a24, &pSSP);
#else
  vmeBusToLocalAdrs(0x39, (char *)addr_a24, (char **)&pSSP);
#endif
  printf("ssp address = 0x%08x -> 0x%08x\n",addr_a24,pSSP);
}

void 
sspSetupPulser(Pulser *pPulser, unsigned int period, unsigned int hightime)
{

  SSPLOCK;
  vmeWrite32(&pPulser->PeriodCycles,period);
  vmeWrite32(&pPulser->HighCycles,hightime);
  SSPUNLOCK;
}

void 
sspSetupOutput(unsigned int *pIOReg, int mux_sel, int val, 
	       int disable_output, int delay)
{
  unsigned int reg = 0;

  if(val)
    reg |= GPIO_VMEREG_1;

  reg = (mux_sel);
  
  SSPLOCK;
  vmeWrite32(pIOReg, reg | (1<<31) );
  while(delay--)
    vmeWrite32(pIOReg, reg | (1<<29) );

  SSPUNLOCK;
}

int 
sspGetInput(unsigned int *pIOReg)
{
  unsigned int rval;

  SSPLOCK;
  rval = vmeRead32(pIOReg) & 0x1;
  SSPUNLOCK;

  if(rval)
    return 1;

  return 0;
}

void 
sspSetupNIM(int port, int mux_sel, int val, int disable_output, int delay)
{
  sspSetupOutput((unsigned int *)&pSSP->NIM_IO[port], 
		 mux_sel, val, disable_output, delay);
}

void 
sspSetupLVDS(int port, int mux_sel, int val, int disable_output, int delay)
{
  sspSetupOutput((unsigned int *)&pSSP->LVDS_IO[port], 
		 mux_sel, val, disable_output, delay);
}

void 
sspSetupDiscriminator(int index, unsigned int threshold, unsigned width, int mode)
{
  SSPLOCK;
  vmeWrite32(&pSSP->Discriminators[index].Mode, mode);
  vmeWrite32(&pSSP->Discriminators[index].Threshold, threshold);
  vmeWrite32(&pSSP->Discriminators[index].PulseWidth, width);
  SSPUNLOCK;
}

void 
sspSetupPulserIndex(int index, float period, float duty)
{
  sspSetupPulser((Pulser *)&pSSP->Pulse_IO[index], period, duty);
}

void 
sspFiberDisableAll()
{
  int i;
  SSPLOCK;
  for(i = 0; i < 4; i++)
    {
      vmeWrite32(&pSSP->LinkA[i].Control, 0x1);
      vmeWrite32(&pSSP->LinkB[i].Control, 0x1);
    }
  SSPUNLOCK;

}

void 
sspFiberEnableAll()
{
  int i;
  SSPLOCK;
  for(i = 0; i < 4; i++)
    {
      vmeWrite32(&pSSP->LinkA[i].Control, 0);
      vmeWrite32(&pSSP->LinkB[i].Control, 0);
    }
  SSPUNLOCK;
}

void 
sspFiberEnable(int port)
{
  SSPLOCK;
  if(port < 4)
    {
      vmeWrite32(&pSSP->LinkA[port].Control, 0x1);
      vmeWrite32(&pSSP->LinkA[port].Control, 0x0);
    }
  else
    {
      vmeWrite32(&pSSP->LinkB[port-4].Control, 0x1);
      vmeWrite32(&pSSP->LinkB[port-4].Control, 0x0);
    }
  SSPUNLOCK;
}

void
sspFiberDisable(int port)
{
  SSPLOCK;
  if(port < 4)
    {
      vmeWrite32(&pSSP->LinkA[port].Control, 0x1);
    }
  else
    {
      vmeWrite32(&pSSP->LinkB[port-4].Control, 0x1);
    }
  SSPUNLOCK;
}

void 
sspFiberStatus(int port)
{

  SSPLOCK;
  if(port < 4)
    {
      vmeWrite32(&pSSP->LinkA[port].Control,
		 0x80000000 | vmeRead32(&pSSP->LinkA[port].Control));

      printf("Link%c.Control      = 0x%08X\n", 'A'+port, 
	     vmeRead32(&pSSP->LinkA[port].Control));
      printf("Link%c.Status       = 0x%08X\n", 'A'+port, 
	     vmeRead32(&pSSP->LinkA[port].Status));
      printf("Link%c.FECStatus    = 0x%08X\n", 'A'+port, 
	     vmeRead32(&pSSP->LinkA[port].FECStatus));
      printf("Link%c.LaneStatus01 = 0x%08X\n", 'A'+port, 
	     vmeRead32(&pSSP->LinkA[port].LaneStatus01));
      printf("Link%c.LaneStatus23 = 0x%08X\n", 'A'+port, 
	     vmeRead32(&pSSP->LinkA[port].LaneStatus23));
      printf("Link%c.GTXDRP       = 0x%08X\n", 'A'+port, 
	     vmeRead32(&pSSP->LinkA[port].GTXDRP));
      printf("Link%c.PRBSStatus   = 0x%08X\n", 'A'+port, 
	     vmeRead32(&pSSP->LinkA[port].PRBSStatus));
    }
  else
    {
      vmeWrite32(&pSSP->LinkB[port-4].Control,
		 0x80000000 | vmeRead32(&pSSP->LinkB[port-4].Control));

      printf("Link%c.Control      = 0x%08X\n", 'A'+port, 
	     vmeRead32(&pSSP->LinkB[port-4].Control));
      printf("Link%c.Status       = 0x%08X\n", 'A'+port, 
	     vmeRead32(&pSSP->LinkB[port-4].Status));
      printf("Link%c.FECStatus    = 0x%08X\n", 'A'+port, 
	     vmeRead32(&pSSP->LinkB[port-4].FECStatus));
      printf("Link%c.LaneStatus01 = 0x%08X\n", 'A'+port, 
	     vmeRead32(&pSSP->LinkB[port-4].LaneStatus01));
      printf("Link%c.LaneStatus23 = 0x%08X\n", 'A'+port, 
	     vmeRead32(&pSSP->LinkB[port-4].LaneStatus23));
      printf("Link%c.GTXDRP       = 0x%08X\n", 'A'+port, 
	     vmeRead32(&pSSP->LinkB[port-4].GTXDRP));
      printf("Link%c.PRBSStatus   = 0x%08X\n", 'A'+port, 
	     vmeRead32(&pSSP->LinkB[port-4].PRBSStatus));
    }

  SSPUNLOCK;
}

/* HPS dependent routines */
void
sspSetCoincidence(unsigned int window)
{

  if(window>0xf)
    {
      printf("%s: ERROR.  Invalid value for window (%d).  Must be less than 16.",
	     __FUNCTION__,window);
      return;
    }

  SSPLOCK;
  vmeWrite32(&pSSP->HPS.Coincidence,window);
  SSPUNLOCK;

}

void
sspSetEnergySumMaximum(unsigned int max)
{
  if(max>0xff)
    {
      printf("%s: ERROR.  Invalid value for max (%d).  Must be less than 256.",
	     __FUNCTION__,max);
      return;
    }

  SSPLOCK;
  vmeWrite32(&pSSP->HPS.PairSumMax,max);
  SSPUNLOCK;

}

void
sspSetPairEnergyDifferenceMaximum(unsigned int max)
{
  if(max>0xff)
    {
      printf("%s: ERROR.  Invalid value for max (%d).  Must be less than 256.",
	     __FUNCTION__,max);
      return;
    }

  SSPLOCK;
  vmeWrite32(&pSSP->HPS.PairDiffMax,max);
  SSPUNLOCK;

}

void
sspSetPairEnergyDistanceThreshold(unsigned int thres)
{
  if(thres>0xff)
    {
      printf("%s: ERROR.  Invalid value for thres (%d).  Must be less than 256.",
	     __FUNCTION__,thres);
      return;
    }

  SSPLOCK;
  vmeWrite32(&pSSP->HPS.EDSlope,thres);
  SSPUNLOCK;

}

void
sspSetEnergyMaximum(unsigned int max)
{
  if(max>0xff)
    {
      printf("%s: ERROR.  Invalid value for max (%d).  Must be less than 256.",
	     __FUNCTION__,max);
      return;
    }

  SSPLOCK;
  vmeWrite32(&pSSP->HPS.EMax,max);
  SSPUNLOCK;

}

void
sspSetEnergyMinimum(unsigned int min)
{
  if(min>0xff)
    {
      printf("%s: ERROR.  Invalid value for min (%d).  Must be less than 256.",
	     __FUNCTION__,min);
      return;
    }

  SSPLOCK;
  vmeWrite32(&pSSP->HPS.EMin,min);
  SSPUNLOCK;

}

void
sspSetClusterDelay(unsigned int delay)
{
  if(delay>0x1ff)
    {
      printf("%s: ERROR.  Invalid value for delay (%d).  Must be less than 512.",
	     __FUNCTION__,delay);
      return;
    }

  SSPLOCK;
  vmeWrite32(&pSSP->HPS.Delay,delay);
  SSPUNLOCK;

}

void
sspSetTriggerLatency(unsigned int late)
{
  if(late>0x3ff)
    {
      printf("%s: ERROR.  Invalid value for late (%d).  Must be less than 1024.",
	     __FUNCTION__,late);
      return;
    }

  SSPLOCK;
  vmeWrite32(&pSSP->HPS.TriggerLatency,late);
  SSPUNLOCK;

}

void
sspSetTriggerCutEnableMask(unsigned int mask)
{
/*   if(mask>0x3ff) */
/*     { */
/*       printf("%s: ERROR.  Invalid value for mask (%d).  Must be less than 1024.", */
/* 	     __FUNCTION__,mask); */
/*       return; */
/*     } */

  SSPLOCK;
  vmeWrite32(&pSSP->HPS.TriggerCutEnable,mask);
  SSPUNLOCK;

}





void
sspInit_HPS(unsigned int addr_a24)
{
  sspInit(addr_a24);

  SSPLOCK;
  /* Setup SYNC and Clk Source */
  vmeWrite32(&pSSP->Config, 
	     CFG_SYNC_SRC_P0 | 
	     CFG_CLOCK_SRC_SWB);
  vmeWrite32(&pSSP->Config, 
	     CFG_SYNC_SRC_P0 | 
	     CFG_RESET  | 
	     CFG_CLOCK_SRC_SWB);
  vmeWrite32(&pSSP->Config, 
	     CFG_SYNC_SRC_P0 | 
	     CFG_CLOCK_SRC_SWB);
  taskDelay(10);    /* ~100ms */
 
  /* Enable Relevant Fiber Ports */
  vmeWrite32(&pSSP->LinkA[1].Control, 0x1);
  vmeWrite32(&pSSP->LinkA[1].Control, 0x0);
  vmeWrite32(&pSSP->LinkB[1].Control, 0x1);
  vmeWrite32(&pSSP->LinkB[1].Control, 0x0);


  /* new registers may 15 14:30 */

  /* trigger pulse width in 4ns ticks, 16 bits (64K max) */
  sspSetTriggerWidth(250);

  /* in 4ns ticks, 14 bits max */
  sspSetEventPatternDelay(210);


  SSPUNLOCK;

  /* Setup outputs for HPS trigger on all GPIO outputs */
  sspSetupNIM(GPIO_NIM_NIMA, GPIO_VMEREG_0|GPIO_MUXSEL_VMEREG, 0, 0, 0);
  sspSetupNIM(GPIO_NIM_NIMB, GPIO_VMEREG_0|GPIO_MUXSEL_VMEREG, 0, 0, 0);
  /*
  sspSetupNIM(GPIO_NIM_NIMA, GPIO_MUXSEL_DISC0, 0, 0, 0);
  sspSetupNIM(GPIO_NIM_NIMB, GPIO_MUXSEL_DISC0, 0, 0, 0);
  */

  sspSetupLVDS(0, GPIO_MUXSEL_DISC0, 0, 0, 0);

  /*sspSetupLVDS(1, GPIO_MUXSEL_DISC0, 0, 0, 0);until may 17 10:50am - HPS trigger */
  sspSetupLVDS(1, GPIO_MUXSEL_PULSER4, 0, 0, 0);/* HPS accepted trigger*/

  sspSetupLVDS(2, GPIO_MUXSEL_DISC0, 0, 0, 0);
  sspSetupLVDS(3, GPIO_MUXSEL_DISC0, 0, 0, 0);

}


/* set trigger width */
void
sspSetTriggerWidth(int width)
{
  int value;
  value = width & 0xFFFF;
  vmeWrite32(&pSSP->HPS.TriggerWidth, value);
}


/* set event patetrn delay in ticks */
void
sspSetEventPatternDelay(int delay)
{
  int value;
  value = delay & 0x3FFF;
  vmeWrite32(&pSSP->HPS.EventPatternDelay, value);
}
void
sspPrintEventPatternDelay()
{
  int value;
  value = vmeRead32(&pSSP->HPS.EventPatternDelay);
  printf("sspPrintEventPatternDelay: address 0x%08x(0x%08x), value=0x%08x\n",
		 &pSSP->HPS.EventPatternDelay,pSSP,value);
}


/* should be called from prestart */
void
sspClearEventFifo()
{
  vmeWrite32(&pSSP->HPS.EventFifoCfg, 1);
}




int
sspReadFifo(unsigned int *buf)
{
  int i;
  unsigned int nw;

  nw = vmeRead32(&pSSP->HPS.EventFifoCfg);
  /*
  if(nw<7)
  {
    printf("ERROR in sspReadFifo: nw=%d\n",nw);
  }
  */
  for(i=0; i<7; i++) buf[i] = vmeRead32(&pSSP->HPS.EventFifoData);

  return(7);
}



#else /* dummy version*/

void
sspLib_dummy()
{
  return;
}

#endif
