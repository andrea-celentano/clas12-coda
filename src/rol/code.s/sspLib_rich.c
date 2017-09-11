#if defined(Linux_vme)
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sspLib.h"

#include "jvme.h"

extern volatile SSP_regs *pSSP[MAX_VME_SLOTS+1];        /* pointers to SSP memory map */
int sspRichConnectedFibers[MAX_VME_SLOTS+1];            /* array of SSPs valid fiber connections */
MAROC_Regs sspRich_MAROC_Regs[MAX_VME_SLOTS+1][RICH_FIBER_NUM][3]; /* array of MAROC ASIC configurations */
MAROC_Regs sspRich_MAROC_Regs_Rd[MAX_VME_SLOTS+1][RICH_FIBER_NUM][3]; /* array of MAROC ASIC configurations */
int sspRichConnectedAsic[MAX_VME_SLOTS+1][RICH_FIBER_NUM];   /* array of valid MAROC ASIC */

int sspRich_IsFiberInvalid(int id, int fiber, const char *func)
{
  if(sspRichConnectedFibers[id] & (1<<fiber))
    return OK;

  printf("%s: SSP Slot %d, Fiber %d - not connected.\n", func, id, fiber);
  return ERROR;
}


int sspRich_IsAsicInvalid(int id, int fiber)
{
  if(sspRichConnectedAsic[id][fiber]==SSP_RICH_ASIC_TYPE_2MAROC||
     sspRichConnectedAsic[id][fiber]==SSP_RICH_ASIC_TYPE_3MAROC){
    return OK;
  }
  return ERROR;
}


int sspRich_GetConnectedFibers(int id, int *fibers)
{
  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH))
    return ERROR;

  *fibers = sspRichConnectedFibers[id];
  
  return OK;
}

int sspRich_SetSoftReset(int id, int fiber, int reset)
{
  unsigned int val;
  
  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH) ||
     sspRich_IsFiberInvalid(id, fiber, __func__))
    return ERROR;

  if(reset)
    val = RICH_CLK_SOFTRESET_MASK;
  else
    val = 0;
  
  SSPLOCK();
  sspWriteReg(&pSSP[id]->rich.fiber[fiber].Clk.Ctrl, val);
  SSPUNLOCK();
  
  return OK;
}

int sspRich_SetCTestSource(int id, int fiber, int src)
{
  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH) ||
     sspRich_IsFiberInvalid(id, fiber, __func__))
    return ERROR;
  
  SSPLOCK();
  sspWriteReg(&pSSP[id]->rich.fiber[fiber].Sd.CTestSrc, src);
  SSPUNLOCK();
  
  return OK;
}

int sspRich_GetCTestSource(int id, int fiber, int *src)
{
  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH) ||
     sspRich_IsFiberInvalid(id, fiber, __func__))
    return ERROR;
  
  SSPLOCK();
  *src = sspReadReg(&pSSP[id]->rich.fiber[fiber].Sd.CTestSrc);
  SSPUNLOCK();
  
  return OK;
}

int sspRich_ReadMonitor(int id, int fiber, sspRich_Monitor *mon)
{
  unsigned int val[32];
  int addr;
  float v;

  memset(mon, 0, sizeof(sspRich_Monitor));
 
  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH) ||
     sspRich_IsFiberInvalid(id, fiber, __func__))
    return ERROR;

  SSPLOCK();
  for(addr = 0; addr < 0x20; addr++)
  {
    sspWriteReg(&pSSP[id]->rich.fiber[fiber].Testing.XAdcCtrl, 0x01000000 | (addr<<16));
    val[addr] = sspReadReg(&pSSP[id]->rich.fiber[fiber].Testing.XAdcStatus);
  }
  SSPUNLOCK();

  // Temperature registers/scaling
  v = (float)(val[0] & 0xFFFF) * 503.975f / 65536.0f - 273.15f;
  v = 1000.0f * v;
  mon->temps.fpga = (int)v;

  v = (float)(val[18] & 0xFFFF) * 1.0f / 65536.0f;
  v = 1000.0f * (27.0f + (0.598f - v) / 0.002f);
  mon->temps.regulator[0] = (int)v;

  v = (float)(val[26] & 0xFFFF) * 1.0f / 65536.0f;
  v = 1000.0f * (27.0f + (0.598f - v) / 0.002f);
  mon->temps.regulator[1] = (int)v;
  
  // Votlage registers/scaling  
  v = (float)(val[16] & 0xFFFF) * 1.0f / 65536.0f;
  v = 1000.0f * (v + (v / 237.0f) * 1240.0f);
  mon->voltages.pcb_5v = (int)v;
  
  v = (float)(val[24] & 0xFFFF) * 1.0f / 65536.0f;
  v = 1000.0f * (v + (v / 249.0f) * 825.0f);
  mon->voltages.pcb_3_3v = (int)v;

  v = (float)(val[1] & 0xFFFF) * 3.0f / 65.536f;
  mon->voltages.fpga_vccint_1v = (int)v;

  v = (float)(val[2] & 0xFFFF) * 3.0f / 65.536f;
  mon->voltages.fpga_vccaux_1_8v = (int)v;
  
  v = (float)(val[17] & 0xFFFF) * 1.0f / 65536.0f;
  v = 1000.0f * (v + (v / 1000.0f) * 249.0f);
  mon->voltages.fpga_mgt_1v = (int)v;
  
  v = (double)(val[22] & 0xFFFF) * 1.0 / 65536.0;
  v = 1000.0f * (v + (v / 604.0f) * 301.0f);
  mon->voltages.fpga_mgt_1_2v = (int)v;
  
  return OK;
}

int sspRich_PrintMonitor(int id, int fiber)
{
  sspRich_Monitor mon;

  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH) ||
     sspRich_IsFiberInvalid(id, fiber, __func__))
    return ERROR;

  sspRich_ReadMonitor(id, fiber, &mon);

  printf("%s(slot=%d,fiber=%d):\n", __func__, id, fiber);
  printf("  Temperatures:\n");
  printf("    FPGA          %.3fC\n", (float)mon.temps.fpga / 1000.0f);
  printf("    Regulator 0   %.3fC\n", (float)mon.temps.regulator[0] / 1000.0f);
  printf("    Regulator 1   %.3fC\n", (float)mon.temps.regulator[1] / 1000.0f);
  printf("  Voltages:\n");
  printf("    +5V           %.3fV\n", (float)mon.voltages.pcb_5v / 1000.0f);
  printf("    +3.3V         %.3fV\n", (float)mon.voltages.pcb_3_3v / 1000.0f);
  printf("    VccInt(+1.0V) %.3fV\n", (float)mon.voltages.fpga_vccint_1v / 1000.0f);
  printf("    VccAux(+1.8V) %.3fV\n", (float)mon.voltages.fpga_vccaux_1_8v / 1000.0f);
  printf("    Mgt(+1.0V)    %.3fV\n", (float)mon.voltages.fpga_mgt_1v / 1000.0f);
  printf("    Mgt(+1.2V)    %.3fV\n", (float)mon.voltages.fpga_mgt_1_2v / 1000.0f);

  return OK;
}

int sspRich_ReadScalers(int id, int fiber, unsigned int *ref, unsigned int maroc[RICH_CHAN_NUM])
{
  int asic, ch;

  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH) ||
     sspRich_IsFiberInvalid(id, fiber, __func__))
    return ERROR;

  SSPLOCK();
  sspWriteReg(&pSSP[id]->rich.fiber[fiber].Sd.ScalerLatch, 0x1);

  *ref = sspReadReg(&pSSP[id]->rich.fiber[fiber].Sd.Scaler_GClk125);

  for(asic = 0; asic < 3; asic++)
  for(ch = 0; ch < 64; ch++)
    maroc[64*asic+ch] = sspReadReg(&pSSP[id]->rich.fiber[fiber].MAROC_Proc[asic].Scalers[ch]);

  sspWriteReg(&pSSP[id]->rich.fiber[fiber].Sd.ScalerLatch, 0x0);
  SSPUNLOCK();

  return OK;
}

int sspRich_PrintScalers(int id, int fiber)
{
  unsigned int maroc[RICH_CHAN_NUM];
  unsigned int chip[3];
  unsigned int ref, i;
  float maroc_hz[RICH_CHAN_NUM], chip_hz[3], scale;
  int ch, asic;

  chip[0] = 0;
  chip[1] = 0;
  chip[2] = 0;
  chip_hz[0] = 0.0;
  chip_hz[1] = 0.0;
  chip_hz[2] = 0.0;

  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH) ||
     sspRich_IsFiberInvalid(id, fiber, __func__))
    return ERROR;
  
  sspRich_ReadScalers(id, fiber, &ref, maroc);

  if(ref>0)
    scale = 125000000.0 / ref;
  else
  {
    printf("%s: ERROR - reference scaler invalid\n");
    scale = 1.0;
  }

  for(i=0;i<RICH_CHAN_NUM;i++)
  {
    maroc_hz[i] = (float)maroc[i]*scale;
    chip_hz[i/64] += maroc_hz[i];
    chip[i/64] += maroc[i];
  }
  
  printf("%s(slot=%d,fiber=%d):\n", __func__, id, fiber);
  printf("Ref = %d, Elapsed Time = %fs\n", ref, 1.0f/scale);
  printf("------------------ RAW COUNTS --------|----------- NORMALIZED RATES -----------\n");
  printf("--------------------------------------|----------------------------------------\n");
  for(ch=0;ch<64;ch++)
  {
    printf("Ch%2d:", ch);
    for(asic=0;asic<3;asic++)
      printf(" %10d", maroc[asic*64+ch]);

   printf("|");

    for(asic=0;asic<3;asic++)
      printf(" %10.0fHz", maroc_hz[asic*64+ch]);

    printf("\n");
  }
  printf("-------------------------------------------------------------------------------\n");
  printf("Tot: ");
  for(asic=0;asic<3;asic++)
    printf(" %10d", chip[asic]);
  printf("|");
  for(asic=0;asic<3;asic++)
    printf(" %10.0fHz", chip_hz[asic]);
  printf("\n");
}

