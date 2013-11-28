/*----------------------------------------------------------------------------*
 *  Copyright (c) 1991, 1992  Southeastern Universities Research Association, *
 *                            Continuous Electron Beam Accelerator Facility   *
 *                                                                            *
 *    This software was developed under a United States Government license    *
 *    described in the NOTICE file included as part of this distribution.     *
 *                                                                            *
 * CEBAF Data Acquisition Group, 12000 Jefferson Ave., Newport News, VA 23606 *
 *      heyes@cebaf.gov   Tel: (804) 249-7030    Fax: (804) 249-7363          *
 *----------------------------------------------------------------------------*
 * Description:
 *	class for general CODA component
 *
 * Author:
 *	Graham Heyes
 *	CEBAF Data Acquisition Group
 *
 *---------------------------------------------------------------------------*/
/* include files follow here */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef VXWORKS

extern char *mystrdup(const char *s);


#include <types.h>
#include <errno.h>
#include <taskLib.h>
#include <bootLib.h>
#include <sysLib.h>
#include <time.h>

#include <varargs.h> /*???*/

#else

#include <pthread.h>
#include <sys/resource.h>
#include <signal.h>
#include <sys/types.h>
#include <time.h>
#include <netinet/in.h>

#ifdef __cplusplus
typedef int 		(*FUNCPTR) (...);     /* ptr to function returning int */
typedef void 		(*VOIDFUNCPTR) (...); /* ptr to function returning void */
#else
typedef int 		(*FUNCPTR) ();	   /* ptr to function returning int */
typedef void 		(*VOIDFUNCPTR) (); /* ptr to function returning void */
#endif			/* _cplusplus */

#endif


#include "rc.h"
#include "da.h"
#include "libdb.h"

#include "circbuf.h" /* to get NIMNETBUFS */

#ifdef LINUX
#include <sys/time.h>
#include <unistd.h>
#include <stdarg.h>
#include <linux/prctl.h>
#endif


#define CODA_ERROR 1
#define CODA_OK 0

#ifndef VXWORKS
#include <et_private.h>
/* more globals */
static char *et_filename = NULL;  /* for command line */
char et_name[ET_FILENAME_LENGTH];
#endif

static int iTaskCODA;
static int iTaskUDP;

/* udp */
static int udp_loop_exit;
static int udpsocket;
static int udpport;
static char udphost[128];

/*static*/ objClass localobject;

/*
 * Start of coda globals etc... There should be no globals !!!!
 */

/*static*/ char configname[128]; /* Sergey: for CODA configuration name */




/*static*/ char    *session = NULL;
static char    *objects = NULL;
/*static*/ char    *mysql_host = NULL;

static char Session[80];
static char Objects[80];
static char Mysql_host[80];

static char ObjectsName[80];
static char ObjectsClass[80];

static char    *debugString = NULL;
FUNCPTR         process_poll_proc = NULL;
unsigned int *eventNumber;
unsigned int *dataSent;


#ifdef VXWORKS
/* function prototypes */
int usrNetStackSysPoolStatus(char *title, int flag);
int usrNetStackDataPoolStatus(char *title, int flag);
#endif


int            rcdebug_level__;
int             global_code[32];
char           *global_routine[8][32];
jmp_buf         global_env[8][32];
int            global_env_depth[32];
int            use_recover = 0;
static int     codaDebugLevel = 0;




#ifdef VXWORKS

/*

taskSpawn "MMM",120,0,200000,tdtest
taskSpawn "MMM",120,0,200000,tdtest
taskSpawn "MMM",120,0,200000,tdtest

vxworks_task_delete("MMM") - deletes all 3 of them at once !!!

void
tdtest()
{
  while(1)
  {
    taskDelay(sysClkRateGet());
  }
}
*/

/* checks if task(s) with specified name exists, and delete */
int
vxworks_task_delete(char *taskname)
{
  int ret, id;

  while( (id=taskNameToId(taskname)) > 0)
  {
    ret = taskIdVerify(id);
    /*printf("taskIdVerify returns %d\n",ret);*/
    if(ret==OK)
	{
      /*printf("INFO: calling taskDelete(%d) for id=%d(0x%08x)\n",id);*/
      ret = taskDelete(id);
      taskDelay(sysClkRateGet());
      /*printf("taskDelete() returns %d\n",ret);*/
	}
    else
	{
      printf("vxworks_task_delete: ERROR: taskIdVerify() returned %d\n",ret);
	}
  }

  return(0);
}
#endif


/*****************************************/
/*** some test functions for tcpServer ***/
void ctest0() {printf("test0\n");}
void ctest1(int a) {printf("test1: a=%d\n",a);}
void ctest2(int a, int b) {printf("test2: a=%d, b=%d\n",a,b);}
void ctest3(int a, float b, int c) {printf("test3: a=%d, b=%f, c=%d\n",a,b,c);}
void ctest33(int a, float b, int c, int d) {printf("test33: a=%d, b=%f, c=%d, d=%d\n",a,b,c,d);}
void ctest4(int a, float b, char *c, int d) {printf("test4: a=%d, b=%f, c=>%s<, d=%d\n",a,b,c,d);}
void ctest5(char *s) {printf("test5: s=>%s<\n",s);}
void ctest6(char ch) {printf("test6: ch=>%c<\n",ch);}
void ctest7(float ff) {printf("test7: ff=>%f<\n",ff);}

void tests()
{
  printf("Sizes: int=%d(%d) float=%d(%d)\n",sizeof(int),sizeof(int*),sizeof(float),sizeof(float*));
}

/*******************************************************/
/*******************************************************/
/* following function provide heartbeat information */
/* heartbeat status array and function prototypes */

static int heartbeat[HB_MAX];
static int heartbeatold[HB_MAX];
static int heartbeatcount[HB_MAX];
static int heartmask[HB_MAX];
static int heartbeaterrorflag[HB_MAX];
static int heartbeaterrormsgcount[HB_MAX];
static int heartbeaterrormsg[HB_MAX][130];


/* called by main controlling thread once */
int
resetHeartBeats()
{
  int i;
  for(i=0; i<HB_MAX; i++)
  {
    heartbeat[i] = heartbeatold[i] = time(0);
    heartbeatcount[i] = -1; /* do not check */
    heartmask[i] = 0;
    heartbeaterrorflag[i] = 0;
    heartbeaterrormsgcount[i] = 0;
  }
  return(0);
}

/* called by monitored threads;
it sets appropriate bits in 'heartmask' amd sets countdown; 'checkheartbeat'
will decrement countdown counter every second and when it reaches 0 next check
will be performed */
int
setHeartBeat(int system, int bit, int countdown)
{
  if(bit<0 || bit>31)
  {
    printf("ERROR in setHeartBeat - do nothing\n");
    return(-1);
  }
  else
  {
    if(countdown<4) countdown = 4;
    /*printf("setHeartBeat: %d %d %d\n",system,bit,countdown)*/;
  }

  heartbeat[system] = time(0);
  heartbeatcount[system] = countdown;

  /*
  if(bit<=0) heartmask[system] = 1;
  else       heartmask[system] |= (1<<bit);
  */
  if(bit<=0) heartmask[system] = 0;
  else       heartmask[system] = bit;

  return(0);
}


/* returns 0 if everything fine, 1 if there is no heartbeat */
int
getHeartBeat(int system)
{
  return(heartbeaterrorflag[system]);
}


/* called by main controlling thread in a loop */
int
checkHeartBeats()
{
  int i;
  char tmp[1000];
  /*
  for(i=0; i<HB_MAX; i++) heartbeat[i] = localpmc.heartbeat[i];
  */
  for(i=0; i<HB_MAX; i++)
  {
    if(heartbeaterrormsgcount[i]>0)
	{

      /*strcpy(tmp,"err:"); send INFO, do not scare shift takers ..*/
      strcpy(tmp,"inf:");

      strcat(tmp,localobject->name);
      strcat(tmp," ");
      strcat(tmp,(char *)&heartbeaterrormsg[i][0]);

      UDP_request(tmp);

      heartbeaterrormsgcount[i] --;
      if(heartbeaterrormsgcount[i]==0) UDP_cancel(tmp);
	}
    /*else if(heartbeatcount[i]<0)
	{
      printf("INFO: HeartBeat[%1d]: do not check (%d)\n",
        i,heartbeatcount[i]);
	}*/
    else if(heartbeatcount[i]>0)
    {
      if(heartbeatcount[i]==3)
      {
        heartbeatold[i] = heartbeat[i];
      }
	  /*
      printf("INFO: HeartBeat[%1d]: decrement heartbeatcount=%d(beat=%d,%d)\n",
        i,heartbeatcount[i],heartbeat[i],heartbeatold[i]);
	  */
      heartbeatcount[i] --;

      if(heartbeatcount[i]==0)
	  {
        if(heartbeat[i]==heartbeatold[i])
        {
          sprintf(tmp,"sys %d, mask %d",i,heartmask[i]);

          /* set error */
          printf("setHeartError: %d >%s<\n",i,tmp);
          heartbeaterrorflag[i] = 1;
          heartbeaterrormsgcount[i] = 2;
          strncpy((char *)&heartbeaterrormsg[i][0],tmp,128);

          printf("WARN: HeartBeat[%1d]: heartbeat=%d(%d) heartmask=%d\n",
            i,heartbeat[i],heartbeatold[i],heartmask[i]);
        }
        else
	    {
		  /*
          printf("INFO: HeartBeat[%1d]: heartbeat=%d(%d) heartmask=0x%08x\n",
            i,heartbeat[i],heartbeatold[i],heartmask[i]);
		  */
	    }
	  }
	}
  }
  return(0);
}

