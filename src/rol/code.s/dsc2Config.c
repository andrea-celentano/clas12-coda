/****************************************************************************
 *
 *  dsc2Config.c  -  configuration library file for dsc2 board 
 *
 *  Author: Serguei Pozdniakov
 *          Jefferson Lab Online Group
 *          September 2009
 *
 *          Sergey Boyarinov Oct 2013: major revision for the new firmware, config file format changed !!!


config file format:

CRATE      <tdcecal1>     <- crate name, usually IP name

DSC2_SLOT  2   <- slot#

DSC2_WIDTH  20  40   <- TDC width (ns), TRG width (ns)

DSC2_TDCMASK  1  1  1  1  1  1  1  1  1  1  1  1  1  1  1  1     <- TDC enable mask

DSC2_TRGMASK  1  1  1  1  1  1  1  1  1  1  1  1  1  1  1  1     <- TRG enable mask

DSC2_TDCORMASK  1  1  1  1  1  1  1  1  1  1  1  1  1  1  1  1     <- TDC OR mask

DSC2_TRGORMASK  1  1  1  1  1  1  1  1  1  1  1  1  1  1  1  1     <- TRG OR mask

DSC2_THRESHOLD  20  50   <- board threshold: TDC threshold (mV), TRG threshold (mV)

DSC2_CH_THRESHOLD    0  20   50   <- channel threshold: channel#, TDC threshold (mV), TRG threshold (mV)

following, if used, will supersede TRG width:

DSC2_TRGDIGITAL   40  64    <- board digital TRG output: width (ns), delay (ns)

DSC2_CH_TRGDIGITAL   0   40  64    <- channel digital TRG output: channel#, width (ns), delay (ns)

*/

#if defined(VXWORKS) || defined(Linux_vme)

#ifdef VXWORKS
#include <vxWorks.h>
#include <vxLib.h>
#include <logLib.h>
#else
#include "jvme.h"
#endif

#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "dsc2Lib.h"
#include "dsc2Config.h"


/* Global variables */
static int Ndsc = 0;                        /* Number of DSCs in Crate */

#undef DEBUG

#define FNLEN     128       /* length of config. file name */
#define STRLEN    250       /* length of str_tmp */
#define ROCLEN     80       /* length of ROC_name */
#define NBOARD     22
#define NCHAN      16


/* Define global variables (NOTE: index in following arrays is SLOT NUMNER !!!) */
static int     TDCWidth[NBOARD];
static int     TRGWidth[NBOARD];
static int     TDCThreshold[NBOARD][NCHAN];
static int     TRGThreshold[NBOARD][NCHAN];
static UINT32  ChannelMask[NBOARD];
static UINT32  ORMask[NBOARD];
static int     TRGDigitalWidth [NBOARD][NCHAN];
static int     TRGDigitalDelay [NBOARD][NCHAN];




/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/



/*
cd "/usr/local/clas12/coda/src/rol/VXWORKS_ppc/lib"
ld < librol.so
dsc2Init(0x100000,0x80000,20,0)
dsc2Config("")
dsc2Mon(0)
*/

/* main function, have to be called after dsc2Init() */
int
dsc2Config(char *fname)
{
  int res;

  /* set defaults */
  dsc2InitGlobals();

  /* dsc2Init() must be called by now; get the number of boards from there */
  Ndsc = dsc2GetNdsc();
  printf("dsc2Config: Ndsc=%d\n",Ndsc);

  /* read config file */
  if( (res = dsc2ReadConfigFile(fname)) < 0 ) return(res);

  /* download to all boards */
  dsc2DownloadAll();

  return(0);
}


void
dsc2InitGlobals()
{
  int ii, jj;

  for(ii=0; ii<NBOARD; ii++)
  {
    TDCWidth[ii] = 40;
    TRGWidth[ii] = 40;
    for(jj=0; jj<NCHAN; jj++) TDCThreshold [ii][jj] = 20;
    for(jj=0; jj<NCHAN; jj++) TRGThreshold [ii][jj] = 40;
    ChannelMask[ii] = 0xffffffff;
    ORMask[ii] = 0xffffffff;
    for(jj=0; jj<NCHAN; jj++) TRGDigitalWidth [ii][jj] = 0;
    for(jj=0; jj<NCHAN; jj++) TRGDigitalDelay [ii][jj] = 0;
  }
}


