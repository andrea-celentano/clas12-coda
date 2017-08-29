//10 AOUT 2016
/******************************************************************************
*
*  mvtLib.h  - Library header file for readout of the Clas12 MVT & FTT
*                
*  Author: Yassir.Moudden@cea.fr 
*          Irakli.Mandjavidze@cea.fr
*          June 2015
*
*  Revision  1.0 - Initial Revision
*                  2015/08/27: mvtUploadAll added
*                  2015/08/28: mvtConfig added
*
*  SVN: $Rev$
*
******************************************************************************/

#ifndef __MVTLIB__
#define __MVTLIB__

#include <stdio.h>

int mvtInit( unsigned int addr, unsigned int addr_inc, int nfind, int iFlag );
int mvtSlot( int id );
int mvtDownload();
int mvtPrestart();
int mvtGo();
int mvtEnd();
int mvtGBReady( int id ); 
int mvtReadBlock( int id, unsigned int *data, int nwrds, int rflag );
int mvtCleanup();
int mvtUploadAll( char *string, int length );
int mvtConfig( char *sys_conf_params_filename, int run_number, int bec_id );

int mvtGetNbrOfFeu(int id, int BeuId);
int mvtGetNbrOfSamplesPerEvent(int id);
int mvtGetNbrOfEventsPerBlock(int id);
int mvtGetNbrOfBeu(int id);
int mvtGetPrescale(int id);
int mvtGetZSMode(int id);
int mvtSetCurrentBlockLevel( int block_level );
int mvtGetRepRawData();
int mvtStatus(int numFeu);
int mvtStatusDump(int numFeu, FILE *fptr);

#endif // #ifndef __MVTLIB__
