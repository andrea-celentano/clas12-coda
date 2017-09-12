
/* rol2th.cc - default second readout list */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <pthread.h>

#ifdef Linux
#include <sys/prctl.h>
#endif

#ifndef VXWORKS
#include <sys/types.h>
#include <time.h>
#endif

#include "circbuf.h"

#define ROL_NAME__ "ROL2TH"
#define INIT_NAME rol2th__init

#define POLLING_MODE
#define EVENT_MODE

#include "rol.h"
#include "EVENT_source.h"

/************************/
/************************/

#undef DEBUG

#define USE_GEM

#ifdef USE_GEM

#include "../../gem/src.s/zero_suppression.h"
#include "../../gem/src.s/PRDMapping.h"
#include "../../gem/src.s/GEMZeroSup.h"

const char *pedestal_file = "/usr/clas12/release/1.3.0/coda/src/gem/run/pedestal.txt";
const char *mapping_file = "/usr/clas12/release/1.3.0/coda/src/gem/run/mapping.cfg";

/* in following: [FEC#][ADC#][CHANNEL#] */ 
static int buf_offset[8][16][128];
static int buf_noise[8][16][128];
static PRDMapping* mapping;

/* counter for GEM row data */
static unsigned int origin_counter = 0;
#define ORIGIN_COUNT 5000

#endif






#define NTHREADS 8
#define MAXBUF 2600

pthread_t threads[NTHREADS];

typedef struct params params_t;
typedef struct params {
  pthread_mutex_t mutex;
  pthread_cond_t done;
  int id_in;

  pthread_mutex_t mutex1[NTHREADS];
  pthread_cond_t done1[NTHREADS];

  pthread_mutex_t mutex2[NTHREADS];
  pthread_cond_t done2[NTHREADS];

  /* two following will be set before thread starts processing event */
  unsigned int *datain[NTHREADS];
  unsigned int lenin[NTHREADS];

  unsigned int dataout[NTHREADS][MAXBUF];
  unsigned int lenout[NTHREADS];

} TH_PARAMS;

TH_PARAMS params;
int thread_exit[NTHREADS];


void *
data_thread(void* arg)
{
  int ret, ii, my_id, lenin, nw;
  unsigned int *datain;
  unsigned int *dataout;
  unsigned int *lenout;
  char myname[80];

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &ret);
  pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, &ret);

  /* Lock */
  ret = pthread_mutex_lock(&(*(params_t*)(arg)).mutex);

  /* remember id */
  my_id = (*(params_t*)(arg)).id_in;
  sprintf(myname,"rol2th%02d",my_id);
#ifdef Linux
  prctl(PR_SET_NAME,myname);
#endif

#ifdef DEBUG
  printf("[%d] data_thread started, ret=%d\n",ret);fflush(stdout);
