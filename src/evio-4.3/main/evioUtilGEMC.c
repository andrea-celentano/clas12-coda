
/* evioBanGEMC.c - converts GEMC banks to DAQ banks */

/* for posix */
#define _POSIX_SOURCE_ 1
#define __EXTENSIONS__

#undef DEBUG

#define USE_PCAL

/*  misc macros, etc. */
#define MAXEVIOBUF 10000000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <evio.h>
#include <evioBankUtil.h>

#ifdef USE_PCAL
#include "pctrans.h"
#else
#include "ectrans.h"
#endif

static unsigned int buf[MAXEVIOBUF];
static char *input_filename;
static int input_handle;
static char *output_filename;
static int output_handle;

#define NHITS 100
static int inBuf[7][NHITS];



typedef struct ecdata
{
  int sector;
  int io;
  int view;
  int strip;
  int adc;
  int tdc;
}
ECData;

static ECData aBuf[NHITS];

static int nevent         = 0;
static int nwrite         = 0;
static int skip_event     = 0;
static int max_event      = 10000000;



/* returns 1 - do sort */
static int
ecdata_compare(ECData *i, ECData *j)
{
  if(i->io > j->io) return(1);
  else if(i->io == j->io && i->view > j->view) return(1);
  else if(i->io == j->io && i->view == j->view && i->strip > j->strip) return(1);
  else return (0);
}


