#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sspLib.h"


#ifdef Linux_vme

#define NAMESCALER "ssprich_scaler_138.txt" // if you modify this, please modify below accordingly

#define gBUF_LEN  10000
unsigned int gBuf[gBUF_LEN];


//ssprich_scaler_%03d.txt
//ssprich_scaler_138.txt

int  sspRich_ReadTempAndVoltages(int slot,int fiber);
int  sspRich_InitMarocReg(int slot,int fiber,int asic,int threshold,int gain);



int
main(int argc, char *argv[])
{
  int d;
  int res;
  char myname[256];
  unsigned int addr, laddr;
  int slot = 0, i, dac;
  int boardId, fibers, asic, ch, wordcnt;
  unsigned int ref, maroc[RICH_CHAN_NUM];
  unsigned int mask0, mask1;
  unsigned int mask2, mask3;
  unsigned int mask4, mask5;

  int fiber;
  int absChannel;
  int threshold = 300;
  int gain = 64;
  int ctestAmplitude= 1000;
  int ret;
  int pulserFrequency=1.0E3;
  FILE * fout;
  char foutName[80]=NAMESCALER; 


 /* Checks Arguments*/
  printf("\n");
  if(argc==4)
  {
    strncpy(myname, argv[0], 255);
    slot =  atoi(argv[1]);
    threshold = atoi(argv[2]);
    ctestAmplitude = atoi(argv[3]);
    printf("\nWelcome to DAQ, ");
    printf("threshold is %d ",threshold);
    printf("ctest Amplitude  is %d\n",ctestAmplitude);
  }
  else
  {
    printf("Usage: ssptest [slot] [TDC threshold] [CTEST Amplitude]\n");
    exit(0);
  }
  printf("\n");

  /* Open the default VME windows */
  vmeOpenDefaultWindows();
  printf("\n");

  //sspConfig(filename)

  sspInit(slot<<19, 0, 1, SSP_INIT_NO_INIT | SSP_INIT_SKIP_FIRMWARE_CHECK);
  sspSetMode(slot, 1 | (0xFF<<16), 1);
  sspGStatus(1);

  // Reset event builder
  sspSetBlockLevel(slot, 1);

  sspSetIOSrc(slot, SD_SRC_SYNC, SD_SRC_SEL_0);
  sspSetIOSrc(slot, SD_SRC_TRIG, SD_SRC_SEL_0);
  sspEbReset(slot, 1);
  sspEbReset(slot, 0);


  // Init SPP and Discovery tiles
  sspRich_Init(slot);
  sspRich_PrintFiberStatus(slot);

  sspRich_GetConnectedFibers(slot, &fibers);
  printf("sspRich_GetConnectedFibers(%d) fibers = 0x%08X\n", slot, fibers);

  //return 0;

 if(fibers<1){
    printf("No fibers connected. Exit\n");
    return -1;
  }

  // Monitor Temperatures and Voltages
  for(i=0; i < 32; i++)
  {
    if(fibers & (1<<i))
    {
      sspRich_ReadTempAndVoltages(slot,i); // on all connected fibers, neglecting if maroc present maroc or not
    }
  }

  // Counts the number of complete tiles (valid FPGA + valid MAROC)
  ret =0;
  for(i=0; i < 32; i++)
  {
    if(fibers & (1<<i))
    {
      if(!sspRich_IsAsicInvalid(slot,i))ret++;
    }
  }
  if(ret<=0){
    printf("No maroc connected. Exit\n");
    return -1;
  }else{
    printf("Found %d complete tiles\n",ret);
  }

  // SSP setup
  // Note: SSP RICH is 125MHz clocked, so sspPulserSetup frequency assumes 250MHz, so:
  //       asking for 20kHz will give 10kHz on SSP RICH (will be fixed at some point)
  sspPulserSetup(slot, 2*pulserFrequency, 0.5, 0xFFFFFFFF);
  sspSetIOSrc(slot, SD_SRC_TRIG2, SD_SRC_SEL_PULSER);


 //  TILES setup
  for(i=0; i < 32; i++)
  {
    if(fibers & (1<<i))
    {
      if(sspRich_IsAsicInvalid(slot,i)) continue;

        // CTEST Amplitude
        sspRich_SetCTestAmplitude(slot, i, ctestAmplitude);


        // MAROC Slow Control (Gains,thresholds,)
        for(asic = 0; asic < 3; asic++) sspRich_InitMarocReg(slot,i,asic,threshold,gain);
        printf("************** WR **** FIBER %d ***********\n",i);
        sspRich_PrintMarocRegs(slot, i, 0, RICH_MAROC_REGS_WR);
        sspRich_UpdateMarocRegs(slot, i);// First update shift into MAROC ASIC
        sspRich_UpdateMarocRegs(slot, i);// Second update shift into MAROC ASIC, and out of MAROC ASIC into FPGA
        printf("************** RD **** FIBER %d ***********\n",i);
        sspRich_PrintMarocRegs(slot, i, 0, RICH_MAROC_REGS_RD);


        // CTEST signal source
        if(ctestAmplitude>0)sspRich_SetCTestSource(slot,i, RICH_SD_CTEST_SRC_SEL_SSP);
        else sspRich_SetCTestSource(slot,i, RICH_SD_CTEST_SRC_SEL_0);

        // TDC enable
        sspRich_SetTDCEnableChannelMask(slot,i,0xFFFFFFFF,0xFFFFFFFF,
                                               0xFFFFFFFF,0xFFFFFFFF,
                                               0xFFFFFFFF,0xFFFFFFFF);
        // TDC window
        sspRich_SetLookback(slot, i, 4000);
        sspRich_SetWindow(slot, i, 1000);
    }
  }

  sspSetIOSrc(slot, SD_SRC_SYNC, SD_SRC_SEL_1);
  usleep(1000);
  sspSetIOSrc(slot, SD_SRC_SYNC, SD_SRC_SEL_0);
  usleep(1000);


  // SCALERS
  for(i=0; i < 32; i++)
  {
    if(fibers & (1<<i))
    {
      if(sspRich_IsAsicInvalid(slot,i)) continue;

      // dummy read to reset scalers
      sspRich_ReadScalers(slot, i, &ref, maroc);
      printf("Scaler Reset for Fiber %d DONE\n",i);
    }
  }

  printf("Counting...\n");
  sleep(1);

  // READ SCALER
  for(i=0; i < 32; i++)
  {
    if(fibers & (1<<i))
    {
      if(sspRich_IsAsicInvalid(slot,i)) continue;

      sprintf(foutName,"ssprich_scaler_%03d.txt",i);
      fout = fopen(foutName,"w");
      if(fout==NULL){printf("Error: file %s not opened\n",foutName); return -1;}

      // reset the counts array
      for(asic =0 ; asic<3 ; asic++){
        for(ch=0 ; ch<64; ch++){
          maroc[asic*64+ch]=666;
        }
      }

      sspRich_ReadScalers(slot, i, &ref, maroc);

      // print screen
      printf("\nScaler Print:\n");
      printf("THR %d\n", threshold);
      printf("GAIN %d\n", gain);
      printf("Ref = %u\n", ref);
      printf("Pulser = %d [Hz]\n", pulserFrequency);
      for(ch = 0; ch < 64; ch++){
        printf("Fiber %2d Ch%2d:",i, ch);
        for(asic = 0; asic < 3; asic++){
          printf(" %10u", maroc[asic*64+ch]);
        }
        printf("\n");
      }

      // export txt
      fprintf(fout,"%d\n", threshold);
      fprintf(fout,"%d\n", gain);
      fprintf(fout, "%d\n", (int)ref);
      fprintf(fout,"%d\n", pulserFrequency);
      for(asic = 0; asic < 3; asic++){
        for (ch = 0; ch < 64; ch++){
          absChannel = ch+64*asic+192*i+192*32*slot;
          fprintf(fout,"%8d %10d\n",absChannel,(int) maroc[asic*64+ch]);
        }
      }
      fclose(fout);
    }
  }
  exit(0);
}


