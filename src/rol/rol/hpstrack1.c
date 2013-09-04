
/* hpstrack1.c */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifndef VXWORKS
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#endif

#include "circbuf.h"



#define ROL_NAME__ "HPSTRACKPRIMARY"

/*#define POLLING___*/
#define POLLING_MODE

#define INIT_NAME hpstrack1__init

#include "rol.h"

void usrtrig(unsigned long, unsigned long);
void usrtrig_done();

#include "HPSTRACKPRIMARY_source.h"

#include "coda.h"
#include "tt.h"

static char rcname[5];



static void
__download()
{
   rol->poll = 1;

  printf(">>>>>>>>>>>>>>>>>>>>>>>>>> ROCID = %d <<<<<<<<<<<<<<<<\n",rol->pid);

  printf("rol1: downloading DDL table ...\n");
  clonbanks_();
  printf("rol1: ... done.\n");

   // Open HP PCI Card
   close(myfd);
   myfd = open("/dev/hppci0",O_RDWR);
   if ( myfd < 0 ) printf("Failed to open HPPCI device\n");
   else printf ("Opened HPPCI device\n");

   // Open shared memory
   if ( controlCmdOpenAndMap(&cmem) < 0 ) printf("Failed to open control mmap\n");
   else printf("Opned control mmap\n");

   // For online viewer
   if ( onlineDataOpenAndMap(&dmem) < 0 ) printf("Failed to open control mmap\n");
   else printf("Opned control mmap\n");
   onlineDataInit(dmem);

   printf("Download Executed\n");

} /*end download */     

static void
__prestart()
{
   CTRIGINIT;
   *(rol->nevents) = 0;

   time_t  currTime;

   time(&currTime);
   printf("Prestart Entered at %s\n",ctime(&currTime));

   HPSTRACKPRIMARY_INIT;
   CTRIGRSS(HPSTRACKPRIMARY,1,usrtrig,usrtrig_done);

   rol->poll = 1;

   sprintf(rcname,"RC%02d",rol->pid);
   printf("rcname >%4.4s<\n",rcname);

   // Send prestart command to DAQ
   controlCmdSend (cmem,"<system><command><CodaPrestart/></command></system>\n");
   while ( controlCmdPending(cmem) ) usleep(1);

   printf("------------- Tracker DAQ State----------------------\n");
   printf("%s\n",controlStatBuffer(cmem));
   printf("-----------------------------------------------------\n");

   printf("Prestart Executed\n");

   *(rol->nevents) = 0;
   rol->recNb = 0;
} /*end prestart */     

static void
__end()
{
   uint    buffer[8192];
   HpPciRx hpRx;
   uint    y;
   int     ret;
   int     fcnt;
   time_t  currTime;

   time(&currTime);
   printf("End Entered at %s\n",ctime(&currTime));

   CDODISABLE(HPSTRACKPRIMARY,1,0);
 
   // Send end command to DAQ
   controlCmdSend (cmem,"<system><command><CodaEnd/></command></system>\n");
   while ( controlCmdPending(cmem) ) usleep(1);

   printf("------------- Tracker DAQ State----------------------\n");
   printf("%s\n",controlStatBuffer(cmem));
   printf("-----------------------------------------------------\n");
   sleep(1);

   hpRx.model      = sizeof(void *);
   hpRx.data       = buffer;
   hpRx.rxMax      = 8192;
   hpRx.debugLevel = 0;
   hpRx.rxLane     = 0;

   // Flush remaining data
   printf("Flushing buffers:\n");
   for (y=0; y < 8; y++) {
      fcnt = 0;
      hpRx.rxLane = y;
      while ( (ret=read(myfd,&hpRx,sizeof(hpRx))) > 0 ) fcnt += ret;
      printf("Fushed %i words from %i\n",fcnt,y);
   }
   printf("Done.\n");

   printf("End Executed\n");

} /* end end block */

static void
__pause()
{
   time_t  currTime;

   time(&currTime);
   printf("Pause Entered at %s\n",ctime(&currTime));

   CDODISABLE(HPSTRACKPRIMARY,1,0);

   // Send pause command to DAQ
   controlCmdSend (cmem,"<system><command><CodaPause/></command></system>\n");
   while ( controlCmdPending(cmem) ) usleep(1);

   printf("------------- Tracker DAQ State----------------------\n");
   printf("%s\n",controlStatBuffer(cmem));
   printf("-----------------------------------------------------\n");

   printf("Pause Executed\n");
  
} /*end pause */

