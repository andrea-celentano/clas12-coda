
/* fadc2.c - second readout list for VXS crates with FADC250 boards */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifndef VXWORKS
#include <sys/types.h>
#include <time.h>
#endif

#include "circbuf.h"

#define ROL_NAME__ "FADC2"
#define INIT_NAME fadc2__init

#define POLLING_MODE
#define EVENT_MODE

#include "rol.h"
#include "EVENT_source.h"

/************************/
/************************/

#include "coda.h"
#include "tt.h"

#define MYNEV 21000

int mynev; /*defined in tttrans.c */

/* user routines */

void rol2trig(int a, int b);
void rol2trig_done();

static void
__download()
{
  rol->poll = 1;

  return;
}

static void
__prestart()
{
  printf("INFO: Entering Prestart ROL2\n");

  /* Clear some global variables etc for a clean start */
  CTRIGINIT;

  /* init trig source EVENT */
  EVENT_INIT;

  /* Register a sync trigger source (up to 32 sources) */
  CTRIGRSS(EVENT, 1, rol2trig, rol2trig_done); /* second arg=1 - what is that ? */

  rol->poll = 1;

  rol->recNb = 0;

  printf("INFO: Prestart ROL22 executed\n");

  return;
}

static void
__end()
{
  printf("INFO: User End 2 Executed\n");

  return;
}

static void
__pause()
{
  printf("INFO: User Pause 2 Executed\n");

  return;
}

static void
__go()
{
  printf("User Go 2 Reached\n");fflush(stdout);

  mynev = 0;

  printf("INFO: User Go 2 Executed\n");fflush(stdout);

  return;
}



#define CCOPEN(btag,fmt,bnum) \
  /*if it is first board, open bank*/ \
  if(a_slot_old==-1) \
  { \
    { \
      int len1, n1; \
      char *ch; \
      len1 = strlen(fmt); /* format length in bytes */ \
      n1 = (len1+5)/4; /* format length in words */ \
      dataout_save1 = dataout ++; /*remember '0xf' bank length location*/ \
      *dataout++ = (btag<<16) + (0xf<<8) + bnum; /*bank header*/ \
      /* tagsegment header following by format */ \
      *dataout++ = (len1<<20) + (0x6<<16) + n1; \
      ch = (char *)dataout; \
      strncpy(ch,fmt,len1); \
      ch[len1]='\0';ch[len1+1]='\4';ch[len1+2]='\4';ch[len1+3]='\4';ch[len1+4]='\4'; \
      dataout += n1; \
      /* 'internal' bank header */ \
      dataout_save2 = dataout ++;  /*remember 'internal' bank length location*/ \
      *dataout++ = (0<<16) + (0x0<<8) + 0; \
    } \
    b08 = (unsigned char *)dataout; \
  } \
  /*if new slot, write stuff*/ \
  if(a_slot != a_slot_old) \
  { \
    a_channel_old = -1; /*for new slot, reset a_channel_old to -1*/ \
    a_slot_old = a_slot; \
    *b08++ = a_slot; \
    b32 = (unsigned int *)b08; \
    *b32 = a_triggernumber; \
    b08 += 4; \
    b64 = (unsigned long long *)b08; \
    *b64 = (((unsigned long long)a_trigtime[0])<<24) | a_trigtime[1];	\
    b08 += 8; \
    /*set pointer for the number of channels*/ \
    Nchan = (unsigned int *)b08; \
    Nchan[0] = 0; \
    b08 += 4; \
  }


#define CCCLOSE \
{ \
  unsigned int padding; \
  dataout = (unsigned int *) ( ( ((unsigned int)b08+3)/4 ) * 4); \
  padding = (unsigned int)dataout - (unsigned int)b08; \
  /*dataout_save1[1] |= (padding&0x3)<<14;*/ \
  dataout_save2[1] |= (padding&0x3)<<14; \
  /*printf("CCCLOSE: 0x%x %d --- 0x%x %d --> padding %d\n",dataout,dataout,b08,b08,((dataout_save2[1])>>14)&0x3);*/ \
  *dataout_save1 = (dataout-dataout_save1-1); \
  *dataout_save2 = (dataout-dataout_save2-1); \
  lenout += (*dataout_save1+1); \
  b08 = NULL; \
}




#undef HAVE_SSP_DATA
#define NSSP 8
#define DO_PEDS
#undef DEBUG

/* 22-max# of blocks=slots, 256-max# of events*/
#define MAXBLOCK 22
#define MAXEVENT 256

