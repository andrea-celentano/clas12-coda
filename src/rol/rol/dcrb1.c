
/* dcrb1.c - first readout list for DCRB (VXWORKS) */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "circbuf.h"

/*****************************/
/* former 'crl' control keys */

#define ROL_NAME__ "DCRB1"

/* ts control */
#ifndef STANDALONE
#define TRIG_SUPV
#endif

/* polling mode if needed 
#define POLLING_MODE
*/

/* name used by loader */
#ifdef STANDALONE
#define INIT_NAME dcrb1_standalone__init
#else
#define INIT_NAME dcrb1__init
#endif

#include "rol.h"

/* vme readout */
#include "VME_source.h" /* POLLING_MODE for tir_triglib.h undefined here */


/* user code */

#include "uthbook.h"
#include "coda.h"
#include "tt.h"

#include "dcrbLib.h"

extern int ndcrb;                                       /* Number of DCRBs in Crate */
extern int dcrbID[DC_MAX_BOARDS]; /* slot numbers */
int DC_SLOT, islot;


/* from fputil.h */
#define SYNC 0x20000000


#undef DEBUG


/* main TI board */
#define TIRADR   0x0ed0

static char *rcname = "RC00";

/* user buffer pointers - all NULL at initialization */
extern int *userbuffers[NUSERBUFFERS];

/* pointer to TTS structures for current roc */
static/*extern*/ TTSPtr ttp1;

/* time profiling: 0-OFF, 1-ON */
static int timeprofile = 0;

/* histogram area */
static UThisti histi[NHIST];
static int neventh = 0;