static void
__go()
{
   uint    errorFlag;
   uint    cmd;
   uint    buffer[8192];
   HpPciRx hpRx;
   uint    y;
   int     ret;
   int     fcnt;
   time_t  currTime;

   time(&currTime);
   printf("Entering Go at %s\n",ctime(&currTime));

   enable   = 1;
   perTime  = 0;
   pollTime = 0;
   total    = 0;
   perTotal = 0;
   perSize  = 0;
   errorCnt = 0;

   // Setup descriptor
   hpRx.model      = sizeof(void *);
   hpRx.data       = buffer;
   hpRx.rxMax      = 8192;
   hpRx.debugLevel = 0;

   printf("Size=%i\n",sizeof(hpRx));

   // Flush previous data
   printf("Flushing buffers:\n");
   for (y=0; y < 8; y++) {
      fcnt = 0;
      hpRx.rxLane = y;
      while ( (ret=read(myfd,&hpRx,sizeof(hpRx))) > 0 ) fcnt += ret;
      printf("Fushed %i words from %i\n",fcnt,y);
   }
   printf("Done!\n");

   // Assert NULL commands to both cobs
   cmd = 0x00000080;
   write(myfd,&cmd,4);
   cmd = 0x00000480;
   write(myfd,&cmd,4);

   // Send go command to DAQ
   controlCmdSend (cmem,"<system><command><CodaGo/></command></system>\n");
   while ( controlCmdPending(cmem) ) usleep(1);

   printf("------------- Tracker DAQ State----------------------\n");
   printf("%s\n",controlStatBuffer(cmem));
   printf("-----------------------------------------------------\n");

   // Get user State
   sscanf(controlUserBuffer(cmem),"%i %i %i %i %i",&cardMask,&bufferDepth,&errorFlag,&intTrig,&liveDisplay);
   printf("DAQ Status: cardMask=0x%x, bufferDepth=0x%x, errorFlag=%i, intTrig=%i, liveDisplay=%i\n",cardMask,bufferDepth,errorFlag,intTrig,liveDisplay);

   if ( errorFlag ) {
      printf("\n");
      printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
      printf("A Tracker DAQ Error Has Occured. Refusing To Continue!!!!!\n");
      printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
      enable = 0;
   }

   CDOENABLE(HPSTRACKPRIMARY,1,0);
   time(&totTime);
   printf("Exiting Go\n");
}

void
__done()
{
   uint   cmd;
   uint   rate;
   uint   arate;
   uint   drate;

   // Assert hardware ack
   write(myfd,NULL,0);

   time(&currTime);
   if ( (currTime - perTime) > 5 )
   {
      rate   = perTotal / (currTime - perTime);
      if ( currTime == totTime ) arate = 0;
      else arate  = total / (currTime - totTime);
      drate  = (perSize*4) / (currTime - perTime);
      printf("Total=%i, Rate=%i Hz, Avg=%i Hz, Size=%i, Throughput=%i Bps, Errors=%i\n",
         total,rate,arate,lastSize,drate,errorCnt);

      perTime  = currTime;
      perTotal = 0;
      perSize  = 0;

      // Assert NULL commands to both cobs
      cmd = 0x00000080;
      write(myfd,&cmd,4);
      cmd = 0x00000480;
      write(myfd,&cmd,4);
   }

   pollCount = 0;


  /* from parser */
  poolEmpty = 0; /* global Done, Buffers have been freed */


} /*end done */

static void
__status()
{
  printf("Status called\n");
} /* end status */

/* This routine is automatically executed just before the shared libary
   is unloaded.

   Clean up memory that was allocated 
*/
__attribute__((destructor)) void end (void)
{
   printf("Roc is exiting. LastSize=%i, Total=%i, PollCount=%i\n",lastSize,total,pollCount);
   close(myfd);
}

