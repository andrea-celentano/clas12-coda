
/* evio_tdcecal1.c */
 
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

struct {
  int iquest[100];
} quest_;


#define MAXEVENTS 10000000

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




/* translated data */

#define NHITS 100

#define ABS(x) ((x) < 0 ? -(x) : (x))

#define NCHAN 256 /*how many channels to draw*/

#define TDCLSB 96/*24*/


int
main(int argc, char **argv)
{
  FILE *fd = NULL;
  int nfile;
  char filename[1024];
  int handler, status, ifpga, nchannels, tdcref, tdc1;
  unsigned long long *b64, timestamp, timestamp_old;
  unsigned int *b32;
  unsigned short *b16;
  unsigned char *b08;
  int trig,chan,fpga,apv,hybrid;
  int i1, type, timestamp_flag;
  float f1,f2,bin12;
  unsigned int word;

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
    printf("Usage: evio_tdcecal1 <evio_filename>\n");
    exit(1);
  }


  nwpawc = NWPAWC;
  hlimit_(&nwpawc);
  lun = 11;
  lrec = LREC;

  strcpy(chrunnum,argv[1]);
  ch = strchr(chrunnum,'0');
  ch[6] = '\0';
  runnum = atoi(ch);
  printf("run number is %s (%d)\n",ch,runnum);

  /*sprintf(HBOOKfilename,"tdcecal1%d.his",runnum);*/
  sprintf(HBOOKfilename,"tdcecal1.his");
  hropen_(&lun,"TDC1",HBOOKfilename,"N",&lrec,&istat,strlen("TDC1"),strlen(HBOOKfilename),1);
  if(istat)
  {
    printf("\aError: cannot open RZ file %s for writing.\n", HBOOKfilename);fflush(stdout);
    exit(0);
  }
  else
  {
    printf("RZ file >%s< opened for writing, istat = %d\n\n", HBOOKfilename, istat);fflush(stdout);
  }



  /*******************/
  /* book histograms */

  /* TDC for every slot/channel (1D) 5*125 idn=1000/2000/3000/.../20000 */
  nbins=3000;
  x1 = 1200.;
  x2 = 4200.;
  ww = 0.;
  for(ii=2; ii<=20; ii++)
  {
    for(jj=0; jj<128; jj++)
    {
      idn = ii*1000+jj;
      sprintf(title,"tdc%02d%03d",ii,jj);
      hbook1_(&idn,title,&nbins,&x1,&x2,&ww,strlen(title));
    }
  }

  nbins=200;
  x1 = 1800.;
  x2 = 2000.;
  ww = 0.;
  for(ii=2; ii<=20; ii++)
  {
    for(jj=0; jj<128; jj++)
    {
      idn = 100000+ii*1000+jj;
      sprintf(title,"tdc%02d%03d",ii,jj);
      hbook1_(&idn,title,&nbins,&x1,&x2,&ww,strlen(title));
    }
  }

  nbins=600;
  x1 = 3200.;
  x2 = 3800.;
  ww = 0.;
  idn=11;
  sprintf(title,"tdc ref bins");
  hbook1_(&idn,title,&nbins,&x1,&x2,&ww,strlen(title));

  nbins=500;
  x1 = 0.;
  x2 = 500.;
  ww = 0.;
  idn=12;
  sprintf(title,"timeline");
  hbook1_(&idn,title,&nbins,&x1,&x2,&ww,strlen(title));




iev = 0;
nfile = 0;
bin12 = 0.5;
while(1)
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

    tdcref = 0;
    tdc1 = 0;
	/*TDCs*/
    if((ind1 = evNlink(buf, 19, 0xe107, 10, &nbytes)) > 0)
    {
      int half,chip,chan,bco,val,vvv,chan1,edge,nw,tdcl,tdcr;
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


      /* go through the buffer looking for reference signal */
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
	    chan = (word>>19)&0x7F;
        val = word&0x3FFFF;

        if(slot==6 && chan==0)
		{
          printf("tdcref candidate=%d\n",val);
          /*if(val<1700)*/
		  {
            if(tdcref==0)
		    {
              tdcref=val;
              /*printf("tdcref accepted=%d\n",tdcref);*/
		    }
            else
		    {
              printf("ERROR: double tdcref = %d(%fns), already defined as %d(%fns)\n",
				     val,((float)val)/1000.,tdcref,((float)tdcref)/1000.);
		    }
		  }
		}

        if(slot==6 && chan==1) tdc1 = val;

#ifdef DEBUG
        printf("TDC[0x%08x]:  slot=%2d  chan=%3d  edge=%1d  tdc=%5d  (hist_id=%d)\n",
          word,slot,chan,edge,val,idn);
#endif
	  }

      if(tdc1==0)
	  {
        printf("ERROR: iev=%d -> there is no tdc1 signal\n",iev);
	  }

      if(tdcref==0)
	  {
        printf("ERROR: iev=%d -> there is no ref signal\n",iev);
	  }
	  else
	  {
        /*printf("tdcref final=%d\n",tdcref);*/
        tmpx = (float)tdcref;
        ww = 1.;
        idn = 11;
        hf1_(&idn,&tmpx,&ww);

        if(bin12<500.)
		{
          idn = 12;
          bin12 = bin12 + 1.0;
          hf1_(&idn,&bin12,&tmpx);

          if(tdc1==0)
		  {
            tmpx = 100.;
            idn = 12;
            hf1_(&idn,&bin12,&tmpx);
		  }
		}

        /* go through the buffer again subtracting reference signal */
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
	      chan = (word>>19)&0x7F;
          val = word&0x3FFFF;

          vvv = val + 1460 - tdcref;

          tmpx = (float)val;
          ww = 1.;
          idn = slot*1000+chan;
          hf1_(&idn,&tmpx,&ww);

		  if(vvv>1460) /*do not plot reference itself*/
		  {
            tmpx = (float)vvv;
            ww = 1.;
            idn = 100000+slot*1000+chan;
            hf1_(&idn,&tmpx,&ww);
		  }
		  
		  if(slot==6&&chan==0) printf("-- slot=%2d chan=%3d val=%6d vvv=%6d\n",slot,chan,val,vvv);
		  

#ifdef DEBUG
          printf("TDC[0x%08x]:  slot=%2d  chan=%3d  edge=%1d  tdc=%5d  (hist_id=%d)\n",
            word,slot,chan,edge,val,idn);
#endif
	    }
	  }


	} /* if bank exists */

  } /* while(iev<MAXEVENTS) */




  printf("evClose after %d events\n",iev);fflush(stdout);
  evClose(handler);

  if(iev>=MAXEVENTS) break;

} /*while*/


  /* closing HBOOK file */
  idn = 0;
  printf("befor hrout_\n");fflush(stdout);
  hrout_(&idn,&icycle," ",1);
  printf("after hrout_\n");fflush(stdout);
  hrend_("TDC1", 4);
  printf("after hrend_\n");fflush(stdout);

  exit(0);
}
