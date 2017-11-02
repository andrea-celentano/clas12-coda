/****************************************************************************
 *
 *  v1725Config.c  -  configuration library file for v1725 board
 *
 *  SP, 07-Nov-2013
 *  Sergey Boyarinov Nov 2013 - simplify/adjust for Hall B
 *  Andrea Celentano Oct 2017 - ported to v1725
 *
 *  empty lines and line startes with # - will be ignored
 *  config file format:

 CRATE             rocbcal1   <- ROC/crate name, usually IP name
 V1725_ALLSLOTS              <- just keyword - all settings after this line will be implemented
 #                                              for all slots, till V1725_SLOTS will be met
 V1725_SLOTS     3  8  15   <- slot_numbers - in which next settings will be implemented
 #                                              till file ends or next V1725_SLOTS will be met
 V1725_F_REV     0x02c1     <- firmware revision  (0x0 Bits:7-0)
 V1725_B_REV     0x0a03     <- board revision     (0x0 Bits:15-8)
 V1725_ID        0xfadc     <- board type         (0x0 Bits:31-16)
 #
 V1725_W_OFFSET  50  <- number of samples after trigger point, according to:
 number of post_trigger_samples = 4*OFFSET + CONST (const is contant for each v1725)
 #
 V1725_W_WIDTH   9  <- number of samples to include in acquisition window, according to this table:
 (1 sample = 4 ns)

 value n. buffers n. samples/event
 0x00 1 640k - 10
 0x01 2 320k - 10
 0x02 4 160k - 10
 0x03 8 80k - 10
 0x04 16 40k - 10
 0x05 32 20k - 10
 0x06 64 10k - 10
 0x07 128 5k - 10
 0x08 256 2560 - 10
 0x09 512 1280 - 10
 0x0A 1024 640 - 10
 #

 #               0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 - channels ##
 V1725_ADC_MASK  1  0  1  0  1  0  1  0  1  0  1  0  1  0  1  0   <- channel enable mask
 V1725_TRG_MASK  1  1  1  1  1  1  1  1  1  1  1  1  1  1  1  1   <- trigger enable mask

 V1725_TET       110        <- board Trigger Energy Threshold (TET), same for all 16 channels
 V1725_CH_TET    0    110   <- channel# and TET_value for this channel
 V1725_ALLCH_TET 111  222  2  3  4  5  6  7  8  9  10  11  12  13  14  15   <- 16 TETs (0x12C - 0x148)

 V1725_DAC       3300       <- board DAC, one and the same for all 16 channels
 V1725_CH_DAC    0    3300  <- channel# and DAC_value for this channel
 V1725_ALLCH_DAC 3300 3280 3310 3280 3310 3280 3310 3280 3300 3280 3300 3280 3310 3280 3310 3280 <- 16 DACs

 V1725_PED       210        <- board Pedestals, same for all channels
 V1725_CH_PED    0    210   <- channel# and Pedestal_value for this channel
 V1725_ALLCH_PED 210  220  210  215  215  220  220  210  210  215  215  220  220  210  215  220  <- 16 PEDs

 V1725_GAIN       1        <- board Gains, same for all channels
 V1725_CH_GAIN    0   1   <- channel# and Gain_value for this channel
 V1725_ALLCH_GAIN 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 <- 16 GAINs



 cc -rdynamic -shared -o v1725Config.so v1725Config.c -I/home/halld/test_setup/coda/linuxvme/include /home/halld/test_setup/coda/linuxvme/jvme/libjvme.a /home/halld/test_setup/coda/linuxvme/fadcV2/libfadc.a -ldl -lpthread -lrt

 */

#if defined(VXWORKS) || defined(Linux_vme)

#include <stdio.h>
#include <string.h>

#include "v1725Config.h"
#include "v1725.h"
#include "xxxConfig.h"

#undef DEBUG

static int active;

static int nv1725; /* Number of V1725s */
static V1725_CONF v1725[NBOARD + 1];

