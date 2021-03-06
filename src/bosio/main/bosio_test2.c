
/* bosio_test2.c */
 
#include <stdio.h>
#include <math.h>
 
#include "bosio.h"

#define NBCS 700000
#include "bcs.h"

#define RADDEG 57.2957795130823209
#define NWPAWC 1000000 /* Length of the PAWC common block. */
#define LREC 1024      /* Record length in machine words. */

struct {
  float hmemor[NWPAWC];
} pawc_;


  

typedef struct LeCroy1877Head
{
  unsigned slot    :  5;
  unsigned empty0  :  3;
  unsigned empty1  :  8;
  unsigned empty   :  5;
  unsigned count   : 11;
} MTDCHead;

typedef struct LeCroy1877
{
  unsigned slot    :  5;
  unsigned type    :  3;
  unsigned channel :  7;
  unsigned edge    :  1;
  unsigned data    : 16;
} MTDC;

typedef struct LeCroy1881MHead *ADCHeadPtr;
typedef struct LeCroy1881MHead
{
  unsigned slot    :  5;
  unsigned empty   : 20;
  unsigned count   :  7;
} ADCHead;

typedef struct LeCroy1881M *ADCPtr;
typedef struct LeCroy1881M
{
  unsigned slot    :  5;
  unsigned type    :  3;
  unsigned channel :  7;
  unsigned empty   :  3;
  unsigned data    : 14;
} ADC;

void
add_bank(char *name, int num, char *format, int ncol, int nrow, int ndata, int data[])
{
  int i,ind;

printf("format >%s<\n",format);
printf("name >%s<\n",name);

  bosNformat(bcs_.iw,name,format);
/*bosnformat_(bcs_.iw,name,format,strlen(name),strlen(format));*/

  ind=bosNcreate(bcs_.iw,name,num,ncol,nrow);
printf("ind=%d\n",ind);

  /*  for(i=0; i<ndata; i++) bcs_.iw[ind+i]=data[i];*/

  return;
}





main(int argc, char **argv)
{
int nr,layer,strip,nl,ncol,nrow,i,j,l,l1,l2,ichan,nn,iev;
int ind,ind1,ind2,status,status1,handle,handle1,k,m;
int scal,nw,scaler,scaler_old;
unsigned int hel,str,strob,helicity,strob_old,helicity_old;
int tmp1 = 1, tmp2 = 2, iret, bit1, bit2;
float *bcsfl, rndm_();
char strr[1000];

int ecgood1[7][7][37];
int ecbad1[7][7][37];


static char *str1 = "OPEN INPUT UNIT=1 FILE=\"FPACK.DAT\" RECL=36000";
static char *str2 = "OPEN OUTPUT UNIT=2 FILE=\"FPACK.A00\" RECL=36000 SPLITMB=2 RAW WRITE SEQ NEW BINARY";
/*static char *str3 = "OPEN INPUT UNIT=1 FILE=\"FPACK.A00-A06\" ";*/
/*static char *str3 = "OPEN INPUT UNIT=1 FILE=\"/mnt/raid3/stage_in/clas_017599.A00\" ";*/

static char *str3 = "OPEN INPUT UNIT=1 FILE=\"/raid/stage_in/dctest_037348.A00\" ";
/*static char *str3 = "OPEN INPUT UNIT=1 FILE=\"/work/clas/disk1/boiarino/clas_030921.A00\" ";*/
/*static char *str3 = "OPEN INPUT UNIT=1 FILE=\"/work/boiarino/dctest_036884.A00\" ";*/

/*static char *str3 = "OPEN INPUT UNIT=1 FILE=\"./test.dat\" ";*/
/*static char *str3 = "OPEN INPUT UNIT=1 FILE=\"/work/clas/disk1/boiarino/test.dat\" ";*/
static char *str4 = "OPEN OUTPUT UNIT=2 FILE=\"./test.dat\" RECL=32768 SPLITMB=2047 WRITE SEQ NEW BINARY";

static char *str5 = "OPEN INPUT UNIT=1 FILE=\"FPACK6.DAT\" RECL=36000";
static char *str6 = "OPEN INPUT UNIT=1 FILE=\"/work/clas/disk3/sep97/cerenkov/run9470.B00.00\" RECL=36000";

static int syn[32], id;

char *HBOOKfile = "test.his";
int nwpawc,lun,lrec,istat,icycle,idn,nbins,nbins1,igood,offset;
int lookup[32] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  25,27,29,31,33,35, 2, 4, 6, 8,10,12,14,16,18,20};
float x1,x2,y1,y2,ww,tmpx,tmpy,ref;
unsigned short *tlv1, *tlv2, *buf16;

/* run 7989
int nev[] = {39,79,673,2664,2707,3095,3213,3260,3371,3505,
      3579,4169,4557,4788,5259,5463,6749,7496,8897,9297,
      10367,11424,11557,11579,11586,11768,12362,12518,12606,13816,
      16240,16273,17282,17773,18615,18734,19488,20347,20537,21874,
      22033,22789,23930,24669,25242,25532,26017,26862,27397,28657, 
      29282,30074,30322,31437,31940,33371,34731,35779,36434,38831,
      40334,40705,42204,42519,51655,52779,54069,54496,54613,55074,
      56173,56367,56484,56931,57382,57651,59403,59810,60212,60232,
      60490,60736,61532,61541,62475,63347,64891,66897,68282,69776,
      69795,69842,70703,71306,71500,72418,72493,73114,73799,74499, 
      74911,75662,76037,78186,79133,79733,81814,82249,82922,83123,
      83164,84293,84520,84600,85242,85329,85592,86511,86535,87335,
      88148,89678,90165,90180,90374,90378,91124,91393,92824,93293,
      93656,93876,94031,94089,94131,94847,96189,96478,97502,97697,
      99808,100181,100673,100760,100779,101562,101709,102299,102938,
      103504,104531,105743,107690,107823,108397,108685,109792,111374,
      112641,112835};
*/

