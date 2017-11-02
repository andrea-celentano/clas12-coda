/* v1725.c - CAEN FADC driver */

#if defined(VXWORKS) || defined(Linux_vme)

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#ifdef VXWORKS
#include <vxWorks.h>
#include <logLib.h>
#include <taskLib.h>
#include <intLib.h>
#include <iv.h>
#include <vxLib.h>
#else
#include "jvme.h"
#endif

#include "v1725.h"

/* Define external Functions */
#ifdef VXWORKS
IMPORT STATUS sysBusToLocalAdrs(int, char *, char **);
IMPORT STATUS intDisconnect(int);
IMPORT STATUS sysIntEnable(int);
IMPORT STATUS sysIntDisable(int);
IMPORT STATUS sysVmeDmaDone(int, int);
IMPORT STATUS sysVmeDmaSend(UINT32, UINT32, int, BOOL);

#define EIEIO    __asm__ volatile ("eieio")
#define SYNC     __asm__ volatile ("sync")
#endif

/* Macro to check id and c1190p */
#define CHECKID(id) {							\
		if((id<0) || (c1725p[id] == NULL)) {				\
			logMsg("%s: ERROR : ADC id %d not initialized \n",		\
					(int)__FUNCTION__,id,3,4,5,6);				\
					return ERROR;							\
		}									\
}

#ifdef VXWORKS
#define EIEIO    __asm__ volatile ("eieio")
#define SynC     __asm__ volatile ("sync")
#else
#define EIEIO    
#define SynC     
#endif

/* Mutex to hold of reads and writes from competing threads */
pthread_mutex_t c1725_mutex = PTHREAD_MUTEX_INITIALIZER;
#define LOCK_1725 {				\
		if(pthread_mutex_lock(&c1725_mutex)<0)	\
		perror("pthread_mutex_lock");	\
}
#define UNLOCK_1725 {				\
		if(pthread_mutex_unlock(&c1725_mutex)<0)	\
		perror("pthread_mutex_unlock");	\
}

		unsigned int adcAddrOffsetv1725 = 0; /* Difference in CPU (USERSPACE) Base */

		int Nc1725 = 0; /* Number of ADCs in crate */
		int v1725ID[V1725_MAX_MODULES]; /* array of IDs for FADCs indexed by slot number */
		int v1725Slot[V1725_MAX_MODULES]; /* array of slot numbers for FADCs indexed by id */
		unsigned int v1725AddrList[V1725_MAX_MODULES]; /* array of addresses for FADCs indexed by id */
		unsigned int c1725vme[V1725_MAX_MODULES]; /* jumper addresses for Dma */

		volatile struct v1725_struct *c1725p[V1725_MAX_MODULES]; /* pointers to ADC memory map */

		/* readout options */
		static int cblt_not,
a24_a32, sngl_blt_mblt, berr_fifo;

/* variables filled by DmaStart and used by DmaDone */
static int ndata_save[21];
static int extra_save[21];
static int size_save[21];

/*******************************************************************************
 *
 * v1725Init - Initialization
 *
 * RETURNS: the number of boards detected, or ERROR if the address is invalid
 *   or board is not present.
 */

int v1725Init(UINT32 addr, UINT32 addr_inc, int nadc, int iFlag) {
	int ii, jj, res = 0, errFlag = 0;
	unsigned short rdata = 0;
	int boardID = 0;
	unsigned int laddr;
	unsigned int firmwareRev;
	volatile struct v1725_ROM_struct *rp;

	/* Check for valid address */
	if (addr == 0) {
		printf("v1725Init: ERROR: Must specify a Bus (VME-based A32/A24) address for board 0\n");
		return (ERROR);
	} else if (addr < 0x00ffffff) /* A24 Addressing */
	{
		a24_a32 = 1;
		if ((addr_inc == 0) || (nadc == 0))
			nadc = 1; /* assume only one board to initialize */

		/* get the board address */
#ifdef VXWORKS
		res = sysBusToLocalAdrs(0x39,(char *)addr,(char **)&laddr);
		if (res != 0)
		{
			printf("v1725Init: ERROR in sysBusToLocalAdrs(0x39,0x%x,&laddr) \n",addr);
			return(ERROR);
		}
		adcAddrOffsetv1725 = (unsigned int)laddr-(unsigned int)addr;
#else
		res = vmeBusToLocalAdrs(0x39, (char *) addr, (char **) &laddr);
		if (res != 0) {
			printf("v1725Init: ERROR in vmeBusToLocalAdrs(0x39,0x%x,&laddr) \n", addr);
			return (ERROR);
		}
		adcAddrOffsetv1725 = (unsigned int) laddr - (unsigned int) addr;
#endif
	} else /* A32 Addressing */
	{
		a24_a32 = 2;
		if ((addr_inc == 0) || (nadc == 0))
			nadc = 1; /* assume only one board to initialize */

		/* get the board address */
#ifdef VXWORKS
		res = sysBusToLocalAdrs(0x09,(char *)addr,(char **)&laddr);
		if (res != 0)
		{
			printf("v1725Init: ERROR in sysBusToLocalAdrs(0x09,0x%x,&laddr) \n",addr);
			return(ERROR);
		}
		adcAddrOffsetv1725 = (unsigned int)laddr-(unsigned int)addr;
#else
		res = vmeBusToLocalAdrs(0x09, (char *) addr, (char **) &laddr);
		if (res != 0) {
			printf("v1725Init: ERROR in vmeBusToLocalAdrs(0x09,0x%x,&laddr) - res: %i \n", addr, res);
			return (ERROR);
		}
		adcAddrOffsetv1725 = (unsigned int) laddr - (unsigned int) addr;
#endif
	}

	Nc1725 = 0;
	for (ii = 0; ii < nadc; ii++) {
		printf("ADDR: iteration %i, local -> %x vme-> %x \n", ii, laddr, addr);
		c1725p[ii] = (struct v1725_struct *) (laddr + ii * addr_inc); /*This points to the local address the VME is mapped to*/

		/* Check if Board exists at that address */
#ifdef VXWORKS
		res = vxMemProbe((char *) &(c1725p[ii]->roc_fpga_firmware_rev),0,2,(char *)&rdata);
#else
		res = vmeMemProbe((char *) &(c1725p[ii]->roc_fpga_firmware_rev), 2, (char *) &rdata);
#endif
		if (res < 0)
		/*       if(res < 0 || rdata==0xffff)  */
		{
			printf("v1725Init: ERROR: No addressable board at addr=0x%x\n", (UINT32) c1725p[ii]);
			c1725p[ii] = NULL;
			errFlag = 1;
			break;
		} else {
			/* Check if this is a Model 1725 */
			rp = (struct v1725_ROM_struct *) ((UINT32) c1725p[ii] + V1725_ROM_OFFSET);
			boardID = ((vmeRead32(&(rp->boardID1)) & (0xff)) << 8) + (vmeRead32(&(rp->boardID0)) & (0xff));
			if (boardID != V1725_BOARD_ID) {
				printf("v1725Init: ERROR: Board ID does not match: 0x%x expected 0x%x \n", boardID, V1725_BOARD_ID);
				break;
			}

			/*Get the board ID from GEO - actually this works in VME64x only!*/
			v1725Slot[ii] = v1725GetGeoAddress(ii);
			v1725ID[v1725Slot[ii]] = ii;

			firmwareRev = vmeRead32(&(c1725p[ii]->roc_fpga_firmware_rev));
			firmwareRev &= 0xFFFF; //take 16 LSB where fir. rev is coded
			/* Check if this is the firmware we expect V1725_FIRMWARE_REV */
			if ((firmwareRev != V1725_FIRMWARE_REV) && (firmwareRev != (V1725_FIRMWARE_REV1))) {
				printf("0x%08x \n", vmeRead32(&(c1725p[ii]->roc_fpga_firmware_rev)));
				fflush(stdout);
				printf("WARN: Firmware does not match: 0x%08x (expected 0x%08x or 0x%08x)\n", rdata, V1725_FIRMWARE_REV,
				V1725_FIRMWARE_REV1);
				fflush(stdout);
				break;
			}
		}
		c1725vme[ii] = (addr & 0xFF000000) + ii * addr_inc;
		printf("Initialized ADC ID %2d at address 0x%08x - dma 0x%08x \n", ii, (UINT32) c1725p[ii], c1725vme[ii]);

		Nc1725++;
	}

	/* Disable/Clear all ADCs */
	for (ii = 0; ii < Nc1725; ii++) {
		vmeWrite32(&(c1725p[ii]->sw_reset), 1);
		vmeWrite32(&(c1725p[ii]->sw_clear), 1);
	}
	/*
	 #ifndef VXWORKS
	 usrVmeserToPhysOffset(adcAddrOffsetv1725);
	 #endif
	 */

	v1725ConfigBoard(iFlag);

	return (Nc1725);
}

int v1725GetNfadc() {
	return Nc1725;
}

int v1725GetSlot(unsigned int id) {
	if (id >= Nc1725) {
		printf("%s: ERROR: Index (%d) >= v1725 initialized (%d).\n", __FUNCTION__, id, Nc1725);
		return ERROR;
	}

	return v1725Slot[id];
}

int v1725GetID(unsigned int slot) {
	return v1725ID[slot];
}

/*input:
 sng_blt_mblt: bits 8-15.
 berr_fifo: bits 0 - 7
 */

