/****************************************************************************
 *
 *  v895.h  -  Driver library header file for readout of a C.A.E.N. Mod. V895
 *             VME 16 Channel Leading Edge Discriminator
 *
 *  Author: Serguei Pozdniakov
 *          Jefferson Lab Online Group
 *          March 2007
 */
#ifndef __V895LIB__
#define __V895LIB__

/* Define a Structure for access to Discriminator */
typedef struct v895_struct
{
  /*0x00*/ volatile unsigned short threshold[16];         /* write only; D8  */
  /*0x20*/ volatile unsigned short res1[16];
  /*0x40*/ volatile unsigned short width[2];              /* write only; D8  */
  /*0x44*/ volatile unsigned short res2[2];
  /*0x48*/ volatile unsigned short majority_thresh;       /* write only; D8  */
  /*0x4A*/ volatile unsigned short pattern_inhibit;       /* write only; D16 */
  /*0x4C*/ volatile unsigned short test_pulse;            /* write only; D8  */
  /*0x4E*/ volatile unsigned short res3[86];
  /*0xFA*/ volatile unsigned short fixed_code;            /* read only; D16  */
  /*0xFC*/ volatile unsigned short module_type;           /* read only; D16  */
  /*0xFE*/ volatile unsigned short serial_number;         /* read only; D16  */

} V895;


#define V895_BOARD_ID 	0x0854
#define V895_ADDR       0x200000
#define V895_STEP		0x010000

#define V895_ENABLE_ALL            0xffff
#define V895_VERSION               0xf000
#define V895_SERIAL_NUMBER         0x0fff
#define V895_MANUFACTURER_NUMBER   0xfc00
#define V895_MODULE_TYPE           0x03ff
#define V895_FIXED_CODE_1          0x00ff
#define V895_FIXED_CODE_2          0xff00


#define UINT32 unsigned int
#define UINT16 unsigned short
#define INT32  int



/* Function Prototypes */
void   v895Init            ();
int    v895SetConfig       (char *fname);
void   v895MonAll          ();
void   v895MonConfig       ();
void   v895MonBoard        (int board_number);
void   v895InitGlobals     ();
int    v895ScanCLAS        (int options, int *nboards, UINT32 *discradr);
int    v895ReadConfigFile  (char *fname);
int    v895DownloadAll     ();
int    v895DownloadConfig  ();

int    v895SetMask         (UINT32 addr, UINT16 mask);
int    v895EnableChannel   (UINT32 addr, int channel);
int    v895EnableAllChan   (UINT32 addr);
int    v895EnableAll       ();
int    v895DisableChannel  (UINT32 addr, int channel);
int    v895DisableAllChan  (UINT32 addr);
int    v895DisableAll      ();
int    v895SetThreshold    (UINT32 addr, int channel, int thresh);
int    v895ReadThreshold   (UINT32 addr, int channel);
int    v895SetUniThreshold (UINT32 addr, int thresh);
int    v895SetThresholdAll (int thresh);
int    v895SetWidth        (UINT32 addr, int channel, int width);
int    v895ReadWidth       (UINT32 addr, int channel);
int    v895SetUniWidth     (UINT32 addr, int width);
int    v895SetWidthAll     (int width);
int    v895SetMajority     (UINT32 addr, int majority_level);
int    v895ReadMajority    (UINT32 addr);
int    v895TestPulse       (UINT32 addr);
int    v895GetVersion      (UINT32 addr);
int    v895GetSerialN      (UINT32 addr);
int    v895GetManufactN    (UINT32 addr);
int    v895GetModuleType   (UINT32 addr);
int    v895GetFixedCode1   (UINT32 addr);
int    v895GetFixedCode2   (UINT32 addr);

int    v895GetNumberByLocalAddr (UINT32 addr);
int    v895GetNumberByVMEAddr(UINT32 addr);
UINT32 v895GetAddrByNumber (int board_number);

#endif

