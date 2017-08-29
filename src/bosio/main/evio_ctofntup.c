
/* evio_ctofntup.c */

#define USE_TEST_IN_HALL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "evio.h"


#include "et.h"
#define ET_EVENT_ARRAY_SIZE 100
static char et_name[ET_FILENAME_LENGTH];
static et_stat_id  et_statid;
static et_sys_id   et_sys;
static et_att_id   et_attach;
static int         et_locality, et_init = 0, et_reinit = 0;
static et_event *pe[ET_EVENT_ARRAY_SIZE];
static int done;
static int PrestartCount, prestartEvent=17, endEvent=20;
static int use_et = 0;

/* ET Initialization */    
int
et_initialize(void)
{
  et_statconfig   sconfig;
  et_openconfig   openconfig;
  int             status;
  struct timespec timeout;

  timeout.tv_sec  = 2;
  timeout.tv_nsec = 0;

  /* Normally, initialization is done only once. However, if the ET
   * system dies and is restarted, and we're running on a Linux or
   * Linux-like operating system, then we need to re-initalize in
   * order to reestablish the tcp connection for communication etc.
   * Thus, we must undo some of the previous initialization before
   * we do it again.
   */
  if(et_init > 0)
  {
    /* unmap shared mem, detach attachment, close socket, free et_sys */
    et_forcedclose(et_sys);
  }
  
  printf("er_et_initialize: start ET stuff\n");

  if(et_open_config_init(&openconfig) != ET_OK)
  {
    printf("ERROR: er ET init: cannot allocate mem to open ET system\n");
    return(-1);;
  }
  et_open_config_setwait(openconfig, ET_OPEN_WAIT);
  et_open_config_settimeout(openconfig, timeout);
  if(et_open(&et_sys, et_name, openconfig) != ET_OK)
  {
    printf("ERROR: er ET init: cannot open ET system\n");
    return(-2);;
  }
  et_open_config_destroy(openconfig);

  /* set level of debug output */
  et_system_setdebug(et_sys, ET_DEBUG_ERROR);

  /* where am I relative to the ET system? */
  et_system_getlocality(et_sys, &et_locality);

  et_station_config_init(&sconfig);
  et_station_config_setselect(sconfig,  ET_STATION_SELECT_ALL);
  et_station_config_setblock(sconfig,   ET_STATION_BLOCKING);
  et_station_config_setuser(sconfig,    ET_STATION_USER_MULTI);
  et_station_config_setrestore(sconfig, ET_STATION_RESTORE_OUT);
  et_station_config_setprescale(sconfig,1);

  if((status = et_station_create(et_sys, &et_statid, "CTOF", sconfig)) < 0)
  {
    if (status == ET_ERROR_EXISTS) {
      printf("er ET init: station exists, will attach\n");
    }
    else
    {
      et_close(et_sys);
      et_station_config_destroy(sconfig);
      printf("ERROR: er ET init: cannot create ET station (status = %d)\n",
        status);
      return(-3);
    }
  }
  et_station_config_destroy(sconfig);

  if (et_station_attach(et_sys, et_statid, &et_attach) != ET_OK) {
    et_close(et_sys);
    printf("ERROR: er ET init: cannot attached to ET station\n");
    return(-4);;
  }

  et_init++;
  et_reinit = 0;
  printf("er ET init: ET fully initialized\n");
  return(0);
}

int 
gotControlEvent(et_event **pe, int size)
{
  int i;
    
  for (i=0; i<size; i++)
  {
    if ((pe[i]->control[0] == 17) || (pe[i]->control[0] == 20))
    {
      return(1);
    }
  }
  return(0);
}







#undef DEBUG_SEARCH

//#define DEBUG
 
#define NWPAWC 2000000 /* Length of the PAWC common block. */
#define LREC 1024      /* Record length in machine words. */

struct {
  float hmemor[NWPAWC];
} pawc_;



