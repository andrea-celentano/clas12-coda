/*
--------------------------------------------------------------------------------
-- Company:        IRFU / CEA Saclay
-- Engineers:      Irakli.MANDJAVIDZE@cea.fr (IM)
-- 
-- Project Name:   Clas12 Micromegas Vertex Tracker
-- Design Name:    Clas12 Dream testbench software
--
-- Module Name:    TiConfig.c
-- Description:    Trigger Interface (TI) configuration
--
-- Target Devices: 
-- Tool versions:  
-- 
-- Create Date:    0.0 2014/10/13 IM
-- Revision:       1.0 2014/11/20 IM Take into account new Feu_RunCtrl_EvTstExt parameter
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
#include "Parser.h"
#include "BecConfigParams.h"

#include "CBus.h"
#include "CBus_Common.h"
#include "CBus_Common.h"
#include "Feu.h"
#include "PedMem.h"
#include "ThreshMem.h"
#include "TrigGen.h"
#include "TrigInt.h"
#include "SelfTrig.h"
#include "Spi_Dream.h"
#include "Spi_Ad9222.h"
#include "Semaphores.h"
#include "DrmClk.h"
#include "UdpChan.h"

#include "BeuConfig.h"

#include "FeuConfigParams.h"
#include "SysConfig.h"

/*
 * Beu slow control interface
 */
/*******************************************************************
  *************** Beu slow control interface ***********************
  ******************************************************************/
 #ifndef OK
 #define OK 0
 #endif 

#define DEF_CMD_INDEX_MASK 0x1F
#define DEF_FEU_WRITE         0
#define DEF_FEU_READ       0x20
#define DEF_FEU_BCST       0x40
#define DEF_FEU_ERROR      0x40

static int sem_init_done[DEF_MAX_NB_OF_FEU];
static int sem_init_initialized = 0;
static int cmd_index[DEF_MAX_NB_OF_FEU];
static int cmd_index_initialized = 0;

// This is a hidden function, so we rely on the fact that all parameters were correctly set up and checked in calling function
// So no parameters check
#define DEF_LNK_TIME_OUT_SEC 2
#define DEF_LnkToValInit() {.tv_sec=DEF_LNK_TIME_OUT_SEC, .tv_usec=0}
int Beu_CheckFeuLink(int feu, int beu, int lnk)
{
	struct timeval t0;
	struct timeval t1;
	struct timeval dt;
	struct timeval to = DEF_LnkToValInit();
	int ret;
	volatile struct BEUSSP_A24RegStruct *beu_reg_struct;

	// Identify control structure of active BEU
	if( (beu_reg_struct = beu_reg_control[beu]) == NULL )
	{
		fprintf( stderr, "%s: beu_reg_struct NULL for beu %d\n", __FUNCTION__, beu ); 
		return D_RetCode_Err_Null_Pointer;
	}

	// Check the link
	gettimeofday(&t0,0);
	while(1)
	{
		ret = beusspCheckFeuLink(beu_reg_struct, lnk);
		if( ret < 0 )
		{
			fprintf( stderr, "%s: beusspCheckFeuLink failed for beu=%d lnk=%d\n\r", __FUNCTION__, beu, lnk );
		}
		else if( ret == 1 )
			break;
		else
		{
			gettimeofday(&t1, 0);
			timersub(&t1,&t0,&dt);
			if( timercmp(&dt,&to,>) )
			{
				fprintf( stderr, "%s: timeout\n\r", __FUNCTION__);
printf("%s: timeout: t0 sec=%8d usec=%6d\n\r", __FUNCTION__, t0.tv_sec, t0.tv_usec);
printf("%s: timeout: t1 sec=%8d usec=%6d\n\r", __FUNCTION__, t1.tv_sec, t1.tv_usec);
printf("%s: timeout: dt sec=%8d usec=%6d\n\r", __FUNCTION__, dt.tv_sec, dt.tv_usec);
printf("%s: timeout: to sec=%8d usec=%6d\n\r", __FUNCTION__, to.tv_sec, to.tv_usec);
			return D_RetCode_Err_Timeout;
			}
			else
				usleep(1000);
		}
	}
	return D_RetCode_Sucsess;
}
			

// This is a hidden function, so we rely on the fact that all parameters were correctly set up and checked in calling function
// So no parameters check
int Beu_ReqResp(int feu, int beu, int lnk, unsigned int adr, unsigned int data_in, int dir, unsigned int *data_out)
{
	unsigned int wr_adr_wrd;
	unsigned int rd_adr_wrd;
	unsigned int rd_dat_wrd;
	int ret;
	int index;
	volatile struct BEUSSP_A24RegStruct *beu_reg_struct;

	// Initiakize command counters if called for the first time
	if( cmd_index_initialized == 0 )
	{
		cmd_index_initialized = 1;
		for( index=0; index<DEF_MAX_NB_OF_FEU; index++ )
		{
			cmd_index[index] = 0;
		}
	}
	
	// Identify control structure of active BEU
	if( (beu_reg_struct = beu_reg_control[beu]) == NULL )
	{
		fprintf( stderr, "%s: beu_reg_struct NULL for beu %d\n", __FUNCTION__, beu ); 
		return D_RetCode_Err_Null_Pointer;
	}
//printf( "%s: beu_reg_struct=0x%08x beu adrgeo=0x%08x feu=%d beu=%d lnk=%d wr_adr_wrd=0x%08x\n",
//__FUNCTION__, beu_reg_struct, beu_reg_struct->adrgeo, feu, beu, lnk, wr_adr_wrd );
//				beusspDisplayAllReg(beu_reg_struct);

	// Prepare command
	cmd_index[feu] = (cmd_index[feu] + 1) & DEF_CMD_INDEX_MASK;
	wr_adr_wrd = cmd_index[feu];
	if( dir == DEF_FEU_READ )
		wr_adr_wrd |= DEF_FEU_READ;
	wr_adr_wrd = ((wr_adr_wrd << 24) & 0xFF000000) | (adr & 0xFFffFF);
//printf( "%s: feu=%d beu=%d lnk=%d wr_adr_wrd=0x%08x\n", __FUNCTION__, feu, beu, lnk, wr_adr_wrd );
	// Call BEU slowcontrol function
	if( (ret=beusspSendSlowControl(beu_reg_struct, lnk, &wr_adr_wrd, &data_in, &rd_adr_wrd, data_out)) != OK )
	{
		fprintf( stderr, "%s: beusspSendSlowControl fail for feu=%d beu=%d lnk=%d with ret %d\n", __FUNCTION__, feu, beu, lnk, ret ); 
		return ret;
	}

	// Check here as musch as one can
	if( ((wr_adr_wrd >> 24) & DEF_CMD_INDEX_MASK) != ((rd_adr_wrd >> 24) & DEF_CMD_INDEX_MASK) )
	{
		fprintf( stderr, "%s: for feu=%d beu=%d lnk=%d cmd index missmatch %d expected - %d received\n",
			__FUNCTION__, feu, beu, lnk, ((wr_adr_wrd >> 24) & DEF_CMD_INDEX_MASK), ((rd_adr_wrd >> 24) & DEF_CMD_INDEX_MASK)  ); 
		return D_RetCode_Err_NetIO;
	}
	if( ((wr_adr_wrd >> 24) & DEF_FEU_READ) != ((rd_adr_wrd >> 24) & DEF_FEU_READ))
	{
		fprintf( stderr, "%s: for feu=%d beu=%d lnk=%d direction missmatch %d expected - %d received\n",
			__FUNCTION__, feu, beu, lnk, ((wr_adr_wrd >> 24) & DEF_CMD_INDEX_MASK), ((rd_adr_wrd >> 24) & DEF_CMD_INDEX_MASK)  ); 
		return D_RetCode_Err_NetIO;
	}
	if( ((rd_adr_wrd >> 24) & DEF_FEU_ERROR))
	{
		fprintf( stderr, "%s: for feu=%d beu=%d lnk=%d error set in response rd_adr_wrd=0x%08x\n",
			__FUNCTION__, feu, beu, lnk, rd_adr_wrd ); 
		return D_RetCode_Err_NetIO;
	}
	return D_RetCode_Sucsess;
}

/*******************************************************************
 ****************** Peek / Poke definitions ************************
 ****************** Dirty and Uglu quickfix ************************
 *******************************************************************/
int PeekPoke_Feu =  0;
int PeekPoke_Beu = -1;
int PeekPoke_Lnk = -1;
int PeekPoke_Err =  0;
char  feu_msg_container[128];
char *feu_msg = feu_msg_container;
unsigned int Peek( unsigned int adr )
{
	int ret;
	int data_out;
	if( (ret=Beu_ReqResp( PeekPoke_Feu, PeekPoke_Beu, PeekPoke_Lnk, adr, 0, DEF_FEU_READ, &data_out)) != D_RetCode_Sucsess )
	{
		fprintf( stderr, "%s: adr=0x%08x for feu=%d beu=%d lnk=%d failed wirth %d\n",
			__FUNCTION__, adr, PeekPoke_Feu, PeekPoke_Beu, PeekPoke_Lnk, ret );
		PeekPoke_Err = ret;
		return ret;
	}
	return data_out;
}
int Poke( unsigned int adr, unsigned int dat )
{
	int ret;
	int data_out;
	if( (ret=Beu_ReqResp( PeekPoke_Feu, PeekPoke_Beu, PeekPoke_Lnk, adr, dat, DEF_FEU_WRITE, &data_out)) != D_RetCode_Sucsess )
	{
		fprintf( stderr, "%s: adr=0x%08x dat=0x%08x for feu=%d beu=%d lnk=%d failed wirth %d\n",
			__FUNCTION__, adr, dat, PeekPoke_Feu, PeekPoke_Beu, PeekPoke_Lnk, ret );
		PeekPoke_Err = ret;
		return ret;
	}
//fprintf( stdoutr, "%s: for feu=%d beu=%d lnk=%d adr=0x%68x data_in=0x%08x dataout=0x%08x\n,
//__FUNCTION__, PeekPoke_Feu, PeekPoke_Beu, PeekPoke_Lnk, adr, dat, data_out);
	return D_RetCode_Sucsess;
}

/*****************************************************************
 ******************       Adc Config      ************************
 *****************************************************************/
int AdcConfig( AdcParams *adc_params, int feu_id, int beu_id, int beu_lnk_id )
{
	int ret;
	int reg;
	unsigned char wr_val;
	unsigned char rd_val;

	// First set the ADC registers
	for( reg=1; reg<D_AdcPar_NumOfRegs; reg++ )
	{
		if( adc_params->adc_reg[reg].flg != ((char)AdcRegFlag_Unset) )
		{
//fprintf( stderr,  "%s: Spi_AdcRegWrite(0x%02x, 0x%02x) for feu=%d beu=%d lnk=%d\n",
//	__FUNCTION__, reg, adc_params->adc_reg[reg].val, feu_id, beu_id, beu_lnk_id );
			wr_val = adc_params->adc_reg[reg].val;
			if( (ret = Spi_AdcRegWrite( (unsigned short)reg, &wr_val )) !=  D_RetCode_Sucsess )
			{
				fprintf( stderr,  "%s: Spi_AdcRegWrite(0x%02x, 0x%02x) failed for feu=%d beu=%d lnk=%d with %d\n",
					__FUNCTION__, reg, adc_params->adc_reg[reg].val, feu_id, beu_id, beu_lnk_id, ret );
				return ret;
			}
			// If requested: force Adc to take into account internal register values
			if( adc_params->adc_reg[reg].flg == AdcRegFlag_SetAndUpd )
			{
				wr_val = 0x01;
				if( Spi_AdcRegWrite( 0xFF, &wr_val ) !=  D_RetCode_Sucsess )
				{
					fprintf( stderr,  "%s: Spi_AdcRegWrite( 0xFF, 0x01 ) failed for feu=%d beu=%d lnk=%d with %d\n",
						__FUNCTION__, feu_id, beu_id, beu_lnk_id, ret );
					return ret;
				}
				wr_val = 0x00;
				if( Spi_AdcRegWrite( 0xFF, &wr_val ) !=  D_RetCode_Sucsess )
				{
					fprintf( stderr,  "%s: Spi_AdcRegWrite( 0xFF, 0x00 ) failed for feu=%d beu=%d lnk=%d with %d\n",
						__FUNCTION__, feu_id, beu_id, beu_lnk_id, ret );
					return ret;
				}
			} // if( adc_params->adc_reg[reg].flg == AdcRegFlag_SetAndUpd )
		} // if( feu_params->adc_params.adc_reg[reg].flg != AdcRegFlag_Unset )
	} // for( reg=1; reg<D_AdcPar_NumOfRegs; reg++ )

	// Now verify the ADC registers
	for( reg=1; reg<D_AdcPar_NumOfRegs; reg++ )
	{
		if( adc_params->adc_reg[reg].flg != ((char)AdcRegFlag_Unset) )
		{
			if( (ret = Spi_AdcRegRead( (unsigned short)reg, &rd_val )) !=  D_RetCode_Sucsess )
			{
				fprintf( stderr,  "%s: Spi_AdcRegRead(0x%02x) failed for feu=%d beu=%d lnk=%d with %d\n",
					__FUNCTION__, reg, feu_id, beu_id, beu_lnk_id, ret );
				return ret;
			}
//fprintf( stderr,  "%s: Spi_AdcRegRead(0x%02x)=0x%02x for feu=%d beu=%d lnk=%d with %d\n",
//	__FUNCTION__, reg, (rd_val&0xFF), feu_id, beu_id, beu_lnk_id, ret );
			if( adc_params->adc_reg[reg].val != (rd_val&0xFF) )
			{
				fprintf( stderr, "%s: AdcRead failed for feu=%d beu=%d lnk=%d Adc reg=0x%02x (%d) reg_val_exp=0x%02x != reg_val_rcv=0x%02x\n",
					__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg, reg, adc_params->adc_reg[reg].val, rd_val );
				return D_RetCode_Err_WrRd_Missmatch;
			}
		} // if( adc_params->adc_reg[reg].flg != AdcRegFlag_Unset )
	} // for( reg=1; reg<D_AdcPar_NumOfRegs; reg++ )
	return D_RetCode_Sucsess;
}

