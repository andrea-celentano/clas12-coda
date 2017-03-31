/****************************************************************************
 *
 *  sspConfig.c  -  configuration library file for SSP board 
 *
 *  SP, 07-Nov-2013
 * Sergey Boyarinov Nov 2013 - simplify/adjust for Hall B
 *
 *  empty lines and line startes with # - will be ignored
 *  config file format:
 * 
 *
#
# ssp config file
#
# this file contains settings for SSP
#
# format:
# ~~~~~~~
# SSP_CRATE             rocbcal1   <- ROC/crate name, usually IP name
#
# SSP_SLOTS all            <- just keyword - all settings after this line will be implemented
#                                                for all slots, till SSP_SLOTS will be met
#
# SSP_SLOTS     3  8  19   <- slot_numbers - in which next settings will be implemented
#                                                till file ends or next SSP_SLOTS will be met
#
# SSP_BERR  1   <- DMA Nwords method readout: 1 - enable VME BUS error
#                                             0 - disable VME BUS error
#
# SSP_BLOCK_LEVEL 1      <- to set block level
#
# SSP_W_WIDTH    N
#    N: 0 to 1023 - Trigger Window Width, in 4ns ticks
#
# SSP_W_OFFSET   N
#    N: 0 to 1023 - Trigger Window Offset, in 4ns ticks
#
# SSP_HPS_LATENCY  LATENCY
#    LATENCY: 0 to 511 - sets trigger latency (in 4ns ticks)
#
# SSP_HPS_SINGLES:
# ----------------
# SSP_HPS_SINGLES_EMIN  N  EMIN   EN
#    N:    0 or 1 - selects the hps singles trigger bit
#    EMIN: 0 to 8191MeV - cluster energies below this threshold are cut
#    EN:   1 - enables, 0 - disables
#
# SSP_HPS_SINGLES_EMAX  N  EMAX   EN
#    N:    0 or 1 - selects the hps singles trigger bit
#    EMAX: 0 to 8191MeV - cluster energies above this threshold are cut
#    EN:   1 - enables, 0 - disables
#
# SSP_HPS_SINGLES_NMIN  N  NMIN   EN
#    N:    0 or 1 - selects the hps singles trigger bit
#    NMIN: 0 to 9 - cluster with #hits below this threshold are cut
#    EN:   1 - enables, 0 - disables
#
# SSP_HPS_SINGLES_PRESCALE N REGION XMIN XMAX PRESCALE
#    N:        0 or 1 - selects the hps singles trigger bit
#    REGION:   0 to 6 - selects which region index to define
#    XMIN:    -31 to 31  - chooses the ECAL x coordinate to start region definition (inclusive)
#    XMAX:    -31 to 31  - chooses the ECAL x coordinate to end region definition  (inclusive)
#    PRESCALE: 0 to 65535 - set the prescale value for the region
#
#    Notes:
#         1) To disable prescaling, set a region to cover the whole ECAL (i.e. xmin = -31, max = 31) and use PRESCALE = 0
#         2) When using prescaling, define regions to be mutually exclusive to make the actual prescaling simple to understand
#         3) The prescalers for all regions are OR'd together to form the actual singles trigger bit
#         4) By default (if not defining anything) Region 0 is defined as the whole ECAL with prescaling disabled
#            and Regions 1-6 are defined outside the ECAL range (i.e. disabled)
#
# SSP_HPS_COSMICS:
# ----------------
# SSP_HPS_COSMIC_TIMECOINCIDENCE  T
#    T:    0 to 255 - sets cosmic hit coincidence time (in 4ns ticks)
#
# SSP_HPS_COSMIC_PATTERNCOINCIDENCE  pattern
#    pattern  - 3:1 LUT definition for scintillator coincidence pattern that is accepted/rejected
#                Scintillator channels are the last 3 channels (14-16) of FADC in slot 20
#                pattern = 254 will trigger on any hit channels
#                pattern = 128 will trigger when all 3 channels hit
#                pattern = 136 will trigger when channels 14&15 are hit
#                pattern = 232 will trigger when any 2 channels hit
#
# SSP_HPS_PAIRS:
# ----------------
# SSP_HPS_PAIRS_TIMECOINCIDENCE  N   T
#    N:    0 or 1 - selects the hps pair trigger bit
#    T:    0 to 15 - sets cluster pair hit coincidence time (in +/-4ns ticks)
#
# SSP_HPS_PAIRS_EMIN  N   EMIN
#    N:    0 or 1 - selects the hps pair trigger bit
#    EMIN: 0 to 8191MeV - clusters energies below this threshold are cut
#
# SSP_HPS_PAIRS_EMAX  N   EMAX
#    N:    0 or 1 - selects the hps pair trigger bit
#    EMAX: 0 to 8191MeV - clusters energies above this threshold are cut
#
# SSP_HPS_PAIRS_NMIN  N   NMIN
#    N:    0 or 1 - selects the hps pair trigger bit
#    NMIN: 0 to 9 - cluster with #hits below this threshold are cut
#
# SSP_HPS_PAIRS_SUMMAX  N   MAX   EN
#    N:    0 or 1 - selects the hps pair trigger bit
#    MAX:  0 to 8191MeV - cluster pairs energy sums above this threshold are cut
#    EN:   1 - enables, 0 - disables
#
# SSP_HPS_PAIRS_DIFFMAX  N  MAX   EN 
#    N:    0 or 1 - selects the hps pair trigger bit
#    MAX:  0 to 8191MeV - cluster pairs energy differences above this threshold are cut
#    EN:   1 - enables, 0 - disables
#
# SSP_HPS_PAIRS_COPLANARITY  N   ANGLE  EN
#    N:    0 or 1 - selects the hps pair trigger bit
#    ANGLE: 0 to 180 - cluster pair coplanarity with photon beam must be < +/-ANGLE (in degress)
#    EN:   1 - enables, 0 - disables
#
# SSP_HPS_PAIRS_ENERGYDIST  N   F  MIN  EN
#    N:    0 or 1 - selects the hps pair trigger bit
#    F:    0 to 8191 - conversion factor in MeV/mm to compensate cluster energy by radial distance from photon beam
#    MIN:  0 to 8191 - minimum threshold in MeV to accept cluster pair.
#          For each pair, the cluster with the smallest energy, Emin, is used in this cut.
#          That cluster's minimum distance to photon beam is computed, R, and tested in the equation:
#              MIN < Emin + F * R
#          When the above is true the cluster pair passes this cut, otherwise it is cut.
#    EN:   1 - enables, 0 - disables
#
# SSP_HPS_PULSER   F
#    F:     Pulser frequency (0 to 125MHz)
#
# SSP_HPS_SET_IO_SRC   OUTPUT_PIN     SIGNAL_SEL
#    OUTPUT_PIN selection:
#       LVDSOUT0        0
#       LVDSOUT1        1
#       LVDSOUT2        2
#       LVDSOUT3        3
#       LVDSOUT4        4
#       GPIO0           5
#       GPIO1           6
#       P2_LVDSOUT0     7
#       P2_LVDSOUT1     8
#       P2_LVDSOUT2     9
#       P2_LVDSOUT3     10
#       P2_LVDSOUT4     11
#       P2_LVDSOUT5     12
#       P2_LVDSOUT6     13
#       P2_LVDSOUT7     14
#       TRIG            15
#       SYNC            16
#
#    SIGNAL_SEL selection:
#       0          0
#       1          1
#       SYNC       2
#       TRIG1      3
#       TRIG2      4
#       LVDSIN0    5
#       LVDSIN1    6
#       LVDSIN2    7
#       LVDSIN3    8
#       LVDSIN4    9
#       P2LVDSIN0  10
#       P2LVDSIN1  11
#       P2LVDSIN2  12
#       P2LVDSIN3  13
#       P2LVDSIN4  14
#       P2LVDSIN5  15
#       P2LVDSIN6  16
#       P2LVDSIN7  17
#       PULSER     18
#       BUSY       19
#       TRIGGER0   20   (HPS SINGLES 0)
#       TRIGGER1   21   (HPS SINGLES 1)
#       TRIGGER2   22   (HPS PAIRS 0)
#       TRIGGER3   23   (HPS PAIRS 1)
#       TRIGGER4   24   (HPS LED)
#       TRIGGER5   25   (HPS COSMIC)
#       TRIGGER6   26
#       TRIGGER7   27
*/

#if defined(VXWORKS) || defined(Linux_vme)

#include <stdio.h>
#include <string.h>

#include "sspConfig.h"
#include "sspLib.h"
#include "xxxConfig.h"

#undef DEBUG
#define DEBUG

static int active;

static int          nssp;                        /* Number of SSPs */
static SSP_CONF ssp[NBOARD+1];

#define NCHAN 8

