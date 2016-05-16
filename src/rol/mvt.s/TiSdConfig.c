#ifdef Linux_vme

/*
--------------------------------------------------------------------------------
-- Company:        IRFU / CEA Saclay
-- Engineers:      Irakli.MANDJAVIDZE@cea.fr (IM)
-- 
-- Project Name:   Clas12 Micromegas Vertex Tracker
-- Design Name:    Clas12 Dream testbench software
--
-- Module Name:    TiSdConfig.c
-- Description:    Trigger Interface (TI) and Signal Distribution (SD) configuration
--
-- Target Devices: 
-- Tool versions:  
-- 
-- Create Date:    0.0 2014/10/13 IM
-- Revision:       1.0 2015/03/23 IM Extended Eid & Tstp capability
--                     2015/03/27 IM Thrigger inhibit threshold
--
-- Comments:
--
--------------------------------------------------------------------------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "ReturnCodes.h"
#include "TiConfigParams.h"
#include "SysConfig.h"
#include "jvme.h"
#include "tiLib.h"
#include "sdLib.h"

// external global variables to detect if VME intialisation has to be done
extern volatile struct TI_A24RegStruct  *TIp;
// standalone application flag:
// if set to 0 CODA handles some part of initialisation
// else the application is standalone
static int stand_alone_app = 0;

// Configure TI with parameters
int TiConfig( TiParams *params )
{
	int ret;
	int rule;
	int rul_cnt;
	int rule_unit;

	// Check for Null pointer
	if( params == (TiParams *)NULL )
	{
		fprintf( stderr, "%s: params=0\n", __FUNCTION__ );
		return D_RetCode_Err_Null_Pointer;
	}
	
	// Check if the TI had already been initialized
	if( TIp == NULL )
	{
		// No need to check if vmeOpenDefaultWindows was already called
		// The function tests internally its state
		if( (ret = vmeOpenDefaultWindows()) < 0 )
		{
			fprintf( stderr, "%s: vmeOpenDefaultWindows failed with %d\n", __FUNCTION__, ret );
			return ret;
		}
		// For the moment only polling mode is supported
		if( params->TrgSrc == TiTrgSrc_HFBR1 )
		{
			if( (ret = tiInit(0,TI_READOUT_TS_POLL,0)) != OK )
			{
				fprintf( stderr, "%s: tiInit failed for TI_READOUT_TS_POLL with %d\n", __FUNCTION__, ret );
				return ret;
			}
		}
		else
		{
			if( (ret = tiInit(0,TI_READOUT_EXT_POLL,0)) != OK )
			{
				fprintf( stderr, "%s: tiInit failed for TI_READOUT_EXT_POLL with %d\n", __FUNCTION__, ret );
				return ret;
			}
		}
		// Seems that the function has been called outside CODA
		// Assume the standalone application
		stand_alone_app = 1;
fprintf( stdout, "%s: **** Standalone APPLICATION ****\n", __FUNCTION__ );
	}
	
	/* Set crate ID */
	if( (ret=tiSetCrateID( params->Id )) != OK )
	{
		fprintf( stderr, "%s: tiSetCrateID failed for id=%d with %d\n", __FUNCTION__, params->Id, ret );
		return ret;
	}
	
	/* Set Trigger Source */
	if( params->TrgSrc == TiTrgSrc_HFBR1 )
	{
		if( (ret=tiSetTriggerSource(TI_TRIGGER_HFBR1)) != OK )
		{
			fprintf( stderr, "%s: tiSetTriggerSource(TI_TRIGGER_HFBR1) failed for TI %d with %d\n", __FUNCTION__, params->Id, ret );
			return ret;
		}
	}
	else if( (params->TrgSrc == TiTrgSrc_FpInp1) || (params->TrgSrc == TiTrgSrc_FpInp2) )
	{
		if( (ret=tiSetTriggerSource(TI_TRIGGER_TSINPUTS)) != OK )
		{
			fprintf( stderr, "%s: tiSetTriggerSource(TI_TRIGGER_TSINPUTS) failed for TI %d with %d\n", __FUNCTION__, params->Id, ret );
			return ret;
		}
		if( (ret=tiDisableTSInput(TI_TSINPUT_ALL) )!= OK )
		{
			fprintf( stderr, "%s: tiDisableTSInput(TI_TSINPUT_ALL) failed for TI %d with %d\n", __FUNCTION__, params->Id, ret );
			return ret;
		}
		if( params->TrgSrc == TiTrgSrc_FpInp1 )
		{
			if( (ret=tiEnableTSInput(TI_TSINPUT_1)) != OK )
			{
				fprintf( stderr, "%s: tiEnableTSInput(TI_TSINPUT_1) failed for TI %d with %d\n", __FUNCTION__, params->Id, ret );
				return ret;
			}
		}
		else
		{
			if( (ret=tiEnableTSInput(TI_TSINPUT_2)) != OK )
			{
				fprintf( stderr, "%s: tiEnableTSInput(TI_TSINPUT_2) failed for TI %d with %d\n", __FUNCTION__, params->Id, ret );
				return ret;
			}
		}
		/* Load the trigger table that associates 
     		pins 21/22 | 23/24 | 25/26 : trigger1
     		pins 29/30 | 31/32 | 33/34 : trigger2
  		*/
  		if( (ret=tiLoadTriggerTable(0)) != OK )
		{
			fprintf( stderr, "%s: tiLoadTriggerTable(0) failed for TI %d with %d\n", __FUNCTION__, params->Id, ret );
			return ret;
		}
	}
	else if (params->TrgSrc == TiTrgSrc_IntCst)
	{	int i_rate;	
		i_rate = 1000000 /( 30.72 *(params->TrgRate));  
		fprintf( stderr, "%s: PULSER RATE  %d, INCREMENT RATE = %d \n", __FUNCTION__, params->TrgRate, i_rate );

		
		if( (ret=tiSoftTrig(1, 0x7FFF, i_rate , 1)) != OK )
		{
			fprintf( stderr, "%s: tiSoftTrig failed for TI %d with %d\n", __FUNCTION__, params->Id, ret );
			return ret;
		}

		if( (ret=tiSoftTrig(2, 0x7FFF, i_rate , 1)) != OK )
		{
			fprintf( stderr, "%s: tiSoftTrig failed for TI %d with %d\n", __FUNCTION__, params->Id, ret );
			return ret;
		}

		if( (ret= tiSetRandomTrigger(1, 0x7)) != OK )
		{
			fprintf( stderr, "%s: tiSoftTrig failed for TI %d with %d\n", __FUNCTION__, params->Id, ret );
			return ret;
		}
		if( (ret= tiSetRandomTrigger(2, 0x7)) != OK )
		{
			fprintf( stderr, "%s: tiSoftTrig failed for TI %d with %d\n", __FUNCTION__, params->Id, ret );
			return ret;
		}


		if( (ret=tiSetTriggerPulse(1, 10, 10, 0)) != OK )
		{
			fprintf( stderr, "%s: tiSetTriggerPulse failed for TI %d with %d\n", __FUNCTION__, params->Id, ret );
			return ret;
		}
		if( (ret=tiSetTriggerPulse(2, 10, 10, 0)) != OK )
		{
			fprintf( stderr, "%s: tiSetTriggerPulse failed for TI %d with %d\n", __FUNCTION__, params->Id, ret );
			return ret;
		}		
		if( (ret=tiSetTriggerSource(TI_TRIGGER_PULSER)) != OK )
		{
			fprintf( stderr, "%s: tiSetTriggerSource(TI_TRIGGER_PULSER) failed for TI %d with %d\n", __FUNCTION__, params->Id, ret );
			return ret;
		}
		if( (ret=tiDisableTSInput(TI_TSINPUT_ALL) )!= OK )
		{
			fprintf( stderr, "%s: tiDisableTSInput(TI_TSINPUT_ALL) failed for TI %d with %d\n", __FUNCTION__, params->Id, ret );
			return ret;
		}

	}
	else if(  (params->TrgSrc == TiTrgSrc_IntRnd) || (params->TrgSrc == TiTrgSrc_Soft) || (params->TrgSrc == TiTrgSrc_None))
	{
		fprintf( stderr, "%s: TI %d trigger source %s not yet implemented\n", __FUNCTION__, params->Id, TiTrgSrc2Str(params->TrgSrc) );
		return D_RetCode_Err_Wrong_Param;
	}
	else
	{
		fprintf( stderr, "%s: TI %d usupported trigger source %s\n", __FUNCTION__, params->Id, TiTrgSrc2Str(params->TrgSrc) );
		return D_RetCode_Err_Wrong_Param;
	}

	/* Set Trigger Rules */
	for( rule=0; rule<Def_Ti_TrgRules_MaxNum; rule++ )
	{
		if( (params->TrgRules_TimeUnit[rule] < 0) || (params->TrgRules_UnitCount[rule]) < 0 )
		{
			rul_cnt   = 0;
			rule_unit = 0;
		}
		else
		{
			rul_cnt   = params->TrgRules_UnitCount[rule];
			rule_unit = params->TrgRules_TimeUnit[rule];
		}
		if( (ret=tiSetTriggerHoldoff(rule+1,rul_cnt,rule_unit) ) != OK )
		{
			fprintf( stderr, "%s: tiSetTriggerHoldoff failed for TI %d rule=%d cnt=%d unit=%d with %d\n",
				__FUNCTION__, params->Id, rule+1, rul_cnt, rule_unit, ret );
			return D_RetCode_Err_Wrong_Param;
		}
	}

	/* Set Prescale */
	if( (ret=tiSetPrescale( params->TrgPrescale )) != OK )
	{
		fprintf( stderr, "%s: tiSetPrescale failed for TI %d prescale=%d with %d\n", __FUNCTION__, params->Id, params->TrgPrescale, ret );
		return ret;
	}

	/* Set Sync Delay */
	// This is something to be understood; for the moment copy-paste of existing line
	// Set the sync delay width to 0x40*32 = 2.048us
	// For some reason this function is void.
	tiSetSyncDelayWidth(0x54, 0x40, 1);

	/* Set Busy source */
