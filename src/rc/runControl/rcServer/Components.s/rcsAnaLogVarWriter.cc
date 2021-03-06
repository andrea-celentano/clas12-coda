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
//      CODA RunControl data variable ANA Log Writer
//
// Author:  
//      Jie Chen
//      CEBAF Data Acquisition Group
//
// Revision History:
//   $Log: rcsAnaLogVarWriter.cc,v $
//   Revision 1.1.1.1  1996/10/11 13:39:20  chen
//   run control source
//
//
#include <daqRun.h>
#include "rcsAnaLogVarWriter.h"

rcsAnaLogVarWriter::rcsAnaLogVarWriter (daqRun* run)
:daqDataWriter (), run_ (run)
{
#ifdef _TRACE_OBJECTS
  printf ("    Create rcsAnaLogVarWriter Class Object\n");
#endif
  // empty
}

rcsAnaLogVarWriter::~rcsAnaLogVarWriter (void)
{
#ifdef _TRACE_OBJECTS
  printf ("    delete rcsAnaLogVarWriter Class Object\n");
#endif
  // empty
}

void
rcsAnaLogVarWriter::write (daqData* data)
{
  run_->updateAnaLogInfo (data->compname(), (char *)(*data));
}
