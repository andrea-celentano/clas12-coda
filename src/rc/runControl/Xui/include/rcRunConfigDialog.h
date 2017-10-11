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
//      RunControl RunConfig Dialog Class
//
// Author:  
//      Jie Chen
//      CEBAF Data Acquisition Group
//
// Revision History:
//   $Log: rcRunConfigDialog.h,v $
//   Revision 1.2  1997/06/16 12:26:51  heyes
//   add dbedit and so on
//
//   Revision 1.1.1.1  1996/10/11 13:39:24  chen
//   run control source
//
//

#ifndef _RC_RUN_CONFIG_DIALOG_H
#define _RC_RUN_CONFIG_DIALOG_H

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <rcClientHandler.h>
#include <XcodaFileSelDialog.h>

class rcRunConfigOption;
class XcodaErrorDialog;
class XcodaFileSelDialog; /*sergey*/

class rcRunConfigDialog: public XcodaFileSelDialog
{

public:

  // constructor and destructor
  rcRunConfigDialog (Widget parent,
		   char* name,
		   char* title,
		   rcClientHandler& handler);
  virtual ~rcRunConfigDialog (void);

  // dialog sends out download command
  void download (void);

  // redefine popup function
  void popup (void);

  // class name
  virtual const char* className (void) const {return "rcRunConfigDialog";}


protected:

  /* sergey: providing method(s) required by XcodaFileSelDialog */
  void execute(void);

  // report error message
  void reportErrorMsg (char *msg);

  // send download command to the server callback
  static void downloadCallback (int status, void* arg, daqNetData* data);

  /* send command to update 'confFile' in database */
  static void confFileCallback (int status, void* arg, daqNetData* );

  /*sergey*/
  int parseConfigFile(char *fname, int iter, char *);
  void updateConfFile (char *fname);


private:

  // network handler
  rcClientHandler& netHandler_;

  // option menu
  //rcRunTypeOption* option_;
  rcRunConfigOption* option_;

  // widget of pushbuttons

  // error dialog
  XcodaErrorDialog* errDialog_;

  /*sergey*/
  char *confname; 

};
#endif