int
main(int argc, char **argv)
{
  int i, nw, status, trig, sector, layer, strip;
  int l, tag, num, nchan[22], *nchanptr, nn, mm, npulses;
  int pulse_integral, pulse_time, pulse_min, pulse_max;
  int ind, fragtag, fragnum, fragtag1, fragnum1, nbytes, ind_data, timestamp_flag, type, *nhits;
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


#ifdef USE_ECAL

    /********/
    /* ECAL */
    /********/

    sector = 5;

	fragtag = 1602;
    fragnum = 0;
    tag = 1602;

    printf("\n");
    for(num=1; num<=6; num++) /* 1-sector, 2-io, 3-view, 4-strip, 5-adc, 6-tdc */
	{
      ind = evLinkBank(buf, fragtag, fragnum, tag, num, &nbytes, &ind_data);
      if(ind > 0)
	  {
		/*
		PRINT_BUFFER(&buf[ind_data], &buf[ind_data]+(nbytes/4));
		*/

        if(num==1)      printf("SECTOR: ");
        else if(num==2) printf("    IO: ");
        else if(num==3) printf("  VIEW: ");
        else if(num==4) printf(" STRIP: ");
        else if(num==5) printf("   ADC: ");
        else if(num==6) printf("   TDC: ");

        for(i=0; i<nbytes/4; i++)
		{
          inBuf[num][i] = buf[ind_data+i];
          printf("%6d",inBuf[num][i]);
		}
        printf("\n");
	  }
	}
    printf("\n");



    /* clean up */
    nw = 0;
	printf("Process %d hit candidates\n",nbytes/4);
    for(i=0; i<nbytes/4; i++)
	{
      if(inBuf[1][i] != sector) continue; /* skip wrong sectors */
      if(inBuf[5][i] <= 0) continue; /* skip zero energy */

      aBuf[nw].sector = inBuf[1][i];
      aBuf[nw].io     = inBuf[2][i];
      aBuf[nw].view   = inBuf[3][i];
      aBuf[nw].strip  = inBuf[4][i];
      aBuf[nw].adc    = inBuf[5][i];
      aBuf[nw].tdc    = inBuf[6][i];
	  nw++;
	}

	printf("Found %d good hits\n",nw);
    for(num=1; num<=6; num++)
	{
      if(num==1)      printf("SECTOR: ");
      else if(num==2) printf("    IO: ");
      else if(num==3) printf("  VIEW: ");
      else if(num==4) printf(" STRIP: ");
      else if(num==5) printf("   ADC: ");
      else if(num==6) printf("   TDC: ");
      for(i=0; i<nw; i++)
	  {
        if(num==1)      printf("%6d",aBuf[i].sector);
        else if(num==2) printf("%6d",aBuf[i].io);
        else if(num==3) printf("%6d",aBuf[i].view);
        else if(num==4) printf("%6d",aBuf[i].strip);
        else if(num==5) printf("%6d",aBuf[i].adc);
        else if(num==6) printf("%6d",aBuf[i].tdc);
	  }
	  printf("\n");
	}
	printf("\n");

    /* sort */
    qsort((void *)aBuf, nw, sizeof(ECData), (int (*) (const void *, const void *))ecdata_compare);

	printf("Sorted %d hits\n",nw);
    for(num=1; num<=6; num++)
	{
      if(num==1)      printf("SECTOR: ");
      else if(num==2) printf("    IO: ");
      else if(num==3) printf("  VIEW: ");
      else if(num==4) printf(" STRIP: ");
      else if(num==5) printf("   ADC: ");
      else if(num==6) printf("   TDC: ");
      for(i=0; i<nw; i++)
	  {
        if(num==1)      printf("%6d",aBuf[i].sector);
        else if(num==2) printf("%6d",aBuf[i].io);
        else if(num==3) printf("%6d",aBuf[i].view);
        else if(num==4) printf("%6d",aBuf[i].strip);
        else if(num==5) printf("%6d",aBuf[i].adc);
        else if(num==6) printf("%6d",aBuf[i].tdc);
	  }
	  printf("\n");
	}

    if(ind > 0)
	{
      trig ++;
      slot_old = 0;
	  
	  fragtag1 = 1/*7*/; /* 1 for sector 1, 7 for sector 2 */
      fragnum1 = 1;

      i = evOpenFrag(buf, fragtag1, 1);
      printf("evOpenFrag returned %d\n",i);
	  
      ret = evOpenBank(buf, fragtag1, fragnum1, banktag, banknum, banktyp, fmt, &ind_data);
      printf("evOpenBank returns = %d\n",ret);

      b08out = (unsigned char *)&buf[ind_data];
      printf("first b08out = 0x%08x\n",b08out);

	  for(i=0; i<22; i++) nchan[i] = 0;

      for(i=0; i<nw; i++)
      {
        printf("begin while: b08=0x%08x\n",b08);

		layer = (aBuf[i].io-1)*3+aBuf[i].view-1;
		printf("+++++++++++++ %d %d -> %d\n",aBuf[i].io,aBuf[i].view,layer);
        strip = aBuf[i].strip-1;
        slot = adcslotecal [layer] [strip];
		chan = adcchanecal [layer] [strip] - 1;
        printf("### layer=%d strip=%d -> slot=%d chan=%d (nchan(so far)=%d)\n",layer,strip,slot,chan,nchan[slot]);

        if(slot != slot_old)
		{
          slot_old = slot;

          PUT8(slot);
          PUT32(trig);
          PUT64(timestamp);

          nchan[slot]=0;
          nchanptr = (unsigned int *)b08out;
          PUT32(nchan[slot]); /* reserve space for channel counter */

          printf("slot=%d, trig=%d, timestamp=%lld nchan[%d]=%d\n",slot,trig,timestamp,slot,nchan[slot]);
		}

        /*for(nn=0; nn<nchan[slot]; nn++)*/
	    {
	      nchan[slot] ++;
          *nchanptr = nchan[slot];

          PUT8((chan));
          PUT32(npulses);
          printf("  chan=%d, npulses=%d\n",chan,npulses);

          for(mm=0; mm<npulses; mm++)
	      {
            pulse_integral = aBuf[i].adc;
            pulse_time = aBuf[i].tdc-25000; /* sergey: arbitrary !!! */
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
      evCloseBank(buf, fragtag1, fragnum1, banktag, banknum, b08out);
	}

#endif




#ifdef USE_PCAL

    /********/
    /* PCAL */
    /********/

	fragtag = 1502;
    fragnum = 0;
    tag = 1502;

    printf("\n");
    for(num=1; num<=6; num++) /* 1-sector, 2-io, 3-view, 4-strip, 5-adc, 6-tdc */
	{
      ind = evLinkBank(buf, fragtag, fragnum, tag, num, &nbytes, &ind_data);
      if(ind > 0)
	  {
		/*
		PRINT_BUFFER(&buf[ind_data], &buf[ind_data]+(nbytes/4));
		*/

        if(num==1)      printf("SECTOR: ");
        else if(num==2) printf("    IO: ");
        else if(num==3) printf("  VIEW: ");
        else if(num==4) printf(" STRIP: ");
        else if(num==5) printf("   ADC: ");
        else if(num==6) printf("   TDC: ");

        for(i=0; i<nbytes/4; i++)
		{
          inBuf[num][i] = buf[ind_data+i];
          printf("%6d",inBuf[num][i]);
		}
        printf("\n");
	  }
	}
    printf("\n");



    /* clean up */
    nw = 0;
	printf("Process %d hit candidates\n",nbytes/4);
    for(i=0; i<nbytes/4; i++)
	{
      if(inBuf[1][i] != 5) continue; /* skip wrong sectors */
      if(inBuf[5][i] <= 0) continue; /* skip zero energy */

      aBuf[nw].sector = inBuf[1][i];
      aBuf[nw].io     = inBuf[2][i];
      aBuf[nw].view   = inBuf[3][i];
      aBuf[nw].strip  = inBuf[4][i];
      aBuf[nw].adc    = inBuf[5][i];
      aBuf[nw].tdc    = inBuf[6][i];
	  nw++;
	}

	printf("Found %d good hits\n",nw);
    for(num=1; num<=6; num++)
	{
      if(num==1)      printf("SECTOR: ");
      else if(num==2) printf("    IO: ");
      else if(num==3) printf("  VIEW: ");
      else if(num==4) printf(" STRIP: ");
      else if(num==5) printf("   ADC: ");
      else if(num==6) printf("   TDC: ");
      for(i=0; i<nw; i++)
	  {
        if(num==1)      printf("%6d",aBuf[i].sector);
        else if(num==2) printf("%6d",aBuf[i].io);
        else if(num==3) printf("%6d",aBuf[i].view);
        else if(num==4) printf("%6d",aBuf[i].strip);
        else if(num==5) printf("%6d",aBuf[i].adc);
        else if(num==6) printf("%6d",aBuf[i].tdc);
	  }
	  printf("\n");
	}
	printf("\n");

    /* sort */
    qsort((void *)aBuf, nw, sizeof(ECData), (int (*) (const void *, const void *))ecdata_compare);

	printf("Sorted %d hits\n",nw);
    for(num=1; num<=6; num++)
	{
      if(num==1)      printf("SECTOR: ");
      else if(num==2) printf("    IO: ");
      else if(num==3) printf("  VIEW: ");
      else if(num==4) printf(" STRIP: ");
      else if(num==5) printf("   ADC: ");
      else if(num==6) printf("   TDC: ");
      for(i=0; i<nw; i++)
	  {
        if(num==1)      printf("%6d",aBuf[i].sector);
        else if(num==2) printf("%6d",aBuf[i].io);
        else if(num==3) printf("%6d",aBuf[i].view);
        else if(num==4) printf("%6d",aBuf[i].strip);
        else if(num==5) printf("%6d",aBuf[i].adc);
        else if(num==6) printf("%6d",aBuf[i].tdc);
	  }
	  printf("\n");
	}

	fragtag1 = 9; /* 3 for sector 1, 9 for sector 2 */
    fragnum1 = 1;

    if(ind > 0)
	{
      trig ++;
      slot_old = 0;
	  
      i = evOpenFrag(buf, fragtag1, 1);
      printf("evOpenFrag returned %d\n",i);
	  
      ret = evOpenBank(buf, fragtag1, fragnum1, banktag, banknum, banktyp, fmt, &ind_data);
      printf("evOpenBank returns = %d\n",ret);

      b08out = (unsigned char *)&buf[ind_data];
      printf("first b08out = 0x%08x\n",b08out);

	  for(i=0; i<22; i++) nchan[i] = 0;

      for(i=0; i<nw; i++)
      {
        printf("begin while: b08=0x%08x\n",b08);

		layer = (aBuf[i].io-1)*3+aBuf[i].view-1;
		printf("+++++++++++++ %d %d -> %d\n",aBuf[i].io,aBuf[i].view,layer);
        strip = aBuf[i].strip-1;
        slot = adcslotecal [layer] [strip];
		chan = adcchanecal [layer] [strip] - 1;
        printf("### layer=%d strip=%d -> slot=%d chan=%d (nchan(so far)=%d)\n",layer,strip,slot,chan,nchan[slot]);

        if(slot != slot_old)
		{
          slot_old = slot;

          PUT8(slot);
          PUT32(trig);
          PUT64(timestamp);

          nchan[slot]=0;
          nchanptr = (unsigned int *)b08out;
          PUT32(nchan[slot]); /* reserve space for channel counter */

          printf("slot=%d, trig=%d, timestamp=%lld nchan[%d]=%d\n",slot,trig,timestamp,slot,nchan[slot]);
		}

        /*for(nn=0; nn<nchan[slot]; nn++)*/
	    {
	      nchan[slot] ++;
          *nchanptr = nchan[slot];

          PUT8(chan);
          PUT32(npulses);
          printf("  chan=%d, npulses=%d\n",chan,npulses);

          for(mm=0; mm<npulses; mm++)
	      {
            pulse_integral = aBuf[i].adc;
            pulse_time = aBuf[i].tdc-25000; /* sergey: arbitrary !!! */
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
      evCloseBank(buf, fragtag1, fragnum1, banktag, banknum, b08out);
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