STATUS v1725ConfigBoard(int options) {
	int ii, ifull;
	unsigned short value, value0, value1, array0[32], array1[32], data16;
	unsigned short dat16[21][10];
	short tmp[5], tdata;
	unsigned int ww, wm, wr;
	int wo;

	printf("v1725ConfigBoard starts with options: %i \n", options);

	/*These are defaults!*/
	berr_fifo = 0; //use BERR
	sngl_blt_mblt = 0x5;
	/****************************/
	/* setting boards parameters */

	for (ii = 0; ii < Nc1725; ii++) {
		v1725SetAllChannels(ii, 1, 0, 0); /*negative polarity, no overlap, no test pattern*/
		v1725SetBuffer(ii, 0x0A); /*1024 buffers 640-10 samples max size*/
		v1725SetAcquisition(ii, 0, 0, 0, 0, 0, 0, 0); /*last two zeros: acquisition STOP, Sw-controlled*/
		v1725SetTriggerEnableMask(ii, 0, 1, 1, 0, 0, 0);/*(board,sw trigger,ext trigger,lvds trigger,coinc level,coinc_window,chan mask)*/
		v1725SetFrontPanelTriggerOutEnableMask(ii, 0, 1, 1, 0, 0, 0); /*board,sw,ext,lvds,majority,trg_out_logic,chan_mask*/
		v1725SetBLTRange(ii, 0);
		/*
		 input
		 mobo_probe: 2-bit mask. 0- RUN, 1- CLKout, 2- clkphase, 3- busy
		 trg_out: 4-bit mask. Bit 0:  0 - force trg_out to 0, 1 - force trg_out to 1, Bit 1: 0 - normal, 1- fixed leveld according to previous, Bit 2-3: 0- normal, 1- mobo probe, 2- channel probe, 3-S IN propagation.
		 trg_in: 2-bit mask. Bit 0: 0- trg-in formed as a pulse, 1- not formed. Bit 1: 0- standard, 1- TRG-in sent directly to mezzanines
		 lvds: 8-bit mask. Bit 0-3: sets LVDS signals [i,i*4-1] to be 0 - input, 1 - output. Bit 4-5:  LVDS I/O signal config - old features. 00 - GPIO, 1- programmable I/O, 2- pattern mode. Bit 6:  0 - keep old features, 1 - use new features. Bit 7: 0 - latch when global trigger is sent to channels, 1- latch when external trigger arrives
		 trg_out_enable : 0 enable, 1 high Z
		 lemo_level:  0 - LEMO are NIM, 1 - LEMO are TTL
		 */
		v1725SetFrontPanelIO(ii, 0, 0, 0, 0x4E, 0, 0); //enable new features, first 4 are inputs, last 12 are outputs (0xe=1110)
		/*
		 * input
		 * chA,chB,chC,chD: 4-bit mask, configuration of LVDS I/O blocks (0..3)/(4..7)/(8..11)/(12..15)
		 * 0000 Register
		 * 0001 TRIGGER
		 * 0010 nBUSY/nVETO
		 * 0011 LEGACY
		 */
		//A(0..3): nBUSY-nVETO (IN)
		//B(4..7): nBUSY-nVETO (OUT)
		//C(8..11): TRIGGER (OUT)
		//D(12..15): TRIGGER (OUT)
		v1725SetNewFeaturesIO(ii, 0x2, 0x2, 0x1, 0x1);

		v1725SetPostTrigger(ii, 16);
		v1725SetChannelEnableMask(ii, 0xFFFF);
		v1725SetBLTEventNumber(ii, 1);
		//		 v1725SetAlmostFullLevel(ii, 16384);
	}

	/********************/
	/* readout settings */

	printf("enable VME BUS error\n");
	for (ii = 0; ii < Nc1725; ii++) {
		v1725SetBusError(ii, 1);
	}

	/* enable 64-bit alignment */
	/* NOTE: 64-bit alignment must be enabled for any DMA readout,
	 not only for MBLT(D64) but for BLT(D32) as well; some DMA engines
	 (for example the one on mv5100) will cut off last 32-bit word if
	 it is not even, for example if event size is equal to 137 D32 words,
	 only 136 D32 words will be transfered */
	/* in addition universe library contains check for 64-bit alignment
	 in dmastart procedure, it will return error if ... */
	printf("enable align 64\n");
	for (ii = 0; ii < Nc1725; ii++) {
		v1725SetAlign64(ii, 1);
	}

	return OK;
}

/*******************/
/* low-level calls */

STATUS v1725Clear(int id) {
	CHECKID(id);
	LOCK_1725;
	vmeWrite32(&(c1725p[id]->sw_clear), 1);
	UNLOCK_1725;
	return OK;
}

STATUS v1725Reset(int id) {
	CHECKID(id);
	LOCK_1725;
	vmeWrite32(&(c1725p[id]->sw_reset), 1);
	UNLOCK_1725;
	return OK;
}

STATUS v1725SetChannelGain(int id, int ch, int gain) {
	unsigned int mask;
	if ((gain < 0) || (gain > 1)) {
		printf("v1725SetChannelGain ERROR: bad parameter\n");
		return ERROR;
	}
	mask = gain;
	CHECKID(id);
	LOCK_1725;
	vmeWrite32(&(c1725p[id]->chan[ch].gain), mask);
	UNLOCK_1725;
	return OK;
}

int v1725GetChannelGain(int id, int ch) {
	int gain;
	CHECKID(id);
	LOCK_1725;
	gain = vmeRead32(&(c1725p[id]->chan[ch].gain));
	UNLOCK_1725;
	return gain;
}

STATUS v1725SetChannelPulseWidth(int id, int ch, int width) {
	unsigned int mask;
	if ((width < 0) || (width > 255)) {
		printf("v1725SetPulseWidth ERROR: bad parameter\n");
		return ERROR;
	}
	mask = width & 0xff;
	CHECKID(id);
	LOCK_1725;
	vmeWrite32(&(c1725p[id]->chan[ch].pulse_width), mask);
	UNLOCK_1725;
	return OK;
}

STATUS v1725SetChannelThreshold(int id, int ch, int threshold) {
	unsigned int mask;

	if ((threshold < 0) || (threshold > 16383)) {
		printf("v1725SetChannelThreshold ERROR: bad parameter(s)\n");
		return ERROR;
	}

	mask = threshold & 0x3FFF;

	CHECKID(id);
	LOCK_1725;
	vmeWrite32(&(c1725p[id]->chan[ch].threshold), mask);
	UNLOCK_1725;
	return OK;
}

int v1725GetChannelThreshold(int id, int ch) {
	int threshold;
	CHECKID(id);
	LOCK_1725;
	threshold = vmeRead32(&(c1725p[id]->chan[ch].threshold));
	threshold = threshold & 0x3FFF;
	UNLOCK_1725;
	return threshold;
}

STATUS v1725SetSelfTriggerLogic(int id, int ch, int logic, int pulse) {
	unsigned int mask;

	if ((logic < 0) || (logic > 3) || (pulse < 0) || (pulse > 1)) {
		printf("v1725SetSelfTriggerLogic ERROR: bad parameter(s)\n");
		return ERROR;
	}

	mask = logic;
	mask = mask | (pulse << 2);
	CHECKID(id);
	LOCK_1725;
	vmeWrite32(&(c1725p[id]->chan[ch].self_trigger_logic), mask);
	UNLOCK_1725;
	return OK;
}

STATUS v1725SetChannelPedestal(int id, int ch, int pedestal) { /*This function will simply write to the DUMMY32 register, nothing will physically happen to the board*/
	if (pedestal < 0) {
		printf("v1725SetChannelPedestal ERROR: bad paramter(s) %i\n", pedestal);
	}
	CHECKID(id);
	LOCK_1725;
	vmeWrite32(&(c1725p[id]->chan[ch].dummy32), pedestal);
	UNLOCK_1725;
	return OK;
}

int v1725GetChannelPedestal(int id, int ch) { /*This function will simply read the DUMMY32 register, nothing will physically happen to the board*/
	int pedestal;
	CHECKID(id);
	LOCK_1725;
	pedestal = vmeRead32(&(c1725p[id]->chan[ch].dummy32));
	UNLOCK_1725;
	return pedestal;
}

STATUS v1725SetChannelDACOffset(int id, int ch, int offset) {
	unsigned int mask;
	unsigned int ret;
	ret = 1;
	if (offset < 0 || offset > 0xFFFF) {
		printf("v1725SetChannelDACOffset ERROR: bad parameter(s)\n");
		return ERROR;
	}

	mask = offset;
	while (ret != 0) {
		LOCK_1725;
		ret = vmeRead32(&(c1725p[id]->chan[ch].status)) & V1725_CH_DAC_BUSY;
		UNLOCK_1725;
	}

	CHECKID(id);
	LOCK_1725;
	vmeWrite32(&(c1725p[id]->chan[ch].dac_offset), mask);
	UNLOCK_1725;

	LOCK_1725;
	ret = vmeRead32(&(c1725p[id]->chan[ch].status)) & V1725_CH_DAC_BUSY;
	UNLOCK_1725;
	return OK;
}

unsigned int v1725GetChannelDACOffset(int id, int ch) {
	unsigned int offset;
	unsigned int ret;

	CHECKID(id);
	LOCK_1725;
	offset = vmeRead32(&(c1725p[id]->chan[ch].dac_offset));
	UNLOCK_1725;

	return offset;
}

STATUS v1725PrintChannelDACOffset(int id, int ch) {

	unsigned int ret;
	ret = 0;

	CHECKID(id);
	LOCK_1725;
	ret = vmeRead32(&(c1725p[id]->chan[ch].dac_offset));
	UNLOCK_1725;

	printf("The DAC Offset for FADC %i, ch %i is: %i \n", id, ch, ret);

	return OK;
}

STATUS v1725PrintChannelTemperature(int id, int ch) {

	unsigned int ret;
	ret = 0;

	CHECKID(id);
	LOCK_1725;
	ret = vmeRead32(&(c1725p[id]->chan[ch].temperature_monitor));
	UNLOCK_1725;

	printf("The Temperature monitor for FADC %i, ch %i reads: %i \n", id, ch, ret);

	return OK;
}

/*
 inputs:
 trigger_polarity: 0 - positive,1 - negative
 trigger_overlap:  0 - trigger overlap not allowed, 1 - trigger overlap allowed
 test_pattern:     0 - test pattern disabled, 1 - test pattern enabled

 */
STATUS v1725SetAllChannels(int id, int trigger_polarity, int test_pattern, int trigger_overlap) {
	unsigned int mask;

	if (test_pattern < 0 || test_pattern > 1 || trigger_overlap < 0 || trigger_overlap > 1 || trigger_polarity < 0
			|| trigger_polarity > 1) {
		printf("v1725SetAllChannels ERROR: bad parameter(s)\n");
		return ERROR;
	}

	mask = (trigger_polarity << 6) + (1 << 4) + (test_pattern << 3) + (trigger_overlap << 1);

	CHECKID(id);
	LOCK_1725;
	vmeWrite32(&(c1725p[id]->chan_config), mask);
	UNLOCK_1725;
	return OK;
}

