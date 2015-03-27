/******************************************************************************
*
*  dcrbLib.h  - Driver library header file for readout of the dcrb boards
*
*  Author: Ber Raydo
*          Jefferson Lab Data Acquisition Group
*
*/

#ifndef __DCRBLIB__
#define __DCRBLIB__

#define DCRB_BOARD_ID       0x44435242
#define DCRB_MAX_BOARDS             20
#define DCRB_MAX_TDC_CHANNELS       96
#define DCRB_MAX_DATA_PER_CHANNEL  512
#define DCRB_MAX_A32_MEM      0x800000   /* 8 Meg */
#define DCRB_MAX_A32MB_SIZE   0x800000  /*  8 MB */
#define DCRB_VME_INT_LEVEL           3     
#define DCRB_VME_INT_VEC          0xDC
#define DCRB_MAX_DAC_VAL			 175


struct dcrb_struct {
/* 0x0000-0x0003 */ volatile unsigned int FirmwareRev;
/* 0x0004-0x0007 */ volatile unsigned int BoardID;
/* 0x0008-0x000B */ volatile unsigned int GrpBusyFifo;
/* 0x000C-0x000F */ volatile unsigned int GrpBusyTrig;
/* 0x0010-0x0013 */ volatile unsigned int GrpErrorFifo;
/* 0x0014-0x0017 */ volatile unsigned int SpiFlash;
/* 0x0018-0x001B */ volatile unsigned int ICap;
/* 0x001C-0x001F */ volatile unsigned int ADR32M;
/* 0x0020-0x0023 */ volatile unsigned int LookBack;
/* 0x0024-0x0027 */ volatile unsigned int WindowWidth;
/* 0x0028-0x002B */ volatile unsigned int BlockConfig;
/* 0x002C-0x002F */ volatile unsigned int TDCConfig;
/* 0x0030-0x0033 */ volatile unsigned int ClockConfig;
/* 0x0034-0x0037 */ volatile unsigned int TestPulseConfig;
/* 0x0038-0x003B */ volatile unsigned int DACConfig;
/* 0x003C-0x003F */ volatile unsigned int TriggerBusyThreshold;
/* 0x0040-0x0043 */ volatile unsigned int TriggerSource;
/* 0x0044-0x0047 */ volatile unsigned int ADR32;
/* 0x0048-0x004B */ volatile unsigned int Interrupt;
/* 0x004C-0x004F */ volatile unsigned int InterruptAck;
/* 0x0050-0x0053 */ volatile unsigned int Geo;
/* 0x0054-0x0057 */ volatile unsigned int FifoWordCnt;
/* 0x0058-0x005B */ volatile unsigned int FifoEventCnt;
/* 0x005C-0x005F */ volatile unsigned int ReadoutConfig;
/* 0x0060-0x0063 */ volatile unsigned int SRAMDebugAddr;
/* 0x0064-0x0067 */ volatile unsigned int SRAMDebugData;
/* 0x0068-0x006B */ volatile unsigned int Reset;
/* 0x006C-0x0077 */ volatile unsigned int ChDisable[3];
/* 0x0078-0x007B */ volatile unsigned int ScalerLatch;
/* 0x007C-0x007F */ volatile unsigned int TriggerLogic;
/* 0x0080-0x0083 */ volatile unsigned int FifoBlockCnt;
/* 0x0084-0x0087 */ volatile unsigned int IntWordCnt;
/* 0x0088-0x008B */ volatile unsigned int IntEventCnt;
/* 0x008C-0x008F */ volatile unsigned int IntBlockCnt;
/* 0x0090-0x0093 */ volatile unsigned int BusyWordCnt;
/* 0x0094-0x0097 */ volatile unsigned int BusyEventCnt;
/* 0x0098-0x009B */ volatile unsigned int BusyBlockCnt;

/* 0x009C-0x009F */ volatile unsigned int TriggerCtrl;
/* 0x00A0-0x00A3 */ volatile unsigned int GtpCtrl;
/* 0x00A4-0x00A7 */ volatile unsigned int GtpCtrlTile0;
/* 0x00A8-0x00AB */ volatile unsigned int GtpDrpCtrl;
/* 0x00AC-0x00AF */ volatile unsigned int GtpStatus;
/* 0x00B0-0x00B3 */ volatile unsigned int SoftErrCnt;
/* 0x00B4-0x00B7 */ volatile unsigned int PrbsErrCnt;
/* 0x00B8-0x0107 */ volatile unsigned int Reserved3[(0x0108-0x00B8)/4];
 
/* 0x0108-0x010B */ volatile unsigned int SwAGpio;
/* 0x010C-0x010F */ volatile unsigned int SwBGpio;
/* 0x0110-0x0113 */ volatile unsigned int TokenInCfg;
/* 0x0114-0x0117 */ volatile unsigned int TokenOutCfg;
/* 0x0118-0x011B */ volatile unsigned int SdLinkCfg;
/* 0x011C-0x011F */ volatile unsigned int TrigOutCfg;
/* 0x0120-0x0123 */ volatile unsigned int PulserPeriod;
/* 0x0124-0x0127 */ volatile unsigned int PulserHigh;
/* 0x0128-0x012B */ volatile unsigned int PulserN;
/* 0x012C-0x012F */ volatile unsigned int PulserStart;
/* 0x0130-0x0133 */ volatile unsigned int PulserStatus;
/* 0x0134-0x0FE7 */ volatile unsigned int Dummy1[(0x0FE8-0x0134)/4];
/* 0x0FE8-0x0FEB */ volatile unsigned int ScalerBusy;
/* 0x0FEC-0x0FEF */ volatile unsigned int ScalerBusyCycles;
/* 0x0FF0-0x0FF3 */ volatile unsigned int ScalerVmeClk;
/* 0x0FF4-0x0FF7 */ volatile unsigned int ScalerSync;
/* 0x0FF8-0x0FFB */ volatile unsigned int ScalerTrig1;
/* 0x0FFC-0x0FFF */ volatile unsigned int ScalerTrig2;
/* 0x1000-0x117F */ volatile unsigned int ScalerTDC[96];
};