#define MAXEVENTS 10000000
#define MAXNTUPEVENTS 150000 /* max events in one ntuple file */

#define MAXBUF 10000000
unsigned int buf[MAXBUF];
unsigned int *bufptr;

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






/*translation tables: args - board#/chan, funcs - layer/slab */
#ifdef USE_TEST_IN_HALL

/* adcs */

/*slab numbers from 1 to 96*/
static int adcslab[22][16] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

  1, 2, 3, 1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*slot 3*/

  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
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

  1, 1, 1, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*slot 3*/

  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};






/* tdcs */

/*slab numbers from 1 to 96*/
static int tdcslab[22][16] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

  1, 2, 3, 1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*slot 9*/

  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*1-left,2-right*/
static int tdclr[22][16] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

  1, 1, 1, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*slot 9*/

  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

#else


/* adcs */

/*slab numbers from 1 to 96*/
static int adcslab[22][16] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 3, 5, 7, 1, 3, 5, 7, 9,11,13,15, 9,11,13,15, /*slot 13*/
  2, 4, 6, 8, 2, 4, 6, 8,10,12,14,16,10,12,14,16, /*slot 14*/
 17,19,21,23,17,19,21,23,25,27,29,31,25,27,29,31, /*slot 15*/
 18,20,22,24,18,20,22,24,26,28,30,32,26,28,30,32, /*slot 16*/
 33,35,37,39,33,35,37,39,41,43,45,47,41,43,45,47, /*slot 17*/
 34,36,38,40,34,36,38,40,42,44,46,48,42,44,46,48, /*slot 18*/
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*1-left,2-right*/
static int adclr[22][16] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 2, 2, 2, 2, /*slot 13*/
  1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 2, 2, 2, 2, /*slot 14*/
  1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 2, 2, 2, 2, /*slot 15*/
  1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 2, 2, 2, 2, /*slot 16*/
  1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 2, 2, 2, 2, /*slot 17*/
  1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 2, 2, 2, 2, /*slot 18*/
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};






/* tdcs */

/*slab numbers from 1 to 96*/
static int tdcslab[22][16] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 3, 5, 7, 1, 3, 5, 7, 9,11,13,15, 9,11,13,15, /*slot 3*/
  2, 4, 6, 8, 2, 4, 6, 8,10,12,14,16,10,12,14,16, /*slot 4*/
 17,19,21,23,17,19,21,23,25,27,29,31,25,27,29,31, /*slot 5*/
 18,20,22,24,18,20,22,24,26,28,30,32,26,28,30,32, /*slot 6*/
 33,35,37,39,33,35,37,39,41,43,45,47,41,43,45,47, /*slot 7*/
 34,36,38,40,34,36,38,40,42,44,46,48,42,44,46,48, /*slot 8*/
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*1-left,2-right*/
static int tdclr[22][16] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 2, 2, 2, 2, /*slot 3*/
  1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 2, 2, 2, 2, /*slot 4*/
  1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 2, 2, 2, 2, /*slot 5*/
  1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 2, 2, 2, 2, /*slot 6*/
  1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 2, 2, 2, 2, /*slot 7*/
  1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 2, 2, 2, 2, /*slot 8*/
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

#endif


/* translated data */

#define NHITS 100
static int nadc[2][48], adc[2][48][NHITS];
static int ntdc[2][48], tdc[2][48][NHITS];


#define ABS(x) ((x) < 0 ? -(x) : (x))

#define NCHAN 256 /*how many channels to draw*/

#define TDCRES 41.66667
#define TDCLSB 24

#define LENEVENT 193 /* for NTuple: the number of 32-bit words in event */