/*******************************************************************
 ******************       Dream Write       ************************
 ****************** Dirty and Uglu quickfix ************************
 *******************************************************************/
int DreamWrite( int id, unsigned char reg, int val[4] )
{
	int ret;
	unsigned short dream_val[4];
	dream_val[0] = (unsigned short)val[0];
	dream_val[1] = (unsigned short)val[1];
	dream_val[2] = (unsigned short)val[2];
	dream_val[3] = (unsigned short)val[3];
//fprintf( stdout, "%s: id=%d reg=%2d val[0]=0x%08x dream_val[0]=0x%04x\n", __FUNCTION__, id, reg, val[0], dream_val[0]);
	if( (ret=Spi_DreamRegWrite(id, reg, dream_val)) !=  D_RetCode_Sucsess )
	{
		fprintf(stderr, "%s: Spi_DreamRegWrite failed for d=%d a=%d d=0x%08x 0x%08x 0x%08x 0x%08x with %s\n\r",
			__FUNCTION__, id, reg, dream_val[0], dream_val[1], dream_val[2], dream_val[3], feu_msg );
		return ret;
	}
	return D_RetCode_Sucsess;
}
	
/*******************************************************************
 ******************       Dream Read        ************************
 ****************** Dirty and Uglu quickfix ************************
 *******************************************************************/
int DreamRead( int id, unsigned char reg, int val[4] )
{
	int ret;
	int dream;
	unsigned short dream_val[4];
	if( (ret=Spi_DreamRegRead(id, reg, dream_val)) !=  D_RetCode_Sucsess )
	{
		fprintf(stderr, "%s: Spi_DreamRegRead failed for d=%d a=%d d=0x%08x 0x%08x 0x%08x 0x%08x with %s\n\r",
			__FUNCTION__, id, reg, dream_val[0], dream_val[1], dream_val[2], dream_val[3], feu_msg );
		return ret;
	}
	val[0] = (int)( dream_val[0] & 0xFFFF );
	val[1] = (int)( dream_val[1] & 0xFFFF );
	val[2] = (int)( dream_val[2] & 0xFFFF );
	val[3] = (int)( dream_val[3] & 0xFFFF );
//fprintf( stdout, "%s: id=%d reg=%2d val[0]=0x%08x dream_val[0]=0x%04x\n", __FUNCTION__, id, reg, val[0], dream_val[0]);
	return D_RetCode_Sucsess;
}

/*******************************************************************
 ******************       Dream Config      ************************
 *******************************************************************/
int DreamConfig( DreamParams dream_params[D_FeuPar_NumOfDreams], int feu_id, int beu_id, int beu_lnk_id, int msk )
{
	int ret;
	int dream;
	int reg;
	unsigned int rd_val[4];

	/*
	 * Dream configuration: common to all dreams
	 */
/*
	for( reg=1; reg<D_DreamPar_NumOfRegs; reg++ )
	{
		// Write
		if( dream_params[D_FeuPar_NumOfDreams-1].dream_reg[reg].reg[0] != -1 )
		{
			for( dream=0; dream<D_FeuPar_NumOfDreams-1; dream++ )
			{
				if( (msk & (1 << dream)) == 0 )
				{
					if( (ret=DreamWrite( dream, reg, dream_params[D_FeuPar_NumOfDreams-1].dream_reg[reg].reg )) != D_RetCode_Sucsess )
					{
						fprintf( stderr,  "%s: DreamWrite failed for feu=%d beu=%d lnk=%d dream=%d reg=%d with %d\n",
							__FUNCTION__, feu_id, beu_id, beu_lnk_id, D_FeuPar_NumOfDreams-1, reg, ret );
						return ret;
					}
				}
			}

			// Read & compare
			for( dream=0; dream<D_FeuPar_NumOfDreams-1; dream++ )
			{
				if( (msk & (1 << dream)) == 0 )
				{
					// Read
					if( (ret=DreamRead( dream, reg, rd_val )) != D_RetCode_Sucsess )
					{
						fprintf( stderr,  "%s: DreamRead failed for feu=%d beu=%d lnk=%d dream=%d reg=%d with %d\n",
							__FUNCTION__, feu_id, beu_id, beu_lnk_id, D_FeuPar_NumOfDreams-1, reg, ret );
						return ret;
					}
					// Compare
					// all types of registers
					if( dream_params[D_FeuPar_NumOfDreams-1].dream_reg[reg].reg[0] != rd_val[0] )
					{
						fprintf( stderr, "%s: DreamRead failed for feu=%d beu=%d lnk=%d dream=%d reg=%d wr0=0x%x != rd0=0x%x\n",
							__FUNCTION__, feu_id, beu_id, beu_lnk_id, dream, reg, dream_params[D_FeuPar_NumOfDreams-1].dream_reg[reg].reg[0], rd_val[0] );
						return D_RetCode_Err_WrRd_Missmatch;
					}
					// > 16-bit registers
					if( (reg!=12) && (dream_params[D_FeuPar_NumOfDreams-1].dream_reg[reg].reg[1]) != rd_val[1] )
					{
						fprintf( stderr, "%s: DreamRead failed for feu=%d beu=%d lnk=%d dream=%d reg=%d wr1=0x%x != rd1=0x%x\n",
							__FUNCTION__, feu_id, beu_id, beu_lnk_id, dream, reg, dream_params[D_FeuPar_NumOfDreams-1].dream_reg[reg].reg[1], rd_val[1] );
						return D_RetCode_Err_WrRd_Missmatch;
					}
					// 64-bit registers
					if( (6<=reg) && (reg<=9) )
					{
						if( dream_params[D_FeuPar_NumOfDreams-1].dream_reg[reg].reg[2] != rd_val[2] )
						{
							fprintf( stderr, "%s: DreamRead failed for feu=%d beu=%d lnk=%d dream=%d reg=%d wr1=0x%x != rd1=0x%x\n",
								__FUNCTION__, feu_id, beu_id, beu_lnk_id, dream, reg, dream_params[D_FeuPar_NumOfDreams-1].dream_reg[reg].reg[2], rd_val[2] );
							return D_RetCode_Err_WrRd_Missmatch;
						}
						if( dream_params[D_FeuPar_NumOfDreams-1].dream_reg[reg].reg[3] != rd_val[3] )
						{
							fprintf( stderr, "%s: DreamRead failed for feu=%d beu=%d lnk=%d dream=%d reg=%d wr1=0x%x != rd1=0x%x\n",
								__FUNCTION__, feu_id, beu_id, beu_lnk_id, dream, reg, dream_params[D_FeuPar_NumOfDreams-1].dream_reg[reg].reg[2], rd_val[2] );
							return D_RetCode_Err_WrRd_Missmatch;
						}
					}
				} // if( (msk & (1 << dream)) == 0 )
			} // for( dream=0; dream<DEF_MAX_NB_OF_DREAM; dream++ )
		} // if( dream_params[D_FeuPar_NumOfDreams-1].dream_reg[reg].reg[0] != -1 )
	} // for( reg=1; reg<D_DreamPar_NumOfRegs; reg++ )
*/
	/*
	 * Dream configuration: dream specific parameters if any
	 */
	for( dream=0; dream<D_FeuPar_NumOfDreams-1; dream++ )
	{
		if( (msk & (1 << dream)) == 0 )
		{
			for( reg=1; reg<D_DreamPar_NumOfRegs; reg++ )
			{
				if( dream_params[dream].dream_reg[reg].reg[0] != -1 )
				{
					// Write
					if( (ret=DreamWrite( dream, reg, dream_params[dream].dream_reg[reg].reg )) != D_RetCode_Sucsess )
					{
						fprintf( stderr,  "%s: DreamWrite failed for feu=%d beu=%d lnk=%d dream=%d reg=%d with %d\n",
							__FUNCTION__, feu_id, beu_id, beu_lnk_id, D_FeuPar_NumOfDreams-1, reg, ret );
						return ret;
					}
					// Read
					if( (ret=DreamRead( dream, reg, rd_val )) != D_RetCode_Sucsess )
					{
						fprintf( stderr,  "%s: DreamRead failed for feu=%d beu=%d lnk=%d dream=%d reg=%d with %d\n",
							__FUNCTION__, feu_id, beu_id, beu_lnk_id, D_FeuPar_NumOfDreams-1, reg, ret );
						return ret;
					}
					// Compare
					// all types of registers
					if( dream_params[dream].dream_reg[reg].reg[0] != rd_val[0] )
					{
						fprintf( stderr, "%s: DreamRead failed for feu=%d beu=%d lnk=%d dream=%d reg=%d wr0=0x%x != rd0=0x%x\n",
							__FUNCTION__, feu_id, beu_id, beu_lnk_id, dream, reg, dream_params[dream].dream_reg[reg].reg[0], rd_val[0] );
						return D_RetCode_Err_WrRd_Missmatch;
					}
					// > 16-bit registers
					if( (reg!=12) && (dream_params[dream].dream_reg[reg].reg[1]) != rd_val[1] )
					{
						fprintf( stderr, "%s: DreamRead failed for feu=%d beu=%d lnk=%d dream=%d reg=%d wr1=0x%x != rd1=0x%x\n",
							__FUNCTION__, feu_id, beu_id, beu_lnk_id, dream, reg, dream_params[dream].dream_reg[reg].reg[1], rd_val[1] );
						return D_RetCode_Err_WrRd_Missmatch;
					}
					// 64-bit registers
					if( (6<=reg) && (reg<=7) )
					{
						if( dream_params[dream].dream_reg[reg].reg[2] != rd_val[2] )
						{
							fprintf( stderr, "%s: DreamRead failed for feu=%d beu=%d lnk=%d dream=%d reg=%d wr1=0x%x != rd1=0x%x\n",
								__FUNCTION__, feu_id, beu_id, beu_lnk_id, dream, reg, dream_params[dream].dream_reg[reg].reg[2], rd_val[2] );
							return D_RetCode_Err_WrRd_Missmatch;
						}
						if( dream_params[dream].dream_reg[reg].reg[3] != rd_val[3] )
						{
							fprintf( stderr, "%s: DreamRead failed for feu=%d beu=%d lnk=%d dream=%d reg=%d wr1=0x%x != rd1=0x%x\n",
								__FUNCTION__, feu_id, beu_id, beu_lnk_id, dream, reg, dream_params[dream].dream_reg[reg].reg[2], rd_val[2] );
							return D_RetCode_Err_WrRd_Missmatch;
						}
					}
				} // if( dream_params[dream].dream_reg[reg].reg[0] != -1 )
			} // for( reg=1; reg<D_DreamPar_NumOfRegs; reg++ )
		} // if( (msk & (1 << dream)) == 0 )
	} // for( dream=0; dream<D_FeuPar_NumOfDreams-1; dream++ )

	return D_RetCode_Sucsess;
}

/*******************************************************************
 ******************       Dream SPI Config      ********************
 *******************************************************************/
 #define Def_DreamRegConfigRetry 5
int DreamSpiConfig( DreamParams dream_params[D_FeuPar_NumOfDreams], int feu_id, int beu_id, int beu_lnk_id, int msk )
{
	int ret;
	int dream;
	int reg;

	unsigned int reg_wr_val_31_00;
	unsigned int reg_wr_val_63_32;

	unsigned int reg_wr_adr_31_00;
	unsigned int reg_wr_adr_63_32;

	unsigned int reg_rd_val_31_00;
	unsigned int reg_rd_val_63_32;
	
	int retry;

	/*
	 * Dream configuration: dream specific parameters if any
	 */
	for( dream=0; dream<D_FeuPar_NumOfDreams-1; dream++ )
	{
		if( (msk & (1 << dream)) == 0 )
		{
			retry = 0;
			for( reg=1; reg<D_DreamPar_NumOfRegs; reg++ )
			{
				reg_wr_adr_31_00 = 0x700000 + (dream << (8+2)) + (reg<<(2+1));
				reg_wr_adr_63_32 = reg_wr_adr_31_00 + 4;
				if( dream_params[dream].dream_reg[reg].reg[0] != -1 )
				{
					if( reg == 12 ) // 16-bit register
					{
						reg_wr_val_63_32 = 0;
						reg_wr_val_31_00 = dream_params[dream].dream_reg[reg].reg[0];
					}
					else if( (reg == 6) || (reg == 7) ) // 64-bit registers
					{
						reg_wr_val_63_32 = (dream_params[dream].dream_reg[reg].reg[0] << 16) | dream_params[dream].dream_reg[reg].reg[1];
						reg_wr_val_31_00 = (dream_params[dream].dream_reg[reg].reg[2] << 16) | dream_params[dream].dream_reg[reg].reg[3];
					}
					else // 32-bit registers
					{
						reg_wr_val_63_32 = 0;
						reg_wr_val_31_00 = (dream_params[dream].dream_reg[reg].reg[0] << 16) | dream_params[dream].dream_reg[reg].reg[1];
					}

					// Write LSB-s
					if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_wr_adr_31_00, reg_wr_val_31_00, DEF_FEU_WRITE, &reg_rd_val_31_00 ) ) < 0 )
					{
						fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x\n",
							__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_wr_adr_31_00 );
						return D_RetCode_Err_NetIO;
					}
					// Write MSB-s
					if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_wr_adr_63_32, reg_wr_val_63_32, DEF_FEU_WRITE, &reg_rd_val_63_32 ) ) < 0 )
					{
						fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x\n",
							__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_wr_adr_63_32 );
						return D_RetCode_Err_NetIO;
					}
					// Read MSB-s
					if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_wr_adr_63_32, 0, DEF_FEU_READ, &reg_rd_val_63_32 ) ) < 0 )
					{
						fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x\n",
							__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_wr_adr_63_32 );
						return D_RetCode_Err_NetIO;
					}
					// Read LSB-s
					if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_wr_adr_31_00, 0, DEF_FEU_READ, &reg_rd_val_31_00 ) ) < 0 )
					{
						fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x\n",
							__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_wr_adr_31_00 );
						return D_RetCode_Err_NetIO;
					}