int sspRich_GetBoardId(int id, int fiber, int *boardId)
{
  unsigned int val;
  
  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH) ||
     sspRich_IsFiberInvalid(id, fiber, __func__))
    return ERROR;
  
  SSPLOCK();
  *boardId = sspReadReg(&pSSP[id]->rich.fiber[fiber].Clk.BoardId);
  SSPUNLOCK();
  
  printf("%s: BoardId = 0x%08X\n", __func__, *boardId);
  
  return OK;
}

int sspRich_PrintFiberStatus(int id)
{
  unsigned int Ctrl[RICH_FIBER_NUM], Status[RICH_FIBER_NUM], EvtEnable[RICH_FIBER_NUM];
  int i;

  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH))
    return ERROR;
  
  SSPLOCK();
  for(i = 0; i < RICH_FIBER_NUM; i++)
  {
    Ctrl[i] = sspReadReg(&pSSP[id]->rich.fiber[i].GtxCtrl);
    Status[i] = sspReadReg(&pSSP[id]->rich.fiber[i].GtxStatus);
    EvtEnable[i] = sspReadReg(&pSSP[id]->rich.fiber[i].EbCtrl);
  }
  SSPUNLOCK();
  
  printf("SSP RICH Fiber Status (slot=%d):\n", id);
  printf("------------------------------------------------------------------------\n");
  printf("     Fiber  Rst    ChUp   HrdErr FrmErr ErrCnt EvtEn  Valid #ASIC       \n");
  printf("------------------------------------------------------------------------\n");
  for(i = 0; i < RICH_FIBER_NUM; i++)
  {
    printf("%6d ", i);
    printf("%6d ", (Ctrl[i] & (SSP_RICH_GTXCTRL_FIBER_RESET |SSP_RICH_GTXCTRL_FIBER_GT_RESET)) ? 1 : 0);
    printf("%6d ", (Status[i] & SSP_RICH_GTXSTATUS_CHANNELUP) ? 1 : 0);
    printf("%6d ", (Status[i] & SSP_RICH_GTXSTATUS_HARDERR) ? 1 : 0);
    printf("%6d ", (Status[i] & SSP_RICH_GTXSTATUS_FRAMEERR) ? 1 : 0);
    printf("%6d ", (Status[i] & SSP_RICH_GTXSTATUS_ERRCNT)>>8);
    printf("%6d ", (EvtEnable[i] & SSP_RICH_EVT_ENABLE) ? 1 : 0);
    printf("%6d ", (sspRichConnectedFibers[id] & (1<<i)) ? 1 : 0);
    printf("%6d\n", sspRichConnectedAsic[id][i]);
  }
  
  return OK;
}

int sspRich_ScanFibers_NoInit(int id)
{
  unsigned int i, channelsUp = 0xFFFFFFFF;
  int status, boardid;
  
  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH))
    return ERROR;
  
  printf("ScanFibers without init\n");
  SSPLOCK();
  for(i = 0; i < RICH_FIBER_NUM; i++)
  {
    status = sspReadReg(&pSSP[id]->rich.fiber[i].GtxStatus);
    if(!(status & SSP_RICH_GTXSTATUS_CHANNELUP))
    {
      channelsUp &= ~(1<<i);
      continue;
    }
    
    boardid = sspReadReg(&pSSP[id]->rich.fiber[i].Clk.BoardId);
    printf("%s (fiber %d): BoardId = 0x%08X\n", __func__, i, boardid);
    boardid = sspReadReg(&pSSP[id]->rich.fiber[i].Clk.BoardId);
    printf("%s (fiber %d): BoardId = 0x%08X\n", __func__, i, boardid);
    if(boardid != RICH_CLK_BOARDID)
    {
      channelsUp &= ~(1<<i);
      continue;
    }
  }
  
  SSPUNLOCK();
  
  sspRichConnectedFibers[id] = channelsUp;
  
  printf("%s: Slot %d, connected fibers: 0x%08X\n", __func__, id, channelsUp);
  return OK;
}

int sspRich_ScanFibers(int id)
{
  unsigned int i, channelsUp = 0xFFFFFFFF;
  int status, boardid;
  
  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH))
    return ERROR;
  
  printf("ScanFibers with init: fiber=0x%08x 0x%08x\n",&pSSP[id]->rich.fiber[0],pSSP[id]);
  SSPLOCK();
  for(i = 0; i < RICH_FIBER_NUM; i++)
  {
    sspWriteReg(&pSSP[id]->rich.fiber[i].GtxCtrl, 0x7);
    usleep(1000);
    sspWriteReg(&pSSP[id]->rich.fiber[i].GtxCtrl, 0x0);
  }
  SSPUNLOCK();
  
  usleep(100000);

  SSPLOCK();
  for(i = 0; i < RICH_FIBER_NUM; i++)
  {
    status = sspReadReg(&pSSP[id]->rich.fiber[i].GtxStatus);
    if(!(status & SSP_RICH_GTXSTATUS_CHANNELUP))
    {
      channelsUp &= ~(1<<i);
      continue;
    }
    
    boardid = sspReadReg(&pSSP[id]->rich.fiber[i].Clk.BoardId);
    printf("%s: BoardId = 0x%08X\n", __func__, boardid);
    boardid = sspReadReg(&pSSP[id]->rich.fiber[i].Clk.BoardId);
    printf("%s: BoardId = 0x%08X\n", __func__, boardid);
    if(boardid != RICH_CLK_BOARDID)
    {
      channelsUp &= ~(1<<i);
      continue;
    }
  }
  
  for(i = 0; i < RICH_FIBER_NUM; i++)
  {
    if(!(channelsUp & (1<<i)))
    {
      sspWriteReg(&pSSP[id]->rich.fiber[i].GtxCtrl, 0x3);
      sspWriteReg(&pSSP[id]->rich.fiber[i].EbCtrl, 0x0);
    }
    else
    {
      //sspWriteReg(&pSSP[id]->rich.fiber[i].EbCtrl, 0x0); //disable this to readout RICH
      sspWriteReg(&pSSP[id]->rich.fiber[i].EbCtrl, 0x1); //enable this to readout RICH
    }
  }
  SSPUNLOCK();
  
  sspRichConnectedFibers[id] = channelsUp;
  
  printf("%s: Slot %d, connected fibers: 0x%08X\n", __func__, id, channelsUp);
  return OK;
}

int sspRich_SetLookback(int id, int fiber, int lookback)
{
  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH) ||
     sspRich_IsFiberInvalid(id, fiber, __func__))
    return ERROR;

  // convert lookback from ns to 8ns ticks
  lookback = lookback/8;
  
  if(lookback >= RICH_EB_LOOKBACK_MAX)
    lookback = RICH_EB_LOOKBACK_MAX;
  else if(lookback < 0)
    lookback = 0;

  SSPLOCK();
  sspWriteReg(&pSSP[id]->rich.fiber[fiber].EvtBuilder.Lookback, lookback);
  SSPUNLOCK();
  
  return OK;
}

int sspRich_GetLookback(int id, int fiber, int *lookback)
{
  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH) ||
     sspRich_IsFiberInvalid(id, fiber, __func__))
    return ERROR;

  SSPLOCK();
  // convert lookback from 8ns ticks to ns
  *lookback = sspReadReg(&pSSP[id]->rich.fiber[fiber].EvtBuilder.Lookback) * 8;
  SSPUNLOCK();
  
  return OK;
}


int sspRich_SetWindow(int id, int fiber, int window)
{
  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH) ||
     sspRich_IsFiberInvalid(id, fiber, __func__))
    return ERROR;

  // convert from ns to 8ns ticks
  window = window/8;
  
  if(window >= RICH_EB_WINDOWWIDTH_MAX)
    window = RICH_EB_WINDOWWIDTH_MAX;
  else if(window < 0)
    window = 0;

  SSPLOCK();
  sspWriteReg(&pSSP[id]->rich.fiber[fiber].EvtBuilder.WindowWidth, window);
  SSPUNLOCK();
  
  return OK;
}

int sspRich_GetWindow(int id, int fiber, int *window)
{
  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH) ||
     sspRich_IsFiberInvalid(id, fiber, __func__))
    return ERROR;

  SSPLOCK();
  // convert from 8ns ticks to ns
  *window = sspReadReg(&pSSP[id]->rich.fiber[fiber].EvtBuilder.WindowWidth) * 8;
  SSPUNLOCK();
  
  return OK;
}