int  sspRich_InitMarocReg(int slot,int fiber,int asic,int threshold,int gain){

    int i;

    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_CMD_FSU, 0, 1);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_CMD_SS, 0, 1);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_CMD_FSB, 0, 1);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_SWB_BUF_250F, 0, 0);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_SWB_BUF_500F, 0, 0);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_SWB_BUF_1P, 0, 0);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_SWB_BUF_2P, 0, 0);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_ONOFF_SS, 0, 1);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_SW_SS_300F, 0, 1);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_SW_SS_600F, 0, 1);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_SW_SS1200F, 0, 0);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_EN_ADC, 0, 1);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_H1H2_CHOICE, 0, 0);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_SW_FSU_20F, 0, 1);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_SW_FSU_40F, 0, 1);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_SW_FSU_25K, 0, 0);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_SW_FSU_50K, 0, 0);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_SW_FSU_100K, 0, 0);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_SW_FSB1_50K, 0, 0);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_SW_FSB1_100K, 0, 0);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_SW_FSB1_100F, 0, 1);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_SW_FSB1_50F, 0, 1);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_CMD_FSB_FSU, 0, 0);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_VALID_DC_FS, 0, 1);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_SW_FSB2_50K, 0, 0);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_SW_FSB2_100K, 0, 0);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_SW_FSB2_100F, 0, 0);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_SW_FSB2_50F, 0, 1);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_VALID_DC_FSB2, 0, 0);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_ENB_TRISTATE, 0, 1);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_POLAR_DISCRI, 0, 0);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_INV_DISCRIADC, 0, 0);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_D1_D2, 0, 0);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_CMD_CK_MUX, 0, 0);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_ONOFF_OTABG, 0, 0);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_ONOFF_DAC, 0, 0);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_SMALL_DAC, 0, 0);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_ENB_OUTADC, 0, 0);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_INV_STARTCMPTGRAY, 0, 0);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_RAMP_8BIT, 0, 0);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_RAMP_10BIT, 0, 0);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_DAC0, 0, threshold);
    sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_DAC1, 0, 0);

    for(i = 0; i < 64; i++)
    {
      sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_GAIN, i, gain);
      sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_SUM, i, 0);
      sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_CTEST, i, 1);
      sspRich_SetMarocReg(slot, fiber, asic, RICH_MAROC_REG_MASKOR, i, 0);
    }

  return 0;
}