/*******************************************************/
/*******************************************************/
/*******************************************************/



/* routine to dynamically load and unload readout list */

#ifdef VXWORKS
static int dalf;
#else
#include <dlfcn.h>
#endif


#include "rolInt.h"


int
codaLoadROL(ROLPARAMS *rolP, char *rolname, char *params)
{
  char ObjInitName[100];
  char *p1, *p2;
  int nchar = 0;
  int res;
#ifdef VXWORKS
  SYM_TYPE sType;
#endif

  memset((char *) ObjInitName, 0, 100);
  /* 'strrchr' returns the pointer to the last occurrence of '/' */
  /* (actual readout list name starts after last '/') */
  if((p1 = strrchr (rolname, '/')) == 0) p1 = rolname;
  /* 'strrchr' returns the pointer to the last occurrence of '.' */
  /* (actual readout list name ends before last '.') */
  if((p2 = strrchr (rolname, '.')) == 0) p2 = rolname;
  nchar = (p2 - p1) - 1;
  if(nchar > 0)
  {
    ObjInitName[0] = '_';
    strncpy(&ObjInitName[1], (p1 + 1), nchar);
    strcat(ObjInitName, "__init");
    printf("ObjInitName >%s< \n",ObjInitName);
  }
  else
  {
    printf("ERROR: cannot extract ObjInitName from the rolname\n");
  }

#if defined(VXWORKS)
  dalf = open (rolname, 0, 0);
  if(dalf == ERROR)
  {
    printf("ERROR: open failed with status=%d for rol (unable to open ROL): >%s<\n",
             dalf,rolname);
    return(CODA_ERROR);
  }

  rolP->id = (void *) loadModuleAt(dalf, GLOBAL_SYMBOLS, NULL, NULL, NULL);
  close(dalf);
#elif defined __sun||LINUX
  rolP->id = dlopen (rolname, RTLD_NOW | RTLD_GLOBAL);
  if(rolP->id == 0)
  {
	printf ("ERROR: dlopen failed on rol: >%s<\n",dlerror());
  }
#else
  printf ("WARN: dynamic loading not supported\n");
  return(CODA_ERROR);
#endif

  if(rolP->id == NULL)
  {
    printf("ERROR: unable to load readout list >%s<\n",rolname);
    return(CODA_ERROR);
  }
  rolP->nounload = 0;

#if defined(VXWORKS)
  res = (long) symFindByName (sysSymTbl, &ObjInitName[1], &rolP->rol_code,
             &sType);
  if((res != ERROR))
#else
  res = (long) dlsym (rolP->id, &ObjInitName[1]);
  rolP->rol_code = (VOIDFUNCPTR) res;
  if((res != (-1)) && (res != 0))
#endif
  {
    printf("INFO: >%s()< routine found\n",ObjInitName);
  }
  else
  {
    printf("ERROR: dlsym returned %d\n",res);
    printf("ERROR: >%s()< routine not found\n",ObjInitName);
    printf("ERROR: <ObjName>__init() routine not found in >%s<\n",rolname);
    return(CODA_ERROR);
  }

  strncpy(rolP->usrString, params, 30);
  printf ("readout list >%s< loaded\n",rolname);

  return(CODA_OK);
}


int
codaUnloadROL(ROLPARAMS *rolP)
{
#if defined(VXWORKS)
  if(unldByModuleId (rolP->id) == ERROR)
  {
    close(dalf);/*ERROR ??? must be closed already in codaLoadROL ...*/
    printf("ERROR: failed to unload old list %d\n",rolP->id);
    printf("ERROR: failed to unload list %s\n",rolP->tclName);
    return(CODA_ERROR);
  }
  printf("Old ROL object module 0x%08x unloaded\n",rolP->id);
#elif defined __sun || LINUX
  if(dlclose ((void *) rolP->id) != 0)
  {
    printf("ERROR: failed to unload list %s\n",rolP->tclName);
    return(CODA_ERROR);
  }
  printf("Old ROL object module 0x%08x unloaded\n",rolP->id);
#else
  printf("WARN: dynamic loading not yet supported on this platform\n");
#endif
  return(CODA_OK);
}






/* Routine to check the Endianness of host machine */
int
isBigEndian(void)
{
  unsigned short num, switched_num;
  
  num = (2<<8) + 1;
  switched_num = htons(num);
  if(num == switched_num) return(1);
  else                    return(0);
}


void
debug_printf(int level, char *fmt,...)
{
  va_list ap;
  int severity;
  /*
printf("debug_printf reached\n");fflush(stdout);
printf("debug_printf: level=%d, fmt >%s<\n",level,fmt);fflush(stdout);
  */
#ifdef VXWORKS
  /* Sergey ?????: va_start(ap);*/
  va_start(ap,fmt);
#else
  va_start(ap,fmt);
#endif 
 
  if (level & codaDebugLevel) {
    fprintf(stderr, "[CODA debug] ");
    
    vfprintf (stderr,fmt, ap);
  }
  /*
printf("debug_printf done\n");fflush(stdout);
  */
}

int pr_time(char *stuff) 
{
  double d1=0.,d2;
#if defined SOLARIS
  struct timespec to;
  clock_gettime(CLOCK_REALTIME,&to);
  d1 = to.tv_sec;
  d2 = to.tv_nsec;
  d1 = d1 + (d2/1000000000.0);
#elif defined LINUX
  struct timeval to;
  gettimeofday(&to,NULL);
  d1 = to.tv_sec;
  d2 = to.tv_usec;
  d1 = d1 + (d2/1000000.0);
#endif
  printf("%f %s\n",d1,stuff);
}
/*
 * C error recovery, we attempt to recover from (normally) fatal system errors.
 */

/**********************************************************
 * System error Recovery - save context to jump to in case
 * of Fatal system error in C code.
 **********************************************************/
 
/*
 * Pop a stored context off the stack (A stack of process contexts is saved, in
 * case of a system level error we can jump back to a safe state. This gives
 * simple error recovery..
 */
int
lastContext ()
{
  /*
#define pthread_self() 1
  if (global_env_depth[pthread_self() & 31] > 0)
    global_env_depth[pthread_self() & 31]--;
  */
  return(0);
}

#ifndef VXWORKS

/*
 * Error recovery Init. . .
 */

void *
signal_thread (void *arg)
{
  sigset_t   signal_set;
  int        sig_number, status;
  int        thr = (int) arg;
  char       *rtn;


  if (global_env_depth[thr]>0) {
    global_env_depth[thr]--;
  }
  rtn = global_routine[global_env_depth[thr]][thr];
  
  sigemptyset(&signal_set);
  sigaddset(&signal_set, SIGSEGV);
  sigaddset(&signal_set, SIGBUS);
  sigaddset(&signal_set, SIGILL);
  sigaddset(&signal_set, SIGFPE);
  sigaddset(&signal_set, SIGTRAP);
  sigaddset(&signal_set, SIGINT);
  sigaddset(&signal_set, SIGTERM);
  
  while (1) {
    sigwait(&signal_set, &sig_number);
    printf("signal_thread got %d\n",sig_number);
    switch (sig_number) {
      case SIGSEGV:
        printf ("ERROR Segmentation fault - presumed fatal in %s\n", rtn);
        fprintf (stdout, "ERROR Segmentation fault - presumed fatal in %s\n", rtn);
        break;
    
      case SIGBUS:
        printf ("ERROR Bus error in %s - presumed fatal\n", rtn);
        break;
		/*
      case SIGILL:
        printf ("ERROR %s, in %s", sys_errlist[errno],rtn);
        perror ("error was-");
        break;
		*/
      case SIGFPE:
        printf ("ERROR Math error (probably 1/0) in %s\n", rtn);
        break;
    
      case SIGTRAP:
        printf ("ERROR Math error (probably 1/0) in %s\n", rtn);
        break;
         
      case SIGINT:
        printf ("ERROR got SIGINT\n");
	    exit(-1);
      case SIGTERM:
		/*
        debug_printf (2,"killed by: %s", Tcl_SignalMsg (sig_number));
        Tk_doneFlag__ = 1;
		*/
        return;
        break;
        
      default:
        printf ("ERROR Unknown signal %d in %s\n", sig_number, rtn);
        return;
    }

#ifdef Linux_vme
	bb_dma_free();
#endif

    fflush(stdout);
    global_code[thr] = sig_number; 
    siglongjmp(global_env[global_env_depth[thr]][thr],sig_number);
  }
}
#endif

