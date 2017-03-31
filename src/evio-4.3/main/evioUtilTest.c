
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
static int max_event      = 3/*1000002*/;


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
  int ii, l, tag, num;
  int ind, fragtag, fragnum, nbytes, ind_data, timestamp_flag, type, *nhits;
  int slot, slot_old, event, chan, tdc;
  int banktag = 0xe116, banknum = 0, banktyp = 0xf;
  char *fmt = "c,i,l,N(c,s)";
  unsigned int ret, word;
  unsigned long long timestamp, timestamp_old;
  unsigned char *end, *start;
  FILE *fd;
  GET_PUT_INIT;

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


  /* open evio output file */
  if((status = evOpen(output_filename,"w",&output_handle))!=0)
  {
    printf("\n ?Unable to open output file %s, status=%d\n\n",output_filename,status);
    exit(1);
  }



  nevent=0;
  nwrite=0;
  while (nevent<max_event)
  {

    printf("\n\nProcessing event %d\n",nevent);

    nevent++;

    if(!(nevent%10000)) printf("evioUtilTest1: processed %d events\n",nevent);
    if(skip_event>=nevent) continue;
    /*if(user_event_select(buf)==0) continue;*/


    /**********************/
	/* evioBankUtil stuff */

	fragtag = 67;
    fragnum = 1;
    tag = 0xe105;
    num = 67;


    status = evOpenEvent(buf, 17);
    printf("\n\nevOpenEvent returns %d\n",status);

    status = evOpenFrag(buf, fragtag, fragnum);
    printf("\n\nevOpenFrag returns %d\n",status);

    if(status >= 0)
	{
      ret = evOpenBank(buf, fragtag, fragnum, banktag, banknum, banktyp, fmt, &ind_data);
      printf("evOpenBank returns = %d\n",ret);

      b08out = (unsigned char *)&buf[ind_data];
      printf("first b08out=0x%08x\n",b08out);

	  slot = 13;
      event = nevent;
      timestamp = 0x123456789ABCLL;

      PUT8(slot);
      PUT32(event);
      PUT64(timestamp);

      nhits = (unsigned int *)b08out;
	  PUT32(0);

      for(ii=0; ii<20; ii++)
	  {
        (*nhits)++;

        chan = ii+1;
        tdc = ii*10+100;

        PUT8(chan);
        PUT16(tdc);
	  }

	  printf("last b08out = 0x%08x\n",b08out);

      evCloseBank(buf, fragtag, fragnum, banktag, banknum, b08out);

	}



    fragnum ++;
    banknum ++;

    status = evOpenFrag(buf, fragtag, fragnum);
    printf("\n\nevOpenFrag returns %d\n",status);

    if(status >= 0)
	{
      ret = evOpenBank(buf, fragtag, fragnum, banktag, banknum, banktyp, fmt, &ind_data);
      printf("evOpenBank returns = %d\n",ret);

      b08out = (unsigned char *)&buf[ind_data];
      printf("first b08out=0x%08x\n",b08out);

	  slot = 13;
      event = nevent;
      timestamp = 0x123456789ABCLL;

      PUT8(slot);
      PUT32(event);
      PUT64(timestamp);

      nhits = (unsigned int *)b08out;
	  PUT32(0);

      for(ii=0; ii<20; ii++)
	  {
        (*nhits)++;

        chan = ii+1;
        tdc = ii*10+100;

        PUT8(chan);
        PUT16(tdc);
	  }

	  printf("last b08out = 0x%08x\n",b08out);

      evCloseBank(buf, fragtag, fragnum, banktag, banknum, b08out);

	}

	/* evioBankUtil stuff */
    /**********************/


    nwrite++;
    status = evWrite(output_handle,&buf[0]);
    if(status!=0)
    {
      printf("\n ?evWrite error output file %s, status=%d\n\n",output_filename,status);
      exit(1);
    }

    printf("nevent=%d max_event=%d\n",nevent,max_event);

  }


  /* done */
  printf("\n  Read %d events, copied %d events\n\n",nevent,nwrite);
  evClose(output_handle);

  exit(0);
}
