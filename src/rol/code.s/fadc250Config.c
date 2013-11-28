/****************************************************************************
 *
 *  fadc250Config.c  -  configuration library file for fADC250 board 
 *
 *  SP, 07-Nov-2013
 * Sergey Boyarinov Nov 2013 - simplify/adjust for Hall B
 *
 *  empty lines and line startes with # - will be ignored
 *  config file format:

CRATE             rocbcal1   <- ROC/crate name, usually IP name
FADC250_ALLSLOTS             <- just keyword - all settings after this line will be implemented
#                                              for all slots, till FADC250_SLOTS will be met
FADC250_SLOTS     3  8  15   <- slot_numbers - in which next settings will be implemented
#                                              till file ends or next FADC250_SLOTS will be met
FADC250_F_REV     0x02c1     <- firmware revision  (0x0 Bits:7-0)
FADC250_B_REV     0x0a03     <- board revision     (0x0 Bits:15-8)
FADC250_ID        0xfadc     <- board type         (0x0 Bits:31-16)

FADC250_MODE      1   <- process mode: 1-4  (0x10C Bits:2-0)
FADC250_W_OFFSET  50  <- number of sample back from trigger point. (0x120)
#                           (in Manual it is  PL=Trigger_Window(ns) * 250MHz)
FADC250_W_WIDTH   49  <- number of ADC sample to include in trigger window. (0x11C)
#                           (in M:  PTW=Trigger_Window(ns) * 250MHz, minimum is 6)
FADC250_NSB       3   <- number of sample before trigger point to include in data processing. (0x124)
#                           This include the trigger Point. (minimum is 2 in all mode)
FADC250_NSA       6   <- number of sample after trigger point to include in data processing. (0x128)
#                           Minimum is (6 in mode 2) and ( 3 in mode 0 and 1).
#                           Number of sample report is 1 more for odd and 2 more for even NSA number.
FADC250_NPEAK     1   <- number of Pulses in Mode 2 and 3.  (0x10C Bits:6-5)

#                 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 - channels ##
FADC250_DIS_MASK  1  0  1  0  1  0  1  0  1  0  1  0  1  0  1  0   <- channel disable mask
#                                               (ch.ADC.values=0, if bit set to 1). (0x110)
FADC250_TRG_MASK  1  1  1  1  1  1  1  1  1  1  1  1  1  1  1  1   <- trigger enable mask 
#                                               (channel includes in global trigger, if bit set to 1)

FADC250_TET       110        <- board Trigger Energy Threshold (TET), same for all 16 channels
FADC250_CH_TET    0    110   <- channel# and TET_value for this channel
FADC250_ALLCH_TET 111  222  2  3  4  5  6  7  8  9  10  11  12  13  14  15   <- 16 TETs (0x12C - 0x148)

FADC250_DAC       3300       <- board DAC, one and the same for all 16 channels
FADC250_CH_DAC    0    3300  <- channel# and DAC_value for this channel
FADC250_ALLCH_DAC 3300 3280 3310 3280 3310 3280 3310 3280 3300 3280 3300 3280 3310 3280 3310 3280 <- 16 DACs

FADC250_PED       210        <- board Pedestals, same for all channels
FADC250_CH_PED    0    210   <- channel# and Pedestal_value for this channel
FADC250_ALLCH_PED 210  220  210  215  215  220  220  210  210  215  215  220  220  210  215  220  <- 16 PEDs



 cc -rdynamic -shared -o fadc250Config.so fadc250Config.c -I/home/halld/test_setup/coda/linuxvme/include /home/halld/test_setup/coda/linuxvme/jvme/libjvme.a /home/halld/test_setup/coda/linuxvme/fadcV2/libfadc.a -ldl -lpthread -lrt

 */

#if defined(VXWORKS) || defined(Linux_vme)

#include <stdio.h>
#include <string.h>

#include "fadc250Config.h"
#include "fadcLib.h"

