// run_status

//  gets run status

//  ejw, 9-apr-97


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <fstream>

using namespace std;



char *msql_database = getenv("EXPID");
char *session       = getenv("SESSION");

#include "codautil.h"

void decode_command_line(int argc, char **argv);


//----------------------------------------------------------------

int
main(int argc, char **argv)
{
  char line[200];
  char state[32];
  char fsession[32];
  char *config, *conffile, *datafile;
  char fconfig[32];
  int run;
  int msql_run;
  int stat;


  decode_command_line(argc,argv);

  if(session==NULL) session=(char*)"clasprod";

  // try get_run_status
  config = get_run_status(msql_database,session);
  cout << config << endl;
  

/*
  // get run,config from database
  get_run_config(msql_database,session,&msql_run,&config,&conffile,&datafile);

  // get info from rcstate file
  sprintf(line,"%s/run_log/rcstate_%s.txt",getenv("CLON_PARMS"),session);
  ifstream file(line);
  if(!file.is_open()||!file.good()||file.eof()||file.fail()||file.bad()) {
    cout << "\n?Unable to read " << line << endl << endl;
    exit(EXIT_FAILURE);
  }

  file.getline(line,sizeof(line));
  sscanf(line,"%s %d %s %s",state,&run,fsession,fconfig);

  cout << "\n Info for session " << session << " (" << fsession << "):"  << endl << endl
       << "     current run: " << run 
       << "     config:  " << config
       << "     state:  " << state << endl << endl;

  if(msql_run!=run) {
    cout << "?msql database run is " << msql_run << ", disagrees with rcstate file: " 
	 << run << endl << endl;
  }
  if(strcmp(config,fconfig)!=0) {
    cout << "?msql database config is " << config << ", disagrees with rcstate file: "
	 << fconfig << endl << endl;
  }
  */  

}


//----------------------------------------------------------------


void
decode_command_line(int argc, char **argv)
{
  int i=1;
  const char *help="\nusage:\n\n  run_status [-s session] [-m msql_database]\n\n\n";

  while(i<argc) {
    
    if(strncasecmp(argv[i],"-h",2)==0){
      cout << help;
      exit(EXIT_SUCCESS);
    }
    else if (strncasecmp(argv[i],"-s",2)==0){
      session=strdup(argv[i+1]);
      i=i+2;
    }
    else if (strncasecmp(argv[i],"-m",2)==0){
      msql_database=strdup(argv[i+1]);
      i=i+2;
    }
  }
}


//--------------------------------------------------------------------