//fprintf( stdout, "%s: BsySrc=%d OK for TI %d\n", __FUNCTION__, params->BsySrc, params->Id );
	if( params->BsySrc == TiBsySrc_None )
{
		if( (ret=tiSetBusySource(TI_BUSY_LOOPBACK, 1)) != OK )
		{
			fprintf( stderr, "%s: tiSetBusySource(0,1) failed for TI %d with %d\n", __FUNCTION__, params->Id, ret );
			return ret;
		}
//fprintf( stdout, "%s: tiSetBusySource(0,1) OK for TI %d with %d\n", __FUNCTION__, params->Id, ret );
}
	else if( params->BsySrc == TiBsySrc_SwB )
{
		if( (ret=tiSetBusySource(TI_BUSY_SWB, 0)) != OK )
		{
			fprintf( stderr, "%s: tiSetBusySource(TI_BUSY_SWB,0) failed for TI %d with %d\n", __FUNCTION__, params->Id, ret );
			return ret;
		}
//fprintf( stdout, "%s: tiSetBusySource(TI_BUSY_SWB,0) OK for TI %d with %d\n", __FUNCTION__, params->Id, ret );
}
	else
	{
		fprintf( stderr, "%s: unsupporetd TI Busy Source %d for TI %d\n", __FUNCTION__, params->BsySrc, params->Id );
		return D_RetCode_Err_Wrong_Param;
	}

	/* Set number of events per block */
	if( (ret=tiSetBlockLevel(params->NbOfEvtPerBlk)) != OK )
	{
		fprintf( stderr, "%s: tiSetBlockLevel(%d) failed for TI %d with %d\n", __FUNCTION__, params->NbOfEvtPerBlk, params->Id, ret );
		return ret;
	}

	/* Set Number of block threshold */
	if( (ret=tiSetBlockBufferLevel(params->TrgInhThr)) != OK )
	{
		fprintf( stderr, "%s: tiSetBlockBufferLevel(%d) failed for TI %d with %d\n", __FUNCTION__, params->TrgInhThr, params->Id, ret );
		return ret;
	}

	/* Set event format */
	// This corresponds to Timing word enabled: two block placeholdr is disabled
	// if ExtEidTstp is set higher 16 MSB-s are enabled for EiD and Tstp 
	if( (ret=tiSetEventFormat(params->ExtEidTstp*2+1)) != OK )
	{
		fprintf( stderr, "%s: tiSetEventFormat(%d) failed for TI %d with %d\n", __FUNCTION__, params->ExtEidTstp*2+1, params->Id, ret );
		return ret;
	}
	
	/* Get TI status */
	// For some reason this function is void.
	tiStatus(0);

	// All went fine
	return D_RetCode_Sucsess;
}


