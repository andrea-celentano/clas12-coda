//  event_monitor

//  analyzes events, creates hist

//  still to do
//    timed wait not working?

//  ejw, 21-dec-99


// read from file 'bla'
//    ./level3 -i bla
//

#undef DEBUG
#undef DEBUG1

#undef USE_IPC

// for posix
#define _POSIX_SOURCE_ 1
#define __EXTENSIONS__


#ifdef USE_IPC
// for smartsockets
#include <rtworks/cxxipc.hxx>
#endif


// for et
extern "C" {
#include <et.h>
#include <evio.h> 
}


// system stuff
#include <fstream.h>
#include <iomanip.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdarg.h>
//#include <macros.h>
#include <time.h>
#include <ctype.h>
#include <pthread.h>



#ifdef USE_IPC
// for CLAS ipc
#include <clas_ipc_prototypes.h>
#endif


// for evio
static int handler, iev;
#define MAXBUF 1000000
unsigned int localbuf[MAXBUF];
extern "C"{
#include <evio.h>
#define MAXPCALHITS 1000
int get_one_pcal_event_(unsigned int *event, int *nhits, int slotout[MAXPCALHITS], int chanout[MAXPCALHITS],
			int tdcout[MAXPCALHITS], int adcout[MAXPCALHITS], int rawadcout[MAXPCALHITS][100]);
}


// misc variables
/*
Tcl_Interp *interp;  
static char *init_tcl_script  = NULL;
*/
static char *application      = "clastest";
static char *session          = NULL;
static char *uniq_name        = "level3";
static char *unique_id        = (char *) malloc(64);
static int evt_from_file      = 0;
static int rawmode            = 0;
static char *input_file       = NULL;
static int max_event          = -1;
static int wait_time          = 100;    // msec
static int et_ok              = 0;
static int debug              = 0;
static int done               = 0;
static time_t start_time      = time(NULL);
static char temp[256];
static char filename[256];


// et stuff
static char *et_station_name        = "level3ec";
static et_openconfig openconfig;
static et_sys_id et_system_id;
static char et_filename[256];
static et_stat_id et_station_id;
static et_statconfig et_station_config;
static et_att_id et_attach_id;
static et_event *et_event_ptr;


// for hbook
static int no_global          = -1;
static char *global_sect      = "none";
static int reset_hist_bor     = 0;
static int no_archive         = 0;
static int no_snapshot        = 0;
static char *archive_dir      =".";
static time_t last_archive    = time(NULL);
static time_t last_reset      = 0;
static int archive_time       = 600;       // secs
static int archive_evt        = 1000000;   // events


// event counts, rates, etc.
static int last_run           = 0;
static double et_rate         = 0.;
static double proc_rate       = 0.;
static time_t last_time       = time(NULL);
static int last_nevet         = 0;
static int last_nevproc       = 0;
static time_t delta;
static int nevents            = 0;


// fortran common block
extern "C" {
struct {
  long nevet;
  long nevproc;
  long nev_this_run;
  long nrun_proc;

  long current_run;
  long current_event;

  long no_cc;
  long no_dc;
  long no_ec;
  long no_sc;
  long no_st;

  long no_lac;
  long no_trig;
  long no_tg;
  long no_photon;
  long no_call;
  long no_l2;

  long no_scaler;
  long no_hist;
  long no_timeline;

  long event_length;
} event_monitor_ = {0,0,0,0, 0,0, 0,0,0,0,0,  0,0,0,0,0,0,  0,0,0, 0};
}


// prototypes
void decode_command_line(int argc, char **argv);
/*
void init_tcl(void);
*/
void init_et();
void connect_et();
void *control_thread(void *param);
void process_events(void);
void quit_callback(int sig);
#ifdef USE_IPC
void status_poll_callback(T_IPC_MSG msg);
void control_message_callback(T_IPC_CONN conn,
			      T_IPC_CONN_PROCESS_CB_DATA data,
			      T_CB_ARG arg);
#endif
void decode_hbook_id(char *cid, long *id, char *dir);

extern "C" {
  char *strdupf(char *fstring, int len);
#ifdef USE_IPC
  int insert_msg(char *name, char *facility, char *process, char *msgclass, 
	       int severity, char *status, int code, char *message);
#endif
}


// fortran prototypes
extern "C" {
void reset_hist_(char *flag, int len);
void hcdir_(char *dir, char *flag, int l1, int l2);
void hrput_(long *id, char *filename, char *flag, int len1, int len2);
void hbook1_(long*,char*,int*,float*,float*,float*,int);
void hbook2_(long*,char*,int*,float*,float*,int*,float*,float*,float*,int);
void hidall_(long*,int*);
void hgive_(long*,char*,int*,float*,float*,int*,float*,float*,int*,int*,int);
void hrdir_(int*,char*,int*,int);
void cltou_(char *flag, int len);
void dd2bos_(long *evlen, long *pdata, char *rt, int len);
}


// level3 prototypes
extern "C"{
  void level3_init_(void);
  void level3_packev_(int *ctl, int *data, int *len);
  void level3_analyze_(void);
  void level3_stop_(void);
  void level3_done_(void);
  long *find_bank_in_dd_(long *fevent, char *fbank, int *fnbank, int *fnwrds, int fblen);
  char *strdupf(char *fortran_string, int len);
}


#ifdef USE_IPC
// ref to IPC server (connection created later)
TipcSrv &server=TipcSrv::Instance();
#endif




/*-----------------------------------------------------------------------*/