int  sspRich_ReadTempAndVoltages(int slot,int fiber){

  sspRich_Monitor mon;
  FILE * fmon;
  const char * fmonName = "ssprich_monitor.txt";

  fmon=fopen(fmonName,"a");
  if(!fmon){
    printf("Error in %s: cannot open file %s",__FUNCTION__,fmonName);
    exit(0);
  }

  sspRich_ReadMonitor(slot, fiber, &mon);

  fprintf(fmon,"%d ",(int)time(NULL));
  fprintf(fmon,"%d %d ",slot, fiber);
  fprintf(fmon,"%.3f ",(float)mon.temps.fpga / 1000.0f);
  fprintf(fmon,"%.3f ",(float)mon.temps.regulator[0] / 1000.0f);
  fprintf(fmon,"%.3f ",(float)mon.temps.regulator[1] / 1000.0f);
  fprintf(fmon,"%.3f ",(float)mon.voltages.pcb_5v / 1000.0f);
  fprintf(fmon,"%.3f ",(float)mon.voltages.pcb_3_3v / 1000.0f);
  fprintf(fmon,"%.3f ",(float)mon.voltages.fpga_vccint_1v / 1000.0f);
  fprintf(fmon,"%.3f ",(float)mon.voltages.fpga_vccaux_1_8v / 1000.0f);
  fprintf(fmon,"%.3f ",(float)mon.voltages.fpga_mgt_1v / 1000.0f);
  fprintf(fmon,"%.3f ",(float)mon.voltages.fpga_mgt_1_2v / 1000.0f);
  fprintf(fmon,"\n");
  fclose(fmon);

  return 0;
}

#else

int
main()
{
  return(0);
}
#endif