#endif  

  /* whose two never changed */
  dataout = (*(params_t*)(arg)).dataout[my_id];
  lenout = &((*(params_t*)(arg)).lenout[my_id]);

  /* Unlock and signal completion */
  ret = pthread_mutex_unlock(&(*(params_t*)(arg)).mutex);
  ret = pthread_cond_signal (&(*(params_t*)(arg)).done);

  /* After signaling `main`, the thread is ready for work */
  while(1)
  {







#ifdef DEBUG
    printf("[%d] trying to lock mutex1[%d] ..\n",my_id,my_id);fflush(stdout);
    printf("[%d] trying to lock mutex1 (0x%08x)\n",my_id,&(*(params_t*)(arg)).mutex1[my_id]);fflush(stdout);
#endif
    ret = pthread_mutex_lock(&(*(params_t*)(arg)).mutex1[my_id]);
#ifdef DEBUG
    printf("[%d] .. got lock mutex1[%d], ret=%d\n",my_id,my_id,ret);fflush(stdout);
#endif






    datain = (*(params_t*)(arg)).datain[my_id];
    lenin = (*(params_t*)(arg)).lenin[my_id];
#ifdef DEBUG
    printf("[%d] obtaining parameters\n",my_id);fflush(stdout);
    printf("[%d] obtained: input data at 0x%08x, lenin=%d, output data to 0x%08x, lenout to 0x%08x\n",
		 my_id,datain,lenin,dataout,lenout);fflush(stdout);
#endif










    /* Unlock and signal completion */
#ifdef DEBUG
    printf("[%d] releasing mutex 1\n",my_id);fflush(stdout);
#endif
    ret = pthread_mutex_unlock(&(*(params_t*)(arg)).mutex1[my_id]);
#ifdef DEBUG
    printf("[%d] mutex 1 released, ret=%d\n",my_id,ret);fflush(stdout);
#endif
    ret = pthread_cond_signal (&(*(params_t*)(arg)).done1[my_id]);
#ifdef DEBUG
    printf("[%d] cond 1 released, ret=%d\n",my_id,ret);fflush(stdout);
#endif



    if(thread_exit[my_id]) {printf("[%d] exiting1 ..\n",my_id);break;} /* exit if flag is set by main program */






    /*******************/
	/* processing data */

	/* clondaq3: 3.1 kHz / 3500 kB/sec */
    nw = zero_suppression((uint32_t *)datain, lenin, (uint32_t *)dataout, buf_offset, buf_noise);
	

	/* clondaq3: 3.0 kHz / 240000 kB/s 
    nw = lenin;
	for(ii=0; ii<nw; ii++) dataout[ii] = datain[ii];
    */

#ifdef DEBUG
    printf("[%d] nw_in_thread=%d\n",my_id,nw);fflush(stdout);
#endif
    *lenout = nw;

	/* processing data */
    /*******************/





    







#ifdef DEBUG
    printf("[%d] trying to lock mutex2[%d] ..\n",my_id,my_id);fflush(stdout);
#endif
    ret = pthread_mutex_lock(&(*(params_t*)(arg)).mutex2[my_id]);
#ifdef DEBUG
    printf("[%d] .. got lock mutex2[%d], ret=%d\n",my_id,my_id,ret);fflush(stdout);
#endif

#ifdef DEBUG
    printf("[%d] releasing mutex 2[%d]\n",my_id,my_id);fflush(stdout);
#endif
    ret = pthread_mutex_unlock(&(*(params_t*)(arg)).mutex2[my_id]);
#ifdef DEBUG
    printf("[%d] mutex 2[%d] released, ret=%d\n",my_id,my_id,ret);fflush(stdout);
#endif
    ret = pthread_cond_signal (&(*(params_t*)(arg)).done2[my_id]);
#ifdef DEBUG
    printf("[%d] cond 2[%d] released, ret=%d\n",my_id,my_id,ret);fflush(stdout);
#endif


    if(thread_exit[my_id]) {printf("[%d] exiting2 ..\n",my_id);break;} /* exit if flag is set by main program */



  }

  printf("%s exited\n",myname);fflush(stdout);
  thread_exit[my_id] = 0;
  pthread_exit(NULL);
}



int mynev; /*defined in tttrans.c */


/* user routines */








void rol2trig(int a, int b);
void rol2trig_done();

static void
__download()
{
  int ii;

  rol->poll = 1;

  for(ii=0; ii<NTHREADS; ii++) threads[ii] = NULL;

  return;
}

static void
__prestart()
{
  int ii, status;
  void *res;

  printf("INFO: Entering Prestart ROL2TH\n");

  pthread_attr_t attr;
  pthread_attr_init(&attr); /* initialize attr with default attributes */
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  /* Clear some global variables etc for a clean start */
  CTRIGINIT;

  /* init trig source EVENT */
  EVENT_INIT;

  /* Register a sync trigger source (up to 32 sources) */
  CTRIGRSS(EVENT, 1, rol2trig, rol2trig_done); /* second arg=1 - what is that ? */

  rol->poll = 1;

  rol->recNb = 0;


#ifdef USE_GEM
  printf("GEM Intitialization ..\n");

  mapping = PRDMapping::GetInstance();
  mapping->LoadMapping(mapping_file);
  mapping->SaveMapping("/tmp/saved_mapping.cfg");

  status = load_pedestal(pedestal_file, buf_noise, buf_offset, 72);

  printf("GEM Intitialization done\n");
#endif




  pthread_mutex_init (&params.mutex, NULL);
  pthread_cond_init (&params.done, NULL);
  pthread_mutex_lock (&params.mutex);

  for(ii=0; ii<NTHREADS; ii++)
  {
    pthread_mutex_init (&params.mutex1[ii], NULL);
    pthread_cond_init (&params.done1[ii], NULL);
    pthread_mutex_lock (&params.mutex1[ii]);

    pthread_mutex_init (&params.mutex2[ii], NULL);
    pthread_cond_init (&params.done2[ii], NULL);
    pthread_mutex_lock (&params.mutex2[ii]);

    thread_exit[ii] = 0;
  }



  for(ii=0; ii<NTHREADS; ii++)
  {
    /* set the parameters in structure being passed to thread */
    params.id_in = ii;

    /* Spawn a thread */
    pthread_create(&threads[ii], /*NULL*/&attr, data_thread, &params);

    /* Give up the lock, wait till thread is 'done', then reacquire the lock.  */
    pthread_cond_wait (&params.done, &params.mutex);
  }



  printf("INFO: Prestart ROL2TH executed\n");

  return;
}