int sspRich_SetCTestAmplitude(int id, int fiber, int dac)
{
  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH) ||
     sspRich_IsFiberInvalid(id, fiber, __func__))
    return ERROR;

  if(dac >= RICH_MAROCCFG_DAC_MAX)
    dac = RICH_MAROCCFG_DAC_MAX;
  else if(dac < 0)
    dac = 0;

  SSPLOCK();
  sspWriteReg(&pSSP[id]->rich.fiber[fiber].MAROC_Cfg.DACAmplitude, dac<<2);
  SSPUNLOCK();
  
  return OK;
}

int sspRich_GetCTestAmplitude(int id, int fiber, int *dac)
{
  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH) ||
     sspRich_IsFiberInvalid(id, fiber, __func__))
    return ERROR;

  SSPLOCK();
  // convert from 8ns ticks to ns
  *dac = sspReadReg(&pSSP[id]->rich.fiber[fiber].MAROC_Cfg.DACAmplitude)>>2;
  SSPUNLOCK();
  
  return OK;
}

int sspRich_SetTDCEnableChannelMask(
    int id, int fiber,
    unsigned int mask0, unsigned int mask1,
    unsigned int mask2, unsigned int mask3,
    unsigned int mask4, unsigned int mask5
  )
{
  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH) ||
     sspRich_IsFiberInvalid(id, fiber, __func__))
    return ERROR;

  mask0 = ~mask0;
  mask1 = ~mask1;
  mask2 = ~mask2;
  mask3 = ~mask3;
  mask4 = ~mask4;
  mask5 = ~mask5;
  
  SSPLOCK();

  sspWriteReg(&pSSP[id]->rich.fiber[fiber].MAROC_Proc[0].DisableCh[0],
              (mask0 & 0xFFFF) | ((mask0 & 0xFFFF)<<16));
  
  sspWriteReg(&pSSP[id]->rich.fiber[fiber].MAROC_Proc[0].DisableCh[1],
              ((mask0 & 0xFFFF0000)>>16) | (mask0 & 0xFFFF0000));
  
  sspWriteReg(&pSSP[id]->rich.fiber[fiber].MAROC_Proc[0].DisableCh[2],
              (mask1 & 0xFFFF) | ((mask1 & 0xFFFF)<<16));
  
  sspWriteReg(&pSSP[id]->rich.fiber[fiber].MAROC_Proc[0].DisableCh[3],
              ((mask1 & 0xFFFF0000)>>16) | (mask1 & 0xFFFF0000));

  sspWriteReg(&pSSP[id]->rich.fiber[fiber].MAROC_Proc[1].DisableCh[0],
              (mask2 & 0xFFFF) | ((mask2 & 0xFFFF)<<16));
  
  sspWriteReg(&pSSP[id]->rich.fiber[fiber].MAROC_Proc[1].DisableCh[1],
              ((mask2 & 0xFFFF0000)>>16) | (mask2 & 0xFFFF0000));
  
  sspWriteReg(&pSSP[id]->rich.fiber[fiber].MAROC_Proc[1].DisableCh[2],
              (mask3 & 0xFFFF) | ((mask3 & 0xFFFF)<<16));
  
  sspWriteReg(&pSSP[id]->rich.fiber[fiber].MAROC_Proc[1].DisableCh[3],
              ((mask3 & 0xFFFF0000)>>16) | (mask3 & 0xFFFF0000));

  sspWriteReg(&pSSP[id]->rich.fiber[fiber].MAROC_Proc[2].DisableCh[0],
              (mask4 & 0xFFFF) | ((mask4 & 0xFFFF)<<16));
  
  sspWriteReg(&pSSP[id]->rich.fiber[fiber].MAROC_Proc[2].DisableCh[1],
              ((mask4 & 0xFFFF0000)>>16) | (mask4 & 0xFFFF0000));
  
  sspWriteReg(&pSSP[id]->rich.fiber[fiber].MAROC_Proc[2].DisableCh[2],
              (mask5 & 0xFFFF) | ((mask5 & 0xFFFF)<<16));
  
  sspWriteReg(&pSSP[id]->rich.fiber[fiber].MAROC_Proc[2].DisableCh[3],
              ((mask5 & 0xFFFF0000)>>16) | (mask5 & 0xFFFF0000));
  
  SSPUNLOCK();
  
  return OK;
}

int sspRich_GetTDCEnableChannelMask(
    int id, int fiber,
    unsigned int *mask0, unsigned int *mask1,
    unsigned int *mask2, unsigned int *mask3,
    unsigned int *mask4, unsigned int *mask5
  )
{
  unsigned int val0, val1;
  
  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH) ||
     sspRich_IsFiberInvalid(id, fiber, __func__))
    return ERROR;

  SSPLOCK();
  
  val0 = sspReadReg(&pSSP[id]->rich.fiber[fiber].MAROC_Proc[0].DisableCh[0]);
  val1 = sspReadReg(&pSSP[id]->rich.fiber[fiber].MAROC_Proc[0].DisableCh[1]);
  *mask0 = ((val0>>0) & 0xFFFF)     | ((val0>>16) & 0xFFFF) |
           ((val1<<0) & 0xFFFF0000) | ((val1<<16) & 0xFFFF0000);
  
  val0 = sspReadReg(&pSSP[id]->rich.fiber[fiber].MAROC_Proc[0].DisableCh[2]);
  val1 = sspReadReg(&pSSP[id]->rich.fiber[fiber].MAROC_Proc[0].DisableCh[3]);
  *mask1 = ((val0>>0) & 0xFFFF)     | ((val0>>16) & 0xFFFF) |
           ((val1<<0) & 0xFFFF0000) | ((val1<<16) & 0xFFFF0000);

  val0 = sspReadReg(&pSSP[id]->rich.fiber[fiber].MAROC_Proc[1].DisableCh[0]);
  val1 = sspReadReg(&pSSP[id]->rich.fiber[fiber].MAROC_Proc[1].DisableCh[1]);
  *mask2 = ((val0>>0) & 0xFFFF)     | ((val0>>16) & 0xFFFF) |
           ((val1<<0) & 0xFFFF0000) | ((val1<<16) & 0xFFFF0000);
  
  val0 = sspReadReg(&pSSP[id]->rich.fiber[fiber].MAROC_Proc[1].DisableCh[2]);
  val1 = sspReadReg(&pSSP[id]->rich.fiber[fiber].MAROC_Proc[1].DisableCh[3]);
  *mask3 = ((val0>>0) & 0xFFFF)     | ((val0>>16) & 0xFFFF) |
           ((val1<<0) & 0xFFFF0000) | ((val1<<16) & 0xFFFF0000);

  val0 = sspReadReg(&pSSP[id]->rich.fiber[fiber].MAROC_Proc[2].DisableCh[0]);
  val1 = sspReadReg(&pSSP[id]->rich.fiber[fiber].MAROC_Proc[2].DisableCh[1]);
  *mask4 = ((val0>>0) & 0xFFFF)     | ((val0>>16) & 0xFFFF) |
           ((val1<<0) & 0xFFFF0000) | ((val1<<16) & 0xFFFF0000);
  
  val0 = sspReadReg(&pSSP[id]->rich.fiber[fiber].MAROC_Proc[2].DisableCh[2]);
  val1 = sspReadReg(&pSSP[id]->rich.fiber[fiber].MAROC_Proc[2].DisableCh[3]);
  *mask5 = ((val0>>0) & 0xFFFF)     | ((val0>>16) & 0xFFFF) |
           ((val1<<0) & 0xFFFF0000) | ((val1<<16) & 0xFFFF0000);

  SSPUNLOCK();
  
  return OK;
}

int sspRich_SetDeviceId(int id, int fiber, int devid)
{
  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH) ||
     sspRich_IsFiberInvalid(id, fiber, __func__))
    return ERROR;

  SSPLOCK();
  sspWriteReg(&pSSP[id]->rich.fiber[fiber].EvtBuilder.DeviceID, devid);
  SSPUNLOCK();
  
  return OK;
}