/* run 9183 */
int nev[] = {1358,1573,2027,2166,2838,4242,4350,6143,6645,7212,
      7608,9227,9279,10626,12002,12437,13116,14429,14814,16829,
      17093,17430,18568,18569,20426,20784,21338,21495,21792,
      24377,24466,24941,27990,28409,29950};


  printf(" boshist2 reached !\n");


  if(argc != 2)
  {
    printf("Usage: boshist2 <fpack_filename>\n");
    exit(1);
  }



  bcsfl = (float*)bcs_.iw;
  bosInit(bcs_.iw,NBCS);




  /* v1190 test */

  nwpawc = NWPAWC;
  hlimit_(&nwpawc);
  lun = 11;
  lrec = LREC;
  hropen_(&lun,"NTUPEL",HBOOKfile,"N",&lrec,&istat,
     strlen("NTUPEL"),strlen(HBOOKfile),1);
  if(istat)
  {
    printf("\aError: cannot open RZ file %s for writing.\n", HBOOKfile);
    exit(0);
  }
  else
  {
    printf("RZ file >%s< opened for writing, istat = %d\n\n", HBOOKfile, istat);
  }


  /* REF bank */
  nbins=1000;
  x1 = 0.;
  x2 = 50000.;
  ww = 0.;

  idn = 2015;
  hbook1_(&idn,"roc20slot15",&nbins,&x1,&x2,&ww,11);
  idn = 2016;
  hbook1_(&idn,"roc20slot16",&nbins,&x1,&x2,&ww,11);
  idn = 2017;
  hbook1_(&idn,"roc20slot17",&nbins,&x1,&x2,&ww,11);
  idn = 2018;
  hbook1_(&idn,"roc20slot18",&nbins,&x1,&x2,&ww,11);
  idn = 2019;
  hbook1_(&idn,"roc20slot19",&nbins,&x1,&x2,&ww,11);
  idn = 2020;
  hbook1_(&idn,"roc20slot20",&nbins,&x1,&x2,&ww,11);
  idn = 2021;
  hbook1_(&idn,"roc20slot21",&nbins,&x1,&x2,&ww,11);

  idn = 2115;
  hbook1_(&idn,"roc21slot15",&nbins,&x1,&x2,&ww,11);
  idn = 2116;
  hbook1_(&idn,"roc21slot16",&nbins,&x1,&x2,&ww,11);
  idn = 2117;
  hbook1_(&idn,"roc21slot17",&nbins,&x1,&x2,&ww,11);
  idn = 2118;
  hbook1_(&idn,"roc21slot18",&nbins,&x1,&x2,&ww,11);
  idn = 2119;
  hbook1_(&idn,"roc21slot19",&nbins,&x1,&x2,&ww,11);
  idn = 2120;
  hbook1_(&idn,"roc21slot20",&nbins,&x1,&x2,&ww,11);
  idn = 2121;
  hbook1_(&idn,"roc21slot21",&nbins,&x1,&x2,&ww,11);

  idn = 2217;
  hbook1_(&idn,"roc22slot17",&nbins,&x1,&x2,&ww,11);
  idn = 2218;
  hbook1_(&idn,"roc22slot18",&nbins,&x1,&x2,&ww,11);
  idn = 2219;
  hbook1_(&idn,"roc22slot19",&nbins,&x1,&x2,&ww,11);
  idn = 2220;
  hbook1_(&idn,"roc22slot20",&nbins,&x1,&x2,&ww,11);
  idn = 2221;
  hbook1_(&idn,"roc22slot21",&nbins,&x1,&x2,&ww,11);

  idn = 2909;
  hbook1_(&idn,"roc29slot09",&nbins,&x1,&x2,&ww,11);
  idn = 2910;
  hbook1_(&idn,"roc29slot10",&nbins,&x1,&x2,&ww,11);
  idn = 2911;
  hbook1_(&idn,"roc29slot11",&nbins,&x1,&x2,&ww,11);
  idn = 2912;
  hbook1_(&idn,"roc29slot12",&nbins,&x1,&x2,&ww,11);

  nbins=1000;
  x1 = 0.;
  x2 = 10000.;
  ww = 0.;

  idn = 10;
  hbook1_(&idn,"all",&nbins,&x1,&x2,&ww,4);

  idn = 100;
  hbook1_(&idn,"ch00",&nbins,&x1,&x2,&ww,4);
  idn = 101;
  hbook1_(&idn,"ch01",&nbins,&x1,&x2,&ww,4);
  idn = 102;
  hbook1_(&idn,"ch02",&nbins,&x1,&x2,&ww,4);
  idn = 103;
  hbook1_(&idn,"ch03",&nbins,&x1,&x2,&ww,4);
  idn = 104;
  hbook1_(&idn,"ch04",&nbins,&x1,&x2,&ww,4);
  idn = 105;
  hbook1_(&idn,"ch05",&nbins,&x1,&x2,&ww,4);
  idn = 106;
  hbook1_(&idn,"ch06",&nbins,&x1,&x2,&ww,4);
  idn = 107;
  hbook1_(&idn,"ch07",&nbins,&x1,&x2,&ww,4);
  idn = 108;
  hbook1_(&idn,"ch08",&nbins,&x1,&x2,&ww,4);
  idn = 109;
  hbook1_(&idn,"ch09",&nbins,&x1,&x2,&ww,4);
  idn = 110;
  hbook1_(&idn,"ch10",&nbins,&x1,&x2,&ww,4);
  idn = 111;
  hbook1_(&idn,"ch11",&nbins,&x1,&x2,&ww,4);
  idn = 112;
  hbook1_(&idn,"ch12",&nbins,&x1,&x2,&ww,4);
  idn = 113;
  hbook1_(&idn,"ch13",&nbins,&x1,&x2,&ww,4);
  idn = 114;
  hbook1_(&idn,"ch14",&nbins,&x1,&x2,&ww,4);
  idn = 115;
  hbook1_(&idn,"ch15",&nbins,&x1,&x2,&ww,4);

  idn = 162;
  hbook1_(&idn,"ch62",&nbins,&x1,&x2,&ww,4);
  idn = 163;
  hbook1_(&idn,"ch63",&nbins,&x1,&x2,&ww,4);

  nbins=1000;
  x1 = 16500.;
  x2 = 17500.;
  ww = 0.;



  idn = 200;
  hbook1_(&idn,"ch00",&nbins,&x1,&x2,&ww,4);
  idn = 201;
  hbook1_(&idn,"ch01",&nbins,&x1,&x2,&ww,4);
  idn = 202;
  hbook1_(&idn,"ch02",&nbins,&x1,&x2,&ww,4);
  idn = 203;
  hbook1_(&idn,"ch03",&nbins,&x1,&x2,&ww,4);
  idn = 204;
  hbook1_(&idn,"ch04",&nbins,&x1,&x2,&ww,4);
  idn = 205;
  hbook1_(&idn,"ch05",&nbins,&x1,&x2,&ww,4);
  idn = 206;
  hbook1_(&idn,"ch06",&nbins,&x1,&x2,&ww,4);
  idn = 207;
  hbook1_(&idn,"ch07",&nbins,&x1,&x2,&ww,4);
  idn = 208;
  hbook1_(&idn,"ch08",&nbins,&x1,&x2,&ww,4);
  idn = 209;
  hbook1_(&idn,"ch09",&nbins,&x1,&x2,&ww,4);
  idn = 210;
  hbook1_(&idn,"ch10",&nbins,&x1,&x2,&ww,4);
  idn = 211;
  hbook1_(&idn,"ch11",&nbins,&x1,&x2,&ww,4);
  idn = 212;
  hbook1_(&idn,"ch12",&nbins,&x1,&x2,&ww,4);
  idn = 213;
  hbook1_(&idn,"ch13",&nbins,&x1,&x2,&ww,4);
  idn = 214;
  hbook1_(&idn,"ch14",&nbins,&x1,&x2,&ww,4);
  idn = 215;
  hbook1_(&idn,"ch15",&nbins,&x1,&x2,&ww,4);


  /*
#define PRINT1 1
  */
  /*
#define PRINT2 1
  */

