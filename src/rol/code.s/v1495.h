
/* v1495.h */


/* Define a Structure for access */
/* all addressing is A24/A32, all data D16 except scratch32 */
typedef struct v1495_struct
{

  /*0x0000 - 0x0FFC*/ volatile unsigned int BLTdata[1024]; /* R */
  /*0x1000*/volatile unsigned short a_status_l;
  /*0x1002*/volatile unsigned short a_status_h;
  /*0x1004*/volatile unsigned short b_status_l;
  /*0x1006*/volatile unsigned short b_status_h;
  /*0x1008*/volatile unsigned short c_status_l;
  /*0x100A*/volatile unsigned short c_status_h;
  /*0x100C*/volatile unsigned short a_mask_l;
  /*0x100E*/volatile unsigned short a_mask_h;
  /*0x1010*/volatile unsigned short b_mask_l;
  /*0x1012*/volatile unsigned short b_mask_h;
  /*0x1014*/volatile unsigned short c_mask_l;
  /*0x1016*/volatile unsigned short c_mask_h;
  /*0x1018*/volatile unsigned short gatewidth;
  /*0x101a-0x2000*/ unsigned short blank0[2036];
  /*0x02002*/ volatile unsigned short multiplicity;
  /*0x02004-0x200E*/ unsigned short blank1[6];
  /*0x2010-0x204E*/ volatile unsigned short prescale[32];
  /*0x2050-0x20FE*/ unsigned short blank2[88];
  /*0x2100*/		volatile unsigned short triggerStatus;
  /*0x2102*/ 		volatile unsigned short triggerNWords;
  /*0x2104-0x210A*/ volatile unsigned short triggerWords[4];
  /*0x210C-0x21FE*/ unsigned short blank3[122];
  /*0x2200*/		volatile unsigned short scalerChannelCtrl;
  /*0x2202*/		volatile unsigned short scalerChannelDataL;
  /*0x2204*/		volatile unsigned short scalerChannelDataH;
  /*0x2206-0x220E*/ unsigned short blank4[5];
  /*0x2210*/		volatile unsigned short scalerLogicCtrl;
  /*0x2212*/		volatile unsigned short scalerLogicDataL;
  /*0x2214*/		volatile unsigned short scalerLogicDataH;
  /*0x2216-0x221E*/ volatile short blank5[5];
  /*0x2220*/		volatile unsigned short scalerTrgCtrl;
  /*0x2222*/		volatile unsigned short scalerTrgDataL;
  /*0x2224*/		volatile unsigned short scalerTrgDataH;
  /*0x2226-0x7FFE*/ unsigned short blank6[12013];
  /*0x8000*/ volatile unsigned short control;      		    /* R/W */
  /*0x8002*/ volatile unsigned short status;        	   /* R */
  /*0x8004*/ volatile unsigned short intLevel;    		   /* R/W */
  /*0x8006*/ volatile unsigned short intVector;    		  /* R/W */
  /*0x8008*/ volatile unsigned short geoAddr;       	 /* R */
  /*0x800A*/ volatile unsigned short moduleReset;   	 /* W */
  /*0x800C*/ volatile unsigned short firmwareRev;   	 /* R */
  /*0x800E*/ volatile unsigned short selflashVME;   	 /* R/W */
  /*0x8010*/ volatile unsigned short flashVME;      	 /* R/W */
  /*0x8012*/ volatile unsigned short selflashUSER;  	 /* R/W */
  /*0x8014*/ volatile unsigned short flashUSER;     	 /* R/W */
  /*0x8016*/ volatile unsigned short configUSER;    	 /* R/W */
  /*0x8018*/ volatile unsigned short scratch16;     	 /* R/W */
  /*0x801A*/ volatile unsigned short res1[3];
  /*0x8020*/ volatile unsigned int   scratch32;       	 /* R/W */
  /*0x8024*/ volatile unsigned short res2[110];
  /*0x8100*/ volatile unsigned short configROM[127]; /* R */

} V1495;

#define V1495_BOARD_ID   0x00E64000 /*  */
#define V1495_NSCALERS_CHANNEL 	 32
#define V1495_NSCALERS_LOGIC	 32
#define V1495_SLOT_NONGEO 0x1F /*content of GEO addr for non-vme64x boards*/
#define V1495_SLOT_VME64  0x14 /*Slot 20, before last (21) where TI should sit*/

#define V1495_HEADER_PATTERN 0xa

/*INIT*/
#define V1495_INIT_SKIP                (1<<16)

/*SCALERS*/
#define V1495_SCALER_CTRL_MASK 0x3 /*2 LSBs*/
//prototypes

int v1495Init(unsigned int addr,int iflag);
unsigned int v1495Slot();

int v1495GetTriggerStatus();
int v1495GetTriggerEmptyFifo();
int v1495GetTriggerFullFifo();
void v1495PrintTriggerStatus();

int v1495GetNTriggerWords();
void v1495PrintNTriggerWords();

void v1495ReadSingleTrgWord(int *buf);
void v1495PrintSingleTrgWord();

void v1495ReadMultiTrgWord(int N,int *buf);
void v1495PrintMultiTrgWord(int N);


int v1495ReadStart(int *adcbuf);

void v1495SetPrescale(int logicCh, int factor);
int v1495GetPrescale(int logicCh);
int v1495PrintPrescale(int logicCh);

/*Scalers routine - channels*/
unsigned int v1495ReadChannelScaler(int scaler,int clear,int latch);
int v1495ReadAllChannelScalers(volatile unsigned int *data, unsigned int chmask, int rflag);
void v1495PrintChannelScaler(int scaler,int clear,int latch);
void v1495PrintAllChannelScaler(int clear,int latch);

/*Scalers routine - logic */
unsigned int v1495ReadLogicScaler(int scaler,int clear,int latch);
int v1495ReadAllLogicScaler(volatile unsigned int *data, unsigned int chmask, int rflag);
void v1495PrintLogicScaler(int scaler,int clear,int latch);
void v1495PrintAllLogicScaler(int clear,int latch);


/*Scalers routine -trg*/
unsigned int v1495ReadLogicScaler(int scaler,int clear,int latch);
int v1495ReadAllTrgScaler(volatile unsigned int *data, unsigned int chmask, int rflag);
void v1495PrintTrgScaler(int scaler,int clear,int latch);
void v1495PrintAllTrgScaler(int clear,int latch);


void v1495Reset();