//fprintf( stdout, "%s: feu=%d beu=%d lnk=%d dream=%d reg=%d WR 63_32=0x%08x 31_00=0x%08x RD 63_32=0x%08x 31_00=0x%08x\n",
//__FUNCTION__, feu_id, beu_id, beu_lnk_id, dream, reg, reg_wr_val_63_32, reg_wr_val_31_00, reg_rd_val_63_32, reg_rd_val_31_00 );

					// Compare
					if( reg == 12 ) // 16-bit register
					{
						if( reg_wr_val_31_00 != ((reg_rd_val_63_32 >> 16) & 0xFFFF)  )
						{
							if( retry == Def_DreamRegConfigRetry )
							{
								fprintf( stderr, "%s: WrRd_Missmatch failed for feu=%d beu=%d lnk=%d dream=%d reg=%d wr0=0x%x != rd0=0x%x\n",
									__FUNCTION__, feu_id, beu_id, beu_lnk_id, dream, reg, reg_wr_val_31_00, (reg_rd_val_63_32 >> 16) & 0xFFFF );
								return D_RetCode_Err_WrRd_Missmatch;
							}
							retry++;
						}
						else
							retry = 0;
					}
					else if( (reg == 6) || (reg == 7) ) // 64-bit registers
					{
						if( reg_wr_val_63_32 != reg_rd_val_63_32 )
						{
							if( retry == Def_DreamRegConfigRetry )
							{
								fprintf( stderr, "%s: WrRd_Missmatch failed for feu=%d beu=%d lnk=%d dream=%d reg=%d MSB wr0=0x%x != rd0=0x%x\n",
									__FUNCTION__, feu_id, beu_id, beu_lnk_id, dream, reg, reg_wr_val_63_32, reg_rd_val_63_32 );
								return D_RetCode_Err_WrRd_Missmatch;
							}
							retry++;
						}
						else if( reg_wr_val_31_00 != reg_rd_val_31_00 )
						{
							if( retry == Def_DreamRegConfigRetry )
							{
								fprintf( stderr, "%s: WrRd_Missmatch failed for feu=%d beu=%d lnk=%d dream=%d reg=%d LSB wr0=0x%x != rd0=0x%x\n",
									__FUNCTION__, feu_id, beu_id, beu_lnk_id, dream, reg, reg_wr_val_31_00, reg_rd_val_31_00 );
								return D_RetCode_Err_WrRd_Missmatch;
							}
							retry++;
						}
						else
							retry = 0;
					}
					else // 32-bit registers
					{
						if( reg_wr_val_31_00 != reg_rd_val_63_32 )
						{
							if( retry == Def_DreamRegConfigRetry )
							{
								fprintf( stderr, "%s: WrRd_Missmatch failed for feu=%d beu=%d lnk=%d dream=%d reg=%d wr0=0x%x != rd0=0x%x\n",
									__FUNCTION__, feu_id, beu_id, beu_lnk_id, dream, reg, reg_wr_val_31_00, reg_rd_val_63_32 );
								return D_RetCode_Err_WrRd_Missmatch;
							}
							retry++;
						}
						else
							retry = 0;
					}
					if( retry )
					{
//fprintf( stderr, "%s: Retry %d for feu=%d beu=%d lnk=%d dream=%d reg=%d WR 63_32=0x%08x 31_00=0x%08x RD 63_32=0x%08x 31_00=0x%08x\n",
//__FUNCTION__, retry, feu_id, beu_id, beu_lnk_id, dream, reg, reg_wr_val_63_32, reg_wr_val_31_00, reg_rd_val_63_32, reg_rd_val_31_00 );
						reg--;
					}
				} // if( dream_params[dream].dream_reg[reg].reg[0] != -1 )
			} // for( reg=1; reg<D_DreamPar_NumOfRegs; reg++ )
		} // if( (msk & (1 << dream)) == 0 )
	} // for( dream=0; dream<D_FeuPar_NumOfDreams-1; dream++ )

	return D_RetCode_Sucsess;
}

/********************
 ****  FEU Lock  ****
 *******************/
#define DEF_SEM_TIME_OUT_SEC 5
#define DEF_TimeValInit() {.tv_sec=DEF_SEM_TIME_OUT_SEC, .tv_usec=0}
 int FeuConfig_Lock()
 {
 	int index;
	struct timeval t0;
	struct timeval t1;
	struct timeval dt;
	struct timeval to = DEF_TimeValInit();
	int ret;

	// Initialize sem init done table when called for the first time
	if( sem_init_initialized == 0 )
	{
		sem_init_initialized = 1;
		for( index=0; index<DEF_MAX_NB_OF_FEU; index++ )
		{
			sem_init_done[index] = 0;
		}
	}

	// Initialize semaphore of the FEU when called for the first time
	if( sem_init_done[PeekPoke_Feu] == 0 )
	{
		// Get exclusive access to FEU
		if( (ret=Sem_Init(D_Feu_AdrReg_Set( D_CBus_SetModType( 0, D_CBus_Mod_Feu ), C_Feu_AdrReg_SlowCtrl ))) ==  D_RetCode_Wrn_SemBusy )
		{
			fprintf( stderr,  "%s: Sem_Init warning Semaphore Busy for feu=%d beu=%d lnk=%d\n",
				__FUNCTION__, PeekPoke_Feu, PeekPoke_Beu, PeekPoke_Lnk );
		}
		sem_init_done[PeekPoke_Feu] = 1;
	}

	// Get the semaphore
	gettimeofday(&t0,0);
	while(1)
	{
		ret = Sem_Acquire();
		if( ret == D_RetCode_Sucsess )
			break;
		else if( ret == D_RetCode_Err_Sem )
		{
			fprintf( stderr,  "%s: Sem_Acquire failed with %s\n", __FUNCTION__, sem_msg);
			return D_RetCode_Err_Sem;
		}
		gettimeofday(&t1, 0);
		timersub(&t1,&t0,&dt);
		if( timercmp(&dt,&to,>) )
		{
			fprintf( stderr, "%s: Sem_Acquire timeout\n\r", __FUNCTION__);
printf("%s: Sem_Acquire timeout: t0 sec=%8d usec=%6d\n\r", __FUNCTION__, t0.tv_sec, t0.tv_usec);
printf("%s: Sem_Acquire timeout: t1 sec=%8d usec=%6d\n\r", __FUNCTION__, t1.tv_sec, t1.tv_usec);
printf("%s: Sem_Acquire timeout: dt sec=%8d usec=%6d\n\r", __FUNCTION__, dt.tv_sec, dt.tv_usec);
printf("%s: Sem_Acquire timeout: to sec=%8d usec=%6d\n\r", __FUNCTION__, to.tv_sec, to.tv_usec);
			return D_RetCode_Err_Timeout;
		}
	};
	return D_RetCode_Sucsess;
}

 int FeuConfig_UnLock()
 {
 	int ret;
	// Free the semaphore for others
	if( (ret = Sem_Free()) != D_RetCode_Sucsess )
	{
		fprintf(stderr, "%s: Sem_Free failed with %s\n", __FUNCTION__, sem_msg);
		return ret;
	}
	return D_RetCode_Sucsess;
}
/*******************************************************************
 ************       FEU Memory configuration      ******************
 *******************************************************************/
// Global variable for feu memory configuration files
static FILE *feu_cfg_mem_fptr = (FILE *)NULL;

// Cleanup function
void FeuConfig_CleanUp()
{
	// At least try to free semaphore
	Sem_Free();
	// Close file pointer if any open
	if(feu_cfg_mem_fptr != (FILE *)NULL)
	{
		fclose(feu_cfg_mem_fptr);
		feu_cfg_mem_fptr = (FILE *)NULL;
	}
}

// Pedestal and threshold memory configuration
int FeuMemConfig( char *conf_file_name, int mem_type, int feu_id, int beu_id, int beu_lnk_id )
{
	char line[LINE_SIZE];
	int line_num;
	char *end_ptr;

	unsigned int reg_adr;
	unsigned int wr_val;
	unsigned int rd_val;
	int ret;
//	unsigned int wr_val_swap;

	int buf_index;
	int mem_adr;
	int entry;

	// Check parameters
	if( conf_file_name==NULL )
	{
		fprintf( stderr, "%s: conf_file_name=Null", __FUNCTION__ );
		return D_RetCode_Err_Wrong_Param;
	}
	// Open configuration file
	if( (feu_cfg_mem_fptr=fopen(conf_file_name, "r")) == NULL )
	{
		fprintf( stderr, "%s: fopen failed for config file %s type=%d in read mode\n", __FUNCTION__, conf_file_name, mem_type );
		fprintf( stderr, "%s: fopen failed with %s\n", __FUNCTION__, strerror(errno) );
		return D_RetCode_Err_FileIO;
	}

	// Check parameters a initialize variables
	if( mem_type == D_CBus_Mod_Pedestal )
	{
		reg_adr = D_CBus_SetModType( 0, D_CBus_Mod_Pedestal );
		reg_adr = D_PedMem_AdrZone_Set( reg_adr, D_PedMem_AdrZone_Mem );
	}
	else if( mem_type == D_CBus_Mod_Threshold )
	{
		reg_adr = D_CBus_SetModType( 0, D_CBus_Mod_Threshold );
		reg_adr = D_ThreshMem_AdrZone_Set( reg_adr, D_ThreshMem_AdrZone_Mem );
	}
	else
	{
		fprintf( stderr, "%s: memory type=%d is neither D_CBus_Mod_Pedestal=%d nor D_CBus_Mod_Threshold=%d",
			__FUNCTION__, mem_type, D_CBus_Mod_Pedestal, D_CBus_Mod_Threshold );
		return D_RetCode_Err_Wrong_Param;
	}

	// Process memory configuration file
	line_num = 0;
	entry = 0;
	while( fgets( line, LINE_SIZE, feu_cfg_mem_fptr ) != NULL )
	{
		line_num++;
		/* parse the line */
		parse_line(line);
		if( argc > 0 )
		{
			// determine bufdfer index and address within the buffer
			buf_index = entry / 64;
			mem_adr = entry % 64;
			entry++;

			// Check if this is a new buffer
			if( mem_adr == 0 )
			{
				// Set address address
				if( mem_type == D_CBus_Mod_Pedestal )
					reg_adr = D_PedMem_BramId_Set( reg_adr, buf_index );
				else
					reg_adr = D_ThreshMem_BramId_Set( reg_adr, buf_index );
			}
			if( mem_type == D_CBus_Mod_Pedestal )
				reg_adr = D_PedMem_BramAdr_Set(reg_adr, (mem_adr<<2));
			else 
				reg_adr = D_ThreshMem_BramAdr_Set(reg_adr, (mem_adr<<2));
			wr_val = strtol( argv[0], &end_ptr, 16 );
//			wr_val_swap = strtol( argv[0], &end_ptr, 16 );
//			wr_val = ((wr_val_swap & 0xFFFF) << 16) + ((wr_val_swap >> 16)&0xFFFF);
//printf("%s: line_num=%d line=%s reg_adr=0x%08x wr_val=0x%08x\n", __FUNCTION__, line_num, line, reg_adr, wr_val);
//getchar();
			// Write
			if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_WRITE, &rd_val ) ) < 0 )
			{
				fprintf( stderr, "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x\n",
					__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr );
				return D_RetCode_Err_NetIO;
			}
			else
			{
				if( wr_val != rd_val )
				{
					fprintf( stderr, "%s: Main config WrRd_Missmatch for feu=%d beu=%d lnk=%d reg=0x%06x wr_val=0x%08x rd_val=0x%08x\n",
						__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, rd_val );
					return D_RetCode_Err_WrRd_Missmatch;
				}
			}
		}
	} // while( 1 ); // Keep reading until end of file

	return D_RetCode_Sucsess;
}

