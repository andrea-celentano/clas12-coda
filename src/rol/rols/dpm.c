
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
#include "roldpm.h"

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
char confFileFeb[SVTDAQMAXSTRLEN];
char confFileThr[SVTDAQMAXSTRLEN];

//#define DUMP_TIMER_BANK
//#define DUMP_TRIG_COUNT

/************************/
/************************/
/*
#include "coda.h"
#include "tt.h"
*/
static char rcname[5];

#define ABS(x)      ((x) < 0 ? -(x) : (x))

#ifdef DUMP_TIMER_BANK 

#define TIMERL_VAR \
  static hrtime_t startTim, stopTim, dTim; \
  static int nTim; \
  static int whentoprint_time=20000; \
  static hrtime_t Tim, rmsTim, minTim=10000000, maxTim, normTim=1, calc_aveTim, calc_stddevTim, calc_minTim, calc_maxTim

#define TIMERL_START \
{ \
  startTim = gethrtime(); \
  /*logMsg("timer start called: startTim %7llu\n",startTim);*/	\
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

#define TIMERL_CALC \
{ \
    if(nTim == whentoprint_time) \
    { \
      calc_aveTim = Tim/nTim/normTim; \
      calc_stddevTim = ABS(rmsTim/nTim-Tim*Tim/nTim/nTim)/normTim/normTim; \
      calc_minTim = minTim/normTim; \
      calc_maxTim = maxTim/normTim; \
    } \
}

#define TIMERL_PRINT \
{ \
    if(nTim == whentoprint_time) \
    { \
      logMsg("timer print: %7llu microsec (min=%7llu max=%7llu rms**2=%7llu)\n", \
	     calc_aveTim,calc_minTim,calc_maxTim,calc_stddevTim); \
    } \
}

#define TIMERL_RESET \
{ \
  /*logMsg("reset timer called with nTim %d\n", nTim);*/	\
    if(nTim == whentoprint_time) \
    { \
      /*logMsg("reset timer\n");*/		\
       nTim = 0; \
       Tim = calc_aveTim = calc_stddevTim = calc_minTim = calc_maxTim = 0.0; \
    } \
    /*else {*/								\
    /*  logMsg("dont reset timer %d vs %d\n", nTim, whentoprint_time);*/ \
    /*}*/								\
}

#define TIMERL_STOP_ONLY \
{ \
  stopTim = gethrtime(); \
  /*logMsg("timer stop called: startTime %7llu stopTime %7llu\n",startTim,stopTim);*/ \
  if(stopTim >= startTim) \
  { \
    nTim++; \
    dTim = stopTim - startTim; \
    Tim += dTim; \
    rmsTim += dTim*dTim; \
    minTim = minTim < dTim ? minTim : dTim; \
    maxTim = maxTim > dTim ? maxTim : dTim; \
    /*logMsg("timer stop called: nTim %d dTim %7llu Tim %7llu\n",nTim,dTim,Tim);*/ \
  } \
  else {								\
    logMsg("timer stop called: START IS AFTER STOP?? startTime %7llu stopTime %7llu\n",startTim,stopTim); \
  } \
}

/*
#define TIMERL_DUMP(roc_id, dabuf_ptr, lastev_ptr)		\
{ \
  if(nTim == whentoprint_time) \
  { \
    long* b = (long *) (dabuf_ptr);				    \
    logMsg("dump roc_id %d timer to data stream b %p\n",(roc_id), b);	\
    BANKOPEN(0xe10E,3,(roc_id));						\
    logMsg("first word adress dabuf_ptr %p\n",b); \
    *b = 0xdeadbeef; \
    logMsg("value 0x%x\n",*b);	\
    b++;					\
    BANKCLOSE;					 \
    lastev_ptr[0] = rol->dabufp - pLastEv - 1; 
    logMsg("done dumping timer to data stream (dabuf_ptr %p)\n",b);	\
  } \
}
*/

#endif


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



// Get the xml string from the DAQ
int svtDaqReadConfig(char* str, const char* type) {
   int len;
   const char* configStr;
   printf("svtDaqReadConfig: called.\n");
   if(strcmp(type,"config")==0) {
      printf("svtDaqReadConfig: read config\n",len);
      configStr = controlCmdGetConfig( smem );
   } else if (strcmp(type,"status")==0) {
      printf("svtDaqReadConfig: read status\n",len);
      configStr = controlCmdGetStatus( smem );
   } else {
      printf("svtDaqReadConfig: this type is not allowed\n");
      exit(1);
   }
   if(configStr==NULL) {
      printf("Error getting string from control server!\n");
      exit(1);
   }
   // get the length not null character
   len = strlen(configStr);
   //printf("svtDaqReadConfig: got string at %p of strlen=%d.\n",configStr,len);
   //printf("\"%s\"\n",configStr);
   strncpy(str,configStr,len);
   printf("svtDaqReadConfig: copied string to %p with len=%d.\n",str,len);

   return len;
}


int svtDaqUploadCompact(char* chptr, int group, int level, int delay) {
   printf("svtDaqUploadCompact called with chptr=%p.\n",chptr);

   int nbytes;
   int len;
   char tmp[50];
   nbytes = 0;
          
   /* add one 'return' to make evio2xml output nicer */
   *chptr++ = '\n';
   nbytes++;
          
   /* read the actual string from the daq */
   /* this do not include null character */
   sprintf(tmp,"cal_group_%d cal_level_%d cal_delay_%d",group,level,delay);
   len = strlen(tmp);
   strncpy(chptr,tmp,len);
   printf("cfg strlen=%d\n",len);
   printf("\"%s\"\n",tmp);

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
       


// Get the xml configuration string from the DAQ          
int svtDaqUploadAll(char* chptr, const char* type) {
   printf("svtDaqUploadAll called with chptr=%p.\n",chptr);

   int nbytes;
   int len;
   nbytes = 0;
          
   /* add one 'return' to make evio2xml output nicer */
   *chptr++ = '\n';
   nbytes++;
          
   /* read the actual string from the daq */
   /* this do not include null character */
   len = svtDaqReadConfig(chptr, type);
   //printf("cfg strlen=%d\n",len);
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
       


int getSvtSyncFlag(int* startOfSvtBank, int nw, int** pLastEv) {
   int debug_local = 0;
   if(debug_local!=0) printf("getSvtSyncFlag : startOfSvtBank %p\n", startOfSvtBank);
   int* pCurEv;
   int* pLastEvLoop;
   int nEv;
   int len;
   int evNumber;
   int evType;
   int syncFlag;
   int syncFlagEv;
   syncFlagEv = 0;
   pCurEv = startOfSvtBank;
   nEv = 0;
   while(nEv < nw) {
      len = pCurEv[0];
      if( len > 0 ) {
         evNumber = pCurEv[1] & 0xff;
         evType = (pCurEv[1] >> 16) & 0xff;
         syncFlag = (pCurEv[1] >> 24);
         if(syncFlag!=0) {
            if(debug_local!=0) {
               printf("getSvtSyncFlag : found sync flag on pCurEv %p (startOfSvtBank at %p)\n", pCurEv, startOfSvtBank);
               //printf("getSvtSyncFlag :pCurEv[0] 0x%x pCurEv[1] 0x%x evNumber %d (0x%x) evType %d syncFlag %d\n",pCurEv[0], pCurEv[1], evNumber, evNumber, evType, syncFlag);
            }
            syncFlagEv = syncFlagEv + 1;
         }

         pLastEvLoop = pCurEv;
         pCurEv += (len+1);


      }else if(len < 0) {
         printf("getSvtSyncFlag : ERROR: len = %d\n",len);	
         exit(1);
      }                                                             
      else {                                                                 
         printf("getSvtSyncFlag : SHOULD NEVER BE HERE: len = %d\n",len);
         exit(1);
      }
      
      nEv += (len+1);

   } //nw
   
   *pLastEv = pLastEvLoop;

   return syncFlagEv;

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

  char confFile[SVTDAQMAXSTRLEN];
  
  getDpmConfigFilePath(rol->confFile, confFileFeb, SVTDAQMAXSTRLEN);

  // Extract the file path to the xml config file
//  getFebConfigFilePath(confFile,"CONFIG",confFileFeb,SVTDAQMAXSTRLEN);
  if(strlen(confFileFeb)>0) {
     printf("Got FEB config file: %s\n",confFileFeb);
  } else {
     printf("No FEB config file extracted!?\n");        
     exit(1);
  }

  // Find the threshold xml file
  getDpmThresholdFilePath(rol->confFile, confFileThr, SVTDAQMAXSTRLEN);
  //getFebConfigFilePath(confFile,"THRESHOLDS",confFileThr,SVTDAQMAXSTRLEN);
  if(strlen(confFileThr)>0) {
     printf("Got threhold config file: %s\n",confFileThr);
  } else {
     printf("WARNING WARNING -> No threshold config file extracted!?\n");        
     //exit(1);
  }

  /* Flag if this is a charge injection run */
  run_type = getRunType(rol->confFile);
  if (run_type == 0) printf("Normal run type\n");
  else if (run_type == 1) printf("Gain calibration RUN\n");
  else if (run_type == 2) printf("t0 calibration RUN\n");
  else if (run_type == 3) printf("t0 calibration RUN over single calibration group\n");
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
  char rocIdStr[10];


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
  ackCount  = 0;

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


  sprintf(rocIdStr,"%i",rol->pid);
  printf("Setting rocId to %s\n",rocIdStr);
  controlCmdSetCommand ( smem, CONTROL_CMD_TYPE_EXEC_COMMAND, "SetRocId",rocIdStr);
  //controlCmdSetCommand ( smem, CONTROL_CMD_TYPE_SET_CONFIG, "RocId",rocIdStr);
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


     // enable threshold cut
     printf("Enable threshold cut\n");
     controlCmdSetCommand ( smem, CONTROL_CMD_TYPE_EXEC_COMMAND, "SetThrCutEn","1");
     if ( ! controlCmdGetResultTimeout(smem,NULL,10000) ) {
        printf ("Timeout waiting for daq response\n");
        exit(1);
     }
     

  } else {
     printf("WARNING: No thresholds being set!\n");

     // enable threshold cut
     printf("Disable threshold cut\n");
     controlCmdSetCommand ( smem, CONTROL_CMD_TYPE_EXEC_COMMAND, "SetThrCutEn","0");
     if ( ! controlCmdGetResultTimeout(smem,NULL,10000) ) {
        printf ("Timeout waiting for daq response\n");
        exit(1);
     }
     
  }

  /* set calibration specific configuration */
  if(run_type==1 || run_type==2 || run_type==3) {
	printf("set charge injection specific configuration\n");
    // reset calib group
    cal_group=0;
    cal_delay=1;
    cal_level=0;

    strcpy(rocIdStr,"True");
    printf("Setting CalibMode to %s\n",rocIdStr);
    controlCmdSetCommand ( smem, CONTROL_CMD_TYPE_EXEC_COMMAND, "SetCalibMode",rocIdStr);
    if ( ! controlCmdGetResultTimeout(smem,NULL,10000) ) {
       printf ("Timeout waiting for daq response\n");
       exit(1);
    }

    sprintf(rocIdStr,"%i",cal_group);
    printf("Setting calibration group to %s\n",rocIdStr);
    controlCmdSetCommand ( smem, CONTROL_CMD_TYPE_EXEC_COMMAND, "SetCalibGroup",rocIdStr);
    if ( ! controlCmdGetResultTimeout(smem,NULL,10000) ) {
       printf ("Timeout waiting for daq response\n");
       exit(1);
    }

    // increment to start at the right place in the usrtrig loop
    cal_level++;
	cal_group++;	

    if(run_type==2 || run_type==3) {
       //reset delay
       sprintf(rocIdStr,"%i",cal_delay);
       printf("Setting CalDelay to %s\n",rocIdStr);
       controlCmdSetCommand ( smem, CONTROL_CMD_TYPE_EXEC_COMMAND, "SetCalibDelay",rocIdStr);
       if ( ! controlCmdGetResultTimeout(smem,NULL,10000) ) {
          printf ("Timeout waiting for daq response\n");
          exit(1);
        }
       cal_delay++;
    }
    
    
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

  printf("[ end() ]: trigCount %ld ackCount %ld\n",trigCount, ackCount);
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
  char rocIdStr[10];
  int* startOfSvtBank;
  int* pLastEv;
  int usrTrigDebug = 0;
#ifdef DUMP_TIMER_BANK 
  TIMERL_VAR;
#endif
  syncEventFlag = 0; 
  rol->dabufp = (long *) 0;

#ifdef MY_FRAG_HEADER
  CEOPEN(EVTYPE, BT_BANKS);
  printf("SOFTWARE: word0=%d, word1=0x%08x\n",*(rol->dabufp-2),*(rol->dabufp-1));
#else
  CEOPEN1;
#endif

  // Timer start
  // Note that this timer do not include CECLOSE 
#ifdef DUMP_TIMER_BANK
  TIMERL_START;
#endif

  // Invalid buffer
  if(rol->dabufp == NULL) printf("ERROR rol->dabufp is NULL -- Event lost\n");
  else
  {

     startOfSvtBank = rol->dabufp;

     //if(usrTrigDebug>0) printf("nwords=%d dabufp=%p before dma \n",nwords, rol->dabufp);

    int ret = axisRead(myfd,rol->dabufp,1024*1024,0,0,0);

	/*for(ii=0; ii<ret/4; ii++) printf("  DATA [%3d] 0x%08x (%d)\n",ii,rol->dabufp[ii],rol->dabufp[ii]);*/

    if(ret > 0) rol->dabufp += ret/4;
    lastSize = ret;
    nwords = ret/4;

#ifdef DUMP_TRIG_COUNT
    printf("[ usrtrig() ]: trigCount %ld ackCount %ld nwords=%d dabufp=%p after dma \n", trigCount, ackCount, nwords, rol->dabufp);
#endif
    //if(usrTrigDebug>0)  printf("nwords=%d dabufp=%p after dma \n",nwords, rol->dabufp);

    /* Grab sync flag from the SVT data and find the last event in the block*/	
    pLastEv = NULL;
    syncEventFlag = getSvtSyncFlag(startOfSvtBank, nwords,&pLastEv);


    /* dump config to data stream */
    if(syncEventFlag>0 && EVENT_NUMBER%50000==0 && run_type!=1 && run_type!=2 && run_type!=3 && (rol->pid==dpmWithConfigDump|| rol->pid==controlDpmRocId)) {       
      //if(syncEventFlag>0 && run_type!=1 && run_type!=2 && run_type!=3 && (rol->pid==dpmWithConfigDump|| rol->pid==controlDpmRocId)) {       
       printf("open config bank\n");
       BANKOPEN(0xe10E,3,rol->pid);
       chptr =(char *)rol->dabufp;
       nbytes = svtDaqUploadAll(chptr, "config");
       nwords += (nbytes+4-1)/4;
       rol->dabufp += (nbytes+4-1)/4;       
       printf("close config bank\n");
       BANKCLOSE;
       pLastEv[0] = rol->dabufp - pLastEv - 1; 

       /* dump status to data stream */
       printf("open status bank\n");
       BANKOPEN(0xe10E,3,rol->pid);
       chptr =(char *)rol->dabufp;
       nbytes = svtDaqUploadAll(chptr, "status");
       nwords += (nbytes+4-1)/4;
       rol->dabufp += (nbytes+4-1)/4;       
       printf("close status bank\n");
       BANKCLOSE;
       pLastEv[0] = rol->dabufp - pLastEv - 1; 
    }


    //========
    // Calibration
    if( run_type==1 || run_type==2 || run_type==3) {
       
       // Switch on config dump for the first event
       if( EVENT_NUMBER==1) {              
          writeConfig = 1;
       } 
       //writeConfig = 0;
       // Write config to data stream for every ROC
       //if(writeConfig==1 && (rol->pid==dpmWithConfigDump || rol->pid==controlDpmRocId)) {
       if(writeConfig==1) {
          /* dump config to data stream */          
          printf("open config bank\n");
          BANKOPEN(0xe10E,3,rol->pid);
          printf("open\n");
          chptr =(char *)rol->dabufp;
          nbytes =svtDaqUploadCompact(chptr,cal_group,cal_level,cal_delay);
          nwords += (nbytes+4-1)/4; 
          rol->dabufp += (nbytes+4-1)/4;                 
          printf("close config bank\n");
          BANKCLOSE;
          pLastEv[0] = rol->dabufp - pLastEv - 1; 
          printf("nbytes %d nw %d\n",nbytes,(nbytes+4-1)/4);
          printf("closed\n");
       }
       
       
       if( EVENT_NUMBER%100==0 ) {

          // make sure the config gets written on the next event
          writeConfig = 1;
          
          printf("Processing cal_group %d at EVENT_NUMBER %d\n",cal_group,EVENT_NUMBER);
          
          // only 8 cal groups. Should stop before the 9th	
          // one run_type only uses one group
          if(cal_group<8 && run_type!=3) {
             sprintf(rocIdStr,"%i",cal_group);
             printf("Setting calibration group to %s\n",rocIdStr);
             controlCmdSetCommand ( smem, CONTROL_CMD_TYPE_EXEC_COMMAND, "SetCalibGroup",rocIdStr);
             if ( ! controlCmdGetResultTimeout(smem,NULL,10000) ) {
                printf ("Timeout waiting for daq response\n");
                exit(1);
             }
             
             /* update cal group */
             cal_group++;	
             
          } else {
             
             // start all over again for different delay
             if(run_type==3 && cal_delay<9) {              
                sprintf(rocIdStr,"%i",cal_delay);
                printf("Setting CalDelay to %s\n",rocIdStr);
                controlCmdSetCommand ( smem, CONTROL_CMD_TYPE_EXEC_COMMAND, "SetCalibDelay",rocIdStr);
                if ( ! controlCmdGetResultTimeout(smem,NULL,10000) ) {
                   printf ("Timeout waiting for daq response\n");
                   exit(1);
                }              
                
                cal_delay++;
                
                //do not update calgroup for this run type
                
             } 
             // start all over again for different delay
             else if(run_type==2 && cal_delay<9) {              
                sprintf(rocIdStr,"%i",cal_delay);                
                printf("Setting CalDelay to %s\n",rocIdStr);
                controlCmdSetCommand ( smem, CONTROL_CMD_TYPE_EXEC_COMMAND, "SetCalibDelay",rocIdStr);
                if ( ! controlCmdGetResultTimeout(smem,NULL,10000) ) {
                   printf ("Timeout waiting for daq response\n");
                   exit(1);
                }              
                
                
                
                cal_delay++;
                
                //reset cal_group
                cal_group = 0;
                sprintf(rocIdStr,"%i",cal_group);
                printf("Setting calibration group to %s\n",rocIdStr);
                controlCmdSetCommand ( smem, CONTROL_CMD_TYPE_EXEC_COMMAND, "SetCalibGroup",rocIdStr);
                if ( ! controlCmdGetResultTimeout(smem,NULL,10000) ) {
                   printf ("Timeout waiting for daq response\n");
                   exit(1);
                }
                
                //increment for next loop
                cal_group++;
                
             } 
             // start all over again with a different charge level
             else if(run_type==1 && cal_level<5) {
                
                sprintf(rocIdStr,"%i",getCalLevel(cal_level));
                printf("Setting CalLevel to %s\n",rocIdStr);
                controlCmdSetCommand ( smem, CONTROL_CMD_TYPE_EXEC_COMMAND, "SetCalibLevel",rocIdStr);
                if ( ! controlCmdGetResultTimeout(smem,NULL,10000) ) {
                   printf ("Timeout waiting for daq response\n");
                   exit(1);
                }              
                
                cal_level++;
                
                //reset cal_group
                cal_group = 0;
                sprintf(rocIdStr,"%i",cal_group);
                printf("Setting calibration group to %s\n",rocIdStr);
                controlCmdSetCommand ( smem, CONTROL_CMD_TYPE_EXEC_COMMAND, "SetCalibGroup",rocIdStr);
                if ( ! controlCmdGetResultTimeout(smem,NULL,10000) ) {
                   printf ("Timeout waiting for daq response\n");
                   exit(1);
                }
                
                //increment for next loop
                cal_group++;
                
                
             } else {
                printf("Reached all cal groups. End run.\n");
                __end();
             }
          } 
          
       } else {
          writeConfig = 0;
       }

       
    }
    // End of Calibration
    //========




  }

#ifdef DUMP_TIMER_BANK 

  // === TIMER START===
  // Stop timer 
  TIMERL_STOP_ONLY;
  // Calculate averate and RMS
  TIMERL_CALC;
  // Print to log
  TIMERL_PRINT;
  // Dump timer to data stream 
  // Don't do this in a macro...
  //TIMERL_DUMP(rol->pid,&rol->dabufp, &pLastEv);  
  if( nTim == whentoprint_time) {
    logMsg("call dump below (dabufp %p nwords %d %d)\n", rol->dabufp,nwords,(nwords&0xFFFFFFFF));
    BANKOPEN(0xe11E,1,rol->pid);
    *(rol->dabufp) = nwords & 0xFFFFFFFF;
    rol->dabufp++;
    *(rol->dabufp) = calc_aveTim & 0xFFFFFFFF;
    rol->dabufp++;
    *(rol->dabufp) = calc_stddevTim & 0xFFFFFFFF;
    rol->dabufp++;
    *(rol->dabufp) = calc_minTim & 0xFFFFFFFF;
    rol->dabufp++;
    *(rol->dabufp) = calc_maxTim & 0xFFFFFFFF;
    rol->dabufp++;
    BANKCLOSE;					 
    pLastEv[0] = rol->dabufp - pLastEv - 1; 
    logMsg("call dump done (dabufp %p)\n", rol->dabufp);
  }

  // Reset timer
  TIMERL_RESET;
  // === TIMER EMD ===
#endif    

#ifdef MY_FRAG_HEADER
  CECLOSE;
#else
  CECLOSE1;
#endif

  //TIMERL_STOP(10000,1000+rol->pid);


  ackTrig = 1;
  return;
}
  
void
usrtrig_done()
{

#ifdef DUMP_TRIG_COUNT
  printf("[ usrtrig_done() ]: trigCount %ld ackCount %ld \n",trigCount, ackCount);
#endif

  return;
}  

void
__done()
{
  if (ackTrig) {
     axisReadAck(myfd); // Ack
     ackTrig =  0;
     ackCount++;

#ifdef DUMP_TRIG_COUNT
     printf("[ done() ]: ackTrig=1 trigCount %ld ackCount %ld \n",trigCount, ackCount);
#endif

  } else {
#ifdef DUMP_TRIG_COUNT
    printf("[ done() ]: ackTrig=0 trigCount %ld ackCount %ld \n",trigCount, ackCount);
#endif
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


