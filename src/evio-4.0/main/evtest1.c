
/* evtest1.c - simulate and write CLAS12-style data file */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "evio.h"

int *makeEvent(int nev);

#define NEVMAX 100000
#define NRAWSAMPLES 40
#define NSAMPLES 15
unsigned int buffer[NEVMAX];

#define MIN(a,b) (a<b)? a : b


int
main()
{
  int i, handle, status, nevents, nlong, handle2;

  printf("Event I/O tests...\n");
  status = evOpen("sample.dat","w",&handle);
  printf ("Opening sample.dat status %d\n",status);

  for(i=0; i<10; i++)
  {
    makeEvent(i+1);
    status = evWrite(handle,buffer);
    printf ("Writing single.dat %d\n",status);
  }

  status = evClose(handle);
  printf ("Closing single.dat status %d ",status);


  exit(0);
}



/* disentangled physics event:

   length
   0xe00
*/


int *
makeEvent(int nev)
{
  unsigned int *dabufp;
  unsigned int *dabufp_save0, *dabufp_save1, *dabufp_save2;
  char *ch;
  char *str1="c,i,l,N(c,Ns)", *str2="c,i,l,N(c,N(c,Ns))", *str3="c,i,l,N(c,N(c,i))";
  int len1, len2, len3;
  int n1, n2, n3;
  unsigned char *b08;
  unsigned short *b16;
  unsigned int *b32;
  unsigned long long *b64;
  int islot, ichan, ii, jj, npulses;

  len1 = strlen(str1); /* format length in bytes */
  len2 = strlen(str2);
  len3 = strlen(str3);
  n1 = (len1+5)/4; /* format length in words */
  n2 = (len2+5)/4;
  n3 = (len3+5)/4;

  dabufp = buffer;

  /* disentangled physics record header: length will be updated in the end,
  2nd word contains contectType=0xe(bank of banks), no tag or num for now */
  dabufp_save0 = dabufp ++;
  *dabufp++ = (0xe<<8);

  printf("1\n");fflush(stdout);

  /*****************************************************/
  /* data bank1(composite bank) (FADC WINDOW RAW MODE) */

  /* bank-of-banks header */
  dabufp_save1 = dabufp ++;
  *dabufp++ = (0xe101<<16) + (0xf<<8);

  /* format tagsegment header */
  *dabufp++ = (len1<<20) + (0x6<<16) + n1;
  ch = (char *)dabufp;
  strncpy(ch,str1,len1);
  ch[len1]='\0';ch[len1+1]='\4';ch[len1+2]='\4';ch[len1+3]='\4';ch[len1+4]='\4';
  dabufp += n1;

  /* data bank header */
  dabufp_save2 = dabufp ++;
  *dabufp++ = (0<<16) + (0x0<<8) + 0;

  /* data */
  b08 = (unsigned char *)dabufp;

  for(islot=3; islot<21; islot++)
  {
    if(islot==11||islot==12) continue;

    *b08++ = islot;

    b32 = (unsigned int *)b08;
    *b32 = nev;
    b08 += 4;

    b64 = (unsigned long long *)b08;
    *b64 = 0x123456789abcdef0LL;
    b08 += 8;

    b32 = (unsigned int *)b08;
    *b32 = 16;
    b08 += 4;

    for(ichan=0; ichan<16; ichan++)
    {
      *b08++ = ichan;

      b32 = (unsigned int *)b08;
      *b32 = NRAWSAMPLES;
      b08 += 4;

      b16 = (unsigned short *)b08;
      for(ii=0; ii<NRAWSAMPLES; ii++) b16[ii] = 250+rand()/200000000;
      b08 += (NRAWSAMPLES*2);
    }
  }


  /* update bank lengths */
  dabufp = (unsigned int *) ( ( ((int)(b08+3)) / 4 ) * 4);
  dabufp_save2[1] = ((int)dabufp - (int)b08)<<14;
  printf("0x%x %d --- 0x%x %d --> padding %d\n",dabufp,dabufp,b08,b08,((dabufp_save2[1])>>14)&0x3);

  *dabufp_save1 = (dabufp-dabufp_save1-1);
  *dabufp_save2 = (dabufp-dabufp_save2-1);


  printf("2\n");fflush(stdout);

  /****************************************************/
  /* data bank2(composite bank) (FADC PULSE RAW MODE) */

  /* bank-of-banks header */
  dabufp_save1 = dabufp ++;
  *dabufp++ = (0xe102<<16) + (0xf<<8);

  /* format tagsegment header */
  *dabufp++ = (len2<<20) + (0x6<<16) + n2;
  ch = (char *)dabufp;
  strncpy(ch,str2,len2);
  ch[len2]='\0';ch[len2+1]='\4';ch[len2+2]='\4';ch[len2+3]='\4';ch[len2+4]='\4';
  dabufp += n2;

  /* data bank header */
  dabufp_save2 = dabufp ++;
  *dabufp++ = (0<<16) + (0x0<<8) + 0;

  /* data */
  b08 = (unsigned char *)dabufp;

  for(islot=3; islot<21; islot++)
  {
    if(islot==11||islot==12) continue;

    *b08++ = islot;

    b32 = (unsigned int *)b08;
    *b32 = nev;
    b08 += 4;

    b64 = (unsigned long long *)b08;
    *b64 = 0x123456789abcdef0LL;
    b08 += 8;

    b32 = (unsigned int *)b08;
    *b32 = 16;
    b08 += 4;

    for(ichan=0; ichan<16; ichan++)
	{
      *b08++ = ichan;

      npulses = 1+(int)(4.0*(rand()/(RAND_MAX+1.0))); /*from 1 to 4*/

      b32 = (unsigned int *)b08;
      *b32 = npulses;
      b08 += 4;

      for(jj=0; jj<npulses; jj++)
	  {
        /*first sample*/
        *b08++ = 10+(int)(4.0*(rand()/(RAND_MAX+1.0))); /*from 10 to 14*/;

        /*the number of samples*/
        b32 = (unsigned int *)b08;
        *b32 = NSAMPLES;
        b08 += 4;

        b16 = (unsigned short *)b08;
        for(ii=0; ii<NSAMPLES; ii++) b16[ii] = 500+rand()/100000000;
        b08 += (NSAMPLES*2);
	  }

    }
  }

  /* update bank lengths */
  dabufp = (unsigned int *) ( ( ((int)(b08+3)) / 4 ) * 4);
  dabufp_save2[1] = ((int)dabufp - (int)b08)<<14;
  printf("0x%x %d --- 0x%x %d --> padding %d\n",dabufp,dabufp,b08,b08,((dabufp_save2[1])>>14)&0x3);


  *dabufp_save1 = (dabufp-dabufp_save1-1);
  *dabufp_save2 = (dabufp-dabufp_save2-1);



  printf("3\n");fflush(stdout);

  /*********************************************************/
  /* data bank3(composite bank) (FADC PULSE INTEGRAL MODE) */

  /* bank-of-banks header */
  dabufp_save1 = dabufp ++;
  *dabufp++ = (0xe103<<16) + (0xf<<8);

  /* format tagsegment header */
  *dabufp++ = (len3<<20) + (0x6<<16) + n3;
  ch = (char *)dabufp;
  strncpy(ch,str3,len3);
  ch[len3]='\0';ch[len3+1]='\4';ch[len3+2]='\4';ch[len3+3]='\4';ch[len3+4]='\4';
  dabufp += n3;

  /* data bank header */
  dabufp_save2 = dabufp ++;
  *dabufp++ = (0<<16) + (0x0<<8) + 0;

  /* data */
  b08 = (unsigned char *)dabufp;

  for(islot=3; islot<21; islot++)
  {
    if(islot==11||islot==12) continue;

    *b08++ = islot;

    b32 = (unsigned int *)b08;
    *b32 = nev;
    b08 += 4;

    b64 = (unsigned long long *)b08;
    *b64 = 0x123456789abcdef0LL;
    b08 += 8;

    b32 = (unsigned int *)b08;
    *b32 = 16;
    b08 += 4;

    for(ichan=0; ichan<16; ichan++)
	{
      *b08++ = ichan;

      npulses = 1+(int)(4.0*(rand()/(RAND_MAX+1.0))); /*from 1 to 4*/

      b32 = (unsigned int *)b08;
      *b32 = npulses;
      b08 += 4;

      for(jj=0; jj<npulses; jj++)
	  {
        /*quality factor*/
        *b08++ = 1+(int)(3.0*(rand()/(RAND_MAX+1.0))); /*from 1 to 3*/;

        /*pulse integral*/
        b32 = (unsigned int *)b08;
        *b32 = 5000+(int)(50.0*(rand()/(RAND_MAX+1.0)));
        b08 += 4;
	  }

    }
  }

  /* update bank lengths */
  dabufp = (unsigned int *) ( ( ((int)(b08+3)) / 4 ) * 4);
  dabufp_save2[1] = ((int)dabufp - (int)b08)<<14;
  printf("0x%x %d --- 0x%x %d --> padding %d\n",dabufp,dabufp,b08,b08,((dabufp_save2[1])>>14)&0x3);

  *dabufp_save1 = (dabufp-dabufp_save1-1);
  *dabufp_save2 = (dabufp-dabufp_save2-1);


  /* update length of the extarnal bank (we calls it 'event') */
  *dabufp_save0 = (dabufp-dabufp_save0-1);

  printf("4\n");fflush(stdout);



  return(0);
}
