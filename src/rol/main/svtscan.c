
/* svtscan.c */
/*
./Linux_i686_vme/bin/svtscan 3 /usr/local/clas12/parms/vscm/VSCMConfig.txt 100 1 0 7 0 127
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vscmLib.h"


#ifdef Linux_vme

int
main(int argc, char *argv[])
{
  int board_id;
  char filename[256];
  int beg_chip, end_chip, beg_chan, end_chan;
  int start_thr, chan_mult;
  int ii, jj, nvscm1;

  if(argc!=5 && argc!=7 && argc!=9)
  {
    printf("usage: svtscan <slot#> <filename> <start threshold> <channel multiplicity> [beg_chip(0-7) end_chip(0-7)] [beg_chan(0-127) end_chan(0-127)]\n");
    exit(0);
  }
  else
  {
    board_id = atoi(argv[1]);
    strncpy(filename, argv[2], 255);
    start_thr = atoi(argv[3]);
    chan_mult = atoi(argv[4]);
    printf("use arguments: %d >%s< %d %d\n",board_id,filename,start_thr,chan_mult);

    beg_chip = 0;
    end_chip = 3;
    beg_chan = 0;
    end_chan = 127;

    if(argc==7||argc==9)
	{
      beg_chip = atoi(argv[5]);
      end_chip = atoi(argv[6]);
    }

    if(argc==9)
	{
      beg_chan = atoi(argv[7]);
      end_chan = atoi(argv[8]);
    }

  }

  /* Open the default VME windows */
  vmeOpenDefaultWindows();

  /* initialize VSCM board(s) */
  nvscm1 = vscmInit((unsigned int)(3<<19),(1<<19),20,1);

  vscmPrestart("/usr/local/clas/clas12/parms/vscm/VSCMConfig.txt");
  for(ii=0; ii<nvscm1; ii++)
  {
    for(jj=0; jj<8; jj++)
    {
      fssrSCR(vscmID[ii], jj);
	}
  }

  /* scan */
  fssrGainScan(board_id, filename, beg_chip, end_chip, beg_chan, end_chan, start_thr, chan_mult);

  exit(0);
}

#else

int
main()
{
  return(0);
}

#endif