void
usrtrig(unsigned long EVTYPE, unsigned long EVSOURCE)
{
  int len, blen, ind;
  uint *jw;
   uint     y;
   uint     toCount;
   HpPciRx  hpRx;
   uint     evtSize;
   int      ret;
   uint     idx;
   struct   sockaddr_in net_addr_;
   int      slen = sizeof(net_addr_);
   int      dataNetFd_;

   // Update counters
   perTotal++;
   total++;

   // Receive constants
   hpRx.rxMax      = 7770;
   hpRx.model      = sizeof(void *);
   hpRx.debugLevel = 0;

   /* Open event (trigType is defined if HPSTRACKPRIMARY_source.h) trigType */
  CEOPEN(EVTYPE, BT_BANKS);


  jw = rol->dabufp; 
  jw[-2] = 1;


   // Invalid buffer
   if(rol->dabufp == NULL) printf("ERROR rol->dabufp is NULL -- Event lost\n");
   else
   {

/* for EVIO format, will dump raw data */
if((ind = bosMopen_(jw, rcname, 0, 1, 0)) <=0)
{
  printf("bosMopen_ Error: %d\n",ind);
}
rol->dabufp += NHEAD;



/*remove
      // Copy TI data
      memcpy(rol->dabufp,tiData,40);
      rol->dabufp[1] = 0x00070101; // Tag = 7, UI32, Idx = 1
      idx = 2;

      rol->dabufp += 10;
      evtSize      = 10;
*/

      evtSize = 0;
      idx = 0; /*May 7 9:23am*/


	  /*commented out on Ryan's request May 1, 2012
      // Check sequence
      if ( tiData[2] != total ) {
         printf("Bad sequence number in Trigger. Got=%i, Exp=%i\n",tiData[2],total);
         enable = 0;
         errorCnt++;
      }
	  */

      // Get event data from data sources
      for (y=0; y < 7; y++) {

         // Card is enabled
         if ( cardMask & (1<<y) ) {

            // Setup data frame receive
            hpRx.data   = rol->dabufp; /* set our buffer pointer to the 1nd word of the data bank */
            hpRx.rxLane = y;

            // Get frame
            toCount = 0; 
            while ( (ret=read(myfd,&hpRx,sizeof(hpRx))) <= 0 ) {
               if ( toCount++ == 1000000 ) break;
            }

            // Valid length
            if ( ret > 0 && hpRx.eofe == 0 && hpRx.fifoErr == 0 && hpRx.lengthErr == 0 ) {
               perSize += hpRx.rxSize;

               // Send to live display
               if ( liveDisplay ) onlineDataPush(dmem,&(hpRx.data[1]),hpRx.rxSize-1);

               // Check sequence
               if ( rol->dabufp[2] != total ) {
                  printf("Bad sequence number from %i. Got=%i, Exp=%i\n",y,rol->dabufp[2],total);
                  enable = 0;
                  errorCnt++;
               }

               /* make 2nd word of the data bank */
               rol->dabufp[1]  = 0x00000100; // UI32
               rol->dabufp[1] |= (y << 16);  // Bank Tag (fpga address)
               rol->dabufp[1] |= idx;        // Bank Index ID

               idx++;

               // Update pointer 
               rol->dabufp   += hpRx.rxSize;
               evtSize       += hpRx.rxSize;
            }
            else {
               printf("Error waiting for data %i. total=%i, eofe=%i, fifoErr=%i, lengthErr=%i\n",
                       y,total,hpRx.eofe,hpRx.fifoErr,hpRx.lengthErr);
               errorCnt++;
            }
         }
      }



    blen = rol->dabufp - (int *)&jw[ind+1];
#ifdef DEBUG
    printf("rol1trig: len=%d\n",blen);
#endif
    if(blen == 0) /* no data - return pointer to the initial position */
    {
      rol->dabufp -= NHEAD;
	  /* NEED TO CLOSE BANK !!?? */
    }
    else if(blen >= (MAX_EVENT_LENGTH/4))
    {
      logMsg("1ERROR: event too long, blen=%d, ind=%d\n",blen,ind,0,0,0,0);
    }
    else if(bosMclose_(jw,ind,1,blen) <= 0)
    {
      logMsg("2ERROR in bosMclose_ - space is not enough !!!\n",1,2,3,4,5,6);
    }





   }

   // Update event size at previously stored pointer
   lastSize = evtSize;

   // Close event
   CECLOSE;

} /*end trigger */

void
usrtrig_done()
{

} /*end done */


