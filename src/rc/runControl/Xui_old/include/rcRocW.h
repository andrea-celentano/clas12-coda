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
//      RunControl Top Level Window
//
// Author:  
//      Jie Chen
//      CEBAF Data Acquisition Group
//
// Revision History:
//   $Log: rcRocW.h,v $
//   Revision 1.2  1997/07/08 14:59:17  heyes
//   deep trouble
//
//   Revision 1.1.1.1  1996/10/11 13:39:24  chen
//   run control source
//
//
#ifndef _RC_ROCW_H
#define _RC_ROCW_H

#include <XcodaTopLevel.h>
#include <rcClientHandler.h>
#include <rcRocMenuWindow.h>

class rcRocMenuWindow;

class rcRocW: public XcodaTopLevel
{
public:
  // constructor and destructor
  rcRocW   (char* name, rcClientHandler& handler);
  virtual ~rcRocW (void);
  Widget RightWindow();
  rcRocMenuWindow* window_;

protected:
  virtual Widget CreateBaseWidget (Widget parent);

private:
  rcClientHandler& netHandler_;
};
#endif