for(nr=0;nr<=6;nr++)
  for(layer=0;layer<=6;layer++)
    for(strip=0;strip<=36;strip++)
	{
      ecgood1[nr][layer][strip] = 0;
      ecbad1[nr][layer][strip] = 0;
	}

  sprintf(strr,"OPEN INPUT UNIT=1 FILE='%s' ",argv[1]);
  printf("fparm string: >%s<\n",strr);
  status = fparm_(strr,strlen(strr));
  for(iev=100; iev<100100; iev++)
  {
    frbos_(bcs_.iw,&tmp1,"E",&iret,1);

	/*
printf("===== Event no. %d\n",iev);
	*/

	/* RAW bank */
    if((ind1=bosNlink(bcs_.iw,"RC20",0)) > 0)
    {
      unsigned int *raw1;
      int crate,slot,channel,edge,data,count,ncol1,nrow1;
      int oldslot = 100;
      int ndata0[22], data0[21][8];

      ncol1 = bcs_.iw[ind1-6];
      nrow1 = bcs_.iw[ind1-5];
      nw = nrow1;
      offset = 0;
	  /*
      printf("ncol=%d nrow=%d nw=%d\n",ncol1,nrow1,nw);
	  */
      ww = 1.0;

      /*printf("\n");*/
      raw1 = (unsigned int *)&bcs_.iw[ind1];
      for(k=0; k<nrow1; k++) /* skip first word - board header */
      {
        slot = (raw1[0]>>27)&0x1f;
        channel = (raw1[0]>>19)&0x1f;
        edge = (raw1[0]>>26)&0x1;
        data = raw1[0]&0x7ffff;
		/*
        printf("RAW: slot=%d chan=%d data=%d\n",
          slot,channel,data);
		*/

        raw1 ++;
	  }
	}


	/* REF bank */
    if((ind1=bosNlink(bcs_.iw,"REF ",0)) > 0)
    {
      unsigned short *ref1;
      int crate,slot,channel,edge,data,count,ncol1,nrow1;
      int oldslot = 100;
      int ndata0[22], data0[21][8];

      ncol1 = bcs_.iw[ind1-6];
      nrow1 = bcs_.iw[ind1-5];
      nw = nrow1;
      offset = 0;
	  /*
      printf("ncol=%d nrow=%d nw=%d\n",ncol1,nrow1,nw);
	  */
      ww = 1.0;

      /*printf("\n");*/
      ref1 = (unsigned short *)&bcs_.iw[ind1];
      for(k=0; k<nrow1; k++) /* skip first word - board header */
      {
        crate = (ref1[0]>>8)&0xFF;
        slot = ref1[0]&0xFF;
        data = ref1[1];


        tmpx = (float)data;
        idn=crate*100+slot;
		/*
        printf("REF: crate=%d slot=%d data=%d -> hist=%d\n",
          crate,slot,data,idn);
		*/
        hf1_(&idn,&tmpx,&ww);

        ref1 += 2;
	  }
	}


#define SLMIN 15

	/* check for reference channels */
    if((ind1=bosNlink(bcs_.iw,"RC20",0)) > 0)
    {
      unsigned int *tlv1;
      int slot,channel,edge,data,count,ncol1,nrow1;
      int oldslot = 100;
      int ndata0[22], data0[21][8];

      for(i=0; i<=21; i++) ndata0[i] = 0;

      ncol1 = bcs_.iw[ind1-6];
      nrow1 = bcs_.iw[ind1-5];
      nw = nrow1;
      offset = 0;
	  /*
      printf("ncol=%d nrow=%d nw=%d\n",ncol1,nrow1,nw);
	  */
      ww = 1.0;

      tlv1 = (unsigned int *)&bcs_.iw[ind1];
      for(k=0; k<nrow1; k++) /* skip first word - board header */
      {
        slot = (tlv1[k]>>27)&0x1f;
        channel = (tlv1[k]>>19)&0x1f;
        edge = (tlv1[k]>>26)&0x1;
        data = tlv1[k]&0x7ffff;
		/*
        printf("slot=%2d chan=%3d edge=%1d data=%6d\n",slot,channel,edge,data);
		*/

        if(slot == 0) continue; /* skip board header */
        if(channel==0)
        {
          offset = data;

          if(ndata0[slot] < 8)
          {
            data0[slot][ndata0[slot]] = data;
            ndata0[slot] ++;
          }
          else
          {
            printf("ERROR: slot=%2d - more then 8 !!!\n",slot);
		  }
          /*
          printf("slot=%d offset=%d\n",slot,offset);
		  */
        }
		/*
        printf("data: chan=%2d edge=%1d data=%8d (%8d)\n",
                    channel,edge,data,data-offset);
		*/
        tmpx = (float)(data-offset);
        idn=10;
        hf1_(&idn,&tmpx,&ww);
        idn=100+channel;
        hf1_(&idn,&tmpx,&ww);

      }
	  /*
      printf("\n");
	  */
      j=0;




      /* print condition */

      /* too few hits */
      for(slot=SLMIN; slot<=21; slot++)
        if(ndata0[slot]<2) j=1;

      /* too many hits */
      for(slot=SLMIN; slot<=21; slot++)
        if(ndata0[slot]>2) j=1;

      /* out of range */
      for(slot=SLMIN; slot<=21; slot++)
        for(i=0; i<ndata0[slot]; i++)
          if((data0[slot][0]<0x9380) || (data0[slot][0]>0x9b00)) j=1;

      /* not a minimum */
      for(slot=SLMIN; slot<=21; slot++)
        for(i=1; i<ndata0[slot]; i++)
          if(data0[slot][0]>data0[slot][i]) j=1;



      if(j)
	  {
      printf("Event no. %6d\n",iev);
      for(slot=SLMIN; slot<=21; slot++)
	  {
        printf("slot=%2d offset=",slot);
        for(i=0; i<ndata0[slot]; i++)
        {
          printf("0x%04x ",data0[slot][i]);
        }
        printf("\n");
	  }
	  }

    }





    ww = 1.0;
    nr = 0;
    igood = 0;
    /*if((ind=bosNlink(bcs_.iw,"TGTR",nr)) > 0)*/
    if((ind=bosNlink(bcs_.iw,"STN0",nr)) > 0)
    /*if((ind=bosNlink(bcs_.iw,"SCT ",1)) > 0)*/
    {
      int idx;
      short *cc;

      ncol = bcs_.iw[ind-6];
      nrow = bcs_.iw[ind-5];
	  /*
      printf("TGTL: ncol=%d nrow=%d\n",ncol,nrow);
      */

      cc = (short *)&bcs_.iw[ind];
      ref = 100000.0;
      for(j=0; j<nrow; j++)
      {
        int id;
        id = cc[0];

		/*printf("\n");*/
        if(id == 62)
		{
          /* search for min */
          if(ref > (float)cc[1])
          {
            ref = cc[1];
            /*printf("ref=%f\n",ref);*/
		  }
		}

        cc += 2;
	  }

      cc = (short *)&bcs_.iw[ind];
      offset = 0;
      for(j=0; j<nrow; j++)
      {
        int id;
        id = cc[0];
		/* ec */

        if( ((cc[0]&0xff)==34) && ( ((cc[0]>>8)&0xff)==1) )
          id = 1;
        else
		  {
          id = cc[0]&0xFF;
          if(id==1) id=100;
		  }

		/*				
printf("id=0x%04x tdc=0x%04x\n",cc[0],cc[1]);
		*/
			/* search for the corresponding hit from ECT */

        if((id > 0 && id < 16) || (id > 61 && id < 64))
        {
          tmpx = cc[1];
          idx = 100 + id;
		  /*
printf("   id=0x%04x, tmpx=%f -> fill hist %d\n",id,tmpx,idx);
		  */
          hf1_(&idx,&tmpx,&ww);

          tmpx = ref - tmpx;
          idx = 200 + id;
          hf1_(&idx,&tmpx,&ww);
        }             

        cc += 2;
      }
    }





    if(iret == -1 || iret > 0)
    {
      printf(" End-of-File flag, iret =%d\n",iret);
      goto a111111;
    }
    else if(iret < 0)
    {
      printf(" Error1, iret =%d\n",iret);
      goto a111111;
    }
    if(iret != 0)
    {
      printf(" Error2, iret =%d\n",iret);
    }
    bdrop_(bcs_.iw,"E",1);
    bgarb_(bcs_.iw);
  }
a111111:

  fparm_("CLOSE",5);

  /* closing HBOOK file */
  idn = 0;
  hrout_(&idn,&icycle," ",1);
  /*hprint_(&idn);*/
  hrend_("NTUPEL", 6);