#define SCAN_MSK \
	args = sscanf (str_tmp, "%*s %d %d %d %d %d %d %d %d   \
                                     %d %d %d %d %d %d %d %d", \
		       &msk[ 0], &msk[ 1], &msk[ 2], &msk[ 3], \
		       &msk[ 4], &msk[ 5], &msk[ 6], &msk[ 7], \
		       &msk[ 8], &msk[ 9], &msk[10], &msk[11], \
		       &msk[12], &msk[13], &msk[14], &msk[15])

static char *expid = NULL;

void
sspSetExpid(char *string)
{
  expid = strdup(string);
}

int
sspConfig(char *fname)
{
  int res;
  char string[10]; /*dummy, will not be used*/

  /* faInit() must be called by now; get the number of boards from there */
  nssp = sspGetNssp();
  printf("sspConfig: nssp=%d\n",nssp);

  if(strlen(fname) > 0) /* filename specified  - upload initial settings from the hardware */
  {
    printf("sspConfig: fname >%s< specified\n",fname);
    sspUploadAll(string, 0);
  }
  else /* filename not specified  - set defaults */
  {
    printf("sspConfig: fname not specified - load defaults\n");
    sspInitGlobals();
  }

  /* reading and parsing config file */
  if( (res = sspReadConfigFile(fname)) < 0 )
  {
    printf("sspConfig: ERROR: sspReadConfigFile() returns %d\n",res);
    return(res);
  }

  /* download to all boards */
  sspDownloadAll();

  return(0);
}

void
sspInitGlobals()
{
  int ii, jj, kk;

  printf("sspInitGlobals reached\n");

  nssp = sspGetNssp();
  for(jj=0; jj<NBOARD; jj++)
  {
    ssp[jj].fw_rev = 0;
    ssp[jj].fw_type = 0;
    
    ssp[jj].block_level = 1;
    ssp[jj].bus_error = 1;
    ssp[jj].window_width = 100;
    ssp[jj].window_offset = 825;
    ssp[jj].pulser_freq = 0;
    ssp[jj].hps.trigger_latency = 500;

    for(ii=0; ii<SD_SRC_NUM; ii++)
        ssp[jj].ssp_io_mux[ii] = SD_SRC_SEL_0;
    
    ssp[jj].ssp_io_mux[SD_SRC_TRIG] = SD_SRC_SEL_TRIG1;
    ssp[jj].ssp_io_mux[SD_SRC_SYNC] = SD_SRC_SEL_SYNC;

    for(ii=0; ii<2; ii++)
    {
      // HPS singles
      ssp[jj].hps.s[ii].emin = 0;
      ssp[jj].hps.s[ii].emax = 8191;
      ssp[jj].hps.s[ii].nmin = 0;
      ssp[jj].hps.s[ii].emin_en = 1;
      ssp[jj].hps.s[ii].emax_en = 1;
      ssp[jj].hps.s[ii].nmin_en = 1;
      
      // HPS singles: enable region 0 to cover whole ECAL with no prescaling
      ssp[jj].hps.s[ii].prescale_xmin[0] = -31;
      ssp[jj].hps.s[ii].prescale_xmax[0] = 31;
      ssp[jj].hps.s[ii].prescale[0] = 0;
      // HPS singles: disable regions 1-6
      for(kk=1; kk<7; kk++)
      {
        ssp[jj].hps.s[ii].prescale_xmin[kk] = -31;
        ssp[jj].hps.s[ii].prescale_xmax[kk] = -31;
        ssp[jj].hps.s[ii].prescale[kk] = 0;
      }

      // HPS pairs
      ssp[jj].hps.p[ii].timecoincidence = 2;
      ssp[jj].hps.p[ii].emin = 0;
      ssp[jj].hps.p[ii].emax = 8191;
      ssp[jj].hps.p[ii].nmin = 0;
      ssp[jj].hps.p[ii].summax = 8191;
      ssp[jj].hps.p[ii].summin = 0;
      ssp[jj].hps.p[ii].summax_en = 1;
      ssp[jj].hps.p[ii].diffmax = 8191;
      ssp[jj].hps.p[ii].diffmax_en = 1;
      ssp[jj].hps.p[ii].coplanartolerance = 180;
      ssp[jj].hps.p[ii].coplanartolerance_en = 1;
      ssp[jj].hps.p[ii].edfactor = 1;
      ssp[jj].hps.p[ii].edmin = 0;
      ssp[jj].hps.p[ii].ed_en = 1;
    }
    
    // HPS cosmic
    ssp[jj].hps.cosmic_timecoincidence = 10;
    ssp[jj].hps.cosmic_pattern = 0xFE;
    
    // CLAS12 sector trigger
    for(ii=0; ii<4; ii++)
    {
      ssp[jj].gt.strg[ii].en = 0;
      ssp[jj].gt.strg[ii].pcal_cluster_emin_en = 0;
      ssp[jj].gt.strg[ii].pcal_cluster_emin = 0;
      ssp[jj].gt.strg[ii].pcal_cluster_width = 0;
      ssp[jj].gt.strg[ii].ecalout_cluster_emin_en = 0;
      ssp[jj].gt.strg[ii].ecalout_cluster_emin = 0;
      ssp[jj].gt.strg[ii].ecalout_cluster_width = 0;
      ssp[jj].gt.strg[ii].ecalin_cluster_emin_en = 0;
      ssp[jj].gt.strg[ii].ecalin_cluster_emin = 0;
      ssp[jj].gt.strg[ii].ecalin_cluster_width = 0;
      ssp[jj].gt.strg[ii].pcal_esum_en = 0;
      ssp[jj].gt.strg[ii].pcal_esum_emin = 0;
      ssp[jj].gt.strg[ii].pcal_esum_width = 0;
      ssp[jj].gt.strg[ii].ecal_esum_en = 0;
      ssp[jj].gt.strg[ii].ecal_esum_emin = 0;
      ssp[jj].gt.strg[ii].ecal_esum_width = 0;
    }
    ssp[jj].gt.ecal.esum_delay = 0;
    ssp[jj].gt.ecal.cluster_delay = 0;
    ssp[jj].gt.ecal.esum_intwidth = 0;
    ssp[jj].gt.pcal.esum_delay = 0;
    ssp[jj].gt.pcal.cluster_delay = 0;
    ssp[jj].gt.pcal.esum_intwidth = 0;
    ssp[jj].gt.gtpif_latency = 0;
  }
}


