
/* tttdc.c - translate CLAS12 CAEN v1190/v1290 data into EVIO format */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "coda.h"
#include "tt.h"


#undef DEBUG

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





static char* Version = " VERSION: ttvme.c v1.0 - initial release " ;
static char* CompileTime = DAYTIME;

/***********************************************************************************

  Routine             : TT_TranslateTDCBank

  Parameters          : bufin - input(CODA from ROL1), bufout - output(EVIO)

  Discription         : This routine translates one event from crate with CLAS12 TDC
                        
***********************************************************************************/

 /* 22-max# of blocks=slots, 256-max# of events*/
#define MAXBLOCK 22
#define MAXEVENT 256

int
TT_TranslateTDCBank(long *bufin, long *bufout, TTSPtr ttp)
{
  int nB, iB[MAXBLOCK], sB[MAXBLOCK];
  int nE[MAXBLOCK], iE[MAXBLOCK][MAXEVENT], lenE[MAXBLOCK][MAXEVENT];
  unsigned int *iw, *data, *endofbufout;
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
  int tdcref;
  int islot, ichan, ii, jj, npulses, type, nw, tdc, edge, tmpx0, tmpx2;

  /*  
return(0);
  */

  /* input info */
  rlen = bufin[0]; /* the number of 32bit data words */
  data = (unsigned int *)&bufin[1]; /* first data word */

  /*
  if(rlen<=10) return(0);
  */

#ifndef VXWORKS
  /* swap input buffer (assume that data from VME is big-endian, and we are on little-endian Intel) */
  for(ii=0; ii<rlen; ii++) data[ii] = LSWAP(data[ii]);
#endif


#ifdef DEBUG
  printf("\n**********************\ntttdc: mini-bos bank: rlen=%d\n",rlen);
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


  CPOPEN(0xe107,1,10);

  while(rlen>0)
  {
#ifndef VXWORKS
    word = LSWAP(data[0]);
#else
    word = data[0];
#endif
    nw = (word>>5)&0xFFFF;

#ifdef DEBUG
    printf("tttdc: rlen=%d, nw=%d\n",rlen,nw);
#endif

    if(nw>0)
    {
#ifdef DEBUG
      printf("tttdc: ROL2 TDC HEADER: 0x%08x (len=%d)\n",word,nw);
#endif
      for(ii=1; ii<nw; ii++)
      {
#ifndef VXWORKS
        word = LSWAP(data[ii]);
#else
        word = data[ii];
#endif
        islot = (word>>27)&0x1F;

        if(ttp->roc==19)
        {
		  /*v1190*/
          ichan = (word>>19)&0x7F;
          tdc = word&0x7FFFF;
		}
        else
		{
          /*v1290*/
	      ichan = (word>>21)&0x1F;
          tdc = word&0x1FFFFF;
		}

        edge = (word>>26)&0x1;
#ifdef DEBUG
	    printf("tttdc: ROL2 TDC DATA: 0x%08x (slot=%d channel=%d tdc=%d edge=%d)\n",word,islot,ichan,tdc,edge);
#endif

        /* output data in 'standard' format: tdc[0-18], chan[19-25], edge[26], slot[27-31] */
        *dabufp++ = (tdc&0x3FFFF) + (ichan<<19) + (edge<<26) + (islot<<27);
        b08 += 4;
      }
      data += nw;
      rlen -= nw;
    }
  }

  CPCLOSE;

  /* returns full fragment length (long words) */  

#ifdef DEBUG 
  printf("tttdc: return len=%d\n**********************\n\n",len);
#endif

  return(len);
}