/* 
*  duplicates fortran string, removes trailing blanks and inserts ending NULL 
*/

char *
strdupf(char *fortran_string, int len) {

  char *s;
  int len_occ;

  /* find occupied length, not including trailing blanks */
  for(len_occ=len;(len_occ>0)&&(fortran_string[len_occ-1]==' ');len_occ--) ;

  s=(char *)malloc(len_occ+1);
  strncpy(s,fortran_string,(size_t)len_occ);
  s[len_occ]='\0';
  
  return(s);

}



//--------------------------------------------------------------------------
#define SWAP32(x) ( (((x) >> 24) & 0x000000FF) | \
                    (((x) >> 8)  & 0x0000FF00) | \
                    (((x) << 8)  & 0x00FF0000) | \
                    (((x) << 24) & 0xFF000000) )

#define PRINT_BUFFER \
  b08 = start; \
  while(b08<end) \
  { \
    GET32(tmp); \
    printf("== 0x%08x\n",tmp); \
  } \
  b08 = start


#define GET8(ret_val) \
  ret_val = *b08++

#define GET16(ret_val) \
  b16 = (unsigned short *)b08; \
  ret_val = *b16; \
  b08+=2

#define GET32(ret_val) \
  b32 = (unsigned int *)b08; \
  ret_val = *b32; \
  b08+=4

#define GET64(ret_val) \
  b64 = (unsigned long long *)b08; \
  ret_val = *b64; \
  b08+=8

int
evNlink(unsigned int *buf, int frag, int tag, int num, int *nbytes)
{
  int ii, len, nw, tag1, pad1, typ1, num1, len2, pad3, ind;
  int right_frag = 0;


#ifdef DEBUG
  printf("\n\n Reach evNlink\n");
  printf("0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
		 buf[0],buf[1],buf[2],buf[3],buf[4],buf[5]);
  printf("%d %d %d %d %d %d\n",
		 buf[0],buf[1],buf[2],buf[3],buf[4],buf[5]);
#endif

  len = buf[0]+1;
  ii = 2;
  while(ii<len)
  {
    nw = buf[ii] + 1;
    tag1 = (buf[ii+1]>>16)&0xffff;
    pad1 = (buf[ii+1]>>14)&0x3;
    typ1 = (buf[ii+1]>>8)&0x3f;
    num1 =  buf[ii+1]&0xff;
#ifdef DEBUG
    printf("[%5d] nw=%d, tag1=0x%04x, pad1=0x%02x, typ1=0x%02x, num1=0x%02x\n",ii,nw,tag1,pad1,typ1,num1);
#endif
    /*check if it is right fragment*/
    if(typ1==0xe || typ1==0x10)
    {
      if(tag1==frag)
      {
#ifdef DEBUG
        printf("right frag\n");
#endif
        right_frag = 1;
      }
      else
      {
#ifdef DEBUG
        printf("wrong frag\n");
#endif
        right_frag = 0;
      }
    }

#ifdef DEBUG
    printf("search ==> %d=1?  %d=%d?  %d=%d?\n",right_frag,tag1,tag,num1,num);
#endif
    if(typ1!=0xe && typ1!=0x10) /*assumes there are no bank-of-banks inside fragment, will redo later*/
    {
      if( right_frag==1 && tag1==tag && num1==num )
      {
        if(typ1!=0xf)
        {
#ifdef DEBUG
          printf(">>> return primitive bank data index %d\n\n\n",ii+2);
#endif
          *nbytes = (nw-2)<<2;
          return(ii+2);
        }
        else
        {
          len2 = (buf[ii+2]&0xffff) + 1; /* tagsegment length (tagsegment contains format description) */
          ind = ii + len2+2; /* internal bank */
          pad3 = (buf[ind+1]>>14)&0x3; /* padding from internal bank */
#ifdef DEBUG
	  printf(">>> found composite bank: tag=%d, type=%d, exclusive len=%d (padding from internal bank=%d)\n",((buf[ii+2]>>20)&0xfff),((buf[ii+2]>>16)&0xf),len2-1,pad3);
          printf("return composite bank data index %d\n",ii+2+len2+2);
#endif
          *nbytes = ((nw-(2+len2+2))<<2)-pad3;
#ifdef DEBUG
	  printf(">>> nbytes=%d\n\n\n",*nbytes);
#endif
          return(ii+2+len2+2);
        }
      }
    }

    if(typ1==0xe || typ1==0x10) ii += 2; /* bank of banks */
    else ii += nw;
  }
#ifdef DEBUG
  printf(">>> Exit evNlink\n\n\n");
#endif

  return(0);
}


//--------------------------------------------------------------------------


