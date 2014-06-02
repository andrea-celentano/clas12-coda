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
//      CODA RunControl data variable RCS conf file name writer
//
// Author:  
//      Sergey: copied from rcsLogFileDesWriter and modified
//

#include <daqRun.h>
#include <daqData.h>
#include "rcsConfFileNameWriter.h"

rcsConfFileNameWriter::rcsConfFileNameWriter (daqRun* r)
:daqDataWriter (), run_ (r)
{
#ifdef _TRACE_OBJECTS
  printf ("    Create rcsConfFileNameWriter Class Object\n");
#endif
  // empty
}

rcsConfFileNameWriter::~rcsConfFileNameWriter (void)
{
#ifdef _TRACE_OBJECTS
  printf ("    Delete rcsConfFileNameWriter Class Object\n");
#endif
  // empty
}

void
rcsConfFileNameWriter::write (daqData* data)
{
  // get my own copy
  static char filename[128];
  /*
printf("111\n");
sleep(3);
  */
  /*sergey: (char *)(*data) calls overloaded operator char* from daqData */
  ::strncpy (filename, (char *)(*data), sizeof (filename));
  /*
printf("222\n");
sleep(3);
  */
  /*printf("!!!!!!!!!!!!!!! rcsConfFileNameWriter::write >%s<\n",filename);*/

  run_->updateConfFile (filename);
}