#define SCAN_MSK \
		args = sscanf (str_tmp, "%*s %d %d %d %d %d %d %d %d   \
				%d %d %d %d %d %d %d %d", \
				&msk[ 0], &msk[ 1], &msk[ 2], &msk[ 3], \
				&msk[ 4], &msk[ 5], &msk[ 6], &msk[ 7], \
				&msk[ 8], &msk[ 9], &msk[10], &msk[11], \
				&msk[12], &msk[13], &msk[14], &msk[15])

#define SCAN_FMSK \
		args = sscanf (str_tmp, "%*s %f %f %f %f %f %f %f %f   \
				%f %f %f %f %f %f %f %f", \
				&fmsk[ 0], &fmsk[ 1], &fmsk[ 2], &fmsk[ 3], \
				&fmsk[ 4], &fmsk[ 5], &fmsk[ 6], &fmsk[ 7], \
				&fmsk[ 8], &fmsk[ 9], &fmsk[10], &fmsk[11], \
				&fmsk[12], &fmsk[13], &fmsk[14], &fmsk[15])

#define GET_READ_MSK \
		SCAN_MSK; \
		ui1 = 0; \
		for(jj=0; jj<NCHAN; jj++) \
		{ \
			if((msk[jj] < 0) || (msk[jj] > 1)) \
			{ \
				printf("\nReadConfigFile: Wrong mask bit value, %d\n\n",msk[jj]); return(-6); \
			} \
			if(strcmp(keyword,"V1725_ADC_MASK") == 0) msk[jj] = (msk[jj])&0x1; \
			ui1 |= (msk[jj]<<jj); \
		}

static char *expid = NULL;

void v1725SetExpid(char *string) {
	expid = strdup(string);
}

int v1725Config(char *fname) {
	int res;
	char string[10]; /*dummy, will not be used*/

	/* faInit() must be called by now; get the number of boards from there */
	nv1725 = v1725GetNfadc();
	printf("v1725Config: nv1725=%d fname: %s\n", nv1725, fname);fflush(stdout);

	if (strlen(fname) > 0) {
		v1725UploadAll(string, 0);
	} else {
		v1725InitGlobals();
	}
	/* reading and parsing config file */
	if ((res = v1725ReadConfigFile(fname)) < 0) {
		printf("ERROR in v1725Config: v1725ReadConfigFile() returns %d\n", res);fflush(stdout);
		return (res);
	}

	/* download to all boards */
	v1725DownloadAll();

	return (0);
}

void v1725InitGlobals() {
	int ii, jj;

	printf("v1725InitGlobals reached\n");fflush(stdout);

	/*nv1725 = 0;*/
	nv1725 = faGetNfadc();
	for (jj = 0; jj < NBOARD; jj++) {
		v1725[jj].winOffset = 0x09; /*9x4=36 samples after trigger */
		v1725[jj].winWidth = 0x09; /*1270 samples, 5080 ns*/
		v1725[jj].chMask = 0xffff;
		v1725[jj].trigMask = 0xffff;


		for (ii = 0; ii < NCHAN; ii++) {
			v1725[jj].thr[ii] = 110;
			v1725[jj].dac[ii] = 3300;
			v1725[jj].gain[ii] = 1;
			v1725[jj].ped[ii] = 0;
		}
	}
}