int main(int argc, char **argv) {

  int status;
  pthread_t t1;
  int r1=0;


  // synch with c i/o
  ios::sync_with_stdio();


  // decode command line
  decode_command_line(argc,argv);


  // get session name
  if(session==NULL) session="clasprod";

  
  // create unique_id from uniq_name and session name
  strcpy(unique_id,uniq_name);
  strcat(unique_id,"_");
  strcat(unique_id,getenv("HOST")+4);
  strcat(unique_id,"_");
  strcat(unique_id,session);
  
  /*
  // get Tcl interp, create tcl commands, link vars, process Tcl startup script, etc.
  interp=tclinterp_init();
  init_tcl();
  if(init_tcl_script!=NULL)Tcl_EvalFile(interp,init_tcl_script);
  */
  
  // init ipc thread package
  //thr_setconcurrency(thr_getconcurrency()+2);
  //if(!TipcInitThreads()) {
  //  cerr << "Unable to init IPC thread package" << endl;
  //  exit(EXIT_FAILURE);
  //}

#ifdef USE_IPC
  // set ipc parameters, connect to server, etc.
  ipc_set_application(application);
  ipc_set_user_status_poll_callback(status_poll_callback);
  ipc_set_quit_callback(quit_callback);
  status=ipc_init(unique_id,"level3");
  if(status<0) {
    cerr << "\n?Unable to connect to server..."
	 << "probably duplicate unique id\n"
	 << "   ...check for another level3 connected to "
	 << session << " using ipc_info\n"
	 << "   ...only one such process allowed!" << endl << endl;
    exit(EXIT_FAILURE);
  }
  server.SubjectSubscribe("evt_system",TRUE);
  server.Flush();

  // launch ipc thread
  if(pthread_create(&t1,NULL,control_thread,(void *)&r1)!=0) {
    cerr << "\nUnable to create ipc thread" << endl << endl;
    exit(EXIT_FAILURE);
  }
#endif


  // init et structures, etc;
  init_et();

  printf("22\n");fflush(stdout);

  
  // initialize hbook, packages
  level3_init_();


  // init ipc control callback, or open file
  if(evt_from_file==0) {
#ifdef USE_IPC
    ipc_set_control_message_callback(control_message_callback);
    server.Flush();
#endif
    ;
  } else {
    status = evOpen(input_file,"r",&handler);
    if(status < 0)
    {
      printf("evOpen error %d - exit\n",status);
      exit(0);
    }
    else
    {
      printf("Opened evio file >%s<, handler=%d\n",input_file,handler);
    }
  }


#ifdef USE_IPC
  // post startup message
  sprintf(temp,"Process startup:    %15s  in application:  %s",unique_id,application);
  status=insert_msg("level3",unique_id,unique_id,"status",0,"START",0,temp);
#endif

  // flush output
  fflush(NULL);


  // process events
  process_events();


  // last eor
  if(event_monitor_.nev_this_run>0) {
    // nothing
    cout << "End of run       " << event_monitor_.current_run << endl;
  }


  // final package cleanup
  level3_done_();


  // close data source
  if(evt_from_file==0) {
    et_close(et_system_id);
  } else {
    evClose(handler);
  }
  

  // last archive hist
  if(no_archive==0) {
    hcdir_("//PAWC"," ",6,1);
    sprintf(filename,"%s/%s_%s_%06d.hbook",archive_dir,session,uniq_name,
	    event_monitor_.current_run);
    long id=0;
    hrput_(&id, filename, "T", strlen(filename), 1);
  }


  // print stats
  if(evt_from_file!=0) {
    cout << "\n\n   level3 read events from file:  " << input_file << endl << endl;
  } else {
    cout << "\n\n   level3 received " << event_monitor_.nevet 
	 << " events from et" << endl << endl;
  }
  cout << "   Events processed:   " << event_monitor_.nevproc << endl
       << "   Runs processed:     " << event_monitor_.nrun_proc << endl << endl;
  

#ifdef USE_IPC    
  // post shutdown message
  sprintf(temp,"Process shutdown:  %15s",unique_id);
  status=insert_msg("level3",unique_id,unique_id,"status",0,"STOP",0,temp);

  // close ipc and exit
  ipc_close();
#endif


  exit(EXIT_SUCCESS);
}


//--------------------------------------------------------------------------


void *control_thread(void *param) {

  while(done==0) {

#ifdef USE_IPC
    server.MainLoop(2.0);
#endif

    // calc event rates every 10 seconds
    delta=time(NULL)-last_time;
    if(delta>10) {
      et_rate =(double)(event_monitor_.nevet-last_nevet)/delta;
      proc_rate=(double)(event_monitor_.nevproc-last_nevproc)/delta;
      last_time=time(NULL);
      last_nevet=event_monitor_.nevet;
      last_nevproc=event_monitor_.nevproc;
    }      
  }


  return (void *)0;
}


//------------------------------------------------------------------------


void init_et() {
  

  et_ok=0;

 

  // create et file name, etc.
  sprintf(et_filename,"/tmp/et_sys_%s",session);
  printf("init_et: opening >%s<\n",et_filename);
  et_open_config_init(&openconfig);


  /*
  // get et library name
  char *et_user_library     = (char *) malloc(256);
  strcpy(et_user_library,getenv("CODA_LIB"));
  strcat(et_user_library,"/libet_user.so");
  */

  // create station config in case no station exists

  et_station_config_init(&et_station_config);

  et_station_config_setblock(et_station_config,ET_STATION_BLOCKING);
  et_station_config_setselect(et_station_config,ET_STATION_SELECT_ALL);
  et_station_config_setuser(et_station_config,ET_STATION_USER_SINGLE);
  et_station_config_setrestore(et_station_config,ET_STATION_RESTORE_OUT);
  et_station_config_setcue(et_station_config,100);
  et_station_config_setprescale(et_station_config,1);

  /*
  et_station_config_setlib(et_station_config,et_user_library);
  */

  et_station_config_setfunction(et_station_config,"et_mon_function");


  return;
}


//--------------------------------------------------------------------------