/*lookup table for dcrb tests

PRE_LAST ADB board: crate 7 slot 11

#bank	sector#	sd1#	sd2#	ind#	tag#	stat#	roc#	slot#	type	channel#
#ADB connector 2, pins 9-16 (4)
DC0 	2	31	165	0	23	0	3	23	1877	0
DC0 	2	33	165	0	23	0	3	23	1877	1
DC0 	2	35	165	0	23	0	3	23	1877	2
DC0 	2	36	165	0	23	0	3	23	1877	3
DC0 	2	34	165	0	23	0	3	23	1877	4
DC0 	2	32	165	0	23	0	3	23	1877	5
DC0 	2	31	166	0	23	0	3	23	1877	6
DC0 	2	33	166	0	23	0	3	23	1877	7

#ADB connector 1, pins 9-16 (2)
DC0 	2	35	162	0	23	0	3	23	1877	8
DC0 	2	36	162	0	23	0	3	23	1877	9
DC0 	2	34	162	0	23	0	3	23	1877	10
DC0 	2	32	162	0	23	0	3	23	1877	11
DC0 	2	31	163	0	23	0	3	23	1877	12
DC0 	2	33	163	0	23	0	3	23	1877	13
DC0 	2	35	163	0	23	0	3	23	1877	14
DC0 	2	36	163	0	23	0	3	23	1877	15

#ADB connector 3, pins 9-16 (6)
DC0 	2	34	167	0	23	0	3	23	1877	16
DC0 	2	32	167	0	23	0	3	23	1877	17
DC0 	2	31	168	0	23	0	3	23	1877	18
DC0 	2	33	168	0	23	0	3	23	1877	19
DC0 	2	35	168	0	23	0	3	23	1877	20
DC0 	2	36	168	0	23	0	3	23	1877	21
DC0 	2	34	168	0	23	0	3	23	1877	22
DC0 	2	32	168	0	23	0	3	23	1877	23

#ADB connector 5, pins 9-16 (10)
DC0 	2	31	173	0	23	0	3	23	1877	24
DC0 	2	33	173	0	23	0	3	23	1877	25
DC0 	2	35	173	0	23	0	3	23	1877	26
DC0 	2	36	173	0	23	0	3	23	1877	27
DC0 	2	34	173	0	23	0	3	23	1877	28
DC0 	2	32	173	0	23	0	3	23	1877	29
DC0 	2	31	174	0	23	0	3	23	1877	30
DC0 	2	33	174	0	23	0	3	23	1877	31

#ADB connector 4, pins 9-16 (8)
DC0 	2	35	170	0	23	0	3	23	1877	32
DC0 	2	36	170	0	23	0	3	23	1877	33
DC0 	2	34	170	0	23	0	3	23	1877	34
DC0 	2	32	170	0	23	0	3	23	1877	35
DC0 	2	31	171	0	23	0	3	23	1877	36
DC0 	2	33	171	0	23	0	3	23	1877	37
DC0 	2	35	171	0	23	0	3	23	1877	38
DC0 	2	36	171	0	23	0	3	23	1877	39

#ADB connector 6, pins 9-16 (12)
DC0 	2	34	175	0	23	0	3	23	1877	40
DC0 	2	32	175	0	23	0	3	23	1877	41
DC0 	2	31	176	0	23	0	3	23	1877	42
DC0 	2	33	176	0	23	0	3	23	1877	43
DC0 	2	35	176	0	23	0	3	23	1877	44
DC0 	2	36	176	0	23	0	3	23	1877	45
DC0 	2	34	176	0	23	0	3	23	1877	46
DC0 	2	32	176	0	23	0	3	23	1877	47

#bank	sector#	sd1#	sd2#	ind#	tag#	stat#	roc#	slot#	type	channel#

#ADB connector 1, pins 1-8 (1)
DC0 	2	31	161	1	23	0	3	23	1877	0
DC0 	2	33	161	1	23	0	3	23	1877	1
DC0 	2	35	161	1	23	0	3	23	1877	2
DC0 	2	36	161	1	23	0	3	23	1877	3
DC0 	2	34	161	1	23	0	3	23	1877	4
DC0 	2	32	161	1	23	0	3	23	1877	5
DC0 	2	31	162	1	23	0	3	23	1877	6
DC0 	2	33	162	1	23	0	3	23	1877	7

#ADB connector 3, pins 1-8 (5)
DC0 	2	35	166	1	23	0	3	23	1877	8
DC0 	2	36	166	1	23	0	3	23	1877	9
DC0 	2	34	166	1	23	0	3	23	1877	10
DC0 	2	32	166	1	23	0	3	23	1877	11
DC0 	2	31	167	1	23	0	3	23	1877	12
DC0 	2	33	167	1	23	0	3	23	1877	13
DC0 	2	35	167	1	23	0	3	23	1877	14
DC0 	2	36	167	1	23	0	3	23	1877	15

#ADB connector 2 , pins 1-8 (3)
DC0 	2	34	163	1	23	0	3	23	1877	16
DC0 	2	32	163	1	23	0	3	23	1877	17
DC0 	2	31	164	1	23	0	3	23	1877	18
DC0 	2	33	164	1	23	0	3	23	1877	19
DC0 	2	35	164	1	23	0	3	23	1877	20
DC0 	2	36	164	1	23	0	3	23	1877	21
DC0 	2	34	164	1	23	0	3	23	1877	22
DC0 	2	32	164	1	23	0	3	23	1877	23

#ADB connector 4, pins 1-8 (7)
DC0 	2	31	169	1	23	0	3	23	1877	24
DC0 	2	33	169	1	23	0	3	23	1877	25
DC0 	2	35	169	1	23	0	3	23	1877	26
DC0 	2	36	169	1	23	0	3	23	1877	27
DC0 	2	34	169	1	23	0	3	23	1877	28
DC0 	2	32	169	1	23	0	3	23	1877	29
DC0 	2	31	170	1	23	0	3	23	1877	30
DC0 	2	33	170	1	23	0	3	23	1877	31

#ADB connector 6, pins 1-8 (11)
DC0 	2	35	174	1	23	0	3	23	1877	32
DC0 	2	36	174	1	23	0	3	23	1877	33
DC0 	2	34	174	1	23	0	3	23	1877	34
DC0 	2	32	174	1	23	0	3	23	1877	35
DC0 	2	31	175	1	23	0	3	23	1877	36
DC0 	2	33	175	1	23	0	3	23	1877	37
DC0 	2	35	175	1	23	0	3	23	1877	38
DC0 	2	36	175	1	23	0	3	23	1877	39

#ADB connector 5, pins 1-8 (9)
DC0 	2	34	171	1	23	0	3	23	1877	40
DC0 	2	32	171	1	23	0	3	23	1877	41
DC0 	2	31	172	1	23	0	3	23	1877	42
DC0 	2	33	172	1	23	0	3	23	1877	43
DC0 	2	35	172	1	23	0	3	23	1877	44
DC0 	2	36	172	1	23	0	3	23	1877	45
DC0 	2	34	172	1	23	0	3	23	1877	46
DC0 	2	32	172	1	23	0	3	23	1877	47

*/