int sspRich_PrintMarocRegs(int id, int fiber, int chip, int type)
{
  int i;
  MAROC_Regs *pMAROC_Regs;
  
  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH) ||
     sspRich_IsFiberInvalid(id, fiber, __func__))
    return ERROR;
  
  if(chip < 0 || chip > 2)
  {
    printf("%s: ERROR invalid chip specified: %d\n", __func__, chip);
    return ERROR;
  }

  if(type == RICH_MAROC_REGS_WR)
    pMAROC_Regs = &sspRich_MAROC_Regs[id][fiber][chip];
  else if(type == RICH_MAROC_REGS_RD)
    pMAROC_Regs = &sspRich_MAROC_Regs_Rd[id][fiber][chip];
  else
  {
    printf("%s: ERROR - unknown type specified %d\n", __func__, type);
    return ERROR;
  }

  printf("%s: MAROC Register Dump (SSP Slot %d, Fiber %d, Chip %d, %s):\n",
         __func__, id, fiber, chip, (type == RICH_MAROC_REGS_WR) ? "Write" : "Read");
  
  printf("Global0 = 0x%08X \n", pMAROC_Regs->Global0.val);

  printf("   cmd_fsu           = %d\n", pMAROC_Regs->Global0.bits.cmd_fsu);
  printf("   cmd_ss            = %d\n", pMAROC_Regs->Global0.bits.cmd_ss);
  printf("   cmd_fsb           = %d\n", pMAROC_Regs->Global0.bits.cmd_fsb);
  printf("   swb_buf_250f      = %d\n", pMAROC_Regs->Global0.bits.swb_buf_250f);
  printf("   swb_buf_500f      = %d\n", pMAROC_Regs->Global0.bits.swb_buf_500f);
  printf("   swb_buf_1p        = %d\n", pMAROC_Regs->Global0.bits.swb_buf_1p);
  printf("   swb_buf_2p        = %d\n", pMAROC_Regs->Global0.bits.swb_buf_2p);
  printf("   ONOFF_ss          = %d\n", pMAROC_Regs->Global0.bits.ONOFF_ss);
  printf("   sw_ss_300f        = %d\n", pMAROC_Regs->Global0.bits.sw_ss_300f);
  printf("   sw_ss_600f        = %d\n", pMAROC_Regs->Global0.bits.sw_ss_600f);
  printf("   sw_ss_1200f       = %d\n", pMAROC_Regs->Global0.bits.sw_ss_1200f);
  printf("   EN_ADC            = %d\n", pMAROC_Regs->Global0.bits.EN_ADC);
  printf("   H1H2_choice       = %d\n", pMAROC_Regs->Global0.bits.H1H2_choice);
  printf("   sw_fsu_20f        = %d\n", pMAROC_Regs->Global0.bits.sw_fsu_20f);
  printf("   sw_fsu_40f        = %d\n", pMAROC_Regs->Global0.bits.sw_fsu_40f);
  printf("   sw_fsu_25k        = %d\n", pMAROC_Regs->Global0.bits.sw_fsu_25k);
  printf("   sw_fsu_50k        = %d\n", pMAROC_Regs->Global0.bits.sw_fsu_50k);
  printf("   sw_fsu_100k       = %d\n", pMAROC_Regs->Global0.bits.sw_fsu_100k);
  printf("   sw_fsb1_50k       = %d\n", pMAROC_Regs->Global0.bits.sw_fsb1_50k);
  printf("   sw_fsb1_100k      = %d\n", pMAROC_Regs->Global0.bits.sw_fsb1_100k);
  printf("   sw_fsb1_100f      = %d\n", pMAROC_Regs->Global0.bits.sw_fsb1_100f);
  printf("   sw_fsb1_50f       = %d\n", pMAROC_Regs->Global0.bits.sw_fsb1_50f);
  printf("   cmd_fsb_fsu       = %d\n", pMAROC_Regs->Global0.bits.cmd_fsb_fsu);
  printf("   valid_dc_fs       = %d\n", pMAROC_Regs->Global0.bits.valid_dc_fs);
  printf("   sw_fsb2_50k       = %d\n", pMAROC_Regs->Global0.bits.sw_fsb2_50k);
  printf("   sw_fsb2_100k      = %d\n", pMAROC_Regs->Global0.bits.sw_fsb2_100k);
  printf("   sw_fsb2_100f      = %d\n", pMAROC_Regs->Global0.bits.sw_fsb2_100f);
  printf("   sw_fsb2_50f       = %d\n", pMAROC_Regs->Global0.bits.sw_fsb2_50f);
  printf("   valid_dc_fsb2     = %d\n", pMAROC_Regs->Global0.bits.valid_dc_fsb2);
  printf("   ENb_tristate      = %d\n", pMAROC_Regs->Global0.bits.ENb_tristate);
  printf("   polar_discri      = %d\n", pMAROC_Regs->Global0.bits.polar_discri);
  printf("   inv_discriADC     = %d\n", pMAROC_Regs->Global0.bits.inv_discriADC);

  printf("Global1 = 0x%08X\n", pMAROC_Regs->Global1.val);
  printf("   d1_d2             = %d\n", pMAROC_Regs->Global1.bits.d1_d2);
  printf("   cmd_CK_mux        = %d\n", pMAROC_Regs->Global1.bits.cmd_CK_mux);
  printf("   ONOFF_otabg       = %d\n", pMAROC_Regs->Global1.bits.ONOFF_otabg);
  printf("   ONOFF_dac         = %d\n", pMAROC_Regs->Global1.bits.ONOFF_dac);
  printf("   small_dac         = %d\n", pMAROC_Regs->Global1.bits.small_dac);
  printf("   enb_outADC        = %d\n", pMAROC_Regs->Global1.bits.enb_outADC);
  printf("   inv_startCmptGray = %d\n", pMAROC_Regs->Global1.bits.inv_startCmptGray);
  printf("   ramp_8bit         = %d\n", pMAROC_Regs->Global1.bits.ramp_8bit);
  printf("   ramp_10bit        = %d\n", pMAROC_Regs->Global1.bits.ramp_10bit);
  printf("DAC = 0x%08X\n", pMAROC_Regs->DAC.val);
  printf("   DAC0              = %d\n", pMAROC_Regs->DAC.bits.DAC0);
  printf("   DAC1              = %d\n", pMAROC_Regs->DAC.bits.DAC1);
    
  printf("Channels:\n");
  printf("%7s%7s%7s%7s%7s\n", "CH", "Gain", "Sum", "CTest", "MaskOr");
  for(i = 0; i < 64; i++)
  {
    if(i & 0x1)
      printf("%7d%7d%7d%7d%7d\n", i,
        pMAROC_Regs->CH[i>>1].bits.Gain0, pMAROC_Regs->CH[i>>1].bits.Sum0,
        pMAROC_Regs->CH[i>>1].bits.CTest0, pMAROC_Regs->CH[i>>1].bits.MaskOr0);
    else
      printf("%7d%7d%7d%7d%7d\n", i,
        pMAROC_Regs->CH[i>>1].bits.Gain1, pMAROC_Regs->CH[i>>1].bits.Sum1,
        pMAROC_Regs->CH[i>>1].bits.CTest1, pMAROC_Regs->CH[i>>1].bits.MaskOr1);
  }
  printf("\n");
  
  return OK;
}

int sspRich_UpdateMarocRegs(int id, int fiber)
{
  int boardId, type;
  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH) ||
     sspRich_IsFiberInvalid(id, fiber, __func__))
    return ERROR;
  
  type = sspRichConnectedAsic[id][fiber];
  if(type == SSP_RICH_ASIC_TYPE_2MAROC)
  {
    sspRich_ShiftMarocRegs(id, fiber, &sspRich_MAROC_Regs[id][fiber][1], &sspRich_MAROC_Regs_Rd[id][fiber][1]);
    sspRich_ShiftMarocRegs(id, fiber, &sspRich_MAROC_Regs[id][fiber][0], &sspRich_MAROC_Regs_Rd[id][fiber][0]);
  }
  else if(type == SSP_RICH_ASIC_TYPE_3MAROC)
  {
    sspRich_ShiftMarocRegs(id, fiber, &sspRich_MAROC_Regs[id][fiber][2], &sspRich_MAROC_Regs_Rd[id][fiber][2]);
    sspRich_ShiftMarocRegs(id, fiber, &sspRich_MAROC_Regs[id][fiber][1], &sspRich_MAROC_Regs_Rd[id][fiber][1]);
    sspRich_ShiftMarocRegs(id, fiber, &sspRich_MAROC_Regs[id][fiber][0], &sspRich_MAROC_Regs_Rd[id][fiber][0]);
  }
  else
  {
    printf("%s: ERROR - unknown type specified %d\n", __func__, type);
    return ERROR;
  }
  return OK;
}

int sspRich_ShiftMarocRegs(int id, int fiber, MAROC_Regs *regs_in, MAROC_Regs *regs_out)
{
  int i, val;
int boardId;

  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH) ||
     sspRich_IsFiberInvalid(id, fiber, __func__))
    return ERROR;
  
//  SSPLOCK();
  // Write 1 set of MAROC registers into shift register
  sspWriteReg(&pSSP[id]->rich.fiber[fiber].MAROC_Cfg.Regs.Global0.val, regs_in->Global0.val);
  sspWriteReg(&pSSP[id]->rich.fiber[fiber].MAROC_Cfg.Regs.Global1.val, regs_in->Global1.val);
  sspWriteReg(&pSSP[id]->rich.fiber[fiber].MAROC_Cfg.Regs.DAC.val, regs_in->DAC.val);

  for(i = 0; i < 32; i++)
    sspWriteReg(&pSSP[id]->rich.fiber[fiber].MAROC_Cfg.Regs.CH[i].val, regs_in->CH[i].val);

  // Perform shift operation for 1 set of MAROC registers
  val = sspReadReg(&pSSP[id]->rich.fiber[fiber].MAROC_Cfg.SerCtrl);
  val |= 0x00000002;
  sspWriteReg(&pSSP[id]->rich.fiber[fiber].MAROC_Cfg.SerCtrl, val);

//  SSPUNLOCK();
  
  // Check for shift register transfer completion
  for(i = 10; i > 0; i--)
  {
//    SSPLOCK();
    val = sspReadReg(&pSSP[id]->rich.fiber[fiber].MAROC_Cfg.SerStatus);
    usleep(1000);
//    SSPUNLOCK();
    
    if(!(val & 0x00000001))
      break;
    
    if(!i)
    {
      printf("%s: SSP Slot %d, Fiber %d - timeout on serial transfer\n", __func__, id, fiber);
      return ERROR;
    }
    usleep(100);
  }