void
Recover_Init ()
{
#if !defined(VXWORKS) && !defined(Darwin)
  pthread_t  id;
  sigset_t   signal_set;
  int        status;
  int        thr = pthread_self() & 31;
  
  sigfillset(&signal_set);
  status = pthread_sigmask(SIG_BLOCK, &signal_set, NULL);
  if (status != 0) {
    fprintf (stderr, "Recover_Init: error in setting signal mask, %d\n", status);
    exit (3);
  }

  pthread_create(&id, NULL, signal_thread, (void *) thr);
  bzero(global_env_depth, sizeof(global_env_depth));
#endif
}


char *user_flag1 = NULL;
char *user_flag2 = NULL;
unsigned long user_flag3 = 0;
unsigned long user_flag4 = 0;


int
coda_constructor()
{
  MYSQL *dbsock;
  char tmpp[256];

printf("\n\ncoda_constructor reached\n");fflush(stdout);

  /* Allocate storage for local class information */
 localobject = (objClass) calloc (sizeof (objClassStore),1);
  bzero ((char *) localobject, sizeof (objClassStore));

  localobject->name = (char *) calloc(81,1);
  strcpy (localobject->name, ObjectsName);
  printf("object->name >%s<\n",localobject->name);

  localobject->className = ObjectsClass;
  printf("object->className >%s<\n",localobject->className);

  localobject->codaid = 0;

  printf("CODA %s,Name : %s, Type %s Id : %d\x1b[0m\n",
	 VERSION,
	 localobject->name,
	 localobject->className,
	 localobject->codaid);

  eventNumber = (unsigned long *) &localobject->nevents;
  dataSent = (unsigned long *) &localobject->nlongs;

  /* set state to booted and update 'state' field in database*/
  if(codaUpdateStatus("booted") != CODA_OK) return(CODA_ERROR);
  printf("INFO: '%s' state now '%s'\n",localobject->name,localobject->state);

  dbsock = dbConnect(getenv("MYSQL_HOST"), getenv("EXPID"));
  sprintf(tmpp,"SELECT id FROM process WHERE name='%s'",localobject->name);
  if(dbGetInt(dbsock, tmpp, &localobject->codaid)==CODA_ERROR)
  {
    dbDisconnect(dbsock);
    return(CODA_ERROR);
  }
  dbDisconnect(dbsock);
  printf("-------> codaid=%d (localobject=0x%08x)\n",localobject->codaid,localobject);

  /* tells UDP_start() this is a first call */
  udp_loop_exit = -1;

  /*set signal handler*/
  Recover_Init();

  return CODA_OK;
}


int
coda_destructor()
{
  
  if ((localobject != NULL) && (localobject->name != NULL))
  {
    printf("WARN: delete called in object '%s'\n",localobject->name);

	/*
	("database query \"UPDATE process SET inuse='no',state='down' WHERE name='",localobject->name)
	*/
  }
  return CODA_OK;
}

int
listSplit2(char *list, char *separator, int *argc, char argv[LISTARGV1][LISTARGV2])
{
  char *p, str[1024];
  strcpy(str,list);
  p = strtok(str,separator);
  *argc = 0;
  while(p != NULL)
  {
    /*printf("1[%d]: >%s< (%d)\n",*argc,p,strlen(p));*/
    strcpy((char *)&argv[*argc][0], (char *)p);
    /*printf("2[%d]: >%s< (%d)\n",*argc,(char *)&argv[*argc][0],strlen((char *)&argv[*argc][0]));*/
    (*argc) ++;
    if( (*argc) >= LISTARGV1)
	{
      printf("listSplit2 ERROR: too many args\n");
      return(0);
	}
    p = strtok(NULL,separator);
  }

  return(0);
}


/*******************************************************************/
/* main initialization routine, called from all coda_... processes */

void
CODA_Init(int argc, char **argv)
{
  char       *args, *obj, *p, buf[20];
  int        i, code;
  MYSQL *dbsock;
  int  listArgc;
  char listArgv[LISTARGV1][LISTARGV2];

  const char *help = "\nusage:\n\n coda_[roc/eb/er/etc]\n"
    "              [-session Name of current Session]\n"
    "              [-objects Name and type of this object]\n"
    "              [-name Name of object]\n"
    "              [-mysql_host Name of host to connect to for mysql access]\n";


  /* parsing command line options: loop over all arguments, except the 1st (which is program name) */
  i = 1;
  while(i<argc)
  {
    if(strncasecmp(argv[i],"-h",2)==0) {
      printf("%s",help);
      exit(0);
    }
    else if (strncasecmp(argv[i],"-session",2)==0) {
      strcpy(Session,argv[i+1]);
      session = Session;
      i=i+2;
    }
    else if (strncasecmp(argv[i],"-objects",2)==0) {
	  strcpy(Objects,argv[i+1]);
      objects = Objects;
      i=i+2;
    }
    else if (strncasecmp(argv[i],"-mysql_host",2)==0) {
      strcpy(Mysql_host,argv[i+1]);
      mysql_host = Mysql_host;
      i=i+2;
    }
    else if (strncasecmp(argv[i],"-",1)==0) {
      printf("Unknown command line arg: %s %s\n\n",argv[i],argv[i+1]);
      i=i+2;
    }
  }

  printf("CODA_Init reached, input params: >%s< >%s<\n",Session,Objects);

  if(session == NULL) session = getenv("SESSION");

  if (objects == NULL || session == NULL)
  {
    printf("ERROR: objects and session must be defined\n");
    exit(0);
  }
  else
  {
    printf("CODA_Init: use 'SESSION' as >%s<, objects as >%s<\n",session,objects);fflush(stdout);
  }


  /* ET name */
#ifndef VXWORKS
  if (et_filename == NULL)
  {
    sprintf(et_name, "%s%s", "/tmp/et_sys_", session);
  }
  else
  {
    strncpy(et_name, et_filename, ET_FILENAME_LENGTH - 1);
    et_name[ET_FILENAME_LENGTH - 1] = '\0';
  }
#endif

  /* mysql host */
  if(mysql_host)
  {
    static char tmp[100];
    sprintf(tmp,"MYSQL_HOST=%s",mysql_host);
    debug_printf(2, "MYSQL_HOST=%s\n",mysql_host);
    putenv(tmp);
  }
  else
  {
    if(!(mysql_host = getenv("MYSQL_HOST")))
    {
#ifdef VXWORKS
      mysql_host = mystrdup("localhost");
#else
      mysql_host = strdup("localhost");
#endif
    }
  }

  dbsock = dbConnect(mysql_host, getenv("EXPID"));
  printf("333333333333333\n");fflush(stdout);

  /* It would be nice here to set the uid and gid of the process to 
     match that of whoever is running RunControl */
  {
    static char tmp[256], tmpp[256], list[256];


    sprintf(tmp,"SELECT owner FROM sessions WHERE name='%s'",session);
    if(dbGetStr(dbsock, tmp, tmpp)==CODA_ERROR)
    {
      printf("cannot get 'owner' from table sessions for the name>%s<\n",session);
      dbDisconnect(dbsock);
      return;
    }
    else
    {
	  printf(">>>>>>>%d<<<<<<<<< %d\n",strlen(tmpp));fflush(stdout);
	  printf(">>>>>>>%s<<<<<<<<< %d\n",tmpp);fflush(stdout);
	  /*>>>>>>>clon00 boiarino 1538 146<<<<<<<<< 24*/
      dbDisconnect(dbsock);


      if(strlen(tmpp) > 2)
      {
        strncpy(list,tmpp,strlen(tmpp));
        list[strlen(tmpp)] = '\0';
        printf("list >%s<\n",list);
		/*list >clon00 boiarino 1538 146<*/
      }
      else
      {
        printf("ERROR: No owner information from database table sessions\n");
        printf("string length of tmpp = %d\n",strlen(tmpp));
        exit(3);
      }


	  listSplit2(list," ",&listArgc,listArgv);
      if(listArgc == 4)
      {
#ifdef VXWORKS
	    nfsAuthUnixSet(listArgv[0],
			 atoi(listArgv[2]),
			 atoi(listArgv[3]),
			 0,
			 0);
	    nfsAuthUnixShow();
#else
	    /*setuid(atoi(listArgv[2]));
	      setgid(atoi(listArgv[3]));*/
#endif
	  }
      else
      {
	    printf ("WARNING: Could not get uid and gid info from database\n");
	    printf ("         number of args in the id entry of sessions is %d\n",listArgc);
	  }

    }
  }


  
  {
    int ix;
    int  listArgc;
    char listArgv[LISTARGV1][LISTARGV2];	

	listSplit2(objects," ",&listArgc,listArgv);


    for(ix=0; ix<listArgc; ix+=2)
    {
	  
      char *class = listArgv[ix+1];
      obj = listArgv[ix];
      printf("session: >%s<\n",session);
      printf("class:   >%s<\n",class);
      printf("obj:     >%s<\n",obj);
	  

      strcpy(ObjectsName,listArgv[ix]);
      strcpy(ObjectsClass,listArgv[ix+1]);
      printf("session:      >%s<\n",session);/*clastest*/
      printf("object name:  >%s<\n",ObjectsName);/*croctest4*/
      printf("object class: >%s<\n",ObjectsClass);/*TS*/





printf("CODA_Init 13: coda_constructor starts\n");fflush(stdout);
   
 coda_constructor();

printf("CODA_Init 14: coda_constructor ends\n");fflush(stdout);




    }  


  }
}




