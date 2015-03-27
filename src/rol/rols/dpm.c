
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
#define ROL_NAME__ "DPM"

/* polling */
#define POLLING_MODE


/* name used by loader */
#define INIT_NAME dpm__init

#include "rol.h"

void usrtrig(unsigned long, unsigned long);
void usrtrig_done();

/* test readout */
#include "DPM_source.h"
#include "ControlCmdMem.h"

ControlCmdMemory *smem;
char confFileFeb[256];
char confFileThr[256];

/************************/
/************************/

#include "coda.h"
#include "tt.h"

static char rcname[5];


#define TIMERL_VAR \
  static hrtime_t startTim, stopTim, dTim; \
  static int nTim; \
  static hrtime_t Tim, rmsTim, minTim=10000000, maxTim, normTim=1

#define TIMERL_START \
{ \
  startTim = gethrtime(); \
}

#define TIMERL_STOP(whentoprint_macros,histid_macros) \
{ \
  stopTim = gethrtime(); \
  if(stopTim > startTim) \
  { \
    nTim ++; \
    dTim = stopTim - startTim; \
    /*if(histid_macros >= 0)   \
    { \
      uthfill(histi, histid_macros, (int)(dTim/normTim), 0, 1); \
    }*/														\
    Tim += dTim; \
    rmsTim += dTim*dTim; \
    minTim = minTim < dTim ? minTim : dTim; \
    maxTim = maxTim > dTim ? maxTim : dTim; \
    /*logMsg("good: %d %ud %ud -> %d\n",nTim,startTim,stopTim,Tim,5,6);*/ \
    if(nTim == whentoprint_macros) \
    { \
      logMsg("timer: %7llu microsec (min=%7llu max=%7llu rms**2=%7llu)\n", \
                Tim/nTim/normTim,minTim/normTim,maxTim/normTim, \
                ABS(rmsTim/nTim-Tim*Tim/nTim/nTim)/normTim/normTim,5,6); \
      nTim = Tim = 0; \
    } \
  } \
  else \
  { \
    /*logMsg("bad:  %d %ud %ud -> %d\n",nTim,startTim,stopTim,Tim,5,6);*/ \
  } \
}



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



// Get the xml configuration string from the DAQ
int svtDaqReadConfig(const char* str) {
   int len;
   printf("svtDaqUpLoadAll called.\n");
   str = controlCmdGetConfig( smem );
   if ( ! controlCmdGetResultTimeout(smem,NULL,10000) ) {
      printf ("Timeout waiting for daq response\n");
      exit(1);
   }
   if(str==NULL) {
      printf("Error getting string from control server!\n");
      exit(1);
   }
   // get the length not null character
   len = strlen(str);
   printf("svtDaqUpLoadAll got string at %p of strlen=%d.\n",str,len);
   return len;
}
 
// Get the xml configuration string from the DAQ          
int svtDaqUpLoadAll(char* chptr) {
   printf("svtDaqUpLoadAll called with chptr=%p.\n",chptr);

   int nbytes;
   int len;
   nbytes = 0;
          
   /* add one 'return' to make evio2xml output nicer */
   *chptr++ = '\n';
   nbytes++;
          
   /* read the actual string from the daq */
   /* this do not include null character */
   len = svtDaqReadConfig((const char*) chptr);
   printf("cfg len=%d\n",len);
   //printf(">%s<\n",chptr);

   // increment pointers
   // Note that null character is not included. We will add it later.
   chptr += len;
   nbytes += len;
          
   /* add one 'return' to make evio2xml output nicer */
   *chptr++ = '\n';
   nbytes ++;

   /* add end of string */
   *chptr++ = '\0';
   nbytes++;

   return nbytes;
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
  /*
FEB_CONFIG_FILE /mnt/host/daq/config/rce_config.xml
  strcpy(rol->confFile,
  */

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
  getFebConfigFilePath(confFilePtr,"CONFIG",confFileFeb,256);
  if(strlen(confFileFeb)>0) {
     printf("Got FEB config file: %s\n",confFileFeb);
  } else {
     printf("No FEB config file extracted!?\n");        
     exit(1);
  }

  // Find the threshold xml file
  getFebConfigFilePath(confFilePtr,"THRESHOLDS",confFileThr,256);
  if(strlen(confFileThr)>0) {
     printf("Got threhold config file: %s\n",confFileThr);
  } else {
     printf("WARNING WARNING -> No threshold config file extracted!?\n");        
     //exit(1);
  }

  /* Flag if this is a charge injection run */
  run_type = getRunType(confFileFeb);
  if (run_type == 0) printf("Normal run type\n");
  else if (run_type == 1) printf("Charge injection run\n");
  else {
    printf("invalid run type (%d)\n",run_type);
    exit(1);
  }
  
  /* Clear some global variables etc for a clean start */
  CTRIGINIT;

  /* init trig source TEST */
  DPM_INIT;
  /*CDOINIT(TIPRIMARY,TIR_SOURCE);*/

  // Open DMA
  myfd = open("/dev/axi_stream_dma_0",O_RDWR | O_NONBLOCK);
  if ( myfd < 0 ) printf("Failed to open DMA device\n");
  else printf ("Opened DMA device\n");

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

  printf("INFO: User Download 1 Executed\n");

  return;
}