static int lookup_hide[96][2] = {

  /*PRE-LAST ADB, tdc slot 23 channels 0-47, 0 */

/*ADB connector 1, pins 1-8 (1)*/
 31,	161,
 33,	161,
 35,	161,
 36,	161,
 34,	161,
 32,	161,
 31,	162,
 33,	162,

/*ADB connector 1, pins 9-16 (2)*/
 35,	162,
 36,	162,
 34,	162,
 32,	162,
 31,	163,
 33,	163,
 35,	163,
 36,	163,

/*ADB connector 2, pins 1-8 (3)*/
 34,	163,
 32,	163,
 31,	164,
 33,	164,
 35,	164,
 36,	164,
 34,	164,
 32,	164,

/*ADB connector 2, pins 9-16 (4)*/
 31,	165,
 33,	165,
 35,	165,
 36,	165,
 34,	165,
 32,	165,
 31,	166,
 33,	166,

/*ADB connector 3, pins 1-8 (5)*/
 35,	166,
 36,	166,
 34,	166,
 32,	166,
 31,	167,
 33,	167,
 35,	167,
 36,	167,

/*ADB connector 3, pins 9-16 (6)*/
 34,	167,
 32,	167,
 31,	168,
 33,	168,
 35,	168,
 36,	168,
 34,	168,
 32,	168,

/*ADB connector 4, pins 1-8 (7)*/
 31,	169,
 33,	169,
 35,	169,
 36,	169,
 34,	169,
 32,	169,
 31,	170,
 33,	170,

/*ADB connector 4, pins 9-16 (8)*/
 35,	170,
 36,	170,
 34,	170,
 32,	170,
 31,	171,
 33,	171,
 35,	171,
 36,	171,

/*ADB connector 5, pins 1-8 (9)*/
 34,	171,
 32,	171,
 31,	172,
 33,	172,
 35,	172,
 36,	172,
 34,	172,
 32,	172,

/*ADB connector 5, pins 9-16 (10)*/
 31,	173,
 33,	173,
 35,	173,
 36,	173,
 34,	173,
 32,	173,
 31,	174,
 33,	174,

/*ADB connector 6, pins 1-8 (11)*/
 35,	174,
 36,	174,
 34,	174,
 32,	174,
 31,	175,
 33,	175,
 35,	175,
 36,	175,

/*ADB connector 6, pins 9-16 (12)*/
 34,	175,
 32,	175,
 31,	176,
 33,	176,
 35,	176,
 36,	176,
 34,	176,
 32,	176

};


