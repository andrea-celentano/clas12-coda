
/* ttnondc.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "coda.h"
#include "tt.h"


/* for 1877 TDCs only ??? */
#define PCALFB

#define DBGVERS 1

#undef DBG
#define DBG  /*printf*/

/*
 * 1'st find if this data belongs to already known BOS bank or start the new one
 * 2'nd save pointer to this data as a begining of raw data fragment.
 */

#define IS_ZEROSLOT ((SLOT < 1) || (SLOT > 26))

#define IS_HEADER    SLOT != slotold && SLTYPE[SLOT] > 0 && CHANNEL == 0

#define NOT_END_OF_BUFFER (long *)fb < (&bufin[1] + bufin[0])
#define NOT_OR_END_OF_BUFFER (long *)fb <= (&bufin[1] + bufin[0])
#define END_OF_BUFFER (long *)fb >= (&bufin[1] + bufin[0])

#define NEXT_DATAWORD fb++

#define CONTINUE_IF_ZEROSLOT(macro_tag) \
  if (IS_ZEROSLOT) { \
	printf(" --> ZEROSLOT, SKIP %d\n",macro_tag); \
    NEXT_DATAWORD; \
    continue; \
  }
#define CONTINUE_IF_64BIT \
  if (SLOT == SLOT64) \
  { \
	printf("ttnondc: 64 bit signature skiped 1 ...\n"); \
    NEXT_DATAWORD; \
    continue; \
  }
#define SKIP_64BIT \
   if (SLOT == SLOT64) \
   {\
	printf("ttnondc: 64 bit signature skiped 2 ...\n"); \
    NEXT_DATAWORD; \
  }
#define ZEROSLOTS_SKIP \
  while(IS_ZEROSLOT && NOT_END_OF_BUFFER) {\
	printf("ZEROSLOTS_SKIP:  fb=%8.8X  %3i ( %3i - %3i ) %4X\n", \
      fb,fb->slot,fb->channel,fb->channel>>1,fb->data);\
    NEXT_DATAWORD; \
  }

#define START_RAW_FRAG \
  for(i=0; i<=nBanks; i++) { \
     if (NAME[SLOT][CHANNEL] == FragTab[i].name && \
         NR[SLOT][CHANNEL] == FragTab[i].nr) break; /* Found already known BOS Bank -> exit loop */ \
  } \
  FragTab[i].name = NAME[SLOT][CHANNEL]; \
  FragTab[i].nr = NR[SLOT][CHANNEL]; \
  if(FragTab[i].n >= NPREFRAGS) \
  { \
    printf("ERROR: too many fragments !!! will overwrite last fragment ...\n"); \
  } \
  else \
  { \
    FragTab[i].n = FragTab[i].n + 1; \
  } \
  FragTab[i].begin[FragTab[i].n] = fb; \
  nBanks_cur = i; \
  if ( nBanks_cur > nBanks ) nBanks = nBanks_cur;\
  DBG("==> Start of Raw Frag %i of BOSBANK '%4.4s' # %i slot=%d chan=%d\n"\
		  ,FragTab[i].n,&FragTab[i].name,FragTab[i].nr,SLOT,CHANNEL); \
  DBG("    fb=%8.8X Name='%4.4s' nr=%d type=%d roc=%d id=%d place=%d\n", \
		  fb, &(NAME[SLOT][CHANNEL]),\
			NR[SLOT][CHANNEL],\
			SLTYPE[SLOT],ROC,\
			ID[SLOT][CHANNEL],\
			PLACE[SLOT][CHANNEL]) ;
/*
 * 
 */
#define  CLOSE_RAW_FRAG    \
  FragTab[nBanks_cur].end[FragTab[nBanks_cur].n] = fb;\
  DBG("\n=======> End   of Raw Frag %i of BOSBANK '%4.4s' # %i\n"\
	  ,FragTab[i].n,&FragTab[i].name,FragTab[i].nr); \
  if((int)fb > (int)((&bufin[1])+bufin[0])) \
    printf("EEE1: 0x%08x 0x%08x\n",fb,((&bufin[1])+bufin[0]))

