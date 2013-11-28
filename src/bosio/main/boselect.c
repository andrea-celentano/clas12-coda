/* boselect.c - select events in according to table */
 
#include <stdio.h>
 
#include "bos.h"
#include "bosio.h"

#define NBCS 700000
#include "bcs.h"

main()
{
 
int nr,nl,ncol,nrow,i,l,l1,l2,ichan,nn,iev,ind,ind1,status,status1,handle,handle1,k,hel,str,strob,helicity;
int tmp1 = 1, tmp2 = 2, iret, bit1, bit2;
float *bcsfl, rndm_();

static char *str1 = "OPEN INPUT UNIT=1 FILE=\"/work/clas/disk1/boiarino/clas_018318_less_than_10_close_tracks_new.A00\" RECL=36000";
static char *str2 = "OPEN OUTPUT UNIT=2 FILE=\"pair.A00\" RECL=36000 WRITE SEQ NEW BINARY";

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

/* run 9183
int nev[] = {1358,1573,2027,2166,2838,4242,4350,6143,6645,7212,
      7608,9227,9279,10626,12002,12437,13116,14429,14814,16829,
      17093,17430,18568,18569,20426,20784,21338,21495,21792,
      24377,24466,24941,27990,28409,29950};
*/

/* run 16971
C====  events /sector/gap/SC/n_1/n_2/n_3/n_4/n_5/n_6/n_sl/e
*/

int nev[][12] = {
         7651,      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        15003,      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        15375,      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        16598,      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        20158,      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        21395,      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        27651,      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        27818,      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        27996,      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

/*
int nev[][12] = {
         849,      2,  7, 4,  9, 10, 12, 13, 13, 12,  6, 0,
         987,      1,  5, 4,  9, 10, 13, 11, 12, 12,  6, 1,
        1539,      2,  4, 4,  8, 10, 11, 13, 13, 13,  6, 1,
        1977,      1,  4, 4, 10, 10, 13, 10, 12, 12,  5, 0,
        2230,      2,  0, 1,  9,  9, 17, 16, 12, 14,  4, 0,
        3188,      2,  7, 4, 10,  9,  9, 12, 13, 12,  6, 1,
        4959,      6,  2, 4,  8, 12, 12, 13, 15, 11,  6, 1,
        6334,      4,  0, 4,  5, 14, 12, 13, 16, 13,  4, 1,
        7595,      6,  2, 4,  9, 14, 22, 16, 13, 14,  4, 1,
        9074,      4,  7, 3, 12, 14, 11, 12, 12, 11,  6, 1,

        9935,      5,  3, 4,  9, 14, 12,  7, 12, 12,  5, 1,
       10111,      5,  0, 4,  9, 12, 12, 11, 13, 13,  5, 1,
       10839,      3,  2, 1,  8, 16, 11, 12, 13, 14, -6, 1,
       10845,      4,  7, 4,  9, 12, 11, 11, 13, 12,  6, 1,
       12040,      3,  7, 1, 10, 13, 13, 14, 15, 15,  6, 1,
       12263,      3,  4, 4,  8, 13, 13, 11, 14, 14,  6, 0,
       12280,      2,  5, 4,  8, 12, 12, 17, 10, 12,  6, 1,
       12325,      5,  3, 3,  9, 15, 14, 12,  9, 12,  5, 1,
       12352,      6,  4, 4, 11, 12, 13, 12, 13, 11,  6, 1,
       12404,      3,  3, 4,  8, 14, 14, 14, 14, 14,  6, 0,

       12837,      2,  4, 3,  9, 18, 12, 11, 14, 15,  5, 1,
       13192,      3,  4, 4,  8, 13,  9, 11, 13, 11,  5, 1,
       13874,      4,  2, 4,  9, 14, 15, 15, 14, 14,  6, 1,
       14059,      2,  6, 4,  9, 15, 12, 13, 12, 13,  6, 1,
       14482,      5,  4, 2, 10, 12, 10, 10, 11, 10,  5, 1,
       15396,      3,  7, 1,  6, 12, 12, 12, 14, 14,  6, 1,
       15649,      3,  6, 4,  6, 12, 14, 14, 17, 14, -6, 1,
       16116,      2,  7, 4,  9, 13, 10, 11, 10, 12,  5, 0,
       17740,      6,  0, 4,  6, 11, 10, 12, 13, 13,  3, 0,
       19177,      5,  6, 4, 10, 14, 12, 12, 14, 14,  6, 0,

       22159,      4,  2, 1,  9, 15, 13, 13, 12, 12,  5, 0,
       22794,      3,  3, 4,  8, 12, 12, 12, 13,  9,  6, 1,
       26261,      2,  7, 4,  8, 12, 11, 12, 12, 13,  6, 0,
       26483,      6,  4, 4, 15, 13, 12, 11, 12, 12,  5, 1,
       27133,      1,  4, 1,  5, 13,  8, 14, 15,  7,  4, 0,
       27569,      3,  2, 4,  9, 12, 13, 12, 14, 14,  6, 0,
       27938,      3,  0, 4,  5, 17, 11, 12, 12, 12,  4, 1,
       28911,      1,  0, 4, 14,  6, 11, 11, 12, 12,  2, 1,
       29024,      3,  4, 3,  9, 14, 12, 11, 12, 12,  6, 1,
       29269,      3,  4, 1, 12, 13, 13, 14, 14,  6,  5, 0,

       29662,      1,  4, 3, 11, 13, 13, 12, 14, 11,  6, 1,
       30058,      2,  7, 4,  8, 15, 12, 11, 12, 12,  4, 1,
       30117,      5,  4, 4,  9, 14, 12, 12, 12, 13,  6, 1,
       31652,      6,  2, 3,  8, 13, 12, 13, 12,  9,  5, 1,
       32607,      5,  0, 3,  6, 11,  7, 12, 12, 13,  2, 0,
       32830,      2,  3, 4,  8, 10, 13, 11, 12, 12,  5, 1,
       32966,      2,  2, 1,  9, 15, 13, 12, 12, 11,  6, 1,
       34807,      1,  2, 3,  8,  6, 13, 11, 12, 11,  4, 1,
       35027,      3,  5, 4,  8, 14,  9,  9, 11, 12,  6, 1,
       37087,      3,  3, 4, 14, 19, 11, 13, 14, 14,  6, 1,

       37603,      1,  3, 4, 11, 11, 11, 12, 12, 11,  6, 0,
       38760,      1,  4, 4,  9,  8, 12, 11, 15, 14,  6, 1,
       39792,      1,  3, 4,  8,  8,  9, 12, 12, 12,  4, 0,
       39980,      5,  4, 4,  8, 13, 13,  7, 12, 12,  5, 0,
       40518,      5,  2, 3,  8, 14, 12, 12, 13, 13,  4, 0,
       41139,      6,  1, 4,  9, 13, 16, 12, 12, 11,  4, 1,
       41357,      5,  1, 4,  8, 13, 18,  9, 12, 13,  2, 0,
       42270,      1,  3, 4,  9,  9, 13, 13, 11, 13,  5, 1,  
       42677,      1,  1, 4,  8, 10, 11, 12, 12, 11,  4, 1,   
       42723,      1,  6, 4, 10, 12, 11, 12, 12, 12,  6, 1,

       42856,      6,  1, 4,  9, 12, 13, 12, 11, 12,  5, 0,
       43664,      1,  3, 3,  8,  9, 12,  9, 13, 12,  6, 1,   
       44337,      1,  2, 4,  6, 12, 12, 11, 12, 13,  5, 1,   
       44450,      1,  4, 4, 11, 12, 13, 12, 10, 12,  6, 0,   
       45163,      3,  7, 4,  9, 14, 12, 11, 12, 11,  6, 0,   
       45186,      6,  0, 3,  4, 12, 11, 11, 10, 12,  2, 1,
       50907,      3,  5, 4,  8, 13, 12, 11, 12, 10,  6, 0,         
       51241,      5,  3, 4,  8, 12, 13, 12, 12, 12,  5, 1,
       51552,      1,  4, 4,  8, 12, 12, 13, 11, 13,  4, 0,   
       52507,      2,  4, 4, 10, 12, 11, 12, 12, 11,  5, 0,

       53340,      2,  5, 4,  8, 12, 12, 11, 12, 11,  6, 1,
       54575,      1,  3, 4, 14, 15, 14, 12, 15, 19,  5, 0,   
       54937,      3,  7, 4, 11, 17, 19, 14, 12, 13,  6, 1,   
       54959,      5,  4, 4,  7, 13, 10, 13, 10, 13,  6, 1,   
       54964,      2,  7, 4, 11, 13, 12, 11, 11, 11,  6, 0,   
       55358,      6,  1, 4,  8, 12, 13,  6, 14, 13,  5, 1,
       55924,      6,  5, 3,  8, 11, 10, 10, 11,  9,  4, 1,         
       56407,      5,  4, 4,  8, 13, 14,  5, 11, 14,  5, 1,
       56574,      6,  3, 4,  8, 15, 13, 11, 12, 11,  6, 0,   
       56931,      6,  0, 2,  4,  8,  7, 10, 15, 11,  4, 0,   

       58551,      3,  6, 3,  8, 14, 14, 14, 18, 12,  4, 1,
       60055,      3,  2, 4,  7, 16, 11, 11, 11, 13,  6, 0,   
       60491,      4,  1, 4,  8,  9, 16, 16, 19, 18,  4, 1,   
       61022,      1,  4, 4, 10, 15, 12, 12, 12, 12,  6, 1,   
       61035,      5,  7, 2, 11, 12, 12, 11, 11, 13,  6, 1,   
       61035,      6,  2, 3,  8, 20, 13, 12, 16,  0,  1, 1,
       61080,      5,  1, 4,  6, 13, 13,  6, 12, 11,  5, 1,         
       61315,      5,  4, 3,  8, 12, 12, 10, 12, 12,  4, 0,
       61315,      5,  6, 4,  6, 14, 11,  8, 10, 11,  6, 0,   
       61402,      5,  5, 3,  8, 13, 11, 11, 12, 12,  6, 0,   

       62320,      4,  5, 4,  9, 13, 13, 11, 12, 15,  6, 1,
       62350,      3,  3, 4, 11, 15, 11, 11, 12, 11,  4, 1,   
       62514,      2,  7, 4,  6, 13, 13, 12, 13, 17,  6, 0,   
       62862,      6,  5, 4, 11, 15, 12, 11, 12, 13,  6, 1,   
       64579,      2,  6, 4, 10, 16, 11, 16, 14, 13,  4, 1,   
       66365,      6,  4, 4,  8, 13, 12, 12, 11, 12,  6, 1,
       67266,      4,  5, 4,  9, 13, 12, 11, 12, 12,  6, 0,         
       68127,      2,  4, 3, 11, 13, 12, 13, 12, 13,  5, 1,
       68410,      2,  0, 1,  5,  9, 14, 11, 13, 14,  4, 1,   
       68982,      1,  2, 4,  5, 13, 14, 10, 15, 12,  5, 0,   

       69337,      5,  7, 4,  8, 14, 11, 11, 12, 12,  6, 1,
       70833,      2,  6, 4,  8, 11, 12, 12, 14, 11,  6, 1,   
       71145,      1,  7, 4, 11, 13, 12, 12, 12, 13,  6, 0,   
       71541,      3,  4, 3,  6, 13,  8, 12, 12, 12,  3, 0,   
       71806,      3,  2, 4,  9,  9, 12, 11, 20, 15,  5, 0,   
       71829,      1,  1, 3,  8,  9, 14, 12, 14, 12,  5, 1,
       72138,      2,  2, 4,  9, 10, 12, 12, 13, 11,  6, 1,
       73260,      2,  6, 4,  8, 14, 14, 11, 14,  9,  6, 0,
       73779,      3,  1, 4,  8, 15, 14, 11, 11, 12,  4, 1,   
       74140,      2,  7, 4,  9, 11, 12, 11,  8, 11,  6, 1,

       75054,      3,  7, 3,  7, 13, 13, 11, 11, 10,  6, 0,   
       75543,      1,  4, 4,  9,  8, 10, 12, 13, 12,  6, 1,   
       76393,      1,  2, 2,  8, 12,  8, 12, 13,  7,  1, 0,
       77182,      2,  0, 4,  4, 11, 11, 10, 11, 12,  4, 1,
       77390,      1,  6, 1, 10, 13, 13, 14, 12, 14,  6, 1,   
       77490,      4,  5, 4, 10, 13, 15, 13, 14, 14,  6, 1,   
       78808,      1,  4, 3,  9, 10, 12, 12, 11, 13,  4, 1,   
       80404,      3,  4, 4,  8, 12, 11,  8, 12, 11,  5, 1,   
       81118,      2,  7, 4, 10, 13, 11, 11, 12, 10,  6, 0,
       81502,      6,  2, 4,  9, 14, 12, 11, 15, 11,  6, 1,

       81576,      4,  2, 4,  9, 13, 12, 12, 11, 14,  6, 0,
       82004,      3,  3, 1,  8, 13, 12, 12, 10, 13,  6, 0,   
       82446,      3,  7, 4,  7, 12, 13, 13, 14, 13,  6, 1,   
       82955,      5,  2, 4,  8, 12, 12, 12, 12, 10,  6, 0,   
       83622,      2,  0, 3,  4, 10, 12, 12, 14, 12,  2, 1,   
       88031,      5,  2, 4, 10, 14, 12, 11, 12, 13,  5, 0,
       88271,      3,  5, 4,  9, 14, 10,  9, 11, 12,  6, 0,         
       90468,      4,  4, 4,  8, 13, 12, 12, 12, 12,  6, 1,
       90741,      3,  2, 4, 11, 11, 12, 10, 11, 12,  4, 1,   
       92162,      4,  4, 4, 10, 12, 13, 12, 12, 13,  6, 1,   

       92480,      6,  7, 3, 10, 13, 12, 10, 12, 11,  6, 1,
       95164,      5,  3, 4, 11, 14, 11,  9, 13, 13,  5, 1,   
       97172,      2,  2, 4,  9, 14, 11, 13, 12, 12,  4, 1,   
       99383,      3,  3, 4,  9, 15, 13, 15, 14, 19,  6, 0,   
      100142,      2,  4, 4,  9, 13, 12, 13, 12, 13,  6, 1,   
      101104,      5,  7, 2,  7, 13, 12,  6, 11, 10,  4, 0,
      101506,      1,  5, 2,  8, 10, 12, 13, 11, 11,  6, 0,
      101650,      3,  2, 3,  8, 14, 12,  9, 12, 13,  3, 1,
      102379,      3,  2, 4, 11, 14, 11, 10, 20, 14,  4, 1,   
      106345,      1,  3, 4,  8, 13, 13, 11, 11, 12,  6, 0,

      106407,      3,  6, 4,  7, 13, 11, 12, 13, 11, -6, 1,   
      107360,      3,  7, 4, 10, 12, 11, 10, 11, 12,  6, 1,   
      109461,      1,  4, 4,  7, 10, 12, 10, 11, 12,  4, 1,
      111653,      6,  5, 4,  8, 13, 13, 11, 12, 12,  6, 1,
      112582,      2,  2, 2,  8, 10, 11, 16, 10, 14,  5, 0,   
      112735,      4,  0, 4,  5, 14, 10, 12, 11, 11,  4, 0,   
      113339,      2,  2, 4, 12, 15, 13, 11, 13, 12,  4, 0,   
      114418,      5,  2, 4,  8, 10,  8, 11, 12, 13,  5, 1,   
      114519,      5,  4, 3,  9, 15, 10, 12, 12, 13,  6, 1,
      115559,      6,  4, 4,  8, 12, 13, 12, 11, 12,  6, 1
};
*/


  printf(" bosplit reached !\n");
 
  bcsfl = (float*)bcs_.iw;
  bosInit(bcs_.iw,NBCS);


/* extract events in according with table */

  fparm_(str1, strlen(str1));
  fparm_(str2, strlen(str2));

  iev = 0;
  for(i=0;i<116000;i++)
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

    if((ind = bosNlink(bcs_.iw,"HEAD",0)) > 0)
    {
	  /*printf("%d %d -> %d\n",bcs_.iw[ind+2],nev[iev][0],iev);*/
      if(bcs_.iw[ind+2] == nev[iev][0])
      {

bosLctl(bcs_.iw,"E=","HEADDC0 EC  SC  CC  ST  ");

        fwbos_(bcs_.iw,&tmp2,"E",&iret,1);
        iev ++;
      }
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

  exit(0);
}

