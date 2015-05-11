
/* evio_atcahist.c */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#undef DEBUG_SEARCH
#undef DEBUG_SEARCH1

#undef DEBUG1
#undef DEBUG
 
#define NWPAWC 10000000 /* Length of the PAWC common block. */
#define LREC 1024      /* Record length in machine words. */

struct {
  float hmemor[NWPAWC];
} pawc_;

#define LSWAP(x)        ((((x) & 0x000000ff) << 24) | \
                         (((x) & 0x0000ff00) <<  8) | \
                         (((x) & 0x00ff0000) >>  8) | \
                         (((x) & 0xff000000) >> 24))

#define MAXEVENTS 10000000/*3900*/

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
  printf("\n\n0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
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



enum {
  BANK = 0,
  SEGMENT,
  TAGSEGMENT,
};

int
evNlink1(unsigned int *buf, int tag, int num, int *nbytes)
{
  int ii, len, nw, tag1, pad1, typ1, num1, len2, pad3, ind, fragment_type;

#ifdef DEBUG_SEARCH1
  printf("\n\nevNlink1===========\n0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
		 buf[0],buf[1],buf[2],buf[3],buf[4],buf[5]);
  printf("%d %d %d %d %d %d\n",
		 buf[0],buf[1],buf[2],buf[3],buf[4],buf[5]);
#endif


  /*
  bank: 0xe, 0x10
  segment: 0xd, 0x20
  tagsegment: 0xc
  */


  len = buf[0]+1;
  ii = 2;
  while(ii<len)
  {


    nw = buf[ii] + 1;
    tag1 = (buf[ii+1]>>16)&0xffff;
    pad1 = (buf[ii+1]>>14)&0x3;
    typ1 = (buf[ii+1]>>8)&0x3f;
    num1 =  buf[ii+1]&0xff;



#ifdef DEBUG_SEARCH1
    printf("[%5d] nw=%d, tag1=0x%04x(%d), pad1=0x%02x, typ1=0x%02x, num1=0x%02x\n",ii,nw,tag1,tag1,pad1,typ1,num1);
#endif

#ifdef DEBUG_SEARCH1
    printf("search ==> %d=%d?  %d=%d?\n",tag1,tag,num1,num);
#endif
    /*if(typ1!=0xe && typ1!=0x10)*/ /*assumes there are no bank-of-banks inside fragment, will redo later*/
	{
    if( tag1==tag && num1==num )
    {
      if(typ1!=0xf)
	  {
#ifdef DEBUG_SEARCH1
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
#ifdef DEBUG_SEARCH1
		printf(">>> found composite bank: tag=%d, type=%d, exclusive len=%d (padding from internal bank=%d)\n",((buf[ii+2]>>20)&0xfff),((buf[ii+2]>>16)&0xf),len2-1,pad3);
        printf("return composite bank data index %d\n",ii+2+len2+2);
#endif
        *nbytes = ((nw-(2+len2+2))<<2)-pad3;
#ifdef DEBUG_SEARCH1
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


/* translation table from Mac Mestayer
		conn.	sip/pin	layer	wire
		1	1	2	1
		1	2	4	1
		1	3	6	1
		1	4	1	1
		1	5	3	1
		1	6	5	1
		1	7	2	2
		1	8	4	2
		1	9	6	2
		1	10	1	2
		1	11	3	2
		1	12	5	2
		1	13	2	3
		1	14	4	3
		1	15	6	3
		1	16	1	3
		2	1	3	3
		2	2	5	3
		2	3	2	4
		2	4	4	4
		2	5	6	4
		2	6	1	4
		2	7	3	4
		2	8	5	4
		2	9	2	5
		2	10	4	5
		2	11	6	5
		2	12	1	5
		2	13	3	5
		2	14	5	5
		2	15	2	6
		2	16	4	6
		3	1	6	6
		3	2	1	6
		3	3	3	6
		3	4	5	6
		3	5	2	7
		3	6	4	7
		3	7	6	7
		3	8	1	7
		3	9	3	7
		3	10	5	7
		3	11	2	8
		3	12	4	8
		3	13	6	8
		3	14	1	8
		3	15	3	8
		3	16	5	8
		4	1	2	9
		4	2	4	9
		4	3	6	9
		4	4	1	9
		4	5	3	9
		4	6	5	9
		4	7	2	10
		4	8	4	10
		4	9	6	10
		4	10	1	10
		4	11	3	10
		4	12	5	10
		4	13	2	11
		4	14	4	11
		4	15	6	11
		4	16	1	11
		5	1	3	11
		5	2	5	11
		5	3	2	12
		5	4	4	12
		5	5	6	12
		5	6	1	12
		5	7	3	12
		5	8	5	12
		5	9	2	13
		5	10	4	13
		5	11	6	13
		5	12	1	13
		5	13	3	13
		5	14	5	13
		5	15	2	14
		5	16	4	14
		6	1	6	14
		6	2	1	14
		6	3	3	14
		6	4	5	14
		6	5	2	15
		6	6	4	15
		6	7	6	15
		6	8	1	15
		6	9	3	15
		6	10	5	15
		6	11	2	16
		6	12	4	16
		6	13	6	16
		6	14	1	16
		6	15	3	16
		6	16	5	16
*/



static int board_layer[96] = {
  2, 4, 6, 1, 3, 5, 2, 4, 6, 1, 3, 5, 2, 4, 6, 1,
  3, 5, 2, 4, 6, 1, 3, 5, 2, 4, 6, 1, 3, 5, 2, 4,
  6, 1, 3, 5, 2, 4, 6, 1, 3, 5, 2, 4, 6, 1, 3, 5,
  2, 4, 6, 1, 3, 5, 2, 4, 6, 1, 3, 5, 2, 4, 6, 1,
  3, 5, 2, 4, 6, 1, 3, 5, 2, 4, 6, 1, 3, 5, 2, 4,
  6, 1, 3, 5, 2, 4, 6, 1, 3, 5, 2, 4, 6, 1, 3, 5
};

static int board_wire[96] = {
  1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3,
  3, 3, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 6, 6,
  6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8,
  9, 9, 9, 9, 9, 9,10,10,10,10,10,10,11,11,11,11,
 11,11,12,12,12,12,12,12,13,13,13,13,13,13,14,14,
 14,14,14,14,15,15,15,15,15,15,16,16,16,16,16,16
};


static int superlayer[21][96];
static int layer[21][96];
static int wire[21][96];



#define ABS(x) ((x) < 0 ? -(x) : (x))


#define DC_DATA_BLOCK_HEADER      0x00
#define DC_DATA_BLOCK_TRAILER     0x01
#define DC_DATA_EVENT_HEADER      0x02
#define DC_DATA_TRIGGER_TIME      0x03
#define DC_DATA_DCRBEVT           0x08
#define DC_DATA_INVALID           0x0E
#define DC_DATA_FILLER            0x0F


#define NCHAN 256 /*how many channels to draw*/


static int ced[2][6][112];


int
main(int argc, char **argv)
{
  FILE *fd = NULL;
  int bco1[256], bco2[256], bco3[256], bco4[256], nbco1, nbco2, nbco3, nbco4, diff, diff1, diff2;
  char fname[1024];
  int handler, status, ifpga, nchannels, ind;
  unsigned char numbank;
  unsigned long long *b64, timestamp, timestamp_old;
  unsigned int *b32;
  unsigned short *b16;
  unsigned char *b08;
  int trig,slot,chan,fpga,apv,hybrid;
  int i1, type, timestamp_flag;
  float f1,f2;
  unsigned int word, *gsegm;

  int nr,sec,strip,nl,ncol,nrow,i,j, k, ii,jj,kk,l,l1,l2,ichan,nn,iev,nbytes,ind1;
  char title[128];
  char *HBOOKfile = "atcahist.his";
  int nwpawc,lun,lrec,istat,icycle,idn,nbins,nbins1,igood,offset;
  float x1,x2,y1,y2,ww,tmpx,tmpy,ttt,ref;
  int goodevent, icedev;

  if(argc != 2)
  {
    printf("Usage: evio_atcahist <evio_filename>\n");
    exit(1);
  }

  nwpawc = NWPAWC;
  hlimit_(&nwpawc);
  lun = 11;
  lrec = LREC;
  hropen_(&lun,"NTUPEL",HBOOKfile,"N",&lrec,&istat,strlen("NTUPEL"),strlen(HBOOKfile),1);
  if(istat)
  {
    printf("\aError: cannot open RZ file %s for writing.\n", HBOOKfile);fflush(stdout);
    exit(0);
  }
  else
  {
    printf("RZ file >%s< opened for writing, istat = %d\n\n", HBOOKfile, istat);fflush(stdout);
  }



  for(slot=4; slot<11; slot++)
  {
    for(chan=0; chan<96; chan++)
    {
      superlayer[slot][chan] = 0;
      layer[slot][chan] = board_layer[chan]-1;
      wire[slot][chan] = (board_wire[chan]-1)+(slot-4)*16;
	  /*
	  printf("translation table: slot %d, chan %d -> sl=%d la=%d w=%d\n",
        slot,chan,superlayer[slot][chan],layer[slot][chan],wire[slot][chan]);
	  */
	}
  }
  for(slot=13; slot<20; slot++)
  {
    for(chan=0; chan<96; chan++)
    {
      superlayer[slot][chan] = 1;
      layer[slot][chan] = board_layer[chan]-1;
      wire[slot][chan] = (board_wire[chan]-1)+(slot-13)*16;
	  /*
	  printf("translation table: slot %d, chan %d -> sl=%d la=%d w=%d\n",
        slot,chan,superlayer[slot][chan],layer[slot][chan],wire[slot][chan]);
	  */
	}
  }



  nbins=1000;
  x1 = 0.;
  x2 = 1000.;
  ww = 0.;
  idn=1;
  sprintf(title,"chan0");
  hbook1_(&idn,title,&nbins,&x1,&x2,&ww,strlen(title));

  nbins=1000;
  x1 = 0.;
  x2 = 1000.;
  ww = 0.;
  idn=2;
  sprintf(title,"chan2");
  hbook1_(&idn,title,&nbins,&x1,&x2,&ww,strlen(title));

  nbins=1000;
  x1 = -1500.;
  x2 = 1500.;
  ww = 0.;
  idn=3;
  sprintf(title,"chan0-chan2");
  hbook1_(&idn,title,&nbins,&x1,&x2,&ww,strlen(title));




  nbins=1000;
  x1 = -1000.;
  x2 = 1000.;
  ww = 0.;
  idn=10;
  sprintf(title,"timestampdiff");
  hbook1_(&idn,title,&nbins,&x1,&x2,&ww,strlen(title));



  nbins=1000;
  x1 = 0.;
  x2 = 2000.;
  ww = 0.;
  for(ii=4; ii<=19; ii++)
  {
    for(jj=0; jj<96; jj++)
    {
      idn = ii*100+jj;
      sprintf(title,"tdc%02d%02d",ii,jj);
      hbook1_(&idn,title,&nbins,&x1,&x2,&ww,strlen(title));
    }
  }
  idn=11;
  sprintf(title,"tdc_all");
  hbook1_(&idn,title,&nbins,&x1,&x2,&ww,strlen(title));




  nbins=200;
  x1 = -100.;
  x2 = 100.;
  ww = 0.;
  for(ii=4; ii<=19; ii++)
  {
    for(jj=0; jj<96; jj++)
    {
      idn = ii*1000+jj;
      sprintf(title,"tdcdiff%02d%02d",ii,jj);
      hbook1_(&idn,title,&nbins,&x1,&x2,&ww,strlen(title));
    }
  }



  /*
  nbins=32;
  nbins1=32;
  x1 = 0.;
  x2 = 512.;
  y1 = 0.;
  y2 = 512.;
  ww = 0.;
  idn = 10;
  sprintf(title,"dcrb10");
  hbook2_(&idn,title,&nbins,&x1,&x2,&nbins1,&y1,&y2,&ww,strlen(title));
  */


  status = evOpen(argv[1],"r",&handler);
  if(status < 0)
  {
    printf("evOpen error %d - exit\n",status);
    exit(0);
  }

  timestamp_old = 0;

  numbank = 1;
  for(iev=1; iev<MAXEVENTS; iev++)
  {

    if(!(iev%10000)) printf("\n\n\nEvent %d\n\n",iev);
#ifdef DEBUG
    printf("\n\n\nEvent %d ++++++++++++++++++++++++++++++++++\n\n",iev);
#endif



    status = evRead(handler, buf, MAXBUF);
    if(status < 0)
	{
	  if(status==EOF) printf("end of file after %d events - exit\n",iev);
	  else printf("evRead error=%d after %d events - exit\n",status,iev);
      break;
    }

    if(iev < 3) continue; /*skip first 2 events*/


  for(i=0; i<2; i++)
	for(j=0; j<6; j++)
	  for(k=0; k<112; k++)
		ced[i][j][k] = 0;
  


      /*atca*/
      ind1 = evNlink(buf, 51, 3, 51, &nbytes);
      /*if(ind1 <= 0) ind1 = evNlink(buf, 67, 0xe105, 67, &nbytes);*/
      if(ind1 > 0)
      {
        int half,chip,chan,bco,tdc,tdcref,chan1,edge,nw;
        unsigned char *end, *start;
        unsigned int tmp;
        float tmpx0, tmpx2, dcrbref;
        unsigned int temp[6];
        unsigned sample[6];
        int slot;
        int ndata0[22], data0[21][8];
        int baseline, sum, channel, ch1;
        unsigned int head[2];


        head[0] = buf[ind1-2];
        head[1] = buf[ind1-1];
		printf("\nbank length = %d\n",head[0]);
        printf("syncFlag=0x%02x\n",head[1]>>24);




#ifdef DEBUG
        printf("ind1=%d, nbytes=%d\n",ind1,nbytes);fflush(stdout);
#endif
        start = b08 = (unsigned char *) &buf[ind1];
        end = b08 + nbytes;
#ifdef DEBUG
        printf("ind1=%d, nbytes=%d (from 0x%08x to 0x%08x)\n",ind1,nbytes,b08,end);fflush(stdout);
#endif

#ifdef DEBUG
        printf("\n\nBEGIN ATCA EVENT =================================\n");
#endif

        tdcref = 0;
        timestamp_flag = 0;
        while(b08<end)
	    {
#ifdef DEBUG
          /*printf("begin while: b08=0x%08x\n",b08);*/
#endif
          GET32(word);

#ifdef DEBUG
          printf("dpm data word hex=0x%08x uint=%u int=%d\n",word,word,word);
#endif
          if(timestamp_flag)
		  {
		    timestamp |= (word&0xffffff);
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
/*printf("slot=%d\n",slot);*/

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
#ifdef DEBUG
		        printf(" {EVTHDR} EVENT: %d\n", (word>>0)&0x7ffffff);fflush(stdout);
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

		        break;
		      case DC_DATA_INVALID:
		        printf(" {***DNV***}\n");fflush(stdout);
                goto exit;
		        break;
		      case DC_DATA_FILLER:
#ifdef DEBUG
		        printf(" {FILLER}\n");fflush(stdout);
#endif
		        break;
		      default:
		        printf(" {***DATATYPE ERROR***}\n");fflush(stdout);
                goto exit;
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


#ifdef DEBUG
        printf("END DCRB EVENT =================================\n\n\n");
#endif

exit:
		;
      }





  }

  printf("evClose after %d events\n",iev);fflush(stdout);
  evClose(handler);


  /* closing HBOOK file */
  idn = 0;
  printf("befor hrout_\n");fflush(stdout);
  hrout_(&idn,&icycle," ",1);
  printf("after hrout_\n");fflush(stdout);
  hrend_("NTUPEL", 6);
  printf("after hrend_\n");fflush(stdout);


  exit(0);
}
