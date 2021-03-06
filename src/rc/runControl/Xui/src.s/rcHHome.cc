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
//      Implementation of rcConnect Button
//
// Author:  
//      Graham Heyes
//      CEBAF Data Acquisition Group
//
// Revision History:
//   $Log: rcHHome.cc,v $
//   Revision 1.1  1997/07/30 14:33:36  heyes
//   add more xpm support
//
//
//
#include <rcNetStatus.h>
#include <rcButtonPanel.h>
#include <rcComdOption.h>
#include "rcHHome.h"
#include "pixmaps/home.xpm"
rcHHome::rcHHome (Widget parent, rcButtonPanel* panel, 
		      rcClientHandler& handler)
  :rcXpmComdButton (parent, "Home", home_xpm,"Go to top html page", panel, handler)
{
  appContext_ = XtWidgetToApplicationContext (parent);
}

rcHHome::~rcHHome (void)
{
#ifdef _TRACE_OBJECTS
  printf ("               Delete rcHHome Class Object\n");
#endif
  // empty
}

extern int HTMLhomeCallback();

void
rcHHome::doit (void)
{
  HTMLhomeCallback();
}

void
rcHHome::undoit (void)
{
  // empty
}