#ifdef DO_PEDS
#define NPEDEVENTS 100
#define NSLOTS 22
#define NCHANS 16
static int npeds[NSLOTS][NCHANS];
static float pedval[NSLOTS][NCHANS];
static float pedrms[NSLOTS][NCHANS];
static FILE *fd = NULL;
static char *dir = NULL;
static char *expid = NULL;
#endif

void
rol2trig(int a, int b)
{
  CPINIT;
  int nB, iB[MAXBLOCK], sB[MAXBLOCK];
  int nE[MAXBLOCK], iE[MAXBLOCK][MAXEVENT], lenE[MAXBLOCK][MAXEVENT];
  unsigned int *iw, *ssp;
  int i, j, k, m, iii, ind, bank, nhits=0, mux_index, rlen, printing, nnE;
  int nr = 0;
  int ncol = 2;
  int a_channel, a_nevents, a_blocknumber, a_triggernumber;
  int a_windowwidth, a_pulsenumber, a_firstsample, samplecount;
  int a_adc1, a_adc2, a_valid1, a_valid2, a_nwords, a_slot, a_slot2;
  int a_qualityfactor, a_pulseintegral, a_pulsetime, a_vm, a_vp;
  int a_trigtime[4];
  int a_slot_old;
  int a_channel_old;
  int npedsamples;
  int error;
  unsigned int *StartOfBank;
  char *ch;
  unsigned int *Nchan, *Npuls, *Nsamp;
  int islot, ichan, ii, jj, npulses;
  int banknum = 0;
#ifdef DO_PEDS
  char fname[1024];
#endif

  mynev ++; /* needed by ttfa.c */

  /*
  CPOPEN(rol->pid,1,0);
  for(ii=0; ii<lenin; ii++)
  {
    dataout[ii] = datain[ii];
    b08 += 4;
  }
  CPCLOSE;

  CPOPEN(rol->pid,1,1);
  for(ii=0; ii<lenin; ii++)
  {
    dataout[ii] = datain[ii]+0x100;
    b08 += 4;
  }
  CPCLOSE;
  */

  banknum=rol->pid;
  /*
  if(rol->pid==6) banknum=0; ??? - temporary
  else if(rol->pid==27) banknum=10; ftof0 - temporary
  else
  {
    banknum=rol->pid;
  }
  */


#ifdef DEBUG
  printf("\n**********************\nmini-bos bank: lenin=%d\n",lenin);
#endif


#ifndef VXWORKS
  /* swap input buffer (assume that data from VME is big-endian, and we are on little-endian Intel) */
  for(ii=0; ii<lenin; ii++) datain[ii] = LSWAP(datain[ii]);
#endif

#ifdef HAVE_SSP_DATA
  /*assume first NSSP words are ssp fifo*/
  if(rol->pid==1)
  {
    ssp = datain;
    for(ii=0; ii<NSSP; ii++) ssp[ii] = LSWAP(ssp[ii]); /*swap it back, it came in correct endian from rol1*/
    datain += NSSP;
    lenin -= NSSP;
  }
#endif


  error = 0; /* reset error flag */

/*******************************************************/
/* FIRST PASS: check data; fill nB, iB[22], nE and iE[22][256] */

#ifdef DEBUG
  printf("\nFIRST PASS\n\n");
#endif

  ii=0;
  printing=1;
  nB=0; /*cleanup block counter*/
  while(ii<lenin)
  {

    if( ((datain[ii]>>27)&0x1F) == 0x10) /*block header*/
    {
      a_slot = ((datain[ii]>>22)&0x1F);
      a_nevents = ((datain[ii]>>11)&0x3FF);
      a_blocknumber = (datain[ii]&0x3FF);
#ifdef DEBUG
	  printf("[%3d] BLOCK HEADER: slot %d, nevents %d, block number %d\n",ii,
				   a_slot,a_nevents,a_blocknumber);
      printf(">>> update iB and nB\n");
#endif
      nB++;            /*increment block counter*/
      iB[nB-1]=ii;     /*remember block start index*/
      sB[nB-1]=a_slot; /*remember slot number*/
      nE[nB-1]=0;      /*cleanup event counter in current block*/

	  ii++;
    }
    else if( ((datain[ii]>>27)&0x1F) == 0x11) /*block trailer*/
    {
      a_slot2 = ((datain[ii]>>22)&0x1F);
      a_nwords = (datain[ii]&0x3FFFFF);
#ifdef DEBUG
	  printf("[%3d] BLOCK TRAILER: slot %d, nwords %d\n",ii,
				   a_slot2,a_nwords);
      printf(">>> data check\n");
#endif

      /*"close" previous event if any*/
      k = nB-1; /*current block index*/
      if(nE[k] > 0)
	  {
        m = nE[k]-1; /*current event number*/
        lenE[k][m] = ii-iE[k][m]; /*#words in current event*/
	  }     

      if(a_slot2 != a_slot)
	  {
        error ++;
        if(printing)
        {
          printf("[%3d] ERROR1 in FADC data: blockheader slot %d != blocktrailer slot %d\n",
				 ii,a_slot,a_slot2);
          printing=0;
	    }
	  }
      if(a_nwords != (ii-iB[nB-1]+1))
      {
        error ++;
        if(printing)
        {
          printf("[%3d] ERROR2 in FADC data: trailer #words %d != actual #words %d\n",
				 ii,a_nwords,ii-iB[nB-1]+1);
          printing=0;
	    }
      }
	  ii++;
    }
    else if( ((datain[ii]>>27)&0x1F) == 0x12) /*event header*/
    {
      a_triggernumber = (datain[ii]&0x7FFFFFF);
#ifdef DEBUG
	  printf("[%3d] EVENT HEADER: trigger number %d\n",ii,
				   a_triggernumber);
      printf(">>> update iE and nE\n");
#endif

      /*"close" previous event if any*/
      k = nB-1; /*current block index*/
      if(nE[k] > 0)
	  {
        m = nE[k]-1; /*current event number*/
        lenE[k][m] = ii-iE[k][m]; /*#words in current event*/
	  }     

      /*"open" next event*/
      nE[k]++; /*increment event counter in current block*/
      m = nE[k]-1; /*current event number*/
      iE[k][m]=ii; /*remember event start index*/

	  ii++;
    }
    else if( ((datain[ii]>>27)&0x1F) == 0x13) /*trigger time: remember timestamp*/
    {
      a_trigtime[0] = (datain[ii]&0xFFFFFF);
#ifdef DEBUG
	  printf("[%3d] TRIGGER TIME: 0x%06x\n",ii,
			   a_trigtime[0]);
#endif
	  ii++;
      iii=1;
      while( ((datain[ii]>>31)&0x1) == 0 && ii<lenin ) /*must be one more word*/
	  {
        a_trigtime[iii] = (datain[ii]&0xFFFFFF);
#ifdef DEBUG
        printf("   [%3d] TRIGGER TIME: 0x%06x\n",ii,
          a_trigtime[iii]);
        printf(">>> remember timestamp 0x%06x 0x%06x\n",a_trigtime[0],a_trigtime[1]);
#endif
        iii++;
        ii++;
	  }
    }
    else if( ((datain[ii]>>27)&0x1F) == 0x14) /*window raw data: remember channel#*/
    {
      a_channel = ((datain[ii]>>23)&0xF);
      a_windowwidth = (datain[ii]&0xFFF);	  
#ifdef DEBUG
	  printf("[%3d] WINDOW RAW DATA: slot %d, channel %d, window width %d\n",ii,
			 a_slot,a_channel,a_windowwidth);
      printf(">>> remember channel %d\n",a_channel);
#endif

	  ii++;
      samplecount = 0;
      while( ((datain[ii]>>31)&0x1) == 0 && ii<lenin ) /*loop over all samples*/
	  {
        a_valid1 = ((datain[ii]>>29)&0x1);
        a_adc1 = ((datain[ii]>>16)&0xFFF);
        a_valid2 = ((datain[ii]>>13)&0x1);
        a_adc2 = (datain[ii]&0xFFF);
#ifdef DEBUG
        printf("   [%3d] WINDOW RAW DATA: valid1 %d, adc1 0x%04x, valid2 %d, adc2 0x%04x\n",ii,
          a_valid1,a_adc1,a_valid2,a_adc2);
#endif
        samplecount += 2;

        ii++;
	  }

if(a_windowwidth != samplecount)
{
  error ++;
  printf("ERROR1: a_windowwidth=%d != samplecount=%d (slot %d, channel %d)\n",a_windowwidth,samplecount,a_slot,a_channel);
  fflush(stdout);
}


    }
    else if( ((datain[ii]>>27)&0x1F) == 0x15) /*window sum: obsolete*/
    {
#ifdef DEBUG
	  printf("[%3d] WINDOW SUM: must be obsolete\n",ii);
#endif
	  ii++;
    }
    else if( ((datain[ii]>>27)&0x1F) == 0x16) /*pulse raw data*/
    {
      a_channel = ((datain[ii]>>23)&0xF);
      a_pulsenumber = ((datain[ii]>>21)&0x3);
      a_firstsample = (datain[ii]&0x3FF);
#ifdef DEBUG
	  printf("[%3d] PULSE RAW DATA: channel %d, pulse number %d, first sample %d\n",ii,
			   a_channel,a_pulsenumber,a_firstsample);
#endif

	  ii++;

      while( ((datain[ii]>>31)&0x1) == 0)
	  {
        a_valid1 = ((datain[ii]>>29)&0x1);
        a_adc1 = ((datain[ii]>>16)&0xFFF);
        a_valid2 = ((datain[ii]>>13)&0x1);
        a_adc2 = (datain[ii]&0xFFF);
#ifdef DEBUG
        printf("   [%3d] PULSE RAW DATA: valid1 %d, adc1 0x%04x, valid2 %d, adc2 0x%04x\n",ii,
          a_valid1,a_adc1,a_valid2,a_adc2);
#endif

        ii++;
	  }

    }
    else if( ((datain[ii]>>27)&0x1F) == 0x17) /*pulse integral: assume that 'pulse time' was received ! */
    {
      a_channel = ((datain[ii]>>23)&0xF);
      a_pulsenumber = ((datain[ii]>>21)&0x3);
      a_qualityfactor = ((datain[ii]>>19)&0x3);
      a_pulseintegral = (datain[ii]&0x7FFFF);
#ifdef DEBUG
	  printf("[%3d] PULSE INTEGRAL: channel %d, pulse number %d, quality %d, integral %d\n",ii,
			   a_channel,a_pulsenumber,a_qualityfactor,a_pulseintegral);
#endif

	  ii++;
    }




    else if( ((datain[ii]>>27)&0x1F) == 0x18) /* pulse time */
    {
      a_channel = ((datain[ii]>>23)&0xF);
      a_pulsenumber = ((datain[ii]>>21)&0x3);
      a_qualityfactor = ((datain[ii]>>19)&0x3);
      a_pulsetime = (datain[ii]&0xFFFF);
#ifdef DEBUG
	  printf("[%3d] PULSE TIME: channel %d, pulse number %d, quality %d, time %d\n",ii,
			 a_channel,a_pulsenumber,a_qualityfactor,a_pulsetime);
#endif

	  ii++;
    }



    else if( ((datain[ii]>>27)&0x1F) == 0x19)
    {
#ifdef DEBUG
	  printf("[%3d] STREAMING RAW DATA: \n",ii);
#endif
	  ii++;
    }



    else if( ((datain[ii]>>27)&0x1F) == 0x1a) /* Vm Vp */
    {
      a_channel = ((datain[ii]>>23)&0xF);
      a_pulsenumber = ((datain[ii]>>21)&0x3);
      a_vm = ((datain[ii]>>12)&0x1FF);
      a_vp = (datain[ii]&0xFFF);
#ifdef DEBUG
	  printf("[%3d] PULSE VmVp: channel %d, pulse number %d, Vm %d, Vp %d\n",ii,
			 a_channel,a_pulsenumber,a_vm,a_vp);
#endif
	  ii++;
    }



    else if( ((datain[ii]>>27)&0x1F) == 0x1D)
    {
#ifdef DEBUG
	  printf("[%3d] EVENT TRAILER: \n",ii);
#endif
	  ii++;
    }
    else if( ((datain[ii]>>27)&0x1F) == 0x1E)
    {
	  printf("[%3d] : DATA NOT VALID\n",ii);
exit(0);
	  ii++;
    }
    else if( ((datain[ii]>>27)&0x1F) == 0x1F)
    {
#ifdef DEBUG
	  printf("[%3d] FILLER WORD: \n",ii);
      printf(">>> do nothing\n");
#endif
	  ii++;
    }
    else
    {
      error ++;
      if(printing) /* printing only once at every event */
      {
        printf("[%3d] pass1 ERROR: in FADC data format 0x%08x (bits31-27=0x%02x), slot=%d\n",
			   ii,(int)datain[ii],(datain[ii]>>27)&0x1F,a_slot);
        printing=0;
	  }
      ii++;
    }

  } /* loop over 'lenin' words */



  /*check if the number of events in every block is the same*/
  nnE = nE[0];
  for(k=1; k<nB; k++)
  {
    if(nE[k]!=nnE)
	{
      error ++;
      if(printing)
      {
        printf("SEVERE ERROR: different event number in difefrent blocks\n");
        printing=0;
        return;
	  }      
	}
  }


#ifdef DEBUG
  printf("\n=================\n");
  for(k=0; k<nB; k++)
  {
    printf("Block %2d, block index %2d\n",k,iB[k]);
    for(m=0; m<nnE; m++)
	{
      printf("Event %2d, event index %2d, event lenght %2d\n",m,iE[k][m],lenE[k][m]);
    }
  }
  printf("\n=================\n");
#endif




  /* in case of error, pass data in original format as it arrived from rol1 */
  if(error)
  {
    ;
  }





  /********************************************************/
  /* SECOND PASS: disantangling and filling output buffer */

#ifdef DEBUG
  printf("\nSECOND PASS\n\n");
#endif


  lenout = 2; /* already done in CPINIT !!?? */
  b08 = NULL;


#ifdef HAVE_SSP_DATA
  /*assume first NSSP words are ssp fifo*/
  if(rol->pid==1)
  {
    CPOPEN(0xe106,1,1);
    for(ii=0; ii<NSSP; ii++)
    {
      dataout[ii] = ssp[ii];
      b08 += 4;
    }
	/*
    printf("rol2: 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
		 dataout[0],dataout[1],dataout[2],dataout[3],dataout[4],dataout[5],dataout[6]);
	*/
    CPCLOSE;
  }
#endif




  ii=0;
  printing=1;
  /*
  while(ii<lenin)
  */

for(m=0; m<nnE; m++) /*loop over events*/
{
  a_slot_old = -1;
  a_channel_old = -1;

  if(b08 != NULL)
  {
    CCCLOSE; /*call CCCLOSE only if CCOPEN was called*/
	/*
if(mynev>MYNEV)
{
  printf("0x%08x: CCCLOSE1, dataout=0x%08x\n",b08,dataout);
  printf("0x%08x: CCCLOSE2, dataout=0x%08x\n",b08,(unsigned int *) ( ( ((unsigned int)b08+3)/4 ) * 4));
  printf("-> 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
		 (unsigned int)b08+3,((unsigned int)b08+3),
         ((unsigned int)b08+3) / 4,(((unsigned int)b08+3) / 4)*4, 
		 (unsigned int *)((((unsigned int)b08+3) / 4)*4) );
}
	*/
#ifdef DEBUG
    printf("0x%08x: CCCLOSE1, dataout=0x%08x\n",b08,dataout);
    printf("0x%08x: CCCLOSE2, dataout=0x%08x\n",b08,(unsigned int *) ( ( ((unsigned int)b08+3)/4 ) * 4));
    printf("-> 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
		 (unsigned int)b08+3,((unsigned int)b08+3),
         ((unsigned int)b08+3) / 4,(((unsigned int)b08+3) / 4)*4, 
		 (unsigned int *)((((unsigned int)b08+3) / 4)*4) );
#endif
  }

  for(k=0; k<nB; k++) /*loop over blocks*/
  {
#ifdef DEBUG
    printf("Block %d, Event %2d, event index %2d, event lenght %2d\n",k, m,iE[k][m],lenE[k][m]);
#endif
    a_slot = sB[k];
    ii = iE[k][m];
    rlen = ii + lenE[k][m];
    while(ii<rlen)
    {
      if( ((datain[ii]>>27)&0x1F) == 0x12) /*event header: remember trigger#*/
      {
        a_triggernumber = (datain[ii]&0x7FFFFFF);
#ifdef DEBUG
	    printf("[%3d] EVENT HEADER: trigger number %d\n",ii,
				   a_triggernumber);
        printf(">>> remember trigger %d\n",a_triggernumber);
#endif
	    ii++;
      }
      else if( ((datain[ii]>>27)&0x1F) == 0x13) /*trigger time: remember timestamp*/
      {
        a_trigtime[0] = (datain[ii]&0xFFFFFF);
#ifdef DEBUG
	    printf("[%3d] TRIGGER TIME: 0x%06x\n",ii,
			   a_trigtime[0]);
#endif
	    ii++;
        iii=1;
        while( ((datain[ii]>>31)&0x1) == 0 && ii<rlen ) /*must be one more word*/
	    {
          a_trigtime[iii] = (datain[ii]&0xFFFFFF);
#ifdef DEBUG
          printf("   [%3d] TRIGGER TIME: 0x%06x\n",ii,a_trigtime[iii]);
          printf(">>> remember timestamp 0x%06x 0x%06x\n",a_trigtime[0],a_trigtime[1]);
#endif
          iii++;
          ii++;
	    }
      }
      else if( ((datain[ii]>>27)&0x1F) == 0x14) /*window raw data: remember channel#*/
      {
        a_channel = ((datain[ii]>>23)&0xF);
        a_windowwidth = (datain[ii]&0xFFF);
#ifdef DEBUG
	    printf("[%3d] WINDOW RAW DATA: slot %d, channel %d, window width %d\n",ii,
			   a_slot,a_channel,a_windowwidth);
        printf(">>> remember channel %d\n",a_channel);
#endif

	    CCOPEN(0xe101,"c,i,l,N(c,Ns)",banknum);
/*if(mynev>MYNEV) printf("0x%08x: CCOPEN(1)\n",b08);*/
#ifdef DEBUG
        printf("0x%08x: CCOPEN(1)\n",b08);
#endif

#ifdef DO_PEDS
        if(mynev==10 && fd==NULL)
	    {
          printf("mynev=%d - opening pedestal file\n",mynev);
          if((dir=getenv("CLAS")) == NULL)
	      {
            printf("ERROR: environment variable CLAS is not defined - exit\n");
            exit(0);
		  }
          else
		  {
            printf("CLAS env. var. is >%s<\n",dir);
		  }

          if((expid=getenv("EXPID")) == NULL)
	      {
            printf("ERROR: environment variable EXPID is not defined - exit\n");
            exit(0);
		  }
          else
		  {
            printf("EXPID env. var. is >%s<\n",expid);
		  }
          printf("rol->pid is %d\n",rol->pid);

          sprintf(fname,"%s/parms/peds/%s/roc%02d.ped",dir,expid,rol->pid);
          printf("Will try to open file >%s<\n",fname);

          fd = fopen(fname,"w");
          if(fd==NULL)
          {
            printf("ttfa: ERROR: cannot open pedestal file >%s<\n",fname);
		  }
          else
	      {
            printf("ttfa: pedestal file >%s< is opened for writing\n",fname);
          }

          for(i=0; i<NSLOTS; i++)
	      {
            for(j=0; j<NCHANS; j++)
  	        {
              npeds[i][j] = 0;
              pedval[i][j] = 0.0;
              pedrms[i][j] = 0.0;
		    }
	      }
	    }
        else if(mynev==110 && fd!=NULL)
	    {
          printf("mynev=%d - closing pedestal file (Nmeasures=%d (%d %d ..))\n",
            mynev,npeds[3][0],npeds[3][1],npeds[4][0]);
          for(i=0; i<NSLOTS; i++)
	      {
            for(j=0; j<NCHANS; j++)
            {
              if(npeds[i][j]>0)
			  {
                pedval[i][j] = pedval[i][j] / ((float)npeds[i][j]);
#ifdef VXWORKS
                pedrms[i][j] = ( pedrms[i][j]/((float)npeds[i][j]) - (pedval[i][j]*pedval[i][j]) );
#else
                pedrms[i][j] = sqrtf( pedrms[i][j]/((float)npeds[i][j]) - (pedval[i][j]*pedval[i][j]) );
#endif
                fprintf(fd,"%2d %2d %5d %6.3f %2d\n",i,j,(int)pedval[i][j],pedrms[i][j],0);
			  }
		    }
	      }
          fclose(fd);
          printf("ttfa: pedestal file >%s< is closed\n",fname);
          fd = NULL;
	    }
#endif

	    Nchan[0] ++; /* increment channel counter */
#ifdef DEBUG
        printf("0x%08x: increment Nchan[0]=%d\n",b08,Nchan[0]);
#endif

        *b08++ = a_channel; /* channel number */

/*if(mynev>MYNEV) printf("0x%08x: #samples %d\n",b08,a_windowwidth);*/
#ifdef DEBUG
        printf("0x%08x: #samples %d\n",b08,a_windowwidth);
#endif
        b32 = (unsigned int *)b08;
        *b32 = a_windowwidth; /* the number of samples (same as window width) */
        b08 += 4;

	    ii++;
        npedsamples = a_windowwidth / 10; /* use first 10% of the window to measure pedestal */
        while( ((datain[ii]>>31)&0x1) == 0 && ii<rlen ) /*loop over all samples*/
	    {
          a_valid1 = ((datain[ii]>>29)&0x1);
          a_adc1 = ((datain[ii]>>16)&0xFFF);
          a_valid2 = ((datain[ii]>>13)&0x1);
          a_adc2 = (datain[ii]&0xFFF);
/*
if(mynev>MYNEV)
{
  printf("   [%3d] WINDOW RAW DATA: valid1 %d, adc1 0x%04x, valid2 %d, adc2 0x%04x\n",ii,
    a_valid1,a_adc1,a_valid2,a_adc2);
  printf("0x%08x: samples: %d %d\n",b08,a_adc1,a_adc2);
}
*/
#ifdef DEBUG
          printf("   [%3d] WINDOW RAW DATA: valid1 %d, adc1 0x%04x, valid2 %d, adc2 0x%04x\n",ii,
            a_valid1,a_adc1,a_valid2,a_adc2);
          printf("0x%08x: samples: %d %d\n",b08,a_adc1,a_adc2);
#endif

          b16 = (unsigned short *)b08;
          *b16 ++ = a_adc1;
          *b16 ++ = a_adc2;
          b08 += 4;

          ii++;

#ifdef DO_PEDS
          if(mynev>=10 && mynev<110 && npedsamples>0)
		  {
		    /*printf("[%2d][%2d] npedsamples=%d\n",a_slot,a_channel,npedsamples);*/
            npedsamples -= 2;
            npeds[a_slot][a_channel] += 2;
            pedval[a_slot][a_channel] += *(b16-2);
            pedval[a_slot][a_channel] += *(b16-1);
            pedrms[a_slot][a_channel] += (*(b16-2))*(*(b16-2));
            pedrms[a_slot][a_channel] += (*(b16-1))*(*(b16-1));
		  }
#endif
	    }
      }
      else if( ((datain[ii]>>27)&0x1F) == 0x15) /*window sum: obsolete*/
      {
#ifdef DEBUG
	    printf("[%3d] WINDOW SUM: must be obsolete\n",ii);
#endif
	    ii++;
      }
      else if( ((datain[ii]>>27)&0x1F) == 0x16) /*pulse raw data*/
      {
        a_channel = ((datain[ii]>>23)&0xF);
        a_pulsenumber = ((datain[ii]>>21)&0x3);
        a_firstsample = (datain[ii]&0x3FF);
#ifdef DEBUG
	    printf("[%3d] PULSE RAW DATA: channel %d, pulse number %d, first sample %d\n",ii,
			   a_channel,a_pulsenumber,a_firstsample);
#endif

        CCOPEN(0xe102,"c,i,l,N(c,N(c,Ns))",banknum);
#ifdef DEBUG
        printf("0x%08x: CCOPEN(2)\n",b08);
#endif

        if(a_channel != a_channel_old)
        {
          a_channel_old = a_channel;

          Nchan[0] ++; /* increment channel counter */
#ifdef DEBUG
        printf("0x%08x: increment Nchan[0]=%d\n",b08,Nchan[0]);
#endif

          *b08++ = a_channel; /* channel number */

          Npuls = (unsigned int *)b08; /* remember the place to put the number of pulses */
          Npuls[0] = 0;
          b08 += 4;
        }


        Npuls[0] ++;
        *b08++ = a_firstsample; /* first sample */

        Nsamp = (unsigned int *)b08; /* remember the place to put the number of samples */
        Nsamp[0] = 0;
        b08 += 4;

	    ii++;

        while( ((datain[ii]>>31)&0x1) == 0)
	    {
          a_valid1 = ((datain[ii]>>29)&0x1);
          a_adc1 = ((datain[ii]>>16)&0xFFF);
          a_valid2 = ((datain[ii]>>13)&0x1);
          a_adc2 = (datain[ii]&0xFFF);
#ifdef DEBUG
          printf("   [%3d] PULSE RAW DATA: valid1 %d, adc1 0x%04x, valid2 %d, adc2 0x%04x\n",ii,
            a_valid1,a_adc1,a_valid2,a_adc2);
#endif

          b16 = (unsigned short *)b08;
          *b16 ++ = a_adc1;
          *b16 ++ = a_adc2;
          b08 += 4;

          Nsamp[0] += 2;
          ii++;
	    }

      }
      else if( ((datain[ii]>>27)&0x1F) == 0x17) /*pulse integral: assume that 'pulse time' was received ! */
      {
        a_channel = ((datain[ii]>>23)&0xF);
        a_pulsenumber = ((datain[ii]>>21)&0x3);
        a_qualityfactor = ((datain[ii]>>19)&0x3);
        a_pulseintegral = (datain[ii]&0x7FFFF);
#ifdef DEBUG
	    printf("[%3d] PULSE INTEGRAL: channel %d, pulse number %d, quality %d, integral %d\n",ii,
			   a_channel,a_pulsenumber,a_qualityfactor,a_pulseintegral);
#endif


#ifdef DEBUG
        printf("0x%08x: pulseintegral = %d\n",b08,a_pulseintegral);
#endif
        b32 = (unsigned int *)b08;
        *b32 = a_pulseintegral;
        b08 += 4;


	    ii++;
      }




      else if( ((datain[ii]>>27)&0x1F) == 0x18) /* pulse time */
      {
        a_channel = ((datain[ii]>>23)&0xF);
        a_pulsenumber = ((datain[ii]>>21)&0x3);
        a_qualityfactor = ((datain[ii]>>19)&0x3);
        a_pulsetime = (datain[ii]&0xFFFF);
#ifdef DEBUG
	    printf("[%3d] PULSE TIME: channel %d, pulse number %d, quality %d, time %d\n",ii,
			 a_channel,a_pulsenumber,a_qualityfactor,a_pulsetime);
#endif



        CCOPEN(0xe103,"c,i,l,N(c,N(s,i))",banknum);
#ifdef DEBUG
        printf("0x%08x: CCOPEN(3), dataout=0x%08x\n",b08,dataout);
#endif


        if(a_channel != a_channel_old)
        {
          a_channel_old = a_channel;

          Nchan[0] ++; /* increment channel counter */
#ifdef DEBUG
        printf("0x%08x: increment Nchan[0]=%d\n",b08,Nchan[0]);
#endif

#ifdef DEBUG
		  printf("0x%08x: channel %d\n",b08,a_channel);
#endif
          *b08++ = a_channel; /* channel number */

          Npuls = (unsigned int *)b08; /* remember the place to put the number of pulses */
          Npuls[0] = 0;
#ifdef DEBUG
		  printf("0x%08x: Npuls[0]\n",b08);
#endif
          b08 += 4;
        }

        Npuls[0] ++;

#ifdef DEBUG
        printf("0x%08x: pulsetime = %d\n",b08,a_pulsetime);
#endif
        b16 = (unsigned short *)b08;
        *b16 = a_pulsetime;
        b08 += 2;

	    ii++;
      }


      else if( ((datain[ii]>>27)&0x1F) == 0x19)
      {
#ifdef DEBUG
	    printf("[%3d] STREAMING RAW DATA: \n",ii);
#endif
	    ii++;
      }
      else if( ((datain[ii]>>27)&0x1F) == 0x1a) /* Vm Vp */
      {
        a_channel = ((datain[ii]>>23)&0xF);
        a_pulsenumber = ((datain[ii]>>21)&0x3);
        a_vm = ((datain[ii]>>12)&0x1FF);
        a_vp = (datain[ii]&0xFFF);
#ifdef DEBUG
	    printf("[%3d] PULSE VmVp: channel %d, pulse number %d, Vm %d, Vp %d\n",ii,
			 a_channel,a_pulsenumber,a_vm,a_vp);
#endif
	    ii++;
      }



      else if( ((datain[ii]>>27)&0x1F) == 0x1D)
      {
#ifdef DEBUG
	    printf("[%3d] EVENT TRAILER: \n",ii);
#endif
	    ii++;
      }
      else if( ((datain[ii]>>27)&0x1F) == 0x1E)
      {
	    printf("[%3d] : DATA NOT VALID\n",ii);
exit(0);
	    ii++;
      }
      else if( ((datain[ii]>>27)&0x1F) == 0x1F)
      {
#ifdef DEBUG
	    printf("[%3d] FILLER WORD: \n",ii);
        printf(">>> do nothing\n");
#endif
	    ii++;
      }
      else
      {
        error ++;
        if(printing) /* printing only once at every event */
        {
          printf("[%3d] pass2 ERROR: in FADC data format 0x%08x (bits31-27=0x%02x)\n",
			   ii,(int)datain[ii],(datain[ii]>>27)&0x1F);
          printing=0;
	    }
        ii++;
      }

    } /*while*/

  } /* loop over blocks */

} /* loop over events */



  if(b08 != NULL) CCCLOSE; /*call CCCLOSE only if CCOPEN was called*/
/*
if(mynev>MYNEV)
{
  printf("0x%08x: CCCLOSE, dataout=0x%08x\n",b08,dataout);
  dataout = (unsigned int *) ( ( ((unsigned int)b08+3)/4 ) * 4);
  printf("0x%08x: CCCLOSE, dataout=0x%08x\n",b08,dataout);
  printf("-> 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
		 (unsigned int)b08+3,((unsigned int)b08+3),
         ((unsigned int)b08+3) / 4,(((unsigned int)b08+3) / 4)*4, 
		 (unsigned int *)((((unsigned int)b08+3) / 4)*4) );
}
*/
#ifdef DEBUG
  printf("0x%08x: CCCLOSE, dataout=0x%08x\n",b08,dataout);
  dataout = (unsigned int *) ( ( ((unsigned int)b08+3)/4 ) * 4);
  printf("0x%08x: CCCLOSE, dataout=0x%08x\n",b08,dataout);
  printf("-> 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
		 (unsigned int)b08+3,((unsigned int)b08+3),
         ((unsigned int)b08+3) / 4,(((unsigned int)b08+3) / 4)*4, 
		 (unsigned int *)((((unsigned int)b08+3) / 4)*4) );
#endif

  /* returns full fragment length (long words) */  

#ifdef DEBUG 
  printf("return lenout=%d\n**********************\n\n",lenout);
#endif


  CPEXIT;

  return;
}

void
rol2trig_done()
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