STATUS v1725SetBuffer(int id, int code) {
	unsigned int mask;

	if (code < 0 || code > 0x0A) {
		printf("v1725SetBuffer ERROR: bad parameter(s)\n");
		return ERROR;
	}

	mask = code;

	CHECKID(id);
	LOCK_1725;
	vmeWrite32(&(c1725p[id]->buffer_organization), mask);
	UNLOCK_1725;
	return OK;
}

int v1725GetBuffer(int id) {
	int buffer;
	CHECKID(id);
	LOCK_1725;
	buffer = vmeRead32(&(c1725p[id]->buffer_organization));
	UNLOCK_1725;
	return buffer;
}

STATUS v1725SetBufferSizePerEvent(int id, int nloc) {
	unsigned int mask;

	mask = nloc;

	CHECKID(id);
	LOCK_1725;
	vmeWrite32(&(c1725p[id]->custom_size), mask);
	UNLOCK_1725;
	return OK;
}

/*
 inputs:
 lvds_runin: 0 - start on RunIN level, 1 - start on RunIN rising edge
 lvds_veto:  0 - disables, 1 - enable
 lvds_busy:  0 - disable lvds Busy IN, 1 - enable lvds Busy IN
 memory_full:0 - memory full when all buffer full, 1 - memory full when N-1 buffers full. N=nbr. of blocks
 trig_count: 0 - count accepted triggers, 1 - count all triggers
 acquisition_armed: 0 - STOP, 1 - RUN or ARMED
 start_stop: 0 - sw-controlled, 1 - s-in controlled, 2 - first-trigger controlled, 3 - gpio controlled
 */
STATUS v1725SetAcquisition(int id, int lvds_runin, int lvds_veto, int lvds_busy, int memory_full, int trig_count, int acquisition_armed,
		int start_stop) {
	unsigned int mask;

	if (start_stop < 0 || start_stop > 3 || acquisition_armed < 0 || acquisition_armed > 1 || trig_count < 0 || trig_count > 1
			|| memory_full < 0 || memory_full > 1 || lvds_busy < 0 || lvds_busy > 1 || lvds_veto < 0 || lvds_veto > 1 || lvds_runin < 0
			|| lvds_runin > 1) {
		printf("v1725SetAcquisition ERROR: bad parameter(s)\n");
		return ERROR;
	}

	mask = (lvds_runin << 11) + (lvds_veto << 9) + (lvds_busy << 8) + (memory_full << 5) + (trig_count << 3) + (acquisition_armed << 2)
			+ start_stop;

	CHECKID(id);
	LOCK_1725;
	vmeWrite32(&(c1725p[id]->acquisition_control), mask);
	UNLOCK_1725;
	return OK;
}

STATUS v1725GenerateSoftwareTrigger(int id) {
	CHECKID(id);
	LOCK_1725;
	vmeWrite32(&(c1725p[id]->sw_trigger), 1);
	UNLOCK_1725;
	return OK;
}

/*
 inputs:
 sw_trig: 0-software trigger disabled, 1-software trigger enabled
 ext_trig 0-external trigger disabled, 1-external trigger enabled
 coinc_level: 1-at least 2 channels above threshold required to generate trigger, 2-at least 3, and so on
 coinc_window: set the coincidence window (4 bits)
 chan_mask: 8-bit mask enabling/disabling channel triggers, bit 0 is channel pair 0 (ch 0 and 1), bit 7 is channel pair 7 (ch 14 and 15)
 */
STATUS v1725SetTriggerEnableMask(int id, int sw_trig, int ext_trig, int lvds_trig, int coinc_level, int coinc_window, int chan_mask) {
	unsigned int mask;

	if (sw_trig < 0 || sw_trig > 1 || ext_trig < 0 || ext_trig > 1 || lvds_trig < 0 || lvds_trig > 1 || coinc_level < 0 || coinc_level > 7
			|| coinc_window < 0 || coinc_window > 15 || chan_mask < 0 || chan_mask > 0xFF) {
		printf("v1725SetTriggerEnableMask ERROR: bad parameter(s)\n");
		return ERROR;
	}

	mask = (sw_trig << 31) + (ext_trig << 30) + (lvds_trig << 29) + (coinc_level << 24) + (coinc_window << 20) + chan_mask;

	CHECKID(id);
	LOCK_1725;
	vmeWrite32(&(c1725p[id]->trigger_source_enable_mask), mask);
	UNLOCK_1725;
	return OK;
}

/*
 inputs:
 software: enable software output
 external_trigger: enable external output
 lvds_trig: enable LVDS output if enabled as output. If enabled, these are the trigger requests coming from the mezzanines.
 majority: set the majority level if trg_out_logic=2. Values between 0 and 7
 trg_out_logic: 0- OR, 1- AND, 2- MAJORITY
 chan_mask: 8 bit enable out from (internal) channel pairs
 */

STATUS v1725SetFrontPanelTriggerOutEnableMask(int id, int sw_trig, int ext_trig, int lvds_trig, int majority, int trg_out_logic,
		int chan_mask) {
	unsigned int mask;

	if (sw_trig < 0 || sw_trig > 1 || ext_trig < 0 || ext_trig > 1 || lvds_trig < 0 || lvds_trig > 1 || majority < 0 || majority > 1
			|| trg_out_logic < 0 || trg_out_logic > 2 || chan_mask < 0 || chan_mask > 0xFF) {
		printf("v1725SetFontPanelTriggerOutEnableMask ERROR: bad parameter(s)\n");
		return ERROR;
	}

	mask = (sw_trig << 31) + (ext_trig << 30) + (lvds_trig << 29) + (majority << 10) + (trg_out_logic << 8) + chan_mask;

	CHECKID(id);
	LOCK_1725;
	vmeWrite32(&(c1725p[id]->front_panel_trigger_out_enable_mask), mask);
	UNLOCK_1725;
	return OK;
}

STATUS v1725SetPostTrigger(int id, int mask) {

	if (mask < 0 || mask > 0xFFFF) {
		printf("v1725SetPostTrigger ERROR: bad parameter(s)\n");
		return ERROR;
	}
	CHECKID(id);
	LOCK_1725;
	vmeWrite32(&(c1725p[id]->post_trigger_setting), mask);
	UNLOCK_1725;
	return OK;
}

int v1725GetPostTrigger(int id) {
	int post_trigger;
	CHECKID(id);
	LOCK_1725;
	post_trigger = vmeRead32(&(c1725p[id]->post_trigger_setting));
	UNLOCK_1725;
	return post_trigger;
}

STATUS v1725SetFrontPanelIOData(int id, int data) {
	unsigned int mask;

	if (data < 0 || data > 0xFFFF) {
		printf("v1725SetFontPanelIOData ERROR: bad parameter(s)\n");
		return ERROR;
	}

	mask = data;

	CHECKID(id);
	LOCK_1725;
	vmeWrite32(&(c1725p[id]->front_panel_io_data), mask);
	UNLOCK_1725;
	return OK;
}

/*
 input
 mobo_probe: 2-bit mask. 0- RUN, 1- CLKout, 2- clkphase, 3- busy
 trg_out: 4-bit mask. Bit 0:  0 - force trg_out to 0, 1 - force trg_out to 1, Bit 1: 0 - normal, 1- fixed leveld according to previous, Bit 2-3: 0- normal, 1- mobo probe, 2- channel probe, 3-S IN propagation.
 trg_in: 2-bit mask. Bit 0: 0- trg-in formed as a pulse, 1- not formed. Bit 1: 0- standard, 1- TRG-in sent directly to mezzanines
 lvds: 8-bit mask. Bit 0-3: sets LVDS signals [i,i*4-1] to be 0 - input, 1 - output. Bit 4-5:  LVDS I/O signal config - old features. 00 - GPIO, 1- programmable I/O, 2- pattern mode. Bit 6:  0 - keep old features, 1 - use new features. Bit 7: 0 - latch when global trigger is sent to channels, 1- latch when external trigger arrives
 trg_out_enable : 0 enable, 1 high Z
 lemo_level:  0 - LEMO are NIM, 1 - LEMO are TTL
 */

STATUS v1725SetFrontPanelIO(int id, int mobo_probe, int trg_out, int trg_in, int lvds, int trg_out_enable, int lemo_level) {
	unsigned int mask;

	if (mobo_probe < 0 || mobo_probe > 3 || trg_out < 0 || trg_out > 15 || trg_in < 0 || trg_in > 3 || lvds < 0 || lvds > 255
			|| trg_out_enable < 0 || trg_out_enable > 1 || lemo_level < 0 || lemo_level > 1) {
		printf("v1725SetFontPanelIO ERROR: bad parameter(s)\n");
		return ERROR;
	}

	mask = ((0x1) << 21) + (mobo_probe << 18) + (trg_out << 14) + (trg_in << 10) + (lvds << 2) + (trg_out_enable << 1) + lemo_level;

	CHECKID(id);
	LOCK_1725;
	vmeWrite32(&(c1725p[id]->front_panel_io_control), mask);
	UNLOCK_1725;

	printf("v1725SetFrontPanelIO called, mask is: 0x%08x readout is: 0x%08x  -- lvds: 0x%08x\n", mask,
			vmeRead32(&(c1725p[id]->front_panel_io_control)), lvds);
	return OK;
}

/*
 * input
 * chA,chB,chC,chD: 4-bit mask, configuration of LVDS I/O blocks (0..3)/(4..7)/(8..11)/(12..15)
 * 0000 Register
 * 0001 TRIGGER
 * 0010 nBUSY/nVETO
 * 0011 LEGACY
 */
STATUS v1725SetNewFeaturesIO(int id, int chA, int chB, int chC, int chD) {
	unsigned int mask;
	if (chA < 0 || chA > 15 || chB < 0 || chB > 15 || chC < 0 || chC > 15 || chD < 0 || chD > 15) {
		printf("v1725SetNewFeaturesIO ERROR: bad parameter(s)\n");
	}
	mask = (chD << 12) + (chC << 8) + (chB << 4) + chA;
	CHECKID(id);
	LOCK_1725;
	vmeWrite32(&(c1725p[id]->front_panel_lvds_io_new_features), mask);
	UNLOCK_1725;
	return OK;
}