#define DSC2_READ_MASKS(XXMASK,R_MSK,JJ_ADD) \
	else if( (strcmp(keyword,(XXMASK)) == 0) && (kk >= 0) ) \
	{ \
	  sscanf (str_tmp, "%*s %d %d %d %d %d %d %d %d \
                                %d %d %d %d %d %d %d %d", \
		  &msk[ 0], &msk[ 1], &msk[ 2], &msk[ 3], \
		  &msk[ 4], &msk[ 5], &msk[ 6], &msk[ 7], \
		  &msk[ 8], &msk[ 9], &msk[10], &msk[11], \
		  &msk[12], &msk[13], &msk[14], &msk[15]); \
      for(slot=slot1; slot<slot2; slot++) \
      { \
	    for(jj=0; jj<NCHAN; jj++) \
	    { \
	      if( (msk[jj] < 0) || (msk[jj] > 1) ) \
	      { \
	        printf("\nReadConfigFile: Wrong mask bit value, %d\n\n",msk[jj]); return(-4); \
	      } \
	      (R_MSK) [slot] |= (msk[jj]<<(JJ_ADD)); \
	    } \
      } \
	}


/* reading and parsing config file */
/* config file will be selected using following rules:
     1. use 'filename' specified; if name starts from '/' or './', use it,
        otherwise use file from '$CLON_PARMS/dsc2/' area
     2. if name is "", use file 'hostname'.conf from '$CLON_PARMS/dsc2/' area
     3. if previous does not exist, use file '$EXPID.conf' from '$CLON_PARMS/dsc2/' area
 */
