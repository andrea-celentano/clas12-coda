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
//      RunControl Connect Command
//
// Author:  
//      Graham Heyes
//      CEBAF Data Acquisition Group
//
// Revision History:
//   $Log: rcHHome.h,v $
//   Revision 1.1  1997/07/30 14:33:37  heyes
//   add more xpm support
//
//
#ifndef _RC_HOME_B_H
#define _RC_HOME_B_H

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <rcXpmComdButton.h>

class rcButtonPanel;

class rcHHome: public rcXpmComdButton
{
public:
  rcHHome  (Widget parent, rcButtonPanel* panel, rcClientHandler& handler);
  ~rcHHome (void);

  // class name
  virtual const char* className (void) const {return "rcHHome";}
  
protected:
  // inherited functions
  virtual void doit   (void);
  virtual void undoit (void);

private:
  XtAppContext appContext_;
};
#endif