/* reading and parsing config file */
int v1725ReadConfigFile(char *filename) {
	FILE *fd;
	char fname[FNLEN] = { "" }; /* config file name */
	int ii, jj, ch;
	char str_tmp[STRLEN], str2[STRLEN], keyword[ROCLEN];
	char host[ROCLEN], ROC_name[ROCLEN];
	int args, i1, i2, msk[16];
	int slot, slot1, slot2, chan;
	unsigned int ui1, ui2;
	float f1, fmsk[16];
	char *getenv();
	char *clonparms;
	char *expid;

	clonparms = getenv("CLON_PARMS");
	expid = getenv("EXPID");
	if (strlen(filename) != 0) /* filename specified */
	{
		if (filename[0] == '/' || (filename[0] == '.' && filename[1] == '/')) {
			sprintf(fname, "%s", filename);
		} else {
			sprintf(fname, "%s/v1725/%s", clonparms, filename);
		}

		if ((fd = fopen(fname, "r")) == NULL) {
			printf("\nReadConfigFile: Can't open config file >%s<\n", fname);fflush(stdout);
			return (-1);
		}
	} else /* filename does not specified */
	{
		/* obtain our hostname */
		gethostname(host, ROCLEN);
		sprintf(fname, "%s/v1725/%s.cnf", clonparms, host);
		if ((fd = fopen(fname, "r")) == NULL) {
			sprintf(fname, "%s/v1725/%s.cnf", clonparms, expid);
			if ((fd = fopen(fname, "r")) == NULL) {
				printf("\nReadConfigFile: Can't open config file >%s<\n", fname);fflush(stdout);
				return (-2);
			}
		}

	}

	printf("\nReadConfigFile: Using configuration file >%s<\n", fname);fflush(stdout);

	/* Parsing of config file */
	active = 0;
	while ((ch = getc(fd)) != EOF) {
		if (ch == '#' || ch == ' ' || ch == '\t') {
			while (getc(fd) != '\n') {
			}
		} else if (ch == '\n') {
		} else {
			ungetc(ch, fd);
			fgets(str_tmp, STRLEN, fd);
			sscanf(str_tmp, "%s %s", keyword, ROC_name);
#ifdef DEBUG
			printf("\nfgets returns %s so keyword=%s\n\n",str_tmp,keyword);fflush(stdout);
#endif

			/* Start parsing real config inputs */
			if (strcmp(keyword, "V1725_CRATE") == 0) {
				if (strcmp(ROC_name, host) == 0) {
					printf("\nReadConfigFile: crate = %s  host = %s - activated\n", ROC_name, host);fflush(stdout);
					active = 1;
				} else if (strcmp(ROC_name, "all") == 0) {
					printf("\nReadConfigFile: crate = %s  host = %s - activated\n", ROC_name, host);fflush(stdout);
					active = 1;
				} else {
					printf("\nReadConfigFile: crate = %s  host = %s - disactivated\n", ROC_name, host);fflush(stdout);
					active = 0;
				}
			}

			else if (active && ((strcmp(keyword, "V1725_SLOT") == 0) || (strcmp(keyword, "V1725_SLOTS") == 0))) {
				sscanf(str_tmp, "%*s %s", str2);
				/*printf("str2=%s\n",str2);*/
				if (isdigit(str2[0])) {
					slot1 = atoi(str2);
					slot2 = slot1 + 1;
					if (slot1 < 2 && slot1 > 21) {
						printf("\nReadConfigFile: Wrong slot number %d\n\n", slot1);fflush(stdout);
						return (-4);
					}
				} else if (!strcmp(str2, "all")) {
					slot1 = 0;
					slot2 = NBOARD;
				} else {
					printf("\nReadConfigFile: Wrong slot >%s<, must be 'all' or actual slot number\n\n", str2);fflush(stdout);
					return (-4);
				}
				/*printf("slot1=%d slot2=%d\n",slot1,slot2);*/
			}

			else if (active && (strcmp(keyword, "V1725_W_OFFSET") == 0)) {
				sscanf(str_tmp, "%*s %d", &i1);
				for (slot = slot1; slot < slot2; slot++)
					v1725[slot].winOffset = i1;
			}

			else if (active && (strcmp(keyword, "V1725_W_WIDTH") == 0)) {
				sscanf(str_tmp, "%*s %d", &i1);
				for (slot = slot1; slot < slot2; slot++)
					v1725[slot].winWidth = i1;
			}

			else if (active && (strcmp(keyword, "V1725_ADC_MASK") == 0)) {
				GET_READ_MSK
				;
				for (slot = slot1; slot < slot2; slot++)
					v1725[slot].chMask = ui1;
#ifdef DEBUG
				printf("\nReadConfigFile: %s = 0x%04x \n",keyword,ui1);
#endif
			}

			else if (active && (strcmp(keyword, "V1725_TRG_MASK") == 0)) {
				GET_READ_MSK
				;
				for (slot = slot1; slot < slot2; slot++)
					v1725[slot].trigMask = ui1;
#ifdef DEBUG
				printf("\nReadConfigFile: %s = 0x%04x \n",keyword,ui1);
#endif
			}

			else if (active && (strcmp(keyword, "V1725_TET") == 0)) {
				sscanf(str_tmp, "%*s %d", &ui1);
				for (slot = slot1; slot < slot2; slot++)
					for (ii = 0; ii < NCHAN; ii++)
						v1725[slot].thr[ii] = ui1;
			}

			else if (active && (strcmp(keyword, "V1725_CH_TET") == 0)) {
				sscanf(str_tmp, "%*s %d %d", &chan, &ui1);
				if ((chan < 0) || (chan > NCHAN)) {
					printf("\nReadConfigFile: Wrong channel number %d, %s\n", chan, str_tmp);fflush(stdout);
					return (-7);
				}
				for (slot = slot1; slot < slot2; slot++)
					v1725[slot].thr[chan] = ui1;
			}

			else if (active && (strcmp(keyword, "V1725_ALLCH_TET") == 0)) {
				SCAN_MSK;
				if (args != 16) {
					printf("\nReadConfigFile: Wrong argument's number %d, should be 16\n\n", args);fflush(stdout);
					return (-8);
				}
				for (slot = slot1; slot < slot2; slot++)
					for (ii = 0; ii < NCHAN; ii++)
						v1725[slot].thr[ii] = msk[ii];
			}

			else if (active && (strcmp(keyword, "V1725_DAC") == 0)) {
				sscanf(str_tmp, "%*s %d", &ui1);
				for (slot = slot1; slot < slot2; slot++)
					for (ii = 0; ii < NCHAN; ii++)
						v1725[slot].dac[ii] = ui1;
			}

			else if (active && (strcmp(keyword, "V1725_CH_DAC") == 0)) {
				sscanf(str_tmp, "%*s %d %d", &chan, &ui1);
				if ((chan < 0) || (chan > NCHAN)) {
					printf("\nReadConfigFile: Wrong channel number %d, %s\n", chan, str_tmp);fflush(stdout);
					return (-7);
				}
				for (slot = slot1; slot < slot2; slot++)
					v1725[slot].dac[chan] = ui1;
			}

			else if (active && (strcmp(keyword, "V1725_ALLCH_DAC") == 0)) {
				SCAN_MSK;
				if (args != 16) {
					printf("\nReadConfigFile: Wrong argument's number %d, should be 16\n\n", args);fflush(stdout);
					return (-8);
				}
				for (slot = slot1; slot < slot2; slot++)
					for (ii = 0; ii < NCHAN; ii++)
						v1725[slot].dac[ii] = msk[ii];
			}

			else if (active && (strcmp(keyword, "V1725_PED") == 0)) {
				sscanf(str_tmp, "%*s %f", &f1);
				for (slot = slot1; slot < slot2; slot++)
					for (ii = 0; ii < NCHAN; ii++)
						v1725[slot].ped[ii] = f1;
			}

			else if (active && (strcmp(keyword, "V1725_CH_PED") == 0)) {
				sscanf(str_tmp, "%*s %d %f", &chan, &f1);
				if ((chan < 0) || (chan > NCHAN)) {
					printf("\nReadConfigFile: Wrong channel number %d, %s\n", chan, str_tmp);fflush(stdout);
					return (-7);
				}
				for (slot = slot1; slot < slot2; slot++)
					v1725[slot].ped[chan] = f1;
			}

			else if (active && (strcmp(keyword, "V1725_ALLCH_PED") == 0)) {
				SCAN_FMSK;
				if (args != 16) {
					printf("\nReadConfigFile: Wrong argument's number %d, should be 16\n\n", args);fflush(stdout);
					return (-8);
				}
				for (slot = slot1; slot < slot2; slot++)
					for (ii = 0; ii < NCHAN; ii++)
						v1725[slot].ped[ii] = fmsk[ii];
			}

			else if (active && (strcmp(keyword, "V1725_GAIN") == 0)) {
				sscanf(str_tmp, "%*s %f", &f1);
				for (slot = slot1; slot < slot2; slot++)
					for (ii = 0; ii < NCHAN; ii++)
						v1725[slot].gain[ii] = f1;
			}

			else if (active && (strcmp(keyword, "V1725_CH_GAIN") == 0)) {
				sscanf(str_tmp, "%*s %d %f", &chan, &f1);
				if ((chan < 0) || (chan > NCHAN)) {
					printf("\nReadConfigFile: Wrong channel number %d, %s\n", chan, str_tmp);fflush(stdout);
					return (-7);
				}
				for (slot = slot1; slot < slot2; slot++)
					v1725[slot].gain[chan] = f1;
			}

			else if (active && (strcmp(keyword, "V1725_ALLCH_GAIN") == 0)) {
				SCAN_FMSK;
				if (args != 16) {
					printf("\nReadConfigFile: Wrong argument's number %d, should be 16\n\n", args);fflush(stdout);
					return (-8);
				}
				for (slot = slot1; slot < slot2; slot++)
					for (ii = 0; ii < NCHAN; ii++)
						v1725[slot].gain[ii] = fmsk[ii];
			}

			else {
				; /* unknown key - do nothing */
			}

		}
	}
	fclose(fd);

	return (0);
}

