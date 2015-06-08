
/* testing bed for evioBankUtil functions */

/* for posix */
#define _POSIX_SOURCE_ 1
#define __EXTENSIONS__

#undef DEBUG

/*  misc macros, etc. */
#define MAXEVIOBUF 1000000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <evio.h>
#include <evioBankUtil.h>

static unsigned int buf[MAXEVIOBUF];
static char *input_filename/* = "/work/dcrb/dcrb2_000049.evio.0_1600V_60mV"*/;
static int input_handle;
static char *output_filename/* = "test.evio"*/;
static int output_handle;

static int nevent         = 0;
static int nwrite         = 0;
static int skip_event     = 0;
static int max_event      = 1000002;

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

#define PUT8(ret_val) \
  *b08out++ = ret_val 

#define PUT16(ret_val) \
  b16 = (unsigned short *)b08out; \
  *b16 = ret_val; \
  b08out+=2

#define PUT32(ret_val) \
  b32 = (unsigned int *)b08out; \
  *b32 = ret_val; \
  b08out+=4

#define PUT64(ret_val) \
  b64 = (unsigned long long *)b08out; \
  *b64 = ret_val; \
  b08out+=8

#define DC_DATA_BLOCK_HEADER      0x00
#define DC_DATA_BLOCK_TRAILER     0x01
#define DC_DATA_EVENT_HEADER      0x02
#define DC_DATA_TRIGGER_TIME      0x03
#define DC_DATA_DCRBEVT           0x08
#define DC_DATA_INVALID           0x0E
#define DC_DATA_FILLER            0x0F