// Pedestal and threshold memory intialisation
int FeuMemInit( int mem_type, int feu_id, int beu_id, int beu_lnk_id )
{
	unsigned int reg_adr;
	unsigned int wr_val;
	unsigned int rd_val;
	int ret;

	int buf_index;
	int max_mem_adr;
	int mem_adr;

	// Check parameters a initialize variables
	if( mem_type == D_CBus_Mod_Pedestal )
	{
		reg_adr = D_CBus_SetModType( 0, D_CBus_Mod_Pedestal );
		reg_adr = D_PedMem_AdrZone_Set( reg_adr, D_PedMem_AdrZone_Mem );
//		max_mem_adr = 1024;
		max_mem_adr = 64;
	}
	else if( mem_type == D_CBus_Mod_Threshold )
	{
		reg_adr = D_CBus_SetModType( 0, D_CBus_Mod_Threshold );
		reg_adr = D_ThreshMem_AdrZone_Set( reg_adr, D_ThreshMem_AdrZone_Mem );
		max_mem_adr = 64;
	}
	else
	{
		fprintf( stderr, "%s: memory type=%d is neither D_CBus_Mod_Pedestal=%d nor D_CBus_Mod_Threshold=%d",
			__FUNCTION__, mem_type, D_CBus_Mod_Pedestal, D_CBus_Mod_Threshold );
		return D_RetCode_Err_Wrong_Param;
	}

	for( buf_index=0; buf_index<4; buf_index++ )
	{
		if( mem_type == D_CBus_Mod_Pedestal )
			reg_adr = D_PedMem_BramId_Set(  reg_adr, buf_index );
		else
			reg_adr = D_ThreshMem_BramId_Set(  reg_adr, buf_index );
		for( mem_adr=0; mem_adr<max_mem_adr; mem_adr++ )
		{
			if( mem_type == D_CBus_Mod_Pedestal ) 
				reg_adr = D_PedMem_BramAdr_Set(reg_adr, (mem_adr<<2));
			else
				reg_adr = D_ThreshMem_BramAdr_Set(reg_adr, (mem_adr<<2));
//printf("%s: reg_adr=0x%08x\n", __FUNCTION__, reg_adr);
			// Write
			wr_val = 0;
			if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_WRITE, &rd_val ) ) < 0 )
			{
				fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x\n",
					__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr );
				return D_RetCode_Err_NetIO;
			}
			else
			{
				if( rd_val != wr_val )
				{
					fprintf( stderr,  "%s: Main config WrRd_Missmatch for feu=%d beu=%d lnk=%d reg=0x%06x wr_val=0x%08x rd_val=0x%08x\n",
						__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, rd_val );
					return D_RetCode_Err_WrRd_Missmatch;
				}
			}
		} // for( mem_adr=0; mem_adr<max_mem_adr; mem_adr++ )
	} // for( buf_index=0; buf_index<4; buf_index++ )

	return D_RetCode_Sucsess;
}

/************************************************************
 ************      Dream Clock Config      ******************
 ************************************************************/
// The trigger clock frequency is 125 MHz: 8 ns
#define Def_TrgClk_Period_ns 8
#define Drm_RdClk_Div_Min 3.5
#define Drm_WrClk_Div_Min 3

int DrmClkConfig( DrmClkParams *params, int feu_id, int beu_id, int beu_lnk_id )
{
	u32 reg_adr;
	u32 wr_val;
	u32 rd_val;

	int reg;
	char reg_name[64];
	unsigned int half_period;
	unsigned int phase_delay;
	int ret;

	// Check parameters
	if( params == (DrmClkParams *)NULL )
	{
		fprintf( stderr, "%s: DrmClkParams=NULL\n", __FUNCTION__ ); 
		return D_RetCode_Err_Null_Pointer;
	}
	// This function is called from main configuration function
	// The parameters have been already tested

	// Check if the parameters have been set
	if( (params->RdClk_Div < 0) && (params->WrClk_Div < 0) && (params->WrClk_Phase < 0) && (params->AdcClk_Phase < 0) )
	{
		fprintf( stderr, "%s: Nothing to do\n", __FUNCTION__ );
//getchar();
		return D_RetCode_Wrn_NothingToDo;
	}

	// Check if the parameters are correct
	if( (params->RdClk_Div >= 0) && (params->RdClk_Div < Drm_RdClk_Div_Min) )
	{
		fprintf( stderr, "%s: RdClk_Div=%3.1f less than %3.1f\n", __FUNCTION__, params->RdClk_Div, Drm_RdClk_Div_Min ); 
		return D_RetCode_Err_Wrong_Param;
	}
	if( (params->WrClk_Div >= 0) && (params->WrClk_Div < Drm_WrClk_Div_Min) )
	{
		fprintf( stderr, "%s: WrClk_Div=%3.1f less than %3.1f\n", __FUNCTION__, params->WrClk_Div, Drm_WrClk_Div_Min ); 
		return D_RetCode_Err_Wrong_Param;
	}
	if( (params->WrClk_Phase >= 0) && (params->WrClk_Phase>63) )
	{
		fprintf( stderr, "%s: WrClk_Phase=%d not in range [0;63]\n", __FUNCTION__, params->WrClk_Phase ); 
		return D_RetCode_Err_Wrong_Param;
	}
	if( (params->AdcClk_Phase >= 0) && (params->AdcClk_Phase>63) )
	{
		fprintf( stderr, "%s: AdcClk_Phase=%d not in range [0;63]\n", __FUNCTION__, params->AdcClk_Phase ); 
		return D_RetCode_Err_Wrong_Param;
	}

	/*
	 * First reset the MMCM
	 */
/*
	// Send DrmClkRst command
	// Set address 
	reg_adr = D_CBus_SetModType( 0, D_CBus_Mod_Main );
	reg_adr = D_MainAdr_Set( reg_adr, D_MainAdr_RegCommand );
	// Read
	if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, 0, DEF_FEU_READ, &rd_val ) ) < 0 )
	{
		fprintf( stderr,  "%s: Beu_ReqResp DEF_FEU_READ failed for feu=%d beu=%d lnk=%d DrmClkRst reg=0x%06x\n",
			__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr );
		return D_RetCode_Err_NetIO;
	}
	// Set the DrmClkRst bit
	wr_val = D_Main_Cmd_DrmClkRst_Set( rd_val );
//fprintf( stderr, "%s: DrmClkRst adr=0x%08x wr_val=0x%04x\n", __FUNCTION__, reg_adr, wr_val );
	// Write
	if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_WRITE, &rd_val ) ) < 0 )
	{
		fprintf( stderr,  "%s: Beu_ReqResp DEF_FEU_WRITE failed for feu=%d beu=%d lnk=%d DrmClkRst reg=0x%06x\n",
			__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr );
		return D_RetCode_Err_NetIO;
	}
	else
	{
		if( D_Main_Cmd_Config_Get(wr_val) != D_Main_Cmd_Config_Get(rd_val) )
		{
			fprintf( stderr,  "%s: DrmClkRst Set WrRd_Missmatch for feu=%d beu=%d lnk=%d DrmClkRst reg=0x%06x wr_val=0x%08x rd_val=0x%08x\n",
				__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr,  wr_val, rd_val );
			return D_RetCode_Err_WrRd_Missmatch;
		}
	}
//fprintf( stderr, "%s: After MMCM reset\n", __FUNCTION__ );
*/
	/*
	 * Next set MMCM power bits
	 */
	reg_adr = D_CBus_SetModType( 0, D_CBus_Mod_DrmClk );
	reg_adr = D_TrgClk_AdrReg_Set( reg_adr, C_TrgClk_PwrBits );
	// Set all Power bits
	wr_val = D_TrgClk_RegPower_Set( 0 );
//fprintf( stderr, "%s: PwrBits adr=0x%08x wr_val=0x%04x\n", __FUNCTION__, reg_adr, wr_val );
	// Write
	if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_WRITE, &rd_val ) ) < 0 )
	{
		fprintf( stderr,  "%s: Beu_ReqResp DEF_FEU_WRITE failed for feu=%d beu=%d lnk=%d PwrBits reg=0x%06x\n",
			__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr );
		return D_RetCode_Err_NetIO;
	}
	else
	{
		if( D_TrgClk_RegPower_Get(wr_val) != D_TrgClk_RegPower_Get(rd_val) )
		{
			fprintf( stderr,  "%s: PwrBits Set WrRd_Missmatch for feu=%d beu=%d lnk=%d PwrBits reg=0x%06x wr_val=0x%08x rd_val=0x%08x\n",
				__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr,  wr_val, rd_val );
			return D_RetCode_Err_WrRd_Missmatch;
		}
	}
//fprintf( stderr, "%s: After MMCP Power Bits\n", __FUNCTION__ );

	/*
	 * Next set MMCM Register: RdClock_Div
	 */
	for( reg=0; reg<5; reg++ )
	{
		// Perform operations only for set registers
		// skip others
		if
		(
			( (reg == 0) && (params->RdClk_Div >= 0) )
			||
			( (reg == 1) && (params->RdClk_Div >= 0) )
			||
			( (reg == 2) && (params->WrClk_Div >= 0) )
			||
			( (reg == 3) && (params->WrClk_Phase >= 0) )
			||
			( (reg == 4) && (params->AdcClk_Phase >= 0) )
		)
		{
			// Set address
			if( reg == 0 )
			{
				reg_adr = D_TrgClk_AdrReg_Set( reg_adr, C_TrgClk_RdClk_Freq );
				sprintf( reg_name, "%s%", "RdClk_Freq" );
			}
			if( reg == 1 )
			{
				reg_adr = D_TrgClk_AdrReg_Set( reg_adr, C_TrgClk_AdcClk_Freq );
				sprintf( reg_name, "%s%", "AdcClk_Freq" );
			}
			else if( reg == 2 )
			{
				reg_adr = D_TrgClk_AdrReg_Set( reg_adr, C_TrgClk_WrClk_Freq );
				sprintf( reg_name, "%s%", "WrClk_Freq" );
			}
			else if( reg == 3 )
			{
				reg_adr = D_TrgClk_AdrReg_Set( reg_adr, C_TrgClk_WrClk_Phase );
				sprintf( reg_name, "%s%", "WrClk_Phase" );
			}
			else if( reg == 4 )
			{
				reg_adr = D_TrgClk_AdrReg_Set( reg_adr, C_TrgClk_AdcClk_Phase );
				sprintf( reg_name, "%s%", "AdcClk_Phase" );
			}
//fprintf( stderr, "%s: reg=%d %s adr=0x%08x\n", __FUNCTION__, reg, reg_name, reg_adr );
			// Read
			if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, 0, DEF_FEU_READ, &rd_val ) ) < 0 )
			{
				fprintf( stderr,  "%s: Beu_ReqResp DEF_FEU_READ failed for feu=%d beu=%d lnk=%d %s reg=0x%06x\n",
					__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_name, reg_adr );
				return D_RetCode_Err_NetIO;
			}
//fprintf( stderr, "%s: reg=%d %s adr=0x%08x rd_val=0x%04x\n", __FUNCTION__, reg, reg_name, reg_adr, rd_val );
			// Set values to be written
			if( reg == 0 )
			{
				half_period = (unsigned int)(Def_TrgClk_Period_ns * params->RdClk_Div / 2.);
				wr_val = D_TrgClk_RegFreq_LowTime_Set(  rd_val, half_period );
				wr_val = D_TrgClk_RegFreq_HighTime_Set( wr_val, half_period );
			}
			else if( reg == 1 )
			{
				half_period = (unsigned int)(Def_TrgClk_Period_ns * params->RdClk_Div / 2.);
				wr_val = D_TrgClk_RegFreq_LowTime_Set(  rd_val, half_period );
				wr_val = D_TrgClk_RegFreq_HighTime_Set( wr_val, half_period );
			}
			else if( reg == 2 )
			{
				half_period = (unsigned int)(Def_TrgClk_Period_ns * params->WrClk_Div / 2.);
				wr_val = D_TrgClk_RegFreq_LowTime_Set(  rd_val, half_period );
				wr_val = D_TrgClk_RegFreq_HighTime_Set( wr_val, half_period );
			}
			else if( reg == 3 )
			{
/*
				phase_delay = (params->WrClk_Phase * 64) / 360;
				wr_val = D_TrgClk_RegPhase_Delay_Set( rd_val, phase_delay );
*/
				wr_val = D_TrgClk_RegPhase_Delay_Set( rd_val, params->WrClk_Phase );
				wr_val = D_TrgClk_RegPhase_Mx_Clr( wr_val );
			}
			else if( reg == 4 )
			{
/*
				phase_delay = (params->AdcClk_Phase * 64) / 360;
//fprintf( stderr, "%s: reg=%d %s AdcClk_Phase=%d phase_delay=%d\n", __FUNCTION__, reg, reg_name, params->AdcClk_Phase, phase_delay );
				wr_val = D_TrgClk_RegPhase_Delay_Set( rd_val, phase_delay );
*/
				wr_val = D_TrgClk_RegPhase_Delay_Set( rd_val, params->AdcClk_Phase );
				wr_val = D_TrgClk_RegPhase_Mx_Clr( wr_val );
			}
			// Write
//fprintf( stderr, "%s: reg=%d %s adr=0x%08x wr_val=0x%04x\n", __FUNCTION__, reg, reg_name, reg_adr, wr_val );
			if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_WRITE, &rd_val ) ) < 0 )
			{
				fprintf( stderr,  "%s: Beu_ReqResp DEF_FEU_WRITE failed for feu=%d beu=%d lnk=%d %s reg=0x%06x\n",
					__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_name, reg_adr );
				return D_RetCode_Err_NetIO;
			}
			else
			{
				if( (reg == 0) || (reg == 1) || (reg == 2) )
				{
					if( D_TrgClk_RegFreq_LowTime_Get(wr_val) != D_TrgClk_RegFreq_LowTime_Get(rd_val) )
					{
						fprintf( stderr,  "%s: DrmClk reg %s LowTime Set WrRd_Missmatch for feu=%d beu=%d lnk=%d %s reg=0x%06x wr_val=0x%08x rd_val=0x%08x\n",
							__FUNCTION__, reg_name, feu_id, beu_id, beu_lnk_id, reg_name, reg_adr, wr_val, rd_val );
						return D_RetCode_Err_WrRd_Missmatch;
					}
					if( D_TrgClk_RegFreq_HighTime_Get(wr_val) != D_TrgClk_RegFreq_HighTime_Get(rd_val) )
					{
						fprintf( stderr,  "%s: DrmClk reg %s HighTime Set WrRd_Missmatch for feu=%d beu=%d lnk=%d %s reg=0x%06x wr_val=0x%08x rd_val=0x%08x\n",
							__FUNCTION__, reg_name, feu_id, beu_id, beu_lnk_id, reg_name, reg_adr, wr_val, rd_val );
						return D_RetCode_Err_WrRd_Missmatch;
					}
				}
				else if( (reg == 3) || (reg == 4) )
				{
					if( D_TrgClk_RegPhase_Delay_Get(wr_val) != D_TrgClk_RegPhase_Delay_Get(rd_val) )
					{
						fprintf( stderr,  "%s: DrmClk reg %s Delay Set WrRd_Missmatch for feu=%d beu=%d lnk=%d %s reg=0x%06x wr_val=0x%08x rd_val=0x%08x\n",
							__FUNCTION__, reg_name, feu_id, beu_id, beu_lnk_id, reg_name, reg_adr, wr_val, rd_val );
						return D_RetCode_Err_WrRd_Missmatch;
					}
				}
			}
		} // if to Perform operations only for set registers
	} // for( reg=0; reg<5; reg++ )

	/*
	 * Next clear MMCM power bits
	 */