STATUS v1725SetChannelEnableMask(int id, int mask) {
	if (mask < 0 || mask > 0xFFFF) {
		printf("v1725SetChannelEnableMask ERROR: bad parameter(s) --> 0x%04x <---\n", mask);
		return ERROR;
	}

	CHECKID(id);
	LOCK_1725;
	vmeWrite32(&(c1725p[id]->channel_enable_mask), mask);
	UNLOCK_1725;
	return OK;
}

int v1725GetChannelEnableMask(int id) {
	int mask;
	CHECKID(id);
	LOCK_1725;
	mask = vmeRead32(&(c1725p[id]->channel_enable_mask));
	UNLOCK_1725;
	return mask;
}

STATUS v1725SWClockSync(int id) {
	int val = 1;
	CHECKID(id);
	LOCK_1725;
	vmeWrite32(&(c1725p[id]->sw_clk_sync), val);
	UNLOCK_1725;
	return OK;
}

STATUS v1725SetMonitorDAC(int id, int mask) {
	if (mask < 0 || mask > 0xFFF) {
		printf("v1725SetMonitorDAC ERROR: bad parameter(s)\n");
		return ERROR;
	}

	CHECKID(id);
	LOCK_1725;
	vmeWrite32(&(c1725p[id]->monitor_dac), mask);
	UNLOCK_1725;
	return OK;
}

STATUS v1725SetMonitorMode(int id, int mask) {
	if (mask < 0 || mask > 4) {
		printf("v1725SetMonitorMode ERROR: bad parameter(s)\n");
		return ERROR;
	}

	CHECKID(id);
	LOCK_1725;
	vmeWrite32(&(c1725p[id]->monitor_mode), mask);
	UNLOCK_1725;
	return OK;
}

STATUS v1725SetMemoryBufferAlmostFullLevel(int id, int level) {
	if (level < 0 || level > 1023) {
		printf("v1725SetMemoryBufferAlmostFullLevel ERROR: bad parameter(s)\n");
		return ERROR;
	}

	CHECKID(id);
	LOCK_1725;
	vmeWrite32(&(c1725p[id]->memory_buffer_almost_full_level), level);
	UNLOCK_1725;
	return OK;

}

/*****/

STATUS v1725SetGeoAddress(int id, UINT16 geoaddr) {
	CHECKID(id);
	LOCK_1725;
	vmeWrite32(&(c1725p[id]->geo_address), geoaddr & 0x1F);
	UNLOCK_1725;
	return OK;
}

int v1725GetGeoAddress(int id) {
	int rval;

	CHECKID(id);
	LOCK_1725;
	rval = vmeRead32(&(c1725p[id]->geo_address)) & 0x1F;
	UNLOCK_1725;
	return rval;
}

STATUS v1725SetRunStartStopDelay(int id, int delay) {
	if (delay < 0 || delay >= 0xFFFFFFFF) {
		printf("v1725SetRunStartStopDelay ERROR: bad parameter(s)\n");
		return ERROR;
	}
	CHECKID(id);
	LOCK_1725;
	vmeWrite32(&(c1725p[id]->run_start_stop_delay), delay);
	UNLOCK_1725;
	return OK;
}

STATUS v1725SetBLTEventNumber(int id, int nevents) {
	if (nevents < 0 || nevents > 0xFF) {
		printf("v1725SetBLTEventNumber ERROR: bad parameter(s)\n");
		return ERROR;
	}

	CHECKID(id);
	LOCK_1725;
	vmeWrite32(&(c1725p[id]->blt_event_number), nevents);
	UNLOCK_1725;
	return OK;
}

int v1725GetBLTEventNumber(int id) {
	int rval;

	CHECKID(id);
	LOCK_1725;
	rval = vmeRead32(&(c1725p[id]->blt_event_number)) & 0xFF;
	UNLOCK_1725;
	return rval;
}

/*******************************************************************************
 * RETURNS: 1 if Full, 0 if Not Full
 */

int v1725ChannelStatusFull(int id, int ch) {
	int res;

	CHECKID(id);

	if ((ch < 0) || (ch >= V1725_MAX_CHANNELS)) {
		printf("v1725ChannelStatusFull ERROR: bad parameter(s)\n");
		return ERROR;
	}

	LOCK_1725;
	res = vmeRead32(&(c1725p[id]->chan[ch].status)) & 0x1;
	UNLOCK_1725;

	return (res);
}

int v1725StatusFull(int id) {
	int res;

	CHECKID(id);

	LOCK_1725;
	res = vmeRead32(&(c1725p[id]->readout_status)) & V1725_ACQSTATUS_BUFFER_FULL;
	UNLOCK_1725;

	return (res != 0 ? 1 : 0);
}

int v1725GetBusError(int id) {
	int res;

	CHECKID(id);

	LOCK_1725;
	res = vmeRead32(&(c1725p[id]->readout_status)) & V1725_STATUS_BERR_FLAG;
	UNLOCK_1725;

	return (res);
}

/******************************************************************************
 * RETURNS: 0(No Data) or the number of events
 */

int v1725Dready(int id) {
	UINT16 stat = 0, nevents;

	CHECKID(id);

	LOCK_1725;
	stat = vmeRead32(&(c1725p[id]->readout_status)) & V1725_STATUS_DATA_READY;
	if (stat) {
		nevents = vmeRead32(&(c1725p[id]->event_stored));
		UNLOCK_1725;
		return (nevents);
	} else {
		UNLOCK_1725;
		return (0);
	}
}

/*returns the number of words in the next event*/
int v1725GetNextEventSize(int id) {
	UINT32 ret;

	CHECKID(id);

	LOCK_1725;
	ret = vmeRead32(&(c1725p[id]->event_size));
	UNLOCK_1725;

	return (ret);
}

/******************************************************************************
 *
 * v1725SetBusError - Enable/Disable Bus Errors (to finish a block transfer,
 *                   or on an empty buffer read)
 *                    flag = 0 : Disable
 *                           1 : Enable
 *
 * RETURNS: 0 if successful, ERROR otherwise.
 */

int v1725SetBusError(int id, UINT32 flag) {
	unsigned int reg;

	CHECKID(id);

	if (flag < 0 || flag > 1) {
		printf("v1725SetBusError ERROR: Invalid flag = %d", flag);
		return ERROR;
	}

	LOCK_1725;

	reg = vmeRead32(&(c1725p[id]->readout_control));

	if (flag == 1) {
		printf("set BUSerror\n");
		reg = reg | V1725_BERR_ENABLE;
	} else if (flag == 0) {
		printf("reset BUSerror\n");
		reg = reg & ~V1725_BERR_ENABLE;
	}

	vmeWrite32(&(c1725p[id]->readout_control), reg);

	UNLOCK_1725;

	return (0);
}

/******************************************************************************
 *
 * v1725SetAlign64 - Enable/Disable 64 bit alignment for block transfers
 *                   flag = 0 : Disable
 *                          1 : Enable
 *
 * RETURNS: 0 if successful, ERROR otherwise.
 */

int v1725SetAlign64(int id, UINT32 flag) {
	unsigned int reg;

	CHECKID(id);

	if (flag < 0 || flag > 1) {
		printf("v1725SetAlign64 ERROR: Invalid flag = %d", flag);
		return ERROR;
	}

	LOCK_1725;

	reg = vmeRead32(&(c1725p[id]->readout_control));

	if (flag == 1) {
		printf("set Align64\n");
		reg = reg | V1725_ALIGN64;
	} else if (flag == 0) {
		printf("reset Aligh64\n");
		reg = reg & ~V1725_ALIGN64;
	}

	vmeWrite32(&(c1725p[id]->readout_control), reg);

	UNLOCK_1725;

	return (0);
}

int v1725SetRelocation(int id, UINT32 flag) {
	unsigned int reg;

	CHECKID(id);

	if (flag < 0 || flag > 1) {
		printf("v1725SetRelocation ERROR: Invalid fg = %d", flag);
		return ERROR;
	}

	LOCK_1725;

	reg = vmeRead32(&(c1725p[id]->readout_control));

	if (flag == 1) {
		printf("set RELOC\n");
		reg = reg | V1725_RELOC;
	} else if (flag == 0) {
		printf("reset RELOC\n");
		reg = reg & ~V1725_RELOC;
	}

	vmeWrite32(&(c1725p[id]->readout_control), reg);

	UNLOCK_1725;

	return (0);
}

int v1725SetBLTRange(int id, UINT32 flag) {
	unsigned int reg;

	CHECKID(id);

	if (flag < 0 || flag > 1) {
		printf("v1725SetBLTRange ERROR: Invalid fg = %d", flag);
		return ERROR;
	}

	LOCK_1725;

	reg = vmeRead32(&(c1725p[id]->readout_control));

	if (flag == 1) {
	//	printf("set BLT_RANGE\n");
		reg = reg | V1725_BLT_RANGE;
	} else if (flag == 0) {
	//	printf("reset BLT_RANGE\n");
		reg = reg & ~V1725_BLT_RANGE;
	}

	vmeWrite32(&(c1725p[id]->readout_control), reg);

	UNLOCK_1725;

	return (0);
}

STATUS v1725SetRelocationAddress(int id, UINT32 address) {
	if (address < 0 || address > 0xFFFF) {
		printf("v1725SetRelocationAddress ERROR: bad parameter(s)\n");
		return ERROR;
	}

	CHECKID(id);
	LOCK_1725;
	vmeWrite32(&(c1725p[id]->relocation_address), address);
	UNLOCK_1725;
	return OK;
}

unsigned int v1725GetRelocationAddress(int id) {
	UINT32 ret;
	CHECKID(id);
	LOCK_1725;
	ret = vmeRead32(&(c1725p[id]->relocation_address));
	UNLOCK_1725;
	return (ret);
}

int v1725ReadOne(int id) {
	int ret;
	ret = vmeRead32(&(c1725p[id]->data[0]));
	printf("%x \n", ret);
	return OK;

}

