/*----------------------------------------------------------------------------*
 *  Copyright (c) 2010        Southeastern Universities Research Association, *
 *                            Thomas Jefferson National Accelerator Facility  *
 *                                                                            *
 *    This software was developed under a United States Government license    *
 *    described in the NOTICE file included as part of this distribution.     *
 *                                                                            *
 *    Authors: Bryan Moffit                                                   *
 *             moffit@jlab.org                   Jefferson Lab, MS-12B3       *
 *             Phone: (757) 269-5660             12000 Jefferson Ave.         *
 *             Fax:   (757) 269-5800             Newport News, VA 23606       *
 *                                                                            *
 *----------------------------------------------------------------------------*
 *
 * Description:
 *     Status and Control library for the JLAB Signal Distribution
 *     (SD) module using an i2c interface from the JLAB Trigger
 *     Interface/Distribution (TID) module.
 *
 * SVN: $Rev: 509 $
 *
 *----------------------------------------------------------------------------*/
#ifndef SDLIB_H
#define SDLIB_H

/* JLab SD Register definitions (defined within TID register space) */
struct TID_SDStruct
{
  /* 0x0000 */          unsigned int blankSD0[(0x3C00-0x0000)/4];
  /* 0x3C00 */ volatile unsigned int system;  /* Device 1,  Address 0 */
  /* 0x3C04 */ volatile unsigned int status;             /* Address 1 */
  /* 0x3C08 */ volatile unsigned int payloadPorts;       /* Address 2 */
  /* 0x3C0C */ volatile unsigned int tokenPorts;         /* Address 3 */
  /* 0x3C10 */ volatile unsigned int busyoutPorts;       /* Address 4 */
  /* 0x3C14 */ volatile unsigned int trigoutPorts;       /* Address 5 */
  /* 0x3C18 */ volatile unsigned int busyoutStatus;      /* Address 6 */
  /* 0x3C1C */ volatile unsigned int trigoutStatus;      /* Address 7 */
  /* 0x3C20 */ volatile unsigned int busyoutCounter[16]; /* Address 8-23 */
  /* 0x3C60 */          unsigned int RFU[7];             /* Address 24-30 */
  /* 0x3C7C */ volatile unsigned int version;            /* Address 31 */
  /* 0x3C80 */          unsigned int blankSD1[(0x10000-0x3C80)/4];
};

/* SD status bits and masks - not meaningful at the moment? */
#define SD_STATUS_CLKA_BYPASS_MODE     (1<<0)
#define SD_STATUS_CLKA_RESET           (1<<1)
#define SD_STATUS_CLKA_FREQUENCY_MASK  0x000C
#define SD_STATUS_CLKB_BYPASS_MODE     (1<<4)
#define SD_STATUS_CLKB_RESET           (1<<5)
#define SD_STATUS_CLKB_FREQUENCY_MASK  0x00C0
#define SD_STATUS_RESET                (1<<15)

/* SD status2 bits and masks -- FIXME: these have changed name/regs*/
#define SD_STATUS2_CLKB_LOSS_OF_SIGNAL  (1<<0)
#define SD_STATUS2_CLKB_LOSS_OF_LOCK    (1<<1)
#define SD_STATUS2_CLKA_LOSS_OF_SIGNAL  (1<<2)
#define SD_STATUS2_CLKA_LOSS_OF_LOCK    (1<<3)
#define SD_STATUS2_POWER_FAULT          (1<<4)
#define SD_STATUS2_TRIGOUT              (1<<5)
#define SD_STATUS2_BUSYOUT              (1<<6)
#define SD_STATUS2_LAST_TOKEN_ADDR_MASK (0x1F00)

/* SD routine prototypes */
int  sdInit();
int  sdStatus();
int  sdSetClockFrequency(int iclk, int ifreq);
int  sdGetClockFrequency(int iclk);
int  sdSetClockMode(int iclk, int imode);
int  sdGetClockMode(int iclk);
int  sdResetPLL(int iclk);
int  sdReset();
int  sdSetActivePayloadPorts(unsigned int imask);
int  sdSetActiveVmeSlots(unsigned int vmemask);
int  sdGetActivePayloadPorts();
int  sdGetBusyoutCounter(int ipayload);

#endif /* SDLIB_H */
