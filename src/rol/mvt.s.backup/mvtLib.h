
/******************************************************************************
*
*  mvtLib.h  - Library header file for readout of the Clas12 MVT & FTT
*                
*  Author: Yassir.Moudden@cea.fr 
*          Irakli.Mandjavidze@cea.fr
*          June 2015
*
*  Revision  1.0 - Initial Revision
*
*  SVN: $Rev$
*
******************************************************************************/

#ifndef __MVTLIB__
#define __MVTLIB__

int mvtInit( unsigned int addr, unsigned int addr_inc, int nfind, int iFlag );
int mvtSlot( int id );
int mvtDownload();
int mvtPrestart();
int mvtGo();
int mvtEnd();
int mvtGBReady( int id ); 
int mvtReadBlock( int id, unsigned int *data, int nwrds, int rflag );
int mvtCleanup();

#endif // #ifndef __MVTLIB__