void connect_et() {
  
  int status;
  sigset_t sigblock;
  

  et_ok=0;

  
  // open et system
  if(et_open(&et_system_id,et_filename,openconfig)!=ET_OK)
  {
     cerr << "Unable to open ete system " << et_filename << endl;
     return;
  }

  // create station if not already created
  status=et_station_create(et_system_id,&et_station_id,et_station_name,et_station_config);
  if((status!=ET_OK)&&(status!=ET_ERROR_EXISTS))
  { 
      cout << status << endl;
      et_close(et_system_id);
      cerr << "Unable to create station " << et_station_name << endl;
      done=1;
      return;
  }
  
  
  // block signals to THIS thread and any thread created by this thread
  // needed to keep signals from et threads
  sigfillset(&sigblock);
  pthread_sigmask(SIG_BLOCK,&sigblock,NULL);
  
  
  // attach to station
  status=et_station_attach(et_system_id,et_station_id,&et_attach_id);
  if(status!=ET_OK)
  {
      et_close(et_system_id);
      cerr << "Unable to attach to station " << et_station_name << endl;
      done=1;
      return;
  }
  

  // unblock signals
  pthread_sigmask(SIG_UNBLOCK,&sigblock,NULL);


  // success
  et_ok=1; 
  cout << "...now connected to ET system: " << et_filename 
       << ",   station: " << et_station_name << endl;

  return;
}


//--------------------------------------------------------------------------


void process_events() {

  long *evt;
  int handle1,status,stat,len,buflen;
  unsigned int *buffer;
  timespec wait_spec;


  while(done==0) {

    // get an event from et or file
    if(evt_from_file==0) {
      
      // check et system
      if(et_ok==0) {
	connect_et();
	if(et_ok==0) {
	  sleep(1);
	  continue;
	}
      } else if(et_alive(et_system_id)==0) {
	done=1;
	return;
      }	

      
      // et system ok...try to get event...sleep if none available
      wait_spec.tv_sec =(time_t)(wait_time/1000);
      wait_spec.tv_nsec=(long)((wait_time%1000)*1000000);
      //printf("1\n");fflush(stdout);
      status=et_event_get(et_system_id,et_attach_id,&et_event_ptr,ET_ASYNC,NULL);

      //printf("2\n");fflush(stdout);
      if (status==ET_OK)
      {
        nevents++;
        if(!(nevents%1000)) printf("Event %d\n",nevents);

	event_monitor_.nevet++;

        len = et_event_ptr->length>>2;


        stat = evOpenBuffer((char *)et_event_ptr->pdata, MAXBUF, "r", &handle1);
        if(stat!=0) {printf("evOpenBuffer returns %d\n",stat);return;}


        stat = evRead(handle1, localbuf, MAXBUF);
        /*stat = evReadNoCopy(handle1, (const uint32_t**)&buffer, &buflen);*/
        if(stat!=0) {printf("evReadNoCopy returns %d\n",stat);return;}

 
	/*
	et_event_getdata(et_event_ptr,(void**)&evt);
	et_event_getlength(et_event_ptr,(unsigned int *)&len);
	*/


 	event_monitor_.event_length=len/4;

	//	analyze_event();

	event_monitor_.nevproc++;
	event_monitor_.nev_this_run++;
	//	event_monitor_.current_run=bcs_.iw[head+1];
	//	event_monitor_.current_event=bcs_.iw[head+2];
	int llen=len/4;



	/*level3_packev_((int*)0,(int*)evt,&llen);*/
        level3_packev_((int*)0,(int*)localbuf,&llen);


	level3_analyze_();
	
	// check if processed enough events
	if((max_event>0)&&(event_monitor_.nevproc>=max_event))done=1;



        stat = evClose(handle1);
        if(stat!=0) {printf("evClose returns %d\n",stat);return;}

	et_event_put(et_system_id,et_attach_id,et_event_ptr);


      } else if((status==ET_ERROR_TIMEOUT)||(status==ET_ERROR_EMPTY)||(status==ET_ERROR_BUSY)) {
	/*	nanosleep(&wait_spec,NULL); */
	continue;

      } else {
	cerr << "?error return from et_event_get: " << status << endl;
	done=1;
	return;
      }


    } else {

      status = evRead(handler, localbuf, MAXBUF);
      if(status < 0)
      {
	if(status==EOF) printf("end of file after %d events - exit\n",iev);
	else printf("evRead error=%d after %d events - exit\n",status,iev);
        done = 1;
        break;
      }
      iev++;
      event_monitor_.event_length = 0.;


      int llen;
      event_monitor_.nevet++;
      event_monitor_.nevproc++;
      event_monitor_.nev_this_run++;
      level3_packev_((int*)0,(int*)localbuf,&llen);
      level3_analyze_();
      if((max_event>0)&&(event_monitor_.nevproc>=max_event)) done=1;


    }

  }


  return;
}


//-------------------------------------------------------------------


void quit_callback(int sig) {
  
  // received signal or quit control command
  cout << "...stopping, received signal: " << sig << endl;
  done=1;
  
  return;
}


//----------------------------------------------------------------------