/* ADB crate 7 slot 5

#ADB connector 2, pins 9-16 (4)
DC0 	2	31	69	0	23	0	3	20	1877	0
DC0 	2	33	69	0	23	0	3	20	1877	1
DC0 	2	35	69	0	23	0	3	20	1877	2
DC0 	2	36	69	0	23	0	3	20	1877	3
DC0 	2	34	69	0	23	0	3	20	1877	4
DC0 	2	32	69	0	23	0	3	20	1877	5
DC0 	2	31	70	0	23	0	3	20	1877	6
DC0 	2	33	70	0	23	0	3	20	1877	7

#ADB connector 1, pins 9-16 (2)
DC0 	2	35	66	0	23	0	3	20	1877	8
DC0 	2	36	66	0	23	0	3	20	1877	9
DC0 	2	34	66	0	23	0	3	20	1877	10
DC0 	2	32	66	0	23	0	3	20	1877	11
DC0 	2	31	67	0	23	0	3	20	1877	12
DC0 	2	33	67	0	23	0	3	20	1877	13
DC0 	2	35	67	0	23	0	3	20	1877	14
DC0 	2	36	67	0	23	0	3	20	1877	15

#ADB connector 3, pins 9-16 (6)
DC0 	2	34	71	0	23	0	3	20	1877	16
DC0 	2	32	71	0	23	0	3	20	1877	17
DC0 	2	31	72	0	23	0	3	20	1877	18
DC0 	2	33	72	0	23	0	3	20	1877	19
DC0 	2	35	72	0	23	0	3	20	1877	20
DC0 	2	36	72	0	23	0	3	20	1877	21
DC0 	2	34	72	0	23	0	3	20	1877	22
DC0 	2	32	72	0	23	0	3	20	1877	23

#ADB connector 5, pins 9-16 (10)
DC0 	2	31	77	0	23	0	3	20	1877	24
DC0 	2	33	77	0	23	0	3	20	1877	25
DC0 	2	35	77	0	23	0	3	20	1877	26
DC0 	2	36	77	0	23	0	3	20	1877	27
DC0 	2	34	77	0	23	0	3	20	1877	28
DC0 	2	32	77	0	23	0	3	20	1877	29
DC0 	2	31	78	0	23	0	3	20	1877	30
DC0 	2	33	78	0	23	0	3	20	1877	31

#ADB connector 4, pins 9-16 (8)
DC0 	2	35	74	0	23	0	3	20	1877	32
DC0 	2	36	74	0	23	0	3	20	1877	33
DC0 	2	34	74	0	23	0	3	20	1877	34
DC0 	2	32	74	0	23	0	3	20	1877	35
DC0 	2	31	75	0	23	0	3	20	1877	36
DC0 	2	33	75	0	23	0	3	20	1877	37
DC0 	2	35	75	0	23	0	3	20	1877	38
DC0 	2	36	75	0	23	0	3	20	1877	39

#ADB connector 6, pins 9-16 (12)
DC0 	2	34	79	0	23	0	3	20	1877	40
DC0 	2	32	79	0	23	0	3	20	1877	41
DC0 	2	31	80	0	23	0	3	20	1877	42
DC0 	2	33	80	0	23	0	3	20	1877	43
DC0 	2	35	80	0	23	0	3	20	1877	44
DC0 	2	36	80	0	23	0	3	20	1877	45
DC0 	2	34	80	0	23	0	3	20	1877	46
DC0 	2	32	80	0	23	0	3	20	1877	47


#ADB connector 1, pins 1-8 (1)
DC0 	2	31	65	1	23	0	3	20	1877	0
DC0 	2	33	65	1	23	0	3	20	1877	1
DC0 	2	35	65	1	23	0	3	20	1877	2
DC0 	2	36	65	1	23	0	3	20	1877	3
DC0 	2	34	65	1	23	0	3	20	1877	4
DC0 	2	32	65	1	23	0	3	20	1877	5
DC0 	2	31	66	1	23	0	3	20	1877	6
DC0 	2	33	66	1	23	0	3	20	1877	7

#ADB connector 3, pins 1-8 (5)
DC0 	2	35	70	1	23	0	3	20	1877	8
DC0 	2	36	70	1	23	0	3	20	1877	9
DC0 	2	34	70	1	23	0	3	20	1877	10
DC0 	2	32	70	1	23	0	3	20	1877	11
DC0 	2	31	71	1	23	0	3	20	1877	12
DC0 	2	33	71	1	23	0	3	20	1877	13
DC0 	2	35	71	1	23	0	3	20	1877	14
DC0 	2	36	71	1	23	0	3	20	1877	15

#ADB connector 2, pins 1-8 (3)
DC0 	2	34	67	1	23	0	3	20	1877	16
DC0 	2	32	67	1	23	0	3	20	1877	17
DC0 	2	31	68	1	23	0	3	20	1877	18
DC0 	2	33	68	1	23	0	3	20	1877	19
DC0 	2	35	68	1	23	0	3	20	1877	20
DC0 	2	36	68	1	23	0	3	20	1877	21
DC0 	2	34	68	1	23	0	3	20	1877	22
DC0 	2	32	68	1	23	0	3	20	1877	23

#ADB connector 4, pins 1-8 (7)
DC0 	2	31	73	1	23	0	3	20	1877	24
DC0 	2	33	73	1	23	0	3	20	1877	25
DC0 	2	35	73	1	23	0	3	20	1877	26
DC0 	2	36	73	1	23	0	3	20	1877	27
DC0 	2	34	73	1	23	0	3	20	1877	28
DC0 	2	32	73	1	23	0	3	20	1877	29
DC0 	2	31	74	1	23	0	3	20	1877	30
DC0 	2	33	74	1	23	0	3	20	1877	31

#ADB connector 6, pins 1-8 (11)
DC0 	2	35	78	1	23	0	3	20	1877	32
DC0 	2	36	78	1	23	0	3	20	1877	33
DC0 	2	34	78	1	23	0	3	20	1877	34
DC0 	2	32	78	1	23	0	3	20	1877	35
DC0 	2	31	79	1	23	0	3	20	1877	36
DC0 	2	33	79	1	23	0	3	20	1877	37
DC0 	2	35	79	1	23	0	3	20	1877	38
DC0 	2	36	79	1	23	0	3	20	1877	39

#ADB connector 5, pins 1-8 (9)
DC0 	2	34	75	1	23	0	3	20	1877	40
DC0 	2	32	75	1	23	0	3	20	1877	41
DC0 	2	31	76	1	23	0	3	20	1877	42
DC0 	2	33	76	1	23	0	3	20	1877	43
DC0 	2	35	76	1	23	0	3	20	1877	44
DC0 	2	36	76	1	23	0	3	20	1877	45
DC0 	2	34	76	1	23	0	3	20	1877	46
DC0 	2	32	76	1	23	0	3	20	1877	47

*/

static int lookup[96][2] = {

 /*ADB connector 1, pins 1-8 (1)*/
  31,	65,
  33,	65,
  35,	65,
  36,	65,
  34,	65,
  32,	65,
  31,	66,
  33,	66,

 /*ADB connector 1, pins 9-16 (2)*/
  35,	66,
  36,	66,
  34,	66,
  32,	66,
  31,	67,
  33,	67,
  35,	67,
  36,	67,

 /*ADB connector 2, pins 1-8 (3)*/
  34,	67,
  32,	67,
  31,	68,
  33,	68,
  35,	68,
  36,	68,
  34,	68,
  32,	68,

 /*ADB connector 2, pins 9-16 (4)*/
  31,	69,
  33,	69,
  35,	69,
  36,	69,
  34,	69,
  32,	69,
  31,	70,
  33,	70,

 /*ADB connector 3, pins 1-8 (5)*/
  35,	70,
  36,	70,
  34,	70,
  32,	70,
  31,	71,
  33,	71,
  35,	71,
  36,	71,

 /*ADB connector 3, pins 9-16 (6)*/
  34,	71,
  32,	71,
  31,	72,
  33,	72,
  35,	72,
  36,	72,
  34,	72,
  32,	72,

 /*ADB connector 4, pins 1-8 (7)*/
  31,	73,
  33,	73,
  35,	73,
  36,	73,
  34,	73,
  32,	73,
  31,	74,
  33,	74,

 /*ADB connector 4, pins 9-16 (8)*/
  35,	74,
  36,	74,
  34,	74,
  32,	74,
  31,	75,
  33,	75,
  35,	75,
  36,	75,

 /*ADB connector 5, pins 1-8 (9)*/
  34,	75,
  32,	75,
  31,	76,
  33,	76,
  35,	76,
  36,	76,
  34,	76,
  32,	76,

 /*ADB connector 5, pins 9-16 (10)*/
  31,	77,
  33,	77,
  35,	77,
  36,	77,
  34,	77,
  32,	77,
  31,	78,
  33,	78,

 /*ADB connector 6, pins 1-8 (11)*/
  35,	78,
  36,	78,
  34,	78,
  32,	78,
  31,	79,
  33,	79,
  35,	79,
  36,	79,

 /*ADB connector 6, pins 9-16 (12)*/
  34,	79,
  32,	79,
  31,	80,
  33,	80,
  35,	80,
  36,	80,
  34,	80,
  32,	80

};