int
main(int argc, char **argv)
{
  FILE *fd = NULL;
  int bco1[256], bco2[256], bco3[256], bco4[256], nbco1, nbco2, nbco3, nbco4, diff, diff1, diff2;
  int nfile, nevents, etstart, etstop, len;
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
  int iet, maxevents;
  int ntuplefilecount;

  /* Ntuple definitions */
  int ntup;
  int lenevent = LENEVENT;
  int nprime = 4096;
  char chtags[LENEVENT][8] = {
    "iev     ",
	"tdcD01  ",
	"tdcD02  ",
	"tdcD03  ",
	"tdcD04  ",
	"tdcD05  ",
	"tdcD06  ",
	"tdcD07  ",
	"tdcD08  ",
	"tdcD09  ",
	"tdcD10  ",
	"tdcD11  ",
	"tdcD12  ",
	"tdcD13  ",
	"tdcD14  ",
	"tdcD15  ",
	"tdcD16  ",
	"tdcD17  ",
	"tdcD18  ",
	"tdcD19  ",
	"tdcD20  ",
	"tdcD21  ",
	"tdcD22  ",
	"tdcD23  ",
	"tdcD24  ",
	"tdcD25  ",
	"tdcD26  ",
	"tdcD27  ",
	"tdcD28  ",
	"tdcD29  ",
	"tdcD30  ",
	"tdcD31  ",
	"tdcD32  ",
	"tdcD33  ",
	"tdcD34  ",
	"tdcD35  ",
	"tdcD36  ",
	"tdcD37  ",
	"tdcD38  ",
	"tdcD39  ",
	"tdcD40  ",
	"tdcD41  ",
	"tdcD42  ",
	"tdcD43  ",
	"tdcD44  ",
	"tdcD45  ",
	"tdcD46  ",
	"tdcD47  ",
	"tdcD48  ",
	"tdcU01  ",
	"tdcU02  ",
	"tdcU03  ",
	"tdcU04  ",
	"tdcU05  ",
	"tdcU06  ",
	"tdcU07  ",
	"tdcU08  ",
	"tdcU09  ",
	"tdcU10  ",
	"tdcU11  ",
	"tdcU12  ",
	"tdcU13  ",
	"tdcU14  ",
	"tdcU15  ",
	"tdcU16  ",
	"tdcU17  ",
	"tdcU18  ",
	"tdcU19  ",
	"tdcU20  ",
	"tdcU21  ",
	"tdcU22  ",
	"tdcU23  ",
	"tdcU24  ",
	"tdcU25  ",
	"tdcU26  ",
	"tdcU27  ",
	"tdcU28  ",
	"tdcU29  ",
	"tdcU30  ",
	"tdcU31  ",
	"tdcU32  ",
	"tdcU33  ",
	"tdcU34  ",
	"tdcU35  ",
	"tdcU36  ",
	"tdcU37  ",
	"tdcU38  ",
	"tdcU39  ",
	"tdcU40  ",
	"tdcU41  ",
	"tdcU42  ",
	"tdcU43  ",
	"tdcU44  ",
	"tdcU45  ",
	"tdcU46  ",
	"tdcU47  ",
	"tdcU48  ",
	"adcD01  ",
	"adcD02  ",
	"adcD03  ",
	"adcD04  ",
	"adcD05  ",
	"adcD06  ",
	"adcD07  ",
	"adcD08  ",
	"adcD09  ",
	"adcD10  ",
	"adcD11  ",
	"adcD12  ",
	"adcD13  ",
	"adcD14  ",
	"adcD15  ",
	"adcD16  ",
	"adcD17  ",
	"adcD18  ",
	"adcD19  ",
	"adcD20  ",
	"adcD21  ",
	"adcD22  ",
	"adcD23  ",
	"adcD24  ",
	"adcD25  ",
	"adcD26  ",
	"adcD27  ",
	"adcD28  ",
	"adcD29  ",
	"adcD30  ",
	"adcD31  ",
	"adcD32  ",
	"adcD33  ",
	"adcD34  ",
	"adcD35  ",
	"adcD36  ",
	"adcD37  ",
	"adcD38  ",
	"adcD39  ",
	"adcD40  ",
	"adcD41  ",
	"adcD42  ",
	"adcD43  ",
	"adcD44  ",
	"adcD45  ",
	"adcD46  ",
	"adcD47  ",
	"adcD48  ",
	"adcU01  ",
	"adcU02  ",
	"adcU03  ",
	"adcU04  ",
	"adcU05  ",
	"adcU06  ",
	"adcU07  ",
	"adcU08  ",
	"adcU09  ",
	"adcU10  ",
	"adcU11  ",
	"adcU12  ",
	"adcU13  ",
	"adcU14  ",
	"adcU15  ",
	"adcU16  ",
	"adcU17  ",
	"adcU18  ",
	"adcU19  ",
	"adcU20  ",
	"adcU21  ",
	"adcU22  ",
	"adcU23  ",
	"adcU24  ",
	"adcU25  ",
	"adcU26  ",
	"adcU27  ",
	"adcU28  ",
	"adcU29  ",
	"adcU30  ",
	"adcU31  ",
	"adcU32  ",
	"adcU33  ",
	"adcU34  ",
	"adcU35  ",
	"adcU36  ",
	"adcU37  ",
	"adcU38  ",
	"adcU39  ",
	"adcU40  ",
	"adcU41  ",
	"adcU42  ",
	"adcU43  ",
	"adcU44  ",
	"adcU45  ",
	"adcU46  ",
	"adcU47  ",
	"adcU48  "
  };
  float event[LENEVENT];


  /*arg-slot(from 0), fun-histnum*/
  /*13,14,15,16,17,18,19*/
  int adcslot2hist[22] = {0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,3,4,5,6,7,0,0};
  /*3,4,5,6,7,8,9*/
  int tdcslot2hist[22] = {0,0,0,1,2,3,4,5,6,7,0,0,0,0,0,0,0,0,0,0,0,0};

  int nr,sec,strip,nl,ncol,nrow,i,j, k, ii,jj,kk,l,l1,l2,ichan,nn,mm,iev,nbytes,ind1;
  char title[128], *ch;
  char HBOOKfilename[256], chrunnum[32];
  int runnum;
  int nwpawc,lun,lrec,istat,icycle,idn,nbins,nbins1,igood,offset;
  float x1,x2,y1,y2,ww,tmpx,tmpy,ttt,ref;
  /*
  int goodevent, icedev;
  */
  if(argc != 2)
  {
    printf("Usage: evio_ctofntup <evio_filename>\n");
    exit(1);
  }


  nwpawc = NWPAWC;
  hlimit_(&nwpawc);
  lun = 11;
  lrec = LREC;


  /* check if events come from ET system */
  if(!strncmp(argv[1],"/tmp/et_sys_",12))
  {
    /* check if file exist */
    FILE *fd;
    fd = fopen(argv[1],"r");
    if(fd!=NULL)
	{
      fclose(fd);
      strncpy(et_name,argv[1],ET_FILENAME_LENGTH);
      printf("attach to ET system >%s<\n",et_name);
	}
    else
	{
      printf("ET system >%s< does not exist - exit\n",argv[1]);
      exit(0);
	}
	/*
    if (!et_alive(et_sys))
    {
      printf("ERROR: not attached to ET system\n");
      et_reinit = 1;
      exit(0);
    }
	*/
    if(et_initialize() != 0)
    {
      printf("ERROR: cannot initalize ET system\n");
      exit(0);
    }

    done = 0;
    use_et = 1;
  }


  if(use_et)
  {
    runnum = 1; /* temporary fake it, must extract from et */
    printf("run number is %d\n",runnum);
  }
  else
  {
    strcpy(chrunnum,argv[1]);
    ch = strchr(chrunnum,'0');
    ch[6] = '\0';
    runnum = atoi(ch);
    printf("run number is %s (%d)\n",ch,runnum);
  }



iev = 0;
nfile = 0;
ntuplefilecount = 0;

while(1)
{
  if(use_et)
  {
    status = et_events_get(et_sys, et_attach, pe, ET_SLEEP,
                            NULL, ET_EVENT_ARRAY_SIZE, &nevents);
    /* if no events or error ... */
    if ((nevents < 1) || (status != ET_OK))
    {
      /* if status == ET_ERROR_EMPTY or ET_ERROR_BUSY, no reinit is necessary */
      
      /* will wake up with ET_ERROR_WAKEUP only in threaded code */
      if (status == ET_ERROR_WAKEUP)
      {
        printf("status = ET_ERROR_WAKEUP\n");
      }
      else if (status == ET_ERROR_DEAD)
      {
        printf("status = ET_ERROR_DEAD\n");
        et_reinit = 1;
      }
      else if (status == ET_ERROR)
      {
        printf("error in et_events_get, status = ET_ERROR \n");
        et_reinit = 1;
      }
      done = 1;
    }
    else /* if we got events */
    {
      /* by default (no control events) write everything */
      etstart = 0;
      etstop  = nevents - 1;
      
      /* if we got control event(s) */
      if (gotControlEvent(pe, nevents))
      {
        /* scan for prestart and end events */
        for (i=0; i<nevents; i++)
        {
	      if (pe[i]->control[0] == prestartEvent)
          {
	        printf("Got Prestart Event!!\n");
	        /* look for first prestart */
	        if (PrestartCount == 0)
            {
	          /* ignore events before first prestart */
	          etstart = i;
	          if (i != 0)
              {
	            printf("ignoring %d events before prestart\n",i);
	          }
	        }
            PrestartCount++;
	      }
	      else if (pe[i]->control[0] == endEvent)
          {
	        /* ignore events after last end event & quit */
            printf("Got End event\n");
            etstop = i;
	        done = 1;
	      }
        }
      }
	}
    maxevents = iev + etstop; 
    iet = 0;
  }
  else
  {
    sprintf(filename,"%s.%d",argv[1],nfile++);
    printf("opening data file >%s<\n",filename);
    status = evOpen(filename,"r",&handler);
    printf("status=%d\n",status);
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
    maxevents = MAXEVENTS;
  }

  timestamp_old = 0;

  while(iev<maxevents)
  {
    iev ++;

    if(!(iev%1000)) printf("\n\n\nEvent %d\n\n",iev);
#ifdef DEBUG
    printf("\n\n\nEvent %d\n\n",iev);
#endif




	if(use_et)
	{
      int handle1;
      if(iet >= nevents)
	  {
        printf("ERROR: iev=%d, nevents=%d\n",iet,nevents);
        exit(0);
	  }
      et_event_getlength(pe[iet], &len); /*get event length from et*/
	  /*printf("len1=%d\n",len);*/

      /*copy event from et to the temporary buffer
      memcpy((char *)buf, (char *)pe[iet]->pdata, len);
      bufptr = &buf[8];
	  */
      bufptr = (unsigned int *)pe[iet]->pdata;
      bufptr += 8;

	  /*
	  printf("buf: 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
			 bufptr[0],bufptr[1],bufptr[2],bufptr[3],bufptr[4],
			 bufptr[5],bufptr[6],bufptr[7],bufptr[8],bufptr[9]);
	  */
goto a123;
      status = evOpenBuffer(pe[iet]->pdata, MAXBUF, "w", &handle1); /*open 'buffer' in et*/
      if(status!=0) printf("evOpenBuffer returns %d\n",status);
      status = evWrite(handle1, buf); /*write event to the 'buffer'*/
      if(status!=0) printf("evWrite returns %d\n",status);
      evGetBufferLength(handle1,&len); /*get 'buffer' length*/
	  /*printf("len2=%d\n",len);*/
      status = evClose(handle1); /*close 'buffer'*/
      if(status!=0) printf("evClose returns %d\n",status);
      et_event_setlength(pe[iet], len); /*update event length in et*/
a123:

      iet ++;

    }
    else
    {
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
      bufptr = buf;
    }




    /*if(iev < 3) continue;*/ /*skip first 2 events*/

    for(i=0; i<LENEVENT; i++) event[i] = 0.0;
    for(jj=0; jj<2; jj++)
    {
      for(kk=0; kk<48; kk++)
      {
        nadc[jj][kk] = 0;
        ntdc[jj][kk] = 0;
	  }
	}

    tdcref = 0;

	/*TDCs*/
    if((ind1 = evNlink(bufptr, 68, 0xe107, 0, &nbytes)) > 0)
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
      start = b08 = (unsigned char *) &bufptr[ind1];
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
        val = word&0x3FFFF;
        /*if(val>2000000)*/ /*printf("val=%d\n",val);*/
        val = val * TDCLSB;

        if(slot==9 && chan==14)
		{
          /*printf("ot#2=%d\n",val)*/;
		}
        if(slot==9 && chan==15)
		{
          if(tdcref==0)
		  {
            tdcref = val;
            /*printf("tdcref=%d\n",tdcref/TDCLSB);*/
		  }
          else
		  {
            /*printf("ERROR: double tdcref = %d, already defined as %d\n",val,tdcref);*/
		  }
		}

        jj = tdclr[slot][chan] - 1;
        kk = tdcslab[slot][chan] - 1;
        if(jj>=0)
		{
          tdc[jj][kk][ntdc[jj][kk]] = val;
 		  /*printf("1=> ntdc[%2d][%2d]=%2d tdc=%7.3f\n",jj,kk,ntdc[jj][kk],(float)(tdc[jj][kk][ntdc[jj][kk]])/1000.);fflush(stdout);*/
          ntdc[jj][kk] ++;
		}

#ifdef DEBUG
        printf("TDC[0x%08x]:  slot=%2d  chan=%3d  edge=%1d  tdc=%5d(%f)  (hist_id=%d)\n",
			   word,slot,chan,edge,val,(((float)val)/1000.),idn);
#endif

	  }
	}


	/* ADC raw mode bank */
    if((ind1 = evNlink(bufptr, 68, 0xe101, 0, &nbytes)) > 0)
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
      b08 = (unsigned char *) &bufptr[ind1];
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
#ifdef DEBUG
              printf("slot=%d chan=%d baseline=%d\n",slot,chan,baseline);
#endif
			}

            if(mm>35 && mm<70)
            {
              if(summing_in_progress==0 && data>(baseline+20))
			  {
                /*printf("open summing at mm=%d\n",mm);*/
                summing_in_progress = 1;
                sum += (datasaved[mm-3]-baseline);
                sum += (datasaved[mm-2]-baseline);
                sum += (datasaved[mm-1]-baseline);
			  }

              if(summing_in_progress>0 && data<baseline)
			  {
                /*printf("close summing at mm=%d, sum=%d\n",mm,sum);*/
                summing_in_progress = -1;



			  }

              if(summing_in_progress>0)
			  {
                sum += (datasaved[mm]-baseline);
                /*printf("sum=%d (mm=%d)\n",sum,mm);*/
			  }
			}


	      }

		  /*
          for(jj=0; jj<nsamples; jj++) printf(" %3d",datasaved[jj]);
          printf("\n");
		  */




		  if(1/*slot < 17*/)
		  { 
            jj = adclr[slot][chan] - 1;
            kk = adcslab[slot][chan] - 1;
			if(jj>=0 && sum > 0)
			{
			  /*
		printf("2=> %d %d %d\n",ii,jj,kk);fflush(stdout);
		printf("2==> %d\n",nadc[ii][jj][kk]);fflush(stdout);
			  */
              adc[jj][kk][nadc[jj][kk]] = sum;
              nadc[jj][kk] ++;
			}
		  }

        }
