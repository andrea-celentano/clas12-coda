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
//      CODA RunControl data variable RCS Config File Name writer
//
// Author:  
//      Sergey: copied from rcsLogFileDesWriter and modified

//
#ifndef _CODA_RCS_CONFFILENAME_WRITER_H
#define _CODA_RCS_CONFFILENAME_WRITER_H

#include <daqDataWriter.h>

class daqRun;

class rcsConfFileNameWriter: public daqDataWriter
{
public:
  // constructor and destructor
  rcsConfFileNameWriter  (daqRun* run);
  ~rcsConfFileNameWriter (void);

  virtual void write   (daqData* data);

private:
  daqRun* run_;
};

#endif