/*
	reg_adr = D_CBus_SetModType( 0, D_CBus_Mod_DrmClk );
	reg_adr = D_MainAdr_Set( reg_adr, C_TrgClk_PwrBits );
	// Set all Power bits
	wr_val = D_TrgClk_RegPower_Clr( 0 );
	// Write
	sprintf(sc_tx_buf, "poke 0x%x 0x%x", reg_adr, wr_val);
	sc_tx_len = strlen(sc_tx_buf)+1;
	if( (sc_rx_len = UdpSocket_ReqResp(&sc_socket, feu, feu_ip, feu_port, sc_tx_buf, sc_tx_len, sc_rx_buf, 256 ) ) < 0 )
	{
		fprintf( stderr,  "%s: UdpSocket_ReqResp failed for feu_id=%d port=%d for PwrBits Poke\n", __FUNCTION__, feu, feu_port );
		return D_RetCode_Err_NetIO;
	}
	else
	{
		sscanf(sc_rx_buf, "poke 0x%x 0x%x = 0x%x", &reg_adr, &wr_val, &rd_val );
		if( D_TrgClk_RegPower_Get(wr_val) != D_TrgClk_RegPower_Get(rd_val) )
		{
			fprintf( stderr,  "%s: PwrBits Clear WrRd_Missmatch for %s\n", __FUNCTION__, sc_rx_buf );
			return D_RetCode_Err_WrRd_Missmatch;
		}
	}
*/
	/*
	 * Last remove Reset from the MMCM
	 */
/*
	// Send DrmClkRst command
	// Set address 
	reg_adr = D_CBus_SetModType( 0, D_CBus_Mod_Main );
	reg_adr = D_MainAdr_Set( reg_adr, D_MainAdr_RegCommand );
	// Read
	if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, 0, DEF_FEU_READ, &rd_val ) ) < 0 )
	{
		fprintf( stderr,  "%s: Beu_ReqResp DEF_FEU_READ failed for feu=%d beu=%d lnk=%d DrmClkRst reg=0x%06x\n",
			__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr );
		return D_RetCode_Err_NetIO;
	}
	// Set the DrmClkRst bit
	wr_val = D_Main_Cmd_DrmClkRst_Clr( rd_val );
	// Write
	// Write
	if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_WRITE, &rd_val ) ) < 0 )
	{
		fprintf( stderr,  "%s: Beu_ReqResp DEF_FEU_WRITE failed for feu=%d beu=%d lnk=%d DrmClkRst reg=0x%06x\n",
			__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr );
		return D_RetCode_Err_NetIO;
	}
	else
	{
		if( D_Main_Cmd_Config_Get(wr_val) != D_Main_Cmd_Config_Get(rd_val) )
		{
			fprintf( stderr,  "%s: DrmClkRst Clr WrRd_Missmatch for feu=%d beu=%d lnk=%d DrmClkRst reg=0x%06x wr_val=0x%08x rd_val=0x%08x\n",
				__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr,  wr_val, rd_val );
			return D_RetCode_Err_WrRd_Missmatch;
		}
	}
*/
//fprintf( stderr, "%s: Finished\n", __FUNCTION__ );
//getchar();

	// All went fine
	return D_RetCode_Sucsess;
}

/*
 * Get Feu Serial Number
 */
int _FeuGetSn( int feu_id, int beu_id, int beu_lnk_id, int *feu_sn )
{
	unsigned int reg_adr;
	unsigned int wr_val;
	unsigned int rd_val;

	int ret;

	/*
	 *  Try to get FEU ID
	 */
	// Those are RO registers, so no semaphore is needed
	// Set address 
	reg_adr = D_CBus_SetModType( 0, D_CBus_Mod_Main );
	// 4 LSB-s of SN are in Firmware revision register
	reg_adr = D_MainAdr_Set( reg_adr, D_MainAdr_RegFwRev );
	if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_READ, &rd_val ) ) < 0 )
	{
		fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d D_MainAdr_RegFwRev reg=0x%06x\n",
			__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr );
		return D_RetCode_Err_NetIO;
	}

	// Get the 4 LSB-s
	*feu_sn = D_Main_Rev_GetSrN(rd_val);
	// 4 MSB-s of SN are in Software revision register
	reg_adr = D_MainAdr_Set( reg_adr, D_MainAdr_RegSwRev );
	if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_READ, &rd_val ) ) < 0 )
	{
		fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d D_MainAdr_RegSwRev reg=0x%06x\n",
			__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr );
		return D_RetCode_Err_NetIO;
	}

	// Complete with 4 MSB-s
	*feu_sn = (D_Main_Rev_GetSrN(rd_val)<<4) + (*feu_sn);
	printf("%s: Feu %d SN %d (beu %d lnk %d)\n", __FUNCTION__, feu_id, *feu_sn, beu_id, beu_lnk_id);

	// All went fine
	return D_RetCode_Sucsess;
}

/*
 * Internal Reset function
 */
int _FeuReset( FeuParams *feu_params, int feu_id, int beu_id, int beu_lnk_id )
{
	unsigned int reg_adr;
	unsigned int wr_val;
	unsigned int rd_val;

	int ret;

	// Program the Dream clocks
	if( (ret = DrmClkConfig( &(feu_params->dream_clk_params), feu_id, beu_id, beu_lnk_id )) < 0 )
	{
		fprintf( stderr,  "%s: DrmClkConfig failed for feu=%d beu=%d lnk=%d with %d\n", __FUNCTION__, feu_id, beu_id, beu_lnk_id, ret );
		return ret;
	}

	// Reset FEU
	// Send reset command only once when the function is called first time with default parameters 
	// Set address 
	reg_adr = D_CBus_SetModType( 0, D_CBus_Mod_Main );
	reg_adr = D_MainAdr_Set( reg_adr, D_MainAdr_RegCommand );
	wr_val = D_Main_Cmd_Reset_Clr( 0 );
	// Write
	if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_WRITE, &rd_val ) ) < 0 )
	{
		fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x\n",
			__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr );
		return D_RetCode_Err_NetIO;
	}
	else
	{
		if( D_Main_Cmd_Reset_Get(wr_val) != D_Main_Cmd_Reset_Get(rd_val) )
		{
			fprintf( stderr,  "%s: Main Command (Reset) WrRd_Missmatch for feu=%d beu=%d lnk=%d reg=0x%06x wr_val=0x%08x rd_val=0x%08x\n",
				__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr,  wr_val, rd_val );
			return D_RetCode_Err_WrRd_Missmatch;
		}
	}
	// Set reset command
	wr_val = D_Main_Cmd_Reset_Set( wr_val );
	// Write
	if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_WRITE, &rd_val ) ) < 0 )
	{
		fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x\n",
			__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr );
		return D_RetCode_Err_NetIO;
	}
	else
	{
		if( D_Main_Cmd_Reset_Get(wr_val) != D_Main_Cmd_Reset_Get(rd_val) )
		{
			fprintf( stderr,  "%s: Main Command (Reset) WrRd_Missmatch for feu=%d beu=%d lnk=%d reg=0x%06x wr_val=0x%08x rd_val=0x%08x\n",
				__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr,  wr_val, rd_val );
			return D_RetCode_Err_WrRd_Missmatch;
		}
	}
	// Clear reset command
	wr_val = D_Main_Cmd_Reset_Clr( wr_val );
	// Write
	if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_WRITE, &rd_val ) ) < 0 )
	{
		fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x\n",
			__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr );
		return D_RetCode_Err_NetIO;
	}
	else
	{
		if( D_Main_Cmd_Reset_Get(wr_val) != D_Main_Cmd_Reset_Get(rd_val) )
		{
			fprintf( stderr,  "%s: Main Command (Reset) WrRd_Missmatch for feu=%d beu=%d lnk=%d reg=0x%06x wr_val=0x%08x rd_val=0x%08x\n",
				__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr,  wr_val, rd_val );
			return D_RetCode_Err_WrRd_Missmatch;
		}
	}

	return D_RetCode_Sucsess;
}

/*
 * Reset function
 */
int FeuReset( FeuParams *feu_params, int feu_id, int beu_id, int beu_lnk_id )
{
	int ret;

	int feu_sn;

	/*
	 * Check input parameters
	 */
	if( feu_params == (FeuParams *)NULL )
	{
		fprintf( stderr, "%s: feu_params=NULL\n", __FUNCTION__ ); 
		return D_RetCode_Err_Null_Pointer;
	}
	if( (feu_id < 1) || (DEF_MAX_NB_OF_FEU<=feu_id) )
	{
		fprintf( stderr, "%s: feu_index=%d<1 or >=%d\n", __FUNCTION__, feu_id, DEF_MAX_NB_OF_FEU ); 
		return D_RetCode_Err_Wrong_Param;
	}
	if( (beu_id < 1) || (DEF_MAX_NB_OF_BEU<=beu_id) )
	{
		fprintf( stderr, "%s: feu_id=%d<0 or >=%d\n", __FUNCTION__, beu_id, DEF_MAX_NB_OF_BEU ); 
		return D_RetCode_Err_Wrong_Param;
	}
	if( (beu_lnk_id < 0) || (DEF_MAX_NB_OF_FEU_PER_BEU<=beu_lnk_id) )
	{
		fprintf( stderr, "%s: feu_id=%d<0 or >=%d\n", __FUNCTION__, beu_lnk_id, DEF_MAX_NB_OF_FEU_PER_BEU ); 
		return D_RetCode_Err_Wrong_Param;
	}

	// First Fix the Peek/Poke global variables
	PeekPoke_Feu = feu_id;
	PeekPoke_Beu = beu_id;
	PeekPoke_Lnk = beu_lnk_id;

	// Make sure the Link is active
	if( (ret = Beu_CheckFeuLink(feu_id, beu_id, beu_lnk_id)) < 0 )
	{
		fprintf( stderr,  "%s: Beu_ChekFeuLink failed with %d for %d %d %d\n",
			__FUNCTION__, ret, feu_id, beu_id, beu_lnk_id );
		return ret;
	} 

	/*
	 *  Try to get FEU ID
	 */
	if( (ret = _FeuGetSn( feu_id, beu_id, beu_lnk_id, &feu_sn )) != D_RetCode_Sucsess )
	{
		fprintf( stderr,  "%s: _FeuGetSn failed for feu=%d beu=%d lnk=%d with %d\n",
			__FUNCTION__, feu_id, beu_id, beu_lnk_id, ret );
		return ret;
	}

	// Get exclussive access to the FEU
	if( (ret = FeuConfig_Lock()) != D_RetCode_Sucsess )
	{
		fprintf( stderr, "%s: Feu_Lock fail for feu=%d sn=%d beu=%d lnk=%d with ret %d\n", __FUNCTION__, feu_id, feu_sn, beu_id, beu_lnk_id, ret ); 
		return ret;
	}

	// Reset FEU
	if( (ret = _FeuReset( feu_params, feu_id, beu_id, beu_lnk_id )) != D_RetCode_Sucsess )
	{
		fprintf( stderr, "%s: _FeuReset fail for feu=%d sn=%d beu=%d lnk=%d with ret %d\n", __FUNCTION__, feu_id, feu_sn, beu_id, beu_lnk_id, ret ); 
		return ret;
	}

	// Note: After reset the FEU is unlocked by firmware: no need to unlock in software
	// But the software state of the semaphore needs to be changed
	if( (ret = Sem_ClearState()) != D_RetCode_Sucsess )
	{
		fprintf(stderr, "%s: Sem_ClearState failed with %s\n", __FUNCTION__, sem_msg);
		return ret;
	}

	return D_RetCode_Sucsess;
}

/*
 * Internal Configuration function
 */
int _FeuConfig( FeuParams *feu_params, int feu_id, int beu_id, int beu_lnk_id )
{
	unsigned int reg_adr;
	unsigned int wr_val;
	unsigned int rd_val;

	int ret;

	int dream_pair;
	int dream_power;

	int reg;
	int dream_min;
	int dream_max;
	int dream;

	// Main module config parameters
	if
	(
		( feu_params->Main_Conf_DreamMask >= 0 )
		||
		( feu_params->Main_Conf_Samples >= 0 )
	)
	{
		// Main module config parameters
		// Only recovered clock is supported, this is even nonsence to setup 
		wr_val = D_Main_Conf_Set( 0, feu_params->Main_Conf_Samples, (feu_params->Main_Conf_DreamMask&0xFF), 0, 0, RecClk );
		// Set address address 
		reg_adr = D_CBus_SetModType( 0, D_CBus_Mod_Main );
		reg_adr = D_MainAdr_Set( reg_adr, D_MainAdr_RegConfig );
		// Write
		if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_WRITE, &rd_val ) ) < 0 )
		{
			fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x\n",
				__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr );
			return D_RetCode_Err_NetIO;
		}
		else
		{
			if( wr_val != rd_val )
			{
				fprintf( stderr,  "%s: Main config WrRd_Missmatch for feu=%d beu=%d lnk=%d reg=0x%06x wr_val=0x%08x rd_val=0x%08x\n",
					__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr,  wr_val, rd_val );
				return D_RetCode_Err_WrRd_Missmatch;
			}
		}