#ifdef DEBUG
        printf("end loop: b08=0x%08x\n",b08);
#endif
      }
    }

	


	/******************************************************************/
    /* at that point we have adc[][][][] and tdc[][][][] arrays ready */
	/******************************************************************/

#ifdef USE_TEST_IN_HALL
    tdcref=250000;
#endif

	/* correct TDCs using reference signal */
    if(tdcref==0)
	{
      printf("ERROR: iev=%d -> there is no ref signal\n",iev);
	}
    else
	{
      for(jj=0; jj<2; jj++)
      {
        for(kk=0; kk<48; kk++)
        {
          /*printf("befor: ntdc[%2d][%2d]=%d\n",jj,kk,ntdc[jj][kk]);*/
          for(nn=0; nn<ntdc[jj][kk]; nn++)
		  {
			/*printf("befor: tdc=%d tdcref=%d\n",tdc[jj][kk][nn],tdcref);*/
            tdc[jj][kk][nn] += (250000 - tdcref);
		    /*printf("after: %d\n",tdc[jj][kk][nn]);*/
		  }

	    }
	  }
	}
	
	/*
       adc[jj][kk][nadc[jj][kk]]
       nadc[jj][kk]

       tdc[jj][kk][ntdc[jj][kk]]
       ntdc[jj][kk]
	*/



    if(!((iev-1)%MAXNTUPEVENTS))
	{
      printf("Open new ntyple file (event %d)\n",iev);

      /* open HBOOK file and create NTUPLE */
      sprintf(HBOOKfilename,"ctofntup%d.his_%d",runnum,ntuplefilecount++);
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
      hbookn_(&idn,"CTOF",&lenevent,"//NTUPEL",&nprime,chtags,strlen("CTOF"),8,8);
	}



	
	if(tdcref==0) goto a123456;
	




    for(jj=0; jj<2; jj++) /* L/R */
    {
      for(kk=0; kk<48; kk++) /* counter # */
      {
        ntup = 48*jj + kk + 1;
		/*printf("tdc: jj=%d kk=%d ntup=%d\n",jj,kk,ntup);*/
        if(ntdc[jj][kk]>0 && event[ntup]<=0.00001)
        {
          for(nn=0; nn<ntdc[jj][kk]; nn++)
		  {
            if(tdc[jj][kk][nn]>0)
			{
              event[ntup] = (float)tdc[jj][kk][nn];
              /*printf("tdc=%f\n",event[ntup]);*/
		      if(event[ntup]<0.0001) {printf("tdc: jj=%d kk=%d ntup=%d\n",jj,kk,ntup);printf("tdc=%f\n",event[ntup]);}
			}
		  }
        }
        ntup += 96;
        /*printf("adc: jj=%d kk=%d ntup=%d\n",jj,kk,ntup);*/
        if(nadc[jj][kk]>0 && event[ntup]<=0.00001)
        {
          event[ntup] = (float)adc[jj][kk][0];
          /*printf("adc=%f\n",event[ntup]);*/
        }
	  }
	}




a123456:






    event[0] = iev;
    idn = 2;
	/*for(ii=0; ii<LENEVENT; ii++) printf(" [%3d] %f\n",ii,event[ii]);*/
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



  if(use_et)
  {
    /* put et events back into system */
    status = et_events_put(et_sys, et_attach, pe, nevents);            
    if (status != ET_OK)
    {
	  printf("error in et_events_put, status = %i \n",status);
      et_reinit = 1;
      done = 1;
    }	
  }
  else
  {
    printf("evClose after %d events\n",iev);fflush(stdout);
    evClose(handler);
  }

  if(iev>=MAXEVENTS) break;

} /*while*/


  /* closing HBOOK file */
  idn = 0;
  printf("befor hrout_\n");fflush(stdout);
  hrout_(&idn,&icycle," ",1);
  printf("after hrout_\n");fflush(stdout);
  hrend_("CTOF", 4);
  printf("after hrend_\n");fflush(stdout);

  exit(0);
}