#define CONTINUE_IF_HEAD    \
  if ( IS_HEADER ) \
  { \
    slotold = SLOT; \
	DBG(" --> Skip HEADER\n"); \
	NEXT_DATAWORD; \
    continue; \
  }

#define HEAD_SKIP_FIRST \
  while ( IS_HEADER ) \
  { \
    slotold = SLOT; \
   	DBG("CLASFRAGMENT: HEAD_SKIP_FIRST  fb=%8.8X  %3i ( %3i - %3i ) %4X\n", \
      fb,SLOT,CHANNEL,CHANNEL>>1,fb->data); \
    NEXT_DATAWORD; \
	if((long *)fb > (&bufin[1] + bufin[0])) return(0); \
  }


#define REPORT_UNTABLED_ID \
		/*printf(" LIST2 WARN(1): untabled id= %d\n",id); \
	    ll = CHANNEL; \
	    if(SLTYPE[SLOT] > 0) ll = ll >> 1; \
	    printf(" slot=%d channel=%d data=%4X\n",SLOT,ll,fb->data)*/




/**************/
/*PCALFB stuff*/


#define CHANNEL1877   fb1877->channel
#define SLOT1877      fb1877->slot


#define TEST_SLOT_CHANNEL0 \
  if(SLOT1877<=0 || SLOT1877>25 || CHANNEL1877<0 || CHANNEL1877>95) \
  { \
    printf("ERROR0: slot=%d channel=%d\n",SLOT1877,CHANNEL1877); \
  } 

#define TEST_SLOT_CHANNEL1 \
  if(SLOT1877<=0 || SLOT1877>25 || CHANNEL1877<0 || CHANNEL1877>95) \
  { \
    printf("ERROR1: slot=%d channel=%d\n",SLOT1877,CHANNEL1877); \
  } 

#define TEST_SLOT_CHANNEL2 \
  if(SLOT1877<=0 || SLOT1877>25 || CHANNEL1877<0 || CHANNEL1877>95) \
  { \
    printf("ERROR2: slot=%d channel=%d\n",SLOT1877,CHANNEL1877); \
  } 




#define SKIP_ZERO_FIRST \
 while ((long *)mtdc < ((&bufin[1])+bufin[0]) && \
       ( mtdchead->slot == 0 || mtdchead->count == 1) ) \
 { \
   /*printf("tttage: SKIP_ZERO_FIRST INFO 0x%08x (slot=%2d count=%d) - skiped ...\n", \
	*((unsigned long *)mtdchead),mtdchead->slot,mtdchead->count);*/ \
   mtdchead++; \
   mtdc++; \
   fb++; \
 } \
 if((long *)mtdc >= ((&bufin[1])+bufin[0])) \
 { \
   /*printf("LIST2: FIRST ZERO: exit !!!\n");*/ \
   return(/*0*/len); \
 }

#define SKIP_64BIT_1877 \
  if(((long *)mtdc < ((&bufin[1])+bufin[0])) && (mtdchead->slot == SLOT64)) \
  { \
    mtdchead++; \
    mtdc++; \
    fb++; \
	/*printf("ttdc: 64 bit signature skiped ...\n");*/ \
  }

#define BANK_OPEN_1877 \
          savencol = NBOSCOL[SLOT1877][CHANNEL1877]; /* to be used in BANK_CLOSE */ \
          if(savencol <= 0) \
          { \
            printf("tttage0: ERROR: slot=%d channel=%d does not described in translation table\n", \
              SLOT1877,CHANNEL1877); \
            printf("tttage0: returns len=%d\n",len); \
            return(len); \
          } \
          ind = bosNopen(iw,NAME[SLOT1877][CHANNEL1877],NR[SLOT1877][CHANNEL1877],NBOSCOL[SLOT1877][CHANNEL1877],0); \
          if(ind <= 0) \
          { \
            printf("tttage0: bosNopen returned %d !!! -> return len=%d\n",ind,len); \
            printf("tttage0: slot=%d channel=%d name>%4.4s< nr=%d nboscol=%d\n", \
              SLOT1877,CHANNEL1877,NAME[SLOT1877][CHANNEL1877],NR[SLOT1877][CHANNEL1877],NBOSCOL[SLOT1877][CHANNEL1877]); \
            return(len); \
          } \
          else \
          { \
            ; \
          } \
          hit = (unsigned short *)&iw[ind+1]