#ifdef USE_IPC
void status_poll_callback(T_IPC_MSG msg) {
  
  char p[27];

  
  TipcMsgAppendStr(msg,"Session");
  TipcMsgAppendStr(msg,session);
  TipcMsgAppendStr(msg,"application");
  TipcMsgAppendStr(msg,application);
  TipcMsgAppendStr(msg,"unique_id");
  TipcMsgAppendStr(msg,unique_id);

  TipcMsgAppendStr(msg,"evt_from_file");
  TipcMsgAppendInt4(msg,evt_from_file);
  if(evt_from_file==1) {
    TipcMsgAppendStr(msg,"input_file");
    TipcMsgAppendStr(msg,input_file);
  }

  TipcMsgAppendStr(msg,"");
  TipcMsgAppendStr(msg,"");

  TipcMsgAppendStr(msg,"start_time");
  strcpy(p,ctime(&start_time)); *strchr(p,'\n')='\0';
  TipcMsgAppendStr(msg,p);
  TipcMsgAppendStr(msg,"current_run");
  TipcMsgAppendInt4(msg,event_monitor_.current_run);
  TipcMsgAppendStr(msg,"current_event");
  TipcMsgAppendInt4(msg,event_monitor_.current_event);
  TipcMsgAppendStr(msg,"last_run");
  TipcMsgAppendInt4(msg,last_run);
  TipcMsgAppendStr(msg,"last_archive");
  strcpy(p,ctime(&last_archive)); *strchr(p,'\n')='\0';
  TipcMsgAppendStr(msg,p);
  TipcMsgAppendStr(msg,"last_reset");
  strcpy(p,ctime(&last_reset)); *strchr(p,'\n')='\0';
  TipcMsgAppendStr(msg,p);

  TipcMsgAppendStr(msg,"");
  TipcMsgAppendStr(msg,"");

  TipcMsgAppendStr(msg,"nevet");
  TipcMsgAppendInt4(msg,event_monitor_.nevet);
  TipcMsgAppendStr(msg,"nevproc");
  TipcMsgAppendInt4(msg,event_monitor_.nevproc);
  TipcMsgAppendStr(msg,"nev_this_run");
  TipcMsgAppendInt4(msg,event_monitor_.nev_this_run);
  TipcMsgAppendStr(msg,"nrun_proc");
  TipcMsgAppendInt4(msg,event_monitor_.nrun_proc);

  TipcMsgAppendStr(msg,"");
  TipcMsgAppendStr(msg,"");

  /*
  if(init_tcl_script!=NULL) {
    TipcMsgAppendStr(msg,"init_tcl_script");
    TipcMsgAppendStr(msg,init_tcl_script);
  }
  */

  TipcMsgAppendStr(msg,"no_global");
  TipcMsgAppendInt4(msg,no_global);
  TipcMsgAppendStr(msg,"global_sect");
  TipcMsgAppendStr(msg,global_sect);
  TipcMsgAppendStr(msg,"archive_time");
  TipcMsgAppendInt4(msg,archive_time);
  TipcMsgAppendStr(msg,"archive_evt");
  TipcMsgAppendInt4(msg,archive_evt);
  TipcMsgAppendStr(msg,"max_event");
  TipcMsgAppendInt4(msg,max_event);
  TipcMsgAppendStr(msg,"wait_time");
  TipcMsgAppendInt4(msg,wait_time);
  TipcMsgAppendStr(msg,"reset_hist_bor");
  TipcMsgAppendInt4(msg,reset_hist_bor);
  TipcMsgAppendStr(msg,"no_archive");
  TipcMsgAppendInt4(msg,no_archive);
  TipcMsgAppendStr(msg,"archive_dir");
  TipcMsgAppendStr(msg,archive_dir);
  TipcMsgAppendStr(msg,"no_snapshot");
  TipcMsgAppendInt4(msg,no_snapshot);
  TipcMsgAppendStr(msg,p);
  TipcMsgAppendStr(msg,"debug");
  TipcMsgAppendInt4(msg,debug);

  TipcMsgAppendStr(msg,"");
  TipcMsgAppendStr(msg,"");

  TipcMsgAppendStr(msg,"no_cc");
  TipcMsgAppendInt4(msg,event_monitor_.no_cc);
  TipcMsgAppendStr(msg,"no_ec");
  TipcMsgAppendInt4(msg,event_monitor_.no_ec);
  TipcMsgAppendStr(msg,"no_sc");
  TipcMsgAppendInt4(msg,event_monitor_.no_sc);
  TipcMsgAppendStr(msg,"no_dc");
  TipcMsgAppendInt4(msg,event_monitor_.no_dc);
  TipcMsgAppendStr(msg,"no_lac");
  TipcMsgAppendInt4(msg,event_monitor_.no_lac);
  TipcMsgAppendStr(msg,"no_st");
  TipcMsgAppendInt4(msg,event_monitor_.no_st);
  TipcMsgAppendStr(msg,"no_tg");
  TipcMsgAppendInt4(msg,event_monitor_.no_tg);
  TipcMsgAppendStr(msg,"no_trig");
  TipcMsgAppendInt4(msg,event_monitor_.no_trig);
  TipcMsgAppendStr(msg,"no_photon");
  TipcMsgAppendInt4(msg,event_monitor_.no_photon);
  TipcMsgAppendStr(msg,"no_call");
  TipcMsgAppendInt4(msg,event_monitor_.no_call);
  TipcMsgAppendStr(msg,"no_l2");
  TipcMsgAppendInt4(msg,event_monitor_.no_l2);
  TipcMsgAppendStr(msg,"no_scaler");
  TipcMsgAppendInt4(msg,event_monitor_.no_scaler);
  TipcMsgAppendStr(msg,"no_timeline");
  TipcMsgAppendInt4(msg,event_monitor_.no_timeline);
  TipcMsgAppendStr(msg,"no_hist");
  TipcMsgAppendInt4(msg,event_monitor_.no_hist);

  return;
}


//--------------------------------------------------------------------------


