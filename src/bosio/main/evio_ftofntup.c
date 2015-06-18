
/* evio_ftofntup.c */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "evio.h"

#undef DEBUG_SEARCH

#undef DEBUG
 
#define NWPAWC 20000000 /* Length of the PAWC common block. */
#define LREC 1024      /* Record length in machine words. */

struct {
  float hmemor[NWPAWC];
} pawc_;


#define MAXEVENTS 30000000
#define MAXNTUPEVENTS 100000 /* max events in one ntuple file */

#define MAXBUF 10000000
unsigned int buf[MAXBUF];

#define SWAP32(x) ( (((x) >> 24) & 0x000000FF) | \
                    (((x) >> 8)  & 0x0000FF00) | \
                    (((x) << 8)  & 0x00FF0000) | \
                    (((x) << 24) & 0xFF000000) )

#define PRINT_BUFFER \
  b08 = start; \
  while(b08<end) \
  { \
    GET32(tmp); \
    printf("== 0x%08x\n",tmp); \
  } \
  b08 = start

#define GET8(ret_val) \
  ret_val = *b08++

#define GET16(ret_val) \
  b16 = (unsigned short *)b08; \
  ret_val = *b16; \
  b08+=2

#define GET32(ret_val) \
  b32 = (unsigned int *)b08; \
  ret_val = *b32; \
  b08+=4

#define GET64(ret_val) \
  b64 = (unsigned long long *)b08; \
  ret_val = *b64; \
  b08+=8

int
evNlink(unsigned int *buf, int frag, int tag, int num, int *nbytes)
{
  int ii, len, nw, tag1, pad1, typ1, num1, len2, pad3, ind;
  int right_frag = 0;


#ifdef DEBUG_SEARCH
  printf("0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
		 buf[0],buf[1],buf[2],buf[3],buf[4],buf[5]);
  printf("%d %d %d %d %d %d\n",
		 buf[0],buf[1],buf[2],buf[3],buf[4],buf[5]);
#endif

  len = buf[0]+1;
  ii = 2;
  while(ii<len)
  {
    nw = buf[ii] + 1;
    tag1 = (buf[ii+1]>>16)&0xffff;
    pad1 = (buf[ii+1]>>14)&0x3;
    typ1 = (buf[ii+1]>>8)&0x3f;
    num1 =  buf[ii+1]&0xff;
#ifdef DEBUG_SEARCH
    printf("[%5d] nw=%d, tag1=0x%04x, pad1=0x%02x, typ1=0x%02x, num1=0x%02x\n",ii,nw,tag1,pad1,typ1,num1);
#endif
    /*check if it is right fragment*/
    if(typ1==0xe || typ1==0x10)
	{
      if(tag1==frag)
      {
#ifdef DEBUG_SEARCH
        printf("right frag\n");
#endif
        right_frag = 1;
      }
	  else
      {
#ifdef DEBUG_SEARCH
        printf("wrong frag\n");
#endif
        right_frag = 0;
      }
    }

#ifdef DEBUG_SEARCH
    printf("search ==> %d=1?  %d=%d?  %d=%d?\n",right_frag,tag1,tag,num1,num);
#endif
    if(typ1!=0xe && typ1!=0x10) /*assumes there are no bank-of-banks inside fragment, will redo later*/
	{
    if( right_frag==1 && tag1==tag && num1==num )
    {
      if(typ1!=0xf)
	  {
#ifdef DEBUG_SEARCH
        printf("return primitive bank data index %d\n",ii+2);
#endif
        *nbytes = (nw-2)<<2;
        return(ii+2);
	  }
      else
      {
        len2 = (buf[ii+2]&0xffff) + 1; /* tagsegment length (tagsegment contains format description) */
        ind = ii + len2+2; /* internal bank */
        pad3 = (buf[ind+1]>>14)&0x3; /* padding from internal bank */
#ifdef DEBUG_SEARCH
		printf(">>> found composite bank: tag=%d, type=%d, exclusive len=%d (padding from internal bank=%d)\n",((buf[ii+2]>>20)&0xfff),((buf[ii+2]>>16)&0xf),len2-1,pad3);
        printf("return composite bank data index %d\n",ii+2+len2+2);
#endif
        *nbytes = ((nw-(2+len2+2))<<2)-pad3;
#ifdef DEBUG_SEARCH
		printf(">>> nbytes=%d\n",*nbytes);
#endif
        return(ii+2+len2+2);
      }
    }
	}

    if(typ1==0xe || typ1==0x10) ii += 2; /* bank of banks */
    else ii += nw;
  }

  return(0);
}






