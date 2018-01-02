#ifdef Linux_vme

/*----------------------------------------------------------------------------*/
/**
 * @mainpage
 * <pre>
 *  Copyright (c) 2012        Southeastern Universities Research Association, *
 *                            Thomas Jefferson National Accelerator Facility  *
 *                                                                            *
 *    This software was developed under a United States Government license    *
 *    described in the NOTICE file included as part of this distribution.     *
 *                                                                            *
 *    Authors: Bryan Moffit                                                   *
 *             moffit@jlab.org                   Jefferson Lab, MS-12B3       *
 *             Phone: (757) 269-5660             12000 Jefferson Ave.         *
 *             Fax:   (757) 269-5800             Newport News, VA 23606       *
 *                                                                            *
 *----------------------------------------------------------------------------*
 *
 * Description:
 *     Primitive trigger control for VME CPUs using the TJNAF Trigger
 *     Supervisor (TI) card
 *
 * </pre>
 *----------------------------------------------------------------------------*/

#define _GNU_SOURCE

#define DEVEL

#include <sys/prctl.h>
#include <unistd.h>
#include "jvme.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "tiLib.h"

/* Mutex to guard TI read/writes */
extern pthread_mutex_t   tiMutex;
#define TILOCK     if(pthread_mutex_lock(&tiMutex)<0) perror("pthread_mutex_lock");
#define TIUNLOCK   if(pthread_mutex_unlock(&tiMutex)<0) perror("pthread_mutex_unlock");

/* Global Variables */
extern volatile struct TI_A24RegStruct  *TIp;    /* pointer to TI memory map */
extern volatile        unsigned int     *TIpd;  /* pointer to TI data FIFO */
extern unsigned int        tiIntCount;
extern unsigned int        tiAckCount;
static int          tiReadoutEnabled = 1;    /* Readout enabled, by default */


void my_tiIntAck()
{
	int resetbits=0;
	unsigned int request = 0;

	if(TIp == NULL)
	{
		logMsg("tiIntAck: ERROR: TI not initialized\n",0,0,0,0,0,0);
		return;
	}

	TILOCK;
		tiAckCount++;
		resetbits = TI_RESET_BUSYACK;
		if(!tiReadoutEnabled)
		{
			// Readout Acknowledge and decrease the number of available blocks by 1
			resetbits |= TI_RESET_BLOCK_READOUT;
		}

		request = (vmeRead32(&TIp->blockBuffer) & TI_BLOCKBUFFER_SYNCRESET_REQUESTED)>>30;
	  	if( request )  
		{
			printf("%s: ERROR : TI_BLOCKBUFFER %x \n",__FUNCTION__, request);
		}

		vmeWrite32(&TIp->reset, resetbits);
	TIUNLOCK;

	if( request )
		tiSyncResetResync();
}


void my_tiSyncReset()
{
	int resetbits=0;
	unsigned int request = 0;

	if(TIp == NULL)
	{
		logMsg("tiIntAck: ERROR: TI not initialized\n",0,0,0,0,0,0);
		return;
	}

	TILOCK;
		request = (vmeRead32(&TIp->blockBuffer) & TI_BLOCKBUFFER_SYNCRESET_REQUESTED)>>30;
		if( request )  
		{
			printf("%s: ERROR : TI_BLOCKBUFFER  %x \n",__FUNCTION__, request);
			// resetbits |= TI_RESET_BUSYACK;
		}
		vmeWrite32(&TIp->reset, resetbits);
	TIUNLOCK;

	if( request )
		tiSyncResetResync();
}




int my_tiIntEnable(int iflag)
{
	if(TIp == NULL)
	{
		logMsg("tiIntAck: ERROR: TI not initialized\n",0,0,0,0,0,0);
		return ERROR;
	}

	if(iflag == 1)
	{
		tiIntCount = 0;
		tiAckCount = 0;
	}
	return(0);
}


void my_tiDeadtime()
{
	unsigned int livetime, busytime;

	if(TIp == NULL)
	{
		logMsg("tiIntAck: ERROR: TI not initialized\n",0,0,0,0,0,0);
		return;
	}

	/* latch live and busytime scalers */
	tiLatchTimers();

	TILOCK;
		/* Latch scalers first */
		vmeWrite32(&TIp->reset,TI_RESET_SCALERS_LATCH);
		livetime     = vmeRead32(&TIp->livetime);
		busytime     = vmeRead32(&TIp->busytime);
	TIUNLOCK;

	if ( busytime || livetime)
	{ 
		fprintf( stdout, "  deadtime %7.3f %% ",  (double) busytime  / ( (double)busytime + (double) livetime ) *100.); 
	}
}

int my_tiLive()
{
	int reg_bl = 0, bl=0, rval=0;
	float fval=0;
	unsigned int newBusy=0, newLive=0, newTotal=0;
	unsigned int live=0, total=0;
	static unsigned int oldLive=0, oldTotal=0;

	if(TIp == NULL)
	{
		logMsg("tiIntAck: ERROR: TI not initialized\n",0,0,0,0,0,0);
		return ERROR;
	}

	TILOCK;
		vmeWrite32(&TIp->reset,TI_RESET_SCALERS_LATCH);
		newLive = vmeRead32(&TIp->livetime);
		newBusy = vmeRead32(&TIp->busytime);
		//reg_bl = vmeRead32(&TIp->blocklevel);
	TIUNLOCK;
  
	bl = (reg_bl & TI_BLOCKLEVEL_CURRENT_MASK)>>16;
  
	newTotal = newLive+newBusy;
	if((oldTotal<newTotal))
	{
		/* Differential */
		live  = newLive - oldLive;
		total = newTotal - oldTotal;
    	}
	else
	{
		/* Integrated */
		live = newLive;
		total = newTotal;
	}
	oldLive = newLive;
	oldTotal = newTotal;

	if(total>0)
		fval =  (float) total;
	rval = total;

	return rval;
}


#else

void tiUtils_dummy() {}

#endif
