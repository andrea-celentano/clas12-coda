#ifndef PCALTriggerBoardRegs_H
#define PCALTriggerBoardRegs_H


/* Board Supports VME A32/A24 D32 Accesses (BLT32 only in address range 0x0000-0x0FFC */

#define CLOCK_PERIOD_NS					5
#define	MAX_PRESCALE					1023
#define MAX_DELAY_LONG					1023
#define MAX_DELAY						31
#define MAX_STMULT						24
#define MAX_PERSIST_LONG				255
#define MAX_PERSIST						7

#define BOARDID_A395A 					0x00	// 32CH IN LVDS/ECL INTERFACE
#define BOARDID_A395B 					0x01	// 32CH OUT LVDS INTERFACE
#define BOARDID_A395C 					0x02	// 32CH OUT ECL INTERFACE
#define BOARDID_A395D					0x03	// 8CH I/O SELECT NIM/TTL INTER

#define PCAL_BOARD_ADDRESS_1			0x11980000
#define PCAL_BOARD_ADDRESS_2			/*0x11980000*/0x11a00000

#define PCAL_FW_REVISION				0x1000
#define PCAL_CFG_SECTOR					0x1004
#define PCAL_TRIG0_SCALER				0x1008

#define PCAL_REVISION				    0x2000
#define PCAL_ENABLE_SCALERS				0x2004
#define PCAL_REF_SCALER				    0x2008

/*scope*/
#define PCAL_TRIG_STATUS				0x3000
#define PCAL_TRIG_VALUE7				0x3004
#define PCAL_TRIG_VALUE6			   	0x3008
#define PCAL_TRIG_VALUE5			   	0x300C
#define PCAL_TRIG_VALUE4			   	0x3010
#define PCAL_TRIG_VALUE3			   	0x3014
#define PCAL_TRIG_VALUE2			   	0x3018
#define PCAL_TRIG_VALUE1			   	0x301C
#define PCAL_TRIG_VALUE0			   	0x3020
#define PCAL_TRIG_INGORE7			   	0x3024
#define PCAL_TRIG_INGORE6			   	0x3028
#define PCAL_TRIG_INGORE5			   	0x302C
#define PCAL_TRIG_INGORE4			   	0x3030
#define PCAL_TRIG_INGORE3			   	0x3034
#define PCAL_TRIG_INGORE2			   	0x3038
#define PCAL_TRIG_INGORE1			   	0x303C
#define PCAL_TRIG_INGORE0			   	0x3040
#define PCAL_TRIG_BUFFER			   	0x3044


/************************************************/
/************** BEGIN SCALER REGISTERS **********/
/************************************************/
/* Notes:
   1) Scalers are all 32bits, BIG-ENDIAN.
   2) PCAL_REF_SCALER is a reference scaler which contains gate time of all scalers (in 25ns ticks)
   3) Set TS_ENABLE_SCALERS to '1' to enable scalers. Set to '0' to stop scalers for readout.
      Setting back to '1' will clear all scalers and allow them to count again.
   4) Scalers are capable of counting at 100MHz, which is about 43sec before overflowing at this high rate
*/


#define PCAL_U_DELAY_BASE               0x1200
#define PCAL_V_DELAY_BASE               0x1400
#define PCAL_W_DELAY_BASE               0x1600/*0x1200 use U delays from second board*/
#define PCAL_U_SCALER_BASE              0x1800
#define PCAL_V_SCALER_BASE              0x1A00
#define PCAL_W_SCALER_BASE              0x1C00/*0x1800 use U scalers from second board*/



/************************************************/
/******** BEGIN SECTOR TRIGGER REGISTERS ********/
/************************************************/
/* Notes:
	bits (17:16) ECPCC config
		"00" => ECP and CC
		"01" => CC
		"10" => ECP
		"11" => 0
*/

/****************/
/* DSC2 scalers */

#define PCAL_DSC2_ADDRESS_1			0x00100000
#define PCAL_DSC2_ADDRESS_2			0x00380000
#define PCAL_DSC2_ADDRESS_3			0x00600000
#define PCAL_DSC2_ADDRESS_4			0x00880000

#define PCAL_DSC2_SCALER_LATCH      0x009C
#define PCAL_DSC2_SCALER_GATE       0x00BC
#define PCAL_DSC2_SCALER_BASE       0x0100
#define PCAL_DSC2_SCALER_REF        0x0204

#endif
