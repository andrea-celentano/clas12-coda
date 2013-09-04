#ifndef SSP_H
#define SSP_H

#define SSP_MODE_NORMAL		0x0
#define SSP_MODE_HPS		0x1

#define PULSER_CLOCK_FREQ		100000000

#define GPIO_NIM_NIMA			0
#define GPIO_NIM_NIMB			1

// Flags: SSP_regs.Config
#define CFG_CLOCK_SRC_MASK		0x00000003
#define CFG_CLOCK_SRC_SWA		(0x0<<0)
#define CFG_CLOCK_SRC_SWB		(0x1<<0)
#define CFG_CLOCK_SRC_SMA		(0x2<<0)
#define CFG_CLOCK_SRC_LOCAL	(0x3<<0)

#define CFG_RESET					(0x1<<2)

#define CFG_SYNC_SRC_MASK		0x00000038
#define CFG_SYNC_SRC_NIMA		(0x0<<3)
#define CFG_SYNC_SRC_NIMB		(0x1<<3)
#define CFG_SYNC_SRC_LVDS0		(0x2<<3)
#define CFG_SYNC_SRC_LVDS1		(0x3<<3)
#define CFG_SYNC_SRC_LVDS2		(0x4<<3)
#define CFG_SYNC_SRC_LVDS3		(0x5<<3)
#define CFG_SYNC_SRC_P0			(0x6<<3)
#define CFG_SYNC_SRC_NONE		(0x7<<3)

#define CFG_SYNC_SW				(0x1<<6)

// Flags: SSP_regs.SSPConfig
#define SSPCFG_MODE_MASK		0x000000FF
#define SSPCFG_MODE_SUM			0

#define SSPCFG_CHANNEL_MASK	0x0000FF00
#define SSPCFG_CHANNEL_NONE	(0x0<<8)
#define SSPCFG_CHANNEL_0		(0x1<<8)
#define SSPCFG_CHANNEL_1		(0x2<<8)
#define SSPCFG_CHANNEL_2		(0x4<<8)
#define SSPCFG_CHANNEL_3		(0x8<<8)
#define SSPCFG_CHANNEL_4		(0x10<<8)
#define SSPCFG_CHANNEL_5		(0x20<<8)
#define SSPCFG_CHANNEL_6		(0x40<<8)
#define SSPCFG_CHANNEL_7		(0x80<<8)
#define SSPCFG_CHANNEL_ALL		(0xFF<<8)

// Flags: SSP_regs.FLConfig
#define FLCFG_LATENCY_MASK		0x00003FFF
#define FLCFG_SLACK_MASK		0x3FFF0000
#define FLCFG_ERROR_FULL		(0x1<<30)
#define FLCFG_ERROR_EMPTY		(0x1<<31)

// Flags: SSP_regs.SPI0
#define SPI_MISO					(0x1<<0)
#define SPI_MOSI					(0x1<<1)
#define SPI_NSS					(0x1<<2)
#define SPI_SCLK					(0x1<<3)

// Flags: SSP_regs.LVDS_IO[] and SSP_regs.NIM_IO[]
#define GPIO_INPUT_MASK			0x00000001

#define GPIO_OUTPUT_MASK		0x00000002
#define GPIO_VMEREG_0			(0x0<<2)
#define GPIO_VMEREG_1			(0x1<<2)

#define GPIO_MUXSEL_MASK		0x000001F0
#define GPIO_MUXSEL_NIMA		(0x0<<4)
#define GPIO_MUXSEL_NIMB		(0x1<<4)
#define GPIO_MUXSEL_LVDS0		(0x2<<4)
#define GPIO_MUXSEL_LVDS1		(0x3<<4)
#define GPIO_MUXSEL_LVDS2		(0x4<<4)
#define GPIO_MUXSEL_LVDS3		(0x5<<4)
#define GPIO_MUXSEL_CLK100		(0x6<<4)
#define GPIO_MUXSEL_CLK200		(0x7<<4)
#define GPIO_MUXSEL_CLK125		(0x8<<4)
#define GPIO_MUXSEL_CLK250		(0x9<<4)
#define GPIO_MUXSEL_PULSER0	(0xA<<4)
#define GPIO_MUXSEL_PULSER1	(0xB<<4)
#define GPIO_MUXSEL_PULSER2	(0xC<<4)
#define GPIO_MUXSEL_PULSER3	(0xD<<4)
#define GPIO_MUXSEL_PULSER4	(0xE<<4)
#define GPIO_MUXSEL_VMEREG		(0xF<<4)
#define GPIO_MUXSEL_DISC0		(0x10<<4)
#define GPIO_MUXSEL_DISC1		(0x11<<4)
#define GPIO_MUXSEL_DISC2		(0x12<<4)
#define GPIO_MUXSEL_DISC3		(0x13<<4)