static void
__end()
{
  int ii, ret;
  void *res;
  pthread_t next_thread;

  printf("INFO: User End ROL2TH Started\n");


  for(ii=0; ii<NTHREADS; ii++)
  {
    thread_exit[ii] = 1;
    pthread_cond_wait (&params.done1[ii], &params.mutex1[ii]);
  }

  printf("ROL2TH: sent exit request to threads, waiting ..\n");fflush(stdout);
  sleep(3);
  printf("ROL2TH: .. continue\n");fflush(stdout);

  /*
  for(ii=0; ii<NTHREADS; ii++)
  {
    ret = pthread_cond_wait (&params.done1[ii], &params.mutex1[ii]);
  }
  sleep(1);
  */


  for(ii=0; ii<NTHREADS; ii++)
  {
    if( (threads[ii] != NULL) && (thread_exit[ii] != 0) )
	{
      next_thread = threads[ii];
	  printf("000 0x%08x\n",next_thread);fflush(stdout);
      threads[ii] = NULL;
	  printf("111\n");fflush(stdout);
	  if(pthread_cancel(next_thread)<0) perror("pthread_cancel");
	  printf("222\n");fflush(stdout);
	  if(pthread_join(next_thread, &res)<0) perror("pthread_join");
	  printf("333\n");fflush(stdout);
	  if (res == PTHREAD_CANCELED) printf("%s: thread %d canceled\n",__FUNCTION__,ii);
	  else printf("%s: ERROR: thread %d NOT canceled\n",__FUNCTION__,ii);
	}
    else
	{
      printf("WARN: do nothing to cancel pthread %d, probably exited already: 0x%08x %d\n",ii,threads[ii],thread_exit[ii]);
    }
  }

  /*
  pthread_mutex_destroy (&params.mutex);
  pthread_cond_destroy (&params.done);
  for(ii=0; ii<NTHREADS; ii++)
  {
    pthread_mutex_destroy (&params.mutex1[ii]);
    pthread_cond_destroy (&params.done1[ii]);
    pthread_mutex_destroy (&params.mutex2[ii]);
    pthread_cond_destroy (&params.done2[ii]);
  }
  */


  printf("INFO: User End ROL2TH Executed\n");

  return;
}

static void
__pause()
{
  printf("INFO: User Pause 2 Executed\n");

  return;
}

static void
__go()
{
  printf("User Go 2 Reached\n");fflush(stdout);

  mynev = 0;

  printf("INFO: User Go 2 Executed\n");fflush(stdout);

  return;
}