static int          nfadc;                        /* Number of FADC250s */
static FADC250_CONF fa250[NBOARD+1];


#define SCAN_MSK \
	args = sscanf (str_tmp, "%*s %d %d %d %d %d %d %d %d   \
                                     %d %d %d %d %d %d %d %d", \
		       &msk[ 0], &msk[ 1], &msk[ 2], &msk[ 3], \
		       &msk[ 4], &msk[ 5], &msk[ 6], &msk[ 7], \
		       &msk[ 8], &msk[ 9], &msk[10], &msk[11], \
		       &msk[12], &msk[13], &msk[14], &msk[15])

#define GET_READ_MSK \
	SCAN_MSK; \
	ui1 = 0; \
	for(jj=0; jj<NCHAN; jj++) \
	{ \
	  if((msk[jj] < 0) || (msk[jj] > 1)) \
	  { \
	    printf("\nReadConfigFile: Wrong mask bit value, %d\n\n",msk[jj]); return(-6); \
	  } \
	  ui1 |= (msk[jj]<<jj); \
	}


int
fadc250Config(char *fname)
{
  int res;

  /* set defaults */
  fadc250InitGlobals();

  /* faInit() must be called by now; get the number of boards from there */
  nfadc = faGetNfadc();
  printf("dsc2Config: nfadc=%d\n",nfadc);

  /* reading and parsing config file */
  if( (res = fadc250ReadConfigFile(fname)) < 0 ) return(res);

  /* download to all boards */
  fadc250DownloadAll();

  return(0);
}

void
fadc250InitGlobals()
{
  int ii, jj;

  nfadc = 0;

  for(jj=0; jj<NBOARD; jj++)
  {
    fa250[jj].mode      = 1;
    fa250[jj].winOffset = 300;
    fa250[jj].winWidth  = 100;
    fa250[jj].nsa       = 6;
    fa250[jj].nsb       = 3;
    fa250[jj].npeak     = 1;
    fa250[jj].chDisMask = 0x0;
    fa250[jj].trigMask  = 0xffff;

    for(ii=0; ii<NCHAN; ii++)
    {
      fa250[jj].thr[ii] = 110;
      fa250[jj].dac[ii] = 3300;
      fa250[jj].ped[ii] = 210;
    }
  }
}