void control_message_callback(T_IPC_CONN conn,
			      T_IPC_CONN_PROCESS_CB_DATA data,
			      T_CB_ARG arg) {

  T_STR string;


  // get first string
  TipcMsgSetCurrent(data->msg,0);
  TipcMsgNextStr(data->msg,&string);
  
  
  //  evt status request
  if(strcasecmp(string,"evt_status_poll")==0) {

    T_STR srvnode = server.Node();

    TipcMsg status("evt_status");
    status.Dest("/evt_system/status");
    status.Sender(unique_id);
    sprintf(temp,"et:%s",et_station_name);
    status << unique_id << getenv("HOST") << session << srvnode 
	   << (T_INT4) event_monitor_.nevet << et_rate 
	   << (T_INT4) event_monitor_.nevproc << proc_rate
	   << (T_INT4) et_ok << temp;
    server.Send(status,TRUE);
    server.Flush();


  //  don't understand message...ship to smartsockets interpreter
  } else {
    TutCommandParseStr(string);
  }

  return;
}
#endif

//----------------------------------------------------------------------


void decode_command_line(int argc, char**argv) {
  
  char *help = 
    "\nusage:\n\n  level3 [-a application] [-s session] [-u uniq_name] [-g global_sect] [-i input_file]\n"
    "          [-stat et_station_name] [-adir archive_dir] [-at archive_time] [-ae archive_evt\n"
    "          [-m max_event] [-r(eset_hist_bor)] [-no_archive] [-no_snapshot] [-wait wait_time] [-debug]\n"
    "          [-no_cc] [-no_dc] [-no_ec] [-no_sc] [-no_st] [-no_lac]\n"
    "          [-no_trig] [-no_tg] [-no_photon] [-no_call] [-no_l2] [-no_scaler] [-no_timeline] [-no_hist]\n";
  
  
  
  // loop over all arguments, except the 1st (which is program name)
  int i=1;
  while (i<argc) 
    {
      if (strncasecmp(argv[i],"-h",2)==0)
	{
	  cout << help << endl;
	  exit(EXIT_SUCCESS);
	}
      else if (strncasecmp(argv[i],"-debug",6)==0) {
	debug=1;
	i=i+1;
      }
      else if (strncasecmp(argv[i],"-no_archive",11)==0) {
	no_archive=1;
	i=i+1;
      }
      else if (strncasecmp(argv[i],"-no_snapshot",12)==0) {
	no_snapshot=1;
	i=i+1;
      }
      else if (strncasecmp(argv[i],"-adir",5)==0) {
	archive_dir=strdup(argv[i+1]);
	i=i+2;
      }
      else if (strncasecmp(argv[i],"-stat",5)==0) {
	et_station_name=strdup(argv[i+1]);
	i=i+2;
      }
      else if (strncasecmp(argv[i],"-no_cc",6)==0) {
	event_monitor_.no_cc=-1;
	i=i+1;
      }
      else if (strncasecmp(argv[i],"-no_dc",6)==0) {
	event_monitor_.no_dc=-1;
	i=i+1;
      }
      else if (strncasecmp(argv[i],"-no_ec",6)==0) {
	event_monitor_.no_ec=-1;
	i=i+1;
      }
      else if (strncasecmp(argv[i],"-no_sc",6)==0) {
	event_monitor_.no_sc=-1;
	i=i+1;
      }
      else if (strncasecmp(argv[i],"-no_st",6)==0) {
	event_monitor_.no_st=-1;
	i=i+1;
      }
      else if (strncasecmp(argv[i],"-no_lac",7)==0) {
	event_monitor_.no_lac=-1;
	i=i+1;
      }
      else if (strncasecmp(argv[i],"-no_trig",8)==0) {
	event_monitor_.no_trig=-1;
	i=i+1;
      }
      else if (strncasecmp(argv[i],"-no_tg",6)==0) {
	event_monitor_.no_tg=-1;
	i=i+1;
      }
      else if (strncasecmp(argv[i],"-no_photon",10)==0) {
	event_monitor_.no_photon=-1;
	i=i+1;
      }
      else if (strncasecmp(argv[i],"-no_call",8)==0) {
	event_monitor_.no_call=-1;
	i=i+1;
      }
      else if (strncasecmp(argv[i],"-no_l2",6)==0) {
	event_monitor_.no_l2=-1;
	i=i+1;
      }
      else if (strncasecmp(argv[i],"-no_scaler",10)==0) {
	event_monitor_.no_scaler=-1;
	i=i+1;
      }
      else if (strncasecmp(argv[i],"-no_timeline",12)==0) {
	event_monitor_.no_timeline=-1;
	i=i+1;
      }
      else if (strncasecmp(argv[i],"-no_hist",7)==0) {
	event_monitor_.no_hist=-1;
	i=i+1;
      }
      else if (strncasecmp(argv[i],"-wait",5)==0) {
	wait_time=atoi(argv[i+1]);
	i=i+2;
      }
      else if (strncasecmp(argv[i],"-at",3)==0) {
	archive_time=atoi(argv[i+1]);
	i=i+2;
      }
      else if (strncasecmp(argv[i],"-ae",3)==0) {
	archive_evt=atoi(argv[i+1]);
	i=i+2;
      }
      else if (strncasecmp(argv[i],"-a",2)==0) {
	application=strdup(argv[i+1]);
	i=i+2;
      }
      else if (strncasecmp(argv[i],"-s",2)==0) {
	session=strdup(argv[i+1]);
	i=i+2;
      }
      else if (strncasecmp(argv[i],"-u",2)==0) {
	uniq_name=strdup(argv[i+1]);
	i=i+2;
      }
      /*
      else if (strncasecmp(argv[i],"-t",2)==0) {
	init_tcl_script=strdup(argv[i+1]);
	i=i+2;
      }
      */
      else if (strncasecmp(argv[i],"-m",2)==0) {
	max_event=atoi(argv[i+1]);
	i=i+2;
      }
      else if (strncasecmp(argv[i],"-i",2)==0) {
	evt_from_file=1;
	input_file=strdup(argv[i+1]);
	i=i+2;
      }
      else if (strncasecmp(argv[i],"-rawmode",8)==0) {
	rawmode = 1;
	i=i+1;
      }
      else if (strncasecmp(argv[i],"-g",2)==0) {
	no_global=0;
	global_sect=strdup(argv[i+1]);
	i=i+2;
      }
      else if (strncasecmp(argv[i],"-r",2)==0) {
	reset_hist_bor=1;
	i=i+1;
      }
      else if (strncasecmp(argv[i],"-",1)==0) {
	cout << "Unknown command line arg: " << argv[i] 
	     << argv[i+1] << endl << endl;
	i=i+2;
      }
    }
  
  return;
}