int
dsc2ReadConfigFile(char *filename)
{
  FILE   *fd;
  int    ii, jj, ch, kk = -1;
  char   str_tmp[STRLEN], str2[STRLEN], keyword[ROCLEN];
  char   host[ROCLEN], ROC_name[ROCLEN];
  int    i1, i2, i3, i4, msk[NCHAN];
  int    slot, slot1, slot2, chan;
  char fname[FNLEN] = { "" };  /* config file name */
  char *getenv();
  char *clonparms;
  char *expid;

  clonparms = getenv("CLON_PARMS");
  expid = getenv("EXPID");
  if(strlen(filename)!=0) /* filename specified */
  {
    if ( filename[0]=='/' || (filename[0]=='.' && filename[1]=='/') )
	{
      sprintf(fname, "%s", filename);
	}
    else
	{
      sprintf(fname, "%s/dsc2/%s", clonparms, filename);
	}

    if((fd=fopen(fname,"r")) == NULL)
    {
      printf("\nReadConfigFile: Can't open config file >%s<\n",fname);
      return(-1);
    }
  }
  else /* filename does not specified */
  {
    /* obtain our hostname */
    gethostname(host,ROCLEN);
    sprintf(fname, "%s/dsc2/%s.conf", clonparms, host);
    if((fd=fopen(fname,"r")) == NULL)
    {
      sprintf(fname, "%s/dsc2/%s.conf", clonparms, expid);
      if((fd=fopen(fname,"r")) == NULL)
      {
        printf("\nReadConfigFile: Can't open config file >%s<\n",fname);
        return(-2);
	  }
	}

  }

  printf("\nReadConfigFile: Using configuration file >%s<\n",fname);


  /* Parsing of config file */
  while ((ch = getc(fd)) != EOF)
  {
    if ( ch == '#' || ch == ' ' || ch == '\t' )
    {
      while (getc(fd) != '\n') {}
    }
    else if( ch == '\n' ) {}
    else
    {
      ungetc(ch,fd);
      fgets(str_tmp, STRLEN, fd);
      sscanf (str_tmp, "%s", keyword);
#ifdef DEBUG
      printf("\nfgets returns %s so keyword=%s\n\n",str_tmp,keyword);
#endif

      if(strcmp(keyword,"CRATE") == 0)
      {
        /* check if */
        sprintf(ROC_name, "<%s>", host);
        if( (strstr(str_tmp,ROC_name) != NULL))
        {
          printf("\nERROR: ReadConfigFile: Wrong crate name in config file (%s)\n",str_tmp);
          return(-3);
        }
      }

      else if(strcmp(keyword,"DSC2_SLOT") == 0)
      {
        kk++;
        sscanf (str_tmp, "%*s %s", str2);
        /*printf("str2=%s\n",str2);*/
        if(isdigit(str2[0]))
        {
          slot1 = atoi(str2);
          slot2 = slot1 + 1;
          if(slot1<2 && slot1>21)
          {
            printf("\nReadConfigFile: Wrong slot number %d\n\n",slot1);
            return(-4);
          }
        }
        else if(!strcmp(str2,"all"))
        {
          slot1 = 0;
          slot2 = NBOARD;
        }
        else
        {
          printf("\nReadConfigFile: Wrong slot >%s<, must be 'all' or actual slot number\n\n",str2);
          return(-4);
        }
        /*printf("slot1=%d slot2=%d\n",slot1,slot2);*/
	  }

      DSC2_READ_MASKS("DSC2_TDCMASK",ChannelMask,(jj))

      DSC2_READ_MASKS("DSC2_TRGMASK",ChannelMask,(jj+16))

      DSC2_READ_MASKS("DSC2_TDCORMASK",ORMask,(jj))

      DSC2_READ_MASKS("DSC2_TRGORMASK",ORMask,(jj+16))

      else if( (strcmp(keyword,"DSC2_WIDTH") == 0) && (kk >= 0) )
	  {
        sscanf (str_tmp, "%*s %d %d", &i1, &i2);
        for(slot=slot1; slot<slot2; slot++) TDCWidth[slot] = i1;
        for(slot=slot1; slot<slot2; slot++) TRGWidth[slot] = i2;
      }

      else if( (strcmp(keyword,"DSC2_THRESHOLD") == 0) && (kk >= 0) )
	  {
        sscanf (str_tmp, "%*s %d %d", &i1, &i2);
        for(slot=slot1; slot<slot2; slot++) for(chan=0; chan<NCHAN; chan++) TDCThreshold[slot][chan] = i1;
        for(slot=slot1; slot<slot2; slot++) for(chan=0; chan<NCHAN; chan++) TRGThreshold[slot][chan] = i2;
      }

      else if( (strcmp(keyword,"DSC2_CH_THRESHOLD") == 0) && (kk >= 0) )
      {
        sscanf (str_tmp, "%*s %d %d %d", &chan, &i1, &i2);
        if((chan<0) || (chan>=NCHAN))
        {
          printf("\nReadConfigFile: Wrong channel number, %d\n\n",chan);
          return(-5);
        }
        for(slot=slot1; slot<slot2; slot++) TDCThreshold[slot][chan] = i1;
        for(slot=slot1; slot<slot2; slot++) TRGThreshold[slot][chan] = i2;
      }

      else if( (strcmp(keyword,"DSC2_TRGDIGITAL") == 0) && (kk >= 0) )
      {
        sscanf (str_tmp, "%*s %d %d", &i1, &i2);
        for(slot=slot1; slot<slot2; slot++) for(chan=0; chan<NCHAN; chan++) TRGDigitalWidth[slot][chan] = i1;
        for(slot=slot1; slot<slot2; slot++) for(chan=0; chan<NCHAN; chan++) TRGDigitalDelay[slot][chan] = i2;
      }

      else if( (strcmp(keyword,"DSC2_CH_TRGDIGITAL") == 0) && (kk >= 0) )
      {
        sscanf (str_tmp, "%*s %d %d %d", &chan, &i1, &i2);
        if((chan<0) || (chan>=NCHAN))
        {
          printf("\nReadConfigFile: Wrong channel number, %d\n\n",chan);
          return(-6);
        }
        for(slot=slot1; slot<slot2; slot++) TRGDigitalWidth[slot][chan] = i1;
        for(slot=slot1; slot<slot2; slot++) TRGDigitalDelay[slot][chan] = i2;
      }

      else
      {
        printf("ReadConfigFile: Unknown Field or Missed Field in\n");
        printf("   %s \n", fname);
        printf("   str_tmp=%s", str_tmp);
        printf("   keyword=%s \n\n", keyword);
        return(-7);
      }

    }
  }
  fclose(fd);

#ifdef DEBUG
  Ndsc = dsc2GetNdsc();
  for(ii=0; ii<Ndsc; ii++)
  {
    slot = dsc2Slot(ii);
    printf("\nslot=%2d\n",slot);
	printf("   TDCWidth=%d\n",TDCWidth[slot]);
	printf("   TRGWidth=%d\n",TRGWidth[slot]);
	printf("   ChannelMask=0x%08x\n",ChannelMask[slot]);
	printf("   ORMask=0x%08x\n",ORMask[slot]);
    for(jj=0;jj<NCHAN;jj++)
    {
      printf("      chan=%2d, TDCThreshold=%d, TRGThreshold=%d\n",
           jj,TDCThreshold[slot][jj],TRGThreshold[slot][jj]);
	}
    for(jj=0;jj<NCHAN;jj++)
    {
      printf("      chan=%2d, TRGDigitalWidth=%d, TRGDigitalDelay=%d\n",
           jj,TRGDigitalWidth[slot][jj],TRGDigitalDelay[slot][jj]);
	}
  }
#endif

  return(kk);
}