for(nr=1;nr<=6;nr++)
  for(layer=1;layer<=6;layer++)
    for(strip=1;strip<=36;strip++)
    {
      printf("nr=%1d layer=%1d strip=%2d: good=%1d bad=%1d\n",
        nr,layer,strip,ecgood1[nr][layer][strip],ecbad1[nr][layer][strip]);
    }


  exit(0);

  /* end of v1190 test */





















  /* TOF test */

  nwpawc = NWPAWC;
  hlimit_(&nwpawc);
  lun = 11;
  lrec = LREC;
  hropen_(&lun,"NTUPEL",HBOOKfile,"N",&lrec,&istat,
     strlen("NTUPEL"),strlen(HBOOKfile),1);
  if(istat)
  {
    printf("\aError: cannot open RZ file %s for writing.\n", HBOOKfile);
    exit(0);
  }
  else
  {
    printf("RZ file >%s< opened for writing, istat = %d\n\n", HBOOKfile, istat);
  }
  nbins=48;
  x1 = 0.;
  x2 = 48.;
  ww = 0.;

  idn = 10;
  hbook1_(&idn,"TOF SECTOR 3 (ALL TRIGGER BITS)",&nbins,&x1,&x2,&ww,31);

  idn = 11;
  hbook1_(&idn,"TOF SECTOR 3 (TRIGGER BIT 3)",&nbins,&x1,&x2,&ww,28);

  idn = 101;
  hbook1_(&idn,"TOF SECTOR 1 (TRIGGER BIT 1)",&nbins,&x1,&x2,&ww,28);
  idn = 102;
  hbook1_(&idn,"TOF SECTOR 2 (TRIGGER BIT 2)",&nbins,&x1,&x2,&ww,28);
  idn = 103;
  hbook1_(&idn,"TOF SECTOR 3 (TRIGGER BIT 3)",&nbins,&x1,&x2,&ww,28);
  idn = 104;
  hbook1_(&idn,"TOF SECTOR 4 (TRIGGER BIT 4)",&nbins,&x1,&x2,&ww,28);
  idn = 105;
  hbook1_(&idn,"TOF SECTOR 5 (TRIGGER BIT 5)",&nbins,&x1,&x2,&ww,28);
  idn = 106;
  hbook1_(&idn,"TOF SECTOR 6 (TRIGGER BIT 6)",&nbins,&x1,&x2,&ww,28);

  nbins=6;
  x1 = 0.;
  x2 = 6.;

  idn = 21;
  hbook1_(&idn,"TGBI BANK",&nbins,&x1,&x2,&ww,4);

  /*
#define PRINT1 1
  */
  /*
#define PRINT2 1
  */

  sprintf(strr,"OPEN INPUT UNIT=1 FILE='%s' ",argv[1]);
  printf("fparm string: >%s<\n",strr);
  status = fparm_(strr,strlen(strr));
  for(iev=0; iev<100000; iev++)
  {
    int sec1, sec2, sec3, sec4, sec5, sec6;

    frbos_(bcs_.iw,&tmp1,"E",&iret,1);

    if((ind=bosNlink(bcs_.iw,"TGBI",0)) > 0)
    {
      int ncol,nrow;

      ncol = bcs_.iw[ind-6];
      nrow = bcs_.iw[ind-5];
      sec1 = (bcs_.iw[ind])&0x1;
      sec2 = ((bcs_.iw[ind])>>1)&0x1;
      sec3 = ((bcs_.iw[ind])>>2)&0x1;
      sec4 = ((bcs_.iw[ind])>>3)&0x1;
      sec5 = ((bcs_.iw[ind])>>4)&0x1;
      sec6 = ((bcs_.iw[ind])>>5)&0x1;
	  /*
      printf("tgbi=0x%08x sec3=%d\n",bcs_.iw[ind],sec3);
	  */
      idn=21;
      if(sec1==1) {tmpx = 0.5; hf1_(&idn,&tmpx,&ww);}
      if(sec2==1) {tmpx = 1.5; hf1_(&idn,&tmpx,&ww);}
      if(sec3==1) {tmpx = 2.5; hf1_(&idn,&tmpx,&ww);}
      if(sec4==1) {tmpx = 3.5; hf1_(&idn,&tmpx,&ww);}
      if(sec5==1) {tmpx = 4.5; hf1_(&idn,&tmpx,&ww);}
      if(sec6==1) {tmpx = 5.5; hf1_(&idn,&tmpx,&ww);}
      
      
	}
    else
    {
      printf("no TGBI\n");
      continue;
    }

    if((ind1=bosNlink(bcs_.iw,"SC  ",1)) > 0)
    {
      int ncol1,nrow1;
      int id,tdcl,tdcr,adcl,adcr;

      ncol1 = bcs_.iw[ind1-6];
      nrow1 = bcs_.iw[ind1-5];
      nw = nrow1;
      offset = 0;
      ww = 1.0;

      buf16 = (unsigned short *)&bcs_.iw[ind1];
      for(k=0; k<nrow1; k++)
      {
        id   = *buf16 ++;
        tdcl = *buf16 ++;
        adcl = *buf16 ++;
        tdcr = *buf16 ++;
        adcr = *buf16 ++;

		if(tdcl>0 && tdcl<4000 && tdcr>0 && tdcr<4000)
        {
          tmpx = ((float)(id))-0.5;
          if(sec1==1)
          {
            idn=101;
            hf1_(&idn,&tmpx,&ww);
          }
		}
      }
    }
    if((ind1=bosNlink(bcs_.iw,"SC  ",2)) > 0)
    {
      int ncol1,nrow1;
      int id,tdcl,tdcr,adcl,adcr;

      ncol1 = bcs_.iw[ind1-6];
      nrow1 = bcs_.iw[ind1-5];
      nw = nrow1;
      offset = 0;
      ww = 1.0;

      buf16 = (unsigned short *)&bcs_.iw[ind1];
      for(k=0; k<nrow1; k++)
      {
        id   = *buf16 ++;
        tdcl = *buf16 ++;
        adcl = *buf16 ++;
        tdcr = *buf16 ++;
        adcr = *buf16 ++;

		if(tdcl>0 && tdcl<4000 && tdcr>0 && tdcr<4000)
        {
          tmpx = ((float)(id))-0.5;
          if(sec2==1)
          {
            idn=102;
            hf1_(&idn,&tmpx,&ww);
          }
		}
      }
    }
    if((ind1=bosNlink(bcs_.iw,"SC  ",3)) > 0)
    {
      int ncol1,nrow1;
      int id,tdcl,tdcr,adcl,adcr;

      ncol1 = bcs_.iw[ind1-6];
      nrow1 = bcs_.iw[ind1-5];
      nw = nrow1;
      offset = 0;
      ww = 1.0;

      buf16 = (unsigned short *)&bcs_.iw[ind1];
      for(k=0; k<nrow1; k++)
      {
        id   = *buf16 ++;
        tdcl = *buf16 ++;
        adcl = *buf16 ++;
        tdcr = *buf16 ++;
        adcr = *buf16 ++;

		if(tdcl>0 && tdcl<4000 && tdcr>0 && tdcr<4000)
        {
          tmpx = ((float)(id))-0.5;
          idn=10;
          hf1_(&idn,&tmpx,&ww);
          if(sec3==1)
          {
            idn=11;
            hf1_(&idn,&tmpx,&ww);
          }

          if(sec3==1)
          {
            idn=103;
            hf1_(&idn,&tmpx,&ww);
          }
		}
      }
    }
    if((ind1=bosNlink(bcs_.iw,"SC  ",4)) > 0)
    {
      int ncol1,nrow1;
      int id,tdcl,tdcr,adcl,adcr;

      ncol1 = bcs_.iw[ind1-6];
      nrow1 = bcs_.iw[ind1-5];
      nw = nrow1;
      offset = 0;
      ww = 1.0;

      buf16 = (unsigned short *)&bcs_.iw[ind1];
      for(k=0; k<nrow1; k++)
      {
        id   = *buf16 ++;
        tdcl = *buf16 ++;
        adcl = *buf16 ++;
        tdcr = *buf16 ++;
        adcr = *buf16 ++;

		if(tdcl>0 && tdcl<4000 && tdcr>0 && tdcr<4000)
        {
          tmpx = ((float)(id))-0.5;
          if(sec4==1)
          {
            idn=104;
            hf1_(&idn,&tmpx,&ww);
          }
		}
      }
    }
    if((ind1=bosNlink(bcs_.iw,"SC  ",5)) > 0)
    {
      int ncol1,nrow1;
      int id,tdcl,tdcr,adcl,adcr;

      ncol1 = bcs_.iw[ind1-6];
      nrow1 = bcs_.iw[ind1-5];
      nw = nrow1;
      offset = 0;
      ww = 1.0;

      buf16 = (unsigned short *)&bcs_.iw[ind1];
      for(k=0; k<nrow1; k++)
      {
        id   = *buf16 ++;
        tdcl = *buf16 ++;
        adcl = *buf16 ++;
        tdcr = *buf16 ++;
        adcr = *buf16 ++;

		if(tdcl>0 && tdcl<4000 && tdcr>0 && tdcr<4000)
        {
          tmpx = ((float)(id))-0.5;
          if(sec5==1)
          {
            idn=105;
            hf1_(&idn,&tmpx,&ww);
          }
		}
      }
    }
    if((ind1=bosNlink(bcs_.iw,"SC  ",6)) > 0)
    {
      int ncol1,nrow1;
      int id,tdcl,tdcr,adcl,adcr;

      ncol1 = bcs_.iw[ind1-6];
      nrow1 = bcs_.iw[ind1-5];
      nw = nrow1;
      offset = 0;
      ww = 1.0;

      buf16 = (unsigned short *)&bcs_.iw[ind1];
      for(k=0; k<nrow1; k++)
      {
        id   = *buf16 ++;
        tdcl = *buf16 ++;
        adcl = *buf16 ++;
        tdcr = *buf16 ++;
        adcr = *buf16 ++;

		if(tdcl>0 && tdcl<4000 && tdcr>0 && tdcr<4000)
        {
          tmpx = ((float)(id))-0.5;
          if(sec6==1)
          {
            idn=106;
            hf1_(&idn,&tmpx,&ww);
          }
		}
      }
    }

    if(iret == -1 || iret > 0)
    {
      printf(" End-of-File flag, iret =%d\n",iret);
      goto a1113;
    }
    else if(iret < 0)
    {
      printf(" Error1, iret =%d\n",iret);
      goto a1113;
    }
    if(iret != 0)
    {
      printf(" Error2, iret =%d\n",iret);
    }
    bdrop_(bcs_.iw,"E",1);
    bgarb_(bcs_.iw);
  }