//  SSPLOCK();
  regs_out->Global0.val = sspReadReg(&pSSP[id]->rich.fiber[fiber].MAROC_Cfg.Regs.Global0.val);
  regs_out->Global1.val = sspReadReg(&pSSP[id]->rich.fiber[fiber].MAROC_Cfg.Regs.Global1.val);
  regs_out->DAC.val = sspReadReg(&pSSP[id]->rich.fiber[fiber].MAROC_Cfg.Regs.DAC.val);

  for(i = 0; i < 32; i++)
    regs_out->CH[i].val = sspReadReg(&pSSP[id]->rich.fiber[fiber].MAROC_Cfg.Regs.CH[i].val);
//  SSPUNLOCK();
  
  return OK;
}

int sspRich_InitDynRegs(int id, int fiber)
{
  int val;
  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH) ||
     sspRich_IsFiberInvalid(id, fiber, __func__))
    return ERROR;

  sspWriteReg(&pSSP[id]->rich.fiber[fiber].MAROC_Cfg.DyRegs_WrAll.Ch0_31_Hold1, 0x80000000);
  sspWriteReg(&pSSP[id]->rich.fiber[fiber].MAROC_Cfg.DyRegs_WrAll.Ch32_63_Hold1, 0x00000000);
  sspWriteReg(&pSSP[id]->rich.fiber[fiber].MAROC_Cfg.DyRegs_WrAll.Ch0_31_Hold2, 0x80000000);
  sspWriteReg(&pSSP[id]->rich.fiber[fiber].MAROC_Cfg.DyRegs_WrAll.Ch32_63_Hold2, 0x00000000);

  val = sspReadReg(&pSSP[id]->rich.fiber[fiber].MAROC_Cfg.SerCtrl);
  val |= 0x00000008;
  sspWriteReg(&pSSP[id]->rich.fiber[fiber].MAROC_Cfg.SerCtrl, val);

  return OK;
}

int sspRich_SetMarocReg(int id, int fiber, int chip, int reg, int channel, int val)
{
  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH) ||
     sspRich_IsFiberInvalid(id, fiber, __func__))
    return ERROR;
  
//  printf("%s(fiber=%d,chip=%d,reg=%d,channel=%d,val=%d)\n",
//         __func__, fiber, chip, reg, channel, val);
  
  if(chip < 0 || chip > 2)
  {
    printf("%s: ERROR invalid chip specified: %d\n", __func__, chip);
    return ERROR;
  }

  if(channel < 0 || channel > 64)
  {
    printf("%s: ERROR invalid channel specified: %d\n", __func__, channel);
    return ERROR;
  }

  switch(reg)
  {
    case RICH_MAROC_REG_CMD_FSU:
      sspRich_MAROC_Regs[id][fiber][chip].Global0.bits.cmd_fsu = val;
      break;
    case RICH_MAROC_REG_CMD_SS:
      sspRich_MAROC_Regs[id][fiber][chip].Global0.bits.cmd_ss = val;
      break;
    case RICH_MAROC_REG_CMD_FSB:
      sspRich_MAROC_Regs[id][fiber][chip].Global0.bits.cmd_fsb = val;
      break;
    case RICH_MAROC_REG_SWB_BUF_250F:
      sspRich_MAROC_Regs[id][fiber][chip].Global0.bits.swb_buf_250f = val;
      break;
    case RICH_MAROC_REG_SWB_BUF_500F:
      sspRich_MAROC_Regs[id][fiber][chip].Global0.bits.swb_buf_500f = val;
      break;
    case RICH_MAROC_REG_SWB_BUF_1P:
      sspRich_MAROC_Regs[id][fiber][chip].Global0.bits.swb_buf_1p = val;
      break;
    case RICH_MAROC_REG_SWB_BUF_2P:
      sspRich_MAROC_Regs[id][fiber][chip].Global0.bits.swb_buf_2p = val;
      break;
    case RICH_MAROC_REG_ONOFF_SS:
      sspRich_MAROC_Regs[id][fiber][chip].Global0.bits.ONOFF_ss = val;
      break;
    case RICH_MAROC_REG_SW_SS_300F:
      sspRich_MAROC_Regs[id][fiber][chip].Global0.bits.sw_ss_300f = val;
      break;
    case RICH_MAROC_REG_SW_SS_600F:
      sspRich_MAROC_Regs[id][fiber][chip].Global0.bits.sw_ss_600f = val;
      break;
    case RICH_MAROC_REG_SW_SS1200F:
      sspRich_MAROC_Regs[id][fiber][chip].Global0.bits.sw_ss_1200f = val;
      break;
    case RICH_MAROC_REG_EN_ADC:
      sspRich_MAROC_Regs[id][fiber][chip].Global0.bits.EN_ADC = val;
      break;
    case RICH_MAROC_REG_H1H2_CHOICE:
      sspRich_MAROC_Regs[id][fiber][chip].Global0.bits.H1H2_choice = val;
      break;
    case RICH_MAROC_REG_SW_FSU_20F:
      sspRich_MAROC_Regs[id][fiber][chip].Global0.bits.sw_fsu_20f = val;
      break;
    case RICH_MAROC_REG_SW_FSU_40F:
      sspRich_MAROC_Regs[id][fiber][chip].Global0.bits.sw_fsu_40f = val;
      break;
    case RICH_MAROC_REG_SW_FSU_25K:
      sspRich_MAROC_Regs[id][fiber][chip].Global0.bits.sw_fsu_25k = val;
      break;
    case RICH_MAROC_REG_SW_FSU_50K:
      sspRich_MAROC_Regs[id][fiber][chip].Global0.bits.sw_fsu_50k = val;
      break;
    case RICH_MAROC_REG_SW_FSU_100K:
      sspRich_MAROC_Regs[id][fiber][chip].Global0.bits.sw_fsu_100k = val;
      break;
    case RICH_MAROC_REG_SW_FSB1_50K:
      sspRich_MAROC_Regs[id][fiber][chip].Global0.bits.sw_fsb1_50k = val;
      break;
    case RICH_MAROC_REG_SW_FSB1_100K:
      sspRich_MAROC_Regs[id][fiber][chip].Global0.bits.sw_fsb1_100k = val;
      break;
    case RICH_MAROC_REG_SW_FSB1_100F:
      sspRich_MAROC_Regs[id][fiber][chip].Global0.bits.sw_fsb1_100f = val;
      break;
    case RICH_MAROC_REG_SW_FSB1_50F:
      sspRich_MAROC_Regs[id][fiber][chip].Global0.bits.sw_fsb1_50f = val;
      break;
    case RICH_MAROC_REG_CMD_FSB_FSU:
      sspRich_MAROC_Regs[id][fiber][chip].Global0.bits.cmd_fsb_fsu = val;
      break;
    case RICH_MAROC_REG_VALID_DC_FS:
      sspRich_MAROC_Regs[id][fiber][chip].Global0.bits.valid_dc_fs = val;
      break;
    case RICH_MAROC_REG_SW_FSB2_50K:
      sspRich_MAROC_Regs[id][fiber][chip].Global0.bits.sw_fsb2_50k = val;
      break;
    case RICH_MAROC_REG_SW_FSB2_100K:
      sspRich_MAROC_Regs[id][fiber][chip].Global0.bits.sw_fsb2_100k = val;
      break;
    case RICH_MAROC_REG_SW_FSB2_100F:
      sspRich_MAROC_Regs[id][fiber][chip].Global0.bits.sw_fsb2_100f = val;
      break;
    case RICH_MAROC_REG_SW_FSB2_50F:
      sspRich_MAROC_Regs[id][fiber][chip].Global0.bits.sw_fsb2_50f = val;
      break;
    case RICH_MAROC_REG_VALID_DC_FSB2:
      sspRich_MAROC_Regs[id][fiber][chip].Global0.bits.valid_dc_fsb2 = val;
      break;
    case RICH_MAROC_REG_ENB_TRISTATE:
      sspRich_MAROC_Regs[id][fiber][chip].Global0.bits.ENb_tristate = val;
      break;
    case RICH_MAROC_REG_POLAR_DISCRI:
      sspRich_MAROC_Regs[id][fiber][chip].Global0.bits.polar_discri = val;
      break;
    case RICH_MAROC_REG_INV_DISCRIADC:
      sspRich_MAROC_Regs[id][fiber][chip].Global0.bits.inv_discriADC = val;
      break;
    case RICH_MAROC_REG_D1_D2:
      sspRich_MAROC_Regs[id][fiber][chip].Global1.bits.d1_d2 = val;
      break;
    case RICH_MAROC_REG_CMD_CK_MUX:
      sspRich_MAROC_Regs[id][fiber][chip].Global1.bits.cmd_CK_mux = val;
      break;
    case RICH_MAROC_REG_ONOFF_OTABG:
      sspRich_MAROC_Regs[id][fiber][chip].Global1.bits.ONOFF_otabg = val;
      break;
    case RICH_MAROC_REG_ONOFF_DAC:
      sspRich_MAROC_Regs[id][fiber][chip].Global1.bits.ONOFF_dac = val;
      break;
    case RICH_MAROC_REG_SMALL_DAC:
      sspRich_MAROC_Regs[id][fiber][chip].Global1.bits.small_dac = val;
      break;
    case RICH_MAROC_REG_ENB_OUTADC:
      sspRich_MAROC_Regs[id][fiber][chip].Global1.bits.enb_outADC = val;
      break;
    case RICH_MAROC_REG_INV_STARTCMPTGRAY:
      sspRich_MAROC_Regs[id][fiber][chip].Global1.bits.inv_startCmptGray = val;
      break;
    case RICH_MAROC_REG_RAMP_8BIT:
      sspRich_MAROC_Regs[id][fiber][chip].Global1.bits.ramp_8bit = val;
      break;
    case RICH_MAROC_REG_RAMP_10BIT:
      sspRich_MAROC_Regs[id][fiber][chip].Global1.bits.ramp_10bit = val;
      break;
    case RICH_MAROC_REG_DAC0:
      sspRich_MAROC_Regs[id][fiber][chip].DAC.bits.DAC0 = val;
      break;
    case RICH_MAROC_REG_DAC1:
      sspRich_MAROC_Regs[id][fiber][chip].DAC.bits.DAC1 = val;
      break;
    case RICH_MAROC_REG_GAIN:
      if(!(channel & 0x1))
        sspRich_MAROC_Regs[id][fiber][chip].CH[channel>>1].bits.Gain0 = val;
      else
        sspRich_MAROC_Regs[id][fiber][chip].CH[channel>>1].bits.Gain1 = val;
      break;
    case RICH_MAROC_REG_SUM:
      if(!(channel & 0x1))
        sspRich_MAROC_Regs[id][fiber][chip].CH[channel>>1].bits.Sum0 = val;
      else
        sspRich_MAROC_Regs[id][fiber][chip].CH[channel>>1].bits.Sum1 = val;
      break;
    case RICH_MAROC_REG_CTEST:
      if(!(channel & 0x1))
        sspRich_MAROC_Regs[id][fiber][chip].CH[channel>>1].bits.CTest0 = val;
      else
        sspRich_MAROC_Regs[id][fiber][chip].CH[channel>>1].bits.CTest1 = val;
      break;
    case RICH_MAROC_REG_MASKOR:
      if(!(channel & 0x1))
        sspRich_MAROC_Regs[id][fiber][chip].CH[channel>>1].bits.MaskOr0 = val;
      else
        sspRich_MAROC_Regs[id][fiber][chip].CH[channel>>1].bits.MaskOr1 = val;
      break;
    default:
      printf("%s: ERROR invalid register specified: %d\n", __func__, reg);
      return ERROR;
  }
  return OK;
}