static void
__prestart()
{
  unsigned long jj, adc_id, sl;
  char *env;
  char buffer[1024*1024];

  *(rol->nevents) = 0;

  /* Register a sync trigger source */
#ifdef POLLING_MODE
  CTRIGRSS(DPM, 1, usrtrig, usrtrig_done);
#else
  CTRIGRSA(DPM, 1, usrtrig, usrtrig_done);
#endif

  /* not needed here ??? */
  rol->poll = 1;

  lastTime  = 0;
  trigCount = 0;
  lastCount = 0;
  lastSize  = 0;
  ackTrig   = 0;

  sprintf(rcname,"RC%02d",rol->pid);
  printf("rcname >%4.4s<\n",rcname);

  // CONFIG
  uint hitsPerApv    = 1;
  uint numApv        = 5;
  uint numHybrids    = 3;
  uint bytesPerEvent = 16;
  uint bufferCnt     = 5;

  int count = 0;

  while (axisRead(myfd,buffer,1024*1024,0,0,0) > 0 ) count++;
  printf("Flushed %i buffers\n",count);

  // If config was written previsouly it is overwritten with the config file extracted 
  // from coda here. //Pelle
  printf("Set configuration from %s\n",confFileFeb);
  controlCmdSetCommand ( smem, CONTROL_CMD_TYPE_EXEC_COMMAND, "ReadXmlFile", confFileFeb);
  if ( ! controlCmdGetResultTimeout(smem,NULL,10000) ) {
     printf ("Timeout waiting for daq response\n");
     exit(1);
  }

  char rocIdStr[10];
  sprintf(rocIdStr,"%i",rol->pid);
  printf("Setting rocId to %s\n",rocIdStr);
  controlCmdSetCommand ( smem, CONTROL_CMD_TYPE_SET_CONFIG, "RocId",rocIdStr);
  if ( ! controlCmdGetResultTimeout(smem,NULL,10000) ) {
     printf ("Timeout waiting for daq response\n");
     exit(1);
  }

  /* load thresholds */
  if(strcmp(confFileThr,"")!=0) {
     printf("Load thresholds from %s\n",confFileThr);
     controlCmdSetCommand ( smem, CONTROL_CMD_TYPE_EXEC_COMMAND, "ReadXmlFile", confFileThr);
     if ( ! controlCmdGetResultTimeout(smem,NULL,10000) ) {
        printf ("Timeout waiting for daq response\n");
        exit(1);
     }
  } else {
     printf("WARNING: No thresholds being set!\n");
  }

  /* set calibration specific configuration */
  if(run_type==1) {
	printf("set charge injection specific configuration\n");
    // check  that cal group is == 0 here!
    if(cal_group!=0) {
       printf("Error: cal_group=%d in prestart. Should be 0!?\n",cal_group);
       exit(1);
    }
	char cal_cmd[256];
	sprintf(cal_cmd,"<system><config><CalEn>True</CalEn><CalInhibit>False</CalInhibit><FebCore><CalGroup>%d</CalGroup></FebCore></config></system>\f", cal_group);
	printf("cmd: %s\n",cal_cmd);
	controlCmdSetConfig ( smem,cal_cmd);
    if ( ! controlCmdGetResultTimeout(smem,NULL,10000) ) {
      printf ("Timeout waiting for daq response\n");
      exit(1);
    }
    // increment calibration group as it needs to start at 1 in the go loop.
	cal_group++;	
  }




  printf("Set state to prestart\n");
  controlCmdSetCommand ( smem, CONTROL_CMD_TYPE_EXEC_COMMAND, "SetRunState", "Prestart");
  if ( ! controlCmdGetResultTimeout(smem,NULL,10000) ) {
     printf ("Timeout waiting for daq response\n");
     exit(1);
  }

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

  CDODISABLE(DPM,1,0);

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
  CDODISABLE(DPM,1,0);

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

  printf("INFO: User Go 1 Enabling\n");
  CDOENABLE(DPM,1,1);
  printf("INFO: User Go 1 Enabled\n");

  // Send command to daq server
  printf("Set state to go\n");
  controlCmdSetCommand ( smem, CONTROL_CMD_TYPE_EXEC_COMMAND, "SetRunState", "Go");
  if ( ! controlCmdGetResultTimeout(smem,NULL,10000) ) {
     printf ("Timeout waiting for daq response\n");
     exit(1);
  }

  printf("INFO: User Go 1 Executed\n");

  lastTime  = 0;

  // Enable data

  return;
}