a1113:

  fparm_("CLOSE",5);

  /* closing HBOOK file */
  idn = 0;
  hrout_(&idn,&icycle," ",1);
  /*hprint_(&idn);*/
  hrend_("NTUPEL", 6);



  exit(0);

  /* end of TOF stuff */






goto skip_croctest3;


  /* croctest3 stuff */

  nwpawc = NWPAWC;
  hlimit_(&nwpawc);
  lun = 11;
  lrec = LREC;
  hropen_(&lun,"NTUPEL",HBOOKfile,"N",&lrec,&istat,
     strlen("NTUPEL"),strlen(HBOOKfile),1);
  if(istat)
  {
    printf("\aError: cannot open RZ file %s for writing.\n", HBOOKfile);
    exit(0);
  }
  else
  {
    printf("RZ file >%s< opened for writing, istat = %d\n\n", HBOOKfile, istat);
  }
  nbins=100;
  x1 = 500.;
  x2 = 600.;
  ww = 0.;

  idn = 10;
  hbook1_(&idn,"all",&nbins,&x1,&x2,&ww,4);
  idn = 100;
  hbook1_(&idn,"ch00",&nbins,&x1,&x2,&ww,4);
  idn = 101;
  hbook1_(&idn,"ch01",&nbins,&x1,&x2,&ww,4);
  idn = 102;
  hbook1_(&idn,"ch02",&nbins,&x1,&x2,&ww,4);
  idn = 103;
  hbook1_(&idn,"ch03",&nbins,&x1,&x2,&ww,4);
  idn = 104;
  hbook1_(&idn,"ch04",&nbins,&x1,&x2,&ww,4);
  idn = 105;
  hbook1_(&idn,"ch05",&nbins,&x1,&x2,&ww,4);
  idn = 106;
  hbook1_(&idn,"ch06",&nbins,&x1,&x2,&ww,4);
  idn = 107;
  hbook1_(&idn,"ch07",&nbins,&x1,&x2,&ww,4);
  idn = 108;
  hbook1_(&idn,"ch08",&nbins,&x1,&x2,&ww,4);
  idn = 109;
  hbook1_(&idn,"ch09",&nbins,&x1,&x2,&ww,4);
  idn = 110;
  hbook1_(&idn,"ch10",&nbins,&x1,&x2,&ww,4);
  idn = 111;
  hbook1_(&idn,"ch11",&nbins,&x1,&x2,&ww,4);
  idn = 112;
  hbook1_(&idn,"ch12",&nbins,&x1,&x2,&ww,4);
  idn = 113;
  hbook1_(&idn,"ch13",&nbins,&x1,&x2,&ww,4);
  idn = 114;
  hbook1_(&idn,"ch14",&nbins,&x1,&x2,&ww,4);
  idn = 115;
  hbook1_(&idn,"ch15",&nbins,&x1,&x2,&ww,4);

  /*
#define PRINT1 1
  */
  /*
#define PRINT2 1
  */

  sprintf(strr,"OPEN INPUT UNIT=1 FILE='%s' ",argv[1]);
  printf("fparm string: >%s<\n",strr);
  status = fparm_(strr,strlen(strr));
  for(iev=0; iev<2000000; iev++)
  {
    frbos_(bcs_.iw,&tmp1,"E",&iret,1);

    if((ind1=bosNlink(bcs_.iw,"RC22",0)) > 0)
    {
      unsigned int *tlv1;
      int slot,channel,edge,data,count,ncol1,nrow1;
      int oldslot = 100;

      ncol1 = bcs_.iw[ind1-6];
      nrow1 = bcs_.iw[ind1-5];
      nw = nrow1;
      offset = 0;
	  /*
      printf("ncol=%d nrow=%d nw=%d\n",ncol1,nrow1,nw);
	  */
      ww = 1.0;

      tlv1 = (unsigned int *)&bcs_.iw[ind1];
      for(k=0; k<nrow1; k++)
      {
        slot = (tlv1[k]>>27)&0x1f;
        channel = (tlv1[k]>>19)&0x1f;
        edge = (tlv1[k]>>26)&0x1;
        data = tlv1[k]&0x7ffff;
        if(channel==0)
        {
          offset = data;
          /*
          printf("offset=%d\n",offset);
		  */
        }
		/*
        printf("data: chan=%2d edge=%1d data=%8d (%8d)\n",
                    channel,edge,data,data-offset);
		*/
        tmpx = (float)(data-offset);
        idn=10;
        hf1_(&idn,&tmpx,&ww);
        idn=100+channel;
        hf1_(&idn,&tmpx,&ww);

      }
	  /*
      printf("\n");
	  */
    }

    if(iret == -1 || iret > 0)
    {
      printf(" End-of-File flag, iret =%d\n",iret);
      goto a1112;
    }
    else if(iret < 0)
    {
      printf(" Error1, iret =%d\n",iret);
      goto a1112;
    }
    if(iret != 0)
    {
      printf(" Error2, iret =%d\n",iret);
    }
    bdrop_(bcs_.iw,"E",1);
    bgarb_(bcs_.iw);
  }
a1112:

  fparm_("CLOSE",5);

  /* closing HBOOK file */
  idn = 0;
  hrout_(&idn,&icycle," ",1);
  /*hprint_(&idn);*/
  hrend_("NTUPEL", 6);



  exit(0);

  /* end of croctest3 stuff */





skip_croctest3:






goto skip_cc2;


  /* CC2 stuff */

  nwpawc = NWPAWC;
  hlimit_(&nwpawc);
  lun = 11;
  lrec = LREC;
  hropen_(&lun,"NTUPEL",HBOOKfile,"N",&lrec,&istat,
     strlen("NTUPEL"),strlen(HBOOKfile),1);
  if(istat)
  {
    printf("\aError: cannot open RZ file %s for writing.\n", HBOOKfile);
    exit(0);
  }
  else
  {
    printf("RZ file >%s< opened for writing, istat = %d\n\n", HBOOKfile, istat);
  }
  nbins=800;
  x1 = -400.;
  x2 =  400.;
  ww = 0.;

  idn = 10;
  hbook1_(&idn,"all",&nbins,&x1,&x2,&ww,4);
  idn = 100;
  hbook1_(&idn,"ch00",&nbins,&x1,&x2,&ww,4);
  idn = 101;
  hbook1_(&idn,"ch01",&nbins,&x1,&x2,&ww,4);
  idn = 102;
  hbook1_(&idn,"ch02",&nbins,&x1,&x2,&ww,4);
  idn = 103;
  hbook1_(&idn,"ch03",&nbins,&x1,&x2,&ww,4);
  idn = 104;
  hbook1_(&idn,"ch04",&nbins,&x1,&x2,&ww,4);
  idn = 105;
  hbook1_(&idn,"ch05",&nbins,&x1,&x2,&ww,4);
  idn = 106;
  hbook1_(&idn,"ch06",&nbins,&x1,&x2,&ww,4);
  idn = 107;
  hbook1_(&idn,"ch07",&nbins,&x1,&x2,&ww,4);
  idn = 108;
  hbook1_(&idn,"ch08",&nbins,&x1,&x2,&ww,4);
  idn = 109;
  hbook1_(&idn,"ch09",&nbins,&x1,&x2,&ww,4);
  idn = 110;
  hbook1_(&idn,"ch10",&nbins,&x1,&x2,&ww,4);
  idn = 111;
  hbook1_(&idn,"ch11",&nbins,&x1,&x2,&ww,4);
  idn = 112;
  hbook1_(&idn,"ch12",&nbins,&x1,&x2,&ww,4);
  idn = 113;
  hbook1_(&idn,"ch13",&nbins,&x1,&x2,&ww,4);
  idn = 114;
  hbook1_(&idn,"ch14",&nbins,&x1,&x2,&ww,4);
  idn = 115;
  hbook1_(&idn,"ch15",&nbins,&x1,&x2,&ww,4);

  /*
#define PRINT1 1
  */
  /*
#define PRINT2 1
  */

  sprintf(strr,"OPEN INPUT UNIT=1 FILE='%s' ",argv[1]);
  printf("fparm string: >%s<\n",strr);
  status = fparm_(strr,strlen(strr));
  for(iev=0; iev<2000000; iev++)
  {
    frbos_(bcs_.iw,&tmp1,"E",&iret,1);

    igood = 0;
    if((ind=bosNlink(bcs_.iw,"CC  ",6)) > 0)
    {
      unsigned short *cc;

      ncol = bcs_.iw[ind-6];
      nrow = bcs_.iw[ind-5];
	  /*
      printf("CC: ncol=%d nrow=%d\n",ncol,nrow);
      */

      cc = (unsigned short *)&bcs_.iw[ind];
      for(k=0; k<nrow; k++)
      {
        int id;
        id = cc[0];
        if(id==25||id==27||id==29||id==31||id==33||id==35||id==2||id==4||
           id==6||id==8||id==10||id==12||id==14||id==16||id==18||id==20)
  	    {
          if(cc[1]<4095)
          {
            igood ++;
          }
        }
        cc+=3;
      }
      if(igood == 0) continue;
      if((ind1=bosNlink(bcs_.iw,"RC21",0)) <= 0) continue;
	  
#if defined(PRINT1) || defined(PRINT2)
      printf("\nEvent no. %d (%d)\n",iev,igood);
#endif	  
      offset = 0;
      cc = (unsigned short *)&bcs_.iw[ind];
      for(j=0; j<nrow; j++)
      {
        int id;
        id = cc[0];
        if(id==25||id==27||id==29||id==31||id==33||id==35||id==2||id==4||
           id==6||id==8||id==10||id==12||id==14||id==16||id==18||id==20)
  	    {
          if(cc[1]<4095)
          {
#ifdef PRINT1
            printf("  CC: id=%3d tdc=%6d adc=%6d\n",cc[0],cc[1],cc[2]);
#endif
#ifdef PRINT2
            printf("id=%3d tdc=%6d -> ",cc[0],cc[1]);
#endif

			/* search for the corresponding hit from CC2 */
            {
              unsigned int *tlv1;
              int slot,channel,edge,data,count,ncol1,nrow1;
              int oldslot = 100, nhits = 0;

              ncol1 = bcs_.iw[ind1-6];
              nrow1 = bcs_.iw[ind1-5];
              nw = nrow1;
#ifdef PRINT1
              printf("ncol=%d nrow=%d nw=%d\n",ncol1,nrow1,nw);
#endif
              ww = 1.0;

              tlv1 = (unsigned int *)&bcs_.iw[ind1];
              for(k=0; k<nrow1; k++)
              {
                slot = (tlv1[k]>>27)&0x1f;
                channel = (tlv1[k]>>19)&0x1f;
                edge = (tlv1[k]>>26)&0x1;
                data = tlv1[k]&0x7ffff;
				/*
if(channel<16) printf("==================================== ch=%d\n",channel);
				*/
                if(channel==2)
                {
                  offset = data;
                  /*printf("offset=%d\n",offset);*/
                }

                if(cc[0]==lookup[channel])
                {
                  nhits ++;
#ifdef PRINT1
                  printf("data: chan=%2d edge=%1d data=%8d (%8d)\n",
                    channel,edge,data,offset-data);
#endif
#ifdef PRINT2
                  printf("%8d",(data-offset)*2+2400);
#endif

                  if(nhits==1)
				  {
                    /*tmpx = (data-offset)*1.965+2400 - cc[1];*/ /*4.4534*/
                    /*tmpx = (data-offset)*1.966+2400 - cc[1];*/ /*4.5146*/
                    tmpx = (data-offset)*1.964+2400 - cc[1]; /*4.8018*/
                    idn=10;
                    hf1_(&idn,&tmpx,&ww);
                    idn=100+channel-16;
                    hf1_(&idn,&tmpx,&ww);
				  }

                }

              }
#ifdef PRINT2
              printf("\n");
#endif
	        }
          }
        }

        cc += 3;

      }
    }



    if(iret == -1 || iret > 0)
    {
      printf(" End-of-File flag, iret =%d\n",iret);
      goto a111;
    }
    else if(iret < 0)
    {
      printf(" Error1, iret =%d\n",iret);
      goto a111;
    }
    if(iret != 0)
    {
      printf(" Error2, iret =%d\n",iret);
    }
    bdrop_(bcs_.iw,"E",1);
    bgarb_(bcs_.iw);
  }
a111:

  fparm_("CLOSE",5);

  /* closing HBOOK file */
  idn = 0;
  hrout_(&idn,&icycle," ",1);
  /*hprint_(&idn);*/
  hrend_("NTUPEL", 6);



  exit(0);

  /* end of CC2 stuff */

skip_cc2:

goto a321;

/**************/
/* TLV1 stuff */

  nwpawc = NWPAWC;
  hlimit_(&nwpawc);
  lun = 11;
  lrec = LREC;
  hropen_(&lun,"NTUPEL",HBOOKfile,"N",&lrec,&istat,
     strlen("NTUPEL"),strlen(HBOOKfile),1);
  if(istat)
  {
    printf("\aError: cannot open RZ file %s for writing.\n", HBOOKfile);
    exit(0);
  }
  else
  {
    printf("RZ file >%s< opened for writing, istat = %d\n\n", HBOOKfile, istat);
  }
  nbins=400;
  x1 = 1.;
  x2 = 16000.;
  nbins1=400;
  y1 = 1.;
  y2 = 400.;
  ww = 0.;

  idn = 10;
  hbook1_(&idn,"MOR",&nbins,&x1,&x2,&ww,4);
  idn = 20;
  hbook1_(&idn,"ASYNC",&nbins,&x1,&x2,&ww,4);
  idn = 30;
  hbook1_(&idn,"BIT3",&nbins,&x1,&x2,&ww,4);
  idn = 40;
  hbook1_(&idn,"BIT3-ASYNC",&nbins,&x1,&x2,&ww,4);
  idn = 100;
  hbook2_(&idn,"YvxX",&nbins,&x1,&x2,&nbins1,&y1,&y2,&ww,4);




  sprintf(strr,"OPEN INPUT UNIT=1 FILE='%s' ",argv[1]);
  printf("fparm string: >%s<\n",strr);
  status = fparm_(strr,strlen(strr));
  for(i=0; i<100000; i++)
  {
    frbos_(bcs_.iw,&tmp1,"E",&iret,1);
    for(j=0; j<=0; j++)
	{
      if((ind=bosNlink(bcs_.iw,"RC04",j)) > 0)
      {
        unsigned int *tlv1;
        int slot,channel,edge,data,count;
        int oldslot = 100, oldedge = 0;

        ncol = bcs_.iw[ind-6];
        nrow = bcs_.iw[ind-5];
        nw = nrow;
		/*
        printf("ncol=%d nrow=%d nw=%d\n",ncol,nrow,nw);
		*/
        ww = 1.0;

        tlv1 = (unsigned int *)&bcs_.iw[ind];
        for(k=0; k<nrow; k++)
        {
          slot = (tlv1[k]>>27)&0x1f;
          channel = (tlv1[k]>>17)&0x7f;
          edge = (tlv1[k]>>16)&0x1;
          data = tlv1[k]&0xffff;
          count = tlv1[k]&0x7ff;
          if(slot != oldslot)
          {
            oldslot = slot;
			/*
            printf("head: slot=%2d count=%d\n",
			              slot,count);
			*/
		  }
          else
          {
			/*
            printf("data: slot=%2d channnel=%2d edge=%1d data=%6d\n",
              slot,channel,edge,data);
			*/
            idn=10;
            tmpx = data;
            hf1_(&idn,&tmpx,&ww);

            if(oldedge==0 && edge==1)
            {
              idn=20;
              tmpx = data;
              hf1_(&idn,&tmpx,&ww);
            }
            oldedge = edge;
          }

        }


	  }
    }

    if(iret == -1 || iret > 0)
    {
      printf(" End-of-File flag, iret =%d\n",iret);
      goto a1111;
    }
    else if(iret < 0)
    {
      printf(" Error1, iret =%d\n",iret);
      goto a1111;
    }
    if(iret != 0)
    {
      printf(" Error2, iret =%d\n",iret);
    }
    bdrop_(bcs_.iw,"E",1);
    bgarb_(bcs_.iw);
  }