int v1725MeasureChannelPedestal(int id, unsigned int chan, v1725Ped *ped) {
	int status, i, j, reg_bit, n_words, n_samples, slot;
	int var_t;
	int iters;
	double adc_val;

	int adc_val1, adc_val2;
	v1725Ped p;

	UINT32 trigger_mask, channel_mask;
	UINT32 old_trigger_mask, old_channel_mask;
	UINT32 old_acquisition_mask, old_buffer;
	UINT32 buffp[20000];
	UINT32 *buff_point;

	if (chan > 16) {
		logMsg("v1725MeasureChannelPedestal: ERROR : Channel (%d) out of range (0-15) \n", chan, 0, 0, 0, 0, 0);
		return (ERROR);
	}
	slot = v1725GetSlot(id);
	printf("v1725MeasureChannelPedestal for slot %i (id:%i) ch %i \n", slot, id, chan);

	LOCK_1725;
	old_trigger_mask = vmeRead32(&(c1725p[id]->trigger_source_enable_mask));
	old_channel_mask = vmeRead32(&(c1725p[id]->channel_enable_mask));
	old_acquisition_mask = vmeRead32(&(c1725p[id]->acquisition_control));
	old_buffer = vmeRead32(&(c1725p[id]->buffer_organization));
	trigger_mask = 1;
	trigger_mask = (trigger_mask << 31); /*enable sw_trigger and disable any external*/
	UNLOCK_1725;

	/*v1725SetTriggerEnableMask(int id, int sw_trig, int ext_trig,lvds trig, int coinc_level, int coinc_window, int chan_mask)*/

	v1725Clear(id); /*Clear memories but not reset!*/
	v1725SetBuffer(id, 0x0A); /*1024 buffers 640-10 samples max size*/
	v1725SetTriggerEnableMask(id, 1, 0, 0, 0, 0, 0); /*enable SW-trg only*/
	v1725SetAcquisition(id, 0, 0, 0, 0, 0, 1, 0); /*last two: armed, sw-controlled*/
	v1725SetChannelEnableMask(id, 0x1 << chan);

	CHECKID(id);

	p.avg = 0.0;
	p.rms = 0.0;
	p.min = 16383.0;
	p.max = 0.0;

	n_samples = 0;

	iters = 10;

	//v1725Status(0);

	for (i = 0; i < iters; i++) {
		v1725GenerateSoftwareTrigger(id);

		buffp[0] = 0;
		buff_point = buffp;

		var_t = 0;

		while (((*buffp) >> 28) != 0xA) { /*Seems that, reading one by one, the REAL first word could be later*/
			var_t++;
			if (var_t > 40)
				break;
			LOCK_1725;
			(*buff_point) = vmeRead32(&(c1725p[id]->data[0]));
			UNLOCK_1725;
		}
		if (var_t > 40) {
			printf("initial loop failed. Next iteration! \n");
			continue;
		}
		n_words = (*buff_point) & 0xFFFFFFF; /*first word,first 28 bits is total number of words in the data */

		n_words = n_words - 1; /*I already read the first one.*/
		buff_point++;

		LOCK_1725;
		for (j = 0; j < n_words; j++) {
			(*buff_point) = vmeRead32(&(c1725p[id]->data[0]));
			buff_point++;
		}
		UNLOCK_1725;

		buff_point = buffp;
		buff_point += 4; /* this is the first word containing data */
		n_words = n_words - 3; /*This is the number of words containing data */
		if (n_words > 2000)
			break;
		for (j = 0; j < n_words; j++) {
			adc_val1 = ((*buff_point) & 0x3FFF);
			adc_val2 = (((*buff_point) >> 16) & 0x3FFF);
			n_samples += 2;

			p.avg += adc_val1;
			p.rms += adc_val1 * adc_val1;

			p.avg += adc_val2;
			p.rms += adc_val2 * adc_val2;

			if (adc_val1 < p.min)
				p.min = adc_val1;
			if (adc_val1 > p.max)
				p.max = adc_val;

			if (adc_val2 < p.min)
				p.min = adc_val2;
			if (adc_val2 > p.max)
				p.max = adc_val2;

			buff_point++;
		}
	}
	p.avg = p.avg / n_samples;
	p.rms = sqrt(p.rms / n_samples - p.avg * p.avg);

	printf("v1725MeasureChannelPedestal: ID %d,slot %d, chan %d => avg %6.3f, rms %6.3f, min %.0f, max %.0f\n", id, slot, chan, p.avg,
			p.rms, p.min, p.max);

	if (ped)
		*ped = p;

	v1725SetChannelPedestal(id, chan, (int) p.avg); /*upload this info to the board too*/

	LOCK_1725;
	vmeWrite32(&(c1725p[id]->buffer_organization), old_buffer);
	vmeWrite32(&(c1725p[id]->trigger_source_enable_mask), old_trigger_mask);
	vmeWrite32(&(c1725p[id]->channel_enable_mask), old_channel_mask);
	vmeWrite32(&(c1725p[id]->acquisition_control), old_acquisition_mask);
	UNLOCK_1725;

	return (OK);
}

/*Andrea Celentano */
/*This function is used to auto-set the DC offset of the board
 for a negative unipolar signal and for those channels already been enabled
 supposing there's no zero suppression*/
/*Unfortunately, the DAC used to vary the DC offset is not linear at all, so i have to use a recursive solution, paying attention to the fact that, since the FADC can't see signals greater than 0, i can't find a value for which the mean is 0, since the positive part of the signal is not measured */
/*But, if the noise amplitude is v, i can find a DC offset for which the baseline is at value v*/

/*Remember that:
 - 1 FADC count is: 2V/(2^12-1)=0.488mV
 - 1 DAC count for offset is: 2V/(2^16-1)=0.0305mV (16 bit)
 How it works:

 1) The DC offset is set to a "big" starting value(AUTO_DC_OFFSET_START)

 2) An event is read from each channel, sending a soft trigger
 3) The mean value is calculated for each channel: it will be less than 0V, as expected
 4) The DC offset is decreased by DC_OFFSET_VAR_BIG, and point 2 is repeated until the new mean is lower than the desidered value (v).
 5) To better fix the mean, point 2 is repeated again, increasing the offset by DC_OFFSET_VAR_SMALL, until the new mean is greater than the desidered value

 We have a 2-step recursive program.
 */

int v1725AutoSetDCOffset(int id, float BASELINE_GOAL) { /*BASELINE_GOAL is >0 in mV. Should be greater than the noise level, otherwise the mean is not fine.*/
	int ii, jj;
	int var_t;
	UINT32 trigger_mask, channel_mask;
	UINT32 old_trigger_mask, old_channel_mask;
	UINT32 buffp[20000];
	UINT32 *buff_point;

	int n_words;
	int mean_sum;
	float mean;
	int DC_OFFSET;
	int flag; /*flag=1: first step, going down. flag=0: second step, going up.*/

	int ret;
	ret = 0;

	DC_OFFSET = V1725_AUTO_DC_OFFSET_START;

	LOCK_1725;
	old_trigger_mask = vmeRead32(&(c1725p[id]->trigger_source_enable_mask));
	old_channel_mask = vmeRead32(&(c1725p[id]->channel_enable_mask));
	trigger_mask = 1;
	trigger_mask = (trigger_mask << 31); /*enable sw_trigger and disable any external: in any case, we need it!!*/
	UNLOCK_1725;

	/*v1725SetTriggerEnableMask(int id, int sw_trig, int ext_trig, int lvds,int coinc_level, int coinc_window, int chan_mask)*/
	v1725SetTriggerEnableMask(id, 1, 0, 0, 0, 0, 0); /*enable SW-trg only*/
	v1725SetAcquisition(id, 0, 0, 0, 0, 0, 1, 0); /*last two: armed, sw-trig*/

	printf("Old trigger_mask: %x, trigger_mask %x \n", old_trigger_mask, trigger_mask);

	for (ii = 0; ii < V1725_MAX_CHANNELS; ii++) {
		DC_OFFSET = V1725_AUTO_DC_OFFSET_START;
		if (((old_channel_mask >> ii) & 0x1) == 1) {

			printf("\n \n \n CONFIGURING ch %i \n \n", ii);
			ret = 0;
			flag = 1;
			mean = -2000;
			channel_mask = (1 << ii);

			LOCK_1725;
			vmeWrite32(&(c1725p[id]->channel_enable_mask), channel_mask);
			UNLOCK_1725;
			v1725SetChannelDACOffset(id, ii, DC_OFFSET);
			while ((flag) || ((-mean) < BASELINE_GOAL)) {

				/*	v1725SetAcquisition(int id,int lvds_runin,int lvds_busy,int gpio_busy,int memory_full, int trig_count, int acquisition_armed, int start_stop)*/

				v1725GenerateSoftwareTrigger(id);

				/*To do it it the easy way (and port it to VXWORKS in a few seconds), i read word by word */

				buffp[0] = 0;
				buff_point = buffp;

				var_t = 0;

				while (((*buffp) >> 28) != 0xA) { /*Seems that, reading one by one, the REAL first word could be later*/
					var_t++;
					if (var_t > 40)
						break;
					LOCK_1725;
					(*buff_point) = vmeRead32(&(c1725p[id]->data[0]));
					UNLOCK_1725;
				}
				n_words = (*buff_point) & 0xFFFFFFF; /*first word,first 28 bits is total number of words in the data */
				/*printf("Number of words is %i. %x \n",n_words,(*buff_point));*/
				n_words = n_words - 1; /*I already read the first one.*/
				buff_point++;

				LOCK_1725;
				for (jj = 0; jj < n_words; jj++) {

					(*buff_point) = vmeRead32(&(c1725p[id]->data[0]));
					/*printf("%x \n",*buff_point);*/
					buff_point++;
				}
				UNLOCK_1725;

				buff_point = buffp;
				buff_point += 4; /* this is the first word containing data */
				n_words = n_words - 3; /*This is the number of words containing data */
				mean_sum = 0;
				if (n_words > 2000)
					break;
				for (jj = 0; jj < n_words; jj++) {
					mean_sum = mean_sum + ((*buff_point) & 0x3FFF) + (((*buff_point) >> 16) & 0x3FFF);
					buff_point++;
				}
				mean = (double) mean_sum / (n_words * 2);
				mean = -2000 + mean / 16383 * 2000;

				if (flag == 1) {
					if ((-mean) < BASELINE_GOAL) {
						flag = 0;
					} else {
						printf("%f %f ", -mean, BASELINE_GOAL);
						DC_OFFSET = DC_OFFSET - V1725_DC_OFFSET_VAR_BIG;
						v1725SetChannelDACOffset(id, ii, DC_OFFSET);
					}
				} else if (flag == 0) {
					DC_OFFSET = DC_OFFSET + V1725_DC_OFFSET_VAR_SMALL;
					v1725SetChannelDACOffset(id, ii, DC_OFFSET);
				}
				printf("Mean for ch %i is %f mV.Going to set DC offset to %i \n", ii, mean, DC_OFFSET);
				ret++;
			}
			taskDelay(10); /*wait few ticks, 1 tick = 16.6 ms*/
		}
	}
	LOCK_1725;
	vmeWrite32(&(c1725p[id]->trigger_source_enable_mask), old_trigger_mask);
	vmeWrite32(&(c1725p[id]->channel_enable_mask), old_channel_mask);

	UNLOCK_1725;

	return OK;
}