int
dsc2DownloadAll()
{
  int slot, chan, kk;
  UINT16 selMask;
  UINT16 bypMask;

  Ndsc = dsc2GetNdsc();
  for(kk=0; kk<Ndsc; kk++)
  {
    slot = dsc2Slot(kk);
    dsc2SetPulseWidth(slot, TDCWidth[slot], 1);
    dsc2SetPulseWidth(slot, TRGWidth[slot], 2);
    dsc2SetChannelMask(slot, ChannelMask[slot], 0);
    dsc2SetChannelORMask(slot, ORMask[slot], 0);
    for(chan=0; chan<NCHAN; chan++)
    {
      dsc2SetThreshold(slot,chan,TDCThreshold[slot][chan],1);
      dsc2SetThreshold(slot,chan,TRGThreshold[slot][chan],2);
    }

    selMask = 0xFFFF; /* by default, trigger output will use TRG discriminator */
    bypMask = 0xFFFF; /* by default, TRG discriminator output goes directly to trigger output */
    for(chan=0; chan<NCHAN; chan++)
    {
      dsc2SetTRGOut(slot, chan, TRGDigitalWidth[slot][chan], TRGDigitalDelay[slot][chan]);
      /* if TRGDigitalWidth[][]>0, set trigger output to the digital */
      if(TRGDigitalWidth[slot][chan]>0)
	  {
		/*printf("[%2d][%2d] befor 0x%04x\n",slot,chan,bypMask);*/
        bypMask = bypMask ^ (1<<chan);
		/*printf("[%2d][%2d] after 0x%04x\n",slot,chan,bypMask);*/
	  }
	}
	/*printf("[%2d] setup 0x%04x\n",slot,bypMask);*/
    dsc2SetTRGOutSource(slot, selMask, bypMask);

  }
  return(0);
}



/* print board registers; if slot is zero, print all boards */
void
dsc2Mon(int slot)
{
  int id, start, end, kk, jj;
  UINT16 bypMask;

  Ndsc = dsc2GetNdsc();
  if(slot==0)
  {
    start = 0;
    end = Ndsc;
  }
  else if((id = dsc2Id(slot)) >= 0)
  {
    start = id;
    end = start + 1;
  }
  else
  {
    return;
  }

  printf("Ndsc=%d\n",Ndsc);
  for(kk=start; kk<end; kk++)
  {
    slot = dsc2Slot(kk);
    printf("\nDSC2 number %d in slot %d\n",kk,slot);
    printf("   TDCWidth=%d(ns) TRGWidth=%d(ns)\n",
       dsc2GetPulseWidth(slot,1),
       dsc2GetPulseWidth(slot,2));
    printf("   ChannelMask=0x%08x ORMask=0x%08x \n",
       dsc2GetChannelMask(slot, 0),
       dsc2GetChannelORMask(slot, 0));
    for(jj=0;jj<NCHAN;jj++)
    {
      printf("      TDC_threshold[%2d]=%4d(mV)   TRG_threshold[%2d]=%4d(mV)\n",
	   jj, dsc2GetThreshold(slot,jj,1),
	   jj, dsc2GetThreshold(slot,jj,2));
    }
    bypMask = dsc2GetTRGOutSource(slot,1);
    for(jj=0;jj<NCHAN;jj++)
    {
      if( (bypMask&(1<<jj))==0 )
	  {
        printf("      TRGDigitalDelay[%2d]=%4d(ns)   TRGDigitalWidth[%2d]=%4d(ns) <- supersede TRGWidth\n",
	      jj, dsc2GetTRGOutDelay(slot,jj),
	      jj, dsc2GetTRGOutWidth(slot,jj));
	  }
    }
    printf("\n");
  }
}



#else /* dummy version*/

void
dsc2Config_dummy()
{
  return;
}

#endif
