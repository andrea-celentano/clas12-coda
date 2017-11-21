
/* uthbook_test.cc */

#include <stdio.h>

#include "uthbook.h"

#include "ipc_lib.h"
IpcServer &server = IpcServer::Instance();

char *app  = getenv("EXPID");
char *dest = NULL;

int
main()
{
  // connect to ipc server
  server.init(getenv("EXPID"), getenv("SESSION"), "daq", "*", NULL, "*");


  uthbook1(1, "test1", 100, 0.0, 100.0);
  for(int i=0; i<100; i++)
  {
    uthfill(1, ((float)i), 0.0, ((float)i)*2.);
  }
  uthprint(1);

  uthist2ipc(1, "uthbook_send");
 
}