a1111:

  fparm_("CLOSE",5);

  /* closing HBOOK file */
  idn = 0;
  hrout_(&idn,&icycle," ",1);
  /*hprint_(&idn);*/
  hrend_("NTUPEL", 6);



  exit(0);

/* end of TLV1 stuff */
/*********************/

a321:

goto a123;


 /* HBOOK initialization */

  nwpawc = NWPAWC;
  hlimit_(&nwpawc);

  /* Opening an RZ file and booking an Ntuple. */

  lun = 11;
  lrec = LREC;
  hropen_(&lun,"NTUPEL",HBOOKfile,"N",&lrec,&istat,
     strlen("NTUPEL"),strlen(HBOOKfile),1);
  if(istat)
  {
    printf("\aError: cannot open RZ file %s for writing.\n", HBOOKfile);
    exit(0);
  }
  else
  {
    printf("RZ file >%s< opened for writing, istat = %d\n\n", HBOOKfile, istat);
  }

  /* book histos */

/*
  hbookn_(&idn,"ITEPtest",&N_size,"NTUPEL",&nprime,cvar,strlen("ITEPtest"),6,NTAG);
*/
  nbins=400;
  x1 = 1.;
  x2 = 5000.;
  nbins1=400;
  y1 = 1.;
  y2 = 5000;
  ww = 0.;

  idn = 10;
  hbook1_(&idn,"Xras",&nbins,&x1,&x2,&ww,4);
  idn = 20;
  hbook1_(&idn,"Yras",&nbins,&x1,&x2,&ww,4);
  idn = 100;
  hbook2_(&idn,"YvxX",&nbins,&x1,&x2,&nbins1,&y1,&y2,&ww,4);

  nbins=400;
  x1 = -20.0;
  x2 = 20.0;
  idn = 11;
  hbook1_(&idn,"X",&nbins,&x1,&x2,&ww,1);
  for(i=1; i<=6; i++)
  {
    idn = 11 + 1000*i;
    hbook1_(&idn,"X",&nbins,&x1,&x2,&ww,1);
  }

  idn = 12;
  nbins=400;
  x1 = -20.0;
  x2 = 20.0;
  hbook1_(&idn,"Y",&nbins,&x1,&x2,&ww,1);
  for(i=1; i<=6; i++)
  {
    idn = 12 + 1000*i;
    hbook1_(&idn,"Y",&nbins,&x1,&x2,&ww,1);
  }

  idn = 13;
  nbins=400;
  x1 = -75.0;
  x2 = -35.0;
  hbook1_(&idn,"Z",&nbins,&x1,&x2,&ww,1);
  for(i=1; i<=6; i++)
  {
    idn = 13 + 1000*i;
    hbook1_(&idn,"Z",&nbins,&x1,&x2,&ww,1);
  }

  nbins=200;
  x1 = -1.0;
  x2 = 1.0;
  idn = 14;
  hbook1_(&idn,"Cx",&nbins,&x1,&x2,&ww,2);

  idn = 15;
  nbins=200;
  x1 = -1.0;
  x2 = 1.0;
  hbook1_(&idn,"Cy",&nbins,&x1,&x2,&ww,2);

  idn = 16;
  nbins=200;
  x1 = -1.0;
  x2 = 1.0;
  hbook1_(&idn,"Cz",&nbins,&x1,&x2,&ww,2);

  idn = 17;
  nbins=600;
  x1 = 0.0;
  x2 = 6.0;
  hbook1_(&idn,"MOM",&nbins,&x1,&x2,&ww,3);

  idn = 18;
  nbins=100;
  x1 = -180.;
  x2 = 180.;
  hbook1_(&idn,"PHI",&nbins,&x1,&x2,&ww,3);

  idn = 19;
  nbins=100;
  x1 = 0.;
  x2 = 90.;
  hbook1_(&idn,"THETA",&nbins,&x1,&x2,&ww,5);

  idn = 21;
  nbins=100;
  x1 = 0.;
  x2 = 4.;
  hbook1_(&idn,"Q",&nbins,&x1,&x2,&ww,1);

  idn = 22;
  nbins=100;
  x1 = 0.;
  x2 = 4.;
  hbook1_(&idn,"W",&nbins,&x1,&x2,&ww,1);

  idn = 23;
  nbins=100;
  x1 = 0.;
  x2 = 50.;
  hbook1_(&idn,"HI2",&nbins,&x1,&x2,&ww,3);

  /* looking for particular bank */

  fparm_(str3, strlen(str3));
  for(i=5; i<1000000; i++)
  {
    frbos_(bcs_.iw,&tmp1,"E",&iret,1);
    for(j=1; j<=6; j++)
	{
      if((ind=bosNlink(bcs_.iw,"TDPL",j)) > 0)
      {
        int itrk;
        float *xyz;
        ncol = bcs_.iw[ind-6];
        nrow = bcs_.iw[ind-5];
        /*printf("ncol=%d nrow=%d nw=%d\n",ncol,nrow,nw);*/

        itrk = bcs_.iw[ind];
        /*printf("sec=%d itrk=%d\n",j,itrk);*/

        /*if(itrk==101)*/
        {
          xyz = (float *)&bcs_.iw[ind+1];
          ww = 1.0;
          idn=11+1000*j;
          hf1_(&idn,&xyz[0],&ww);
          idn=12+1000*j;
          hf1_(&idn,&xyz[1],&ww);
          idn=13+1000*j;
          hf1_(&idn,&xyz[2],&ww);
        }
	  }
    }

    if((ind=bosNlink(bcs_.iw,"DCPB",0)) > 0)
    {
      float hi2, *ev;
      ev = (float *)&bcs_.iw[ind];

      hi2 = ev[11];
      ww = 1.;
      idn=23;
      hf1_(&idn,&hi2,&ww);
    }

    if((ind=bosNlink(bcs_.iw,"EVNT",0)) > 0)
    {
      float p,cx,cy,cz,x,y,z,th,ph,q,w,Ebeam,Me,Mp,Ee;
      float *ev;
      int *iev;
      ev = (float *)&bcs_.iw[ind];
      iev = (int *)ev;
      nw = bcs_.iw[ind-1];
      ncol = bcs_.iw[ind-6];
      nrow = bcs_.iw[ind-5];
/*
      printf("ncol=%d nrow=%d nw=%d\n",ncol,nrow,nw);
      for(l=0; l<nrow; l++)
      {
        printf("ev=%d %f %f %d %f %f %f %f %f\n",
         iev[0],ev[1],ev[2],iev[3],ev[4],ev[5],ev[6],ev[7],ev[8],ev[9],ev[10]);
        ev += ncol;
        iev = (int *)ev;
      }
*/

      p = ev[1];
      cx= ev[5];
      cy= ev[6];
      cz= ev[7];
      x = ev[8];
      y = ev[9];
      z = ev[10];
      th = acos(cz)*RADDEG;
      ph = atan2(cy,cx)*RADDEG;

      Ebeam=5.74;
      Me=0.000511;
      Mp=0.938272;
      Ee = sqrt(p*p + Me*Me);

      q = 2.*Ebeam*Ee*(1.-cz);
      w = 2.*Mp*(Ebeam - Ee) + Mp*Mp - q;
      if(w > 0.) w = sqrt(w);
      else w=0.;
/*
      printf("mom=%5.3f Cx=%6.3f Cy=%6.3f Cz=%6.3f x=%7.3f y=%7.3f z=%7.3f\n",
         p,cx,cy,cz,x,y,z);
*/

      ww = 1.;

      idn=11;
      hf1_(&idn,&x,&ww);
      idn=12;
      hf1_(&idn,&y,&ww);
      idn=13;
      hf1_(&idn,&z,&ww);
      idn=14;
      hf1_(&idn,&cx,&ww);
      idn=15;
      hf1_(&idn,&cy,&ww);
      idn=16;
      hf1_(&idn,&cz,&ww);
      idn=17;
      hf1_(&idn,&p,&ww);
      idn=18;
      hf1_(&idn,&ph,&ww);
      idn=19;
      hf1_(&idn,&th,&ww);
      idn=21;
      hf1_(&idn,&q,&ww);
      idn=22;
      hf1_(&idn,&w,&ww);

	}

    if((ind=bosNlink(bcs_.iw,"RC11",0)) > 0)
    {
      /*
      MTDC *fb;
      fb = (MTDC *)&bcs_.iw[ind];
      nw = bcs_.iw[ind-1];
      printf("\n");
      for(l=0; l<nw; l++)
      {
        printf("slot=%2d channel=%2d data=%6d (count=%6d)\n",fb->slot,fb->channel,fb->data,fb->data&0x7ff);
        fb++;
      }
      */

      ADC *fb;
      fb = (ADC *)&bcs_.iw[ind];
      nw = bcs_.iw[ind-1];
      fb++; /* skip adc header */
      ww = 1.;
      tmpx=fb->data;
      tmpy=(fb+1)->data;
/*printf("%f %f\n",tmpx,tmpy);*/
      if(tmpx>0.1 && tmpy>0.1)
	  {
        idn=10;
        hf1_(&idn,&tmpx,&ww);
        idn=20;
        hf1_(&idn,&tmpy,&ww);
        idn=100;
        hf2_(&idn,&tmpx,&tmpy,&ww);
	  }
/*
      printf("\n");
      for(l=0; l<nw; l++)
      {
if(fb->slot==15 && fb->channel < 2)
        printf("slot=%2d channel=%2d data=%6d (count=%6d)\n",fb->slot,fb->channel,fb->data,fb->data&0x7f);
        fb++;
      }
*/
    }


/*    
    for(k=0; k<=0; k++)
    {
      if((ind=bosNlink(bcs_.iw,"FBPM",k)) > 0)
      {
        bosNprint(bcs_.iw,"FBPM",k);
      }
    }
  */  

    if(iret == -1 || iret > 0)
    {
      printf(" End-of-File flag, iret =%d\n",iret);
      goto a11;
    }
    else if(iret < 0)
    {
      printf(" Error1, iret =%d\n",iret);
      goto a11;
    }
    if(iret != 0)
    {
      printf(" Error2, iret =%d\n",iret);
    }
    bdrop_(bcs_.iw,"E",1);
    bgarb_(bcs_.iw);
  }