void
rol2trig(int a, int b)
{
  CPINIT;
  BANKINIT;
  int nASIS, iASIS[MAXBANKS];
  int ret;
  int nthread, nw, ii, jj, kk;
  unsigned int tmp;
  bool raw_data = false;

  mynev ++; /* needed by ttfa.c */


  /*printf("befor bankscan\n");fflush(stdout);*/
  BANKSCAN;
  /*printf("after bankscan\n");fflush(stdout);*/

  /*
  printf("\n\n\n\n\n nbanks=%d\n",nbanks);
  for(jj=0; jj<nbanks; jj++) printf("bankscan[%d]: tag 0x%08x typ=%d nr=%d nw=%d dataptr=0x%08x\n",
									jj,banktag[jj],banktyp[jj],banknr[jj],banknw[jj],bankdata[jj]);fflush(stdout);
  */
  nASIS = 0;

  if(++origin_counter > ORIGIN_COUNT) {
       raw_data = true;
       origin_counter = 0;
   }

  for(jj=0; jj<nbanks; jj++)
  {
    datain = bankdata[jj];
    lenin = banknw[jj];
#ifndef VXWORKS
#ifndef NIOS
    /* swap input buffer (assume that data from VME is big-endian, and we are on little-endian Intel) */
    if(banktyp[jj] != 3) for(ii=0; ii<lenin; ii++) datain[ii] = LSWAP(datain[ii]);
#endif
#endif
    iASIS[nASIS++] = jj; /* remember bank number as it reported by BANKSCAN */
  }

  nthread = 0;
  for(ii=0; ii<nASIS; ii++)
  {
    jj = iASIS[ii]; /* bank number as it reported by BANKSCAN */
    datain = bankdata[jj];
    lenin = banknw[jj];
    /*printf("mynev=%d: coping bank number %d (header %d 0x%08x))\n",mynev,jj,*(datain-2),*(datain-1));*/

	/*PRESCALE IT FOR REAL RUN  !!!*/
    if(banktag[jj]!=57631)
    {
      CPOPEN(banktag[jj],banktyp[jj],banknr[jj]);
      for(kk=0; kk<lenin; kk++) dataout[kk] = datain[kk];
      b08 += (lenin<<2);
      CPCLOSE;
	}
#ifdef USE_GEM
	if(banktag[jj]==57631)
	{
      params.datain[nthread] = datain; /* pointer to the input data to be processed by thread */
      params.lenin[nthread] = lenin; /* length of the input data */

      /* Give up the lock for thread 'nthread', wait till thread is 'done', then reacquire the lock */
#ifdef DEBUG
      printf("===== releasing mutex1[%d]\n",nthread);fflush(stdout);
#endif
      ret = pthread_cond_wait (&params.done1[nthread], &params.mutex1[nthread]);
#ifdef DEBUG
      printf("===== reacquired mutex1[%d], ret=%d\n",nthread,ret);fflush(stdout);
#endif

      nthread++;
      if(nthread > NTHREADS) {printf("rol2th: ERROR: nthread=%d - exit\n",nthread);fflush(stdout);}
      /*printf("datain=0x%08x len=%d nthread=%d\n",datain,lenin,nthread-1);*/


#if 1
      if(raw_data) {
          /* copy original bank */
          /*printf("GEM: bank number=%d nw=%d\n",banknr[jj],lenin);fflush(stdout);*/
          CPOPEN(banktag[jj],banktyp[jj],banknr[jj]);
          for(kk=0; kk<lenin; kk++) dataout[kk] = datain[kk];
          b08 += (lenin<<2);
          CPCLOSE;
      }
#endif


	}

#endif

  }




  if(nthread>0)
  {

    /* open output bank */
    CPOPEN(57631,1,99);
    nw = 0;
    dataout[nw++] = 0xfecfec99; /* signature */



    for(ii=0; ii<nthread; ii++)
	{
      /* Give up the lock 'ii', wait till thread is 'done', then reacquire the lock */
#ifdef DEBUG
      printf("===== releasing mutex2[%d]\n",ii);fflush(stdout);
#endif
      ret = pthread_cond_wait (&params.done2[ii], &params.mutex2[ii]);
#ifdef DEBUG
      printf("===== reacquired mutex2[%d], ret=%d\n",ii,ret);fflush(stdout);
#endif


      /* copy data into output bank */
#ifdef DEBUG
      printf("lenout[%d] = %d\n",ii,params.lenout[ii]);fflush(stdout);
#endif
      for(kk=0; kk<params.lenout[ii]; kk++) dataout[nw++] = params.dataout[ii][kk];
    }



#ifdef DEBUG
    printf(" -> nw=%d, signature=0x%08x\n",nw,dataout[0]);fflush(stdout);
#endif

#ifdef DEBUG_HIDE
    for(ii=1; ii<nw; ii++)
    {
      tmp = dataout[ii];
      printf("-> [%4d] 0x%08x -> det=%d plane=%d fec=%2d adcch=%2d strip=%3d ts=%2d pol=%d val=%4d\n",ii,tmp,
			 (tmp>>31)&0x1, (tmp>>30)&0x1, (tmp>>26)&0xf, (tmp>>22)&0xf,
             (tmp>>15)&0x7f, (tmp>>12)&0xf, (tmp>>11)&0x1, tmp&0x7ff  );fflush(stdout);
    }
#endif



    /* close output bank */
    b08 += (nw<<2);
    CPCLOSE;

  }

#ifdef DEBUG
  printf(" -> lenout=%d\n",lenout);fflush(stdout);
#endif

  rol->user_storage[0] = lenout;
  CPEXIT;
  rol->user_storage[1] = 1;

  return;
}

void
rol2trig_done()
{
  return;
}  


void
__done()
{
  /* from parser */
  poolEmpty = 0; /* global Done, Buffers have been freed */

  return;
}
  
static void
__status()
{
  return;
}  