// Flags: SSP_regs.Pulse_IO[].PeriodCycles
#define PULSER_PERIOD_MASK		0xFFFFFFFF

// Flags: SSP_regs.Pulse_IO[].HighCycles
#define PULSER_HIGH_MASK		0xFFFFFFFF

// Flags: SSP_regs.DDRStatus[]
#define DDR_STATUS_RDY_MASK	0x80000000
#define DDR_STATUS_TAP_A		0x0000003F
#define DDR_STATUS_TAP_B		0x00000F30
#define DDR_STATUS_TAP_C		0x0003F000

// Flags: SSP_regs.Discriminators[].Mode
#define DISC_MODE_MASK			0x00000003
#define DISC_MODE_DISABLE		(0x0<<0)
#define DISC_MODE_NONUPDATING	(0x1<<0)
#define DISC_MODE_UPDATING		(0x2<<0)
#define DISC_MODE_TOT			(0x3<<0)

// Flags: SSP_regs.Discriminators[].ModeThreshold
#define DISC_THRESHOLD_MASK	0xFFFFFFFF

// Flags: SSP_regs.Discriminators[].ModePulseWidth
#define DISC_THRESHOLD_MASK	0xFFFFFFFF

// Flags: SSP_regs.LinkA.Control and SSP_regs.LinkB.Control
#define LINK_CTRL_RESET			(0x1<<0)
#define LINK_CTRL_PRBS_RX_EN	(0x3<<2)
#define LINK_CTRL_PRBS_TX_EN	(0x3<<4)
#define LINK_CTRL_TSTTX_EN		(0x1<<6)
#define LINK_CTRL_TSTTX_MASK	(0x3FFFF<<8)

// Flags: SSP_regs.LinkA.Status and SSP_regs.LinkB.Status
#define LINK_STATUS_CH_MASK	0x000F0000
#define LINK_STATUS_LANE_MASK	0x00F00000
#define LINK_STATUS_REM_MASK	0x0F000000
#define LINK_STATUS_GTX_RDY	0x10000000
#define LINK_STATUS_DDR_RDY	0x20000000

// Flags: SSP_regs.HPSRegs.TriggerCutEnable
#define HPS_TRIGCUT_EN_CLUSTER_SUM			0x1
#define HPS_TRIGCUT_EN_CLUSTER_DIFF			0x2
#define HPS_TRIGCUT_EN_CLUSTER_ENERGYDIST	0x4
#define HPS_TRIGCUT_EN_CLUSTER_COPLANAR		0x8
#define HPS_TRIGMODE_PAIRS					(0x0<<4)
#define HPS_TRIGMODE_TOPCLUSTERS			(0x1<<4)
#define HPS_TRIGMODE_BOTCLUSTERS			(0x2<<4)
#define HPS_TRIGMODE_DISABLED				(0x3<<4)

// Flags: SSP_regs.LinkA.FECStatus and SSP_regs.LinkB.FECStatus
// Flags: SSP_regs.LinkA.LaneStatus01 and SSP_regs.LinkB.LaneStatus01 and SSP_regs.LinkA.LaneStatus23 and SSP_regs.LinkB.LaneStatus23
// Flags: SSP_regs.LinkA.GTXDRP and SSP_regs.LinkB.GTXDRP
// Flags: SSP_regs.LinkA.PRBSStatus and SSP_regs.LinkB.PRBSStatus

/*#ifdef WIN32
	#pragma pack(push)
	#pragma pack(1)
#else
	#error "Set structure packing here"
#endif*/

typedef struct
{
	unsigned int PeriodCycles;
	unsigned int HighCycles;
} Pulser;

typedef struct
{
	unsigned int Mode;
	unsigned int Threshold;
	unsigned int PulseWidth;
} Discriminator;

typedef struct
{
	unsigned int Control;
	unsigned int Status;
	unsigned int FECStatus;
	unsigned int LaneStatus01;
	unsigned int LaneStatus23;
	unsigned int GTXDRP;
	unsigned int PRBSStatus;
	unsigned int Reserved[1];
} FiberLink;

