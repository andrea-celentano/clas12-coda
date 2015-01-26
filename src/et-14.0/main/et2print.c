
/* et2evio.c */


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

  if((status = et_station_create(et_sys, &et_statid, "FTOF", sconfig)) < 0)
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

#define DEBUG
 

#define MAXEVENTS 10000000

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




/* translated data */

#define NHITS 100
static int nadc[6][2][16], adc[6][2][16][NHITS];
static int ntdc[6][2][16], tdc[6][2][16][NHITS];


#define ABS(x) ((x) < 0 ? -(x) : (x))

#define NCHAN 256 /*how many channels to draw*/

#define TDCRES 41.66667
#define TDCLSB 24


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
  int handle1, buflen;

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
    printf("Usage: et2print <et_filename>\n");
    exit(1);
  }


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
      break;
    }
    maxevents = MAXEVENTS;
  }

  timestamp_old = 0;

  while(iev<maxevents)
  {
    iev ++;

    if(!(iev%10000)) printf("\n\n\nEvent %d\n\n",iev);
#ifdef DEBUG
    printf("\n\n\nEvent %d\n\n",iev);
#endif




	if(use_et)
	{
      if(iet >= nevents)
	  {
        printf("ERROR: iev=%d, nevents=%d\n",iet,nevents);
        exit(0);
	  }
      et_event_getlength(pe[iet], &len); /*get event length from et*/
	  /*printf("len1=%d\n",len);*/


	  /*following 2 lines are working, but 'offitial' is to use  evOpenBuffer()
      bufptr = (unsigned int *)pe[iet]->pdata;
      bufptr += 8;
	  */
      status = evOpenBuffer(pe[iet]->pdata, MAXBUF, "r", &handle1);
      if(status!=0) {printf("evOpenBuffer returns %d\n",status);exit(1);}




      /* in following, you can ether copy from et, or work directly in et */

	  /*actual copy from et
      status = evRead(handle1, buf, MAXBUF);
      if(status!=0) {printf("evRead returns %d\n",status);exit(1);}
	  bufptr = buf;
      */

	  /*get pointer to et buffer (no coping) */
      status = evReadNoCopy(handle1, &bufptr, &buflen);
      if(status!=0) {printf("evReadNoCopy returns %d\n",status);exit(1);}



	  

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




    /*
    for(ii=0; ii<6; ii++)
    {
      for(jj=0; jj<2; jj++)
      {
        for(kk=0; kk<16; kk++)
        {
          nadc[ii][jj][kk] = 0;
          ntdc[ii][jj][kk] = 0;
	    }
	  }
    }
    tdcref = 0;
	*/



	/*TDCs*/
    if((ind1 = evNlink(bufptr, 28, 0xe107, 10, &nbytes)) > 0)
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
        val = (word&0x3FFFF)*TDCLSB;
#ifdef DEBUG
        printf("TDC[0x%08x]:  slot=%2d  chan=%3d  edge=%1d  tdc=%5d\n",
          word,slot,chan,edge,val);
#endif
	  }
	}


	/* ADC raw mode bank */
    if((ind1 = evNlink(bufptr, 8, 0xe101, 8, &nbytes)) > 0)
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
              printf("slot=%d chan=%d baseline=%d\n",slot,chan,baseline);
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
		  if(slot < 17)
		  {
            ii = adclayer[slot][chan] - 1;
            jj = adclr[slot][chan] - 1;
            kk = adcslab[slot][chan] - 1;
			if(ii>=0 && sum > 0)
			{
              adc[ii][jj][kk][nadc[ii][jj][kk]] = sum;
              nadc[ii][jj][kk] ++;
			}
		  }
		  */



        }
#ifdef DEBUG
        printf("end loop: b08=0x%08x\n",b08);
#endif
      }
    }

	

	/* ADC pulsed mode bank */
    if((ind1 = evNlink(bufptr, 27, 0xe103, 10, &nbytes)) > 0)
    {
      unsigned short pulse_time;
      unsigned int pulse_integral;
      unsigned char *end;
      unsigned long long time;
      int crate,slot,trig,nchan,chan,npulses,notvalid,edge,data,count,ncol1,nrow1;
      int oldslot = 100;
      int ndata0[22], data0[21][8];
      int baseline, sum, channel;

      b08 = (unsigned char *) &bufptr[ind1];
      b16 = (unsigned short *) &bufptr[ind1];
      b32 = (unsigned int *) &bufptr[ind1];

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


			/*
			if(slot < 17)
			{
              ii = adclayer[slot][chan] - 1;
              jj = adclr[slot][chan] - 1;
              kk = adcslab[slot][chan] - 1;
			  if(ii>=0)
			  {
                adc[ii][jj][kk][nadc[ii][jj][kk]] = pulse_integral;
                nadc[ii][jj][kk] ++;
		      }
		    }
            */

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
	/*
    if(tdcref==0)
	{
      printf("ERROR: iev=%d -> there is no ref signal\n",iev);
	}
    else
	{
      for(ii=0; ii<6; ii++)
      {
        for(jj=0; jj<2; jj++)
        {
          for(kk=0; kk<16; kk++)
          {
            if(ntdc[ii][jj][kk]>1)
			{
              ntdc[ii][jj][kk]=1;
			  printf("ev %5d double tdc: %d %d %d -> %d\n",iev,ii,jj,kk,tdc[ii][jj][kk][1]);
			}
            for(nn=0; nn<ntdc[ii][jj][kk]; nn++)
		    {
              tdc[ii][jj][kk][nn] = tdc[ii][jj][kk][nn] + 250000 - tdcref;
		    }
	      }
	    }
      }
	}
	*/	

	/*
       adc[ii][jj][kk][nadc[ii][jj][kk]]
       nadc[ii][jj][kk]

       tdc[ii][jj][kk][ntdc[ii][jj][kk]]
       ntdc[ii][jj][kk]
	*/

    if(use_et)
	{
      status = evClose(handle1);
      if(status!=0) {printf("evClose returns %d\n",status);exit(1);}
	}

  } /*while*/



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


  exit(0);
}