int sspRich_GetMarocReg(int id, int fiber, int chip, int reg, int channel, int *val)
{
  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH) ||
     sspRich_IsFiberInvalid(id, fiber, __func__))
    return ERROR;
  
  if(chip < 0 || chip > 2)
  {
    printf("%s: ERROR invalid chip specified: %d\n", __func__, chip);
    return ERROR;
  }

  if(channel < 0 || channel > 64)
  {
    printf("%s: ERROR invalid channel specified: %d\n", __func__, channel);
    return ERROR;
  }

  switch(reg)
  {
    case RICH_MAROC_REG_CMD_FSU:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global0.bits.cmd_fsu;
      break;
    case RICH_MAROC_REG_CMD_SS:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global0.bits.cmd_ss;
      break;
    case RICH_MAROC_REG_CMD_FSB:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global0.bits.cmd_fsb;
      break;
    case RICH_MAROC_REG_SWB_BUF_250F:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global0.bits.swb_buf_250f;
      break;
    case RICH_MAROC_REG_SWB_BUF_500F:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global0.bits.swb_buf_500f;
      break;
    case RICH_MAROC_REG_SWB_BUF_1P:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global0.bits.swb_buf_1p;
      break;
    case RICH_MAROC_REG_SWB_BUF_2P:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global0.bits.swb_buf_2p;
      break;
    case RICH_MAROC_REG_ONOFF_SS:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global0.bits.ONOFF_ss;
      break;
    case RICH_MAROC_REG_SW_SS_300F:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global0.bits.sw_ss_300f;
      break;
    case RICH_MAROC_REG_SW_SS_600F:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global0.bits.sw_ss_600f;
      break;
    case RICH_MAROC_REG_SW_SS1200F:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global0.bits.sw_ss_1200f;
      break;
    case RICH_MAROC_REG_EN_ADC:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global0.bits.EN_ADC;
      break;
    case RICH_MAROC_REG_H1H2_CHOICE:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global0.bits.H1H2_choice;
      break;
    case RICH_MAROC_REG_SW_FSU_20F:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global0.bits.sw_fsu_20f;
      break;
    case RICH_MAROC_REG_SW_FSU_40F:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global0.bits.sw_fsu_40f;
      break;
    case RICH_MAROC_REG_SW_FSU_25K:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global0.bits.sw_fsu_25k;
      break;
    case RICH_MAROC_REG_SW_FSU_50K:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global0.bits.sw_fsu_50k;
      break;
    case RICH_MAROC_REG_SW_FSU_100K:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global0.bits.sw_fsu_100k;
      break;
    case RICH_MAROC_REG_SW_FSB1_50K:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global0.bits.sw_fsb1_50k;
      break;
    case RICH_MAROC_REG_SW_FSB1_100K:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global0.bits.sw_fsb1_100k;
      break;
    case RICH_MAROC_REG_SW_FSB1_100F:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global0.bits.sw_fsb1_100f;
      break;
    case RICH_MAROC_REG_SW_FSB1_50F:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global0.bits.sw_fsb1_50f;
      break;
    case RICH_MAROC_REG_CMD_FSB_FSU:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global0.bits.cmd_fsb_fsu;
      break;
    case RICH_MAROC_REG_VALID_DC_FS:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global0.bits.valid_dc_fs;
      break;
    case RICH_MAROC_REG_SW_FSB2_50K:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global0.bits.sw_fsb2_50k;
      break;
    case RICH_MAROC_REG_SW_FSB2_100K:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global0.bits.sw_fsb2_100k;
      break;
    case RICH_MAROC_REG_SW_FSB2_100F:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global0.bits.sw_fsb2_100f;
      break;
    case RICH_MAROC_REG_SW_FSB2_50F:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global0.bits.sw_fsb2_50f;
      break;
    case RICH_MAROC_REG_VALID_DC_FSB2:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global0.bits.valid_dc_fsb2;
      break;
    case RICH_MAROC_REG_ENB_TRISTATE:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global0.bits.ENb_tristate;
      break;
    case RICH_MAROC_REG_POLAR_DISCRI:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global0.bits.polar_discri;
      break;
    case RICH_MAROC_REG_INV_DISCRIADC:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global0.bits.inv_discriADC;
      break;
    case RICH_MAROC_REG_D1_D2:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global1.bits.d1_d2;
      break;
    case RICH_MAROC_REG_CMD_CK_MUX:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global1.bits.cmd_CK_mux;
      break;
    case RICH_MAROC_REG_ONOFF_OTABG:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global1.bits.ONOFF_otabg;
      break;
    case RICH_MAROC_REG_ONOFF_DAC:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global1.bits.ONOFF_dac;
      break;
    case RICH_MAROC_REG_SMALL_DAC:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global1.bits.small_dac;
      break;
    case RICH_MAROC_REG_ENB_OUTADC:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global1.bits.enb_outADC;
      break;
    case RICH_MAROC_REG_INV_STARTCMPTGRAY:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global1.bits.inv_startCmptGray;
      break;
    case RICH_MAROC_REG_RAMP_8BIT:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global1.bits.ramp_8bit;
      break;
    case RICH_MAROC_REG_RAMP_10BIT:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].Global1.bits.ramp_10bit;
      break;
    case RICH_MAROC_REG_DAC0:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].DAC.bits.DAC0;
      break;
    case RICH_MAROC_REG_DAC1:
      *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].DAC.bits.DAC1;
      break;
    case RICH_MAROC_REG_GAIN:
      if(!(channel & 0x1))
        *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].CH[channel>>1].bits.Gain0;
      else
        *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].CH[channel>>1].bits.Gain1;
      break;
    case RICH_MAROC_REG_SUM:
      if(!(channel & 0x1))
        *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].CH[channel>>1].bits.Sum0;
      else
        *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].CH[channel>>1].bits.Sum1;
      break;
    case RICH_MAROC_REG_CTEST:
      if(!(channel & 0x1))
        *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].CH[channel>>1].bits.CTest0;
      else
        *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].CH[channel>>1].bits.CTest1;
      break;
    case RICH_MAROC_REG_MASKOR:
      if(!(channel & 0x1))
        *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].CH[channel>>1].bits.MaskOr0;
      else
        *val = sspRich_MAROC_Regs_Rd[id][fiber][chip].CH[channel>>1].bits.MaskOr1;
      break;
    default:
      printf("%s: ERROR invalid register specified: %d\n", __func__, reg);
      return ERROR;
  }
  return OK;
}

int countOnes(unsigned int word)
{
  int i;
  int err=0;
  for(i=0; i<8*sizeof(word);i++)
  {
      if((word>>i)&0x1) err++;
  }
  return err;
}