/* download setting into all found FADCs */
int v1725DownloadAll() {
	int slot, ii, jj, nsamples, threshold;
	int ped, data;

	int updateThresholds = 1;

	printf("\n\nv1725DownloadAll reached, nv1725=%d\n", nv1725);fflush(stdout);
	for (jj = 0; jj < nv1725; jj++) {
		slot = v1725GetSlot(jj);

		v1725SetBuffer(jj, v1725[slot].winWidth);
		v1725SetPostTrigger(jj, v1725[slot].winOffset);
		v1725SetChannelEnableMask(jj, v1725[slot].chMask);

		for (ii = 0; ii < NCHAN; ii++) {

			v1725SetChannelDACOffset(jj, ii, v1725[slot].dac[ii]);
			v1725SetChannelGain(jj, ii, v1725[slot].gain[ii]);

			if (ii % 2 == 1) { /*Odd channels*/
				data = (v1725[slot].trigMask) >> (ii - 1) & 0x3;
				v1725SetSelfTriggerLogic(jj, ii, data, 0);
				v1725SetSelfTriggerLogic(jj, ii - 1, data, 0);
			}

			ped = v1725GetChannelPedestal(jj, ii); /*This will simply read the pedestal from DUMMY32. It is 0 if not set*/

			if (ped > 0) {
				if (v1725[slot].thr[ii]>0) v1725SetChannelThreshold(jj, ii, ped + v1725[slot].thr[ii]); //only when there's a thr!
			} else {
				v1725SetChannelThreshold(jj, ii, v1725[slot].thr[ii]);
			}
		}
	}
	return (0);
}

