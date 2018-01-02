
/* send_daq_message_to_epics_recv.cc */

#define USE_ACTIVEMQ

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


using namespace std;
#include <strstream>
#include <fstream>
#include <iomanip>

#include <string>
#include <iostream>

#include "ipc_lib.h"
#include "MessageActionDAQ2EPICS.h"

IpcServer &server = IpcServer::Instance();

int
main()
{
  int debug = 1;
  int done = 0;

  printf(" use IPC_HOST >%s<\n",getenv("IPC_HOST"));

  // connect to ipc server
  //server.init(getenv("EXPID"), NULL, NULL, "*", NULL, "*");
  server.init(NULL, NULL, NULL, "HallB_DAQ", NULL, "HallB_DAQ");

  MessageActionDAQ2EPICS *epics = new MessageActionDAQ2EPICS();
  epics->set_debug(debug);
  server.addActionListener(epics);

  while(done==0)
  {
    sleep(1);
  }


}