int
main(int argc, char **argv)
{
  int status;
  int l, tag, num;
  int ind, fragtag, fragnum, nbytes, timestamp_flag, type, *nhits;
  int slot, slot_old, event, chan, tdc;
  int banktag = 0xe116, banknum = 0, banktyp = 0xf;
  char *fmt = "c,i,l,N(c,s)"; /* slot,event#,timestamp,Nhits(channel,tdc) */
  unsigned int *dabufp, word;
  unsigned char *b08, *b08out;
  unsigned short *b16;
  unsigned int *b32;
  unsigned long long *b64, timestamp, timestamp_old;
  unsigned char *end, *start;
  FILE *fd;

  if(argc!=3) printf("Usage: evioUtilTest <input evio file> <output evio file>\n");
  input_filename = strdup(argv[1]);
  output_filename = strdup(argv[2]);
  printf("Use >%s< as input file\n",input_filename);
  printf("Use >%s< as output file\n",output_filename);
  if(!strcmp(input_filename,output_filename))
  {
    printf("input and output files must be different - exit\n");
    exit(0);
  }

  fd = fopen(output_filename,"r");
  if(fd>0)
  {
    printf("Output file %s exist - exit\n",output_filename);
    fclose(fd);
    exit(0);
  }

  /* open evio input file */
  if((status = evOpen(input_filename,"r",&input_handle))!=0)
  {
    printf("\n ?Unable to open input file %s, status=%d\n\n",input_filename,status);
    exit(1);
  }

  /* open evio output file */
  if((status = evOpen(output_filename,"w",&output_handle))!=0)
  {
    printf("\n ?Unable to open output file %s, status=%d\n\n",output_filename,status);
    exit(1);
  }



  nevent=0;
  nwrite=0;
  while ((status=evRead(input_handle,buf,MAXEVIOBUF))==0)
  {
    nevent++;
    if(!(nevent%10000)) printf("processed %d events\n",nevent);
    if(skip_event>=nevent) continue;
    /*if(user_event_select(buf)==0) continue;*/

    /**********************/
	/* evioBankUtil stuff */

	fragtag = 67;
    fragnum = -1;
    tag = 0xe105;
    num = 67;

	ind = evLinkFrag(buf, fragtag, fragnum);
    /*printf("evLinkFrag returns %d\n",ind);*/
	if(ind<=0) continue;

    ind = evLinkBank(buf, fragtag, fragnum, tag, num, &nbytes);
    /*printf("evLinkBank returns %d\n",ind);*/
	if(ind<=0) continue;

    start = (unsigned char *) &buf[ind+2];
    end = start + nbytes;
	/*printf("input: nbytes=%d (%d words)\n",nbytes,nbytes>>2);*/

    if(ind > 0)
	{
      dabufp = evOpenBank(buf, fragtag, fragnum, banktag, banknum, banktyp, fmt);
      /*printf("dabufp = 0x%08x\n",dabufp);*/

      b08out = (unsigned char *)dabufp;
      /*printf("first b08out = 0x%08x\n",b08out);*/
      





        timestamp_flag = 0;
        slot_old = -1;
        b08 = start;
        while(b08<end)
	    {
#ifdef DEBUG
          /*printf("begin while: b08=0x%08x\n",b08);*/
#endif
          GET32(word);
#ifdef DEBUG
          printf("dcrb data word hex=0x%08x uint=%u int=%d\n",word,word,word);
#endif
          if(timestamp_flag)
		  {
			/* ???????????????????
		    timestamp |= (word&0xffffff);
			*/
            timestamp = timestamp >> 24;
            timestamp = (((unsigned long long)word&0xffffff)<<24) | timestamp;

#ifdef DEBUG
		    printf(" {TRGTIME} TRIG TIME (3 low BYTES)\n");fflush(stdout);
		    printf(" {TRGTIME} timestamp=%lld (%lld)\n",timestamp,timestamp_old);fflush(stdout);
#endif
            timestamp_flag = 0;
			continue;
		  }

	      if(word & 0x80000000)
	      {
	        type = (word>>27)&0xF;
	        switch(type)
	        {
		      case DC_DATA_BLOCK_HEADER:
                slot = (word>>22)&0x1f;
#ifdef DEBUG
		        printf(" {BLKHDR} SLOTID: %d", (word>>22)&0x1f);fflush(stdout);
		        printf(" NEVENTS: %d", (word>>11)&0x7ff);fflush(stdout);
		        printf(" BLOCK: %d\n", (word>>0)&0x7ff);fflush(stdout);
#endif
		        break;
		      case DC_DATA_BLOCK_TRAILER:
#ifdef DEBUG
		        printf(" {BLKTLR} SLOTID: %d", (word>>22)&0x1f);fflush(stdout);
		        printf(" NWORDS: %d\n", (word>>0)&0x3fffff);fflush(stdout);
#endif
		        break;

		      case DC_DATA_EVENT_HEADER:
                event = (word>>0)&0xfffff/*0x7ffffff*/;
#ifdef DEBUG
		        printf(" {EVTHDR} EVENT: %d\n", event);fflush(stdout);
#endif
		        break;
		      case DC_DATA_TRIGGER_TIME:
		        timestamp = (((unsigned long long)word&0xffffff)<<24);
                timestamp_flag = 1;
#ifdef DEBUG
		        printf(" {TRGTIME} TRIG TIME (3 HIGH BYTES)\n");fflush(stdout);
#endif
		        break;
		      case DC_DATA_DCRBEVT:
                chan = (word>>16)&0x7F;
                tdc = (word>>0)&0xFFFF;
#ifdef DEBUG
		        printf(" {DCRBEVT} 0x%08x",word);fflush(stdout);
		        printf(" CH: %3u", chan);fflush(stdout);
		        printf(" TDC: %6u\n", tdc);fflush(stdout);
#endif
				if(slot!=slot_old)
				{
                  PUT8(slot);
                  PUT32(event);
                  PUT64(timestamp);

                  nhits = (unsigned int *)b08out;
			      PUT32(0);

                  slot_old = slot;
				}
                (*nhits)++;
                PUT8(chan);
                PUT16(tdc);

		        break;
		      case DC_DATA_INVALID:
		        printf(" {***DNV***}\n");fflush(stdout);
                exit(1);
		        break;
		      case DC_DATA_FILLER:
#ifdef DEBUG
		        printf(" {FILLER}\n");fflush(stdout);
#endif
		        break;
		      default:
		        printf(" {***DATATYPE ERROR***}\n");fflush(stdout);
                exit(1);
		        break;
	        }
	      }
#ifdef DEBUG
	      else
	      {
	        printf("\n");fflush(stdout);
	      }
#endif


#ifdef DEBUG
          /*printf("end loop: b08=0x%08x\n",b08);*/
#endif
        }




	  /*printf("last b08out = 0x%08x\n",b08out);*/

      evCloseBank(buf, fragtag, fragnum, banktag, banknum, b08out);
	  

	}

	/* evioBankUtil stuff */
    /**********************/



    nwrite++;
    status = evWrite(output_handle,buf);
    if(status!=0)
    {
      printf("\n ?evWrite error output file %s, status=%d\n\n",output_filename,status);
      exit(1);
    }
    if( (max_event>0) && (nevent>=max_event+skip_event) ) break;
  }


  /* done */
  printf("\n  Read %d events, copied %d events\n\n",nevent,nwrite);
  evClose(output_handle);
  evClose(input_handle);

  exit(0);
}