//---------------------------------------------------------------------


void decode_hbook_id(char *cid, long *id, char *dir) {


  int last_slash=0;
  int id_exists=0;
  char *idp;
  char *dp;


  /* find last slash in cid, set id pointer, and decode id...0 if no id */
  *id=0;
  for(last_slash=strlen(cid); (last_slash>=0)&&((cid+last_slash)[0]!='/'); last_slash--);
  idp=cid+last_slash+1;
  if( ((strlen(idp)>0)&&(isdigit(*idp)!=0)) || 
      ((strlen(idp)>1)&&(strncmp(idp,"-",1)==0)&&(isdigit(*(idp+1))!=0)) ) {
    id_exists=1;
    sscanf(idp,"%d",id);
  }
  

  /* returned dir always start with //pawc */
  strcat(dir,"//PAWC");


  /* null, //, //pawc, //pawc/, id */
  if(
     (strlen(cid)<=0)         	   	    ||
     (strcmp(cid,"//")==0)    	   	    ||  
     (strcasecmp(cid,"//pawc")==0) 	    ||    
     (strcasecmp(cid,"//pawc/")==0)         || 
     ((last_slash<0)&&(id_exists!=0)) )
    return;


  /* more than just //pawc, add slash */
  strcat(dir,"/");


  /* get pointer to after //pawc stuff and reset last_slash */
  dp=cid;
  if(strncasecmp(dp,"//PAWC",6)==0){
    dp=cid+6;
    last_slash-=6;
  }

  /* move dir pointer if leading slash */
  if(strncmp(dp,"/",1)==0)dp++;


  /* append remainder excluding id */
  if(id_exists==0){
    strcat(dir,dp);
  } else {
    strncat(dir,dp,last_slash);
  }


  /* chop off trailing / if it exists */
  if(dir[strlen(dir)-1]=='/')dir[strlen(dir)-1]='\0';


  /* convert dir to upper case */
  cltou_(dir,strlen(dir));


  return;

}


#undef DEBUG