int v1725peds(char *filename, int newfile) {

	FILE *fd;
	v1725Ped ped;
	int ifa, slot, ch;
	char opt[10];

	if (newfile == 1)
		strcpy(opt, "w");
	else
		strcpy(opt, "a");

	if (strlen(filename) != 0) /* filename specified */
	{

		printf("v1725peds: opening %s for writing\n", filename);
		fflush(stdout);

		if ((fd = fopen(filename, opt)) == NULL) {
			printf("\n v1725peds: Can't open config file >%s<\n", filename);
			fflush(stdout);
			return (-1);
		}
	} else {

		printf("v1725peds: opening DEFAULT: %s for writing\n", filename);
		fflush(stdout);
		if ((fd = fopen(filename, "w")) == NULL) {
			printf("\n v1725peds: Can't open config file >%s<\n", filename);
			return (-1);
		}
	}

	if (Nc1725 == 0) {
		printf("No FADCS are initialized ... \n");
		return (-1);
	}

	for (ifa = 0; ifa < Nc1725; ifa++) {
		slot = v1725GetSlot(ifa);
		for (ch = 0; ch < V1725_MAX_CHANNELS; ch++) {
			usleep(100000); //sleep .1 s
			if (v1725MeasureChannelPedestal(ifa, ch, &ped) != OK) { /*This also triggers the pedestal writing to the DUMMY32 register*/
				printf(" Unabled to measure pedestal on slot %d, ch %d...\n", slot, ch);
				fclose(fd);
				return -1;
			}
			if (fd)
				fprintf(fd, "%2d %2d %7.3f %7.3f %2d\n", slot, ch, ped.avg, ped.rms, 0);
		}
	}

	if (fd)
		fclose(fd);

	return (0);
}

/*sergey: my mid-  and top-level functions*/

int v1725ReadBoard(int iadc, UINT32 *tdata) {
	UINT32 *output = tdata;
	int i, ndata;

	/* get event length in words */
	ndata = v1725GetNextEventSize(iadc);

	logMsg("v1725ReadBoard: ndata=%d\n", ndata, 2, 3, 4, 5, 6);

	for (i = 0; i < ndata; i++) {
		*output++ = vmeRead32(&(c1725p[iadc]->data[0]));
		if (i < 6 || i > (ndata - 7))
			logMsg("v1725ReadBoard:  [%d] 0x%08x \n", i, *(output - 1), 3, 4, 5, 6);
	}

	return (ndata);
}

int v1725ReadBoardDmaStart(int ib, UINT32 *tdata) {
	volatile UINT32 *vmeAdr;
	int mdata, res;
	int i, nbytes;

	if (berr_fifo == 0x01) /*Berr=0 Fifo=1*/
	{
		/* FIFO: get event length in words */
		ndata_save[ib] = v1725GetNextEventSize(ib);
		/*
		 logMsg("v1725ReadBoardDmaStart: INFO: event fifo reports %d words\n",
		 ndata_save[ib],0,0,0,0,0);
		 */

	} else /*Berr*/
	{
		ndata_save[ib] = V1725_MAX_WORDS_PER_BOARD;
		mdata = 0;
		/*
		 logMsg("v1725ReadBoardDmaStart: INFO: trying to DMA %d words\n",
		 ndata_save[ib],0,0,0,0,0);
		 */
	}

	/*usrVmeDmaReset();*/

	if (berr_fifo == 0x01) /*FIF0*/
	{
		if (sngl_blt_mblt >= 0x04) /* 128 bit alignment *//*2eVME or 2eSST*/
		{
			extra_save[ib] = (4 - (ndata_save[ib] % 4));
			if (extra_save[ib] == 4)
				extra_save[ib] = 0;
		} else /* 64 bit alignment */
		{
			/*
			 extra_save[ib] = ndata_save[ib]%2;
			 */
			if ((ndata_save[ib] % 2) != 0)
				extra_save[ib] = 1;
			else
				extra_save[ib] = 0;
		}
		/*
		 logMsg("v1725ReadBoardDmaStart: ndata_save=%d extra_save=%d\n",
		 ndata_save[ib],extra_save[ib],3,4,5,6);
		 */

		nbytes = ((ndata_save[ib] + extra_save[ib]) << 2); /*multiply by 4, since V1725 has 32bit=4bite data words*/
	} else {
		nbytes = (ndata_save[ib] << 2); /*multiply by 4*/
	}

	/*Andrea*/
	/*ASSUME THAT v1725SetBLTRange(ii,1) WAS CALLED!!!*/
	vmeAdr = c1725vme[ib];
	/*
	 logMsg("vmeAdr=0x%08x\n",vmeAdr,2,3,4,5,6);
	 */
	res = usrVme2MemDmaStart((UINT32 *) vmeAdr, (UINT32 *) tdata, nbytes);

	if (res < 0) {
		logMsg("v1725ReadEventDmaStart: ERROR: usrVme2MemDmaStart returned %d\n", res, 0, 0, 0, 0, 0);
	}
	/*else
	 {
	 logMsg("v1725ReadBoardDmaStart: usrVme2MemDmaStart returned %d\n",
	 res,0,0,0,0,0);
	 }
	 */

	return (res);
}

int v1725ReadBoardDmaDone(int ib) {
	int mdata, res;
	int i;

	if (berr_fifo == 0x01) {
		/* check if transfer is completed; returns zero or ERROR  */
		if ((res = usrVme2MemDmaDone()) < 0) {
			logMsg("v1725ReadBoardDmaDone: ERROR: usrVme2MemDmaDone returned = %d\n", res, 2, 3, 4, 5, 6);
			return (0);
		} else {
			mdata = (ndata_save[ib] + extra_save[ib]) - (res >> 2);
			/*
			 logMsg("v1725ReadBoardDmaDone: res=%d ndata_save=%d extra_save=%d -> mdata=%d\n",
			 res,ndata_save[ib],extra_save[ib],mdata,5,6);
			 */

			if ((res > 4) || ((mdata % 2) != 0)) {
				logMsg("v1725ReadBoardDmaDone: WRONG: res=%d (ndata_save=%d, extra_save=%d => was %d), mdata=%d\n", res, ndata_save[ib],
						extra_save[ib], ((ndata_save[ib] + extra_save[ib]) << 2), mdata, 6);
			}

			/*
			 logMsg("v1725ReadBoardDmaDone: INFO: usrVme2MemDmaDone returned = %d (%d)\n",
			 res,mdata,3,4,5,6);
			 */
		}
	} else {
		/* check if transfer is completed; returns zero or ERROR  */
		if ((res = usrVme2MemDmaDone()) < 0) {
			logMsg("v1725ReadBoardDmaDone: ERROR: usrVme2MemDmaDone returned = %d\n", res, 2, 3, 4, 5, 6);
			return (0);
		} else if (res == 0) {
			logMsg("v1725ReadBoardDmaDone: ERROR: usrVme2MemDmaDone returned = %d\n", res, 2, 3, 4, 5, 6);
			logMsg("v1725ReadBoardDmaDone: ERROR: we are expecting VME BUS error !\n", res, 2, 3, 4, 5, 6);
			return (0);
		} else {
			mdata = (res >> 2); //divide by 4 - this is number of 32-bit words
			//if (mdata > 999)
			//logMsg("v1725ReadBoardDmaDone: INFO: usrVme2MemDmaDone returned = %d (%d - %d)\n", res, mdata, ndata_save[ib], 4, 5, 6);
		}
	}

	return (mdata);
}

/* generic readout for v1725 ADC boards */
/* time profiling data for 2 boards 550 bytes event size */
/* Mode=0 programmed transfer
 * Mode=1 DMA
 */
