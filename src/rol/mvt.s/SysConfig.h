/*
--------------------------------------------------------------------------------
-- Company:        IRFU / CEA Saclay
-- Engineers:      Irakli.MANDJAVIDZE@cea.fr (IM)
-- 
-- Project Name:   Clas12 Micromegas Vertex Tracker
-- Design Name:    Clas12 Dream testbench software
--
-- Module Name:    SysConfig.h
-- Description:    System configuration
--
-- Target Devices: 
-- Tool versions:  
-- 
-- Create Date:    0.0 2014/10/13 IM
-- Revision:       
--
-- Comments:
--
--------------------------------------------------------------------------------
*/
#ifndef H_SysConfiguration
#define H_SysConfiguration

#include "BecConfigParams.h"
#include "TiConfigParams.h"
#include "SdConfigParams.h"
#include "FeuConfigParams.h"
#include "SysConfigParams.h"

/* Max allowed 4194304 */
/* 1 event 48 FEU-s, 64 samples, 604 2-byte words per sample */
#define MAX_BLOCK_LENGTH 3710976

int  TiConfig(   TiParams *params );
int  TiRun(      TiParams *params, int run );
int  TiGo(       TiParams *params );
int  SdConfig(   SdParams *params );

/*
 * Toggle FEU commands
 */
 typedef enum _FeuTogCmd
 {
 	Reset        = 1,
	LatchStat    = 16,
	ClearStat    = 32
 } FeuTogCmd;
int  FeuReset(  FeuParams *params, int feu_id, int beu_id, int beu_lnk_id );
int  FeuConfig( FeuParams *params, int feu_id, int beu_id, int beu_lnk_id );
int  FeuRun(    FeuParams *params, int feu_id, int beu_id, int beu_lnk_id, int run );
int  FeuToggleCommand( FeuParams *feu_params, int feu_id, int beu_id, int beu_lnk_id, FeuTogCmd tog_cmd );
int  FeuConfig_Lock();
int  FeuConfig_UnLock();
void FeuConfig_CleanUp();

int  SysConfig( SysParams *params );
int  SysRun();
int  SysStop();
int  SysGo();
int  SysConfigFromFile( char *params );
void SysConfig_CleanUp();

#endif // #ifndef H_SysConfiguration
