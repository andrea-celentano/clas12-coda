// run_operators

//  gets run operators


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <fstream>

using namespace std;



char *mysql_database = getenv("EXPID");
char *session       = getenv("SESSION");

extern "C" {

char *get_run_operators(char *mysql_database,char *session);
}

void decode_command_line(int argc, char **argv);


//----------------------------------------------------------------

int
main(int argc, char **argv)
{
  char *operators;

  decode_command_line(argc,argv);

  if(session==NULL) session=(char*)"clasprod";

  operators = get_run_operators(mysql_database, session);
  cout << operators << endl;
  
  exit(0);  
}


//----------------------------------------------------------------


void
decode_command_line(int argc, char **argv)
{
  int i=1;
  const char *help="\nusage:\n\n  run_status [-s session] [-m mysql_database]\n\n\n";

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
      mysql_database=strdup(argv[i+1]);
      i=i+2;
    }
  }
}


//--------------------------------------------------------------------