/* reading and parsing config file */
int
sspReadConfigFile(char *filename_in)
{
  FILE   *fd;
  char   filename[FNLEN];
  char   fname[FNLEN] = { "" };  /* config file name */
  int    ii, jj, ch;
  char   str_tmp[STRLEN], str2[STRLEN], keyword[ROCLEN];
  char   host[ROCLEN], ROC_name[ROCLEN];
  int    args, i1, i2, i3, i4, i5, msk[16];
  int    slot, slot1, slot2, chan;
  unsigned int  ui1, ui2;
  float f1, fmsk[16];
  char *getenv();
  char *clonparms;
  int do_parsing;

  gethostname(host,ROCLEN);  /* obtain our hostname */
  clonparms = getenv("CLON_PARMS");

  if(expid==NULL)
  {
    expid = getenv("EXPID");
    printf("\nNOTE: use EXPID=>%s< from environment\n",expid);
  }
  else
  {
    printf("\nNOTE: use EXPID=>%s< from CODA\n",expid);
  }

  strcpy(filename,filename_in); /* copy filename from parameter list to local string */
  do_parsing = 1;

  while(do_parsing)
  {
    if(strlen(filename)!=0) /* filename specified */
    {
      if ( filename[0]=='/' || (filename[0]=='.' && filename[1]=='/') )
      {
        sprintf(fname, "%s", filename);
      }
      else
      {
        sprintf(fname, "%s/ssp/%s", clonparms, filename);
      }

      if((fd=fopen(fname,"r")) == NULL)
      {
        printf("\nsspReadConfigFile: Can't open config file >%s<\n",fname);
        return(-1);
      }
    }
    else if(do_parsing<2) /* filename does not specified */
    {
      sprintf(fname, "%s/ssp/%s.cnf", clonparms, host);
      if((fd=fopen(fname,"r")) == NULL)
      {
        sprintf(fname, "%s/ssp/%s.cnf", clonparms, expid);
        if((fd=fopen(fname,"r")) == NULL)
        {
          printf("\nsspReadConfigFile: Can't open config file >%s<\n",fname);
          return(-2);
        }
      }
    }
    else
    {
      printf("\nsspReadConfigFile: ERROR: since do_parsing=%d (>1), filename must be specified\n",do_parsing);
      return(-1);
    }

    printf("\nsspnReadConfigFile: Using configuration file >%s<\n",fname);

    /* Parsing of config file */
    active = 0;
    do_parsing = 0; /* will parse only one file specified above, unless it changed during parsing */
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
        sscanf (str_tmp, "%s %s", keyword, ROC_name);
#ifdef DEBUG
        printf("\nfgets returns %s so keyword=%s\n\n",str_tmp,keyword);
#endif
        /* Start parsing real config inputs */
        if(strcmp(keyword,"SSP_CRATE") == 0)
        {
          if(strcmp(ROC_name,host) == 0)
          {
            printf("\nReadConfigFile: crate = %s  host = %s - activated\n",ROC_name,host);
            active = 1;
          }
          else if(strcmp(ROC_name,"all") == 0)
          {
            printf("\nReadConfigFile: crate = %s  host = %s - activated\n",ROC_name,host);
            active = 1;
          }
          else
          {
            printf("\nReadConfigFile: crate = %s  host = %s - disactivated\n",ROC_name,host);
            active = 0;
          }
        }
        else if(active && ((strcmp(keyword,"SSP_SLOT")==0) || (strcmp(keyword,"SSP_SLOTS")==0)) )
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
        else if(active && (strcmp(keyword,"SSP_BERR")==0))
        {
          sscanf (str_tmp, "%*s %d %d", &i1);
          if(i1!=0 && i1!=1)
          {
            printf("\nsspReadConfigFile: Wrong DMA bus error %d, %s\n",i1,str_tmp);
            return(-8);
          }
          for(slot=slot1; slot<slot2; slot++) ssp[slot].bus_error = i1;
        }
        else if(active && (strcmp(keyword,"SSP_BLOCK_LEVEL")==0))
        {
          sscanf (str_tmp, "%*s %d %d", &i1);
          for(slot=slot1; slot<slot2; slot++) ssp[slot].block_level= i1;
        }
        else if(active && (strcmp(keyword,"SSP_W_WIDTH")==0))
        {
          sscanf (str_tmp, "%*s %d %d", &i1);
          for(slot=slot1; slot<slot2; slot++) ssp[slot].window_width= i1;
        }
        else if(active && (strcmp(keyword,"SSP_W_OFFSET")==0))
        {
          sscanf (str_tmp, "%*s %d %d", &i1);
          for(slot=slot1; slot<slot2; slot++) ssp[slot].window_offset= i1;
        }
        else if(active && (strcmp(keyword,"SSP_HPS_SINGLES_EMIN")==0))
        {
          sscanf (str_tmp, "%*s %d %d %d", &i1, &i2, &i3);      
          if((i1 < 0) || (i1 > 1))
          {
            printf("\nsspReadConfigFile: Wrong index number %d, %s\n",slot,str_tmp);
            return(-4);
          }
          for(slot=slot1; slot<slot2; slot++)
          {
            ssp[slot].hps.s[i1].emin = i2;
            ssp[slot].hps.s[i1].emin_en = i3;
          }
        }
        else if(active && (strcmp(keyword,"SSP_HPS_SINGLES_EMAX")==0))
        {
          sscanf (str_tmp, "%*s %d %d %d", &i1, &i2, &i3);      
          if((i1 < 0) || (i1 > 1))
          {
            printf("\nsspReadConfigFile: Wrong index number %d, %s\n",slot,str_tmp);
            return(-4);
          }
          for(slot=slot1; slot<slot2; slot++)
          {
            ssp[slot].hps.s[i1].emax = i2;
            ssp[slot].hps.s[i1].emax_en = i3;
          }
        }
        else if(active && (strcmp(keyword,"SSP_HPS_SINGLES_NMIN")==0))
        {
          sscanf (str_tmp, "%*s %d %d %d", &i1, &i2, &i3);      
          if((i1 < 0) || (i1 > 1))
          {
            printf("\nsspReadConfigFile: Wrong index number %d, %s\n",slot,str_tmp);
            return(-4);
          }
          for(slot=slot1; slot<slot2; slot++)
          {
            ssp[slot].hps.s[i1].nmin = i2;
            ssp[slot].hps.s[i1].nmin_en = i3;
          }
        }
        else if(active && (strcmp(keyword, "SSP_HPS_SINGLES_PRESCALE")==0))
        {
          sscanf (str_tmp, "%*s %d %d %d %d %d", &i1, &i2, &i3, &i4, &i5);
          if((i1 < 0) || (i1 > 1))
          {
            printf("\nsspReadConfigFile: Wrong index number %d, %s\n",slot,str_tmp);
            return(-4);
          }
          if((i2 < 0) || (i2 > 6))
          {
            printf("\nsspReadConfigFile: Wrong region number %d, %s\n",slot,str_tmp);
            return(-4);
          }
          if((i3 < -31) || (i3 > 31))
          {
            printf("\nsspReadConfigFile: Wrong xmin number %d, %s\n",slot,str_tmp);
            return(-4);
          }
          if((i4 < -31) || (i4 > 31))
          {
            printf("\nsspReadConfigFile: Wrong xmax number %d, %s\n",slot,str_tmp);
            return(-4);
          }
          if((i5 < 0) || (i5 > 65535))
          {
            printf("\nsspReadConfigFile: Wrong prescale number %d, %s\n",slot,str_tmp);
            return(-4);
          }
          for(slot=slot1; slot<slot2; slot++)
          {
            ssp[slot].hps.s[i1].prescale_xmin[i2] = i3;
            ssp[slot].hps.s[i1].prescale_xmax[i2] = i4;
            ssp[slot].hps.s[i1].prescale[i2] = i5;
          }
        }
        else if(active && (strcmp(keyword,"SSP_HPS_LATENCY")==0))
        {
          sscanf (str_tmp, "%*s %d", &i1);
          for(slot=slot1; slot<slot2; slot++)   ssp[slot].hps.trigger_latency = i1;
        }
        else if(active && (strcmp(keyword,"SSP_HPS_COSMIC_TIMECOINCIDENCE")==0))
        {
          sscanf (str_tmp, "%*s %d", &i1);
          for(slot=slot1; slot<slot2; slot++)   ssp[slot].hps.cosmic_timecoincidence = i1;
        }
        else if(active && (strcmp(keyword,"SSP_HPS_COSMIC_PATTERNCOINCIDENCE")==0))
        {
          sscanf (str_tmp, "%*s %d", &i1);
          for(slot=slot1; slot<slot2; slot++)   ssp[slot].hps.cosmic_pattern = i1;
        }
        else if(active && (strcmp(keyword,"SSP_HPS_PAIRS_TIMECOINCIDENCE")==0))
        {
          sscanf (str_tmp, "%*s %d %d", &i1, &i2);
          if((i1 < 0) || (i1 > 1))
          {
            printf("\nsspReadConfigFile: Wrong index number %d, %s\n",slot,str_tmp);
            return(-4);
          }
          for(slot=slot1; slot<slot2; slot++)   ssp[slot].hps.p[i1].timecoincidence = i2;
        }
        else if(active && (strcmp(keyword,"SSP_HPS_PAIRS_EMIN")==0))
        {
          sscanf (str_tmp, "%*s %d %d", &i1, &i2);      
          if((i1 < 0) || (i1 > 1))
          {
            printf("\nsspReadConfigFile: Wrong index number %d, %s\n",slot,str_tmp);
            return(-4);
          }
          for(slot=slot1; slot<slot2; slot++)   ssp[slot].hps.p[i1].emin = i2;
        }
        else if(active && (strcmp(keyword,"SSP_HPS_PAIRS_EMAX")==0))
        {
          sscanf (str_tmp, "%*s %d %d %d", &i1, &i2);     
          if((i1 < 0) || (i1 > 1))
          {
            printf("\nsspReadConfigFile: Wrong index number %d, %s\n",slot,str_tmp);
            return(-4);
          }
          for(slot=slot1; slot<slot2; slot++)   ssp[slot].hps.p[i1].emax = i2;
        }
        else if(active && (strcmp(keyword,"SSP_HPS_PAIRS_NMIN")==0))
        {
          sscanf (str_tmp, "%*s %d %d", &i1, &i2);      
          if((i1 < 0) || (i1 > 1))
          {
            printf("\nsspReadConfigFile: Wrong index number %d, %s\n",slot,str_tmp);
            return(-4);
          }
          for(slot=slot1; slot<slot2; slot++)   ssp[slot].hps.p[i1].nmin = i2;
        }
        else if(active && (strcmp(keyword,"SSP_HPS_PAIRS_SUMMAX")==0))
        {
          sscanf (str_tmp, "%*s %d %d %d", &i1, &i2, &i3);
          if((i1 < 0) || (i1 > 1))
          {
            printf("\nsspReadConfigFile: Wrong index number %d, %s\n",slot,str_tmp);
            return(-4);
          }
          for(slot=slot1; slot<slot2; slot++)
          {
            ssp[slot].hps.p[i1].summax = i2;
            ssp[slot].hps.p[i1].summin = 0;
            ssp[slot].hps.p[i1].summax_en = i3;
          }
        }
        else if(active && (strcmp(keyword,"SSP_HPS_PAIRS_SUMMAX_MIN")==0))
        {
          sscanf (str_tmp, "%*s %d %d %d %d", &i1, &i2, &i3, &i4);
          if((i1 < 0) || (i1 > 1))
          {
            printf("\nsspReadConfigFile: Wrong index number %d, %s\n",slot,str_tmp);
            return(-4);
          }
          for(slot=slot1; slot<slot2; slot++)
          {
            ssp[slot].hps.p[i1].summax = i2;
            ssp[slot].hps.p[i1].summin = i3;
            ssp[slot].hps.p[i1].summax_en = i4;
          }
        }
        else if(active && (strcmp(keyword,"SSP_HPS_PAIRS_DIFFMAX")==0))
        {
          sscanf (str_tmp, "%*s %d %d %d", &i1, &i2, &i3);
          if((i1 < 0) || (i1 > 1))
          {
            printf("\nsspReadConfigFile: Wrong index number %d, %s\n",slot,str_tmp);
            return(-4);
          }
          for(slot=slot1; slot<slot2; slot++)
          {
            ssp[slot].hps.p[i1].diffmax = i2;
            ssp[slot].hps.p[i1].diffmax_en = i3;
          }
        }
        else if(active && (strcmp(keyword,"SSP_HPS_PAIRS_COPLANARITY")==0))
        {
          sscanf (str_tmp, "%*s %d %d %d", &i1, &i2, &i3);
          if((i1 < 0) || (i1 > 1))
          {
            printf("\nsspReadConfigFile: Wrong index number %d, %s\n",slot,str_tmp);
            return(-4);
          }
          for(slot=slot1; slot<slot2; slot++)
          {
            ssp[slot].hps.p[i1].coplanartolerance = i2;
            ssp[slot].hps.p[i1].coplanartolerance_en = i3;
          }
        }
        else if(active && (strcmp(keyword,"SSP_HPS_PAIRS_ENERGYDIST")==0))
        {
          sscanf (str_tmp, "%*s %d %f %d %d", &i1, &f1, &i3, &i4);
          if((i1 < 0) || (i1 > 1))
          {
            printf("\nsspReadConfigFile: Wrong index number %d, %s\n",slot,str_tmp);
            return(-4);
          }
          for(slot=slot1; slot<slot2; slot++)
          {
            ssp[slot].hps.p[i1].edfactor = f1;
            ssp[slot].hps.p[i1].edmin = i3;
            ssp[slot].hps.p[i1].ed_en = i4;
          }
        }
        else if(active && (strcmp(keyword,"SSP_HPS_PULSER")==0))
        {
          sscanf (str_tmp, "%*s %d", &i1);
          for(slot=slot1; slot<slot2; slot++)
		  {
            ssp[slot].pulser_freq = i1;
            /*printf("ssp[%d].pulser_freq=%d\n",slot,ssp[slot].pulser_freq);*/
		  }
        }
        else if(active && (strcmp(keyword,"SSP_HPS_SET_IO_SRC")==0))
        {
          sscanf (str_tmp, "%*s %d %d", &i1, &i2);
          if((i1 < 0) || (i1 >= SD_SRC_NUM))
          {
            printf("\nsspReadConfigFile: Wrong src number %d, %s\n",slot,str_tmp);
            return(-4);
          }
          if((i2 < 0) || (i2 >= SD_SRC_SEL_NUM))
          {
            printf("\nsspReadConfigFile: Wrong sel number %d, %s\n",slot,str_tmp);
            return(-4);
          }
          for(slot=slot1; slot<slot2; slot++) ssp[slot].ssp_io_mux[i1] = i2;
        }
        else if(active && (strcmp(keyword,"SSP_GT_LATENCY")==0))
        {        
          sscanf (str_tmp, "%*s %d", &i1);
          for(slot=slot1; slot<slot2; slot++) ssp[slot].gt.gtpif_latency = i1;
        }        
        else if(active && (strcmp(keyword,"SSP_GT_ECAL_ESUM_DELAY")==0))
        {        
          sscanf (str_tmp, "%*s %d", &i1);
          for(slot=slot1; slot<slot2; slot++) ssp[slot].gt.ecal.esum_delay = i1;
        }
        else if(active && (strcmp(keyword,"SSP_GT_ECAL_CLUSTER_DELAY")==0))
        {        
          sscanf (str_tmp, "%*s %d", &i1);
          for(slot=slot1; slot<slot2; slot++) ssp[slot].gt.ecal.cluster_delay = i1;
        }
        else if(active && (strcmp(keyword,"SSP_GT_ECAL_ESUM_INTWIDTH")==0))
        {        
          sscanf (str_tmp, "%*s %d", &i1);
          for(slot=slot1; slot<slot2; slot++) ssp[slot].gt.ecal.esum_intwidth = i1;
        }
        else if(active && (strcmp(keyword,"SSP_GT_PCAL_ESUM_DELAY")==0))
        {        
          sscanf (str_tmp, "%*s %d", &i1);
          for(slot=slot1; slot<slot2; slot++) ssp[slot].gt.pcal.esum_delay = i1;
        }
        else if(active && (strcmp(keyword,"SSP_GT_PCAL_CLUSTER_DELAY")==0))
        {        
          sscanf (str_tmp, "%*s %d", &i1);
          for(slot=slot1; slot<slot2; slot++) ssp[slot].gt.pcal.cluster_delay = i1;
        }
        else if(active && (strcmp(keyword,"SSP_GT_PCAL_ESUM_INTWIDTH")==0))
        {        
          sscanf (str_tmp, "%*s %d", &i1);
          for(slot=slot1; slot<slot2; slot++) ssp[slot].gt.pcal.esum_intwidth = i1;
        }
        else if(active && (strcmp(keyword,"SSP_GT_STRG_EN")==0))
        {        
          sscanf (str_tmp, "%*s %d %d", &i1, &i2);
          for(slot=slot1; slot<slot2; slot++) ssp[slot].gt.strg[i1].en = i2;
        }
        else if(active && (strcmp(keyword,"SSP_GT_STRG_PCAL_CLUSTER_EMIN_EN")==0))
        {        
          sscanf (str_tmp, "%*s %d %d", &i1, &i2);
          for(slot=slot1; slot<slot2; slot++) ssp[slot].gt.strg[i1].pcal_cluster_emin_en = i2;
        }
        else if(active && (strcmp(keyword,"SSP_GT_STRG_ECALOUT_CLUSTER_EMIN_EN")==0))
        {        
          sscanf (str_tmp, "%*s %d %d", &i1, &i2);
          for(slot=slot1; slot<slot2; slot++) ssp[slot].gt.strg[i1].ecalout_cluster_emin_en = i2;
        }
        else if(active && (strcmp(keyword,"SSP_GT_STRG_ECALIN_CLUSTER_EMIN_EN")==0))
        {        
          sscanf (str_tmp, "%*s %d %d", &i1, &i2);
          for(slot=slot1; slot<slot2; slot++) ssp[slot].gt.strg[i1].ecalin_cluster_emin_en = i2;
        }
        else if(active && (strcmp(keyword,"SSP_GT_STRG_PCAL_ESUM_EN")==0))
        {        
          sscanf (str_tmp, "%*s %d %d", &i1, &i2);
          for(slot=slot1; slot<slot2; slot++) ssp[slot].gt.strg[i1].pcal_esum_en = i2;
        }
        else if(active && (strcmp(keyword,"SSP_GT_STRG_ECAL_ESUM_EN")==0))
        {        
          sscanf (str_tmp, "%*s %d %d", &i1, &i2);
          for(slot=slot1; slot<slot2; slot++) ssp[slot].gt.strg[i1].ecal_esum_en = i2;
        }
        else if(active && (strcmp(keyword,"SSP_GT_STRG_ECAL_ESUM_MIN")==0))
        {        
          sscanf (str_tmp, "%*s %d %d", &i1, &i2);
          for(slot=slot1; slot<slot2; slot++) ssp[slot].gt.strg[i1].ecal_esum_emin = i2;
        }
        else if(active && (strcmp(keyword,"SSP_GT_STRG_ECAL_ESUM_WIDTH")==0))
        {        
          sscanf (str_tmp, "%*s %d %d", &i1, &i2);
          for(slot=slot1; slot<slot2; slot++) ssp[slot].gt.strg[i1].ecal_esum_width = i2;
        }
        else if(active && (strcmp(keyword,"SSP_GT_STRG_PCAL_ESUM_MIN")==0))
        {        
          sscanf (str_tmp, "%*s %d %d", &i1, &i2);
          for(slot=slot1; slot<slot2; slot++) ssp[slot].gt.strg[i1].pcal_esum_emin = i2;
        }
        else if(active && (strcmp(keyword,"SSP_GT_STRG_PCAL_ESUM_WIDTH")==0))
        {        
          sscanf (str_tmp, "%*s %d %d", &i1, &i2);
          for(slot=slot1; slot<slot2; slot++) ssp[slot].gt.strg[i1].pcal_esum_width = i2;
        }
        else if(active && (strcmp(keyword,"SSP_GT_STRG_ECALIN_CLUSTER_EMIN")==0))
        {        
          sscanf (str_tmp, "%*s %d %d", &i1, &i2);
          for(slot=slot1; slot<slot2; slot++) ssp[slot].gt.strg[i1].ecalin_cluster_emin = i2;
        }
        else if(active && (strcmp(keyword,"SSP_GT_STRG_ECALIN_CLUSTER_WIDTH")==0))
        {        
          sscanf (str_tmp, "%*s %d %d", &i1, &i2);
          for(slot=slot1; slot<slot2; slot++) ssp[slot].gt.strg[i1].ecalin_cluster_width = i2;
        }
        else if(active && (strcmp(keyword,"SSP_GT_STRG_ECALOUT_CLUSTER_EMIN")==0))
        {        
          sscanf (str_tmp, "%*s %d %d", &i1, &i2);
          for(slot=slot1; slot<slot2; slot++) ssp[slot].gt.strg[i1].ecalout_cluster_emin = i2;
        }
        else if(active && (strcmp(keyword,"SSP_GT_STRG_ECALOUT_CLUSTER_WIDTH")==0))
        {        
          sscanf (str_tmp, "%*s %d %d", &i1, &i2);
          for(slot=slot1; slot<slot2; slot++) ssp[slot].gt.strg[i1].ecalout_cluster_width = i2;
        }
        else if(active && (strcmp(keyword,"SSP_GT_STRG_PCAL_CLUSTER_EMIN")==0))
        {        
          sscanf (str_tmp, "%*s %d %d", &i1, &i2);
          for(slot=slot1; slot<slot2; slot++) ssp[slot].gt.strg[i1].pcal_cluster_emin = i2;
        }
        else if(active && (strcmp(keyword,"SSP_GT_STRG_PCAL_CLUSTER_WIDTH")==0))
        {        
          sscanf (str_tmp, "%*s %d %d", &i1, &i2);
          for(slot=slot1; slot<slot2; slot++) ssp[slot].gt.strg[i1].pcal_cluster_width = i2;
        }
        else
        {
          ; /* unknown key - do nothing */
          /*
          printf("sspReadConfigFile: Unknown Field or Missed Field in\n");
          printf("   %s \n", fname);
          printf("   str_tmp=%s", str_tmp);
          printf("   keyword=%s \n\n", keyword);
          return(-10);
          */
        }
      }
    } /* end of while */
    fclose(fd);
  }

  return(0);
}