/* Sergey */
static int CODAtcpServer(void);




/*************************************************/
/* main loop: called from all coda_... processes */

void
CODA_Execute ()
{
#ifndef VXWORKS
  pthread_t id;
#endif  
  int fd;
  int status;

  /* Sergey: start CODAtcpServer as thread */
#ifdef VXWORKS
  {
    iTaskCODA = taskSpawn("CODATCPSRV", 150, 0, 200000, CODAtcpServer,
                          0, 0, 0,0,0,0,0,0,0,0);
    printf("taskSpawn(\"CODATCPSRV\") returns %d\n",iTaskCODA);
  }
#else
  {
    pthread_attr_t attr;

    pthread_attr_init(&attr); /* initialize attr with default attributes */
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    pthread_create(&id, &attr, CODAtcpServer, NULL);
  }
#endif


  /*
   * TODO: die gracefully.  This function never returns.
   */

#ifdef VXWORKS
  while(1)
  {
    taskDelay(sysClkRateGet());
  }
#else
  while(1) sleep(1);
#endif

  exit(0);
}
    










  /*************************************************/
  /*************************************************/
  /*************************************************/
  /*************************************************/
  /*************************************************/
  /* Sergey: CLAS stuff  */




  
/*
 *-------------------------------------------------------------------------*
 *   Routine to take a Data buffer in CODA format and perform a
 *   a byte swap based on the data type in the Bank structure headers.
 *
 *   Data is maniputlated in the existing buffer so that function
 *   irreversably mangles the data. If there is an error in the Bank
 *   structure the function returns -1, otherwize it returns 0;
 *   
 *-------------------------------------------------------------------------*
 */
int
CODA_bswap(cbuf, nlongs)
  long *cbuf;
  int nlongs;
{
    int ii, jj, ix;
    int tlen, blen, dtype;
    long lwd;
    short shd;
    char cd;
    char *cp;
    short *sp;
    long *lp;

    ii = 0;
    while (ii<nlongs) {
      lp = (long *)&cbuf[ii];
      blen = cbuf[ii] - 1;
      dtype = ((cbuf[ii+1])&0xff00)>>8;
      lwd = LSWAP(*lp);    /* Swap the length      */
      *lp++ = lwd;         
      lwd = LSWAP(*lp);    /* Swap the bank header */
      *lp++ = lwd;       
      ii += 2;
      if(dtype != DT_BANK) {
	switch(dtswap[dtype]) {
	case 0:
	  /* No swap */
	  ii += blen;
	  break;
	case 1:
	  /* short swap */
	  sp = (short *)&cbuf[ii];
	  for(jj=0; jj<(blen<<1); jj++) {
	    shd = SSWAP(*sp);
	    *sp++ = shd;
	  }
	  ii += blen;
	  break;
	case 2:
          /* long swap */
	  lp = (long *)&cbuf[ii];
	  for(jj=0; jj<blen; jj++) {
	    lwd = LSWAP(*lp);
	    *lp++ = lwd;
	  }
	  ii += blen;
	  break;
	case 3:
	  /* double swap */
	  lp = (long *)&cbuf[ii];
	  for(jj=0; jj<blen; jj++) {
	    lwd = LSWAP(*lp);
	    *lp++ = lwd;
	  }
	  ii += blen;
	  break;
	default:
	  /* No swap */
	  ii += blen;
	}
      }
    }
 return(0);
}




/* returns session name */

void
getSessionName(char *sessionName, int lname)
{
  int len;

  len = strlen(session);
  /*printf("getSessionName: len %d, available %d\n",len,lname-1);*/
  len = len < (lname-1) ? len : (lname-1);
  /*printf("getSessionName: returns %d symbols\n",len);*/

  strncpy(sessionName,session,len);
  sessionName[len] = '\0';
  /*printf("getSessionName: return session name >%s<\n",sessionName);*/

  return;
}




/*****************/
/* UDP functions */


#include <sys/types.h>
#include <sys/socket.h>

#ifdef VXWORKS
static SEM_ID udp_lock;
#else
static pthread_mutex_t udp_lock;
#endif


typedef struct udpstruct
{
  int  active;          /* if 1 - send message; if 0 - do not send */
  int  timeinterval;    /* how often to send (sec) (not in use yet) */
  char message[1024];   /* message to be sent */

} UDPSTRUCT;

#define MAXUDPS 2
static UDPSTRUCT udpstr[MAXUDPS];



/* create a connection to given host and port; returns socket number
or 0 if connection was not established; host and port usually obtained
from 'process' table of database where name is session name (clasprod etc) */
int
UDP_establish(char *host, int port)
{
  struct hostent *hp, *gethostbyname();
  struct sockaddr_in sin;
  int s, slen;
  int socketnum;

  /* */
  bzero((char *)&sin, sizeof(sin));

  /* for vxworks - in libdb.c */
  hp = gethostbyname(host);
  if(hp == 0 && (sin.sin_addr.s_addr = inet_addr(host)) == -1)
  {
	printf("UDP_establish: unknown host >%s<\n",host);
	return(0);
  }
  if(hp != 0) bcopy(hp->h_addr, &sin.sin_addr, hp->h_length);
  sin.sin_port = htons(port);
  sin.sin_family = AF_INET;

  /* create a socket (use 'SOCK_STREAM' for TCP, 'SOCK_DGRAM' for UDP */
  s = socket(AF_INET, SOCK_DGRAM, 0);
  if(s < 0)
  {
    printf("UDP_establish: cannot open socket\n");
    return(0);
  }
  else
  {
    socketnum = s;
    printf("UDP_establish: socket # %d\n",socketnum);
  }


  /**********************/
  /* set socket options */
  /**********************/

  /* socket buffer size */

  slen = 48000/*29200*/;
  if(setsockopt (s, SOL_SOCKET, SO_SNDBUF, &slen, sizeof(slen)) < 0)
  {
	printf("UDP_establish: setsockopt SO_SNDBUF failed\n");
	return(0);
  }

  {
    int nbytes, lbytes;

    nbytes = 0;
    lbytes = 4;

    getsockopt(s, SOL_SOCKET, SO_SNDBUF, (int *) &nbytes, &lbytes); 
    printf("UDP_establish: socket buffer size is %d(0x%08x) bytes\n",
      nbytes,nbytes);
  }

  {
    int optval, lbytes;

	optval = 1; /* 1-yes, 0-no */
	/*make sence for TCP only, not for UDP
    if(setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) < 0)
	{
	  printf("UDP_establish: setsockopt SO_KEEPALIVE failed\n");
	  return(0);
	}

    optval = 0;
    lbytes = 4;
    getsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (int *) &optval, &lbytes);
    printf("UDP_establish: keepAlive is %d\n",optval);
	*/
  }

#ifdef VXWORKS_needheaderfiles
  {
    int optval = 0;
    if(setsockopt (s, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof (optval)) < 0)
    {
	  printf("UDP_establish: setsockopt TCP_NODELAY failed\n");
      return(0);
    }
  }
#endif

  /* connect */
  if(connect(s, (const struct sockaddr *)&sin, sizeof (sin)) < 0)
  {
    printf("UDP_establish: connect failed: host %s port %d\n",
      inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));
fflush(stdout);
exit(0);
	return(0);
  }

  printf("UDP_establish: socket %d is ready: host %s port %d\n",
      socketnum, inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));



#ifdef VXWORKS
  udp_lock = semBCreate(SEM_Q_FIFO, SEM_FULL);
  if(udp_lock == NULL)
  {
    logMsg("UDP_establish: ERROR: could not allocate a semaphore\n",1,2,3,4,5,6);
    return(0);
  }
#else
  pthread_mutex_init(&udp_lock, NULL);
#endif

  return(socketnum);
}


int
UDP_close(int socket)
{
  int retsocket;

  /* wait a while to let a few messages to be send */
#ifdef VXWORKS
    taskDelay(300);
#else
    sleep(3);
#endif

  /* reset all messages first to cleanup static structure */
  UDP_reset();

#ifdef VXWORKS
  /*semFlush(udp_lock);*/
  semGive(udp_lock);
  semDelete(udp_lock);
#else
  pthread_mutex_unlock(&udp_lock);
  pthread_mutex_destroy(&udp_lock);
#endif

  if(socket != 0)
  {
    if(shutdown(socket, 2)==0)
    {
      printf("UDP_close: socket #%d connection closed\n",socket);
      close(socket);
      retsocket = 0;
    }
    else
    {
      printf("UDP_close: ERROR in socket #%d connection closing\n",
        socket);
      retsocket = socket;
    }
  }

  return(retsocket);
}