#define NBOARDS 22    /* maximum number of VME boards: we have 21 boards, but numbering starts from 1 */


/* v1190 start */

#define MY_MAX_EVENT_LENGTH 3000/*3200*/ /* max words per v1190 board */
#define TDC_OFFSET 0
#define CH_OFFSET  0

static int nboards, nadcs;
static int maxbytes = 0;
static int error_flag[NBOARDS];
#ifdef SLOTWORKAROUND
static int slotnums[NBOARDS];
#endif
static unsigned int tdcbuftmp[NBOARDS*MY_MAX_EVENT_LENGTH+16];
static unsigned int *tdcbuf;

static unsigned int NBsubtract = 9; /* # words to subtract including errors (5 for v1290N, 9 for others) */
/*
static int tmpgood[MY_MAX_EVENT_LENGTH];
static int tmpbad[MY_MAX_EVENT_LENGTH];
*/
#define NTICKS 1000 /* the number of ticks per second */

void
tsleep(int n)
{
  taskDelay ((sysClkRateGet() / NTICKS) * n);
}

/* v1190 end */








/* standart functions */


static void
__download()
{
  unsigned long offset, ii;

#ifdef POLLING_MODE
  rol->poll = 1;
#else
  rol->poll = 0;
#endif

  printf(">>>>>>>>>>>>>>>>>>>>>>>>>> ROCID = %d <<<<<<<<<<<<<<<<\n",rol->pid);

  printf("rol1: downloading DDL table ...\n");
  clonbanks_();
  printf("rol1: ... done.\n");

  printf("rol1: downloading translation table for roc=%d (ttp1=0x%08x)\n",rol->pid,ttp1);
  ttp1 = TT_LoadROCTT_(rol->pid, ttp1);
  printf("rol1: ... done, ttp1=0x%08x\n",ttp1);

  /* Must check address in TI module */
  /* TI uses A16 (0x29, 4 digits), not A24 */

  /* get global offset for A16 */
  sysBusToLocalAdrs(0x29,0,&offset);
  printf("A16 offset = 0x%08x\n",offset);

  tir[1] = (struct vme_tir *)(offset+TIRADR);


  /* get global offset for A24 */
  sysBusToLocalAdrs(0x39,0,&offset);
  printf("A24 offset = 0x%08x\n",offset);


  /* DMA setup */
  usrVmeDmaInit();
  usrVmeDmaSetConfig(2,2,0); /* A32, BLT */


/* RCDB start */

  /* check 'tdcbuftmp' alignment and set 'tdcbuf' to 16-byte boundary */
  tdcbuf = &tdcbuftmp[0];
  printf("alignment0: tdcbuftmp=0x%08x, tdcbuf=0x%08x\n",tdcbuftmp,tdcbuf);
  if( (((int)tdcbuf)&7) == 0xc )
  {
    tdcbuf += 1;
    printf("alignment1: tdcbuftmp=0x%08x, tdcbuf=0x%08x\n",tdcbuftmp,tdcbuf);
  }
  else if( (((int)tdcbuf)&7) == 0x8 )
  {
    tdcbuf += 2;
    printf("alignment2: tdcbuftmp=0x%08x, tdcbuf=0x%08x\n",tdcbuftmp,tdcbuf);
  }
  else if( (((int)tdcbuf)&7) == 0x4 )
  {
    tdcbuf += 3;
    printf("alignment3: tdcbuftmp=0x%08x, tdcbuf=0x%08x\n",tdcbuftmp,tdcbuf);
  }
  printf("alignment: tdcbuftmp=0x%08x, tdcbuf=0x%08x\n",tdcbuftmp,tdcbuf);


  /* 900-11mV, 2900-30mV, 4200-45.0mV, 4400-47.2mV*/
  /*DCRBInit(4200);*/
  dcInit (0x10000,0x10000,7,0);
  dcGSetDAC(30);
  dcGSetCalMask(0,0x3f);
  dcGSetProcMode(4000,2000,32);
/* RCDB end */





  logMsg("INFO: User Download Executed\n",1,2,3,4,5,6);
}


