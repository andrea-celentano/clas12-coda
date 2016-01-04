/*
--------------------------------------------------------------------------------
-- Company:        IRFU / CEA Saclay
-- Engineers:      Irakli.MANDJAVIDZE@cea.fr (IM)
-- 
-- Project Name:   Clas12 Micromegas Vertex Tracker
-- Design Name:    Clas12 Dream testbench software
--
-- Module Name:    SysConfig.c
-- Description:    System configuration
--
-- Target Devices: Windows or Linux PC
-- Tool versions:  Windows Visual C++ or Linux Make
-- 
-- Create Date:    0.0 2014/10/15 IM
-- Revision:       
--
-- Comments:
--
--------------------------------------------------------------------------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

#include "Platform.h"
#include "ReturnCodes.h"
#include "BecConfigParams.h"
#include "FeuConfigParams.h"
#include "BeuConfig.h"
#include "SysConfig.h"

SysParams sys_params;
SysParams *sys_params_ptr = &sys_params;

// configuration file pointer
static FILE *sys_conf_params_fptr = (FILE *)NULL;

// Cleanup function
void SysConfig_CleanUp()
{
	FeuConfig_CleanUp();
	if(sys_conf_params_fptr != (FILE *)NULL)
	{
		fclose(sys_conf_params_fptr);
		sys_conf_params_fptr = (FILE *)NULL;
	}
}

/*
 * Configuration function reading parameters file
 */
int SysConfigFromFile( char *sys_conf_params_filename )
{
	// time variables
	time_t     cur_time;
	struct tm *time_struct;
	struct timeval t0;
	struct timeval t1;
	struct timeval dt;

	int ret;
	char filename[128];

	// Get execution time
	cur_time = time(NULL);
	time_struct = localtime(&cur_time);
	// Get start time for performance measurements
	gettimeofday(&t0,0);

	// Check for Null pointer
	if( sys_conf_params_filename == (char *)NULL )
	{
		fprintf( stderr, "%s: sys_conf_params_filename=NULL\n", __FUNCTION__ );
		return D_RetCode_Err_Null_Pointer;
	}

	/**********************
	 * Read configuration *
	 **********************/
	// Open config file
	if( (sys_conf_params_fptr=fopen(sys_conf_params_filename, "r")) == NULL )
	{
		fprintf( stderr, "%s: fopen failed for config file %s in read mode\n", __FUNCTION__, sys_conf_params_fptr );
		fprintf( stderr, "%s: fopen failed with %s\n", __FUNCTION__, strerror(errno) );
		return D_RetCode_Err_FileIO;
	}

	// Initialize parameters
	if( (ret = SysParams_Init( &sys_params ) ) != D_RetCode_Sucsess )
	{
		fprintf( stderr, "%s: SysParams_Init failed with %d\n", __FUNCTION__, ret );
		return ret;
	}

	// Read system parameters from file
	if( (ret = SysParams_Fread( &sys_params, sys_conf_params_fptr ) ) != D_RetCode_Sucsess )
	{
		fprintf( stderr, "%s: SysParams_Fread failed for config file %s with %d\n", __FUNCTION__, sys_conf_params_filename, ret );
		return ret;
	}

	// Propagate Global parameters
	if( (ret = SysParams_Prop( &sys_params )) != D_RetCode_Sucsess )
	{
		fprintf( stderr, "%s: SysParams_Prop failed with %d\n", __FUNCTION__, ret );
		return ret;
	}

	// Close config file
	fclose( sys_conf_params_fptr );
	sys_conf_params_fptr = (FILE *)NULL;

	/**********************
	 * Copy configuration *
	 **********************/
	// Prepare filename for configuration copy 
	sprintf
	(
		filename,
		"%s_%02d%02d%02d_%02dH%02d.cfg",
		rootfilename(sys_conf_params_filename),
		time_struct->tm_year%100, time_struct->tm_mon+1, time_struct->tm_mday,
		time_struct->tm_hour, time_struct->tm_min
	);

	// Open config file to copy configuration
	if( (sys_conf_params_fptr=fopen(filename, "w")) == NULL )
	{
		fprintf( stderr, "%s: fopen failed for config file %s in write mode\n", __FUNCTION__, filename );
		fprintf( stderr, "%s: fopen failed with %s\n", __FUNCTION__, strerror(errno) );
		return D_RetCode_Err_FileIO;
	}

	// Copy configuration to the file
	if( (ret = SysParams_Fprintf( &sys_params, sys_conf_params_fptr )) != D_RetCode_Sucsess )
	{
		fprintf( stderr, "%s: SysParams_Fprintf failed for config file %s with %d\n", __FUNCTION__, filename, ret );
		return ret;
	}
	
	// Close config file
	fclose( sys_conf_params_fptr );
	sys_conf_params_fptr = (FILE *)NULL;

	/*
	 * Configure the system
	 */
	if( (ret=SysConfig( &sys_params )) != D_RetCode_Sucsess )
	{
		fprintf( stderr, "%s: SysConfig failed for parameters from conf file %s with %d\n", __FUNCTION__, sys_conf_params_filename, ret );
		return ret;
	}

	// Get end time for performance measurements
	gettimeofday(&t1, 0);
	timersub(&t1,&t0,&dt);
	fprintf(stdout, "%s: The system has been configured in %d.%d sec\n", __FUNCTION__, dt.tv_sec, (int)(dt.tv_usec / 100000. + 0.5) );
	return D_RetCode_Sucsess;
}

