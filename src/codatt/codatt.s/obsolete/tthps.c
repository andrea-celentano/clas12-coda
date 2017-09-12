
/* tthps.c - translate HPS SVT data into EVIO composite format */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "coda.h"
#include "tt.h"


#undef DEBUG


#define LSWAP(x)        ((((x) & 0x000000ff) << 24) | \
                         (((x) & 0x0000ff00) <<  8) | \
                         (((x) & 0x00ff0000) >>  8) | \
                         (((x) & 0xff000000) >> 24))

#define CCOPEN(btag,fmt,bnum) \
{ \
  int len1, n1; \
  char *ch; \
  len1 = strlen(fmt); /* format length in bytes */ \
  n1 = (len1+5)/4; /* format length in words */ \
  dabufp_save1 = dabufp ++; /*remember '0xf' bank length location*/ \
  *dabufp++ = (btag<<16) + (0xf<<8) + bnum; /*bank header*/ \
  /* tagsegment header following by format */ \
  *dabufp++ = (len1<<20) + (0x6<<16) + n1; \
  ch = (char *)dabufp; \
  strncpy(ch,fmt,len1); \
  ch[len1]='\0';ch[len1+1]='\4';ch[len1+2]='\4';ch[len1+3]='\4';ch[len1+4]='\4'; \
  dabufp += n1; \
  /* 'internal' bank header */ \
  dabufp_save2 = dabufp ++;  /*remember 'internal' bank length location*/ \
  *dabufp++ = (0<<16) + (0x0<<8) + 0; \
  b08 = (unsigned char *)dabufp; \
}


#define CCCLOSE \
{ \
  unsigned int padding; \
  dabufp = (unsigned int *) ( ( ((unsigned int)b08+3)/4 ) * 4); \
  padding = (unsigned int)dabufp - (unsigned int)b08; \
  /*dabufp_save1[1] |= (padding&0x3)<<14;*/ \
  dabufp_save2[1] |= (padding&0x3)<<14; \
  /*printf("CCCLOSE: 0x%x %d --- 0x%x %d --> padding %d\n",dabufp,dabufp,b08,b08,((dabufp_save2[1])>>14)&0x3);*/ \
  *dabufp_save1 = (dabufp-dabufp_save1-1); \
  *dabufp_save2 = (dabufp-dabufp_save2-1); \
  len += (*dabufp_save1+1); \
  b08 = NULL; \
}

#ifdef VXWORKS
int mynev;
#else
extern int mynev; /*defined in tttrans.c */
#endif

static char* Version = " VERSION: ttvme.c v1.0 - initial release " ;
static char* CompileTime = DAYTIME;

/***********************************************************************************

  Routine             : TT_TranslateHPSBank

  Parameters          : bufin - input(CODA from ROL1), bufout - output(EVIO)

  Discription         : This routine translates one event from crate with HPS SVT
                        
***********************************************************************************/

 /* 22-max# of blocks=slots, 256-max# of events*/
#define MAXBLOCK 22
#define MAXEVENT 256

int
TT_TranslateHPSBank(long *bufin, long *bufout, TTSPtr ttp)
{
  int nB, iB[MAXBLOCK], sB[MAXBLOCK];
  int nE[MAXBLOCK], iE[MAXBLOCK][MAXEVENT], lenE[MAXBLOCK][MAXEVENT];
  unsigned int *iw, *adc, *endofbufout;
  int i, j, k, m, iii, ind, len, bank, nhits=0, mux_index, rlen, printing, nnE;
  int nr = 0;
  int ncol = 2;
  int a_channel, a_nevents, a_blocknumber, a_triggernumber;
  int a_windowwidth, a_pulsenumber, a_firstsample;
  int a_adc1, a_adc2, a_valid1, a_valid2, a_nwords, a_slot, a_slot2;
  int a_qualityfactor, a_pulseintegral, a_pulsetime;
  int a_trigtime[4];
  int a_slot_old;
  int a_channel_old;

  unsigned int *dabufp_save1, *dabufp_save2;
  unsigned int *StartOfBank;
  unsigned int *dabufp;
  char *ch;
  unsigned char *b08;
  unsigned short *b16;
  unsigned int *b32, *Nchan, *Npuls, *Nsamp;
  unsigned long long *b64;
  int islot, ichan, ii, jj, npulses;
  /*
  return(0);
  */

  /* input info */
  rlen = bufin[0]; /* the number of 32bit data words */
  adc = (unsigned int *)&bufin[1]; /* first data word */

  /* swap input buffer (assume that data from VME is big-endian, and we are on little-endian Intel)
  for(ii=0; ii<rlen; ii++) adc[ii] = LSWAP(adc[ii]);
  */

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


  /* all banks including headers are created already in rol1, just copy here */
  for(ii=0; ii<rlen; ii++)
  {
    dabufp[ii] = adc[ii];
#ifdef DEBUG
    printf("[%5d] 0x%08x (%d)\n",ii,adc[ii],adc[ii]);
#endif
  }
  len = rlen;

/*
  CCOPEN(0xe201,"i",0);
#ifdef DEBUG
  printf("0x%08x: CCOPEN(1)\n",b08);
#endif


  for(ii=0; ii<rlen; ii++)
  {
#ifdef DEBUG
    printf("[%5d] 0x%08x\n",ii,adc[ii]);
#endif
    b32 = (unsigned int *)b08;
    *b32 = adc[ii];
    b08 += 4;
  }


  if(b08 != NULL) CCCLOSE;
#ifdef DEBUG
  printf("0x%08x: CCCLOSE, dabufp=0x%08x\n",b08,dabufp);
  dabufp = (unsigned int *) ( ( ((unsigned int)b08+3)/4 ) * 4);
  printf("0x%08x: CCCLOSE, dabufp=0x%08x\n",b08,dabufp);
  printf("-> 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
		 (unsigned int)b08+3,((unsigned int)b08+3),
         ((unsigned int)b08+3) / 4,(((unsigned int)b08+3) / 4)*4, 
		 (unsigned int *)((((unsigned int)b08+3) / 4)*4) );
#endif
*/


  /* returns full fragment length (long words) */  

#ifdef DEBUG 
  printf("return len=%d\n**********************\n\n",len);
#endif

  return(len);
}