// Configure SD with parameters
int SdConfig( SdParams *params )
{
	int ret;

	// Check for Null pointer
	if( params == (SdParams *)NULL )
	{
		fprintf( stderr, "%s: params=0\n", __FUNCTION__ );
		return D_RetCode_Err_Null_Pointer;
	}
	
	// Check if the TI had already been initialized
	if( TIp == NULL )
	{
		fprintf( stderr, "%s: DS %d: looks that TI is not initialized yet, return with error\n", __FUNCTION__, params->Id );
		return D_RetCode_Err_Null_Pointer;
	}
	
	/* Initialize  */
	if( (ret=sdInit(0)) != OK )
	{
		fprintf( stderr, "%s: sdInit() failed for id=%d with %d\n", __FUNCTION__, params->Id, ret );
		return ret;
	}
	
	/* Set Active VME slots */
	if( (ret=sdSetActiveVmeSlots(params->ActiveSlotFlags)) != OK )
	{
		fprintf( stderr, "%s: sdSetActiveVmeSlots(0x%x) failed for SD %d with %d\n", __FUNCTION__, params->ActiveSlotFlags, params->Id, ret );
		return ret;
	}
	
	/* Get SD status */
	if( (ret=sdStatus()) != OK )
	{
		fprintf( stderr, "%s: sdStatus() failed for id=%d with %d\n", __FUNCTION__, params->Id, ret );
		return ret;
	}

	// All went fine
	return D_RetCode_Sucsess;
}