/*
 * Configuration function
 */
int SysConfig( SysParams *params )
{
	int ret;
	int bec;
	int feu;
	int beu;
	int beu_lnk;
//	FeuParams *feu_def_params;
	FeuParams *feu_cur_params;
	
	// Check for Null pointer
	if( params == (SysParams *)NULL )
	{
		fprintf( stderr, "%s: params=0\n", __FUNCTION__ );
		return D_RetCode_Err_Null_Pointer;
	}

	/* First clear tables with pointers to the BEU control structures and data fifo-s */
	for( beu=1; beu<DEF_MAX_NB_OF_BEU; beu++ )
	{
		beu_reg_control[beu]=NULL;
		beu_fifo[beu]=NULL;
	}

	// Default parameters for  FEU-s
//	feu_def_params = &(params->FeuParams_Col.feu_params[0]);

	// Go through back end crates
	for( bec=1; bec<DEF_MAX_NB_OF_BEC; bec++ )
	{
		/* First configure TI */
		/* But do it only in Clas12 running mode */
		if( params->RunMode != Clas12 )
		{
			if( params->Ti_Params[bec].Id > 0 )
			{
				//if( (ret=TiConfig( &(params->Ti_Params[1]))) != D_RetCode_Sucsess )			// SOMETHING STRANGE HERE and below : should it be  params->Ti_Params[bec] ??
				if( (ret=TiConfig( &(params->Ti_Params[bec]))) != D_RetCode_Sucsess )			// SOMETHING STRANGE HERE and below : should it be  params->Ti_Params[bec] ??
				{
					fprintf(stderr, "%s: TiConfig failed with %d for ti %d\n", __FUNCTION__, ret, params->Ti_Params[bec].Id );
					return ret;
				}
			}
		}

		/* Next configure SD */
		if( params->Sd_Params[bec].Id > 0 )
		{
			if( (ret=SdConfig( &(params->Sd_Params[bec]))) != D_RetCode_Sucsess )  // SOMETHING STRANGE HERE and below : should it be  params->Sd_Params[bec] ??
			{
				fprintf(stderr, "%s: SdConfig failed with %d\n", __FUNCTION__, ret );
				return ret;
			}
		}

		/* Configure BEU-s */
		for( beu=1; beu<DEF_MAX_NB_OF_BEU; beu++ )
		{
			if( params->Bec_Params[bec].Beu_Id[beu] > 0 )
			{
				if( (params->Bec_Params[bec].Beu_Slot[beu]<DEF_MIN_BEU_SLOT) || (DEF_MAX_BEU_SLOT<=params->Bec_Params[bec].Beu_Slot[beu]) )
				{
					fprintf( stderr, "%s: usupported slot=%d for beu %d in bec %d, must be in [%d;%d] range\n",
						__FUNCTION__, params->Bec_Params[bec].Beu_Slot[beu], params->Bec_Params[bec].Beu_Id[beu], bec, DEF_MIN_BEU_SLOT, DEF_MAX_BEU_SLOT );
					return D_RetCode_Err_Wrong_Param;
				}
				if( (ret=beusspInit(params->BeuSspConf_Col.beu_conf[beu].base_adr_reg, &(beu_reg_control[beu]), &(beu_fifo[beu]), &(params->BeuSspConf_Col.beu_conf[beu]))) < 0 )
				{
					fprintf( stderr, "%s: beusspInit failed for beu %d in bec %d with %d\n",
						__FUNCTION__, beu, bec, ret );
					return D_RetCode_Err_Wrong_Param;
				}
				// Display BEU registers
				beusspDisplayAllReg(beu_reg_control[beu]);
			}
		} // for( beu=1; beu<DEF_MAX_NB_OF_BEU; feu++ )

		// If needed Multi module block transfer requested
		if( params->Bec_Params[bec].BaseAdr_A32m_Com_Enb )
		{
			if( (ret=beusspInitMblk(beu_reg_control, &BEUSSPmblk)) < 0 )
			{
				fprintf( stderr, "%s: beusspInitMblk failed for bec %d with %d\n",
					__FUNCTION__, bec, ret );
				return D_RetCode_Err_Wrong_Param;
			}

			/* Configure BEU-s */
			for( beu=1; beu<DEF_MAX_NB_OF_BEU; beu++ )
			{
				if( params->Bec_Params[bec].Beu_Id[beu] > 0 )
				{
					if( (ret=beusspEnableA32m(beu_reg_control[beu])) < 0 )
					{
						fprintf( stderr, "%s: beusspEnableA32m failed for beu %d in bec %d with %d\n",
							__FUNCTION__, beu, bec, ret );
						return D_RetCode_Err_Wrong_Param;
					}
					if( (ret=beusspEnableA32(beu_reg_control[beu])) < 0 )
					{
						fprintf( stderr, "%s: beusspEnableA32 failed for beu %d in bec %d with %d\n",
							__FUNCTION__, beu, bec, ret );
						return D_RetCode_Err_Wrong_Param;
					}

/*					if( (ret=beusspDisableA32(beu_reg_control[beu])) < 0 )
					{
						fprintf( stderr, "%s: beusspDisableA32 failed for beu %d in bec %d with %d\n",
							__FUNCTION__, beu, bec, ret );
						return D_RetCode_Err_Wrong_Param;
					}
*/						
					if( (ret=beusspEnBerrTerm(beu_reg_control[beu])) < 0 )
					{
						fprintf( stderr, "%s: beusspEnBerrTerm failed for beu %d in bec %d with %d\n",
							__FUNCTION__, beu, bec, ret );
						return D_RetCode_Err_Wrong_Param;
					}
					// Display BEU registers
					beusspDisplayAllReg(beu_reg_control[beu]);
				}
			} // for( beu=1; beu<DEF_MAX_NB_OF_BEU; feu++ )
		}
		else
		{
			/* Configure BEU-s */
			for( beu=1; beu<DEF_MAX_NB_OF_BEU; beu++ )
			{
				if( params->Bec_Params[bec].Beu_Id[beu] > 0 )
				{
					if( (ret=beusspDisableA32m(beu_reg_control[beu])) < 0 )
					{
						fprintf( stderr, "%s: beusspDisableA32m failed for beu %d in bec %d with %d\n",
							__FUNCTION__, beu, bec, ret );
						return D_RetCode_Err_Wrong_Param;
					}
					if( (ret=beusspEnableA32(beu_reg_control[beu])) < 0 )
					{
						fprintf( stderr, "%s: beusspEnableA32 failed for beu %d in bec %d with %d\n",
							__FUNCTION__, beu, bec, ret );
						return D_RetCode_Err_Wrong_Param;
					}
/*
					if( (ret=beusspEnBerrTerm(beu_reg_control[beu])) < 0 )
					{
						fprintf( stderr, "%s: beusspEnBerrTerm failed for beu %d in bec %d with %d\n",
							__FUNCTION__, beu, bec, ret );
						return D_RetCode_Err_Wrong_Param;
					}	
*/					
					//The readout list does not use bus error termination when reading BEUs separately, but it can be done !		
					if( (ret=beusspDisBerrTerm(beu_reg_control[beu])) < 0 )
					{
						fprintf( stderr, "%s: beusspDisBerrTerm failed for beu %d in bec %d with %d\n",
							__FUNCTION__, beu, bec, ret );
						return D_RetCode_Err_Wrong_Param;
					}
					
					// Display BEU registers
					beusspDisplayAllReg(beu_reg_control[beu]);
				}
			} // for( beu=1; beu<DEF_MAX_NB_OF_BEU; feu++ )
		} // if( params->Bec_Params[bec].BaseAdr_A32m_Com_Enb )

		/*
		 * Take care of FEUs
		 */
		/* First reset all FEU-s */
		for( feu=1; feu<DEF_MAX_NB_OF_FEU; feu++ )
		{
			if( params->Bec_Params[bec].FeuId2BeuId[feu] == 0 )
			{
				fprintf( stderr, "%s: Usupported BeuId 0 for feu %d in bec %d\n", __FUNCTION__, feu, bec );
				return D_RetCode_Err_Wrong_Param;
			}
			if( params->Bec_Params[bec].FeuId2BeuId[feu] > 0 )
			{
				if( (params->Bec_Params[bec].FeuId2BeuLnkId[feu]<0) || (DEF_MAX_NB_OF_FEU_PER_BEU<=params->Bec_Params[bec].FeuId2BeuLnkId[feu]) )
				{
					fprintf( stderr, "%s: Usupported beu %d link=%d for feu %d in bec %d, must be in [0;%d] range\n",
						__FUNCTION__, params->Bec_Params[bec].FeuId2BeuId[feu], params->Bec_Params[bec].FeuId2BeuLnkId[feu], feu, bec, DEF_MAX_NB_OF_FEU_PER_BEU-1 );
					return D_RetCode_Err_Wrong_Param;
				}
				beu        = params->Bec_Params[bec].FeuId2BeuId[feu];
				beu_lnk    = params->Bec_Params[bec].FeuId2BeuLnkId[feu];
				feu_cur_params = &(params->FeuParams_Col.feu_params[feu]);
				// Reset the FEU
				if( (ret=FeuReset( feu_cur_params, feu_cur_params->Feu_RunCtrl_Id, beu, beu_lnk )) != D_RetCode_Sucsess )
				{
					fprintf( stderr, "%s: FeuReset failed for feu %d beu %d link %d in bec %d with %d\n",
						__FUNCTION__, feu, beu, beu_lnk, bec, ret );
					return ret;
				}
				usleep(100000);
			}
		} // for( feu=1; feu<DEF_MAX_NB_OF_FEU; feu++ )

		// Wait before the reset is done and links are ready
		usleep( 500000 );

		/* Configure FEU-s */
		for( feu=1; feu<DEF_MAX_NB_OF_FEU; feu++ )
		{
			if( params->Bec_Params[bec].FeuId2BeuId[feu] == 0 )
			{
				fprintf( stderr, "%s: Usupported BeuId 0 for feu %d in bec %d\n", __FUNCTION__, feu, bec );
				return D_RetCode_Err_Wrong_Param;
			}
			if( params->Bec_Params[bec].FeuId2BeuId[feu] > 0 )
			{
				if( (params->Bec_Params[bec].FeuId2BeuLnkId[feu]<0) || (DEF_MAX_NB_OF_FEU_PER_BEU<=params->Bec_Params[bec].FeuId2BeuLnkId[feu]) )
				{
					fprintf( stderr, "%s: Usupported beu %d link=%d for feu %d in bec %d, must be in [0;%d] range\n",
						__FUNCTION__, params->Bec_Params[bec].FeuId2BeuId[feu], params->Bec_Params[bec].FeuId2BeuLnkId[feu], feu, bec, DEF_MAX_NB_OF_FEU_PER_BEU-1 );
					return D_RetCode_Err_Wrong_Param;
				}
				beu        = params->Bec_Params[bec].FeuId2BeuId[feu];
				beu_lnk    = params->Bec_Params[bec].FeuId2BeuLnkId[feu];
				feu_cur_params = &(params->FeuParams_Col.feu_params[feu]);
				// Configure the FEU
				if( (ret=FeuConfig( feu_cur_params, feu_cur_params->Feu_RunCtrl_Id, beu, beu_lnk )) != D_RetCode_Sucsess )
				{
					fprintf( stderr, "%s: FeuConfig failed for feu %d beu %d link %d in bec %d with %d\n",
						__FUNCTION__, feu, beu, beu_lnk, bec, ret );
					return ret;
				}
			}
		} // for( feu=1; feu<DEF_MAX_NB_OF_FEU; feu++ )

		/* Synchronize BEU - FEU optical links */
		for( beu=1; beu<DEF_MAX_NB_OF_BEU; beu++ )
		{
			if( params->Bec_Params[bec].Beu_Id[beu] > 0 )
			{
				if( (ret=beusspResetGClkPll( beu_reg_control[beu] ) ) < 0 )
				{
					fprintf( stderr, "%s: beusspResetGClkPll failed for beu %d in bec %d with %d\n",
						__FUNCTION__, beu, bec, ret );
					return D_RetCode_Err_Wrong_Param;
				}
				if( (ret=beusspResetMultiGTX( beu_reg_control[beu] ) ) < 0 )
				{
					fprintf( stderr, "%s: beusspResetMultiGTX failed for beu %d in bec %d with %d\n",
						__FUNCTION__, beu, bec, ret );
					return D_RetCode_Err_Wrong_Param;
				}

				// Display BEU registers
				beusspDisplayAllReg(beu_reg_control[beu]);
			}
		} // for( beu=1; beu<DEF_MAX_NB_OF_BEU; feu++ )
	} // for( bec=1; bec<DEF_MAX_NB_OF_BEC; bec++ )

	return D_RetCode_Sucsess;
}