/*translation tables: args - board#/chan, funs - layer/slab */

/* adcs */

/*layers from 1 to 3*/
static int adclayer[22][16] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2,
  1, 1, 1, 1, 1, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 3,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  3, 3, 3, 3, 3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*slab numbers from 1 to 23*/
static int adcslab[22][16] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 3, 1, 3, 5, 7, 5, 7, 9,11, 9,11,13,15,13,15,
  2, 4, 2, 4, 6, 8, 6, 8,10,12,10,12,14,16,14,16,
 17,19,17,19,21,23,21,23, 2, 4, 2, 4, 6, 8, 6, 8,
 18,20,18,20,22, 1,22, 1, 3, 5, 3, 5, 7, 9, 7, 9,
 10,12,10,12,14,16,14,16,18,20,18,20,22, 1,22, 1,
 11,13,11,13,15,17,15,17,19,21,19,21,23, 2,23, 2,
  3, 5, 3, 5, 7, 9, 7, 9,11,13,11,13,15,17,15,17,
  4, 6, 4, 6, 8,10, 8,10,12,14,12,14,16,18,16,18,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 19,21,19,21,23, 0,23, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 20,22,20,22, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*1-left,2-right*/
static int adclr[22][16] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2,
  1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2,
  1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2,
  1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2,
  1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2,
  1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2,
  1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2,
  1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 2, 2, 1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};



/* tdcs */

/*layers from 1 to 3*/
static int tdclayer[22][32] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 3,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  3, 3, 3, 3, 3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*slab numbers from 1 to 23*/
static int tdcslab[22][32] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 3, 1, 3, 5, 7, 5, 7, 9,11, 9,11,13,15,13,15, 2, 4, 2, 4, 6, 8, 6, 8,10,12,10,12,14,16,14,16,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 17,19,17,19,21,23,21,23, 2, 4, 2, 4, 6, 8, 6, 8,18,20,18,20,22, 1,22, 1, 3, 5, 3, 5, 7, 9, 7, 9,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 10,12,10,12,14,16,14,16,18,20,18,20,22, 1,22, 1,11,13,11,13,15,17,15,17,19,21,19,21,23, 2,23, 2,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  3, 5, 3, 5, 7, 9, 7, 9,11,13,11,13,15,17,15,17, 4, 6, 4, 6, 8,10, 8,10,12,14,12,14,16,18,16,18,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 19,21,19,21,23, 0,23, 0, 0, 0, 0, 0, 0, 0, 0, 0,20,22,20,22, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*1-left,2-right*/
static int tdclr[22][32] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 2, 2, 1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};



/* translated data */

#define NHITS 100
static int nadc[3][2][23], adc[3][2][23][NHITS];
static int ntdc[3][2][23], tdc[3][2][23][NHITS];


#define ABS(x) ((x) < 0 ? -(x) : (x))

#define NCHAN 256 /*how many channels to draw*/

#define TDCRES 41.66667
#define TDCLSB 24

#define LENEVENT 277 /* for NTuple: the number of 32-bit words in event */

