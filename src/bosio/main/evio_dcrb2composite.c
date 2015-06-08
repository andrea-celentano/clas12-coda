
/* evio_dcrb2composite.c */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "evio.h"


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

#define MAXEVENTS 100/*3900*/

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



	/*
    switch(fragment_type) {
        case BANK:
            nw   = buf[ii]+1;
            tag1 = (buf[ii+1]>>16)&0xffff;
            typ1 = (buf[ii+1]>>8)&0x3f;
            pad1 = (buf[ii+1]>>14)&0x3;
            num1 = buf[ii+1]&0xff;
            break;

        case SEGMENT:
            nw   = (buf[ii]&0xffff)+1;
            typ1 = (buf[ii]>>16)&0x3f;
            pad1 = (buf[ii]>>22)&0x3;
            tag1 = (buf[ii]>>24)&0xff;
            num1 = -1;
            break;
    
        case TAGSEGMENT:
            nw   = (buf[ii]&0xffff)+1;
            typ1 = (buf[ii]>>16)&0xf;
            tag1 = (buf[ii]>>20)&0xfff;
            num1 = -1;
            break;

        default:
            printf("?illegal fragment_type in dump_fragment: %d",fragment_type);
            exit(EXIT_FAILURE);
            break;
    }
	*/










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




#define ABS(x) ((x) < 0 ? -(x) : (x))


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
  char fileout[256];
  int bco1[256], bco2[256], bco3[256], bco4[256], nbco1, nbco2, nbco3, nbco4, diff, diff1, diff2;
  char fname[1024];
  int handler_in, handler_out, status, ifpga, nchannels, ind;
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
  int nwpawc,lun,lrec,istat,icycle,idn,nbins,nbins1,igood,offset;
  float x1,x2,y1,y2,ww,tmpx,tmpy,ttt,ref;
  int goodevent, icedev;

  if(argc != 2)
  {
    printf("Usage: evio_dcrbhist <evio_filename>\n");
    exit(1);
  }


  status = evOpen(argv[1],"r",&handler_in);
  if(status < 0)
  {
    printf("evOpen error %d - exit\n",status);
    exit(0);
  }
  else
  {
    printf("Open file >%s< for reading\n",argv[1]);
  }

  sprintf(fileout,"%s.out",argv[1]);
  status = evOpen(fileout,"w",&handler_out);
  if(status < 0)
  {
    printf("evOpen error %d - exit\n",status);
    exit(0);
  }
  else
  {
    printf("Open file >%s< for writing\n",fileout);
  }

  timestamp_old = 0;

  numbank = 1;
  for(iev=1; iev<MAXEVENTS; iev++)
  {

    if(!(iev%10000)) printf("\n\n\nEvent %d\n\n",iev);
#ifdef DEBUG
    printf("\n\n\nEvent %d ++++++++++++++++++++++++++++++++++\n\n",iev);
#endif



    status = evRead(handler_in, buf, MAXBUF);
    if(status < 0)
	{
	  if(status==EOF) printf("evRead end of file after %d events - exit\n",iev);
	  else printf("evRead error=%d after %d events - exit\n",status,iev);
      break;
    }


    printf("evread: %d %d %d %d %d %d %d %d - %d %d %d %d\n",
      buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],buf[8],buf[9],buf[10],buf[11]);


    status = evWrite(handler_out, buf);
    if(status < 0)
	{
	  if(status==EOF) printf("evWrite end of file after %d events - exit\n",iev);
	  else printf("evWrite error=%d after %d events - exit\n",status,iev);
      break;
    }


      /*dcrb*/
      ind1 = evNlink(buf, 42, 0xe105, 42, &nbytes);
      if(ind1 <= 0) ind1 = evNlink(buf, 67, 0xe105, 67, &nbytes);
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
#ifdef DEBUG
        printf("ind1=%d, nbytes=%d\n",ind1,nbytes);fflush(stdout);
#endif
        start = b08 = (unsigned char *) &buf[ind1];
        end = b08 + nbytes;
#ifdef DEBUG
        printf("ind1=%d, nbytes=%d (from 0x%08x to 0x%08x)\n",ind1,nbytes,b08,end);fflush(stdout);
#endif

#ifdef DEBUG
        printf("\n\nBEGIN DCRB EVENT =================================\n");
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
          printf("dcrb data word hex=0x%08x uint=%u int=%d\n",word,word,word);
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

/*printf("slot=%d nw=%d\n",(word>>22)&0x1f,(word>>0)&0x3fffff);*/


/*
if((word>>0)&0x3fffff > 6) goto exit;
*/

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

		/*
        if(goodevent) icedev ++;
		*/

exit:
		;
      }


  }

  printf("evClose after %d events\n",iev);fflush(stdout);
  evClose(handler_in);
  evClose(handler_out);


  exit(0);
}