/*
 * Set TI in RUNNING or IDLE state
 */
int TiRun( TiParams *params, int run )
{
	int ret;

	// Check for Null pointer
	if( params == (TiParams *)NULL )
	{
		fprintf( stderr, "%s: params=0\n", __FUNCTION__ );
		return D_RetCode_Err_Null_Pointer;
	}

	if( run )
	{
		// Enable trigger and sync signals sent through the VXS
		if( (ret=tiEnableVXSSignals()) != OK )
		{
			fprintf( stderr, "%s: tiEnableVXSSignals failed with %d\n", __FUNCTION__, ret );
			return D_RetCode_Err_NetIO;
		}
		/* 
		 * Determine if the TI is master or slave 
		 * if the trigger source is set to be optical then TI is slave
		 * else it is a master
		 */
		if( params->TrgSrc != TiTrgSrc_HFBR1 )
		{
			// Do this only if the function is called from a standalone application
			if( stand_alone_app == 1 )
			{
//fprintf( stdout, "%s: **** Standalone APPLICATION ****\n", __FUNCTION__, params->BsySrc, params->Id );
//usleep(100000);
				tiClockReset();
				taskDelay(2);
				tiTrigLinkReset();
				tiSyncReset(0);
				taskDelay(2);
			}
		}
	}
	else
	{
		// Disable trigger sources 
		if( (ret=tiDisableTriggerSource(0)) != OK )
		{
			fprintf( stderr, "%s: tiDisableTriggerSource failed with %d\n", __FUNCTION__, ret );
			return D_RetCode_Err_NetIO;
		}
		// Disable trigger and sync signals sent through the VXS 
		if( (ret=tiDisableVXSSignals()) != OK )
		{
			fprintf( stderr, "%s: tiDisableVXSSignals failed with %d\n", __FUNCTION__, ret );
			return D_RetCode_Err_NetIO;
		}
	}

	// All went fine
	return D_RetCode_Sucsess;
}

/*
 * Enable trigger processing
 */
int TiGo( TiParams *params )
{
	int ret;

	// Check for Null pointer
	if( params == (TiParams *)NULL )
	{
		fprintf( stderr, "%s: params=0\n", __FUNCTION__ );
		return D_RetCode_Err_Null_Pointer;
	}
/*
	// Enable trigger and sync signals sent through the VXS
	if( (ret=tiEnableVXSSignals()) != OK )
	{
		fprintf( stderr, "%s: tiEnableVXSSignals failed with %d\n", __FUNCTION__, ret );
		return D_RetCode_Err_NetIO;
	}

	// Enable trigger sources 
	if( (ret=tiEnableTriggerSource()) != OK )
	{
		fprintf( stderr, "%s: tiEnableTriggerSource failed with %d\n", __FUNCTION__, ret );
		return D_RetCode_Err_NetIO;
	}
*/
	// All went fine
	return D_RetCode_Sucsess;
}

#else

void TiSdConfig_dummy() {}

#endif