static void
__prestart()
{
  char *env;
  char *histname = "ROL1 RC00";
  unsigned short value, value0, value1, array0[32], array1[32], data16, iflag;
  int i, ii, jj, ifull, istat;
  short buf;
  unsigned short slot, channel, pattern[8];

#ifdef POLLING_MODE
  rol->poll = 1;
#else
  rol->poll = 0;
#endif

  /* Clear some global variables etc for a clean start */
  CTRIGINIT;
  *(rol->nevents) = 0;

  VME_INIT; /* init trig source VME */

  /* Register an async trigger source, (up to 32 sources) */
  /* CTRIGRSA(source,code,handler,done)*/
  /* link async trig source VME 1 to usrtrig and usrtrig_done */
#ifdef POLLING_MODE
  CTRIGRSS(VME, TIR_SOURCE, usrtrig, usrtrig_done); /* second arg=1 - what is that ? */
#else
  CTRIGRSA(VME, TIR_SOURCE, usrtrig, usrtrig_done); /* second arg=1 - what is that ? */
#endif

#ifdef POLLING_MODE
  rol->poll = 1;
#else
  rol->poll = 0;
#endif

  tttest("\npolar rol1:");

  sprintf((char *)&rcname[2],"%02d",rol->pid);
  printf("rcname >%4.4s< (ttp1=0x%08x)\n",rcname,ttp1);


  if( (env=getenv("PROFILE")) != NULL )
  {
    if(*env == 'T')
    {
      timeprofile = 1;
      logMsg("rol1: time profiling is ON\n",1,2,3,4,5,6);
    }
    else
    {
      timeprofile = 0;
      logMsg("rol1: time profiling is OFF\n",1,2,3,4,5,6);
    }
  }

  if(timeprofile)
  {
    sprintf((char *)&histname[7],"%02d",rol->pid);
    uthbook1(histi, 1000+rol->pid, histname, 200, 0, 200);
  }
/*
  printf("tmpgood=0x%08x tmpbad=0x%08x\n",tmpgood,tmpbad);
*/
  printf("INFO: User Prestart Executed\n");

  *(rol->nevents) = 0;
  rol->recNb = 0;

  return;
}       

static void
__pause()
{
  CDODISABLE(VME,TIR_SOURCE,0);

  logMsg("INFO: User Pause Executed\n",1,2,3,4,5,6);

  return;
}

static void
__go()
{  
  unsigned short value, value0, value1, array0[32], array1[32];
  int i, ii;
  int extra;

  logMsg("INFO: User Go ...\n",1,2,3,4,5,6);

#ifdef POLLING_MODE
  rol->poll = 1;
#else
  rol->poll = 0;
#endif

  CDOENABLE(VME,TIR_SOURCE,0);

  return;
}