/* reading and parsing config file */
int
fadc250ReadConfigFile(char *filename)
{
  FILE   *fd;
  char   fname[FNLEN] = { "" };  /* config file name */
  int    ii, jj, ch;
  char   str_tmp[STRLEN], str2[STRLEN], keyword[ROCLEN];
  char   host[ROCLEN], ROC_name[ROCLEN];
  int    args, i1, i2, msk[16];
  int    slot, slot1, slot2, chan;
  unsigned int  ui1, ui2;
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
      sprintf(fname, "%s/fadc250/%s", clonparms, filename);
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
    sprintf(fname, "%s/fadc250/%s.cnf", clonparms, host);
    if((fd=fopen(fname,"r")) == NULL)
    {
      sprintf(fname, "%s/fadc250/%s.cnf", clonparms, expid);
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

      /* Start parsing real config inputs */
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

      else if( (strcmp(keyword,"FADC250_SLOT")==0) || (strcmp(keyword,"FADC250_SLOTS")==0) )
      {
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


      else if(strcmp(keyword,"FADC250_MODE") == 0)
      {
        sscanf (str_tmp, "%*s %d", &i1);
	    for(slot=slot1; slot<slot2; slot++) fa250[slot].mode = i1;
      }

      else if(strcmp(keyword,"FADC250_W_OFFSET") == 0)
      {
        sscanf (str_tmp, "%*s %d", &i1);
	    for(slot=slot1; slot<slot2; slot++) fa250[slot].winOffset = i1/4;
      }

      else if(strcmp(keyword,"FADC250_W_WIDTH") == 0)
      {
        sscanf (str_tmp, "%*s %d", &i1);
	    for(slot=slot1; slot<slot2; slot++) fa250[slot].winWidth = i1/4;
      }

      else if(strcmp(keyword,"FADC250_NSA") == 0)
      {
        sscanf (str_tmp, "%*s %d", &i1);
	    for(slot=slot1; slot<slot2; slot++) fa250[slot].nsa = i1/4;
      }

      else if(strcmp(keyword,"FADC250_NSB") == 0)
      {
        sscanf (str_tmp, "%*s %d", &i1);
	    for(slot=slot1; slot<slot2; slot++) fa250[slot].nsb = i1/4;
      }

      else if(strcmp(keyword,"FADC250_NPEAK") == 0)
      {
        sscanf (str_tmp, "%*s %d", &i1);
	    for(slot=slot1; slot<slot2; slot++) fa250[slot].npeak = i1;
      }

      else if(strcmp(keyword,"FADC250_DIS_MASK") == 0)
      {
	    GET_READ_MSK;
	    for(slot=slot1; slot<slot2; slot++) fa250[slot].chDisMask = ui1;
	    printf("\nReadConfigFile: %s = 0x%04x \n",keyword,ui1);
      }

      else if(strcmp(keyword,"FADC250_TRG_MASK") == 0)
      {
	    GET_READ_MSK;
	    for(slot=slot1; slot<slot2; slot++) fa250[slot].trigMask = ui1;
	    printf("\nReadConfigFile: %s = 0x%04x \n",keyword,ui1);
      }





      else if(strcmp(keyword,"FADC250_TET") == 0)
      {
        sscanf (str_tmp, "%*s %d", &ui1);
	    for(slot=slot1; slot<slot2; slot++) for(ii=0; ii<NCHAN; ii++) fa250[slot].thr[ii] = ui1;
      }

      else if(strcmp(keyword,"FADC250_CH_TET") == 0)
      {
        sscanf (str_tmp, "%*s %d %d", &chan, &ui1);
        if((chan<0) || (chan>NCHAN))
        {
	      printf("\nReadConfigFile: Wrong channel number %d, %s\n",chan,str_tmp);
	      return(-7);
        }
	    for(slot=slot1; slot<slot2; slot++) fa250[slot].thr[chan] = ui1;
      }

      else if(strcmp(keyword,"FADC250_ALLCH_TET") == 0)
      {
	    SCAN_MSK;
	    if(args != 16)
        {
	      printf("\nReadConfigFile: Wrong argument's number %d, should be 16\n\n",args);
          return(-8);
        }
	    for(slot=slot1; slot<slot2; slot++) for(ii=0; ii<NCHAN; ii++) fa250[slot].thr[ii] = msk[ii];
      }





      else if(strcmp(keyword,"FADC250_DAC") == 0)
      {
        sscanf (str_tmp, "%*s %d", &ui1);
	    for(slot=slot1; slot<slot2; slot++) for(ii=0; ii<NCHAN; ii++) fa250[slot].dac[ii] = ui1;
      }

      else if(strcmp(keyword,"FADC250_CH_DAC") == 0)
      {
        sscanf (str_tmp, "%*s %d %d", &chan, &ui1);
        if((chan<0) || (chan>NCHAN))
        {
	      printf("\nReadConfigFile: Wrong channel number %d, %s\n",chan,str_tmp);
	      return(-7);
        }
	    for(slot=slot1; slot<slot2; slot++) fa250[slot].dac[chan] = ui1;
      }

      else if(strcmp(keyword,"FADC250_ALLCH_DAC") == 0)
      {
	    SCAN_MSK;
	    if(args != 16)
        {
	      printf("\nReadConfigFile: Wrong argument's number %d, should be 16\n\n",args);
          return(-8);
        }
	    for(slot=slot1; slot<slot2; slot++) for(ii=0; ii<NCHAN; ii++) fa250[slot].dac[ii] = msk[ii];
      }



      else if(strcmp(keyword,"FADC250_PED") == 0)
      {
        sscanf (str_tmp, "%*s %d", &ui1);
	    for(slot=slot1; slot<slot2; slot++) for(ii=0; ii<NCHAN; ii++) fa250[slot].ped[ii] = ui1;
      }

      else if(strcmp(keyword,"FADC250_CH_PED") == 0)
      {
        sscanf (str_tmp, "%*s %d %d", &chan, &ui1);
        if((chan<0) || (chan>NCHAN))
        {
	      printf("\nReadConfigFile: Wrong channel number %d, %s\n",chan,str_tmp);
	      return(-7);
        }
	    for(slot=slot1; slot<slot2; slot++) fa250[slot].ped[chan] = ui1;
      }

      else if(strcmp(keyword,"FADC250_ALLCH_PED") == 0)
      {
	    SCAN_MSK;
	    if(args != 16)
        {
	      printf("\nReadConfigFile: Wrong argument's number %d, should be 16\n\n",args);
          return(-8);
        }
	    for(slot=slot1; slot<slot2; slot++) for(ii=0; ii<NCHAN; ii++) fa250[slot].ped[ii] = msk[ii];
      }

    }
  }
  fclose(fd);

  return(0);
}