/* request 'standard' UDP message to be send */
int
UDP_standard_request(char *name, char *state)
{
  char tmp[1000];

  strcpy(tmp,"sta:");
  strcat(tmp,name);
  strcat(tmp," ");
  strcat(tmp,state);

  printf("UDP_standard_request >%s<",tmp);
  printf("\n");
  printf("UDP_standard_request >%s<",tmp);
  printf("\n");
  printf("UDP_standard_request >%s<",tmp);
  printf("\n");
  printf("UDP_standard_request >%s<",tmp);
  printf("\n");
  printf("UDP_standard_request >%s<",tmp);
  printf("\n");
  printf("UDP_standard_request >%s<",tmp);
  printf("\n");

  UDP_request(tmp);

  return(0);
}

/* remove all UDP messages */
int
UDP_reset()
{
  int i;

#ifdef VXWORKS
  semTake(udp_lock, WAIT_FOREVER);
#else
  pthread_mutex_lock(&udp_lock);
#endif

  printf("UDP_reset reached\n");
  /* cancel all messages */
  for(i=0; i<MAXUDPS; i++)
  {
    printf("UDP_cancel: cancel >%s<\n",udpstr[i].message);
    udpstr[i].active = 0;
    udpstr[i].message[0] = '\0'; /* just in case */
  }

#ifdef VXWORKS
  semGive(udp_lock);
#else
  pthread_mutex_unlock(&udp_lock);
#endif

  return(0);
}

/* cancel UDP message */
int
UDP_cancel(char *str)
{
  int i;

#ifdef VXWORKS
  semTake(udp_lock, WAIT_FOREVER);
#else
  pthread_mutex_lock(&udp_lock);
#endif

  /* cancel all messages with identical key (first 4 characters) */
  for(i=0; i<MAXUDPS; i++)
  {
    if(!strncmp(udpstr[i].message, str, 4))
    {
      printf("UDP_cancel: cancel >%s<\n",udpstr[i].message);
      udpstr[i].active = 0;
      udpstr[i].message[0] = '\0'; /* just in case */
      /*break;*/ /*scan whole structure in case if more then one message*/
    }
  }

#ifdef VXWORKS
  semGive(udp_lock);
#else
  pthread_mutex_unlock(&udp_lock);
#endif

  return(0);
}

/* request UDP message to be send */
int
UDP_request(char *str)
{
  int i, found;

  /* cancel all messages with the same key (first 4 characters) */
  UDP_cancel(str);

#ifdef VXWORKS
  semTake(udp_lock, WAIT_FOREVER);
#else
  pthread_mutex_lock(&udp_lock);
#endif

  found = 0;
  for(i=0; i<MAXUDPS; i++)
  {
    if(udpstr[i].active == 0)
    {
      found = 1;
      udpstr[i].active = 1;
      strcpy(udpstr[i].message, str);
      break;
    }
  }

#ifdef VXWORKS
  semGive(udp_lock);
#else
  pthread_mutex_unlock(&udp_lock);
#endif

  if(found) return(0);

  printf("UDP_request: no empty slots, message was not accepted\n");
  return(1);
}


/* returns number of data bytes written or -1 if error */
/* 'str' must be string */
int
UDP_send(int socket)
{
  char tmp[1000], tmpp[1000];
  int i, nevents, nlongs, eventdiff;
  time_t newtime, timediff;
  float event_rate, data_rate;
  static int oldevents, oldlongs;
  static time_t oldtime;
  int nbytes, cc, rembytes;
  char *buffer2;

  if(oldtime==0) oldtime=time(0);

#ifdef VXWORKS
  semTake(udp_lock, WAIT_FOREVER);
#else
  pthread_mutex_lock(&udp_lock);
#endif

  for(i=0; i<MAXUDPS; i++)
  {
	/*printf("UDP_send[%d]: active=%d\n",i,udpstr[i].active);*/
    if(udpstr[i].active==0) continue;

    /* for the message started from 'sta:' update statistic info */
    if( !strncmp(udpstr[i].message,"sta:",4) )
    {
      nevents = *eventNumber;
      nlongs = *dataSent;

      newtime = time(0); /* time in seconds */

      event_rate = data_rate = 0.0;

      timediff = newtime - oldtime;
      eventdiff = nevents - oldevents;
	  /*printf("timediff: %u (%u - %u)\n",timediff,newtime,oldtime);*/
      if(timediff != 0)
      {
		if(eventdiff != 0)
	    {
          event_rate = eventdiff/timediff;
  	      /*printf("event_rate: %f (%u - %u)\n",event_rate,nevents,oldevents);*/
          data_rate = 4*(nlongs - oldlongs)/timediff;

          oldlongs = nlongs;
          oldevents = nevents;
          oldtime = newtime;

	      strcpy(tmp,udpstr[i].message);
          sprintf(tmpp," %d %9.3f %d %12.3f",nevents,event_rate,nlongs,data_rate);
	      /*printf("UDP_send[%d]: %d %9.3f %d %12.3f\n",i,nevents,event_rate,nlongs,data_rate);*/
          strcat(tmp,tmpp);
		}
        else if(timediff>=3) /* if 3 seconds without rate, send message with zero rates: */
		{                    /* have to send something to make runcontrol happy          */
	      strcpy(tmp,udpstr[i].message);
          sprintf(tmpp," %d %9.3f %d %12.3f",nevents,event_rate,nlongs,data_rate);
	      /*printf("UDP_send[%d]: %d %9.3f %d %12.3f\n",i,nevents,event_rate,nlongs,data_rate);*/
          strcat(tmp,tmpp);
		}
        else
		{
          tmp[0] = '\0'; /* do not send anything */
		}
      }
      else
	  {
        tmp[0] = '\0'; /* do not send anything */
	  }

    }
    else
    {
      strcpy(tmp,udpstr[i].message); /* not 'sta:' messages */
    }

	/*	
    printf("UDP_send[%d] >%s<\n",i,tmp);
    printf(" udpport=%d\n",udpport);
	*/

    nbytes = strlen(tmp);
    if(nbytes == 0)
    {
      /*printf("UDP_send: no data - return\n");*/
      goto exit3;
    }

    nbytes++; /* add one char for the end of string sign */
    rembytes = nbytes;
    buffer2 = tmp;
    while(rembytes)
    {
retry3:

      cc = write(socket,buffer2,rembytes);
	  /*cc = send(socket,buffer2,rembytes,NULL);*/
/*printf("2: %d %d\n",rembytes,cc);*/

      if(cc == -1)
      {
        if(errno == EWOULDBLOCK)
        {
          printf("Operation would block 2: retry ...\n");
          goto retry3;
        }

        if(errno==ENOBUFS) printf("No buffer space available (errno=%d)\n",errno);
        else if(errno==EPIPE) printf("Broken pipe (errno=%d)\n",errno);
        else if(errno==ECONNREFUSED)
        {
          /*if rcServer dies, mv2400/mv5500/mv6100 reports following message,
          while mv5100 does not !!!; I think mv5100 is right and others are
          wrong, but must figure it out ... */
          /*printf("Connection refused (errno=%d)\n",errno);*/
          ;
        }
        else printf("Unknown error errno=%d\n",errno);




/* !!! if errno==61(ECONNREFUSED), must try to reestablish connection (rcServer died) */



/*
netStackDataPoolShow - not enough buffers !!!
netStackSysPoolShow
*/
        nbytes = -1;
        goto exit3;
      }

      buffer2 += cc;
      rembytes -= cc;
      if(rembytes!=0) printf("WARN: UDP_send: rembytes=%d\n",rembytes);
    }

exit3:
    ;

  } /* for() */

#ifdef VXWORKS
  semGive(udp_lock);
#else
  pthread_mutex_unlock(&udp_lock);
#endif

  return(nbytes);
}

void
UDP_show()
{
  int i;

  for(i=0; i<MAXUDPS; i++)
  {
    if(udpstr[i].active)
      printf("udpstr[%1d].message >%s<\n",i,udpstr[i].message);
  }
}


int
codaUpdateStatus(char *status)
{
  objClass object = localobject;
  MYSQL *dbsock;
  char tmp[1000];

  /* update database */
  printf("codaUpdateStatus: dbConnecting ..\n");fflush(stdout);
  dbsock = dbConnect(getenv("MYSQL_HOST"), getenv("EXPID"));
  if(dbsock==NULL)
  {
    printf("cannot connect to the database 7 - exit\n");
    exit(0);
  }
printf("codaUpdateStatus: dbConnect done\n");fflush(stdout);
  sprintf(tmp,"UPDATE process SET state='%s' WHERE name='%s'",
    status,localobject->name);
  printf("codaUpdateStatus: >%s<\n",tmp);
  if(mysql_query(dbsock, tmp) != 0)
  {
    printf("codaUpdateStatus: ERROR\n");
    return(CODA_ERROR);
  }
printf("codaUpdateStatus: dbDisconnecting ..\n");fflush(stdout);
  dbDisconnect(dbsock);
printf("codaUpdateStatus: dbDisconnect done\n");fflush(stdout);
  
  /* update request record */
printf("codaUpdateStatus: updating request ..\n");fflush(stdout);
  UDP_standard_request(localobject->name,status);
printf("codaUpdateStatus: updating request done\n");fflush(stdout);

  /* TODO: need also to keep status in some local variable .. */
  /*tcpState ? rocp->state ? */

  return(CODA_OK);
}