/*
 * Set system to running state
 */
int SysRun()
{
	int ret;
	int bec;
	int feu;
	int beu;
	int beu_lnk;
//	FeuParams *feu_def_params;
	FeuParams *feu_cur_params;
	
	// Check for Null pointer
	if( sys_params_ptr == (SysParams *)NULL )
	{
		fprintf( stderr, "%s: params=0\n", __FUNCTION__ );
		return D_RetCode_Err_Null_Pointer;
	}

	// Go through back end crates
	for( bec=1; bec<DEF_MAX_NB_OF_BEC; bec++ )
	{
		/* Configure FEU-s */
		for( feu=1; feu<DEF_MAX_NB_OF_FEU; feu++ )
		{
			if( sys_params_ptr->Bec_Params[bec].FeuId2BeuId[feu] == 0 )
			{
				fprintf( stderr, "%s: Usupported BeuId 0 for feu %d in bec %d\n", __FUNCTION__, feu, bec );
				return D_RetCode_Err_Wrong_Param;
			}
			if( sys_params_ptr->Bec_Params[bec].FeuId2BeuId[feu] > 0 )
			{
				if( (sys_params_ptr->Bec_Params[bec].FeuId2BeuLnkId[feu]<0) || (DEF_MAX_NB_OF_FEU_PER_BEU<=sys_params_ptr->Bec_Params[bec].FeuId2BeuLnkId[feu]) )
				{
					fprintf( stderr, "%s: Usupported beu %d link=%d for feu %d in bec %d, must be in [0;%d] range\n",
						__FUNCTION__, sys_params_ptr->Bec_Params[bec].FeuId2BeuId[feu], sys_params_ptr->Bec_Params[bec].FeuId2BeuLnkId[feu], feu, bec, DEF_MAX_NB_OF_FEU_PER_BEU-1 );
					return D_RetCode_Err_Wrong_Param;
				}
				beu        = sys_params_ptr->Bec_Params[bec].FeuId2BeuId[feu];
				beu_lnk    = sys_params_ptr->Bec_Params[bec].FeuId2BeuLnkId[feu];
				feu_cur_params = &(sys_params_ptr->FeuParams_Col.feu_params[feu]);
				// Then set specific parameters
				if( (ret=FeuRun( feu_cur_params, feu_cur_params->Feu_RunCtrl_Id, beu, beu_lnk, 1 )) != D_RetCode_Sucsess )
				{
					fprintf( stderr, "%s: FeuRun(1) failed for feu %d beu %d link %d in bec %d with %d\n",
						__FUNCTION__, feu, beu, beu_lnk, bec, ret );
					return ret;
				}
			}
		} // for( feu=1; feu<DEF_MAX_NB_OF_FEU; feu++ )

		/* Configure BEU-s */
		for( beu=1; beu<DEF_MAX_NB_OF_BEU; beu++ )
		{
			if( sys_params_ptr->Bec_Params[bec].Beu_Id[beu] > 0 )
			{
				if( (sys_params_ptr->Bec_Params[bec].Beu_Slot[beu]<DEF_MIN_BEU_SLOT) || (DEF_MAX_BEU_SLOT<=sys_params_ptr->Bec_Params[bec].Beu_Slot[beu]) )
				{
					fprintf( stderr, "%s: usupported slot=%d for beu %d in bec %d, must be in [%d;%d] range\n",
						__FUNCTION__, sys_params_ptr->Bec_Params[bec].Beu_Slot[beu], sys_params_ptr->Bec_Params[bec].Beu_Id[beu], bec, DEF_MIN_BEU_SLOT, DEF_MAX_BEU_SLOT );
					return D_RetCode_Err_Wrong_Param;
				}
				if( (ret=BeuRun(beu_reg_control[beu], 1)) !=  D_RetCode_Sucsess )
				{
					fprintf( stderr, "%s: BeuRun(1) failed for beu %d in bec %d with %d\n",
						__FUNCTION__, beu, bec, ret );
					return ret;
				}
			}
		} // for( beu=1; beu<DEF_MAX_NB_OF_BEU; feu++ )

		/* Configure TI */
		if( sys_params_ptr->Ti_Params[bec].Id > 0 )
		{
			if( (ret=TiRun( &(sys_params_ptr->Ti_Params[1]), 1)) != D_RetCode_Sucsess )
			{
				fprintf(stderr, "%s: TiRun(1) failed with %d for ti %d\n", __FUNCTION__, ret, sys_params_ptr->Ti_Params[1].Id );
				return ret;
			}
		}
	} // for( bec=1; bec<DEF_MAX_NB_OF_BEC; bec++ )

	return D_RetCode_Sucsess;
}