/* upload setting from all found FADCs */
int v1725UploadAll(char *string, int length) {

	int slot, i, ii, jj, len1, len2;
	char *str, sss[1024];
	unsigned int tmp, val[NCHAN], adcChanEnabled;
	unsigned short sval[NCHAN];
	float fval[NCHAN];
	printf("v1725UploadAll reached %s %i \n", string, length);
	fflush(stdout);
	for (jj = 0; jj < nv1725; jj++) {
		slot = v1725GetSlot(jj);

		v1725[slot].winWidth = v1725GetBuffer(jj);
		v1725[slot].winOffset = v1725GetPostTrigger(jj);

		v1725[slot].chMask = ~(v1725GetChannelEnableMask(jj));

		for (i = 0; i < NCHAN; i++) {
			v1725[slot].dac[i] = v1725GetChannelDACOffset(jj, i);

			v1725[slot].ped[i] = v1725GetChannelPedestal(jj, i);

			v1725[slot].thr[i] = v1725GetChannelThreshold(jj, i);

			if (v1725[slot].thr[i] > 0)
				v1725[slot].thr[i] = v1725[slot].thr[i] - v1725[slot].ped[i]; /* MUST SUBTRACT PEDESTAL TO BE CONSISTENT WITH DOWNLOADED THRESHOLD */

			v1725[slot].gain[i] = v1725GetChannelGain(slot, i);
		}
	}

	if (length) {
		str = string;
		str[0] = '\0';

		for (jj = 0; jj < nv1725; jj++) {
			slot = v1725GetSlot(jj);

			sprintf(sss, "V1725_SLOT %d\n", slot);
			ADD_TO_STRING
;
			sprintf(sss, "V1725_W_OFFSET %d\n", v1725[slot].winOffset);
			ADD_TO_STRING
;
			sprintf(sss, "V1725_W_WIDTH  %d\n", v1725[slot].winWidth);
			ADD_TO_STRING
;			adcChanEnabled = v1725[slot].chMask;
			sprintf(sss, "V1725_ADC_MASK");
			ADD_TO_STRING
;
			for (i = 0; i < 16; i++) {
				sprintf(sss, " %d", (adcChanEnabled >> (15 - i)) & 0x1);
				ADD_TO_STRING
;			}
			sprintf(sss, "\n");
			ADD_TO_STRING
;			sprintf(sss, "V1725_TRG_MASK");
			ADD_TO_STRING
;
			for (i = 0; i < 16; i++) {
				sprintf(sss, " %d", (v1725[jj].trigMask >> (15 - i)) & 0x1);
				ADD_TO_STRING
;			}
			sprintf(sss, "\n");
			ADD_TO_STRING
;			sprintf(sss, "V1725_ALLCH_DAC");
			ADD_TO_STRING
;			for (i = 0; i < 16; i++) {
				sprintf(sss, " %d", v1725[slot].dac[i]);
				ADD_TO_STRING
;			}
			sprintf(sss, "\n");
			ADD_TO_STRING
;			sprintf(sss, "V1725_ALLCH_PED");
			ADD_TO_STRING
;
			for (i = 0; i < 16; i++) {
				sprintf(sss, " %7.3f", v1725[slot].ped[i]);
				ADD_TO_STRING
;			}
			sprintf(sss, "\n");
			ADD_TO_STRING
;			sprintf(sss, "V1725_ALLCH_TET");
			ADD_TO_STRING
;
			for (i = 0; i < 16; i++) {
				sprintf(sss, " %d", v1725[slot].thr[i]);
				ADD_TO_STRING
;			}
			sprintf(sss, "\n");
			ADD_TO_STRING
;			sprintf(sss, "V1725_ALLCH_GAIN");
			ADD_TO_STRING
;
			for (i = 0; i < 16; i++) {
				sprintf(sss, " %7.3f", v1725[slot].gain[i]);
				ADD_TO_STRING
;			}
			sprintf(sss, "\n");
			ADD_TO_STRING
;		}

		CLOSE_STRING
;	}

}

/* print board registers; if slot is zero, print all boards */
void v1725Mon(int slot) {
	int id, start, end, kk, jj;

	nv1725 = v1725GetNfadc();
	if (slot == 0) {
		start = 0;
		end = nv1725;
	} else if ((id = faId(slot)) >= 0) {
		start = id;
		end = start + 1;
	} else {
		return;
	}

	printf("nv1725=%d\n", nv1725);
	for (kk = start; kk < end; kk++) {
		v1725Status(kk);
	}

	return;
}

#else /* dummy version*/

void
v1725Config_dummy()
{
	return;
}

#endif
