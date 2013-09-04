
/* svtscan.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vscmLib.h"

#ifdef VXWORKS

int
svtScan()
{
  return(0);
}

#else

/*
#include "../../dac/main/tcpServer.c"
*/

int
main(int argc, char *argv[])
{
  char filename[256];
  int minthreshold, pulser_rate;
  int ii;
  int beg_chip, end_chip, beg_chan, end_chan;

  if(argc!=4 && argc!=6 && argc!=8)
  {
    printf("usage: svtscan <filename> min_threshold pulser_rate [beg_chip(0-7) end_chip(0-7)] [beg_chan(0-127) end_chan(0-127)]\n");
    exit(0);
  }
  else
  {
    strncpy(filename, argv[1], 255);
    minthreshold = atoi(argv[2]);
    pulser_rate = atoi(argv[3]);
    printf("use arguments: >%s< %d %d\n",filename,minthreshold,pulser_rate);

    beg_chip = 0;
    end_chip = 7;
    beg_chan = 0;
    end_chan = 127;

    if(argc==6||argc==8)
	{
      beg_chip = atoi(argv[4]);
      end_chip = atoi(argv[5]);
    }

    if(argc==8)
	{
      beg_chan = atoi(argv[6]);
      end_chan = atoi(argv[7]);
    }

  }

  /* Open the default VME windows */
  vmeOpenDefaultWindows();

  VSCMInit();

  VSCMClear();

  /*for(ii=0; ii<8; ii++) FSSR_SCR(ii);*/

/*  ExternalAmplitudeScan(filename, minthreshold, pulser_rate, beg_chip, end_chip, beg_chan, end_chan);*/
  /*FSSRAmplitudeScan(filename, minthreshold, pulser_rate, beg_chip, end_chip, beg_chan, end_chan);*/
//  VSCMThresholdScan(filename, minthreshold, pulser_rate, beg_chip, end_chip, beg_chan, end_chan);

  exit(0);
}

#endif