//printf("%s: Feu %d Main configure done (beu %d lnk %d)\n", __FUNCTION__, feu_id, beu_id, beu_lnk_id);
	}

	// Main module trigger register
	if
	(
		( feu_params->Main_Trig_TimeStamp >= 0 )
		||
		( feu_params->Main_Trig_OvrWrnLwm >= 0 )
		||
		( feu_params->Main_Trig_OvrWrnHwm >= 0 )
		||
		( feu_params->Main_Trig_OvrThersh >= 0 )
		||
		( feu_params->Main_Trig_LocThrot >= 0 )
	)
	{
		// Main module trigger parameters
		wr_val = D_Main_Trig_Set( 0, feu_params->Main_Trig_TimeStamp, feu_params->Main_Trig_OvrWrnLwm, feu_params->Main_Trig_OvrWrnHwm, feu_params->Main_Trig_OvrThersh, feu_params->Main_Trig_LocThrot );
		// Set address address 
		reg_adr = D_CBus_SetModType( 0, D_CBus_Mod_Main );
		reg_adr = D_MainAdr_Set( reg_adr, D_MainAdr_RegTrigConfig );
		// Write
		if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_WRITE, &rd_val ) ) < 0 )
		{
			fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x\n",
				__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr );
			return D_RetCode_Err_NetIO;
		}
		else
		{
			if( wr_val != rd_val )
			{
				fprintf( stderr,  "%s: Main trigger WrRd_Missmatch for feu=%d beu=%d lnk=%d reg=0x%06x wr_val=0x%08x rd_val=0x%08x\n",
					__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr,  wr_val, rd_val );
				return D_RetCode_Err_WrRd_Missmatch;
			}
		}
//printf("%s: Feu %d Main trigger done (beu %d lnk %d)\n", __FUNCTION__, feu_id, beu_id, beu_lnk_id);
	}

	// FEU PowerUp Register
	if
	(
		( feu_params->Feu_Pwr_Dream >= 0 )
		||
		( feu_params->Feu_Pwr_PrtFlt >= 0 )
	)
	{
		// Set address address
		reg_adr = D_CBus_SetModType( 0, D_CBus_Mod_Feu );
		reg_adr = D_MainAdr_Set( reg_adr, C_Feu_AdrReg_PwrUp );
		wr_val = D_Feu_RegPwr_PrtFlt_Set( 0, feu_params->Feu_Pwr_PrtFlt );
		if( feu_params->Feu_Pwr_PrtFlt == 0 )
		{
			// Write
			if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_WRITE, &rd_val ) ) < 0 )
			{
				fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x\n",
					__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr );
				return D_RetCode_Err_NetIO;
			}
			else
			{
				if( D_Feu_RegPwr_Get(wr_val) != D_Feu_RegPwr_Get(rd_val) )
				{
					fprintf( stderr,  "%s: FEU PowerUp WrRd_Missmatch for feu=%d beu=%d lnk=%d reg=0x%06x wr_val=0x%08x rd_val=0x%08x\n",
						__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr,  wr_val, rd_val );
					return D_RetCode_Err_WrRd_Missmatch;
				}
			}
		}
		// Power dreams
		dream_power = 0;
		for( dream_pair=0; dream_pair<4; dream_pair++ )
		{
			if( (feu_params->Feu_Pwr_Dream) & (1 << dream_pair) )
			{
				dream_power += (1 << dream_pair);
				wr_val = D_Feu_RegPwr_Dream_Set( wr_val, dream_power );
				// Write
				if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_WRITE, &rd_val ) ) < 0 )
				{
					fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x\n",
						__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr );
					return D_RetCode_Err_NetIO;
				}
				else
				{
					if( D_Feu_RegPwr_Get(wr_val) != D_Feu_RegPwr_Get(rd_val) )
					{
						fprintf( stderr,  "%s: FEU PowerUp WrRd_Missmatch for feu=%d beu=%d lnk=%d reg=0x%06x wr_val=0x%08x rd_val=0x%08x\n",
							__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr,  wr_val, rd_val );
						return D_RetCode_Err_WrRd_Missmatch;
					}
				}
				usleep(50000);
			} // if( feu_params->Feu_Pwr_Dream & (1 << dream_pair) )
		} // for( dream_pair=0; dream_pair<4; dream_pair++ )
//printf("%s: Feu %d power 0x%08x done (beu %d lnk %d)\n", __FUNCTION__, feu_id, rd_val, beu_id, beu_lnk_id);
	}

	// FEU Run Control parameters
	if
	(
		( feu_params->Feu_RunCtrl_Pd >= 0 )
		||
		( feu_params->Feu_RunCtrl_CM >= 0 )
		||
		( feu_params->Feu_RunCtrl_ZS >= 0 )
		||
		( feu_params->Feu_RunCtrl_DrOvr >= 0 )
		||
		( feu_params->Feu_RunCtrl_DrDblSmpClk >= 0 )
		||
		( feu_params->Feu_RunCtrl_RdDel >= 0 )
		||
		( feu_params->Feu_RunCtrl_EvTstExt >= 0 )
		||
		( feu_params->Feu_RunCtrl_ZsChkSmp >= 0 )
		||
		( feu_params->Feu_RunCtrl_CmOffset >= 0 )
		||
		( feu_params->Feu_RunCtrl_AdcDatRdyDel >= 0 )
	)
	{
		// FEU Run Control parameters
		wr_val = D_Feu_RunCtrl_Set( 0, feu_params->Feu_RunCtrl_Pd, feu_params->Feu_RunCtrl_CM, feu_params->Feu_RunCtrl_ZS, feu_params->Feu_RunCtrl_ZsChkSmp, feu_id, feu_params->Feu_RunCtrl_AdcDatRdyDel, feu_params->Feu_RunCtrl_DrDblSmpClk, feu_params->Feu_RunCtrl_DrOvr, feu_params->Feu_RunCtrl_EvTstExt, feu_params->Feu_RunCtrl_RdDel, feu_params->Feu_RunCtrl_CmOffset );
		// Set address address 
		reg_adr = D_CBus_SetModType( 0, D_CBus_Mod_Feu );
		reg_adr = D_Feu_AdrReg_Set( reg_adr, C_Feu_AdrReg_RunCtrl );
		// Write
		if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_WRITE, &rd_val ) ) < 0 )
		{
			fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x\n",
				__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr );
			return D_RetCode_Err_NetIO;
		}
		else
		{
			if( wr_val != rd_val )
			{
				fprintf( stderr,  "%s: FEU Run Control WrRd_Missmatch for feu=%d beu=%d lnk=%d reg=0x%06x wr_val=0x%08x rd_val=0x%08x\n",
					__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr,  wr_val, rd_val );
				return D_RetCode_Err_WrRd_Missmatch;
			}
		}
//printf("%s: Feu %d run ctrl 0x%08x (beu %d lnk %d) extracted id=%d\n", __FUNCTION__, feu_id, rd_val, beu_id, beu_lnk_id, D_Feu_RegRC_Id_Get(wr_val));
	}

	// FEU Pedestal memory
	if( (feu_params->Feu_RunCtrl_PdFile[0]!='\0') && (strcmp(feu_params->Feu_RunCtrl_PdFile,"None")!= 0) )
	{
		if( (ret = FeuMemConfig(feu_params->Feu_RunCtrl_PdFile, D_CBus_Mod_Pedestal, feu_id, beu_id, beu_lnk_id )) < 0 )
		{
			fprintf( stderr,  "%s: FeuMemConfig failed for feu=%d beu=%d lnk=%d pedestal file=0xs with %d\n",
				__FUNCTION__, feu_id, beu_id, beu_lnk_id, feu_params->Feu_RunCtrl_PdFile, ret );
			return D_RetCode_Err_NetIO;
		}
//printf("%s: Feu %d Feu Ped Mem configured (beu %d lnk %d) with %s\n", __FUNCTION__, feu_id, beu_id, beu_lnk_id, feu_params->Feu_RunCtrl_PdFile);
	}
	else if( feu_id != 0 )
	{
		if( (ret = FeuMemInit(D_CBus_Mod_Pedestal, feu_id, beu_id, beu_lnk_id )) < 0 )
		{
			fprintf( stderr,  "%s: FeuMemInit failed for feu=%d beu=%d lnk=%d pedestals with ret=%d\n",
				__FUNCTION__, feu_id, beu_id, beu_lnk_id, ret );
			return D_RetCode_Err_NetIO;
		}
//printf("%s: Feu %d Feu Ped Mem initialized (beu %d lnk %d)\n", __FUNCTION__, feu_id, beu_id, beu_lnk_id);
	}
	// FEU Threshold memory
	if( (feu_params->Feu_RunCtrl_ZsFile[0]!='\0') && (strcmp(feu_params->Feu_RunCtrl_ZsFile,"None")!= 0) )
	{
		if( (ret = FeuMemConfig(feu_params->Feu_RunCtrl_ZsFile, D_CBus_Mod_Threshold, feu_id, beu_id, beu_lnk_id )) < 0 )
		{
			fprintf( stderr,  "%s: FeuMemConfig failed for feu=%d beu=%d lnk=%d thershold file=0xs with %d\n",
				__FUNCTION__, feu_id, beu_id, beu_lnk_id, feu_params->Feu_RunCtrl_ZsFile, ret );
			return D_RetCode_Err_NetIO;
		}
//printf("%s: Feu %d Feu Thresh Mem configured (beu %d lnk %d) with %s\n", __FUNCTION__, feu_id, beu_id, beu_lnk_id, feu_params->Feu_RunCtrl_ZsFile);
	}
	else if( feu_id != 0 )
	{
		if( (ret = FeuMemInit(D_CBus_Mod_Threshold, feu_id, beu_id, beu_lnk_id )) < 0 )
		{
			fprintf( stderr,  "%s: FeuMemInit failed for feu=%d beu=%d lnk=%d thresholds with ret=%d\n",
				__FUNCTION__, feu_id, beu_id, beu_lnk_id, ret );
			return D_RetCode_Err_NetIO;
		}
//printf("%s: Feu %d Feu Thresh Mem initialized (beu %d lnk %d)\n", __FUNCTION__, feu_id, beu_id, beu_lnk_id);
	}

	// FEU Pulser Register
	if
	(
		( feu_params->Feu_Pulser_DreamTst >= 0 )
		||
		( feu_params->Feu_Pulser_PulseWid >= 0 )
	)
	{
		// FEU Pulser parameters
		wr_val = feu_params->Feu_Pulser_DreamTst + (feu_params->Feu_Pulser_PulseWid << 8);
		// Enable Pulser
		if( feu_params->Feu_Pulser_Enable >= 0 )
			wr_val += (feu_params->Feu_Pulser_Enable << 24);
		// Set address 
		reg_adr = D_CBus_SetModType( 0, D_CBus_Mod_Feu );
		reg_adr = D_Feu_AdrReg_Set( reg_adr, C_Feu_AdrReg_Pulser );
		// Write
		if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_WRITE, &rd_val ) ) < 0 )
		{
			fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x\n",
				__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr );
			return D_RetCode_Err_NetIO;
		}
		else
		{
			if( wr_val != rd_val )
			{
				fprintf( stderr,  "%s: FEU Pulser WrRd_Missmatch for feu=%d beu=%d lnk=%d reg=0x%06x wr_val=0x%08x rd_val=0x%08x\n",
					__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr,  wr_val, rd_val );
				return D_RetCode_Err_WrRd_Missmatch;
			}
		}
//printf("%s: Feu %d pulser 0x%08x (beu %d lnk %d)\n", __FUNCTION__, feu_id, rd_val, beu_id, beu_lnk_id);
	}

	// FEU Prescale Register
	if
	(
		(feu_params->Feu_PreScale_EvtData > 0)
		||
		(feu_params->Feu_InterPacket_Delay > 0)
	)
	{
		// FEU Pulser parameters
		wr_val = D_Feu_RegPreScale_EvtData_Put( 0, feu_params->Feu_PreScale_EvtData );
		// Delay is given in ns; The Inter Packet delay is in ticks of 8 ns
		wr_val = D_Feu_RegPreScale_Ipd_Put( wr_val, (int)(((double)feu_params->Feu_InterPacket_Delay) / 8.) );
		// Set address address 
		reg_adr = D_CBus_SetModType( 0, D_CBus_Mod_Feu );
		reg_adr = D_Feu_AdrReg_Set( reg_adr, C_Feu_AdrReg_PreScale );
		// Write
		if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_WRITE, &rd_val ) ) < 0 )
		{
			fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x\n",
				__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr );
			return D_RetCode_Err_NetIO;
		}
		else
		{
			if( wr_val != rd_val )
			{
				fprintf( stderr,  "%s: FEU Prescale WrRd_Missmatch for feu=%d beu=%d lnk=%d reg=0x%06x wr_val=0x%08x rd_val=0x%08x\n",
					__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr,  wr_val, rd_val );
				return D_RetCode_Err_WrRd_Missmatch;
			}
		}