// Offset HPS struct in SSP to 0x0700
typedef struct
{
/* 0x0000-0x0003 */ volatile unsigned int Coincidence;
/* 0x0004-0x0007 */ volatile unsigned int PairSumMax;
/* 0x0008-0x000B */ volatile unsigned int PairDiffMax;
/* 0x000C-0x000F */ volatile unsigned int EDSlope;
/* 0x0010-0x0013 */ volatile unsigned int EMax;
/* 0x0014-0x0017 */ volatile unsigned int EMin;
/* 0x0018-0x001B */ volatile unsigned int Delay;
/* 0x001C-0x001F */ volatile unsigned int TriggerLatency;
/* 0x0020-0x0023 */ volatile unsigned int TriggerCutEnable;
/* 0x0024-0x0027 */ volatile unsigned int RAMEDSlope;
/* 0x0028-0x002B */ volatile unsigned int RAMEDSlopeData;
/* 0x002C-0x002F */ volatile unsigned int RAMCoplanar;
/* 0x0030-0x0033 */ volatile unsigned int RAMCoplanarData;
/* 0x0034-0x0037 */ volatile unsigned int TriggerWidth;
/* 0x0038-0x003B */ volatile unsigned int EventFifoCfg;
/* 0x003C-0x003F */ volatile unsigned int EventFifoData;
/* 0x0040-0x0043 */ volatile unsigned int EventPatternDelay;
/* 0x0044-0x00FF */ volatile unsigned int Reserved1[(0x0100-0x0044)/4];
/* 0x0100-0x0103 */ volatile unsigned int ScalerBottomCluster;
/* 0x0104-0x0107 */ volatile unsigned int ScalerTopCluster;
/* 0x0108-0x010B */ volatile unsigned int ScalerBottomClusterDisc;
/* 0x010C-0x010F */ volatile unsigned int ScalerTopClusterDisc;
/* 0x0110-0x012F */ volatile unsigned int ScalerPairs[8];
/* 0x0130-0x014F */ volatile unsigned int ScalerSumCut[8];
/* 0x0150-0x016F */ volatile unsigned int ScalerDiffCut[8];
/* 0x0170-0x018F */ volatile unsigned int ScalerEDSlopeCut[8];
/* 0x0190-0x01AF */ volatile unsigned int ScalerCoplanarCut[8];
/* 0x01B0-0x01CF */ volatile unsigned int ScalerTriggerCut[8];
/* 0x01D0-0x01FF */ volatile unsigned int Reserved2[(0x0200-0x01D0)/4];
/* 0x0200-0x0203 */ volatile unsigned int TopEnergyHistData;
/* 0x0204-0x0207 */ volatile unsigned int TopEnergyHistTime;
/* 0x0208-0x020B */ volatile unsigned int BottomEnergyHistData;
/* 0x020C-0x020F */ volatile unsigned int BottomEnergyHistTime;
/* 0x0210-0x0213 */ volatile unsigned int TopPositionHistData;
/* 0x0214-0x0217 */ volatile unsigned int TopPositionHistTime;
/* 0x0218-0x021B */ volatile unsigned int BottomPositionHistData;
/* 0x021C-0x021F */ volatile unsigned int BottomPositionHistTime;
/* 0x0220-0x0223 */ volatile unsigned int TopLatencyHistData;
/* 0x0224-0x0227 */ volatile unsigned int TopLatencyHistTime;
/* 0x0228-0x022B */ volatile unsigned int BottomLatencyHistData;
/* 0x022C-0x022F */ volatile unsigned int BottomLatencyHistTime;
} HPSRegs;

