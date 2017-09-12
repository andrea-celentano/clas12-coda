
/* ttdcrbgtp.c - translate CLAS12 DCRBGTP data into EVIO format */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "coda.h"
#include "tt.h"


#define DEBUG
#undef DEBUG1

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
  printf("CPCLOSE: 0x%x %d --- 0x%x %d --> padding %d\n",dabufp,dabufp,b08,b08,((dabufp_save1[1])>>14)&0x3); \
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

  Routine             : TT_TranslateDCRBGTPBank

  Parameters          : bufin - input(CODA from ROL1), bufout - output(EVIO)

  Discription         : This routine translates one event from crate with CLAS12 DCRBGTP
                        
***********************************************************************************/

 /* 22-max# of blocks=slots, 256-max# of events*/
#define MAXBLOCK 22
#define MAXEVENT 256

int
TT_TranslateDCRBGTPBank(long *bufin, long *bufout, TTSPtr ttp)
{
  int nB, iB[MAXBLOCK], sB[MAXBLOCK];
  int nE[MAXBLOCK], iE[MAXBLOCK][MAXEVENT], lenE[MAXBLOCK][MAXEVENT];
  unsigned int *iw, *dcrbgtp, *endofbufout;
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

  unsigned int *dabufp_save1;
  unsigned int *StartOfBank;
  unsigned int *dabufp, word;
  char *ch;
  unsigned char *b08;
  unsigned short *b16;
  unsigned int *b32, *Nchan, *Npuls, *Nsamp;
  unsigned long long *b64, timestamp;
  int dcrbref;
  int islot, ichan, ii, jj, npulses, type, nw, tdc, edge, tmpx0, tmpx2;
#ifdef DEBUG
  int k1, k2, k3;
  unsigned char sl[2][16][112], sl2[2][16][112];
#endif

  /* input info */
  rlen = bufin[0]; /* the number of 32bit data words */
  dcrbgtp = (unsigned int *)&bufin[1]; /* first data word */

#ifndef VXWORKS
  /* swap input buffer (assume that data from VME is big-endian, and we are on little-endian Intel) */
  /*tcpvme already converted everything to local endianess
  for(ii=0; ii<rlen; ii++) dcrbgtp[ii] = LSWAP(dcrbgtp[ii]);
  */
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


  CPOPEN(0xe108,1,1);


  /* all banks including headers are created already in rol1, just copy here */
  for(ii=0; ii<rlen; ii++)
  {
    dabufp[ii] = dcrbgtp[ii];
    b08 += 4;
    word = dcrbgtp[ii];
#ifdef DEBUG
    printf("data word [%5d] = 0x%08x\n",ii,word);
#endif
	if(ii==0) /* gtp event number */
	{
      printf("\n\nGTP event %d\n",word);
	}
	else if(ii==1) /* first trigger time word */
	{
      ;
	}
	else if(ii==2) /* second trigger time word */
	{
	  timestamp = (((unsigned long long)dcrbgtp[ii]&0xffff)<<32) | (dcrbgtp[ii-1]);
#ifdef DEBUG
	  printf("GTP timestamp=%lld ns (%lld us)\n",timestamp,timestamp/(long long)1000);
#endif
	}
  }

exit:

  CPCLOSE;
  
  /* returns full fragment length (long words) */  

#ifdef DEBUG
  {
    unsigned int *gsegm;

    for(ii=0; ii<16; ii++)
    {
      for(jj=0; jj<112; jj++)
      {
        sl[0][ii][jj] = 0;
        sl[1][ii][jj] = 0;
	  }
      k1 = k2 = k3 = 0;
    }

    gsegm = (unsigned int *) &dcrbgtp[3];


        k3 = 0; /*2 superlayers*/
        k2 = 0; /*16 angles*/
        k1 = 0; /*112 wires*/
        for(ii=0; ii<112; ii++)
		{ 
#ifdef DEBUG1
          printf("===> gsegm[%3d] = 0x%08x\n",ii,gsegm[ii]);
#endif
          if(ii==56)
		  {
            k3 = 1;
            k1 = k1 - 112;
		  }

          for(k2=0; k2<16; k2++)
	      {
#ifdef DEBUG1
            printf("ii=%d k1=%d k2=%d k3=%d\n",ii,k1,k2,k3);
#endif
            if(gsegm[ii]&(1<<k2))
			{
              sl[k3][k2][k1] = 1;
#ifdef DEBUG1
              printf("---> HIT !!!!!!!!!!!!!!!!!!!!!\n");
#endif
			}
	      }

          for(k2=16; k2<32; k2++)
	      {
#ifdef DEBUG1
            printf("ii=%d k1=%d k2=%d k3=%d\n",ii,k1+1,k2-16,k3);
#endif
            if(gsegm[ii]&(1<<k2))
			{
              sl[k3][k2-16][k1+1] = 1;
#ifdef DEBUG1
              printf("---> HIT !!!!!!!!!!!!!!!!!!!!!\n");
#endif
			}
	      }

          k1+=2;
		}

        printf("\n");
        for(ii=0; ii<112; ii++)
		{
          if(!(ii%10) && ii>0 && ii<100) printf("%2d",ii);
          else if((ii%9)) printf("+");
		}
        printf("+++\n");
        for(k3=1; k3>=0; k3--) /* 2 sl */
        {
          for(k2=15; k2>=0; k2--) /* 16 angles */
          {
            for(k1=0; k1<112; k1++) /* 112 wires */
            {
              if(sl[k3][k2][k1]==0) printf(" ");
              else printf("X");
            }
            printf("\n");
  	      }
          printf("\n");
          for(ii=0; ii<112; ii++) printf("+");
          printf("\n");
        }
        printf("\n");
  }

#endif

  return(len);
}
