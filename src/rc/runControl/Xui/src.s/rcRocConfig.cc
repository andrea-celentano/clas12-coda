//-----------------------------------------------------------------------------
// Copyright (c) 1994,1995 Southeastern Universities Research Association,
//                         Continuous Electron Beam Accelerator Facility
//
// This software was developed under a United States Government license
// described in the NOTICE file included as part of this distribution.
//
// CEBAF Data Acquisition Group, 12000 Jefferson Ave., Newport News, VA 23606
//       coda@cebaf.gov  Tel: (804) 249-7030     Fax: (804) 249-5800
//-----------------------------------------------------------------------------
//
// Description:
//      Implementation of rcRocConfig Class
//
// Author:  
//      Jie Chen
//      CEBAF Data Acquisition Group
//
// Revision History:
//   $Log: rcRocConfig.cc,v $
//   Revision 1.2  1999/12/09 21:29:01  rwm
//   Do not do clean memory on linux - bug in Xlibs?
//
//   Revision 1.1.1.1  1996/10/11 13:39:25  chen
//   run control source
//
//
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <XcodaApp.h>

#include <codaComdXInterface.h>

#include <rcNetStatus.h>
#include <rcInfoPanel.h>

#include "rcRocConfig.h"


/*sergey: codaedit stuff */
#include "Editor_database.h"


rcRocConfig::rcRocConfig (char* name, int active,
		  char* acc, char* acc_text,
		  rcClientHandler& handler)
  :rcMenuComd (name, active, acc, acc_text, handler)
{
#ifdef _TRACE_OBJECTS
  printf ("         Create rcRocConfig Class Object\n");
#endif

  char *dbasehost;
  char dbasename[128];

  printf("rcRocConfig::config reached\n");

  dbasehost = getenv("MYSQL_HOST");
  sprintf(dbasename,"daq_%s",getenv("EXPID"));

  printf("rcRocConfig::config: database host is >%s<, database name is >%s<\n",dbasehost,dbasename);

  if (connectToDatabase (dbasehost) == 0)
  {
    printf("Cannot connect to a CODA database server\n");
    exit(0);
  }
  selectDatabase(dbasename); /* call codaedit's function to let it know which database to use */
}

rcRocConfig::~rcRocConfig (void)
{
#ifdef _TRACE_OBJECTS
  printf ("         Delete rcRocConfig Class Object\n");
#endif
  // dialog_ will be destroyed by Xt Mechanism
}

void
rcRocConfig::doit (void)
{
  printf("rcRocConfig::doit reached\n");

  config ();
}

void
rcRocConfig::undoit (void)
{
  // empty
}







extern Widget toplevel;


#include "rcRocMenuWindow.h"
class rcRocMenuWindow;
extern rcRocMenuWindow *menu_window;


void
rcRocConfig::config (void)
{

  menu_window->InitConfigSel (toplevel);
  menu_window->ConfigSelPopup();

  if (netSt_) netSt_->stop ();
  if (rcipanel_) rcipanel_->stop ();
  if (netHandler_.connected ()) netHandler_.disconnect ();
}
