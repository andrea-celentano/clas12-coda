/* rc.h */

#ifndef _RC_INCLUDED_
#define _RC_INCLUDED_

#define MAXCLNT 30

#define DANULLPROC      0
#define DACREATE        1
#define DAREMOVE        2
#define DADOWNLOAD      3
#define DAPRESTART      4
#define DAEND           5
#define DAPAUSE         6
#define DAGO            7
#define DATERMINATE     8
#define DAREPORT        9
#define DAREADINT       10
#define DAREADSTRING    11
#define DAREADREAL      12
#define DAWRITEINT      13
#define DAWRITESTRING   14
#define DAWRITEREAL     15

#define DAMODIFYINT     16
#define DAMODIFYSTRING  17
#define DAMODIFYREAL    18
#define DAWRITEEVENT    19
#define DACONFIGURE     20
#define DARESET         21
#define DABECOMEMASTER  22
#define DACANCELMASTER  23
#define DAISMASTER      24
#define DASYNC          25
#define DAREADMESSAGES  26
#define DAINSERTEVENT   27
#define DABCREPLY       28
#define DAREQEVENT      29
#define DADUMP          30
#define DAZAP           -1

#define DA_ACTIONS       30

/* Error Codes */

/*
#define RC__SUCCESS         0
#define RC__RPC_FAILURE    -1
#define RC__NOT_REGISTERED -3
#define RC__NOT_MASTER     -4

typedef struct rp {
  int32_t a ;
  int32_t b ;
} rpStruct;

typedef struct rp *runparameters;
typedef struct rp rp;

typedef struct reti {
  int32_t a;
  int32_t b;
} retiStruct;

typedef struct arg_rs *retrs;

struct arg_rs {
	int32_t value;
	char *name;
} arg_rsStruct;

typedef struct arg_rs arg_rs;

typedef struct retr {
  int32_t a;
  float b;
} retrStruct;

typedef struct argw_f {
  char *name;
  float value;
} argw_fStruct;

typedef struct argw_i {
  char *name;
  int32_t value;
} argw_iStruct;

typedef struct argw_s {
  char *name;
  char *value;
} argw_sStruct;

extern int xdr_record();
extern int xdr_event();
extern int xdr_rp();
extern int xdr_ri();
extern int xdr_arg_rs();
extern int xdr_retrs();
extern int xdr_rr();
extern int xdr_wf();
extern int xdr_wi();
extern int xdr_ws();

extern int *dacreate();
extern int *daremove();
extern int *dadownload();
extern int *daprestart();
extern int *daend();
extern int *dapause();
extern int *dago();
extern int *daterminate();
extern int *dareport();
extern int daWriteEvent();
extern int rcConnect();
extern struct reti *dareadint();
extern retrs *dareadstring();
extern int daSync();
extern char *rcStates();
extern int *dareqevent();
extern int lastContext();
extern int restoreContext();
extern int daHandleSpy();
*/

/******************************************/
/* heartbeat functions (coda_component.c) */

#define HB_MAX  1/*4*/  /* the number of monitored threads */

#define HB_ROL  0  /* from ROL */
#define HB_TCP  1  /* from TCP */
#define HB_PMC  2  /* from PMC */

int32_t checkHeartBeats();
int32_t resetHeartBeats();
int32_t setHeartBeat(int32_t, int32_t, int32_t);

#endif