/* that function sends our status to rcServer every second */
void
UDP_loop()
{
#ifdef Linux
  prctl(PR_SET_NAME,"coda_udp");
#endif

  printf("UDP_loop started\n");fflush(stdout);
  printf("UDP_loop started\n");fflush(stdout);
  printf("UDP_loop started\n");fflush(stdout);
  printf("UDP_loop started\n");fflush(stdout);
  printf("UDP_loop started\n");fflush(stdout);

  udpsocket = UDP_establish(udphost, udpport);

  /* initialize heartbeats */
  resetHeartBeats();

  while(1)
  {
    if(udp_loop_exit)
    {
      break;
    }

    /* this is appropriate place to check if all others are running fine */
    checkHeartBeats();

    UDP_send(udpsocket);

    /* wait 1 sec */
#ifdef VXWORKS
    taskDelay(100);
#else
    sleep(1);
#endif

  }

  UDP_close(udpsocket);
  udp_loop_exit = 0;

#ifndef VXWORKS
  printf("UDP_loop: calls pthread_exit\n");
  pthread_exit(NULL);
#endif

  return;
}

/* start thread to report our status to rcServer's UDP port */
int
UDP_start()
{
  MYSQL *dbsock;
  char tmpp[1000];
  int ii, iii;

  dbsock = dbConnect(getenv("MYSQL_HOST"), getenv("EXPID"));

  sprintf(tmpp,"SELECT host FROM process WHERE name='%s'",session);
  if(dbGetStr(dbsock, tmpp, udphost)==CODA_ERROR) return(CODA_ERROR);

  sprintf(tmpp,"SELECT port FROM process WHERE name='%s'",session);
  if(dbGetInt(dbsock, tmpp, &udpport)==CODA_ERROR) return(CODA_ERROR);

  printf("download: UDP host is >%s< port id %d\n",udphost,udpport);

  dbDisconnect(dbsock);



  /* if it is NOT first call, exit UDP loop and start new one */
  if(udp_loop_exit != -1)
  {
    udp_loop_exit = 1;

    ii = 6;
    while(udp_loop_exit)
    {
      printf("download: wait for udp_loop to exit ..\n");
#ifdef VXWORKS
      taskDelay(100);
#else
      sleep(1);
#endif
      ii --;
      if(ii<0) break;
    }

    if(ii<0)
	{
      printf("WARN: cannot exit udp_loop gracefully, will kill it\n");
#ifdef VXWORKS
	  vxworks_task_delete("UDP_LOOP");
      iTaskUDP = 0;
#else
      /* TODO: delete udp_loop thread */
      sleep(1);
#endif
      udp_loop_exit = 0; /* to let new UDP_LOOP to start */
	}

  }

  /* just in case */
  udp_loop_exit = 0;


#ifdef VXWORKS
  {
    /*taskSpawn(name,pri,opt,stk,adr,args) 10 args required */
    iTaskUDP = taskSpawn("UDP_LOOP", 110, 0, 200000, UDP_loop,
                          0,0,0,0,0,0,0,0,0,0);
    printf("taskSpawn(\"udp_loop\") returns %d\n",iTaskUDP);
  }
#else
  {
    int res;
    pthread_t thread1;
    pthread_attr_t detached_attr;

    pthread_attr_init(&detached_attr);
    pthread_attr_setdetachstate(&detached_attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setscope(&detached_attr, PTHREAD_SCOPE_SYSTEM);

    res = pthread_create( (unsigned int *) &thread1, &detached_attr,
		   (void *(*)(void *)) UDP_loop, (void *) NULL);

    printf("pthread_create returned %d\n",res);fflush(stdout);
    perror("pthread_create");
  }
#endif

  return(0);
}





/***************************************************/
/***************************************************/
/***************************************************/
/* CODAtcpServer functions - to receive commands from runcontrol */

#ifdef VXWORKS

#include <vxWorks.h>
#include <sockLib.h>
#include <inetLib.h>
#include <taskLib.h>
#include <stdioLib.h>
#include <strLib.h>
#include <ioLib.h>
#include <fioLib.h>

#else

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define TRUE 1
#define FALSE 0
#define OK 0
#define ERROR (-1)
#define STD_IN stdin

#endif

#include "libtcp.h" 

/* readiness flag */
static int coda_request_in_progress;
/* currently processed message */
static char coda_current_message[REQUEST_MSG_SIZE];
/* function declarations */ 
static void CODAtcpServerWorkTask(TWORK *targ); 

/**************************************************************************** 
* * CODAtcpServer - accept and process requests over a TCP socket 
* * This routine creates a TCP socket, and accepts connections over the socket 
* from clients. Each client connection is handled by spawning a separate 
* task to handle client requests. 
* * This routine may be invoked as follows: 
* -> sp CODAtcpServer
* task spawned: id = 0x3a6f1c, name = t1 
* value = 3829532 = 0x3a6f1c 
* -> MESSAGE FROM CLIENT (Internet Address 150.12.0.10, port 1027): 
* Hello out there 
* * RETURNS: Never, or ERROR if a resources could not be allocated. */ 

int
CODAtcpServer(void)
{
  struct sockaddr_in serverAddr; /* in (it's me) */
  struct sockaddr_in clientAddr; /* from */
  int sockAddrSize;              /* size of socket address structure */ 
  int sFd;                       /* socket file descriptor */ 
  int ix = 0;                    /* counter for work task names */
  int portnum = SERVER_PORT_NUM; /* desired port number; can be changed if that number in use enc */
  char workName[16];             /* name of work task */ 
  static TWORK targ;
  MYSQL *dbsock;
  char tmp[1000], temp[100];

#ifdef Linux
  prctl(PR_SET_NAME,"coda_tcp");
#endif

  /* some cleanup */
  sockAddrSize = sizeof(struct sockaddr_in);
  bzero((char *)&serverAddr, sockAddrSize); 
  bzero((char *)&clientAddr, sockAddrSize); 

  /* creates an endpoint for communication and returns a socket file descriptor */
  if((sFd = socket(AF_INET, SOCK_STREAM, 0)) == ERROR)
  {
    perror("socket"); 
    return(ERROR); 
  } 

  /* set up the local address */ 
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(portnum); /* set desired port number */
  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* create a TCP-based socket (???) */ 


  /* bind socket to local address */
  while(bind(sFd, (struct sockaddr *)&serverAddr, sockAddrSize) == ERROR)
  {
    perror("bind");

    /* try another port (just increment on one) */
    portnum ++;
    if((portnum-SERVER_PORT_NUM) > 50)
    {
      close(sFd); 
      return(ERROR);
    }

    serverAddr.sin_port = htons(portnum);
  }
  printf("bind on port %d\n",portnum);

  /* create queue for client connection requests */ 
  if(listen(sFd, SERVER_MAX_CONNECTIONS) == ERROR)
  {
    perror("listen"); 
    close(sFd); 
    return(ERROR); 
  }

  /* update database with port number */
  dbsock = dbConnect(getenv("MYSQL_HOST"), getenv("EXPID"));
  sprintf(temp,"%d",portnum);

  /* use 'inuse' field; replace 'inuse' by 'port' when DP_ask not in use !!! */
  sprintf(tmp,"UPDATE process SET inuse='%s' WHERE name='%s'",
    temp,localobject->name);

  printf("DB update: >%s<\n",tmp);
  if(mysql_query(dbsock, tmp) != 0)
  {
    printf("DB update: ERROR\n");
    return(ERROR);
  }
  dbDisconnect(dbsock);

  coda_request_in_progress = 0;
  /* accept new connect requests and spawn tasks to process them */ 
  while(1)
  {
#ifdef VXWORKS
    /* check for free net buffers */
    usrNetStackSysPoolStatus("CODAtcpServer",0);
    usrNetStackDataPoolStatus("CODAtcpServer",0);
#endif

    /* do not accept new request if current one is not finished yet; too
    many requests may create network buffer shortage */
    if(coda_request_in_progress)
    {
      printf("wait: coda request >%s< in progress\n",coda_current_message);
#ifdef VXWORKS
      taskDelay(100);
#else
      sleep(1);
#endif
      continue;
    }

    /*printf("Waiting at 'accept' ..\n"); fflush(stdout);*/
    if((targ.newFd = accept(sFd, (struct sockaddr *)&clientAddr, &sockAddrSize))
          == ERROR)
    {
      perror("accept"); 
      close(sFd); 
      return(ERROR); 
    }
    /*printf(".. 'accept' !!\n"); fflush(stdout);*/

    targ.address = inet_ntoa(clientAddr.sin_addr);
	/*
printf("WorkTask: alloc 0x%08x\n",targ.address);fflush(stdout);
printf("WorkTask: alloc 0x%08x\n",targ.address);fflush(stdout);
printf("WorkTask: alloc 0x%08x\n",targ.address);fflush(stdout);
printf("WorkTask: alloc 0x%08x\n",targ.address);fflush(stdout);
printf("WorkTask: alloc 0x%08x\n",targ.address);fflush(stdout);
	*/
    targ.port = ntohs (clientAddr.sin_port);

    coda_request_in_progress = 1;
#ifdef VXWORKS
    sprintf(workName,"twork%04d",ix++);
    if(taskSpawn(workName, 150/*SERVER_WORK_PRIORITY*/, 0, SERVER_STACK_SIZE,
       (FUNCPTR) CODAtcpServerWorkTask, &targ,
	   0, 0, 0, 0, 0, 0, 0, 0, 0)
          == ERROR)
    {
	  perror("taskSpawn"); 
	  close(targ.newFd); 
      coda_request_in_progress = 0;
    }
#else
    printf("CODAtcpServer: start work thread\n");
	{
      int ret;
	  pthread_t id;
      pthread_attr_t detached_attr;

      pthread_attr_init(&detached_attr);
      pthread_attr_setdetachstate(&detached_attr, PTHREAD_CREATE_DETACHED);
      pthread_attr_setscope(&detached_attr, PTHREAD_SCOPE_SYSTEM);

      printf("befor: socket=%d address>%s< port=%d\n",
        targ.newFd,targ.address,targ.port); fflush(stdout);

      /* block annoying IP address(es) */
      if(!strncmp(targ.address,"129.57.71.",10))
	  {
        printf("WARN: ignore request from %s\n",targ.address);
        close(targ.newFd);
        coda_request_in_progress = 0;
	  }
      else
	  {
        ret = pthread_create(&id, &detached_attr, CODAtcpServerWorkTask, &targ);
        if(ret!=0)
        {
          printf("ERROR: pthread_create(CODAtcpServerWorkTask) returned %d\n",
            ret);
          close(targ.newFd);
          coda_request_in_progress = 0;
        }
	  }
	}
#endif
 
  }

  /* here we will remove port number from DB ??? */ 
  /* or do it in destructor ?? */

  return;
}

/**************************************************************************** 
* * CODAtcpServerWorkTask - process client requests 
* * This routine reads from the server's socket, and processes client 
* requests.
* * RETURNS: N/A. */ 


static int codaExecute(char *message);

static void
CODAtcpServerWorkTask(TWORK *targ) 
{
  TREQUEST clientRequest;            /* request/message from client */ 
  int nRead;                               /* number of bytes read */
  char message[REQUEST_MSG_SIZE];
  int itmp;

#ifdef Linux
  prctl(PR_SET_NAME,"coda_tcp_work");
#endif
  /*setHeartBeat(HB_TCP,0,2);*/

  /*
  printf("CODAtcpServerWorkTask: socket=%d address>%s< port=%d\n",
    targ->newFd,targ->address,targ->port); fflush(stdout);
  */

#ifdef VXWORKS

  if( (nRead = recv(targ->newFd, (char *) &clientRequest, sizeof (TREQUEST), NULL)) > 0 )
  {
	/*
    printf ("MESSAGE (nRead=%d, Address>%s<, port=%d): Executing >%s<\n", 
	    nRead, targ->address, targ->port, clientRequest.message);
	*/
    strcpy(message, clientRequest.message);

    /* store it to be used later for debugging */
    strcpy(coda_current_message, message);

    /* try Executing the message (each component must provide codaExecute() function */
	/*
    printf("Executing >%s<\n",message);
	*/
	/*setHeartBeat(HB_TCP,1,2);*/
    codaExecute(message);
	/*setHeartBeat(HB_TCP,2,2);*/

    free(targ->address); /* free malloc from inet_ntoa() */ 
  }
  else if(nRead == 0)
  {
    printf("connection closed, exit thread\n");
  }
  else
  {
    perror("ERROR (recv)"); 
    fflush(stdout);

	/* do NOT free() here, error was observed:
CODAtcpServerWorkTask: socket=17 address>129.57.68.21< port=40181
Error: nRead=-1, must be 1032
0x8268e0 (twork0008): memPartFree: invalid block 0x86f910 in partition 0x250254.
	 */
  }
  /*
printf("WorkTask: free 0x%08x\n",targ->address);fflush(stdout);
printf("WorkTask: free 0x%08x\n",targ->address);fflush(stdout);
printf("WorkTask: free 0x%08x\n",targ->address);fflush(stdout);
printf("WorkTask: free 0x%08x\n",targ->address);fflush(stdout);
printf("WorkTask: free 0x%08x\n",targ->address);fflush(stdout);
  */
  close(targ->newFd);    /* close server socket connection */ 
  coda_request_in_progress = 0;


#else /* UNIX */


  if( (nRead = recv(targ->newFd, (char *) &clientRequest, sizeof (TREQUEST), NULL)) > 0 )
  {
    /* convert integers from network byte order */
    clientRequest.msgLen = ntohl(clientRequest.msgLen);
    clientRequest.reply = ntohl(clientRequest.reply);

	/*
    printf ("MESSAGE (nRead=%d, Address>%s<, port=%d): Executing >%s<\n", 
	    nRead, targ->address, targ->port, clientRequest.message);
	*/
    strcpy(message, clientRequest.message);

    /* store it to be used later for debugging */
    strcpy(coda_current_message, message);

    /* try Executing the message (each component must provide codaExecute() function */
    /*do not print: message may contains bad characters, it will be checked inside codaExecute
           printf("Executing >%s< (len=%d)\n",message,strlen(message));*/

	/* check if message makes sence */
    codaExecute(message);
  }
  else if(nRead == 0)
  {
    printf("connection closed, exit thread\n");
  }
  else
  {
    perror("ERROR (recv)"); 
  }

  /*free(targ->address);-stuck here !!!*/ /* free malloc from inet_ntoa() */ 
  close(targ->newFd);  /* close server socket connection */ 

  coda_request_in_progress = 0;

  /* terminate calling thread */
  pthread_exit(NULL);

#endif
  /*setHeartBeat(HB_TCP,0,-1);*/

  return;
}







/**************************/
/* some service functions */

/* flag=0: use ' ' as dividers */
/* flag=1: use {} as dividers */
int
listSplit1(char *list, int flag,
           int *argc, char argv[LISTARGV1][LISTARGV2])
{
  int i, i1, i2, len, k, ll;

  len = strlen(list);

  k = 0;
  *argc = 0;
  while(k<len)
  {
    i1 = i2 = -1;
    for(i=k; i<len; i++)
    {
      if(flag)
	  {
        if(list[i]=='{')
        {
          i1 = i;
          break;
        }
	  }
      else
      {
        i1 = i;
        while(list[i1]==' ') i1 ++; /* remove leading spaces if any */
        break;
      }
    }
    if(i1 == -1)
    {
      return(0);
    }
    if(flag) i1 ++; /* next char after { */

    for(i=i1; i<len; i++)
    {
      if(flag)
	  {
        if(list[i]=='}')
        {
          i2 = i;
          break;
        }
	  }
      else
      {
        if(i>=(len-1))
        {
          i2 = i+1;
          break;
        }
        else if(list[i]==' ')
        {
          i2 = i;
          break;
        }
      }
    }
    if(i2 == -1)
    {
      return(0);
    }
    i2 --; /* previous char before '}'  or ' ' */

    /* check for argv overflow */
    if( (*argc) >= LISTARGV1)
    {
      /*logMsg("listSplit1: too many strings in the list - earlear return\n",
        1,2,3,4,5,6);*/
      return(1);
    }
    if( (i2-i1+2) > LISTARGV2)
    {
      /*logMsg("listSplit1: too long strings in the list - earlear return\n",
        1,2,3,4,5,6);*/
      return(2);
    }

    ll = i2-i1+1;
    memcpy((char *)&argv[*argc][0], (char *)&list[i1], ll);

    argv[*argc][ll] = '\0';

    /*logMsg("i1=%d i2=%d >%s<\n",i1,i2,argv[*argc],4,5,6);*/
    (*argc) ++;

    if(flag) k = i2 + 2;
    else     k = i2 + 1;
  }

  return(0);
}



#if defined Linux || defined Darwin

/* Linux and Darwin do not have gethrtime() */

/*
  double d1=0.,d2;
#if defined SOLARIS
  struct timespec to;
  clock_gettime(CLOCK_REALTIME,&to);
  d1 = to.tv_sec;
  d2 = to.tv_nsec;
  d1 = d1 + (d2/1000000000.0);
#elif defined LINUX
  struct timeval to;
  gettimeofday(&to,NULL);
  d1 = to.tv_sec;
  d2 = to.tv_usec;
  d1 = d1 + (d2/1000000.0);
#endif
  printf("%f %s\n",d1,stuff);
*/

hrtime_t/*uint64_t*/
gethrtime(void)
{
  static double firstsecs = 0.;
  uint64_t ret;
  double microsecs;
  double d1=0.,d2;
  struct timeval to;
  gettimeofday(&to, NULL);
  d1 = to.tv_sec;
  if(firstsecs==0) firstsecs=d1;
  d1 = d1 - firstsecs;
  d2 = to.tv_usec;
  /*printf("d1=%f d2=%f\n",d1,d2);*/
  microsecs = d2 + d1*1000000.0;
  /*printf("microsecs=%f\n",microsecs);*/
  ret = microsecs;
  /*printf("ret=%ld\n",ret);*/

  /*
  int nsec_total;
  struct timeval to, abs_time;
  nsec_total = 1000 * to.tv_usec + 1000000000 * to.tv_sec;
  if (nsec_total >= 1000000000) {
    abs_time.tv_nsec = nsec_total - 1000000000;
    abs_time.tv_sec  = deltatime->tv_sec + to.tv_sec + 1;
  }
  else {
    abs_time.tv_nsec = nsec_total;
    abs_time.tv_sec  = deltatime->tv_sec + to.tv_sec;
  }
  */

  return(ret);
}

void
gethrtimetest()
{
  int ii;
  uint64_t jj;

  for(ii=0; ii<10; ii++)
  {
    jj = gethrtime();
    printf("jj=%ld\n",jj);
    sleep(1);
  }
  return;
}

#endif


/******************************************/
/* buffer pool information - VxWorks only */
/******************************************/


#ifdef VXWORKS

/* see netBufLib.h */

/* return the number of times failed to find space */
/* flag: 1 - before, 2 - after */
int
usrNetStackSysPoolStatus(char *title, int flag)
{
  int i;
  NET_POOL *mySysPtr = ( NET_POOL *)_pNetSysPool; /*system mbuf pool (global)*/
  CL_POOL  *myPoolPtr[CL_TBL_SIZE]; /* point at each pool cluster */
  /*
  printf("=======================================\n");
  printf("=======================================\n");
  printf("mbufs   = %d\n",mySysPtr->pPoolStat->mNum);
  printf("failed  = %d\n",mySysPtr->pPoolStat->mDrops);
  printf("waited  = %d\n",mySysPtr->pPoolStat->mWait);
  printf("drained = %d\n",mySysPtr->pPoolStat->mDrain);
  */

  if(flag==1)      printf("netSys(befor):  ");
  else if(flag==2) printf("netSys(after):  ");
  if(flag)
  {
    printf("%4d %4d %4d %4d -",
      mySysPtr->pPoolStat->mNum,
      mySysPtr->pPoolStat->mDrops,
      mySysPtr->pPoolStat->mWait,
      mySysPtr->pPoolStat->mDrain);
  }

  /* print out data pool mbuf usage */
  for(i=0; i<CL_TBL_SIZE; i++)
  {
    myPoolPtr[i] = (CL_POOL *) mySysPtr->clTbl[i];

    /* only interested in non-zero entries */
    if( (NULL == myPoolPtr[i]) || (NULL == myPoolPtr[i]->clSize) ) break ;
    if(flag) printf(" %4d",myPoolPtr[i]->clNumFree);
	/*
    printf("%d %d %d %d\n" ,
               myPoolPtr[i]->clSize ,
               myPoolPtr[i]->clNum ,
               myPoolPtr[i]->clNumFree,
               myPoolPtr[i]->clUsage
          );
	*/
    if(myPoolPtr[i]->clNumFree < MINNETBUFS)
	{
      printf("\nWARN(%s): too few %3d-byte sys  net buffers (%3d)\n",
        title,myPoolPtr[i]->clSize,myPoolPtr[i]->clNumFree);
	}
  }
  if(flag) printf("\n");
  /*
  printf("=======================================\n");
  printf("=======================================\n");
  */
  return(mySysPtr->pPoolStat->mDrops);
}

int
usrNetStackDataPoolStatus(char *title, int flag)
{
  int i;
  NET_POOL *myDataPtr = ( NET_POOL *) _pNetDpool; /*data mbuf pool (global)*/
  CL_POOL  *myPoolPtr[CL_TBL_SIZE]; /* point at each pool cluster */
  /*
  printf("=======================================\n");
  printf("=======================================\n");
  printf("mbufs   = %d\n",myDataPtr->pPoolStat->mNum);
  printf("failed  = %d\n",myDataPtr->pPoolStat->mDrops);
  printf("waited  = %d\n",myDataPtr->pPoolStat->mWait);
  printf("drained = %d\n",myDataPtr->pPoolStat->mDrain);
  */
  if(flag==1)      printf("netData(befor): ");
  else if(flag==2) printf("netData(after): ");
  if(flag)
  {
    printf("%4d %4d %4d %4d -",
      myDataPtr->pPoolStat->mNum,
      myDataPtr->pPoolStat->mDrops,
      myDataPtr->pPoolStat->mWait,
      myDataPtr->pPoolStat->mDrain);
  }

  /* print out data pool mbuf usage */
  for(i=0; i<CL_TBL_SIZE; i++)
  {
    myPoolPtr[i] = (CL_POOL *) myDataPtr->clTbl[i];

    /* only interested in non-zero entries */
    if( (NULL == myPoolPtr[i]) || (NULL == myPoolPtr[i]->clSize) ) break ;
    if(flag) printf(" %4d",myPoolPtr[i]->clNumFree);
	/*
    printf("%d %d %d %d\n" ,
               myPoolPtr[i]->clSize ,
               myPoolPtr[i]->clNum ,
               myPoolPtr[i]->clNumFree,
               myPoolPtr[i]->clUsage
          );
	*/
    if(myPoolPtr[i]->clNumFree < MINNETBUFS)
	{
      printf("\nWARN(%s): too few %3d-byte data net buffers (%3d)\n",
        title,myPoolPtr[i]->clSize,myPoolPtr[i]->clNumFree);
	}
  }
  if(flag) printf("\n");
  /*
  printf("=======================================\n");
  printf("=======================================\n");
  */
  return;
}

#endif


/* codaExecute: was type-specific, trying to make it generic */

/**************************************************************************/
/******************* RunControl interface function ************************/
/**************************************************************************/

/* following messages must be recognized:
 download
 prestart
 end
 pause
 go
 exit
 status
 */

/* example of tcpClient's command:

     ./tcpClient EB5 'download test_ts2'
*/

int codaDownload(char *confname);
int codaPrestart();
int codaGo();
int codaEnd();
int codaPause();
int codaExit();

/* example

codaDownload("adcecal1")

*/


static int
codaExecute(char *command)
{
  int i, j, len, len1;
  char message[128], confname[128];

  len = strlen(command);

  /* check if command length make sence */
  if(len <= 0 || len >= 128)
  {
    printf("codaExecute: ERROR: len=%d - do nothing\n",len);
    return;
  }

  /* check if command contains ascii codes only */
  for(i=0; i<len; i++)
  {
    if( !isascii(command[i]) )
	{
      printf("codaExecute: ERROR: non-ascii code [%d]=%d - do nothing\n",
        i,command[i]);
      return;
	}
  }

  /* remove leading spaces */
  for(i=0; i<len; i++)
  {
    j = i;
    if(command[j]!=' ') break;
  }

  strcpy(message,(char *)&command[j]);
  len = strlen(message);
  printf("codaExecute reached, message >%s<, len=%d\n",message,len);

  if( !strncmp(message, "download", 8) )
  {
    printf("codaExecute: 'download' transition\n");

    len1 = strlen("download");
    len = strlen(message);
    for(i=len1; i<len; i++)
    {
      j = i;
      if(message[j]!=' ') break; /* remove leading spaces */
    }
    strcpy(confname,(char *)&message[j]);
    len = strlen(confname);
    printf("--> confname >%s<, len=%d\n",confname,len);

    codaDownload(confname);
  }
  else if( !strncmp(message, "prestart", 8) )
  {
    printf("codaExecute: 'prestart' transition\n");
    codaPrestart();
  }
  else if( !strncmp(message, "end", 3) )
  {
    printf("codaExecute: 'end' transition\n");
    codaEnd();
  }
  else if( !strncmp(message, "pause", 5) )
  {
    printf("codaExecute: 'pause' transition\n");
    codaPause();
  }
  else if( !strncmp(message, "go", 2) )
  {
    printf("codaExecute: 'go' transition\n");
    codaGo();
  }
  else if( !strncmp(message, "exit", 4) )
  {
    printf("codaExecute: 'exit' transition\n");
    codaExit();
  }
  else
  {
    printf("codaExecute: ERROR: unknown request >%s<\n",message);
  }
  printf("codaExecute done\n");

  return;
}