/* download setting into all found FADCs */
int
fadc250DownloadAll()
{
  int slot, ii, jj;

  for(jj=0; jj<nfadc; jj++)
  {
    slot = faSlot(jj);

    printf("\nfadc250DownloadAll: FA_SLOT=%d\n",slot);

    faSetProcMode(slot,
		    fa250[jj].mode,
		    fa250[jj].winOffset,
		    fa250[jj].winWidth,
		    fa250[jj].nsb,
		    fa250[jj].nsa,
		    fa250[jj].npeak, 0);
    faChanDisable(slot, fa250[jj].chDisMask);
    for(ii=0; ii<NCHAN; ii++)
    {
	  faSetDAC(slot, fa250[jj].dac[ii], (1<<ii));
	  faSetThreshold(slot, fa250[jj].thr[ii],(1<<ii));
      faSetChannelPedestal(slot, ii, fa250[jj].ped[ii]);
    }
  }

  return(0);
}



/* print board registers; if slot is zero, print all boards */
void
fadc250Mon(int slot)
{
  /*
  int sl, sl2;
  int f_rev;
  int b_rev;
  int b_ID;
  int          mode;
  unsigned int winOffset;
  unsigned int winWidth;
  unsigned int nsb;
  unsigned int nsa;
  unsigned int npeak;
  unsigned int chDisMask;
  unsigned int trigMask;
  unsigned int adcConf[3];

  char SerNum[80];
  */
  int id, start, end, kk, jj;


  nfadc = faGetNfadc();
  if(slot==0)
  {
    start = 0;
    end = nfadc;
  }
  else if((id = faId(slot)) >= 0)
  {
    start = id;
    end = start + 1;
  }
  else
  {
    return;
  }


  printf("nfadc=%d\n",nfadc);
  for(kk=start; kk<end; kk++)
  {
    faStatus(faSlot(kk),0);
	/*
    slot = faSlot(kk);
    printf("\nFADC number %d in slot %d\n",kk,slot);

    printf("   Mode %d, Window offset=%d ns, Window width=%d ns, NSA=%d, NSB=%d, Npeaks=%d\n",
	*/

	/*
    faChanDisable(slot, fa250[jj].chDisMask);

    for(ii=0; ii<NCHAN; ii++)
    {
	  faSetDAC(             slot, fa250[jj].dac[ii], (1<<ii));
	  faSetThreshold(       slot, fa250[jj].thr[ii],(1<<ii));
      faSetChannelPedestal( slot, ii, fa250[jj].ped[ii]);
    }
	*/



  }

  return;
}


#else /* dummy version*/

void
fadc250Config_dummy()
{
  return;
}

#endif