struct dcrb_data_struct {
  unsigned int new_type;	
  unsigned int type;	
  unsigned int slot_id_hd;
  unsigned int slot_id_tr;
  unsigned int n_evts;
  unsigned int blk_num;
  unsigned int n_words;
  unsigned int evt_num;
  unsigned int time_now;
  unsigned int time_1;
  unsigned int time_2;
  unsigned int time_3;
  unsigned int time_4;
  unsigned int chan;
  unsigned int width;
  unsigned int valid_1;
  unsigned int adc_1;
  unsigned int valid_2;
  unsigned int adc_2;
  unsigned int over;
  unsigned int adc_sum;
  unsigned int pulse_num;
  unsigned int thres_bin;
  unsigned int quality;
  unsigned int integral;
  unsigned int time;
  unsigned int chan_a;
  unsigned int source_a;
  unsigned int chan_b;
  unsigned int source_b;
  unsigned int group;
  unsigned int time_coarse;
  unsigned int time_fine;
  unsigned int vmin;
  unsigned int vpeak;
  unsigned int trig_type_int;	/* next 4 for internal trigger data */
  unsigned int trig_state_int;	/* e.g. helicity */
  unsigned int evt_num_int;
  unsigned int err_status_int;
};

/* DCRB Special Board IDs */

/* Define ICap Bits */
#define DCRB_ICAP_WRITE           0x10000
#define DCRB_ICAP_CLK             0x20000
#define DCRB_ICAP_CE              0x40000

/* Define Init Flag bits for Clock Source */
#define DCRB_SOURCE_INT         0x00
#define DCRB_SOURCE_VXS         0x01
#define DCRB_SOURCE_MASK        0x01

/* Define ClockCfg Bits */
#define DCRB_REF_CLK_INTERNAL        0x0
#define DCRB_REF_CLK_P0              0x1
#define DCRB_REF_CLK_MASK            0x1
#define DCRB_REF_CLK_RESET           0x2
#define DCRB_TDC_CLK_LOCKED          0x4
#define DCRB_VME_CLK_LOCKED          0x8

#define DCRB_TRIG_P0                0x11
#define DCRB_TRIG_P0_SYNC           0x01
#define DCRB_TRIG_VME               0x08
#define DCRB_SYNC_P0               0x400
#define DCRB_SYNC_VME              0x800
#define DCRB_TRIG_MASK              0x1F
#define DCRB_SYNC_MASK            0x1F00

#define DCRB_ENABLE_BLKLVL_INT      0x40000
#define DCRB_ENABLE_BERR               0x01
#define DCRB_ENABLE_MULTIBLOCK    0x2000000
#define DCRB_FIRST_BOARD          0x4000000
#define DCRB_LAST_BOARD           0x8000000

/* Define Bit Masks */
#define DCRB_VERSION_MASK        0x0000ffff
#define DCRB_BOARD_MASK          0xffffffff
#define DCRB_EVENT_COUNT_MASK    0xffffff
#define DCRB_BLOCK_COUNT_MASK    0xfffff
#define DCRB_BLOCK_LEVEL_MASK    0x7ff
#define DCRB_INT_ENABLE_MASK     0x80000000
#define DCRB_INT_VEC_MASK        0xff
#define DCRB_INT_LEVEL_MASK      0x700
#define DCRB_SLOT_ID_MASK        0xf800