/* download setting into all found SSPs */
int
sspDownloadAll()
{
  int slot, ii, jj, kk;

  printf("\n\nsspDownloadAll reached, nssp=%d\n",nssp);
  for(ii=0; ii<nssp; ii++)
  {
    slot = sspSlot(ii);
    
printf("ssp[%d].%s = %d\n", slot, "fw_rev", ssp[slot].fw_rev);
printf("ssp[%d].%s = %d\n", slot, "fw_type", ssp[slot].fw_type);
printf("ssp[%d].%s = %d\n", slot, "block_level", ssp[slot].block_level);
printf("ssp[%d].%s = %d\n", slot, "bus_error", ssp[slot].bus_error);
printf("ssp[%d].%s = %d\n", slot, "window_width", ssp[slot].window_width);
printf("ssp[%d].%s = %d\n", slot, "window_offset", ssp[slot].window_offset);
printf("ssp[%d].%s = %d\n", slot, "pulser_freq", ssp[slot].pulser_freq);
printf("ssp[%d].%s = %d\n", slot, "ssp_io_mux[0]", ssp[slot].ssp_io_mux[0]);
printf("ssp[%d].%s = %d\n", slot, "gt.strg[0].en", ssp[slot].gt.strg[0].en);
printf("ssp[%d].%s = %d\n", slot, "gt.strg[0].pcal_cluster_emin_en", ssp[slot].gt.strg[0].pcal_cluster_emin_en);
printf("ssp[%d].%s = %d\n", slot, "gt.strg[0].pcal_cluster_emin", ssp[slot].gt.strg[0].pcal_cluster_emin);
printf("ssp[%d].%s = %d\n", slot, "gt.strg[0].pcal_cluster_width", ssp[slot].gt.strg[0].pcal_cluster_width);
printf("ssp[%d].%s = %d\n", slot, "gt.strg[0].ecalout_cluster_emin_en", ssp[slot].gt.strg[0].ecalout_cluster_emin_en);
printf("ssp[%d].%s = %d\n", slot, "gt.strg[0].ecalout_cluster_emin", ssp[slot].gt.strg[0].ecalout_cluster_emin);
printf("ssp[%d].%s = %d\n", slot, "gt.strg[0].ecalout_cluster_width", ssp[slot].gt.strg[0].ecalout_cluster_width);
printf("ssp[%d].%s = %d\n", slot, "gt.strg[0].ecalin_cluster_emin_en", ssp[slot].gt.strg[0].ecalin_cluster_emin_en);
printf("ssp[%d].%s = %d\n", slot, "gt.strg[0].ecalin_cluster_emin", ssp[slot].gt.strg[0].ecalin_cluster_emin);
printf("ssp[%d].%s = %d\n", slot, "gt.strg[0].ecalin_cluster_width", ssp[slot].gt.strg[0].ecalin_cluster_width);
printf("ssp[%d].%s = %d\n", slot, "gt.strg[0].pcal_esum_en", ssp[slot].gt.strg[0].pcal_esum_en);
printf("ssp[%d].%s = %d\n", slot, "gt.strg[0].pcal_esum_emin", ssp[slot].gt.strg[0].pcal_esum_emin);
printf("ssp[%d].%s = %d\n", slot, "gt.strg[0].pcal_esum_width", ssp[slot].gt.strg[0].pcal_esum_width);
printf("ssp[%d].%s = %d\n", slot, "strg[0].ecal_esum_en", ssp[slot].gt.strg[0].ecal_esum_en);
printf("ssp[%d].%s = %d\n", slot, "gt.strg[0].ecal_esum_emin", ssp[slot].gt.strg[0].ecal_esum_emin);
printf("ssp[%d].%s = %d\n", slot, "gt.strg[0].ecal_esum_width", ssp[slot].gt.strg[0].ecal_esum_width);
printf("ssp[%d].%s = %d\n", slot, "gt.ecal.esum_delay", ssp[slot].gt.ecal.esum_delay);
printf("ssp[%d].%s = %d\n", slot, "gt.ecal.cluster_delay", ssp[slot].gt.ecal.cluster_delay);
printf("ssp[%d].%s = %d\n", slot, "gt.ecal.esum_intwidth", ssp[slot].gt.ecal.esum_intwidth);
printf("ssp[%d].%s = %d\n", slot, "gt.pcal.esum_delay", ssp[slot].gt.pcal.esum_delay);
printf("ssp[%d].%s = %d\n", slot, "gt.pcal.cluster_delay", ssp[slot].gt.pcal.cluster_delay);
printf("ssp[%d].%s = %d\n", slot, "gt.pcal.esum_intwidth", ssp[slot].gt.pcal.esum_intwidth);
printf("ssp[%d].%s = %d\n", slot, "gt.gtpif_latency", ssp[slot].gt.gtpif_latency);
printf("\n");
    
    /******************************************/
    /****** CONFIGURATION START: TYPE=ALL *****/
    /******************************************/

    /* the number of events per block */
    sspSetBlockLevel(slot, ssp[slot].block_level);

    /* Enable Bus Error */
    if(ssp[slot].bus_error) sspEnableBusError(slot);
    else                    sspDisableBusError(slot);

    /* window size and position */
    sspSetWindowWidth(slot, ssp[slot].window_width);
    sspSetWindowOffset(slot, ssp[slot].window_offset);

    if(ssp[slot].pulser_freq>0)
    {
      sspPulserSetup(slot, (float)ssp[slot].pulser_freq, 0.5, 0xFFFFFFFF);
      sspPulserStart(slot);
      printf("SSP PULSER STARTED, FREQ=%f\n", (float)ssp[slot].pulser_freq);
    }
    else
	{
      sspPulserSetup(slot, (float)0.0, 0, 0);
      printf("SSP PULSER STOPPED\n");
	}

    for(jj=0; jj<SD_SRC_NUM; jj++)
      sspSetIOSrc(slot, jj, ssp[slot].ssp_io_mux[jj]);    
    /******************************************/
    /****** CONFIGURATION END:   TYPE=ALL *****/
    /******************************************/

    /******************************************/
    /****** CONFIGURATION START: TYPE=GT *****/
    /******************************************/
    if(ssp[slot].fw_type == SSP_CFG_SSPTYPE_HALLBGT)
    {
      sspGt_SetLatency(slot, ssp[slot].gt.gtpif_latency);
      sspGt_SetEcal_EsumDelay(slot, ssp[slot].gt.ecal.esum_delay);
      sspGt_SetEcal_ClusterDelay(slot, ssp[slot].gt.ecal.cluster_delay);
      sspGt_SetEcal_EsumIntegrationWidth(slot, ssp[slot].gt.ecal.esum_intwidth);
      sspGt_SetPcal_EsumDelay(slot, ssp[slot].gt.pcal.esum_delay);
      sspGt_SetPcal_ClusterDelay(slot, ssp[slot].gt.pcal.cluster_delay);
      sspGt_SetPcal_EsumIntegrationWidth(slot, ssp[slot].gt.pcal.esum_intwidth);
      
      for(jj=0; jj<4; jj++)
      {
        sspGt_SetTrigger_Enable(slot, jj,
                                (ssp[slot].gt.strg[jj].en<<0) |
                                (ssp[slot].gt.strg[jj].pcal_cluster_emin_en<<1) |
                                (ssp[slot].gt.strg[jj].ecalout_cluster_emin_en<<2) |
                                (ssp[slot].gt.strg[jj].ecalin_cluster_emin_en<<3) |
                                (ssp[slot].gt.strg[jj].pcal_esum_en<<4) |
                                (ssp[slot].gt.strg[jj].ecal_esum_en<<5));
        
        sspGt_SetTrigger_EcalEsumEmin(slot, jj, ssp[slot].gt.strg[jj].ecal_esum_emin);
        sspGt_SetTrigger_EcalEsumWidth(slot, jj, ssp[slot].gt.strg[jj].ecal_esum_width);
        sspGt_SetTrigger_PcalEsumEmin(slot, jj, ssp[slot].gt.strg[jj].pcal_esum_emin);
        sspGt_SetTrigger_PcalEsumWidth(slot, jj, ssp[slot].gt.strg[jj].pcal_esum_width);
        sspGt_SetTrigger_EcalInnerClusterEmin(slot, jj, ssp[slot].gt.strg[jj].ecalin_cluster_emin);
        sspGt_SetTrigger_EcalInnerClusterWidth(slot, jj, ssp[slot].gt.strg[jj].ecalin_cluster_width);
        sspGt_SetTrigger_EcalOuterClusterEmin(slot, jj, ssp[slot].gt.strg[jj].ecalout_cluster_emin);
        sspGt_SetTrigger_EcalOuterClusterWidth(slot, jj, ssp[slot].gt.strg[jj].ecalout_cluster_width);
        sspGt_SetTrigger_PcalClusterEmin(slot, jj, ssp[slot].gt.strg[jj].pcal_cluster_emin);
        sspGt_SetTrigger_PcalClusterWidth(slot, jj, ssp[slot].gt.strg[jj].pcal_cluster_width);
      }
    }
    /******************************************/
    /****** CONFIGURATION END:   TYPE=GT *****/
    /******************************************/
 
    
    /******************************************/
    /****** CONFIGURATION START: TYPE=HPS *****/
    /******************************************/
    if(ssp[slot].fw_type == SSP_CFG_SSPTYPE_HPS)
    {
      sspHps_SetLatency(slot, ssp[slot].hps.trigger_latency);

      sspHps_SetCosmicTimeCoincidence(slot, ssp[slot].hps.cosmic_timecoincidence);
      sspHps_SetCosmicCoincidencePattern(slot, ssp[slot].hps.cosmic_pattern);

      for(jj=0; jj<2; jj++)
      {
        sspHps_SetSinglesEmin(slot, jj, ssp[slot].hps.s[jj].emin);
        sspHps_SetSinglesEmax(slot, jj, ssp[slot].hps.s[jj].emax);
        sspHps_SetSinglesNHitsmin(slot, jj, ssp[slot].hps.s[jj].nmin);
        sspHps_SetSinglesEnableEmin(slot, jj, ssp[slot].hps.s[jj].emin_en);
        sspHps_SetSinglesEnableEmax(slot, jj, ssp[slot].hps.s[jj].emax_en);
        sspHps_SetSinglesEnableNmin(slot, jj, ssp[slot].hps.s[jj].nmin_en);

        /* setup hps singles prescalers */
        for(kk=0; kk<7; kk++)
        {
          sspHps_SetSinglePrescale(slot, jj, kk, 
              ssp[slot].hps.s[jj].prescale_xmin[kk],
              ssp[slot].hps.s[jj].prescale_xmax[kk],
              ssp[slot].hps.s[jj].prescale[kk]
            );
        }

        sspHps_SetPairsEnableSum(slot, jj, ssp[slot].hps.p[jj].summax_en);
        sspHps_SetPairsEnableDiff(slot, jj, ssp[slot].hps.p[jj].diffmax_en);
        sspHps_SetPairsEnableCoplanar(slot, jj, ssp[slot].hps.p[jj].coplanartolerance_en);
        sspHps_SetPairsEnableED(slot, jj, ssp[slot].hps.p[jj].ed_en);
        sspHps_SetPairsTimeCoincidence(slot, jj, ssp[slot].hps.p[jj].timecoincidence);
        sspHps_SetPairsSummax(slot, jj, ssp[slot].hps.p[jj].summax);
        sspHps_SetPairsSummin(slot, jj, ssp[slot].hps.p[jj].summin);
        sspHps_SetPairsDiffmax(slot, jj, ssp[slot].hps.p[jj].diffmax);
        sspHps_SetPairsEmin(slot, jj, ssp[slot].hps.p[jj].emin);
        sspHps_SetPairsEmax(slot, jj, ssp[slot].hps.p[jj].emax);
        sspHps_SetPairsNHitsmin(slot, jj, ssp[slot].hps.p[jj].nmin);
        sspHps_SetPairsCoplanarTolerance(slot, jj, ssp[slot].hps.p[jj].coplanartolerance);
        sspHps_SetPairsEDFactor(slot, jj, ssp[slot].hps.p[jj].edfactor);
        sspHps_SetPairsEDmin(slot, jj, ssp[slot].hps.p[jj].edmin);
      }
    }
    /******************************************/
    /****** CONFIGURATION END:   TYPE=HPS *****/
    /******************************************/
  }

  return(0);
}