int v1725ReadStart(INT32 *adcbuf, INT32 *rlenbuf, int mode) {
	int ii, jj, nev, iadcbuf, itmp1, itmp2;
	int nn[21];
	unsigned short tdata;

	int nBytesBLT = 4096; //number of bytes can be transferred when doing non-extended BLT
	int nWords;
	/* whole routine: 42 usec */

	/* part1: 6 usec */

	if (Nc1725 == 0) {
		logMsg("v1725ReadStart: ERROR: Nc1725=%d\n", Nc1725, 2, 3, 4, 5, 6);
	}
	if ((mode != 0) && (mode != 1) && (mode != 2)) {
		logMsg("v1725ReadStart: ERROR mode %i\n", mode, 2, 3, 4, 5, 6);
	}

	for (jj = 0; jj < Nc1725; jj++) {
		/* check if board is full */
		/*Check for bus error - disable for mode==0*/
		if ((mode == 0) && (v1725GetBusError(jj) != 0)) {
			logMsg("v1725ReadStart: disable bus error for board - mode is %i\n", jj, mode, 3, 4, 5, 6);
			v1725SetBusError(jj, 0);
		}

		if (v1725StatusFull(jj)) {
			logMsg("ERROR: [%2d] board is full - clear (Nc1725=%d)\n", jj, Nc1725, 3, 4, 5, 6);
			/* clear board, otherwise cannot use it any more ! */
			v1725Clear(jj);
		}

		/* check the number of events */
		nn[jj] = nev = v1725Dready(jj);

		if (nev == 0) {
			logMsg("v1725ReadStart: [%2d] not ready !\n", jj, 2, 3, 4, 5, 6);
		} else if (nev > 1) {
			logMsg("v1725ReadStart: there are %i (>1) events ready \n", nev, 2, 3, 4, 5, 6);
		}

		if (nev == 0){
			rlenbuf[jj]=0;
			return 0;
		}
		/* Trigger Supervisor has 6 event buffer, but we can get 7
		 if 'parallel' readout is in use */
		if (nev > 7) {
			logMsg("v1725readStart: ERROR: [%2d] nev=%d\n", jj, nev, 3, 4, 5, 6);
		}
	}

	/* part2: 36 usec */

	/* readout */
	iadcbuf = 0;
	for (jj = 0; jj < Nc1725; jj++) {

		/* 18usec x 2boards = 36Usec */
		if (mode == 0) {
			rlenbuf[jj] = v1725ReadBoard(jj, &adcbuf[iadcbuf]);
			if (rlenbuf[jj] <= 0) {
				logMsg("[%2d] ERROR: v1725ReadEvent mode0 returns %d\n", jj, rlenbuf[jj], 3, 4, 5, 6);
			} else {
				iadcbuf += rlenbuf[jj];
			}
		} else if (mode == 1) {
			itmp1 = 0;
			itmp2 = 0;
			nWords=v1725GetNextEventSize(jj);
			while (itmp1 < nWords) {
				v1725ReadBoardDmaStart(jj, &adcbuf[iadcbuf]);
				itmp2 = v1725ReadBoardDmaDone(jj); //number of 32-bit words read*/
				if (itmp2 == 0) { //still others to read ???
					itmp1 += nBytesBLT;
				} else {
					itmp1 += itmp2;
				}
			}
			rlenbuf[jj] = itmp1;
			if (rlenbuf[jj] <= 0) {
				logMsg("[%2d] ERROR: v1725ReadEvent mode1 returns %d\n", jj, rlenbuf[jj], 3, 4, 5, 6);
			} else {
				iadcbuf += rlenbuf[jj];
			}
		} else if (mode == 2) {
			v1725SetBLTRange(jj, 1);
			v1725ReadBoardDmaStart(jj, &adcbuf[iadcbuf]);
			rlenbuf[jj] = v1725ReadBoardDmaDone(jj);
			if (rlenbuf[jj] <= 0) {
				logMsg("[%2d] ERROR: v1725ReadEvent mode2 returns %d\n", jj, rlenbuf[jj], 3, 4, 5, 6);
			} else {
				iadcbuf += rlenbuf[jj];
			}
			v1725SetBLTRange(jj, 0);
		}
	}

	return (OK);
}

/* use DMA list */
int v1725ReadListStart(INT32 *adcbuf, INT32 *rlenbuf) {
	int ii, jj, nev;
	int iadcbuf;
	static int *destination[V1725_MAX_MODULES];

	/*
	 TIMER_VAR;
	 */
	if (Nc1725 < 2) {
		logMsg("v1725ReadListStart: ERROR: Nc1725=%d\n", Nc1725, 2, 3, 4, 5, 6);
	}

	for (jj = 0; jj < Nc1725; jj++) {
		/* check if board is full */
		if (v1725StatusFull(jj)) {
			logMsg("ERROR: [%2d] board is full - clear (Nc1725=%d)\n", jj, Nc1725, 3, 4, 5, 6);
			/* clear board, otherwise cannot use it any more ! */
			v1725Clear(jj);
		}

		/* check the number of events */
		nev = v1725Dready(jj);
		if (nev == 0) {
			logMsg("v1725ReadListStart: [%2d] not ready !\n", jj, 2, 3, 4, 5, 6);
		}

		/* Trigger Supervisor has 6 event buffer, but we can get 7
		 if 'parallel' readout is in use */
		if (nev > 7) {
			logMsg("v1725ReadListStart: ERROR: [%2d] nev=%d\n", jj, nev, 3, 4, 5, 6);
		}
	}

	if (sngl_blt_mblt == 0x01) {
		logMsg("ERROR: DMA method must be used: sngl_blt_mblt=%d\n", sngl_blt_mblt, 2, 3, 4, 5, 6);
		return (ERROR);
	}

	/* readout settings */
	if (berr_fifo == 0x01) /* use FIFO reaout */
	{
		iadcbuf = 0;
		for (jj = 0; jj < Nc1725; jj++) {
			/* get event length in words */
			ndata_save[jj] = v1725GetNextEventSize(jj);

			if (sngl_blt_mblt >= 0x04) /* 128 bit alignment */
			{
				extra_save[jj] = (4 - (ndata_save[jj] % 4));
				if (extra_save[jj] == 4)
					extra_save[jj] = 0;
			} else /* 64 bit alignment */
			{
				if ((ndata_save[jj] % 2) != 0)
					extra_save[jj] = 1;
				else
					extra_save[jj] = 0;
			}

			size_save[jj] = (ndata_save[jj] + extra_save[jj]) << 2;
			rlenbuf[jj] = ndata_save[jj] + extra_save[jj];

			destination[jj] = &adcbuf[iadcbuf];

			iadcbuf += rlenbuf[jj];

			/*
			 logMsg("[%d] ask=%d (%d bytes), got=%d (0x%08x to 0x%08x)\n",
			 jj,ndata_save[jj]+extra_save[jj],size_save[jj],rlenbuf[jj],(unsigned int)c1725p[jj],destination[jj]);
			 */

		}
	} else /* use BERR readout */
	{
		iadcbuf = 0;
		for (jj = 0; jj < Nc1725; jj++) {
			ndata_save[jj] = V1725_MAX_WORDS_PER_BOARD;
			extra_save[jj] = 0;

			size_save[jj] = (ndata_save[jj]) << 2;
			rlenbuf[jj] = ndata_save[jj];

			destination[jj] = &adcbuf[iadcbuf];

			iadcbuf += rlenbuf[jj];
		}
	}

	/*
	 {
	 TIMER_NORMALIZE;
	 TIMER_START;
	 }
	 */

	/* set DMA list */
	{
		unsigned int adcadr[21];
		for (ii = 0; ii < Nc1725; ii++)
			adcadr[ii] = (unsigned int) c1725p[ii];
		usrVme2MemDmaListSet(adcadr, destination, size_save, Nc1725);
	}

	/* start DMA list */
	usrVmeDmaListStart();

	/*
	 {
	 TIMER_STOP(100000,-1);
	 }
	 */
	return (OK);
}

int v1725ReadDone() {
	int res;

	res = usrVme2MemDmaDone();

	/* for BERR need something like following:
	 if(berr_fifo == 0x0)
	 {
	 mdata = ndata_save[ib] - (res>>2);
	 }
	 */

	return (res/*OK*/);
}