void
usrtrig(unsigned long EVTYPE, unsigned long EVSOURCE)
{
  int syncEventFlag;
  int len, ii;
  int nwords,nbytes;
  char* chptr;
  TIMERL_VAR;

  syncEventFlag = 0; // Pelle: hard-code for now
  rol->dabufp = (long *) 0;

TIMERL_START;

#ifdef MY_FRAG_HEADER
  CEOPEN(EVTYPE, BT_BANKS);
  printf("SOFTWARE: word0=%d, word1=0x%08x\n",*(rol->dabufp-2),*(rol->dabufp-1));
#else
  CEOPEN1;
#endif

  // Invalid buffer
  if(rol->dabufp == NULL) printf("ERROR rol->dabufp is NULL -- Event lost\n");
  else
  {

    int ret = axisRead(myfd,rol->dabufp,1024*1024,0,0,0);

	/*for(ii=0; ii<ret/4; ii++) printf("  DATA [%3d] 0x%08x (%d)\n",ii,rol->dabufp[ii],rol->dabufp[ii]);*/

    if(ret > 0) rol->dabufp += ret/4;
    lastSize = ret;
    nwords = ret/4;

	/*printf("lastSize=%d (nwords=%d)\n",lastSize,nwords);*/

    /* dump config to data stream */
    //Pelle: add in event number when this works
    if(syncEventFlag==1) { // || EVENT_NUMBER==1) {       
       BANKOPEN(0xe10E,3,rol->pid);
       chptr =(char *)rol->dabufp;
       nbytes = svtDaqReadConfig(chptr);
       nwords = nbytes/4;
       rol->dabufp += nwords;       
       BANKCLOSE;
    }

    //========
    // Calibration
    if(run_type==1 && (EVENT_NUMBER%100==0)) {

       printf("Processing cal_group %d at EVENT_NUMBER %d\n",cal_group,EVENT_NUMBER);

        // only 8 cal groups. Should stop before the 9th	
        if(cal_group<9) {

           // Send cal group command to daq server
           printf("Set calibration group to %d\n",cal_group);
           char cmd[256];
           sprintf(cmd,"<system><config><FebCore><CalGroup>%d</CalGroup></FebCore></config></system>\f", cal_group);
           printf("cal cmd %s\n",cmd);
           controlCmdSetConfig( smem, cmd);
           if ( ! controlCmdGetResultTimeout(smem,NULL,10000) ) {
              printf ("Timeout waiting for daq response\n");
              exit(1);
           }

           /* dump config to data stream */
           BANKOPEN(0xe10E,3,rol->pid);
           chptr =(char *)rol->dabufp;
           nbytes = svtDaqReadConfig(chptr);
           nwords = nbytes/4;
           rol->dabufp += nwords;       
           BANKCLOSE;
      
           /* update cal group */
           cal_group++;	
      
        } else {
           printf("Reached all cal groups. End run.\n");
           __end();
        } 
     }
    // End of Calibration
    //========





  }

#ifdef MY_FRAG_HEADER
  CECLOSE;
#else
  CECLOSE1;
#endif


TIMERL_STOP(10000,1000+rol->pid);

  ackTrig = 1;
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
  if (ackTrig) {
     axisReadAck(myfd); // Ack
     ackTrig =  0;
  }

  /* from parser */
  poolEmpty = 0; /* global Done, Buffers have been freed */

  return;
}
  
static void
__status()
{
  return;
}  