//printf("%s: Feu %d prescale (beu %d lnk %d)\n", __FUNCTION__, feu_id, beu_id, beu_lnk_id);
	}

	// Trigger generator parameters
	if( feu_params->Trig_Conf_TrigPipeLen >= 0 )
	{
		// Trigger module config parameters
		// Only optical link trigger is supported 
		wr_val = D_Tg_RegConf_Set( 0, feu_params->Trig_Conf_TrigPipeLen, Tg_Src_Int, 0 );
		// Set configuration register address 
		reg_adr = D_CBus_SetModType(      0, D_CBus_Mod_TrigGen );
		reg_adr = D_Tg_AdrZone_Set( reg_adr, D_Tg_AdrZone_Reg );
		reg_adr = D_Tg_AdrReg_Set(  reg_adr, D_Tg_RegAdr_Conf );
		// Write
		if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_WRITE, &rd_val ) ) < 0 )
		{
			fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x\n",
				__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr );
			return D_RetCode_Err_NetIO;
		}
		else
		{
			if( D_Tg_RegConf_GetConfig(wr_val) != D_Tg_RegConf_GetConfig(rd_val) )
			{
				fprintf( stderr,  "%s: Trigger generator WrRd_Missmatch for feu=%d beu=%d lnk=%d reg=0x%06x wr_val=0x%08x rd_val=0x%08x\n",
					__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr,  wr_val, rd_val );
				return D_RetCode_Err_WrRd_Missmatch;
			}
		}
//printf("%s: Feu %d trig gen (beu %d lnk %d)\n", __FUNCTION__, feu_id, beu_id, beu_lnk_id);
	}
	// No need to configure Trigger generator memory

	// Auxiliary Trigger Interface parameters
	{
		// Ignore Auxiliary Trigger Interface
		wr_val = D_TrigInt_Csr_Conf_Set( 0, 0, 1, 0, 0  );
		// Set configuration register address 
		reg_adr = D_CBus_SetModType(          0, D_CBus_Mod_TrigInt );
		reg_adr = D_TrigInt_AdrReg_Set( reg_adr, C_TrigInt_AdrReg_Csr );
		// Write
		if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_WRITE, &rd_val ) ) < 0 )
		{
			fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x\n",
				__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr );
			return D_RetCode_Err_NetIO;
		}
		else
		{
			if( D_TrigInt_Csr_Conf_Get(wr_val) != D_TrigInt_Csr_Conf_Get(rd_val) )
			{
				fprintf( stderr,  "%s: Aux Trigger Interface WrRd_Missmatch for feu=%d beu=%d lnk=%d reg=0x%06x wr_val=0x%08x rd_val=0x%08x\n",
					__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr,  wr_val, rd_val );
				return D_RetCode_Err_WrRd_Missmatch;
			}
		}
//printf("%s: Feu %d trig aux (beu %d lnk %d)\n", __FUNCTION__, feu_id, beu_id, beu_lnk_id);
	}

	// Self Trigger Interface parameters
	{
		// Mask Dreams: no self trigger
		wr_val = D_SelfTrig_Csr_Conf_Set( 0, 0xFF, 7, 0, 63, 63, 0 );
		// Set configuration register address 
		reg_adr = D_CBus_SetModType(            0, D_CBus_Mod_SelfTrig );
		reg_adr = D_SelfTrig_AdrZone_Set( reg_adr, D_SelfTrig_AdrZone_Reg );
		reg_adr = D_SelfTrig_AdrReg_Set(  reg_adr, C_SelfTrig_AdrReg_Csr );
		// Write
		if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_WRITE, &rd_val ) ) < 0 )
		{
			fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x with %d\n",
				__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr, ret );
			return D_RetCode_Err_NetIO;
		}
		else
		{
			if( D_SelfTrig_Csr_Conf_Get(wr_val) != D_SelfTrig_Csr_Conf_Get(rd_val) )
			{
				fprintf( stderr,  "%s: Self Trigger WrRd_Missmatch for feu=%d beu=%d lnk=%d reg=0x%06x wr_val=0x%08x rd_val=0x%08x\n",
					__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, rd_val );
				return D_RetCode_Err_WrRd_Missmatch;
			}
		}
//printf("%s: Feu %d trig slf (beu %d lnk %d)\n", __FUNCTION__, feu_id, beu_id, beu_lnk_id);
	}
	// No need to set Self Trigger topology parameters

	// FEU Communication parameters
	// Do not touch the Optical link

	// FEU Communication parameters
	// Not clear what to do with UDP/IP link
	{
		// Disable it for a while
		// Set address 
		// Set configuration register address 
		reg_adr = D_CBus_SetModType(          0, D_CBus_Mod_UdpChan );
		reg_adr = D_UdpChan_AdrReg_Set( reg_adr, C_UdpChan_AdrReg_Csr );

		// Disable everything
		wr_val = D_UdpChan_Csr_Enable_Put(0, 0);
		wr_val = D_UdpChan_Csr_IntEnable_Put(wr_val, 0);
		wr_val = D_UdpChan_Csr_MultiPackEnb_Put(wr_val, 0);
		wr_val = D_UdpChan_Csr_MultiPackThr_Put(wr_val, 0);
		// Write
		if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_WRITE, &rd_val ) ) < 0 )
		{
			fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x with %d\n",
				__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr, ret );
			return D_RetCode_Err_NetIO;
		}
		else
		{
			if
			(
				( D_UdpChan_Csr_Enable_Get(wr_val)       != D_UdpChan_Csr_Enable_Get(rd_val)       )
				||
				( D_UdpChan_Csr_IntEnable_Get(wr_val)    != D_UdpChan_Csr_IntEnable_Get(rd_val)    )
				||
				( D_UdpChan_Csr_MultiPackEnb_Get(wr_val) != D_UdpChan_Csr_MultiPackEnb_Get(rd_val) )
				||
				( D_UdpChan_Csr_MultiPackThr_Get(wr_val) != D_UdpChan_Csr_MultiPackThr_Get(rd_val) )
			)
			{
				fprintf( stderr,  "%s: UdpChan WrRd_Missmatch for feu=%d beu=%d lnk=%d reg=0x%06x wr_val=0x%08x rd_val=0x%08x\n",
					__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, rd_val );
				return D_RetCode_Err_WrRd_Missmatch;
			}
		}
	}
	

	/*
	 * EeProm configuration
	 */
	// Not yet supported

	/*
	 * Adc configuration
	 */
	if( (ret=AdcConfig( &(feu_params->adc_params), feu_id, beu_id, beu_lnk_id )) != D_RetCode_Sucsess )
	{
		fprintf( stderr,  "%s: AdcConfig failed for feu=%d beu=%d lnk=%d with %d\n",
			__FUNCTION__, feu_id, beu_id, beu_lnk_id, ret );
		return D_RetCode_Err_NetIO;
	}

	/*
	 * Dream configuration
	 */
	if( (ret=DreamSpiConfig( feu_params->dream_params, feu_id, beu_id, beu_lnk_id, feu_params->Main_Conf_DreamMask )) != D_RetCode_Sucsess )
	{
		fprintf( stderr,  "%s: DreamSpiConfig failed for feu=%d beu=%d lnk=%d with %d\n",
			__FUNCTION__, feu_id, beu_id, beu_lnk_id, ret );
		return D_RetCode_Err_NetIO;
	}

	// Send config command
	// Set address 
	reg_adr = D_CBus_SetModType( 0, D_CBus_Mod_Main );
	reg_adr = D_MainAdr_Set( reg_adr, D_MainAdr_RegCommand );
	// Read
	if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_READ, &rd_val ) ) < 0 )
	{
		fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x with %d\n",
			__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr, ret );
		return D_RetCode_Err_NetIO;
	}
	wr_val = D_Main_Cmd_Config_Set( rd_val );
	// Write
	if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_WRITE, &rd_val ) ) < 0 )
	{
		fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x with %d\n",
			__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr, ret );
		return D_RetCode_Err_NetIO;
	}
	else
	{
		if( D_Main_Cmd_Config_Get(wr_val) != D_Main_Cmd_Config_Get(rd_val) )
		{
			fprintf( stderr,  "%s: Main Command (Config) WrRd_Missmatch for feu=%d beu=%d lnk=%d reg=0x%06x wr_val=0x%08x rd_val=0x%08x\n",
				__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr,  wr_val, rd_val );
			return D_RetCode_Err_WrRd_Missmatch;
		}
	}
printf("%s: Feu %d Config done (beu %d lnk %d)\n", __FUNCTION__, feu_id, beu_id, beu_lnk_id);

	return D_RetCode_Sucsess;
}

/*
 * Configuration function
 */
int FeuConfig( FeuParams *feu_params, int feu_id, int beu_id, int beu_lnk_id )
{
	int ret;

	int feu_sn;

	/*
	 * Check input parameters
	 */
	if( feu_params == (FeuParams *)NULL )
	{
		fprintf( stderr, "%s: feu_params=NULL\n", __FUNCTION__ ); 
		return D_RetCode_Err_Null_Pointer;
	}
	if( (feu_id < 1) || (DEF_MAX_NB_OF_FEU<=feu_id) )
	{
		fprintf( stderr, "%s: feu_index=%d<1 or >=%d\n", __FUNCTION__, feu_id, DEF_MAX_NB_OF_FEU ); 
		return D_RetCode_Err_Wrong_Param;
	}
	if( (beu_id < 1) || (DEF_MAX_NB_OF_BEU<=beu_id) )
	{
		fprintf( stderr, "%s: feu_id=%d<0 or >=%d\n", __FUNCTION__, beu_id, DEF_MAX_NB_OF_BEU ); 
		return D_RetCode_Err_Wrong_Param;
	}
	if( (beu_lnk_id < 0) || (DEF_MAX_NB_OF_FEU_PER_BEU<=beu_lnk_id) )
	{
		fprintf( stderr, "%s: feu_id=%d<0 or >=%d\n", __FUNCTION__, beu_lnk_id, DEF_MAX_NB_OF_FEU_PER_BEU ); 
		return D_RetCode_Err_Wrong_Param;
	}

	// First Fix the Peek/Poke global variables
	PeekPoke_Feu = feu_id;
	PeekPoke_Beu = beu_id;
	PeekPoke_Lnk = beu_lnk_id;

	// Make sure the Link is active
	if( (ret = Beu_CheckFeuLink(feu_id, beu_id, beu_lnk_id)) < 0 )
	{
		fprintf( stderr,  "%s: Beu_ChekFeuLink failed with %d for %d %d %d\n",
			__FUNCTION__, ret, feu_id, beu_id, beu_lnk_id );
		return ret;
	} 

	/*
	 *  Try to get FEU ID
	 */
	if( (ret = _FeuGetSn( feu_id, beu_id, beu_lnk_id, &feu_sn )) != D_RetCode_Sucsess )
	{
		fprintf( stderr,  "%s: _FeuGetSn failed for feu=%d beu=%d lnk=%d with %d\n",
			__FUNCTION__, feu_id, beu_id, beu_lnk_id, ret );
		return D_RetCode_Err_NetIO;
	}

	// Get exclussive access to the FEU
	if( (ret = FeuConfig_Lock()) != D_RetCode_Sucsess )
	{
		fprintf( stderr, "%s: Feu_Lock fail for feu=%d sn=%d beu=%d lnk=%d with ret %d\n", __FUNCTION__, feu_id, feu_sn, beu_id, beu_lnk_id, ret ); 
		return ret;
	}

	// Configure FEU
	if( (ret = _FeuConfig( feu_params, feu_id, beu_id, beu_lnk_id )) != D_RetCode_Sucsess )
	{
		fprintf( stderr, "%s: _FeuConfig fail for feu=%d sn=%d beu=%d lnk=%d with ret %d\n", __FUNCTION__, feu_id, feu_sn, beu_id, beu_lnk_id, ret ); 
		// Attempt to unlock FEU
		FeuConfig_UnLock();
		return ret;
	}

	// Free FEU
	if( (ret = FeuConfig_UnLock()) != D_RetCode_Sucsess )
	{
		fprintf( stderr, "%s: Feu_Lock fail for feu=%d sn=%d beu=%d lnk=%d with ret %d\n", __FUNCTION__, feu_id, feu_sn, beu_id, beu_lnk_id, ret ); 
		return ret;
	}

	return D_RetCode_Sucsess;
}

/*
 * Enable or disable trigger processing
 */
int FeuRun( FeuParams *feu_params, int feu_id, int beu_id, int beu_lnk_id, int run )
{
	unsigned int reg_adr;
	unsigned int wr_val;
	unsigned int rd_val;

	int ret;

	int dream_pair;
	int dream_power;

	int reg;
	int dream_min;
	int dream_max;
	int dream;

	int feu_sn;

	if( feu_params == (FeuParams *)NULL )
	{
		fprintf( stderr, "%s: feu_params=NULL\n", __FUNCTION__ ); 
		return D_RetCode_Err_Null_Pointer;
	}
	if( (feu_id < 1) || (DEF_MAX_NB_OF_FEU<=feu_id) )
	{
		fprintf( stderr, "%s: feu_index=%d<1 or >=%d\n", __FUNCTION__, feu_id, DEF_MAX_NB_OF_FEU ); 
		return D_RetCode_Err_Wrong_Param;
	}
	if( (beu_id < 1) || (DEF_MAX_NB_OF_BEU<=beu_id) )
	{
		fprintf( stderr, "%s: feu_id=%d<0 or >=%d\n", __FUNCTION__, beu_id, DEF_MAX_NB_OF_BEU ); 
		return D_RetCode_Err_Wrong_Param;
	}
	if( (beu_lnk_id < 0) || (DEF_MAX_NB_OF_FEU_PER_BEU<=beu_lnk_id) )
	{
		fprintf( stderr, "%s: feu_id=%d<0 or >=%d\n", __FUNCTION__, beu_lnk_id, DEF_MAX_NB_OF_FEU_PER_BEU ); 
		return D_RetCode_Err_Wrong_Param;
	}

	// First Fix the Peek/Poke global variables
	PeekPoke_Feu = feu_id;
	PeekPoke_Beu = beu_id;
	PeekPoke_Lnk = beu_lnk_id;

	/*
	 *  Try to get FEU ID
	 */
	// Those are RO registers, so no semaphore is needed
	// Set address 
	reg_adr = D_CBus_SetModType( 0, D_CBus_Mod_Main );
	// 4 LSB-s of SN are in Firmware revision register
	reg_adr = D_MainAdr_Set( reg_adr, D_MainAdr_RegFwRev );
	if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_READ, &rd_val ) ) < 0 )
	{
		fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x\n",
			__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr );
		return D_RetCode_Err_NetIO;
	}
