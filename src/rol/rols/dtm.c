
/* rol1.c - UNIX first readout list (polling mode) */

#define DMA_TO_BIGBUF /*if want to dma directly to the big buffers*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <sys/types.h>
#ifndef VXWORKS
#include <sys/time.h>
#endif

#include "circbuf.h"
#include "dxm.h"


/*****************************/
/* former 'crl' control keys */

/* readout list ROL1 */
#define ROL_NAME__ "DTM"

/* polling */
#define POLLING_MODE


/* name used by loader */
#define INIT_NAME dtm__init

#include "rol.h"

void usrtrig(unsigned long, unsigned long);
void usrtrig_done();

/* test readout */
#include "DTM_source.h"
#include "ControlCmdMem.h"

ControlCmdMemory *smem;
char confFileFeb[256];

/************************/
/************************/

#include "coda.h"
#include "tt.h"

static char rcname[5];

/* for compatibility with hps1.c */
int
getTdcTypes(int *typebyslot)
{
  return(0);
}
int
getTdcSlotNumbers(int *slotnumbers)
{
  return(0);
}

/*
long decrement;
extern long nevent;
*/

static void
__download()
{

#ifdef POLLING_MODE
  rol->poll = 1;
#else
  rol->poll = 0;
#endif

  printf("\n>>>>>>>>>>>>>>> ROCID=%d, CLASSID=%d <<<<<<<<<<<<<<<<\n\n",rol->pid,rol->classid);
  printf("CONFFILE >%s<\n\n",rol->confFile);
  // In the future the config file here points to an expanded HPS config 
  // file that we have access to from all RCEs. To simulate that I put the 
  // config on /mnt/host now. This default file will be used unless something 
  // is explicitely seleected in the RC. 
  // /Pelle
  char* confFilePtr;
  if(strlen(rol->confFile)>0 && strcmp(rol->confFile,"none")!=0) {
     confFilePtr = rol->confFile;
  } else {

/*sergey: replace '/usr/clas12/release/0.2' with CLAS env var */
strcpy(rol->confFile,"/usr/clas12/release/0.2/slac_svt/svtdaq/daq/config/clasdev.cnf");
confFilePtr = rol->confFile;
/*
     fprintf(stderr,"Failed to find config file\n");
     exit(1);
*/
  }
  // Extract the file path to the xml config file
  // If in the end the file is expanded we will not need this but 
  // just make sure that we only read out xml part of the expanded file.
  // /Pelle
  getFebConfigFilePath(confFilePtr,"CONFIG",confFileFeb,256);
  if(strlen(confFileFeb)>0) {
     printf("Got FEB config file: %s\n",confFileFeb);
  } else {
     printf("No FEB config file extracted!?\n");        
     exit(1);
  }


  /* Clear some global variables etc for a clean start */
  CTRIGINIT;

  /* init trig source TEST */
  DTM_INIT;
  /*CDOINIT(TIPRIMARY,TIR_SOURCE);*/

  // Open control shared memory, assume server is running as root
  if ( controlCmdOpenAndMap ( &smem, "hps", 1, 0)  < 0 ) 
     printf("Failed to open shared memory\n");
  else 
     printf ("Opened shared memory\n");

  // Send download command to daq server
  printf("Set state to download\n");
  controlCmdSetCommand ( smem, CONTROL_CMD_TYPE_EXEC_COMMAND, "SetRunState", "Download");
  if ( ! controlCmdGetResultTimeout(smem,NULL,10000) ) {
     printf ("Timeout waiting for daq response\n");
     exit(1);
  }

  printf("INFO: User Download 1 Executed.\n");

  return;
}


static void
__prestart()
{
  unsigned long jj, adc_id, sl;
  char *env;

  *(rol->nevents) = 0;

  /* Register a sync trigger source */
#ifdef POLLING_MODE
  CTRIGRSS(DTM, 1, usrtrig, usrtrig_done);
  rol->poll = 1;
#else
  CTRIGRSA(DTM, 1, usrtrig, usrtrig_done);
  rol->poll = 0;
#endif

  // If config was written previsouly it is overwritten with the config file extracted 
  // from coda here. //Pelle
  printf("Set configuration from %s\n",confFileFeb);
  controlCmdSetCommand ( smem, CONTROL_CMD_TYPE_EXEC_COMMAND, "ReadXmlFile", confFileFeb);
  if ( ! controlCmdGetResultTimeout(smem,NULL,10000) ) {
     printf ("Timeout waiting for daq response\n");
     exit(1);
  }

  // Send prestart command to daq server
  controlCmdSetCommand ( smem, CONTROL_CMD_TYPE_EXEC_COMMAND, "SetRunState", "Prestart");
  if ( ! controlCmdGetResultTimeout(smem,NULL,10000) ) {
     printf ("Timeout waiting for daq response\n");
     exit(1);
  }

  sprintf(rcname,"RC%02d",rol->pid);
  printf("rcname >%4.4s<\n",rcname);

  printf("INFO: User Prestart 1 executed\n");

  /* from parser (do we need that in rol2 ???) */
  *(rol->nevents) = 0;
  rol->recNb = 0;

  return;
}

static void
__end()
{
  int ii, total_count, rem_count;

  CDODISABLE(DTM,1,0);

  printf("INFO: User End 1 Executed\n");

  // Send command to daq server
  printf("Set state to end\n");
  controlCmdSetCommand ( smem, CONTROL_CMD_TYPE_EXEC_COMMAND, "SetRunState", "End");
  if ( ! controlCmdGetResultTimeout(smem,NULL,10000) ) {
     printf ("Timeout waiting for daq response\n");
     exit(1);
  }

  printf("INFO: User End 1 Executed\n");

  return;
}

static void
__pause()
{
  CDODISABLE(DTM,1,0);

  // Send command to daq server
  printf("Set state to pause\n");
  controlCmdSetCommand ( smem, CONTROL_CMD_TYPE_EXEC_COMMAND, "SetRunState", "Pause");
  if ( ! controlCmdGetResultTimeout(smem,NULL,10000) ) {
     printf ("Timeout waiting for daq response\n");
     exit(1);
  }

  printf("INFO: User Pause 1 Executed\n");

  return;
}

static void
__go()
{
  char *env;

  printf("INFO: User Go 1 Executed\n");

  // Send command to daq server
  printf("Set state to go\n");
  controlCmdSetCommand ( smem, CONTROL_CMD_TYPE_EXEC_COMMAND, "SetRunState", "Go");
  if ( ! controlCmdGetResultTimeout(smem,NULL,10000) ) {
     printf ("Timeout waiting for daq response\n");
     exit(1);
  }

  printf("INFO: User Go 1 Enabling\n");
  CDOENABLE(DTM,1,1);
  printf("INFO: User Go 1 Enabled\n");

  return;
}

void
usrtrig(unsigned long EVTYPE, unsigned long EVSOURCE)
{
  int len, ii;

  rol->dabufp = (long *) 0;

  //CEOPEN(EVTYPE, BT_BANKS);
  CEOPEN(EVTYPE, BT_UI4);

  len = 10;
  for(ii=0; ii<len; ii++)
  {
    *rol->dabufp++ = ii;
  }

  CECLOSE;

  return;
}
  
void
usrtrig_done()
{
  return;
}  

void
__done()
{
  /* from parser */
  poolEmpty = 0; /* global Done, Buffers have been freed */

  return;
}
  
static void
__status()
{
  return;
}  