/* upload setting from all found SSPs */
int
sspUploadAll(char *string, int length)
{
  int slot, i, j, jj, kk, ifiber, len1, len2;
  char *str, sss[1024];
  unsigned int tmp, connectedfibers;
  unsigned short sval;
  unsigned short bypMask;
  unsigned short channels[8];
  unsigned int ival;

  for(kk=0; kk<nssp; kk++)
  {
    /******************************************/
    /****** CONFIGURATION START: TYPE=ALL *****/
    /******************************************/
    slot = sspSlot(kk);    
    
    ssp[slot].fw_rev = sspGetFirmwareVersion(slot);
    ssp[slot].fw_type = sspGetFirmwareType(slot);
    ssp[slot].bus_error = sspGetBusError(slot);
    ssp[slot].block_level = sspGetBlockLevel(slot);
    ssp[slot].window_width = sspGetWindowWidth(slot);
    ssp[slot].window_offset = sspGetWindowOffset(slot);
    
    ssp[slot].pulser_freq = (int)sspGetPulserFreq(slot);
    
    for(i = SD_SRC_P2_LVDSOUT0; i <= SD_SRC_P2_LVDSOUT7; i++)
    {
      ssp[slot].ssp_io_mux[i] = sspGetIOSrc(slot, i, 0);
    }
    /******************************************/
    /****** CONFIGURATION END:   TYPE=ALL *****/
    /******************************************/
    
    /******************************************/
    /****** CONFIGURATION START: TYPE=GT *****/
    /******************************************/
    if(ssp[slot].fw_type == SSP_CFG_SSPTYPE_HALLBGT)
    {
      ssp[slot].gt.gtpif_latency = sspGt_GetLatency(slot);
      ssp[slot].gt.ecal.esum_delay = sspGt_GetEcal_EsumDelay(slot);
      ssp[slot].gt.ecal.cluster_delay = sspGt_GetEcal_ClusterDelay(slot);
      ssp[slot].gt.ecal.esum_intwidth = sspGt_GetEcal_EsumIntegrationWidth(slot);
      ssp[slot].gt.pcal.esum_delay = sspGt_GetPcal_EsumDelay(slot);
      ssp[slot].gt.pcal.cluster_delay = sspGt_GetPcal_ClusterDelay(slot);
      ssp[slot].gt.pcal.esum_intwidth = sspGt_GetPcal_EsumIntegrationWidth(slot);
      
      for(jj=0; jj<4; jj++)
      {
        ival = sspGt_GetTrigger_Enable(slot, jj);
        ssp[slot].gt.strg[jj].en                      = (ival & 0x1) ? 1 : 0;
        ssp[slot].gt.strg[jj].pcal_cluster_emin_en    = (ival & 0x2) ? 1 : 0;
        ssp[slot].gt.strg[jj].ecalout_cluster_emin_en = (ival & 0x4) ? 1 : 0;
        ssp[slot].gt.strg[jj].ecalin_cluster_emin_en  = (ival & 0x8) ? 1 : 0;
        ssp[slot].gt.strg[jj].pcal_esum_en            = (ival & 0x10) ? 1 : 0;
        ssp[slot].gt.strg[jj].ecal_esum_en            = (ival & 0x20) ? 1 : 0;
        
        ssp[slot].gt.strg[jj].ecal_esum_emin = sspGt_GetTrigger_EcalEsumEmin(slot, jj);
        ssp[slot].gt.strg[jj].ecal_esum_width = sspGt_GetTrigger_EcalEsumWidth(slot, jj);
        ssp[slot].gt.strg[jj].pcal_esum_emin = sspGt_GetTrigger_PcalEsumEmin(slot, jj);
        ssp[slot].gt.strg[jj].pcal_esum_width = sspGt_GetTrigger_PcalEsumWidth(slot, jj);
        ssp[slot].gt.strg[jj].ecalin_cluster_emin = sspGt_GetTrigger_EcalInnerClusterEmin(slot, jj);
        ssp[slot].gt.strg[jj].ecalin_cluster_width = sspGt_GetTrigger_EcalInnerClusterWidth(slot, jj);
        ssp[slot].gt.strg[jj].ecalout_cluster_emin = sspGt_GetTrigger_EcalOuterClusterEmin(slot, jj);
        ssp[slot].gt.strg[jj].ecalout_cluster_width = sspGt_GetTrigger_EcalOuterClusterWidth(slot, jj);
        ssp[slot].gt.strg[jj].pcal_cluster_emin = sspGt_GetTrigger_PcalClusterEmin(slot, jj);
        ssp[slot].gt.strg[jj].pcal_cluster_width = sspGt_GetTrigger_PcalClusterWidth(slot, jj);
      }
    }
    /******************************************/
    /****** CONFIGURATION END:   TYPE=GT *****/
    /******************************************/

    /******************************************/
    /****** CONFIGURATION START: TYPE=HPS *****/
    /******************************************/
    if(ssp[slot].fw_type == SSP_CFG_SSPTYPE_HPS)
    {
      for(i=0; i<2; i++)
      {
        ssp[slot].hps.s[i].emin = sspHps_GetSinglesEmin(slot,i);
        ssp[slot].hps.s[i].emin_en = sspHps_GetSinglesEnableEmin(slot, i);
        ssp[slot].hps.s[i].emax = sspHps_GetSinglesEmax(slot,i);
        ssp[slot].hps.s[i].emax_en = sspHps_GetSinglesEnableEmax(slot, i);
        ssp[slot].hps.s[i].nmin = sspHps_GetSinglesNHitsmin(slot,i);
        ssp[slot].hps.s[i].nmin_en = sspHps_GetSinglesEnableNmin(slot, i);
      
        for(j=0; j<7;j++)
        {
          ssp[slot].hps.s[i].prescale_xmin[j] = sspHps_GetSinglePrescaleXmin(slot, i, j);
          ssp[slot].hps.s[i].prescale_xmax[j] = sspHps_GetSinglePrescaleXmax(slot, i, j);
          ssp[slot].hps.s[i].prescale[j] = sspHps_GetSinglePrescalePrescale(slot, i, j);
        }
      }

      ssp[slot].hps.trigger_latency = sspHps_GetLatency(slot);
      ssp[slot].hps.cosmic_timecoincidence = sspHps_GetCosmicTimeCoincidence(slot);
      ssp[slot].hps.cosmic_pattern = sspHps_GetCosmicCoincidencePattern(slot);

      for(i=0; i<2; i++)
      {
        ssp[slot].hps.p[i].timecoincidence = sspHps_GetPairsTimeCoincidence(slot,i);
        ssp[slot].hps.p[i].emin = sspHps_GetPairsEmin(slot,i);
        ssp[slot].hps.p[i].emax = sspHps_GetPairsEmax(slot,i);
        ssp[slot].hps.p[i].nmin = sspHps_GetPairsNHitsmin(slot,i);
        ssp[slot].hps.p[i].summax = sspHps_GetPairsSummax(slot,i);
        ssp[slot].hps.p[i].summin = sspHps_GetPairsSummin(slot,i);
        ssp[slot].hps.p[i].summax_en = sspHps_GetPairsEnableSum(slot, i);
        ssp[slot].hps.p[i].diffmax = sspHps_GetPairsDiffmax(slot,i);
        ssp[slot].hps.p[i].diffmax_en = sspHps_GetPairsEnableDiff(slot,i);
        ssp[slot].hps.p[i].coplanartolerance = sspHps_GetPairsCoplanarTolerance(slot,i);
        ssp[slot].hps.p[i].coplanartolerance_en = sspHps_GetPairsEnableCoplanar(slot,i);
        ssp[slot].hps.p[i].edfactor = sspHps_GetPairsEDFactor(slot,i);
        ssp[slot].hps.p[i].edmin = sspHps_GetPairsEDmin(slot,i);
        ssp[slot].hps.p[i].ed_en = sspHps_GetPairsEnableED(slot,i);
      }
    }
    /******************************************/
    /****** CONFIGURATION END:   TYPE=HPS *****/
    /******************************************/
  }
  
  if(length)
  {
    str = string;
    str[0] = '\0';
    for(kk=0; kk<nssp; kk++)
    {
      slot = sspSlot(kk);

      /******************************************/
      /****** CONFIGURATION START: TYPE=ALL *****/
      /******************************************/
      sprintf(sss,"SSP_SLOT %d\n",slot); ADD_TO_STRING;

      sprintf(sss,"SSP_FIRMWAREVERSION %d\n", ssp[slot].fw_rev); ADD_TO_STRING;
      sprintf(sss,"SSP_FIRMWARETYPE %d\n", ssp[slot].fw_type); ADD_TO_STRING;
      sprintf(sss,"SSP_BERR %d\n",ssp[slot].bus_error); ADD_TO_STRING;
      sprintf(sss,"SSP_BLOCK_LEVEL %d\n",ssp[slot].block_level); ADD_TO_STRING;
      sprintf(sss,"SSP_W_WIDTH %d\n",ssp[slot].window_width); ADD_TO_STRING;
      sprintf(sss,"SSP_W_OFFSET %d\n",ssp[slot].window_offset); ADD_TO_STRING;

      sprintf(sss,"SSP_HPS_PULSER %d\n", ssp[slot].pulser_freq); ADD_TO_STRING;
      for(i = SD_SRC_P2_LVDSOUT0; i <= SD_SRC_P2_LVDSOUT7; i++)
      {
        sprintf(sss,"SSP_HPS_SET_IO_SRC %d %d\n",i,ssp[slot].ssp_io_mux[i]); ADD_TO_STRING;
      }
      /******************************************/
      /****** CONFIGURATION END:   TYPE=ALL *****/
      /******************************************/


      /******************************************/
      /****** CONFIGURATION START: TYPE=GT *****/
      /******************************************/
      if(ssp[slot].fw_type == SSP_CFG_SSPTYPE_HALLBGT)
      {
        sprintf(sss,"SSP_GT_LATENCY %d\n",          ssp[slot].gt.gtpif_latency);      ADD_TO_STRING;
        sprintf(sss,"SSP_GT_ECAL_ESUM_DELAY %d\n",     ssp[slot].gt.ecal.esum_delay);    ADD_TO_STRING;
        sprintf(sss,"SSP_GT_ECAL_CLUSTER_DELAY %d\n",  ssp[slot].gt.ecal.cluster_delay); ADD_TO_STRING;
        sprintf(sss,"SSP_GT_ECAL_ESUM_INTWIDTH %d\n",  ssp[slot].gt.ecal.esum_intwidth); ADD_TO_STRING;
        sprintf(sss,"SSP_GT_PCAL_ESUM_DELAY %d\n",     ssp[slot].gt.pcal.esum_delay);    ADD_TO_STRING;
        sprintf(sss,"SSP_GT_PCAL_CLUSTER_DELAY %d\n",  ssp[slot].gt.pcal.cluster_delay); ADD_TO_STRING;
        sprintf(sss,"SSP_GT_PCAL_ESUM_INTWIDTH %d\n",  ssp[slot].gt.pcal.esum_intwidth); ADD_TO_STRING;
        
        for(i=0; i<4; i++)
        {
          sprintf(sss,"SSP_GT_STRG_EN %d %d\n",                       i, ssp[slot].gt.strg[i].en);                       ADD_TO_STRING;
          sprintf(sss,"SSP_GT_STRG_PCAL_CLUSTER_EMIN_EN %d %d\n",     i, ssp[slot].gt.strg[i].pcal_cluster_emin_en);     ADD_TO_STRING;
          sprintf(sss,"SSP_GT_STRG_ECALOUT_CLUSTER_EMIN_EN %d %d\n",  i, ssp[slot].gt.strg[i].ecalout_cluster_emin_en);  ADD_TO_STRING;
          sprintf(sss,"SSP_GT_STRG_ECALIN_CLUSTER_EMIN_EN %d %d\n",   i, ssp[slot].gt.strg[i].ecalin_cluster_emin_en);   ADD_TO_STRING;
          sprintf(sss,"SSP_GT_STRG_PCAL_ESUM_EN %d %d\n",             i, ssp[slot].gt.strg[i].pcal_esum_en);             ADD_TO_STRING;
          sprintf(sss,"SSP_GT_STRG_ECAL_ESUM_EN %d %d\n",             i, ssp[slot].gt.strg[i].ecal_esum_en);             ADD_TO_STRING;
          
          sprintf(sss,"SSP_GT_STRG_ECAL_ESUM_MIN %d %d\n",            i, ssp[slot].gt.strg[i].ecal_esum_emin);           ADD_TO_STRING;
          sprintf(sss,"SSP_GT_STRG_ECAL_ESUM_WIDTH %d %d\n",          i, ssp[slot].gt.strg[i].ecal_esum_width);          ADD_TO_STRING;
          sprintf(sss,"SSP_GT_STRG_PCAL_ESUM_MIN %d %d\n",            i, ssp[slot].gt.strg[i].pcal_esum_emin);           ADD_TO_STRING;
          sprintf(sss,"SSP_GT_STRG_PCAL_ESUM_WIDTH %d %d\n",          i, ssp[slot].gt.strg[i].pcal_esum_width);          ADD_TO_STRING;
          sprintf(sss,"SSP_GT_STRG_ECALIN_CLUSTER_EMIN %d %d\n",      i, ssp[slot].gt.strg[i].ecalin_cluster_emin);      ADD_TO_STRING;
          sprintf(sss,"SSP_GT_STRG_ECALIN_CLUSTER_WIDTH %d %d\n",     i, ssp[slot].gt.strg[i].ecalin_cluster_width);     ADD_TO_STRING;
          sprintf(sss,"SSP_GT_STRG_ECALOUT_CLUSTER_EMIN %d %d\n",     i, ssp[slot].gt.strg[i].ecalout_cluster_emin);     ADD_TO_STRING;
          sprintf(sss,"SSP_GT_STRG_ECALOUT_CLUSTER_WIDTH %d %d\n",    i, ssp[slot].gt.strg[i].ecalout_cluster_width);    ADD_TO_STRING;
          sprintf(sss,"SSP_GT_STRG_PCAL_CLUSTER_EMIN %d %d\n",        i, ssp[slot].gt.strg[i].pcal_cluster_emin);        ADD_TO_STRING;
          sprintf(sss,"SSP_GT_STRG_PCAL_CLUSTER_WIDTH %d %d\n",       i, ssp[slot].gt.strg[i].pcal_cluster_width);       ADD_TO_STRING;
        }
      }
      /******************************************/
      /****** CONFIGURATION END:   TYPE=GT *****/
      /******************************************/
      
      /******************************************/
      /****** CONFIGURATION START: TYPE=HPS *****/
      /******************************************/
      if(ssp[slot].fw_type == SSP_CFG_SSPTYPE_HPS)
      {
        for(i = 0; i < 2; i++)
        {
          sprintf(sss,"SSP_HPS_SINGLES_EMIN %d %d %d\n",i,ssp[slot].hps.s[i].emin,ssp[slot].hps.s[i].emin_en); ADD_TO_STRING;
          sprintf(sss,"SSP_HPS_SINGLES_EMAX %d %d %d\n",i,ssp[slot].hps.s[i].emax,ssp[slot].hps.s[i].emax_en); ADD_TO_STRING;
          sprintf(sss,"SSP_HPS_SINGLES_NMIN %d %d %d\n",i,ssp[slot].hps.s[i].nmin,ssp[slot].hps.s[i].nmin_en); ADD_TO_STRING;

          for(j=0; j<7;j++)
          {
            sprintf(sss,"SSP_HPS_SINGLES_PRESCALE %d %d %d %d %d\n",i,j,
            ssp[slot].hps.s[i].prescale_xmin[j],
            ssp[slot].hps.s[i].prescale_xmax[j],
            ssp[slot].hps.s[i].prescale[j]);
            ADD_TO_STRING;
          }
        }

        sprintf(sss,"SSP_HPS_LATENCY %d\n",ssp[slot].hps.trigger_latency); ADD_TO_STRING;
        sprintf(sss,"SSP_HPS_COSMIC_TIMECOINCIDENCE %d\n",ssp[slot].hps.cosmic_timecoincidence); ADD_TO_STRING;
        sprintf(sss,"SSP_HPS_COSMIC_PATTERNCOINCIDENCE %d\n",ssp[slot].hps.cosmic_pattern); ADD_TO_STRING;

        for(i = 0; i < 2; i++)
        {
          sprintf(sss,"SSP_HPS_PAIRS_TIMECOINCIDENCE %d %d\n",i,ssp[slot].hps.p[i].timecoincidence); ADD_TO_STRING;
            sprintf(sss,"SSP_HPS_PAIRS_EMIN %d %d\n",i,ssp[slot].hps.p[i].emin); ADD_TO_STRING;
            sprintf(sss,"SSP_HPS_PAIRS_EMAX %d %d\n",i,ssp[slot].hps.p[i].emax); ADD_TO_STRING;
            sprintf(sss,"SSP_HPS_PAIRS_NMIN %d %d\n",i,ssp[slot].hps.p[i].nmin); ADD_TO_STRING;
            sprintf(sss,"SSP_HPS_PAIRS_SUMMAX_MIN %d %d %d %d\n",i,ssp[slot].hps.p[i].summax,ssp[slot].hps.p[i].summin,ssp[slot].hps.p[i].summax_en); ADD_TO_STRING;
            sprintf(sss,"SSP_HPS_PAIRS_DIFFMAX %d %d %d\n",i,ssp[slot].hps.p[i].diffmax,ssp[slot].hps.p[i].diffmax_en); ADD_TO_STRING;
            sprintf(sss,"SSP_HPS_PAIRS_COPLANARITY %d %d %d\n",i,ssp[slot].hps.p[i].coplanartolerance,ssp[slot].hps.p[i].coplanartolerance_en); ADD_TO_STRING;
            sprintf(sss,"SSP_HPS_PAIRS_ENERGYDIST %d %7.3f %d %d\n",i,ssp[slot].hps.p[i].edfactor,ssp[slot].hps.p[i].edmin,ssp[slot].hps.p[i].ed_en); ADD_TO_STRING;
        }
      }
      /******************************************/
      /****** CONFIGURATION END:   TYPE=HPS *****/
      /******************************************/
    }

    CLOSE_STRING;
  }
  return(0);
}

