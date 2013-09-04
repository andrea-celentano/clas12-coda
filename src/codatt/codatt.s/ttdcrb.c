
/* ttdcrb.c - translate CLAS12 DCRB data into EVIO format */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "coda.h"
#include "tt.h"


#define DEBUG

#define ABS(x) ((x) < 0 ? -(x) : (x))

#define LSWAP(x)        ((((x) & 0x000000ff) << 24) | \
                         (((x) & 0x0000ff00) <<  8) | \
                         (((x) & 0x00ff0000) >>  8) | \
                         (((x) & 0xff000000) >> 24))

#define CPOPEN(btag,btyp,bnum) \
{ \
  dabufp_save1 = dabufp ++; /*remember beginning of the bank address, exclusive bank length will be here*/ \
  *dabufp++ = (btag<<16) + (btyp<<8) + bnum; /*bank header*/ \
  b08 = (unsigned char *)dabufp; \
}


#define CPCLOSE \
{ \
  unsigned int padding; \
  printf("CPCLOSE: dabufp before = 0x%x\n",dabufp); \
  dabufp = (unsigned int *) ( ( ((unsigned int)b08+3)/4 ) * 4); \
  padding = (unsigned int)dabufp - (unsigned int)b08; \
  dabufp_save1[1] |= (padding&0x3)<<14; /*update bank header (2nd word) with padding info*/ \
  printf("CPCLOSE: 0x%x %d --- 0x%x %d --> padding %d\n",dabufp,dabufp,b08,b08,((dabufp_save2[1])>>14)&0x3); \
  *dabufp_save1 = (dabufp-dabufp_save1-1); /*write bank length*/ \
  printf("CPCLOSE: *dabufp_save1 = %d\n",*dabufp_save1); \
  len += (*dabufp_save1+1); \
  b08 = NULL; \
}


#define DC_DATA_BLOCK_HEADER      0x00
#define DC_DATA_BLOCK_TRAILER     0x01
#define DC_DATA_EVENT_HEADER      0x02
#define DC_DATA_TRIGGER_TIME      0x03
#define DC_DATA_DCRBEVT           0x08
#define DC_DATA_INVALID           0x0E
#define DC_DATA_FILLER            0x0F



static char* Version = " VERSION: ttvme.c v1.0 - initial release " ;
static char* CompileTime = DAYTIME;

/***********************************************************************************

  Routine             : TT_TranslateDCRBBank

  Parameters          : bufin - input(CODA from ROL1), bufout - output(EVIO)

  Discription         : This routine translates one event from crate with CLAS12 DCRB
                        
***********************************************************************************/

 /* 22-max# of blocks=slots, 256-max# of events*/
#define MAXBLOCK 22
#define MAXEVENT 256