//printf("Press CR to continue <-\n");
//getchar();
	// Get the 4 LSB-s
//	printf("%s: rd_val=0x%08x\n", __FUNCTION__, rd_val);
	feu_sn = D_Main_Rev_GetSrN(rd_val);
	// 4 MSB-s of SN are in Software revision register
	reg_adr = D_MainAdr_Set( reg_adr, D_MainAdr_RegSwRev );
	if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_READ, &rd_val ) ) < 0 )
	{
		fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x\n",
			__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr );
		return D_RetCode_Err_NetIO;
	}
//printf("Press CR to continue <-\n");
//getchar();
	// Complete with 4 MSB-s
//	printf("%s: rd_val=0x%08x\n", __FUNCTION__, rd_val);
	feu_sn = (D_Main_Rev_GetSrN(rd_val)<<4) + feu_sn;
	printf("%s: Feu %d SN %d (beu %d lnk %d)\n", __FUNCTION__, feu_id, feu_sn, beu_id, beu_lnk_id);

	/*
	 *  Try to get Exclusive access to FEU
	 */
	if( (ret=Sem_Init(D_Feu_AdrReg_Set( D_CBus_SetModType( 0, D_CBus_Mod_Feu ), C_Feu_AdrReg_SlowCtrl ))) ==  D_RetCode_Wrn_SemBusy )
	{
		fprintf( stderr,  "%s: Sem_Init warning Semaphore Busy for feu=%d beu=%d lnk=%d\n",
			__FUNCTION__, feu_id, beu_id, beu_lnk_id );
	}
	else if( ret != D_RetCode_Sucsess )
	{
		fprintf( stderr,  "%s: Sem_Init failed for feu=%d beu=%d lnk=%d with %s %d\n",
			__FUNCTION__, feu_id, beu_id, beu_lnk_id, sem_msg, ret );
		return ret;
	}
	// Get exclusive access to FEU
	if( (ret = FeuConfig_Lock()) != D_RetCode_Sucsess )
	{
		fprintf( stderr, "%s: Feu_Lock fail for feu=%d sn=%d beu=%d lnk=%d with ret %d\n", __FUNCTION__, feu_id, feu_sn, beu_id, beu_lnk_id, ret ); 
		return ret;
	}

	/*
	 *  Set or reset the command
	 */
	// Set address 
	reg_adr = D_CBus_SetModType( 0, D_CBus_Mod_Main );
	reg_adr = D_MainAdr_Set( reg_adr, D_MainAdr_RegCommand );
	// Read
	if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_READ, &rd_val ) ) < 0 )
	{
		fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x with %d\n",
			__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr, ret );
		return D_RetCode_Err_NetIO;
	}
	if( run )
		wr_val = D_Main_Cmd_Run_Set( rd_val );
	else
		wr_val = D_Main_Cmd_Run_Clr( rd_val );
	// Write
	if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_WRITE, &rd_val ) ) < 0 )
	{
		fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x with %d\n",
			__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr, ret );
		return D_RetCode_Err_NetIO;
	}
	else
	{
		if( D_Main_Cmd_Run_Get(wr_val) != D_Main_Cmd_Run_Get(rd_val) )
		{
			fprintf( stderr,  "%s: Main Command (Run) WrRd_Missmatch for feu=%d beu=%d lnk=%d reg=0x%06x wr_val=0x%08x rd_val=0x%08x\n",
				__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr,  wr_val, rd_val );
			return D_RetCode_Err_WrRd_Missmatch;
		}
	}
	// Check
	if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, 0, DEF_FEU_READ, &rd_val ) ) < 0 )
	{
		fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x with %d\n",
			__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr, ret );
		return D_RetCode_Err_NetIO;
	}
	else
	{
		if( D_Main_Cmd_Run_Get(wr_val) != D_Main_Cmd_Run_Get(rd_val) )
		{
			fprintf( stderr,  "%s: Main Command (Run) WrRd_Missmatch for feu=%d beu=%d lnk=%d reg=0x%06x wr_val=0x%08x rd_val=0x%08x\n",
				__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr,  wr_val, rd_val );
			return D_RetCode_Err_WrRd_Missmatch;
		}
	}	

	/*
	 *  Free exclusive access to FEU
	 */
	if( (ret = FeuConfig_UnLock()) != D_RetCode_Sucsess )
	{
		fprintf( stderr, "%s: Feu_Lock fail for feu=%d sn=%d beu=%d lnk=%d with ret %d\n", __FUNCTION__, feu_id, feu_sn, beu_id, beu_lnk_id, ret ); 
		return ret;
	}

	return D_RetCode_Sucsess;
}

/*
 * Toggle FEU commands
 */
int FeuToggleCommand( FeuParams *feu_params, int feu_id, int beu_id, int beu_lnk_id, FeuTogCmd tog_cmd )
{
	unsigned int reg_adr;
	unsigned int wr_val;
	unsigned int rd_val;

	int ret;

	int dream_pair;
	int dream_power;

	int reg;
	int dream_min;
	int dream_max;
	int dream;

	int feu_sn;

	if( feu_params == (FeuParams *)NULL )
	{
		fprintf( stderr, "%s: feu_params=NULL\n", __FUNCTION__ ); 
		return D_RetCode_Err_Null_Pointer;
	}
	if( (feu_id < 1) || (DEF_MAX_NB_OF_FEU<=feu_id) )
	{
		fprintf( stderr, "%s: feu_index=%d<1 or >=%d\n", __FUNCTION__, feu_id, DEF_MAX_NB_OF_FEU ); 
		return D_RetCode_Err_Wrong_Param;
	}
	if( (beu_id < 1) || (DEF_MAX_NB_OF_BEU<=beu_id) )
	{
		fprintf( stderr, "%s: feu_id=%d<0 or >=%d\n", __FUNCTION__, beu_id, DEF_MAX_NB_OF_BEU ); 
		return D_RetCode_Err_Wrong_Param;
	}
	if( (beu_lnk_id < 0) || (DEF_MAX_NB_OF_FEU_PER_BEU<=beu_lnk_id) )
	{
		fprintf( stderr, "%s: feu_id=%d<0 or >=%d\n", __FUNCTION__, beu_lnk_id, DEF_MAX_NB_OF_FEU_PER_BEU ); 
		return D_RetCode_Err_Wrong_Param;
	}

	// First Fix the Peek/Poke global variables
	PeekPoke_Feu = feu_id;
	PeekPoke_Beu = beu_id;
	PeekPoke_Lnk = beu_lnk_id;

	/*
	 *  Try to get FEU ID
	 */
	// Those are RO registers, so no semaphore is needed
	// Set address 
	reg_adr = D_CBus_SetModType( 0, D_CBus_Mod_Main );
	// 4 LSB-s of SN are in Firmware revision register
	reg_adr = D_MainAdr_Set( reg_adr, D_MainAdr_RegFwRev );
	if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_READ, &rd_val ) ) < 0 )
	{
		fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x\n",
			__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr );
		return D_RetCode_Err_NetIO;
	}
//printf("Press CR to continue <-\n");
//getchar();
	// Get the 4 LSB-s
//	printf("%s: rd_val=0x%08x\n", __FUNCTION__, rd_val);
	feu_sn = D_Main_Rev_GetSrN(rd_val);
	// 4 MSB-s of SN are in Software revision register
	reg_adr = D_MainAdr_Set( reg_adr, D_MainAdr_RegSwRev );
	if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_READ, &rd_val ) ) < 0 )
	{
		fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x\n",
			__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr );
		return D_RetCode_Err_NetIO;
	}
//printf("Press CR to continue <-\n");
//getchar();
	// Complete with 4 MSB-s
//	printf("%s: rd_val=0x%08x\n", __FUNCTION__, rd_val);
	feu_sn = (D_Main_Rev_GetSrN(rd_val)<<4) + feu_sn;
	printf("%s: Feu %d SN %d (beu %d lnk %d)\n", __FUNCTION__, feu_id, feu_sn, beu_id, beu_lnk_id);

	/*
	 *  Try to get Exclusive access to FEU
	 */
	if( (ret=Sem_Init(D_Feu_AdrReg_Set( D_CBus_SetModType( 0, D_CBus_Mod_Feu ), C_Feu_AdrReg_SlowCtrl ))) ==  D_RetCode_Wrn_SemBusy )
	{
		fprintf( stderr,  "%s: Sem_Init warning Semaphore Busy for feu=%d beu=%d lnk=%d\n",
			__FUNCTION__, feu_id, beu_id, beu_lnk_id );
	}
	else if( ret != D_RetCode_Sucsess )
	{
		fprintf( stderr,  "%s: Sem_Init failed for feu=%d beu=%d lnk=%d with %s %d\n",
			__FUNCTION__, feu_id, beu_id, beu_lnk_id, sem_msg, ret );
		return ret;
	}
	// Get exclusive access to FEU
	if( (ret = FeuConfig_Lock()) != D_RetCode_Sucsess )
	{
		fprintf( stderr, "%s: Feu_Lock fail for feu=%d sn=%d beu=%d lnk=%d with ret %d\n", __FUNCTION__, feu_id, feu_sn, beu_id, beu_lnk_id, ret ); 
		return ret;
	}

	/*
	 *  Set the command
	 */
	// Set address 
	reg_adr = D_CBus_SetModType( 0, D_CBus_Mod_Main );
	reg_adr = D_MainAdr_Set( reg_adr, D_MainAdr_RegCommand );
	// Read
	if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_READ, &rd_val ) ) < 0 )
	{
		fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x with %d\n",
			__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr, ret );
		return D_RetCode_Err_NetIO;
	}
	wr_val = rd_val | tog_cmd;
	// Write
	if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_WRITE, &rd_val ) ) < 0 )
	{
		fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x with %d\n",
			__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr, ret );
		return D_RetCode_Err_NetIO;
	}
	else
	{
		if( (wr_val & tog_cmd) != (rd_val & tog_cmd) )
		{
			fprintf( stderr,  "%s: Main Command (%d) WrRd_Missmatch for feu=%d beu=%d lnk=%d reg=0x%06x wr_val=0x%08x rd_val=0x%08x\n",
				__FUNCTION__, tog_cmd, feu_id, beu_id, beu_lnk_id, reg_adr,  wr_val, rd_val );
			return D_RetCode_Err_WrRd_Missmatch;
		}
	}
	// Check
	if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, 0, DEF_FEU_READ, &rd_val ) ) < 0 )
	{
		fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x with %d\n",
			__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr, ret );
		return D_RetCode_Err_NetIO;
	}
	else
	{
		if( (wr_val & tog_cmd) != (rd_val & tog_cmd) )
		{
			fprintf( stderr,  "%s: Main Command (%d) WrRd_Missmatch for feu=%d beu=%d lnk=%d reg=0x%06x wr_val=0x%08x rd_val=0x%08x\n",
				__FUNCTION__, tog_cmd, feu_id, beu_id, beu_lnk_id, reg_adr,  wr_val, rd_val );
			return D_RetCode_Err_WrRd_Missmatch;
		}
	}

	/*
	 *  Reset the command
	 */
	wr_val = rd_val & (~tog_cmd);
	// Write
	if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, wr_val, DEF_FEU_WRITE, &rd_val ) ) < 0 )
	{
		fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x with %d\n",
			__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr, ret );
		return D_RetCode_Err_NetIO;
	}
	else
	{
		if( (wr_val & tog_cmd) != (rd_val & tog_cmd) )
		{
			fprintf( stderr,  "%s: Main Command (%d) WrRd_Missmatch for feu=%d beu=%d lnk=%d reg=0x%06x wr_val=0x%08x rd_val=0x%08x\n",
				__FUNCTION__, tog_cmd, feu_id, beu_id, beu_lnk_id, reg_adr,  wr_val, rd_val );
			return D_RetCode_Err_WrRd_Missmatch;
		}
	}
	// Check
	if( (ret = Beu_ReqResp(feu_id, beu_id, beu_lnk_id, reg_adr, 0, DEF_FEU_READ, &rd_val ) ) < 0 )
	{
		fprintf( stderr,  "%s: Beu_ReqResp failed for feu=%d beu=%d lnk=%d reg=0x%06x with %d\n",
			__FUNCTION__, feu_id, beu_id, beu_lnk_id, reg_adr, ret );
		return D_RetCode_Err_NetIO;
	}
	else
	{
		if( (wr_val & tog_cmd) != (rd_val & tog_cmd) )
		{
			fprintf( stderr,  "%s: Main Command (%d) WrRd_Missmatch for feu=%d beu=%d lnk=%d reg=0x%06x wr_val=0x%08x rd_val=0x%08x\n",
				__FUNCTION__, tog_cmd, feu_id, beu_id, beu_lnk_id, reg_adr,  wr_val, rd_val );
			return D_RetCode_Err_WrRd_Missmatch;
		}
	}

	/*
	 *  Free exclusive access to FEU
	 */
	if( (ret = FeuConfig_UnLock()) != D_RetCode_Sucsess )
	{
		fprintf( stderr, "%s: Feu_Lock fail for feu=%d sn=%d beu=%d lnk=%d with ret %d\n", __FUNCTION__, feu_id, feu_sn, beu_id, beu_lnk_id, ret ); 
		return ret;
	}

	return D_RetCode_Sucsess;
}