int
sspUploadAllPrint()
{
  char str[16001];
  sspUploadAll(str, 16000);
  printf("%s",str);
  return 0;
}

/* print board registers; if slot is zero, print all boards */
void
sspMon(int slot)
{
  int id, kk, start, end, res;
  int a24_a32, sngl_blt_mblt, sst_rate;
  usrVmeDmaGetConfig(&a24_a32, &sngl_blt_mblt, &sst_rate);

  printf("\nCPU DMA settings:\n");
  if(a24_a32==0) printf("  A16 address mode\n");
  else if(a24_a32==1) printf("  A24 address mode\n");
  else if(a24_a32==2) printf("  A32 address mode\n");
  else printf("  unknown address mode\n");

  if(sngl_blt_mblt==0x01) printf("  D32 single word readout\n");
  else if(sngl_blt_mblt==0x02) printf("  D32 DMA (BLT) readout\n");
  else if(sngl_blt_mblt==0x03) printf("  D64 DMA (MBLT) readout\n");
  else if(sngl_blt_mblt==0x04) printf("  D64 DMA (2eVME) readout\n");
  else if(sngl_blt_mblt==0x05) printf("  D64 DMA (2eSST) readout\n");
  else printf("  unknown readout mode !!!\n");

  if(sst_rate==0) printf("  SST160 rate\n");
  else if(sst_rate==1) printf("  SST267 rate\n");
  else printf("  unknown SST rate\n");

  printf("\n");


  nssp = sspGetNssp();
  if(slot==0)
  {
    start = 0;
    end = nssp;
  }
  else if((id = sspId(slot)) >= 0)
  {
    start = id;
    end = start + 1;
  }
  else
  {
    return;
  }

  printf("nssp=%d\n",nssp);
  for(kk=start; kk<end; kk++)
  {
    sspStatus(sspSlot(kk),0);
  }

  return;
}


#else /* dummy version*/

void
sspConfig_dummy()
{
  return;
}

#endif
