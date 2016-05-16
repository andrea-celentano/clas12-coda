
/* evioBanGEMC.c - converts GEMC banks to DAQ banks */

/* for posix */
#define _POSIX_SOURCE_ 1
#define __EXTENSIONS__

#undef DEBUG

/*  misc macros, etc. */
#define MAXEVIOBUF 10000000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <evio.h>
#include <evioBankUtil.h>

#include "ectrans.h"

static unsigned int buf[MAXEVIOBUF];
static char *input_filename;
static int input_handle;
static char *output_filename;
static int output_handle;

#define NHITS 100
static int inBuf[7][NHITS];


static int nevent         = 0;
static int nwrite         = 0;
static int skip_event     = 0;
static int max_event      = 15;


int
main(int argc, char **argv)
{
  int i, status, trig, layer, strip;
  int l, tag, num, nchan[22], nn, mm, npulses;
  int pulse_integral, pulse_time, pulse_min, pulse_max;
  int ind, fragtag, fragnum, nbytes, ind_data, timestamp_flag, type, *nhits;
  int slot, slot_old, event, chan, tdc;
  int banktag = 0xe102, banknum = 0, banktyp = 0xf;
  char *fmt = "c,i,l,N(c,N(s,i,s,s))";
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
  trig = 0;
  pulse_min = 0;
  pulse_max = 0;
  npulses = 1;
  timestamp = 0x0102030405LL;
  while( (status=evRead(input_handle,buf,MAXEVIOBUF))==0 && nevent < max_event)
  {
    nevent++;
    if(!(nevent%10000)) printf("processed %d events\n",nevent);
    if(skip_event>=nevent) continue;
    /*if(user_event_select(buf)==0) continue;*/

    /**********************/
	/* evioBankUtil stuff */

	fragtag = 1602;
    fragnum = 0;
    tag = 1602;

    for(num=1; num<=6; num++) /* 1-sector, 2-io, 3-view, 4-strip, 5-adc, 6-tdc */
	{
      ind = evLinkBank(buf, fragtag, fragnum, tag, num, &nbytes, &ind_data);
      if(ind > 0)
	  {
		/*
		PRINT_BUFFER(&buf[ind_data], &buf[ind_data]+(nbytes/4));
		*/

        for(i=0; i<nbytes/4; i++)
		{
          inBuf[num][i] = buf[ind_data+i];
          /*printf("IN[%2d][%2d] = %d\n",num,i,inBuf[num][i]);*/
		}
	  }
	}

#if 1
    if(ind > 0)
	{
      trig ++;
	  
      i = evOpenFrag(buf, 7, 1);
      printf("evOpenFrag returned %d\n",i);
	  
      ret = evOpenBank(buf, 7/*fragtag*/, 1/*fragnum*/, banktag, banknum, banktyp, fmt, &ind_data);
      printf("evOpenBank returns = %d\n",ret);

      b08out = (unsigned char *)&buf[ind_data];
      printf("first b08out = 0x%08x\n",b08out);

	  for(i=0; i<22; i++) nchan[i] = 0;

      for(i=0; i<nbytes/4; i++)
      {
        printf("begin while: b08=0x%08x\n",b08);
		layer = (inBuf[2][i]-1)*3+inBuf[3][i]-1;
		printf("+++++++++++++ %d %d -> %d\n",inBuf[2][i],inBuf[3][i],layer);
        strip = inBuf[4][i]-1;
        slot = adcslotecal [layer] [strip];
		chan = adcchanecal [layer] [strip];
        printf("### layer=%d strip=%d -> slot=%d chan=%d\n",layer,strip,slot,chan);
	    nchan[slot] ++;

        PUT8(slot);
        PUT32(trig);
        PUT64(timestamp);
        PUT32(nchan[slot]);
        printf("slot=%d, trig=%d, timestamp=%lld nchan[%d]=%d\n",slot,trig,timestamp,slot,nchan[slot]);

        for(nn=0; nn<nchan[slot]; nn++)
	    {
          PUT8(chan);
          PUT32(npulses);
          printf("  chan=%d, npulses=%d\n",chan,npulses);

          for(mm=0; mm<npulses; mm++)
	      {
            pulse_integral = inBuf[5][i];
            pulse_time = inBuf[6][i]-25000; /* sergey: arbitrary !!! */
            PUT16(pulse_time);
            PUT32(pulse_integral);
            PUT16(pulse_min);
            PUT16(pulse_max);
            printf("  pulse_time=%d pulse_integral=%d\n",pulse_time,pulse_integral);
        /*printf(">>> layer %d, strip %d -> adc %d\n",adclayerecal[slot][chan],adcstripecal[slot][chan],pulse_integral);*/
	      }

          printf("last b08out=0x%08x\n",b08out);
        }
      }
	  /*printf("last b08out = 0x%08x\n",b08out);*/
      evCloseBank(buf, 7/*fragtag*/, 1/*fragnum*/, banktag, banknum, b08out);
	}
#endif

	/* evioBankUtil stuff */
    /**********************/


    nwrite++;
	printf("nwrite=%d\n",nwrite);
    status = evWrite(output_handle,buf);
    if(status!=0)
    {
      printf("\n ?evWrite error output file %s, status=%d (0x%08x)\n\n",output_filename,status,status);
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
