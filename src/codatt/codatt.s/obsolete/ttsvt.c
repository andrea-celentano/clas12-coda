
/* ttsvt.c - translate CLAS12 SVT data into EVIO format */

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
  dabufp_save1 = dabufp ++; /*remember beginning of the bank address*/ \
  *dabufp++ = (btag<<16) + (btyp<<8) + bnum; /*bank header*/ \
  b08 = (unsigned char *)dabufp; \
}


#define CPCLOSE \
{ \
  unsigned int padding; \
  dabufp = (unsigned int *) ( ( ((unsigned int)b08+3)/4 ) * 4); \
  padding = (unsigned int)dabufp - (unsigned int)b08; \
  dabufp_save1[1] |= (padding&0x3)<<14; \
  /*printf("CPCLOSE: 0x%x %d --- 0x%x %d --> padding %d\n",dabufp,dabufp,b08,b08,((dabufp_save2[1])>>14)&0x3);*/ \
  *dabufp_save1 = (dabufp-dabufp_save1-1); \
  len += (*dabufp_save1+1); \
  b08 = NULL; \
}

#define DATA_TYPE_BLKHDR		0x00
#define DATA_TYPE_BLKTLR		0x01
#define DATA_TYPE_EVTHDR		0x02
#define DATA_TYPE_TRGTIME		0x03
#define DATA_TYPE_BCOTIME		0x04
#define DATA_TYPE_FSSREVT		0x08
#define DATA_TYPE_DNV			0x0E
#define DATA_TYPE_FILLER		0x0F


static char* Version = " VERSION: ttvme.c v1.0 - initial release " ;
static char* CompileTime = DAYTIME;

/***********************************************************************************

  Routine             : TT_TranslateSVTBank

  Parameters          : bufin - input(CODA from ROL1), bufout - output(EVIO)

  Discription         : This routine translates one event from crate with CLAS12 SVT
                        
***********************************************************************************/

 /* 22-max# of blocks=slots, 256-max# of events*/
#define MAXBLOCK 22
#define MAXEVENT 256

int
TT_TranslateSVTBank(long *bufin, long *bufout, TTSPtr ttp)
{
  int nB, iB[MAXBLOCK], sB[MAXBLOCK];
  int nE[MAXBLOCK], iE[MAXBLOCK][MAXEVENT], lenE[MAXBLOCK][MAXEVENT];
  unsigned int *iw, *svt, *endofbufout;
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
  unsigned int *dabufp, word, oldbco;
  char *ch;
  unsigned char *b08;
  unsigned short *b16;
  unsigned int *b32, *Nchan, *Npuls, *Nsamp;
  unsigned long long *b64, timestamp;
  int latency;
  int islot, ichan, ii, jj, npulses, type;

  /*
return(0);
  */

  /* input info */
  rlen = bufin[0]; /* the number of 32bit data words */
  svt = (unsigned int *)&bufin[1]; /* first data word */

  /*
  if(rlen<=10) return(0);
  */

#ifndef VXWORKS
  /* swap input buffer (assume that data from VME is big-endian, and we are on little-endian Intel) */
  for(ii=0; ii<rlen; ii++) svt[ii] = LSWAP(svt[ii]);
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


  CPOPEN(0xe104,1,1);

  /* all banks including headers are created already in rol1, just copy here */
  for(ii=0; ii<rlen; ii++)
  {
    dabufp[ii] = svt[ii];
    word = svt[ii];
    b08 += 4;
#ifdef DEBUG
    printf("[%5d] 0x%08x -> ",ii,word);
#endif
	if(word & 0x80000000)
	{
	  type = (word>>27)&0xF;
	  switch(type)
	  {
		case DATA_TYPE_BLKHDR:
#ifdef DEBUG
		  printf(" {BLKHDR} SLOTID: %d", (word>>22)&0x1f);
		  printf(" NEVENTS: %d", (word>>11)&0x7ff);
		  printf(" BLOCK: %d\n", (word>>0)&0x7ff);
#endif
		  break;
		case DATA_TYPE_BLKTLR:
#ifdef DEBUG
		  printf(" {BLKTLR} SLOTID: %d", (word>>22)&0x1f);
		  printf(" NWORDS: %d\n", (word>>0)&0x3fffff);
#endif
		  break;
		case DATA_TYPE_EVTHDR:
#ifdef DEBUG
		  printf(" {EVTHDR} EVENT: %d\n", (word>>0)&0x7ffffff);
#endif
		  break;
		case DATA_TYPE_TRGTIME:
		  timestamp = (((unsigned long long)word&0xffffff)<<24) | (svt[ii+1]&0xffffff);
#ifdef DEBUG
		  printf(" {TRGTIME} timestamp=%lld (bco style = %lld)\n",timestamp,((timestamp / (long long)(16)) % 256)); /*16-from par file*/
#endif
		  break;
        case DATA_TYPE_BCOTIME:
#ifdef DEBUG
          printf(" {BCOTIME} START: %u STOP: %u\n", (word>>0) & 0xFF, (word>>16) & 0xFF);
#endif
          break;
		case DATA_TYPE_FSSREVT:

          /* calculate 'latency' and put it in a data instead of 'bco' (26*8 is bco period (ns)) */
		  /*
          oldbco = (word>>4)&0xFF;
          latency = ABS( ((timestamp / (long long)(26*8)) % 256) - (long long)((word>>4)&0xFF) );
          dabufp[ii] = (dabufp[ii]&0xFFFFF00F)|((latency&0xff)<<4);
          word = dabufp[ii];
		  */

#ifdef DEBUG
		  printf(" {FSSREVT}");
		  printf(" HFCBID: %1u", (word>>22)&0x1);
		  printf(" CHIPID: %1u", (word>>19)&0x7);
		  printf(" CH: %3u", (word>>12)&0x7F);
		  printf(" BCO: %3u", (word>>4)&0xFF); /*printf(" BCO: %3u (old=%3u)", (word>>4)&0xFF, oldbco);*/
		  printf(" ADC: %1u", (word>>0)&0x7);
		  /*
          printf(" latency=%d BCO's (%d ns)",latency,latency*(26*8));
		  */
          printf("\n");
#endif
		  break;
		case DATA_TYPE_DNV:
		  printf(" {***DNV***}\n");
          goto exit;
		  break;
		case DATA_TYPE_FILLER:
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
#ifdef DEBUG
  printf("\n");
#endif

exit:

  CPCLOSE;



  /* returns full fragment length (long words) */  

#ifdef DEBUG 
  printf("return len=%d\n**********************\n\n",len);
#endif

  return(len);
}