STATUS v1725Status(int slot) {

	int ii, id;
	volatile struct v1725_ROM_struct *rp;
	unsigned int vers, bid, brev;
	unsigned int chcfg, acqstat, acqctrl;
	unsigned int bufLenght, postTrig;
	unsigned int chEnableMask, chEnableUnaryMask;
	unsigned int count;
	unsigned int fpga_version, fpgaCH0_version, fpgaCH15_version;

	unsigned int trgSourceMask, fpanelTrgOutMask, trgSourceIntUnaryMask;
	unsigned int fPanelCtrl, fPanelNewFeatures;
	unsigned short adcChanDisabled;

	unsigned int vmeCtrl, vmeStatus, vmeGEO, vmeAddr;

	unsigned int tet[16], tet_logic[16], gain[16], dac_offset[16], ped[16];

	id = v1725GetID(slot);
	CHECKID(id);

	rp = (struct v1725_ROM_struct *) ((UINT32) c1725p[ii] + V1725_ROM_OFFSET);

	LOCK_1725;

	vers = (vmeRead32(&(rp->vers)));
	bid = ((vmeRead32(&(rp->boardID1)) & (0xff)) << 8) + (vmeRead32(&(rp->boardID0)) & (0xff));
	brev = (vmeRead32(&(rp->revis0)));
	fpga_version = (vmeRead32(&(c1725p[id]->roc_fpga_firmware_rev))) & 0xFFFF;
	fpgaCH0_version = (vmeRead32(&(c1725p[id]->chan[0].amc_fpga_firmware_rev))) & 0xFFFF;
	fpgaCH15_version = (vmeRead32(&(c1725p[id]->chan[15].amc_fpga_firmware_rev))) & 0xFFFF;

	chcfg = vmeRead32(&(c1725p[id]->chan_config));
	acqstat = vmeRead32(&(c1725p[id]->acquisition_status));
	acqctrl = vmeRead32(&(c1725p[id]->acquisition_control));
	chEnableMask = vmeRead32(&(c1725p[id]->channel_enable_mask));
	chEnableMask = chEnableMask & 0xFFFF; //bits 0-15;

	bufLenght = vmeRead32(&(c1725p[id]->buffer_organization));
	postTrig = vmeRead32(&(c1725p[id]->post_trigger_setting));

	trgSourceMask = vmeRead32(&(c1725p[id]->trigger_source_enable_mask));
	fpanelTrgOutMask = vmeRead32(&(c1725p[id]->front_panel_trigger_out_enable_mask));

	fPanelCtrl = vmeRead32(&(c1725p[id]->front_panel_io_control));
	fPanelNewFeatures = vmeRead32(&(c1725p[id]->front_panel_lvds_io_new_features));

	count = vmeRead32(&(c1725p[id]->event_stored));

	vmeCtrl = vmeRead32(&(c1725p[id]->readout_control));
	vmeStatus = vmeRead32(&(c1725p[id]->readout_status));
	vmeGEO = vmeRead32(&(c1725p[id]->geo_address));
	vmeAddr = vmeRead32(&(c1725p[id]->relocation_address));

	adcChanDisabled = (~chEnableMask) & 0xFFFF;

	for (ii = 0; ii < V1725_MAX_CHANNELS; ii++) {
		gain[ii] = vmeRead32(&(c1725p[id]->chan[ii].gain));
		tet[ii] = vmeRead32(&(c1725p[id]->chan[ii].threshold));
		tet_logic[ii] = vmeRead32(&(c1725p[id]->chan[ii].self_trigger_logic));
		dac_offset[ii] = vmeRead32(&(c1725p[id]->chan[ii].dac_offset));
		ped[ii] = vmeRead32(&(c1725p[id]->chan[ii].dummy32)); /*A.C. read the pedestal from here*/
	}

	UNLOCK_1725;

#ifdef VXWORKS
	printf("\nSTATUS for FADC in slot %d at base address 0x%x - from board: 0x%x\n",
			slot, (UINT32) c1725p[id],vmeAddr);
#else
	printf("\nSTATUS for FADC in slot %d at VME (Local) base address 0x%x (0x%x)- from board: 0x%x  \n", slot,
			(UINT32) c1725p[id] - adcAddrOffsetv1725, (UINT32) c1725p[id], vmeAddr);
#endif
	printf("---------------------------------------------------------------------- \n");

	printf(" Board VERSION = 0x%04x : FPGA MOBO FIRMWARE VERSION = 0x%04x\n", (vers) & 0xffff, fpga_version);
	printf(" FPGA CHANNEL FIRMWARE REVISION = ch0: 0x%04x (ch15: 0x%04x) \n", fpgaCH0_version, fpgaCH15_version);

	printf("\nTrig sources: \n");

	if (trgSourceMask & V1725_TRIGGER_SWT)
		printf("\t Swt trigger enabled \n");
	else
		printf("\t Swt trigger disabled \n");

	if (trgSourceMask & V1725_TRIGGER_LVDS)
		printf("\t LVDS trigger enabled \n");
	else
		printf("\t LVDS trigger disabled \n");

	if (trgSourceMask & V1725_TRIGGER_EXT)
		printf("\t Ext trigger enabled \n");
	else
		printf("\t Ext trigger disabled \n");

	trgSourceIntUnaryMask = 0;

	for (ii = 0; ii < V1725_MAX_CHANNELS; ii++) {

		if ((trgSourceMask & V1725_TRIGGER_INT) & (1 << ii)) {
			trgSourceIntUnaryMask += pow(10., ii);
		}
	}

	printf("\t Int trigger mask: %i (%x) \n", trgSourceIntUnaryMask, trgSourceMask & V1725_TRIGGER_INT);

	printf("\nTrig output to front panel: \n");
	if ((((fpanelTrgOutMask & V1725_TRIGGEROUT_LOGIC) >> 8) & 0x3) == 0x0)
		printf("\t Trig output is OR of all enabled sources \n");
	else if ((((fpanelTrgOutMask & V1725_TRIGGEROUT_LOGIC) >> 8) & 0x3) == 0x1)
		printf("\t Trig output is AND of all enabled sources \n");
	else if ((((fpanelTrgOutMask & V1725_TRIGGEROUT_LOGIC) >> 8) & 0x3) == 0x1) {
		printf("\t Trig output is MAJ of all enabled sources \n");
		printf("\t MAJ level is: %i \n", ((fpanelTrgOutMask & V1725_TRIGGEROUT_MAJ) >> 10) & 0x7);
	} else
		printf("\t Not recognized fpanelTrgOutMask: %x", fpanelTrgOutMask);

	printf("\nFront panel options: (0x%08x) \n", fPanelCtrl);

	if ((fPanelCtrl & V1725_FPANEL_LEMO_TTL) == 0)
		printf("\t Using NIM levels\n");
	else
		printf("\t Using TTL levels\n");

	if ((fPanelCtrl & V1725_FPANEL_TRGOUT_EN) == 0) {
		printf("\t TRG-OUT enabled \n");
	} else {
		printf("\t TRG-OUT disabled - high Z \n");
	}

	if (fPanelCtrl & V1725_FPANEL_NEWFEAT) {
		printf("\t LVDS New features enabled\n");

		if ((((fPanelNewFeatures & V1725_FPANELNEW_CH0) >> 0) & 0xF) == 0x0)
			printf("\t\t NEW-mode ch 0-3 register \n");
		else if ((((fPanelNewFeatures & V1725_FPANELNEW_CH0) >> 0) & 0xF) == 0x1)
			printf("\t\t NEW-mode ch 0-3 trigger \n");
		else if ((((fPanelNewFeatures & V1725_FPANELNEW_CH0) >> 0) & 0xF) == 0x2)
			printf("\t\t NEW-mode ch 0-3 busy-veto \n");
		else if ((((fPanelNewFeatures & V1725_FPANELNEW_CH0) >> 0) & 0xF) == 0x3)
			printf("\t\t NEW-mode ch 0-3 old style \n");
		else
			printf("\t\t Not recognized fPanelNewFeatures %x \n", fPanelNewFeatures);

		if ((((fPanelNewFeatures & V1725_FPANELNEW_CH1) >> 4) & 0xF) == 0x0)
			printf("\t NEW-mode ch 4-7 register \n");
		else if ((((fPanelNewFeatures & V1725_FPANELNEW_CH1) >> 4) & 0xF) == 0x1)
			printf("\t\t NEW-mode ch 4-7 trigger \n");
		else if ((((fPanelNewFeatures & V1725_FPANELNEW_CH1) >> 4) & 0xF) == 0x2)
			printf("\t\t NEW-mode ch 4-7 busy-veto \n");
		else if ((((fPanelNewFeatures & V1725_FPANELNEW_CH1) >> 4) & 0xF) == 0x3)
			printf("\t\t NEW-mode ch 4-7 old style \n");
		else
			printf("\t\t Not recognized fPanelNewFeatures %x \n", fPanelNewFeatures);

		if ((((fPanelNewFeatures & V1725_FPANELNEW_CH2) >> 8) & 0xF) == 0x0)
			printf("\t\t NEW-mode ch 8-11 register \n");
		else if ((((fPanelNewFeatures & V1725_FPANELNEW_CH2) >> 8) & 0xF) == 0x1)
			printf("\t\t NEW-mode ch 8-11 trigger \n");
		else if ((((fPanelNewFeatures & V1725_FPANELNEW_CH2) >> 8) & 0xF) == 0x2)
			printf("\t\t NEW-mode ch 8-11 busy-veto \n");
		else if ((((fPanelNewFeatures & V1725_FPANELNEW_CH2) >> 8) & 0xF) == 0x3)
			printf("\t\t NEW-mode ch 8-11 old style \n");
		else
			printf("\t\t Not recognized fPanelNewFeatures %x \n", fPanelNewFeatures);

		if ((((fPanelNewFeatures & V1725_FPANELNEW_CH3) >> 12) & 0xF) == 0x0)
			printf("\t\t NEW-mode ch 12-15 register \n");
		else if ((((fPanelNewFeatures & V1725_FPANELNEW_CH3) >> 12) & 0xF) == 0x1)
			printf("\t\t NEW-mode ch 12-15 trigger \n");
		else if ((((fPanelNewFeatures & V1725_FPANELNEW_CH3) >> 12) & 0xF) == 0x2)
			printf("\t\t NEW-mode ch 12-15 busy-veto \n");
		else if ((((fPanelNewFeatures & V1725_FPANELNEW_CH3) >> 12) & 0xF) == 0x3)
			printf("\t\t NEW-mode ch 12-15 old style \n");
		else
			printf("\t\t Not recognized fPanelNewFeatures %x \n", fPanelNewFeatures);

	} else {
		printf("\t LVDS New features disabled\n");
		if ((((fPanelCtrl & V1725_FPANEL_SIGCONF) >> 6) & 0x3) == 0x0)
			printf("\t \t OLD-mode GPIO\n");
		else if ((((fPanelCtrl & V1725_FPANEL_SIGCONF) >> 6) & 0x3) == 0x1)
			printf("\t OLD-mode programmed IO\n");
		else if ((((fPanelCtrl & V1725_FPANEL_SIGCONF) >> 6) & 0x3) == 0x2)
			printf("\t OLD-mode pattern \n");
		else if ((((fPanelCtrl & V1725_FPANEL_SIGCONF) >> 6) & 0x3) == 0x3)
			printf("\t OLD-mode reserved \n");
		else
			printf("\t Not recognized fPanelCtrl: %x \n", fPanelCtrl);
	}
	printf("\n\tFront panel I/O: \n", fPanelCtrl);
	if ((fPanelCtrl & V1725_FPANEL_LVDSDIR1) == 0)
		printf("\t\tSig 0 - 3 are inputs\n");
	else
		printf("\t\tSig 0 - 3 are outputs\n");

	if ((fPanelCtrl & V1725_FPANEL_LVDSDIR2) == 0)
		printf("\t\tSig 4 - 7 are inputs\n");
	else
		printf("\t\tSig 4 - 7 are outputs\n");

	if ((fPanelCtrl & V1725_FPANEL_LVDSDIR3) == 0)
		printf("\t\tSig 8 - 11 are inputs\n");
	else
		printf("\t\tSig 8 - 11 are outputs\n");

	if ((fPanelCtrl & V1725_FPANEL_LVDSDIR4) == 0)
		printf("\t\tSig 12 - 15 are inputs\n");
	else
		printf("\t\tSig 12 - 15 are outputs\n");

	printf("\n Configuration: \n");

	if (vmeCtrl & V1725_BERR_ENABLE)
		printf("\t Bus Error ENABLED\n");
	else
		printf("\t Bus Error DISABLED\n");

	if (vmeCtrl & V1725_BLT_RANGE)
		printf("\t VME extended block transfer ENABLED\n");
	else
		printf("\t VME extended block transfer DISABLED\n");

	printf("\n ADC Processing Configuration: \n");
	printf("\t Channel Disable Mask = 0x%04x\n", adcChanDisabled);

	printf("\t Window length  = %d \n", bufLenght);
	printf("\t PostTrigger    = %d \n", postTrig);

	printf("\n");

	printf("  Ch| Threshold     |  GAIN   | PED   | DAC_OFFSET \n");
	printf("  --|---------------|---------|-------|------------\n");
	for (ii = 0; ii < V1725_MAX_CHANNELS; ii++) {
		printf("  %2d|          %4d |    %4d |   %4d|%4d\n", ii, tet[ii], gain[ii], ped[ii], dac_offset[ii]);
	}
	printf("\n");

}

#else /* dummy version*/

void
v1725Lib()
{
	return;
}

#endif