#define BANK_CLOSE_1877 /* fb not defined any more, it points to next word after buffer !!! */ \
          hit[0] = hit[1] = hit[2] = 0; \
          hit = (unsigned short *)(((int)hit + 3)/4 * 4); \
          len += bosNclose(iw,ind,savencol, (unsigned long *)hit - (unsigned long *)&iw[ind+1] )


/*PCALFB stuff*/
/**************/






static char* Version = " VERSION: ttnondc.c v1.0 - initial release " ;
static char* CompileTime = DAYTIME;

/******************************************************************************

  Routine        : TT_TranslatePCALFBBank

  Parameters     : bufin - input(CODA) fragment
                   bufout - output(BOS) fragment (loading from 3rd CODA word !)

  Discription    : This routine translates one fragment from CODA to
		           BOS format for general case

******************************************************************************/

int
TT_TranslatePCALFBBank(long *bufin, long *bufout, TTSPtr ttp)
{
  preStructS FragTab[NPREBANKS];
  int nBanks, nBanks_cur;
  INDPtr fb;                              /* input data pointer */
  unsigned short *hit, *hp;               /* output data pointer */
  unsigned short *ids;         /* partner address pointer */
  int i, j, k, id, place, bank, slotold=0, ll, ind, len, savencol;
  long *iw, *endofbufout;
#ifdef PCALFB
  unsigned short *oldhit;
  MTDCPtr endofboard, mtdc, bufin1877, fb1877;
  MTDCHeadPtr mtdchead;

  bufin1877 = NULL;
#endif

  iw = &bufout[2];       /* set BOS pointer to output buffer */
  len = 0;
  endofbufout = bufout + bosleftspace(iw); /* set end_of_bufout pointer */

  ids = ttp->ids;

  /* ==================================================================================
	 first pass 
	 ================================================================================== */

  fb = (INDPtr)&bufin[1];                /* set input data pointer */
  nBanks = -1;
  nBanks_cur = -1;  
  DBG("Initializing FragTab\n");
  for(i=0; i<NPREBANKS; i++) FragTab[i].n = -1;    

  DBG("beg=0x%8.8X end=0x%8.8X out=0x%8.8X\n",(int)fb,(&bufin[1] + bufin[0]),&bufout[0]);
  DBG("\nCLASFRAGMENT:========= FIRST  PASS ==============\n");
  DBG("DATA11 >>> fb=%8.8X  %3i ( adc%3i - tdc%3i ) %4X\n",fb,fb->slot,fb->channel&0x7f,(fb->channel>>1)&0x7f,fb->data);

#ifdef DBGVERS
  SKIP_64BIT;
  ZEROSLOTS_SKIP;
#endif

#ifdef PCALFB
  while ( IS_HEADER )
  {
    slotold = SLOT;
   	DBG("CLASFRAGMENT: HEAD_SKIP_FIRST  fb=%8.8X  %3i ( %3i - %3i ) %4X\n",
      fb,SLOT,CHANNEL,CHANNEL>>1,fb->data);
    if(slotold<19)
    {
      bufin1877 = (MTDCPtr)fb;
      DBG("FOR TDCs: slot=%d slotold=%d, END ADCs, START TDCs: 0x%08x 0x%08x\n",
          fb->slot,slotold,bufin1877,*bufin1877);
      goto a123;
    }
    else
	{
      NEXT_DATAWORD;
      if((long *)fb > (&bufin[1] + bufin[0])) return(0);
    }
  }
#else
  HEAD_SKIP_FIRST;
#endif

#ifdef DBGVERS
  ZEROSLOTS_SKIP;
#endif
  DBG("DATA12 >>> fb=%8.8X  %3i ( adc%3i - tdc%3i ) %4X\n",fb,fb->slot,fb->channel&0x7f,(fb->channel>>1)&0x7f,fb->data);
  id = ID[SLOT][CHANNEL];

  while ( (id > MAXID || id == 0) && NOT_OR_END_OF_BUFFER )
  {
	DBG("DATA13 >>> fb=%8.8X  %3i ( adc%3i - tdc%3i ) %4X\n",fb,fb->slot,fb->channel&0x7f,(fb->channel>>1)&0x7f,fb->data);
	REPORT_UNTABLED_ID;
	NEXT_DATAWORD;
    id = ID[SLOT][CHANNEL];
  }
  if(END_OF_BUFFER)
  {
    DBG("NO ANY REASONABLE INFO - RETURN\n");
    return(0);
  }

  START_RAW_FRAG;
  NEXT_DATAWORD;
  
  while(NOT_END_OF_BUFFER)
  {
    DBG("DATA14 >>> fb=%8.8X  %3i ( adc%3i - tdc%3i ) %4X\n",fb,fb->slot,fb->channel&0x7f,(fb->channel>>1)&0x7f,fb->data);
#ifdef DBGVERS
    CONTINUE_IF_64BIT;
    CONTINUE_IF_ZEROSLOT(1);
#endif

#ifdef PCALFB
    if ( IS_HEADER )
    {
      slotold = SLOT;
	  DBG(" --> Skip HEADER\n");
      /*if slot < 19 (all 1881's are between 25 and 19), remember input pointer
      to be used in 1877 processing, and exit loop*/
      if(slotold<19)
      {
        bufin1877 = (MTDCPtr)fb;
        DBG("FOR TDCs: slot=%d slotold=%d, END ADCs, START TDCs: 0x%08x 0x%08x\n",
          fb->slot,slotold,bufin1877,*bufin1877);
        break;
      }
      else
	  {
        NEXT_DATAWORD;
        continue;
	  }
    }
#else
    CONTINUE_IF_HEAD; /* skip header for boards with header word */
	/*CONTINUE_IF_ZEROSLOT(2);*/
#endif

    id = ID[SLOT][CHANNEL];
    if( id > MAXID || id == 0 ) /* && NOT_END_OF_BUFFER */
    {
      REPORT_UNTABLED_ID;
      NEXT_DATAWORD;
      continue;
    }

    /*CONTINUE_IF_ZEROSLOT(3);*/

    if(NAME[SLOT][CHANNEL] != FragTab[nBanks_cur].name || NR[SLOT][CHANNEL] != FragTab[nBanks_cur].nr)
    {
      CLOSE_RAW_FRAG;
      START_RAW_FRAG;
    }
    NEXT_DATAWORD;
  }
  CLOSE_RAW_FRAG;

  DBG("\nCLASFRAGMENT:========= SECOND PASS ==============\n");

  /* ==================================================================================
	 second pass 
	 ================================================================================== */

  /* translation */

  for(i=0; i<=nBanks; i++)           /* Loop on BOS Banks  */
  {
    fb = FragTab[i].begin[0];        /* set input data pointer     */
    slotold=SLOT;                    /* Very important to remember */

    DBG("DATA1 >>> fb=%8.8X  %3i ( %3i - %3i ) %4X\n",fb,SLOT,CHANNEL,CHANNEL>>1,fb->data);

    /* create BOS bank */
    ind = bosNopen(iw,NAME[SLOT][CHANNEL],NR[SLOT][CHANNEL],NBOSCOL[SLOT][CHANNEL],0);
    if(ind <= 0)
    {
      printf("ttnondc: bosNopen returned %d !!! Can't create BOS bank --> exiting\n",ind);fflush(stdout);
      printf("ttnondc: fb=%8.8X  %3i ( %3i - %3i ) %4X\n",fb,SLOT,CHANNEL,CHANNEL>>1,fb->data);
      printf("ttnondc: name = 0x%08x\n",NAME[SLOT][CHANNEL]);
      printf("ttnondc: nr = %d\n",NR[SLOT][CHANNEL]);
      printf("ttnondc: nboscol = %d\n",NBOSCOL[SLOT][CHANNEL]);
      return(len);
    }

    savencol = NBOSCOL[SLOT][CHANNEL]; /* to be used in bosNclose() */
    if(savencol <= 0)
    {
      printf("ttnondc: ERROR: slot=%d channel=%d does not described in translation table\n",SLOT,CHANNEL);
      return(len);
    }

    hit = (unsigned short *)&iw[ind+1];

    for(j=0; j<=FragTab[i].n; j++)       /* Loop on fragments of current BOS Bank */
    {
      INDPtr fragstart;

      fb = fragstart = FragTab[i].begin[j];        /* set input data pointer */
      DBG("CLASFRAGMENT: Process Raw Fragment # %i of %i word(s) for %4.4s#%d\n",
              j,FragTab[i].end[j]-FragTab[i].begin[j],&(NAME[SLOT][CHANNEL]),NR[SLOT][CHANNEL]);
      while(fb < FragTab[i].end[j]) /* Loop on data words inside one fragment */
      {
        DBG("DATA2 >>> fb=%8.8X  %3i ( %3i - %3i ) %4X\n",fb,SLOT,CHANNEL,CHANNEL>>1,fb->data);
#ifdef DBGVERS
        CONTINUE_IF_ZEROSLOT(4);
#endif
        if (fb != fragstart)
        {
          CONTINUE_IF_HEAD; /* skip header for boards with header word */
		}

        /*CONTINUE_IF_ZEROSLOT(5);*/

        slotold=SLOT; /********* Very important to remember ********/

        id = ID[SLOT][CHANNEL];
        if(id >= MAXID || id == 0)
        {
          REPORT_UNTABLED_ID;
          NEXT_DATAWORD;
          continue;
        }

        /*CONTINUE_IF_ZEROSLOT(6);*/ 

        place = PLACE[SLOT][CHANNEL];
        DBG ("partner=0x%8.8X id=%4.4X place=%i fb=0x%8.8X\n",ttp->partner,id,place,fb);

        if(ttp->partner[id] == 0)    /* met id first time */
        {
          *hit = id;
          *ids = id;
		  ids++;
          ttp->partner[id] = hit;

          /* check the rest of space */

          if( (long *)(hit+NBOSCOL[SLOT][CHANNEL]) + NHWRES > endofbufout )
          {
            printf(" ERROR: bufout overflow - skip the rest ...\n");
            printf("        bufout=%u hit=%u endofbufout=%u\n",bufout,hit,endofbufout);
            /* close bank: fb not defined any more, it points to next word after buffer !!! */
            len = len + bosNclose(iw,ind,savencol,(hit-(unsigned short *)&iw[ind+1])/savencol);
            return(len);
          }

		  for(k=1; k<NBOSCOL[SLOT][CHANNEL]; k++) *(hit+k) = 0;
		  *(hit+place+1) = fb->data;
		  hit += NBOSCOL[SLOT][CHANNEL];
		  DBG(" --> MET ID %4.4X (pl %i) FIRST  TIME\n",id,place);
		}
        else                        /* met id second time */
		{
		  DBG(" --> MET ID %4.4X (pl %i) SECOND TIME\n",id,place);fflush(stdout);
	      *(ttp->partner[id]+place+1)= fb->data;
        }
		NEXT_DATAWORD;
      }                               /* End of Loop on data words inside fragment */
    }                                 /* End of Loop on fragments of current BOS Bank */

    /* cleanup partner array */

    k = (int)(ids - ttp->ids);
    ids = ttp->ids;
    DBG("Cleaning %i words in partners array\n",k);
    for(j=0; j<k; j++)
    {
      DBG("%i: id = %i  partner[id] = 0x%8.8X\n",i,*ids,ttp->partner[*ids]);
      ttp->partner[*ids++] = 0;
    }
    ids = ttp->ids;
    /* close bank: fb not defined any more, it points to next word after buffer !!! */

    len += bosNclose(iw,ind,savencol,(hit-(unsigned short *)&iw[ind+1])/savencol);
  }                                   /* End of Loop on BOS Banks */



#ifdef PCALFB


a123:

  if(fb1877==NULL) return(len);


  mtdchead = (MTDCHeadPtr)bufin1877; /* first tdc header */
  mtdc = fb1877 = (MTDCPtr)(bufin1877+1);    /* first tdc data word */

  DBG("TDCs: 1: beg=0x%08x end=0x%08x\n",(int)mtdc,(&bufin[1] + bufin[0]));

  SKIP_ZERO_FIRST;
  TEST_SLOT_CHANNEL0;
  SKIP_64BIT_1877;
  BANK_OPEN_1877;

  DBG("TDCs: 2: beg=0x%08x end=0x%08x\n",(int)mtdc,(&bufin[1] + bufin[0]));

  while((long *)mtdc < ((&bufin[1])+bufin[0]))  /* loop over input buffer */  
  {
	
    DBG("MTDC Header 0x%8.8X : slot %d count %d\n",*((unsigned long *)mtdchead),
	mtdchead->slot,(unsigned long)mtdchead->count);
    endofboard = (MTDCPtr)mtdchead + mtdchead->count;  /* end of data for board */

    if(mtdchead->slot>25 || mtdchead->slot==0)
    {
      printf("ROC # %d Event # %d :  Bad Block Read signature 0x%8.8X -> resyncronize !!!\n",
             ttp->roc,0,*((unsigned long *)mtdchead));
      mtdc++;
      fb1877 = mtdc;
      mtdchead = (MTDCHeadPtr)mtdc;
      continue;
    }

    while(mtdc->slot==mtdchead->slot)	        /* loop over present board */
	{
      /* both name and number of BOS bank can be changed for translation */
      /* if changed - close old bank and start new one */

      TEST_SLOT_CHANNEL1;

      /* check the rest of space */

      if( (unsigned long *)(hit+2) + NHWRES > endofbufout )
      {
	    printf(" ERROR: bufout overflow - skip the rest ...\n");
	    printf("        bufout=%u hit=%u endofbufout=%u\n",bufout,hit,endofbufout);
        /* close bank */
        BANK_CLOSE_1877;
        return(len);
      }
      TEST_SLOT_CHANNEL2;


	  /* Sergey: keep hit only if it is in specified range */
      if(1/*mtdc->data > 750*/)
	  {
	    hit[0] = ttp->id2[SLOT1877][0][CHANNEL1877];
	    hit[1] = (unsigned short)(mtdc->data);
	    
	    DBG("Channel %d : %4.4X  -> BOS row : id %d  data %4.4X\n",
		    mtdc->channel,mtdc->data,(unsigned long)hit[0],(unsigned long)hit[1]);
	    
	    hit += 2;
	  }


	  mtdc++;
	  fb1877 = mtdc;

      /* end of buffer ? new slot ? goto loop over input buffer */
	  if((long *)mtdc >= ((&bufin[1])+bufin[0]) || mtdc->slot!=mtdchead->slot)
      {
		if((int)mtdc != (int)endofboard)
        {
          printf("event %d -> mtdc=0x%08x endofboard=0x%08x (slots %2d %2d) (adrs 0x%08x 0x%08x)\n",
		  0,(int)mtdc,(int)endofboard,mtdchead->slot,mtdc->slot,(long *)mtdc,((&bufin[1])+bufin[0]));
        }
        break;
      }

	}                                   /* end of while mtdc < endofboard */

	mtdchead = (MTDCHeadPtr)mtdc;
	mtdc++;
	fb1877 = mtdc;

  }                           /* end of while mtdc < ((&bufin[1])+bufin[0]) */

  /* close bank */
  BANK_CLOSE_1877;




#endif




  /* returns full fragment length (long words) */
  return(len);

}

/*
int
checkpartner (void)
{
  int j;
  for(j=0; j<MAXID; j++)
  {
    if (ttp->partner[j]) printf ("%4.4X: 0x%8.8X\n",j,ttp->partner[j]);
  }
}
*/