int
main(int argc, char **argv)
{
  FILE *fd = NULL;
  int bco1[256], bco2[256], bco3[256], bco4[256], nbco1, nbco2, nbco3, nbco4, diff, diff1, diff2;
  int nfile;
  char filename[1024];
  int handler, status, ifpga, nchannels, tdcref;
  unsigned long long *b64, timestamp, timestamp_old;
  unsigned int *b32;
  unsigned short *b16;
  unsigned char *b08;
  int trig,chan,fpga,apv,hybrid;
  int i1, type, timestamp_flag;
  float f1,f2;
  unsigned int word;

  /* Ntuple definitions */
  int ntup;
  int lenevent = LENEVENT;
  int nprime = 4096;
  char chtags[LENEVENT][8] = {
    "iev     ",
	"tdcS1L01",
	"tdcS1L02",
	"tdcS1L03",
	"tdcS1L04",
	"tdcS1L05",
	"tdcS1L06",
	"tdcS1L07",
	"tdcS1L08",
	"tdcS1L09",
	"tdcS1L10",
	"tdcS1L11",
	"tdcS1L12",
	"tdcS1L13",
	"tdcS1L14",
	"tdcS1L15",
	"tdcS1L16",
	"tdcS1L17",
	"tdcS1L18",
	"tdcS1L19",
	"tdcS1L20",
	"tdcS1L21",
	"tdcS1L22",
	"tdcS1L23",
	"tdcS1R01",
	"tdcS1R02",
	"tdcS1R03",
	"tdcS1R04",
	"tdcS1R05",
	"tdcS1R06",
	"tdcS1R07",
	"tdcS1R08",
	"tdcS1R09",
	"tdcS1R10",
	"tdcS1R11",
	"tdcS1R12",
	"tdcS1R13",
	"tdcS1R14",
	"tdcS1R15",
	"tdcS1R16",
	"tdcS1R17",
	"tdcS1R18",
	"tdcS1R19",
	"tdcS1R20",
	"tdcS1R21",
	"tdcS1R22",
	"tdcS1R23",
	"tdcS5L01",
	"tdcS5L02",
	"tdcS5L03",
	"tdcS5L04",
	"tdcS5L05",
	"tdcS5L06",
	"tdcS5L07",
	"tdcS5L08",
	"tdcS5L09",
	"tdcS5L10",
	"tdcS5L11",
	"tdcS5L12",
	"tdcS5L13",
	"tdcS5L14",
	"tdcS5L15",
	"tdcS5L16",
	"tdcS5L17",
	"tdcS5L18",
	"tdcS5L19",
	"tdcS5L20",
	"tdcS5L21",
	"tdcS5L22",
	"tdcS5L23",
	"tdcS5R01",
	"tdcS5R02",
	"tdcS5R03",
	"tdcS5R04",
	"tdcS5R05",
	"tdcS5R06",
	"tdcS5R07",
	"tdcS5R08",
	"tdcS5R09",
	"tdcS5R10",
	"tdcS5R11",
	"tdcS5R12",
	"tdcS5R13",
	"tdcS5R14",
	"tdcS5R15",
	"tdcS5R16",
	"tdcS5R17",
	"tdcS5R18",
	"tdcS5R19",
	"tdcS5R20",
	"tdcS5R21",
	"tdcS5R22",
	"tdcS5R23",
	"tdcS6L01",
	"tdcS6L02",
	"tdcS6L03",
	"tdcS6L04",
	"tdcS6L05",
	"tdcS6L06",
	"tdcS6L07",
	"tdcS6L08",
	"tdcS6L09",
	"tdcS6L10",
	"tdcS6L11",
	"tdcS6L12",
	"tdcS6L13",
	"tdcS6L14",
	"tdcS6L15",
	"tdcS6L16",
	"tdcS6L17",
	"tdcS6L18",
	"tdcS6L19",
	"tdcS6L20",
	"tdcS6L21",
	"tdcS6L22",
	"tdcS6L23",
	"tdcS6R01",
	"tdcS6R02",
	"tdcS6R03",
	"tdcS6R04",
	"tdcS6R05",
	"tdcS6R06",
	"tdcS6R07",
	"tdcS6R08",
	"tdcS6R09",
	"tdcS6R10",
	"tdcS6R11",
	"tdcS6R12",
	"tdcS6R13",
	"tdcS6R14",
	"tdcS6R15",
	"tdcS6R16",
	"tdcS6R17",
	"tdcS6R18",
	"tdcS6R19",
	"tdcS6R20",
	"tdcS6R21",
	"tdcS6R22",
	"tdcS6R23",
	"adcS1L01",
	"adcS1L02",
	"adcS1L03",
	"adcS1L04",
	"adcS1L05",
	"adcS1L06",
	"adcS1L07",
	"adcS1L08",
	"adcS1L09",
	"adcS1L10",
	"adcS1L11",
	"adcS1L12",
	"adcS1L13",
	"adcS1L14",
	"adcS1L15",
	"adcS1L16",
	"adcS1L17",
	"adcS1L18",
	"adcS1L19",
	"adcS1L20",
	"adcS1L21",
	"adcS1L22",
	"adcS1L23",
	"adcS1R01",
	"adcS1R02",
	"adcS1R03",
	"adcS1R04",
	"adcS1R05",
	"adcS1R06",
	"adcS1R07",
	"adcS1R08",
	"adcS1R09",
	"adcS1R10",
	"adcS1R11",
	"adcS1R12",
	"adcS1R13",
	"adcS1R14",
	"adcS1R15",
	"adcS1R16",
	"adcS1R17",
	"adcS1R18",
	"adcS1R19",
	"adcS1R20",
	"adcS1R21",
	"adcS1R22",
	"adcS1R23",
	"adcS5L01",
	"adcS5L02",
	"adcS5L03",
	"adcS5L04",
	"adcS5L05",
	"adcS5L06",
	"adcS5L07",
	"adcS5L08",
	"adcS5L09",
	"adcS5L10",
	"adcS5L11",
	"adcS5L12",
	"adcS5L13",
	"adcS5L14",
	"adcS5L15",
	"adcS5L16",
	"adcS5L17",
	"adcS5L18",
	"adcS5L19",
	"adcS5L20",
	"adcS5L21",
	"adcS5L22",
	"adcS5L23",
	"adcS5R01",
	"adcS5R02",
	"adcS5R03",
	"adcS5R04",
	"adcS5R05",
	"adcS5R06",
	"adcS5R07",
	"adcS5R08",
	"adcS5R09",
	"adcS5R10",
	"adcS5R11",
	"adcS5R12",
	"adcS5R13",
	"adcS5R14",
	"adcS5R15",
	"adcS5R16",
	"adcS5R17",
	"adcS5R18",
	"adcS5R19",
	"adcS5R20",
	"adcS5R21",
	"adcS5R22",
	"adcS5R23",
	"adcS6L01",
	"adcS6L02",
	"adcS6L03",
	"adcS6L04",
	"adcS6L05",
	"adcS6L06",
	"adcS6L07",
	"adcS6L08",
	"adcS6L09",
	"adcS6L10",
	"adcS6L11",
	"adcS6L12",
	"adcS6L13",
	"adcS6L14",
	"adcS6L15",
	"adcS6L16",
	"adcS6L17",
	"adcS6L18",
	"adcS6L19",
	"adcS6L20",
	"adcS6L21",
	"adcS6L22",
	"adcS6L23",
	"adcS6R01",
	"adcS6R02",
	"adcS6R03",
	"adcS6R04",
	"adcS6R05",
	"adcS6R06",
	"adcS6R07",
	"adcS6R08",
	"adcS6R09",
	"adcS6R10",
	"adcS6R11",
	"adcS6R12",
	"adcS6R13",
	"adcS6R14",
	"adcS6R15",
	"adcS6R16",
	"adcS6R17",
	"adcS6R18",
	"adcS6R19",
	"adcS6R20",
	"adcS6R21",
	"adcS6R22",
	"adcS6R23"
  };
  float event[LENEVENT];

  /*arg-slot(from 0), fun-histnum*/
  /*3,4,5,6,7,8,9,10,13,14*/
  int adcslot2hist[22] = {0,0,0,1,2,3,4,5,6,7,8,0,0,9,10,0,0,0,0,0,0,0};
  /*3,6,9,13,16*/
  int tdcslot2hist[22] = {0,0,0,1,0,0,2,0,0,3,0,0,0,4,0,0,5,0,0,0,0,0};

  int nr,sec,strip,nl,ncol,nrow,i,j, k, ii,jj,kk,l,l1,l2,ichan,nn,mm,iev,nbytes,ind1;
  char title[128], *ch;
  char HBOOKfilename[256], chrunnum[32];
  int ntuplefilecount, runnum;
  int nwpawc,lun,lrec,istat,icycle,idn,nbins,nbins1,igood,offset;
  float x1,x2,y1,y2,ww,tmpx,tmpy,ttt,ref;
  /*
  int goodevent, icedev;
  */
  if(argc != 2)
  {
    printf("Usage: evio_ftofntup <evio_filename>\n");
    exit(1);
  }


  nwpawc = NWPAWC;
  hlimit_(&nwpawc);
  lun = 11;
  lrec = LREC;

  

  iev = 0;
  nfile = 0;
  ntuplefilecount = 0;



  strcpy(chrunnum,argv[1]);
  ch = strchr(chrunnum,'0');
  ch[6] = '\0';
  runnum = atoi(ch);
  printf("run number is %s (%d)\n",ch,runnum);






while(1)
{
  sprintf(filename,"%s.%d",argv[1],nfile++);
  printf("opening data file >%s<\n",filename);

  status = evOpen(filename,"r",&handler);
  printf("status=%d, run number %d\n",status,runnum);

  if(status!=0)
  {
    printf("evOpen error %d - exit\n",status);

      printf("Close ntyple file (event %d)\n",iev);

      idn = 0;
      printf("befor hrout_\n");fflush(stdout);
      hrout_(&idn,&icycle," ",1);
      printf("after hrout_\n");fflush(stdout);
      hrend_("NTUPEL", 6);
      printf("after hrend_\n");fflush(stdout);


    break;
  }

  timestamp_old = 0;

  while(iev<MAXEVENTS)
  {
    iev ++;

    if(!(iev%10000)) printf("\n\n\nEvent %d\n\n",iev);
#ifdef DEBUG
    printf("\n\n\nEvent %d\n\n",iev);
#endif



    status = evRead(handler, buf, MAXBUF);
    if(status < 0)
	{
	  if(status==EOF)
	  {
        printf("evRead: end of file after %d events - exit\n",iev);
        break;
	  }
	  else
	  {
        printf("evRead error=%d after %d events - exit\n",status,iev);
        break;
	  }
    }


    /*if(iev < 3) continue;*/ /*skip first 2 events*/

    for(i=0; i<LENEVENT; i++) event[i] = 0.0;
    for(ii=0; ii<3; ii++)
    {
      for(jj=0; jj<2; jj++)
      {
        for(kk=0; kk<23; kk++)
        {
          nadc[ii][jj][kk] = 0;
          ntdc[ii][jj][kk] = 0;
	    }
	  }
    }
    tdcref = 0;

	/*TDCs*/
    if((ind1 = evNlink(buf, 28, 0xe107, 10, &nbytes)) > 0)
    {
      int half,chip,chan,bco,val,chan1,edge,nw,tdcl,tdcr;
      unsigned char *end, *start;
      unsigned int tmp;
      float tmpx0, tmpx2, dcrbref;
      unsigned int temp[6];
      unsigned sample[6];
      int slot;
      int ndata0[22], data0[21][8];
      int baseline, sum, channel, ch1;
#ifdef DEBUG
      printf("ind1=%d, nbytes=%d\n",ind1,nbytes);fflush(stdout);
#endif
      start = b08 = (unsigned char *) &buf[ind1];
      end = b08 + nbytes;
#ifdef DEBUG
      printf("ind1=%d, nbytes=%d (from 0x%08x to 0x%08x)\n",ind1,nbytes,b08,end);fflush(stdout);
#endif
      tdcl = tdcr = 0;
      while(b08<end)
	  {
        GET32(word);
        slot = (word>>27)&0x1F;
        edge = (word>>26)&0x1;
	    chan = (word>>19)&0x3F;
        val = (word&0x3FFFF)*TDCLSB;

		/*if(slot==16) printf("-- %d(%d) %d %d(%f)\n",slot,tdcslot2hist[slot],chan,val,tmpx);*/
        if(tdcslot2hist[slot]==5 && chan==31)
		{
          if(tdcref==0)
		  {
            tdcref=val;
            /*printf("tdcref=%d\n",tdcref);*/
		  }
          else
		  {
            printf("ERROR: double tdcref = %d, already defined as %d\n",val,tdcref);
		  }
		}

        ii = tdclayer[slot][chan] - 1;
        jj = tdclr[slot][chan] - 1;
        kk = tdcslab[slot][chan] - 1;
        if(ii>=0)
		{
		  /*
		  printf("1=> %d %d %d\n",ii,jj,kk);fflush(stdout);
		  printf("1==> %d\n",ntdc[ii][jj][kk]);fflush(stdout);
		  */
          tdc[ii][jj][kk][ntdc[ii][jj][kk]] = val;
          ntdc[ii][jj][kk] ++;
		}

#ifdef DEBUG
        printf("TDC[0x%08x]:  slot=%2d  chan=%3d  edge=%1d  tdc=%5d  (hist_id=%d)\n",
          word,slot,chan,edge,val,idn);
#endif
	  }
	}


	/* ADC raw mode bank */
    if((ind1 = evNlink(buf, 27, 0xe101, 10, &nbytes)) > 0)
    {
      unsigned char *end;
      unsigned long long time;
      int crate,slot,trig,nchan,chan,nsamples,notvalid,edge,val,data,count,ncol1,nrow1;
      int oldslot = 100;
      int ndata0[22], data0[21][8];
      int baseline, sum, channel, summing_in_progress;
      int datasaved[1000];
#ifdef DEBUG
      printf("ind1=%d, nbytes=%d\n",ind1,nbytes);
#endif
      b08 = (unsigned char *) &buf[ind1];
      end = b08 + nbytes;
#ifdef DEBUG
      printf("ind1=%d, nbytes=%d (from 0x%08x to 0x%08x)\n",ind1,nbytes,b08,end);
#endif
      while(b08<end)
      {
#ifdef DEBUG
        printf("begin while: b08=0x%08x\n",b08);
#endif
        GET8(slot);
        GET32(trig);
        GET64(time);
        GET32(nchan);
#ifdef DEBUG
        printf("slot=%d, trig=%d, time=%lld nchan=%d\n",slot,trig,time,nchan);
#endif
        for(nn=0; nn<nchan; nn++)
	    {
          GET8(chan);
          /*chan++;*/
          GET32(nsamples);
#ifdef DEBUG
          printf("  chan=%d, nsamples=%d\n",chan,nsamples);
#endif
          baseline = sum = summing_in_progress = 0;
          for(mm=0; mm<nsamples; mm++)
	      {
	        GET16(data);
            datasaved[mm] = data;

			/*printf("mm=%d data=%d\n",mm,data);*/
            if(mm<30) baseline += data;

            if(mm==30)
			{
              baseline = baseline / 30;
              /*printf("baseline=%d\n",baseline);*/
			}

            if(mm>35 && mm<100)
            {
              if(summing_in_progress==0 && data>(baseline+50))
			  {
                /*printf("open summing at mm=%d\n",mm);*/
                summing_in_progress = 1;
                sum += (datasaved[mm-3]-baseline);
                sum += (datasaved[mm-2]-baseline);
                sum += (datasaved[mm-1]-baseline);
			  }

              if(summing_in_progress>0 && data<baseline)
			  {
                /*printf("close summing at mm=%d\n",mm);*/
                summing_in_progress = -1;
			  }

              if(summing_in_progress>0)
			  {
                sum += (datasaved[mm]-baseline);
                /*printf("sum=%d (mm=%d)\n",sum,mm);*/
			  }

			}
	      }

		  if(slot < 15)
		  {
            ii = adclayer[slot][chan] - 1;
            jj = adclr[slot][chan] - 1;
            kk = adcslab[slot][chan] - 1;
			if(ii>=0 && sum > 0)
			{
			  /*
		printf("2=> %d %d %d\n",ii,jj,kk);fflush(stdout);
		printf("2==> %d\n",nadc[ii][jj][kk]);fflush(stdout);
			  */
              adc[ii][jj][kk][nadc[ii][jj][kk]] = sum;
              nadc[ii][jj][kk] ++;
			}
		  }

        }
#ifdef DEBUG
        printf("end loop: b08=0x%08x\n",b08);
#endif
      }
    }

	

	/* ADC pulsed mode bank */
    if((ind1 = evNlink(buf, 27, 0xe103, 10, &nbytes)) > 0)
    {
      unsigned short pulse_time;
      unsigned int pulse_integral;
      unsigned char *end;
      unsigned long long time;
      int crate,slot,trig,nchan,chan,npulses,notvalid,edge,data,count,ncol1,nrow1;
      int oldslot = 100;
      int ndata0[22], data0[21][8];
      int baseline, sum, channel;

      b08 = (unsigned char *) &buf[ind1];
      b16 = (unsigned short *) &buf[ind1];
      b32 = (unsigned int *) &buf[ind1];

      end = b08 + nbytes;
#ifdef DEBUG
      printf("ind1=%d, nbytes=%d (from 0x%08x to 0x%08x)\n",ind1,nbytes,b32,end);
#endif
      while(b08<end)
      {
#ifdef DEBUG
        printf("begin while: b08=0x%08x\n",b08);
#endif
        b08 = (unsigned char *)b32;
        slot = *b08 ++;

        b32 = (unsigned int *)b08;
        trig = *b32++;

        b64 = (unsigned long long *)b32;
        time = *b64++;

        b32 = (unsigned int *)b64;
        nchan = *b32++;
#ifdef DEBUG
        printf("slot=%d, trig=%d, time=%lld nchan=%d\n",slot,trig,time,nchan);
#endif
        for(nn=0; nn<nchan; nn++)
	    {
          b08 = (unsigned char *)b32;
          chan = (*b08 ++) /*+ 1*/;
      
          b32 = (unsigned int *)b08;
          npulses = *b32++;
#ifdef DEBUG
          printf("  chan=%d, npulses=%d\n",chan,npulses);
#endif
          for(mm=0; mm<npulses; mm++)
	      {
            b16 = (unsigned short *)b32;
            pulse_time = (*b16++)>>6;
            b32 = (unsigned int *)b16;
            pulse_integral = *b32++;
#ifdef DEBUG
            printf(" b32=0x%08x:  pulse_time=%d pulse_integral=%d\n",b32,pulse_time,pulse_integral);
#endif		
			if(slot < 15)
			{
              ii = adclayer[slot][chan] - 1;
              jj = adclr[slot][chan] - 1;
              kk = adcslab[slot][chan] - 1;
			  if(ii>=0)
			  {
			  /*
		printf("3=> %d %d %d\n",ii,jj,kk);fflush(stdout);
		printf("3==> %d\n",nadc[ii][jj][kk]);fflush(stdout);
			  */
                adc[ii][jj][kk][nadc[ii][jj][kk]] = pulse_integral;
                nadc[ii][jj][kk] ++;
		      }
		    }

	      }
	    }
        b08 = (unsigned char *)b32;
#ifdef DEBUG
        printf("end loop: b08=0x%08x\n",b08);
#endif
      }
    }


	/******************************************************************/
    /* at that point we have adc[][][][] and tdc[][][][] arrays ready */
	/******************************************************************/


	/* correct TDCs using reference signal */
    if(tdcref==0)
	{
      printf("ERROR: iev=%d -> there is no ref signal\n",iev);
	}
    else
	{
      for(ii=0; ii<3; ii++)
      {
        for(jj=0; jj<2; jj++)
        {
          for(kk=0; kk<23; kk++)
          {
            if(ntdc[ii][jj][kk]>1)
			{
              ntdc[ii][jj][kk]=1;
			  printf("ev %5d double tdc: %d %d %d -> %d\n",iev,ii,jj,kk,tdc[ii][jj][kk][1]);
			}
            for(nn=0; nn<ntdc[ii][jj][kk]; nn++)
		    {
			  /*printf("befor: tdc=%d tdcref=%d\n",tdc[ii][jj][kk][nn],tdcref);*/
              tdc[ii][jj][kk][nn] = tdc[ii][jj][kk][nn] + 250000 - tdcref;
			  /*printf("after: %d\n",tdc[ii][jj][kk][nn]);*/
		    }
	      }
	    }
      }
	}
	



    if(!((iev-1)%MAXNTUPEVENTS))
	{
      printf("Open new ntyple file (event %d)\n",iev);

      /* open HBOOK file and create NTUPLE */
      sprintf(HBOOKfilename,"ftofntup%d.his_%d",runnum,ntuplefilecount++);
      hropen_(&lun,"NTUPEL",HBOOKfilename,"N",&lrec,&istat,strlen("NTUPEL"),strlen(HBOOKfilename),1);
      if(istat)
      {
        printf("\aError: cannot open RZ file %s for writing.\n", HBOOKfilename);fflush(stdout);
        exit(0);
      }
      else
      {
        printf("RZ file >%s< opened for writing, istat = %d\n\n", HBOOKfilename, istat);fflush(stdout);
      }
      idn = 2;
      hbookn_(&idn,"FTOF",&lenevent,"//NTUPEL",&nprime,chtags,strlen("FTOF"),8,8);
	}


	/*for(ii=0; ii<277; ii++) printf("before [%3d] %f\n",ii,event[ii]);*/
	/* Ntuple */
    for(ii=0; ii<3; ii++)
    {
      for(jj=0; jj<2; jj++)
      {
        for(kk=0; kk<23; kk++)
        {
          ntup = 46*ii + 23*jj + kk + 1;
		  /*printf("tdc: ii=%d jj=%d kk=%d ntup=%d\n",ii,jj,kk,ntup);*/
          if(ntdc[ii][jj][kk]>0 && event[ntup]<=0.00001)
          {
            event[ntup] = (float)tdc[ii][jj][kk][0];
            /*printf("tdc=%f\n",event[ntup]);*/
          }
          ntup += 138;
		  /*printf("adc: ii=%d jj=%d kk=%d ntup=%d\n",ii,jj,kk,ntup);*/
          if(nadc[ii][jj][kk]>0 && event[ntup]<=0.00001)
          {
            event[ntup] = (float)adc[ii][jj][kk][0];
            /*printf("adc=%f\n",event[ntup]);*/
          }
		}
	  }
	}

    event[0] = iev;
    idn = 2;
	/*for(ii=0; ii<277; ii++) printf(" [%3d] %f\n",ii,event[ii]);*/
    hfn_(&idn,event);

    if(!(iev%MAXNTUPEVENTS))
	{
      printf("Close ntyple file (event %d)\n",iev);

      idn = 0;
      printf("befor hrout_\n");fflush(stdout);
      hrout_(&idn,&icycle," ",1);
      printf("after hrout_\n");fflush(stdout);
      hrend_("NTUPEL", 6);
      printf("after hrend_\n");fflush(stdout);
	}

  }




  printf("evClose %s after %d events\n",HBOOKfilename,iev);fflush(stdout);
  evClose(handler);

  if(iev>=MAXEVENTS) break;

} /*while(1)*/


  exit(0);
}
