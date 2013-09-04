
/* vmeservermain.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef VXWORKS

int
vmeServer()
{
  return(0);
}

#else

int
main(int argc, char *argv[])
{
  char myname[256];

  /* Open the default VME windows */
  vmeOpenDefaultWindows();

  if(argc==2)
  {
    strncpy(myname, argv[1], 255);
    printf("use argument >%s< as host name\n",myname);
  }
  else
  {
    
    strncpy(myname, getenv("HOSTNAME"), 255);
    printf("use env var HOST >%s< as host name\n",myname);
  }

  vmeServer(myname);
  while(1) sleep(1);
}

#endif