#define DCRB_A32_ENABLE        0x1
#define DCRB_AMB_ENABLE        0x2000000
#define DCRB_A32_ADDR_MASK     0xff80   /* 8 MB chunks */
#define DCRB_AMB_MIN_MASK      0xff80
#define DCRB_AMB_MAX_MASK      0xff800000

#define DCRB_MAX_LOOKBACK    65535
#define DCRB_MAX_WINDOW      65535
#define DCRB_MAX_DEADTIME      255
#define DCRB_DEFAULT_LOOKBACK 2000
#define DCRB_DEFAULT_WINDOW   1000
#define DCRB_DEFAULT_DEADTIEM   32

/* Define ADC Data Types and Masks */

#define DCRB_DATA_TYPE_DEFINE       0x80000000
#define DCRB_DATA_TYPE_MASK         0x78000000
#define DCRB_DATA_SLOT_MASK         0x07c00000

#define DCRB_DATA_BLOCK_HEADER      0x00000000
#define DCRB_DATA_BLOCK_TRAILER     0x08000000
#define DCRB_DATA_EVENT_HEADER      0x10000000
#define DCRB_DATA_TRIGGER_TIME      0x18000000
#define DCRB_DATA_INVALID           0x70000000
#define DCRB_DATA_FILLER            0x78000000
#define DCRB_DUMMY_DATA             0xf800fafa
#define DCRB_DATA_TDCEVT            0x40000000

#define DCRB_DATA_BLKNUM_MASK       0x0000003f
#define DCRB_DATA_WRDCNT_MASK       0x003fffff
#define DCRB_DATA_TRIGNUM_MASK      0x07ffffff

#define FLASH_CMD_WRPAGE			0x02
#define FLASH_CMD_RD				0x03
#define FLASH_CMD_GETSTATUS			0x05
#define FLASH_CMD_WREN				0x06
#define FLASH_CMD_GETID				0x9F
#define FLASH_CMD_ERASE64K			0xD8
#define FLASH_CMD_WRBUF1			0x84
#define FLASH_CMD_PGBUF1ERASE		0x83
#define FLASH_CMD_GETSTATUS2		0xD7

#define FLASH_BYTE_LENGTH			8*1024*1024
#define FLASH_MFG_WINBOND			0xEF
#define FLASH_DEVID_W25Q64			0x4017
#define FLASH_MFG_ATMEL				0x1F
#define FLASH_DEVID_AT45DB642D		0x2800


#define FNLEN     128       /* length of config. file name */
#define STRLEN    250       /* length of str_tmp */
#define ROCLEN     80       /* length of ROC_name */

/* Function Prototypes */

STATUS dcrbInit (UINT32 addr, UINT32 addr_inc, int ndcrb, int iFlag);
int dcrbHardReset(int id);
int dcrbSetClockSource(int id, int clkSrc);
void dcrbStatus(int id, int sflag);
void dcrbGStatus(int sflag);
int dcrbSetProcMode(int id, unsigned int lookBack, unsigned int windowWidth, unsigned int deadTime);
void dcrbGSetProcMode(unsigned int lookBack, unsigned int windowWidth, unsigned int deadTime);
int dcrbReadBlock(int id, volatile UINT32 *data, int nwrds, int rflag);
int dcrbReadBlockStatus(int id, volatile UINT32 *data, int nwrds, int rflag);
int dcrbPrintBlock(int id, int rflag);
void dcrbClear(int id);
void dcrbReset(int id, int iFlag);
void dcrbChanDisable(int id, unsigned int cmask0, unsigned int cmask1, unsigned int cmask2);
void dcrbTrig(int id);
void dcrbGTrig();
void dcrbSync(int id);
int dcrbDready(int id, int dflag);
int dcrbBready(int id);
unsigned int dcrbGBready();
unsigned int dcrbScanMask();
int dcrbBusyLevel(int id, unsigned int val, int bflag);
int dcrbBusy(int id);
void dcrbEnableSoftTrig(int id);
void dcrbGEnableSoftTrig();
void dcrbEnableSoftSync(int id);
void dcrbEnableBusError(int id);
void dcrbGEnableBusError();
void dcrbDisableBusError(int id);
void dcrbEnableMultiBlock();
void dcrbDisableMultiBlock();
int dcrbSetBlockLevel(int id, int level);
void dcrbGSetBlockLevel(int level);
int dcrbSetDAC(int id, unsigned int dvalue);
void dcrbGSetDAC(unsigned int dvalue);
int dcrbSetCalMask(int id, unsigned int dce_mask, unsigned fce_mask);
void dcrbGSetCalMask(unsigned int dce_mask, unsigned fce_mask);
void dcrbDataDecode(unsigned int data);
void dcrbGFirmwareUpdateVerify(const char *filename);

#endif /* __DCRBLIB__ */