int sspRich_CompareMarocRegs(MAROC_Regs *regsA , MAROC_Regs *regsB)
{
  int i;
  unsigned int diff;
  unsigned int err =0;
  unsigned int maskGlobal0 = 0xFFFFFFFF;
  unsigned int maskGlobal1 = 0x000001FF;
  unsigned int maskDAC =0x03FF03FF;
  unsigned int maskCH =0x0FFF0FFF;

  diff = (regsA->Global0.val & maskGlobal0) ^ (regsB->Global0.val & maskGlobal0);   // 32 bit XOR
  err += countOnes(diff);

  diff = (regsA->Global1.val & maskGlobal1) ^ (regsB->Global1.val & maskGlobal1);
  err += countOnes(diff);

  diff = (regsA->DAC.val & maskDAC) ^ (regsB->DAC.val & maskDAC);
  err += countOnes(diff);

  for(i = 0; i < 32; i++){
    diff = (regsA->CH[i].val & maskCH) ^( regsB->CH[i].val & maskCH);
    err += countOnes(diff);
  }
  return err;
}

int sspRich_DiscoveryAsic(int id, int fiber)
{
  printf("\n============== %s ===================== \n",__FUNCTION__);
  int err=0;
  int boardId;
  int i;

  MAROC_Regs wr[3];
  MAROC_Regs rd[3];
  MAROC_Regs dummy;

  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH) ||
     sspRich_IsFiberInvalid(id, fiber, __func__))
    return ERROR;

  for(i = 0; i < 3; i++) memset(&wr[i], 0, sizeof(MAROC_Regs));
  for(i = 0; i < 3; i++) memset(&rd[i], 0, sizeof(MAROC_Regs));

  // init registers with fake values
  wr[0].Global0.val = 0xAAAAAAAA;
  wr[0].Global1.val = 0xAAAAAAAA;
  wr[0].DAC.val = 0xAAAAAAAA;
  for(i = 0; i < 32; i++) wr[0].CH[i].val = 0xAAAAAAAA;

  wr[1].Global0.val = 0x55555555;
  wr[1].Global1.val = 0x55555555;
  wr[1].DAC.val = 0x55555555;
  for(i = 0; i < 32; i++) wr[1].CH[i].val = 0x55555555;

  wr[2].Global0.val = 0xFFFFFFFF;
  wr[2].Global1.val = 0xFFFFFFFF;
  wr[2].DAC.val = 0xFFFFFFFF;
  for(i = 0; i < 32; i++) wr[2].CH[i].val = 0xFFFFFFFF;



  // write on MAROC board assuming a 3 MAROC is connected
  sspRich_ShiftMarocRegs(id, fiber, &wr[2], &dummy);
  sspRich_ShiftMarocRegs(id, fiber, &wr[1], &dummy);
  sspRich_ShiftMarocRegs(id, fiber, &wr[0], &dummy);

  //  read from MAROC board what was written
  sspRich_ShiftMarocRegs(id, fiber, &wr[2], &rd[2]);
  sspRich_ShiftMarocRegs(id, fiber, &wr[1], &rd[1]);
  sspRich_ShiftMarocRegs(id, fiber, &wr[0], &rd[0]);

  // compare
  err=0;
  err +=sspRich_CompareMarocRegs(&wr[2],&rd[2]);
  err +=sspRich_CompareMarocRegs(&wr[1],&rd[1]);
  err +=sspRich_CompareMarocRegs(&wr[0],&rd[0]);

  if(err==0)
  {
    printf(" SUCCESSFULLY found MAROC ASIC (3) on slot %d, fiber %d\n", id, fiber);
    sspRichConnectedAsic[id][fiber]=SSP_RICH_ASIC_TYPE_3MAROC;
    return OK;
  }

  // write on MAROC board assuming a 3 MAROC is connected
  sspRich_ShiftMarocRegs(id, fiber, &wr[1], &dummy);
  sspRich_ShiftMarocRegs(id, fiber, &wr[0], &dummy);

  // read from MAROC board what was written
  sspRich_ShiftMarocRegs(id, fiber, &wr[1], &rd[1]);
  sspRich_ShiftMarocRegs(id, fiber, &wr[0], &rd[0]);

  // compare
  err=0;
  err +=sspRich_CompareMarocRegs(&wr[1],&rd[1]);
  err +=sspRich_CompareMarocRegs(&wr[0],&rd[0]);

  if(err==0)
  {
    printf(" SUCCESSFULLY found MAROC ASIC (2) on slot %d, fiber %d\n", id, fiber);
    sspRichConnectedAsic[id][fiber]=SSP_RICH_ASIC_TYPE_2MAROC;
    return OK;
  }

  printf(" ERROR - invalid MAROC ASIC found on slot %d, fiber %d\n", id, fiber);
  sspRichConnectedAsic[id][fiber]=SSP_RICH_ASIC_TYPE_0MAROC;
  return ERROR;
}

int sspRich_SetAllThreshold(int id, int thr)
{
  int chip, fiber;

  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH))
    return ERROR;

  for(fiber=0; fiber<RICH_FIBER_NUM; fiber++)
  {
    if(sspRichConnectedFibers[id] & (1<<fiber))
    {
      for(chip=0; chip<3; chip++)
        sspRich_SetMarocReg(id, fiber, chip, RICH_MAROC_REG_DAC0, 0, thr);

      printf("%s: Setting threshold to %3d on fiber %2d, ssp slot %2d\n", __func__, thr, fiber, id);
      sspRich_UpdateMarocRegs(id, fiber);
    }
  }
}

int sspRich_SelectSpi(int id, int fiber, int sel)
{
  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH) ||
     sspRich_IsFiberInvalid(id, fiber, __func__))
    return ERROR;

  SSPLOCK();
  if(sel)
    sspWriteReg(&pSSP[id]->rich.fiber[fiber].Clk.SpiCtrl, 0x200);
  else
    sspWriteReg(&pSSP[id]->rich.fiber[fiber].Clk.SpiCtrl, 0x100);
  SSPUNLOCK();

  return OK;
}

int sspRich_TransferSpi(int id, int fiber, unsigned char data)
{
	int i;
	unsigned int val;
  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH) ||
     sspRich_IsFiberInvalid(id, fiber, __func__))
    return ERROR;

  SSPLOCK();
	sspWriteReg(&pSSP[id]->rich.fiber[fiber].Clk.SpiCtrl, data | 0x400);
  SSPUNLOCK();

	for(i = 0; i < 1000; i++)
	{
    SSPLOCK();
		val = sspReadReg(&pSSP[id]->rich.fiber[fiber].Clk.SpiStatus);
    SSPUNLOCK();
		if(val & 0x800)
			break;
	}
	if(i == 1000)
		printf("%s: ERROR: Timeout!!!\n", __FUNCTION__);

	return val & 0xFF;
}

int sspRich_FlashGetId(int id, int fiber, unsigned char *rsp)
{
  int result[3];
  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH) ||
     sspRich_IsFiberInvalid(id, fiber, __func__))
    return ERROR;

  printf("%s - started\n", __func__);
  fflush(stdout);
	sspRich_SelectSpi(id, fiber, 1);
	sspRich_TransferSpi(id, fiber, RICH_FLASH_CMD_GETID);
	result[0] = sspRich_TransferSpi(id, fiber, 0xFF);
	result[1] = sspRich_TransferSpi(id, fiber, 0xFF);
	result[2] = sspRich_TransferSpi(id, fiber, 0xFF);
	sspRich_SelectSpi(id, fiber, 0);

  printf("ID = %02X,%02X,%02X\n", result[0], result[1], result[2]);
  fflush(stdout);

  if(rsp)
  {
    rsp[0] = result[0];
    rsp[1] = result[1];
    rsp[2] = result[2];
  }
  return OK;
}

int sspRich_FlashGetStatus(int id, int fiber, unsigned char cmd)
{
	unsigned char rsp;
  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH) ||
     sspRich_IsFiberInvalid(id, fiber, __func__))
    return ERROR;
	
	sspRich_SelectSpi(id, fiber, 1);
	sspRich_TransferSpi(id, fiber, cmd);
	rsp = sspRich_TransferSpi(id, fiber, 0xFF);
	sspRich_SelectSpi(id, fiber, 0);

  return rsp;
}

int sspRich_FirmwareUpdateVerify(int id, int fiber, const char *filename)
{
	int i, result;
  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH) ||
     sspRich_IsFiberInvalid(id, fiber, __func__))
    return ERROR;

	printf("Updating firmware(slot=%d,fiber=%d)...", id, fiber);
	result = sspRich_FirmwareUpdate(id, fiber, filename);
	if(result != OK)
	{
		printf("failed.\n");
		return result;
	}
	else
		printf("succeeded.\n");
	
	printf("\nVerifying...");
	result = sspRich_FirmwareVerify(id, fiber, filename);
	if(result != OK)
	{
		printf("failed.\n");
		return result;
	}
	else
		printf("ok.\n");

	return OK;
}