typedef struct
{
/* 0x0000-0x0003 */ volatile unsigned int Config;
/* 0x0004-0x0007 */ volatile unsigned int SSPConfig;
/* 0x0008-0x000B */ volatile unsigned int FLConfig;
/* 0x000C-0x000F */ volatile unsigned int SPI0;
/* 0x0010-0x0013 */ volatile unsigned int RevisionMain;
/* 0x0014-0x00FF */ volatile unsigned int Reserved1[(0x0100-0x0014)/4];
/* 0x0100-0x010F */ volatile unsigned int LVDS_IO[4];
/* 0x0110-0x011F */ volatile unsigned int Reserved2[(0x0120-0x0110)/4];
/* 0x0120-0x0127 */ volatile unsigned int NIM_IO[2];
/* 0x0128-0x014F */ volatile Pulser Pulse_IO[5];
/* 0x0150-0x01FF */ volatile unsigned int Reserved3[(0x0200-0x0150)/4];
/* 0x0200-0x023F */ volatile unsigned int DDRStatus[16];
/* 0x0240-0x02FF */ volatile unsigned int Reserved4[(0x0300-0x0240)/4];
/* 0x0300-0x032F */ volatile Discriminator Discriminators[4];
/* 0x0330-0x03FF */ volatile unsigned int Reserved5[(0x400-0x0330)/4];
/* 0x0400-0x0403 */ volatile unsigned int HistEnable;
/* 0x0404-0x0407 */ volatile unsigned int HistLogEnable;
/* 0x0408-0x042B */ volatile unsigned int HistData[9];
/* 0x042C-0x044F */ volatile unsigned int HistOffset[9];
/* 0x0450-0x0473 */ volatile unsigned int HistShift[9];
/* 0x0474-0x04FF */ volatile unsigned int Reserved6[(0x0500-0x0474)/4];
/* 0x0500-0x0503 */ volatile unsigned int WaveConfig;
/* 0x0504-0x0527 */ volatile unsigned int WaveThreshold[9];
/* 0x0528-0x054B */ volatile unsigned int WaveData[9];
/* 0x054C-0x05FF */ volatile unsigned int Reserved7[(0x0600-0x054C)/4];
/* 0x0600-0x0603 */ volatile unsigned int ScalerLatch;
/* 0x0604-0x0607 */ volatile unsigned int ScalerCLK100;
/* 0x0608-0x060B */ volatile unsigned int ScalerCLK250;
/* 0x060C-0x060F */ volatile unsigned int ScalerSync;
/* 0x0610-0x0613 */ volatile unsigned int ScalerTrig1;
/* 0x0614-0x0617 */ volatile unsigned int ScalerTrig2;
/* 0x0618-0x061B */ volatile unsigned int ScalerNIMInA;
/* 0x061C-0x061F */ volatile unsigned int ScalerNIMInB;
/* 0x0620-0x0623 */ volatile unsigned int ScalerNIMOutA;
/* 0x0624-0x0627 */ volatile unsigned int ScalerNIMOutB;
/* 0x0628-0x0637 */ volatile unsigned int ScalerLVDSIn[4];
/* 0x0638-0x0647 */ volatile unsigned int ScalerLVDSOut[4];
/* 0x0648-0x06FF */ volatile unsigned int Reserved8[(0x0700-0x0648)/4];
/* 0x0700-0x092F */ volatile HPSRegs  HPS;
/* 0x0930-0x7FFF */ volatile unsigned int Reserved9[(0x8000-0x0930)/4];
/* 0x8000-0x801F */ volatile unsigned int TestRegA[8];
/* 0x8020-0x8023 */ volatile unsigned int SPI1;
/* 0x8024-0x8027 */ volatile unsigned int RevisionFiber1;
/* 0x8028-0x80FF */ volatile unsigned int Reserved10[(0x8100-0x8028)/4];
/* 0x8100-0x817F */ volatile FiberLink LinkA[4];
/* 0x8180-0xBFFF */ volatile unsigned int Reserved11[(0xC000-0x8180)/4];
/* 0xC000-0xC01F */ volatile unsigned int TestRegB[8];
/* 0xC020-0xC023 */ volatile unsigned int SPI2;
/* 0xC024-0xC027 */ volatile unsigned int RevisionFiber2;
/* 0xC028-0xC0FF */ volatile unsigned int Reserved12[(0xC100-0xC028)/4];
/* 0xC100-0xC17F */ volatile FiberLink LinkB[4];
/* 0xC180-0xFFFF */ volatile unsigned int Reserved13[(0x10000-0xC180)/4];
} SSP_regs;

/*#ifdef WIN32
	#pragma pack(pop)
#else
	#error "Restore structure packing here"
#endif*/

void sspInit(unsigned int addr_a24);
void sspSetupPulser(Pulser *pPulser, unsigned int period, unsigned int hightime);
void sspSetupOutput(unsigned int *pIOReg, int mux_sel, int val, 
		    int disable_output, int delay);
int  sspGetInput(unsigned int *pIOReg);
void sspSetupNIM(int port, int mux_sel, int val, int disable_output, int delay);
void sspSetupLVDS(int port, int mux_sel, int val, int disable_output, int delay);
void sspSetupDiscriminator(int index, unsigned int threshold, unsigned width, int mode);
void sspSetupPulserIndex(int index, float period, float duty);
void sspFiberDisableAll();
void sspFiberEnableAll();
void sspFiberEnable(int port);
void sspFiberDisable(int port);
/* HPS dependent routines */
void sspSetCoincidence(unsigned int window);
void sspSetEnergySumMaximum(unsigned int max);
void sspSetPairEnergyDifferenceMaximum(unsigned int max);
void sspSetPairEnergyDistanceThreshold(unsigned int thres);
void sspSetEnergyMaximum(unsigned int max);
void sspSetEnergyMinimum(unsigned int min);
void sspSetClusterDelay(unsigned int delay);
void sspSetTriggerLatency(unsigned int late);
void sspSetTriggerCutEnableMask(unsigned int mask);
void sspInit_HPS(unsigned int addr_a24);


void sspSetTriggerWidth(int width);
void sspSetEventPatternDelay(int delay);
void sspClearEventFifo();
int sspReadFifo(unsigned int *buf);
void sspPrintEventPatternDelay();


#endif