/*
 * Bring system to idle state
 */
int SysStop()
{
	int ret;
	int bec;
	int feu;
	int beu;
	int beu_lnk;
//	FeuParams *feu_def_params;
	FeuParams *feu_cur_params;
	
	// Check for Null pointer
	if( sys_params_ptr == (SysParams *)NULL )
	{
		fprintf( stderr, "%s: params=0\n", __FUNCTION__ );
		return D_RetCode_Err_Null_Pointer;
	}

	// Go through back end crates
	for( bec=1; bec<DEF_MAX_NB_OF_BEC; bec++ )
	{
		/* Configure TI */
		if( sys_params_ptr->Ti_Params[bec].Id > 0 )
		{
			if( (ret=TiRun( &(sys_params_ptr->Ti_Params[1]), 0)) != D_RetCode_Sucsess )
			{
				fprintf(stderr, "%s: TiRun(0) failed with %d for ti %d\n", __FUNCTION__, ret, sys_params_ptr->Ti_Params[1].Id );
				return ret;
			}
		}

		/* Configure BEU-s */
		for( beu=1; beu<DEF_MAX_NB_OF_BEU; beu++ )
		{
			if( sys_params_ptr->Bec_Params[bec].Beu_Id[beu] > 0 )
			{
				if( (sys_params_ptr->Bec_Params[bec].Beu_Slot[beu]<DEF_MIN_BEU_SLOT) || (DEF_MAX_BEU_SLOT<=sys_params_ptr->Bec_Params[bec].Beu_Slot[beu]) )
				{
					fprintf( stderr, "%s: usupported slot=%d for beu %d in bec %d, must be in [%d;%d] range\n",
						__FUNCTION__, sys_params_ptr->Bec_Params[bec].Beu_Slot[beu], sys_params_ptr->Bec_Params[bec].Beu_Id[beu], bec, DEF_MIN_BEU_SLOT, DEF_MAX_BEU_SLOT );
					return D_RetCode_Err_Wrong_Param;
				}
				if( (ret=BeuRun(beu_reg_control[beu], 0)) !=  D_RetCode_Sucsess )
				{
					fprintf( stderr, "%s: BeuRun(0) failed for beu %d in bec %d with %d\n",
						__FUNCTION__, beu, bec, ret );
					return ret;
				}
				// Display BEU registers
				fprintf(stdout, "%s: Beu %d Registers in bec %d slot %d\n", __FUNCTION__, sys_params_ptr->Bec_Params[bec].Beu_Id[beu], bec, sys_params_ptr->Bec_Params[bec].Beu_Slot[beu] );
				beusspDisplayAllReg(beu_reg_control[beu]);
			}
		} // for( beu=1; beu<DEF_MAX_NB_OF_BEU; feu++ )

		/* Configure FEU-s */
		for( feu=1; feu<DEF_MAX_NB_OF_FEU; feu++ )
		{
			if( sys_params_ptr->Bec_Params[bec].FeuId2BeuId[feu] == 0 )
			{
				fprintf( stderr, "%s: Usupported BeuId 0 for feu %d in bec %d\n", __FUNCTION__, feu, bec );
				return D_RetCode_Err_Wrong_Param;
			}
			if( sys_params_ptr->Bec_Params[bec].FeuId2BeuId[feu] > 0 )
			{
				if( (sys_params_ptr->Bec_Params[bec].FeuId2BeuLnkId[feu]<0) || (DEF_MAX_NB_OF_FEU_PER_BEU<=sys_params_ptr->Bec_Params[bec].FeuId2BeuLnkId[feu]) )
				{
					fprintf( stderr, "%s: Usupported beu %d link=%d for feu %d in bec %d, must be in [0;%d] range\n",
						__FUNCTION__, sys_params_ptr->Bec_Params[bec].FeuId2BeuId[feu], sys_params_ptr->Bec_Params[bec].FeuId2BeuLnkId[feu], feu, bec, DEF_MAX_NB_OF_FEU_PER_BEU-1 );
					return D_RetCode_Err_Wrong_Param;
				}
				beu        = sys_params_ptr->Bec_Params[bec].FeuId2BeuId[feu];
				beu_lnk    = sys_params_ptr->Bec_Params[bec].FeuId2BeuLnkId[feu];
				feu_cur_params = &(sys_params_ptr->FeuParams_Col.feu_params[feu]);
				// Then set specific parameters
				if( (ret=FeuRun( feu_cur_params, feu_cur_params->Feu_RunCtrl_Id, beu, beu_lnk, 0 )) != D_RetCode_Sucsess )
				{
					fprintf( stderr, "%s: FeuRun(0) failed for feu %d beu %d link %d in bec %d with %d\n",
						__FUNCTION__, feu, beu, beu_lnk, bec, ret );
					return ret;
				}
			}
		} // for( feu=1; feu<DEF_MAX_NB_OF_FEU; feu++ )
	} // for( bec=1; bec<DEF_MAX_NB_OF_BEC; bec++ )

	return D_RetCode_Sucsess;
}