int sspRich_FirmwareUpdate(int id, int fiber, const char *filename)
{
	FILE *f;
	int i, flashId = 0;
	unsigned int addr = 0, page = 0;
	unsigned char buf[1056], rspId[3];
  if(sspIsNotInit(&id, __func__, SSP_CFG_SSPTYPE_HALLBRICH) ||
     sspRich_IsFiberInvalid(id, fiber, __func__))
    return ERROR;

	sspRich_SelectSpi(id, fiber, 0);
	sspRich_FlashGetId(id, fiber, rspId);
	
	printf("Flash: Mfg=0x%02X, Type=0x%02X, Capacity=0x%02X\n", rspId[0], rspId[1], rspId[2]);

	if( (rspId[0] == RICH_FLASH_MFG_MICRON) &&
	    (rspId[1] == (RICH_FLASH_DEVID_N25Q256A>>8)) &&
	    (rspId[2] == (RICH_FLASH_DEVID_N25Q256A&0xFF)) )
	{
		f = fopen(filename, "rb");
		if(!f)
		{
			printf("%s: ERROR: dcFirmwareUpdate invalid file %s\n", __FUNCTION__, filename);
			return ERROR;
		}
	
		memset(buf, 0xff, 256);
		while(fread(buf, 1, 256, f) > 0)
		{
		  if(!(addr % 65536))		/* sector erase*/
			{
				sspRich_SelectSpi(id, fiber, 1);
				sspRich_TransferSpi(id, fiber, RICH_FLASH_CMD_WREN);	/* write enable*/
				sspRich_SelectSpi(id, fiber, 0);

				sspRich_SelectSpi(id, fiber, 1);
				sspRich_TransferSpi(id, fiber, RICH_FLASH_CMD_ERASE64K);	/* 64k sector erase*/
				sspRich_TransferSpi(id, fiber, (addr>>24)&0xFF);
				sspRich_TransferSpi(id, fiber, (addr>>16)&0xFF);
				sspRich_TransferSpi(id, fiber, (addr>>8)&0xFF);
				sspRich_TransferSpi(id, fiber, (addr)&0xFF);
				sspRich_SelectSpi(id, fiber, 0);

				printf(".");
				i = 0;
				while(1)
				{
					if(!(sspRich_FlashGetStatus(id, fiber, RICH_FLASH_CMD_GETSTATUS) & 0x1))
						break;
					usleep(16000);
					if(i == 60+6)	/* 1000ms maximum sector erase time*/
					{
						fclose(f);
						printf("%s: ERROR: failed to erase flash\n", __FUNCTION__);
						return ERROR;
					}
					i++;
				}
			}

			sspRich_SelectSpi(id, fiber, 1);
			sspRich_TransferSpi(id, fiber, RICH_FLASH_CMD_WREN);	/* write enable*/
			sspRich_SelectSpi(id, fiber, 0);

			sspRich_SelectSpi(id, fiber, 1);
			sspRich_TransferSpi(id, fiber, RICH_FLASH_CMD_WRPAGE);	/* write page*/
			sspRich_TransferSpi(id, fiber, (addr>>24)&0xFF);
			sspRich_TransferSpi(id, fiber, (addr>>16)&0xFF);
			sspRich_TransferSpi(id, fiber, (addr>>8)&0xFF);
			sspRich_TransferSpi(id, fiber, (addr)&0xFF);
			for(i = 0; i < 256; i++)
				sspRich_TransferSpi(id, fiber, buf[i]);
			sspRich_SelectSpi(id, fiber, 0);

			i = 0;
			while(1)
			{
			  if(!(sspRich_FlashGetStatus(id, fiber, RICH_FLASH_CMD_GETSTATUS) & 0x1))	/* no faster than 1us per call*/
					break;
			  if(i == 3000)	/* 3ms maximum page program time*/
				{
					fclose(f);
					printf("%s: ERROR: failed to program flash\n", __FUNCTION__);
					return ERROR;
				}
				i++;
			}
			memset(buf, 0xff, 256);
			addr+= 256;
		}
		fclose(f);
	}
	else
	{
		printf("%s: ERROR: failed to identify flash id (or device not supported)\n", __FUNCTION__);
		return ERROR;
	}
	return OK;
}

int sspRich_FirmwareRead(int id, int fiber, const char *filename)
{
	FILE *f;
	int i,len, flashId = 0;
	unsigned int addr = 0;
	unsigned char buf[256];
	unsigned char rspId[3];

	sspRich_SelectSpi(id, fiber, 0);
	sspRich_FlashGetId(id, fiber, rspId);
	
	printf("Flash: Mfg=0x%02X, Type=0x%02X, Capacity=0x%02X\n", rspId[0], rspId[1], rspId[2]);

	if( (rspId[0] == RICH_FLASH_MFG_MICRON) &&
	    (rspId[1] == (RICH_FLASH_DEVID_N25Q256A>>8)) &&
	    (rspId[2] == (RICH_FLASH_DEVID_N25Q256A&0xFF)) )
	{
		f = fopen(filename, "wb");
		if(!f)
		{
			printf("%s: ERROR: dcFirmwareRead invalid file %s\n", __FUNCTION__, filename);
			return ERROR;
		}
		
		sspRich_SelectSpi(id, fiber, 1);
		sspRich_TransferSpi(id, fiber, RICH_FLASH_CMD_RD);	/* continuous array read */
		sspRich_TransferSpi(id, fiber, (addr>>24)&0xFF);
		sspRich_TransferSpi(id, fiber, (addr>>16)&0xFF);
		sspRich_TransferSpi(id, fiber, (addr>>8)&0xFF);
		sspRich_TransferSpi(id, fiber, (addr)&0xFF);
		
		for(i = 0; i < RICH_FLASH_BYTE_LENGTH; i++)
		{
			fputc(sspRich_TransferSpi(id, fiber, 0xFF), f);
			if(!(i% 65536))
			{
				printf(".");
				usleep(16000);
			}
		}
			
		sspRich_SelectSpi(id, fiber, 0);
		fclose(f);
	}
	else
	{
		printf("%s: ERROR: failed to identify flash id 0x%02X\n", __FUNCTION__, flashId);
		return ERROR;
	}
	return OK;
}

int sspRich_FirmwareVerify(int id, int fiber, const char *filename)
{
	FILE *f;
	int i,len, flashId = 0;
	unsigned int addr = 0;
	unsigned char buf[256];
	unsigned char rspId[3], val;
	
	sspRich_SelectSpi(id, fiber, 0);
	sspRich_FlashGetId(id, fiber, rspId);
	
	printf("Flash: Mfg=0x%02X, Type=0x%02X, Capacity=0x%02X\n", rspId[0], rspId[1], rspId[2]);

	if( (rspId[0] == RICH_FLASH_MFG_MICRON) &&
	    (rspId[1] == (RICH_FLASH_DEVID_N25Q256A>>8)) &&
	    (rspId[2] == (RICH_FLASH_DEVID_N25Q256A&0xFF)) )
	{
		f = fopen(filename, "rb");
		if(!f)
		{
			printf("%s: ERROR: dcFirmwareVerify invalid file %s\n", __FUNCTION__, filename);
			return ERROR;
		}
		
		sspRich_SelectSpi(id, fiber, 1);
		sspRich_TransferSpi(id, fiber, RICH_FLASH_CMD_RD);	/* continuous array read */
		sspRich_TransferSpi(id, fiber, (addr>>24)&0xFF);
		sspRich_TransferSpi(id, fiber, (addr>>16)&0xFF);
		sspRich_TransferSpi(id, fiber, (addr>>8)&0xFF);
		sspRich_TransferSpi(id, fiber, (addr)&0xFF);

		while((len = fread(buf, 1, 256, f)) > 0)
		{
			for(i = 0; i < len; i++)
			{
				val = sspRich_TransferSpi(id, fiber, 0xFF);
				if(buf[i] != val)
				{
					sspRich_SelectSpi(id, fiber, 0);
					fclose(f);					
					printf("%s: ERROR: failed verify at addess 0x%08X[%02X,%02X]\n", __FUNCTION__, addr+i, buf[i], val);
					return ERROR;
				}
			}
			addr+=256;
			if(!(addr & 0xFFFF))
				printf(".");
		}
		sspRich_SelectSpi(id, fiber, 0);
		fclose(f);
	}
	else
	{
		printf("%s: ERROR: failed to identify flash id 0x%02X\n", __FUNCTION__, flashId);
		return ERROR;
	}
	return OK;
}


int sspRich_Init(int id)
{
  int fiber, tries;

  memset(sspRich_MAROC_Regs, 0, sizeof(sspRich_MAROC_Regs));
  memset(sspRich_MAROC_Regs_Rd, 0, sizeof(sspRich_MAROC_Regs_Rd));
  memset(sspRichConnectedAsic, 0, sizeof(sspRichConnectedAsic));

  sspSetIOSrc(id, SD_SRC_TRIG2, SD_SRC_SEL_0);

  sspRich_ScanFibers(id);

  for(fiber=0; fiber<RICH_FIBER_NUM; fiber++)
  {
    if(sspRichConnectedFibers[id] & (1<<fiber))
    {
      sspRich_SetDeviceId(id, fiber, fiber);
      sspRich_SetSoftReset(id, fiber, 1);
      sspRich_SetSoftReset(id, fiber, 0);
      sspRich_SetCTestSource(id, fiber, RICH_SD_CTEST_SRC_SEL_0);

      // Reset slow control and dynamic register contents
      SSPLOCK();
      sspWriteReg(&pSSP[id]->rich.fiber[fiber].MAROC_Cfg.SerCtrl, 0);
      sspWriteReg(&pSSP[id]->rich.fiber[fiber].MAROC_Cfg.SerCtrl, 5);
      SSPUNLOCK();

      for(tries=0; tries<5; tries++)
      {
        if(sspRich_DiscoveryAsic(id, fiber)==OK)
          break;
      }
    }
  }
}

#endif