a11:

  fparm_("CLOSE",5);

  /* closing HBOOK file */
  idn = 0;
  hrout_(&idn,&icycle," ",1);
  /*hprint_(&idn);*/
  hrend_("NTUPEL", 6);


a123:


/* looking for +SYN/SYNC banks

  fparm_(str3, strlen(str3));
  for(i=0;i<10000000;i++)
  {
    frbos_(bcs_.iw,&tmp1,"E",&iret,1);

    for(id=0; id<32; id++)
    {
      syn[id] = 0;
      if(bosNlink(bcs_.iw,"+SYN",id) || bosNlink(bcs_.iw,"SYNC",id))
      {
        syn[id]=1;
      }
    }

    if(syn[1]||syn[2]||syn[3]||syn[4]||syn[5]||syn[6]||syn[7]||syn[8]||syn[9]||syn[10]||syn[11]||
       syn[12]||syn[13]||syn[14]||syn[15]||syn[16]||syn[17]||syn[18]||syn[19]||syn[23]||syn[25]||syn[30])
    {
      if(syn[1]*syn[2]*syn[3]*syn[4]*syn[5]*syn[6]*syn[7]*syn[8]*syn[9]*syn[10]*syn[11]*
       syn[12]*syn[13]*syn[14]*syn[15]*syn[16]*syn[17]*syn[18]*syn[19]*syn[23]*syn[25]*syn[30])
      {
        printf("good ...\n");
      }
      else
      { 
        printf("bad: 1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 23 25 30\n");
        printf("   %3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d\n",
           syn[1],syn[2],syn[3],syn[4],syn[5],syn[6],syn[7],syn[8],syn[9],syn[10],syn[11],
           syn[12],syn[13],syn[14],syn[15],syn[16],syn[17],syn[18],syn[19],syn[23],syn[25],syn[30]);
        printf("event number %d\n",bcs_.iw[bosNlink(bcs_.iw,"HEAD",0)+2]);

      }
    }

    if(iret == -1 || iret > 0)
    {
      printf(" End-of-File flag, iret =%d\n",iret);
      goto a10;
    }
    else if(iret < 0)
    {
      printf(" Error1, iret =%d\n",iret);
      goto a10;
    }
    if(iret != 0)
    {
      printf(" Error2, iret =%d\n",iret);
    }
    bdrop_(bcs_.iw,"E",1);
    bgarb_(bcs_.iw);
  }
a10:

  fparm_("CLOSE",5);
*/


/* extract events in according with table

  fparm_(str6, strlen(str6));
  fparm_(str4, strlen(str4));

  iev = 0;
  for(i=0;i<1;i++)
  {
    int ind;

    frbos_(bcs_.iw,&tmp1,"E",&iret,1);
    if(iret == -1 || iret > 0)
    {
      printf(" End-of-File flag, iret =%d\n",iret);
      goto a10;
    }
    else if(iret < 0)
    {
      printf(" Error1, iret =%d\n",iret);
      goto a10;
    }



  {
    int tpeds[30];
    int nentry=10;
    add_bank("CALL",0,"(I,I,F)",3,nentry,nentry*3,(int*)tpeds);
    fwbos_(bcs_.iw,&tmp2,"E",&iret,1);
  }


    if(iret != 0)
    {
      printf(" Error2, iret =%d\n",iret);
    }
    bdrop_(bcs_.iw,"E",1);
    bgarb_(bcs_.iw);
  }
a10:
  fwbos_(bcs_.iw,&tmp2,"0",&iret,1);

  fparm_("CLOSE",5);
*/


/* splitmb test
  fparm_(str1, strlen(str1));
  fparm_(str2, strlen(str2));
  for(;;)
  {
    frbos_(bcs_.iw,&tmp1,"E",&iret,1);
    if(iret == -1 || iret > 0)
    {
      printf(" End-of-File flag, iret =%d\n",iret);
      goto a10;
    }
    else if(iret < 0)
    {
      printf(" Error1, iret =%d\n",iret);
      goto a10;
    }
    fwbos_(bcs_.iw,&tmp2,"E",&iret,1);
    if(iret != 0)
    {
      printf(" Error2, iret =%d\n",iret);
    }
    bdrop_(bcs_.iw,"E",1);
    bgarb_(bcs_.iw);
  }
a10:
  fwbos_(bcs_.iw,&tmp2,"0",&iret,1);
  fparm_("CLOSE",5);
*/



/*
  status = bosOpen("FPACK5.DAT","r",&handle);
  printf("after bosOpen(FPACK5.DAT) status=%d\n",status);

  status = bosOpen("FPACK6.DAT","w",&handle1);
  printf("after bosOpen(FPACK6.DAT) status=%d\n",status);
  if(status) exit(1);

  while((status=bosRead(handle,bcs_.iw,"E")) == 0)
  {
    ind = bosNdrop(bcs_.iw,"+KYE",0);

    status1=bosWrite(handle1,bcs_.iw,"E");
    if((i=bosLdrop(bcs_.iw,"E")) < 0)
    {
      printf("Error in bosLdrop, number %d\n",i);
      exit(1);
    }
    if((i=bosNgarbage(bcs_.iw)) < 0)
    {
      printf("Error in bosNgarbage, number %d\n",i);
      exit(1);
    }
  }
  status = bosWrite(handle1,bcs_.iw,"0"); 
  status = bosClose(handle);
  status = bosClose(handle1);
*/

  exit(0);
}