void
usrtrig(unsigned long EVTYPE, unsigned long EVSOURCE)
{
  long EVENT_LENGTH;
  int *adrlen, *bufin, *bufout, i, ind, ind2, ncol, nrow, len, len1, len2;
  unsigned int tmp, *secondword, *jw, *buf, *dabufp1, *dabufp2;
  TIMER_VAR;
  unsigned short *buf2;
  unsigned short value;
  unsigned int nwords, nevent, nbcount, buff[32];
  unsigned short level;
  int previousIndex, kk, type;

  int ii, iii, njjloops, blen, jj, nev, rlen, rlenbuf[NBOARDS], nevts, nwrds;
  unsigned int res, datascan, mymask=0xfff0;
  unsigned int tdcslot, tdcchan, tdcval, tdc14, tdcedge, tdceventcount;
  unsigned int tdceventid, tdcbunchid, tdcwordcount, tdcerrorflags;
  unsigned int *tdc, *tdchead, itdcbuf, nbsubtract, word;
  int nheaders, ntrailers;

  rol->dabufp = (long *) 0;

  /*open event type EVTYPE of BT_UI4*/
  CEOPEN(EVTYPE, BT_UI4);

  jw = rol->dabufp;
  /*
logMsg("EVTYPE=%d\n",EVTYPE,2,3,4,5,6);
  */

  /***/

  /* at that moment only second CODA world defined  */
  /* first CODA world (length) undefined, so set it */
  /*jw[ILEN] = 1;*/ jw[-2] = 1;

  secondword = rol->dabufp - 1; /* pointer to the second CODA header word */

  /* for sync events do nothing          */
  /* for physics events send HLS scalers */

/* if want physics_sync events in external mode
if( (*(rol->nevents))%1000)
{
  syncFlag=0;
}
else
{
  syncFlag=1;
  rol->dabufp += bosMmsg_(jw,"INFO",rol->pid,"physics_sync event");
}
*/

  if((syncFlag<0)||(syncFlag>1))         /* illegal */
  {
    logMsg("Illegal1: syncFlag=%d EVTYPE=%d\n",syncFlag,EVTYPE,3,4,5,6);
  }
  else if((syncFlag==0)&&(EVTYPE==0))    /* illegal */
  {
    logMsg("Illegal2: syncFlag=%d EVTYPE=%d\n",syncFlag,EVTYPE,3,4,5,6);
  }
  else if((syncFlag==1)&&(EVTYPE==0))    /* force_sync (scaler) events */
  {

/*
!!! we are geting here on End transition: syncFlag=1 EVTYPE=0 !!!
*/


    /* report histograms
    if(timeprofile)
    {
      rol->dabufp += uth2bos(histi, 1000+rol->pid, jw);
    }
	*/


    /*logMsg("Force Sync: syncFlag=%d EVTYPE=%d\n",syncFlag,EVTYPE,3,4,5,6);*/
    /*
    ncol=32;
    nrow=1;
    if( (ind = bosMopen_(jw,"HLS+",0,ncol,nrow)) > 0)
    {
      rol->dabufp += bosMclose_(jw,ind,ncol,nrow);
    }
    if( (ind = bosMopen_(jw,"HLS+",1,ncol,nrow)) > 0)
    {
      rol->dabufp += bosMclose_(jw,ind,ncol,nrow);
    }
    */
  }
  else if((syncFlag==0)&&(EVTYPE==15)) /* helicity strob events */
  {
    ;
  }
  else           /* physics and physics_sync events */
  {

	/*
goto skip;
	*/

    rlenbuf[0] = NBOARDS*MY_MAX_EVENT_LENGTH;
	/*
    rlenbuf[0] = dcReadBlock(DC_SLOT, tdcbuf, rlenbuf[0], 2);
	*/
    rlen = 0;
    for(i=0; i<ndcrb; i++)
	{
      DC_SLOT = dcrbID[i];
      rlen += dcReadBlock(DC_SLOT, &tdcbuf[rlen], MY_MAX_EVENT_LENGTH, 1);
	}


    if(res<0 || rlenbuf[0]<=0) error_flag[6/*slot*/] = 1;

    /* open data bank */
    if((ind = bosMopen_(jw, rcname, 0, 1, 0)) <=0)
    {
      logMsg("bosMopen_ Error: %d\n",ind,0,0,0,0,0);
    }
    rol->dabufp += NHEAD;
	
    /*rlen = rlenbuf[0];*/
#ifdef DEBUG
    logMsg("rlen=%d\n",rlen,2,3,4,5,6);
#endif
    for(i=0; i<rlen; i++)
    {
      word = tdcbuf[i];                        
      if(word & 0x80000000)
      {
        type = (word>>27)&0xF;
        switch(type)
        {
          case DC_DATA_BLOCK_HEADER:
#ifdef DEBUG
            logMsg("[%3d] 0x%08X {BLKHDR} SLOTID: %d NEVENTS: %d BLOCK: %d\n",i,word,(word>>22)&0x1f,(word>>11)&0x7ff,(word>>0)&0x7ff,6);
#endif
            tdcslot = (word>>22)&0x1f;
            tdchead = rol->dabufp++; /*remember pointer to the slot header*/
            nwords = 1;
            break;

          case DC_DATA_BLOCK_TRAILER:
#ifdef DEBUG
            logMsg("[%3d] 0x%08X {BLKTLR} SLOTID: %d NWORDS: %d\n",i,word,(word>>22)&0x1f,(word>>0)&0x3fffff,5,6);
#endif
			*tdchead = ((tdcslot<<27) + nwords);
#ifdef DEBUG
			logMsg("-----> slot=%d nwords=%d -> output=0x%08x\n",tdcslot,nwords,*tdchead,4,5,6);
#endif
            break;

          case DC_DATA_EVENT_HEADER:
#ifdef DEBUG
            logMsg("[%3d] 0x%08X {EVTHDR} EVENT: %d\n",i,word,(word>>0)&0x7ffffff,4,5,6);
#endif
            break;

          case DC_DATA_TRIGGER_TIME:
#ifdef DEBUG
            logMsg("[%3d] 0x%08X {TRGTIME}\n",i,word,3,4,5,6);
#endif
            break;

          case DC_DATA_TYPE_DEFINE:
#ifdef DEBUG
            logMsg("[%3d] 0x%08X {TDCEVT} CH: %d TIME: %d ns\n",i,word, (word>>16)&0x7f,(word>>0)&0xFFFF,5,6);
#endif
            tdcchan = (word>>16)&0x7f;
            tdcval = word&0xFFFF;
			*rol->dabufp++ = ((tdcslot<<27) + (tdcchan<<17) + tdcval);
            nwords ++;
#ifdef DEBUG
			logMsg("-----> slot=%d chan=%d tdc=%d -> output=0x%08x\n",tdcslot,tdcchan,tdcval,*(rol->dabufp-1),5,6);
#endif
            break;

          case DC_DATA_INVALID:
#ifdef DEBUG
            logMsg("[%3d] 0x%08X {***DNV***}\n",i,word,3,4,5,6);
#endif
            break;

          case DC_DATA_FILLER:
#ifdef DEBUG
            logMsg("[%3d] 0x%08X {FILLER}\n",i,word,3,4,5,6);
#endif
            break;

          default:
#ifdef DEBUG
            logMsg("[%3d] 0x%08X {***DATATYPE ERROR***}\n",i,word,3,4,5,6);
#endif
            break;
        }
      }
      else
	  {
#ifdef DEBUG
        logMsg("[%3d]      0x%08X\n",i,word,3,4,5,6);
#endif
	  }
    }


	/*
	*rol->dabufp++ = 0x12345;
	*rol->dabufp++ = 0x12345;
	*rol->dabufp++ = 0x12345;
	*rol->dabufp++ = 0x12345;
	*rol->dabufp++ = 0x12345;
	*/



    blen = rol->dabufp - (int *)&jw[ind+1];
	/*logMsg("blen=%d\n",blen,2,3,4,5,6);*/
    if(blen == 0) /* no data - return pointer to the initial position */
    {
      rol->dabufp -= NHEAD;
	  /* NEED TO CLOSE BANK !!?? */
    }
    else if(blen >= (MAX_EVENT_LENGTH/4))
    {
      logMsg("1ERROR: event too long, blen=%d, ind=%d\n",blen,ind,0,0,0,0);
      logMsg(": %d %d 0x%x 0x%x\n",blen,ind,dabufp1,dabufp2,0,0);
      tsleep(1); /* 1 = 0.01 sec */
    }
    else if(bosMclose_(jw,ind,1,blen) <= 0)
    {
      logMsg("2ERROR in bosMclose_ - space is not enough !!!\n",1,2,3,4,5,6);
    }






	/*DC0 bank*/
    len = blen - 1; /*will drop header*/
	if(len>0)
	{
      ind2 = bosMopen_(jw,"DC0 ",2,2,len);
      for(i=0; i<len; i++)
      {
        word = jw[ind+2+i];
        tdcslot = (word>>27)&0x1F;
        tdcchan = (word>>17)&0x7F;
        tdcval = (word&0x7FFF)*2;
        jw[ind2+1+i] = (lookup[tdcchan][0]<<24) + (lookup[tdcchan][1]<<16) + tdcval;
      }
      rol->dabufp += bosMclose_(jw,ind2,2,len);
	}






    /* for physics sync event, make sure all board buffers are empty */
    if(syncFlag==1)
    {
      int scan_flag;
      unsigned short slot = 6;

      scan_flag = 0;
      nboards = 0;
      for(ii=0; ii<nboards; ii++)
      {
        if(nboards>0)
		{
          nev = 0/*DCRBFifoEventCount()*/;
          if(nev <= 0) /* clear board if extra event */
          {
            logMsg("SYNC: ERROR: [%2d] slot=%2d nev=%d - clear\n",
              ii,slot,nev,4,5,6);
            /*DCRBSync();*/
            scan_flag |= (1<<slot);
          }
          if(error_flag[slot] == 1) /* clear board if error flag was set */
          {
            logMsg("SYNC: ERROR: [%2d] slot=%2d error_flag=%d - clear\n",
              ii,slot,error_flag[slot],4,5,6);
            /*DCRBSync();*/
            error_flag[slot] = 0;
            scan_flag |= (1<<slot);
          }
		}

      }
	  /*
	  if(scan_flag) logMsg("SYNC: scan_flag=0x%08x\n",scan_flag,2,3,4,5,6);
	  */
	  /*logMsg("PTRN ..\n",1,2,3,4,5,6);*/
      ind2 = bosMopen_(jw,"PTRN",rol->pid,1,1);
      jw[ind2+1] = SYNC + scan_flag;
      rol->dabufp += bosMclose_(jw,ind2,1,1);
	  /*logMsg("PTRN done.\n",1,2,3,4,5,6);*/
    }	

skip:
	;
	/*
	logMsg("TRIG\n",1,2,3,4,5,6);
	*/
  }

  /* close event */
  CECLOSE;

  return;
}

void
usrtrig_done()
{
  return;
}

static void
__end()
{
  int ii;

  CDODISABLE(VME,TIR_SOURCE,0);

  logMsg("INFO: User End Executed\n",1,2,3,4,5,6);

  return;
}

void
__done()
{
  /* from parser */
  poolEmpty = 0; /* global Done, Buffers have been freed */

  /* Acknowledge tir register */
  CDOACK(VME,TIR_SOURCE,0);

  return;
}  

static void
__status()
{
  return;
}  