/*
 * Enable trigger processing
 */
int SysGo()
{
	int ret;
	int bec;
	int feu;
	int beu;
	int beu_lnk;
	
	// Check for Null pointer
	if( sys_params_ptr == (SysParams *)NULL )
	{
		fprintf( stderr, "%s: params=0\n", __FUNCTION__ );
		return D_RetCode_Err_Null_Pointer;
	}

	// Go through back end crates
	for( bec=1; bec<DEF_MAX_NB_OF_BEC; bec++ )
	{
		/* Configure BEU-s */
		for( beu=1; beu<DEF_MAX_NB_OF_BEU; beu++ )
		{
			if( sys_params_ptr->Bec_Params[bec].Beu_Id[beu] > 0 )
			{
				if( (sys_params_ptr->Bec_Params[bec].Beu_Slot[beu]<DEF_MIN_BEU_SLOT) || (DEF_MAX_BEU_SLOT<=sys_params_ptr->Bec_Params[bec].Beu_Slot[beu]) )
				{
					fprintf( stderr, "%s: usupported slot=%d for beu %d in bec %d, must be in [%d;%d] range\n",
						__FUNCTION__, sys_params_ptr->Bec_Params[bec].Beu_Slot[beu], sys_params_ptr->Bec_Params[bec].Beu_Id[beu], bec, DEF_MIN_BEU_SLOT, DEF_MAX_BEU_SLOT );
					return D_RetCode_Err_Wrong_Param;
				}
				if( (ret=BeuGo(beu_reg_control[beu], &(sys_params_ptr->BeuSspConf_Col.beu_conf[beu]))) !=  D_RetCode_Sucsess )
				{
					fprintf( stderr, "%s: BeuGo failed for beu %d in bec %d with %d\n",
						__FUNCTION__, beu, bec, ret );
					return ret;
				}
			}
		} // for( beu=1; beu<DEF_MAX_NB_OF_BEU; feu++ )

		/* Configure TI */
		if( sys_params_ptr->Ti_Params[bec].Id > 0 )
		{
			if( (ret=TiGo( &(sys_params_ptr->Ti_Params[1]))) != D_RetCode_Sucsess )
			{
				fprintf(stderr, "%s: TiGo failed with %d for ti %d\n", __FUNCTION__, ret, sys_params_ptr->Ti_Params[1].Id );
				return ret;
			}
		}
	} // for( bec=1; bec<DEF_MAX_NB_OF_BEC; bec++ )

	return D_RetCode_Sucsess;
}