int
get_one_pcal_event_(unsigned int *event, int *nhits, int slotout[MAXPCALHITS], int chanout[MAXPCALHITS],
                    int tdcout[MAXPCALHITS], int adcout[MAXPCALHITS], int rawadcout[MAXPCALHITS][100])
{
  unsigned long long *b64;
  unsigned int *b32;
  unsigned short *b16;
  unsigned char *b08;
  int jj, kk, nh, ind1, nbytes;

  nh = 0;

#ifdef DEBUG
  printf("header: 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
	 event[0],event[1],event[2],event[3],event[4],event[5],event[6],event[7],event[8],event[9]);
#endif

  /* ..,6,0xe103,0,.. for PCAL, ..,8,0xe103,8,.. for ECAL */
  if((ind1 = evNlink(event, 8, 0xe103, 8, &nbytes)) > 0)
  {
      unsigned short pulse_time;
      unsigned int pulse_integral;
      unsigned char *end;
      unsigned long long time;
      int crate,slot,trig,nchan,chan,npulses,notvalid,edge,data,count,ncol1,nrow1;
      int oldslot = 100;
      int ndata0[22], data0[21][8];


      b08 = (unsigned char *) &event[ind1];
      b16 = (unsigned short *) &event[ind1];
      b32 = (unsigned int *) &event[ind1];

      end = b08 + nbytes;
#ifdef DEBUG
      printf("ind1=%d, nbytes=%d (from 0x%08x to 0x%08x)\n",ind1,nbytes,b32,end);
      printf("event: 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
	 event[ind1],event[ind1+1],event[ind1+2],event[ind1+3],event[ind1+4],event[ind1+5],event[ind1+6],event[ind1+7],event[ind1+8],event[ind1+9]);
#endif
      while(b08<end)
      {
#ifdef DEBUG
        printf("begin while: b08=0x%08x\n",b08);
#endif
        b08 = (unsigned char *)b32;
        slot = *b08 ++;

        b32 = (unsigned int *)b08;
        trig = *b32++;

        b64 = (unsigned long long *)b32;
        time = *b64++;

        b32 = (unsigned int *)b64;
        nchan = *b32++;
#ifdef DEBUG
        printf("slot=%d, trig=%d, time=%lld nchan=%d\n",slot,trig,time,nchan);
#endif
        for(jj=0; jj<nchan; jj++)
	{
          b08 = (unsigned char *)b32;
          chan = (*b08 ++) /*+ 1*/;
      
          b32 = (unsigned int *)b08;
          npulses = *b32++;
#ifdef DEBUG
          printf("  chan=%d, npulses=%d\n",chan,npulses);
#endif
          for(kk=0; kk<npulses; kk++)
	  {
            b16 = (unsigned short *)b32;
	    //	    pulse_time = (*b16++)>>6;
            pulse_time = *b16++;
            b32 = (unsigned int *)b16;
            pulse_integral = *b32++;
#ifdef DEBUG
            printf(" b32=0x%08x:  pulse_time=%d pulse_integral=%d\n",b32,pulse_time,pulse_integral);
#endif			
            slotout[nh] = slot;
            chanout[nh] = chan+1; /*channels are numbered from 0 to 15, FORTRAN like from 1 to 16*/
            tdcout[nh] = pulse_time;
            adcout[nh] = pulse_integral;
	    nh ++;

            if(nh >= MAXPCALHITS)
	    {
              printf("TOO MANY HITS, RETURN %d\n",nh);
              *nhits = nh;
              return(nh);
	    }

	  }
	}
        b08 = (unsigned char *)b32;
#ifdef DEBUG
        printf("end loop: b08=0x%08x\n",b08);
#endif
      }
  }
  /* ..,6,0xe103,0,.. for PCAL, ..,8,0xe103,8,.. for ECAL */
  else if((ind1 = evNlink(event, 8, 0xe101, 8, &nbytes)) > 0)
  {
      unsigned char *end;
      unsigned short adc_sample;
      unsigned long long time;
      int crate,slot,trig,nchan,chan,nsamples,notvalid,edge,data,count,ncol1,nrow1;
      int oldslot = 100;
      int ndata0[22], data0[21][8];
      int baseline, sum, channel;
#ifdef DEBUG1
      printf("Found bank tag 0xe101, ind1=%d, nbytes=%d\n",ind1,nbytes);
      printf("ind1=%d, nbytes=%d\n",ind1,nbytes);
#endif
      b08 = (unsigned char *) &event[ind1];
      end = b08 + nbytes;
#ifdef DEBUG1
      printf("ind1=%d, nbytes=%d (from 0x%08x to 0x%08x)\n",ind1,nbytes,b08,end);
#endif
      while(b08<end)
      {
#ifdef DEBUG1
        printf("begin while: b08=0x%08x\n",b08);
#endif
        GET8(slot);
        GET32(trig);
        GET64(time);
        GET32(nchan);
#ifdef DEBUG1
        printf("slot=%d, trig=%d, time=%lld nchan=%d\n",slot,trig,time,nchan);
#endif
        for(jj=0; jj<nchan; jj++)
	{
          GET8(chan);
          GET32(nsamples);
#ifdef DEBUG1
          printf("  chan=%d, nsamples=%d\n",chan,nsamples);
#endif

          slotout[nh] = slot;
          chanout[nh] = chan+1; /*channels are numbered from 0 to 15, FORTRAN like from 1 to 16*/
          if(nsamples>100)
	  {
            printf("WARN: nsamples=%d, will take 100\n",nsamples);
	  }
          for(kk=0; kk<nsamples; kk++)
	  {
	    GET16(adc_sample);
            if(kk<100) rawadcout[nh][kk] = adc_sample;
	  }

#ifdef DEBUG1
          printf("---> %d %d %d ... %d %d %d\n",rawadcout[nh][0],rawadcout[nh][1],rawadcout[nh][2],
		 rawadcout[nh][97],rawadcout[nh][98],rawadcout[nh][99]);
#endif

	  nh ++;
          if(nh >= MAXPCALHITS)
          {
            printf("TOO MANY HITS, RETURN %d\n",nh);
            *nhits = nh;
            return(nh);
          }

	}
#ifdef DEBUG1
        printf("end loop: b08=0x%08x\n",b08);
#endif
      }
  }



  *nhits = nh;
  return(nh);
}



//------------------------------------------------------------------------


unsigned long *find_bank_in_dd(long event[], char *bank, int nbank, int &nwrds) {


  char *name;
  int nbnk,ncols,nrows;

  int ind   = 11;
  int evlen = event[10] + 11;


  // search all banks in DD event

  while (ind<evlen) {
    
    name  = (char *)&event[ind+1];
    nbnk  = event[ind+3];
    ncols = event[ind+4];
    nrows = event[ind+5];
    nwrds = event[ind+8];


    // index of 1st data word
    ind  += event[ind];
    
    // return pointer to data words if bank found (check name AND number)
    if ((strncmp(name,bank,strlen(bank))==0) && (nbank==nbnk)) return(unsigned long *)&event[ind];
    
    // index of header of next bank
    ind += nwrds;
  }
  

  // didn't find the bank
  return (unsigned long *)0;

}


//-------------------------------------------------------------------


//  fortran interface, returns array element, not pointer
long *find_bank_in_dd_(long *fevent, char *fbank, int *fnbank, int *fnwrds, int fblen) {

  int nwrds;
  long *bankptr;

  char *bank=strdupf(fbank,fblen);
  bankptr=(long *)find_bank_in_dd(fevent, bank, *fnbank, nwrds);
  free(bank);

  if(bankptr!=0) {
    *fnwrds=nwrds;
    return((long *)(bankptr-fevent));
  } else {
    *fnwrds=0;
    return((long *)0);
  }
}


//-------------------------------------------------------------------