int
TT_TranslateDCRBBank(long *bufin, long *bufout, TTSPtr ttp)
{
  int nB, iB[MAXBLOCK], sB[MAXBLOCK];
  int nE[MAXBLOCK], iE[MAXBLOCK][MAXEVENT], lenE[MAXBLOCK][MAXEVENT];
  unsigned int *iw, *dcrb, *endofbufout;
  int i, j, k, m, iii, ind, len, bank, nhits=0, mux_index, rlen, printing, nnE;
  int nr = 0;
  int ncol = 2;
  int a_channel, a_nevents, a_blocknumber, a_triggernumber;
  int a_windowwidth, a_pulsenumber, a_firstsample;
  int a_valid1, a_valid2, a_nwords, a_slot, a_slot2;
  int a_qualityfactor, a_pulseintegral, a_pulsetime;
  int a_trigtime[4];
  int a_slot_old;
  int a_channel_old;

  unsigned int *dabufp_save1, *dabufp_save2;
  unsigned int *StartOfBank;
  unsigned int *dabufp, word;
  char *ch;
  unsigned char *b08;
  unsigned short *b16;
  unsigned int *b32, *Nchan, *Npuls, *Nsamp;
  unsigned long long *b64, timestamp;
  int dcrbref;
  int islot, ichan, ii, jj, npulses, type, nw, tdc, edge, tmpx0, tmpx2;

  /*  
return(0);
  */

  /* input info */
  rlen = bufin[0]; /* the number of 32bit data words */
  dcrb = (unsigned int *)&bufin[1]; /* first data word */

  /*
  if(rlen<=10) return(0);
  */

#ifndef VXWORKS
  /* swap input buffer (assume that data from VME is big-endian, and we are on little-endian Intel) */
  for(ii=0; ii<rlen; ii++) dcrb[ii] = LSWAP(dcrb[ii]);
#endif


#ifdef DEBUG
  printf("\n**********************\nmini-bos bank: rlen=%d\n",rlen);
#endif

  /*
return(0);
  */

  /* set pointer to the 3rd word of the output buffer (first 2 words will be CODA header filles by rol2) */
  dabufp = (unsigned int *)&bufout[2];

  /* set BOS pointer to output buffer */
  iw = (unsigned int *)&bufout[2];

  len = 0;
  b08 = NULL;
  /* set end_of_bufout pointer */
  endofbufout = (unsigned int *)(bufout + bosleftspace(iw));



  CPOPEN(0xe105,1,1);





/*extract v1290 info and calculate reference*/
#ifndef VXWORKS
  word = LSWAP(dcrb[0]);
#else
  word = dcrb[0];
#endif
  nw = (word>>5)&0xFFFF;
  /*printf("ROL2 TDC HEADER: 0x%08x (len=%d)\n",word,nw);*/
dabufp[0]=nw;
  for(ii=1; ii<nw; ii++)
  {
#ifndef VXWORKS
    word = LSWAP(dcrb[ii]);
#else
    word = dcrb[ii];
#endif
	ichan = (word>>21)&0x1F;
    tdc = word&0x1FFFFF;
    edge = (word>>26)&0x1;
	/*printf("ROL2 TDC DATA: 0x%08x (channel=%d tdc=%d edge=%d)\n",word,ichan,tdc,edge);*/
dabufp[ii]=word;

    if(edge==0)
	{
      if(ichan==0)
	  {
        tmpx0 = tdc/40;
	  }
	  else
	  {
        tmpx2 = tdc/40;

        dcrbref = tmpx0-tmpx2;
        /*printf("dcrbref=%d\n",dcrbref);*/

      }
    }


  }
  dcrb += nw;
  rlen -= nw;
dabufp+=nw;
b08+=(nw*4);
/*end if v1290*/





  /* all banks including headers are created already in rol1, just copy here */
  for(ii=0; ii<rlen; ii++)
  {
    dabufp[ii] = dcrb[ii];
    b08 += 4;
    word = dcrb[ii];
#ifdef DEBUG
    printf("[%5d] 0x%08x -> ",ii,word);
#endif
	if(word & 0x80000000)
	{
	  type = (word>>27)&0xF;
	  switch(type)
	  {
		case DC_DATA_BLOCK_HEADER:
#ifdef DEBUG
		  printf(" {BLKHDR} SLOTID: %d", (word>>22)&0x1f);
		  printf(" NEVENTS: %d", (word>>11)&0x7ff);
		  printf(" BLOCK: %d\n", (word>>0)&0x7ff);
#endif
		  break;
		case DC_DATA_BLOCK_TRAILER:
#ifdef DEBUG
		  printf(" {BLKTLR} SLOTID: %d", (word>>22)&0x1f);
		  printf(" NWORDS: %d\n", (word>>0)&0x3fffff);
#endif
		  break;
		case DC_DATA_EVENT_HEADER:
#ifdef DEBUG
		  printf(" {EVTHDR} EVENT: %d\n", (word>>0)&0x7ffffff);
#endif
		  break;
		case DC_DATA_TRIGGER_TIME:
		  timestamp = (((unsigned long long)word&0xffffff)<<24) | (dcrb[ii+1]&0xffffff);
#ifdef DEBUG
		  printf(" {TRGTIME} timestamp=%lld ns (%lld us)\n",timestamp,timestamp/(long long)1000);
#endif
		  break;
		case DC_DATA_DCRBEVT:
#ifdef DEBUG
		  printf(" {DCRBEVT}");
		  printf(" CH: %3u", (word>>16)&0x7F);
		  printf(" TDC: %6u\n", word&0xFFFF);
#endif
		  break;
		case DC_DATA_INVALID:
		  printf(" {***DNV***}\n");
          goto exit;
		  break;
		case DC_DATA_FILLER:
#ifdef DEBUG
		  printf(" {FILLER}\n");
#endif
		  break;
		default:
		  printf(" {***DATATYPE ERROR***}\n");
          goto exit;
		  break;
	  }
	}
#ifdef DEBUG
	else
	{
	  printf("\n");
	}
#endif
  }

exit:

  CPCLOSE;
  
  /* returns full fragment length (long words) */  

#ifdef DEBUG 
  printf("return len=%d\n**********************\n\n",len);
#endif

  return(len);
}
