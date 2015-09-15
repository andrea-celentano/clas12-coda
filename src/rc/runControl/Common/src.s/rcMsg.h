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
//      run control server/client exchange messages
//
// Author:  
//      Jie Chen
//      CEBAF Data Acquisition Group
//
// Revision History:
//   $Log: rcMsg.h,v $
//   Revision 1.1.1.1  1996/10/11 13:39:31  chen
//   run control source
//
//
#ifndef _CODA_RC_MSG_H
#define _CODA_RC_MSG_H

#include <daqCommon.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <INET_Addr.h>
#include <SOCK_Stream.h>

#include <rcSvcProtocol.h>
#include <daqNetData.h>

const int ALIGN_WORD_LEN = 8;

inline int roundUp (int a, int b) 
{
  return ((a + b -1)/b)*b;
}

const int RCMSG_PREDATA_SIZE = 4*sizeof(int64_t);

class rcMsg
{
public:

  // constructor and destructor
  rcMsg  (int64_t type, daqNetData& data, int64_t reqId = 0);
  rcMsg  (const rcMsg&);
  rcMsg& operator = (const rcMsg&);

  ~rcMsg (void);

  int64_t   type     (void) const;
  int64_t   dataSize (void) const;
  int64_t   reqId    (void) const;

  char*  compname (void);
  char*  attrname (void);

  // get daqNetData reference
  operator daqNetData& (void);
  
  // friend functions to perform I/O operation
  friend int operator << (SOCK_Stream& out, rcMsg& msg);
  friend int operator >> (SOCK_Stream& in,  rcMsg& msg);

  friend int operator << (int out, rcMsg& msg);
  friend int operator >> (int in,  rcMsg& msg);

protected:

  /* byte swapping utilities */
  void decode (void);
  void encode (void);

private:

  /***********************************************************************/
  /* header elements; defined above RCMSG_PREDATA_SIZE have to correspond
     to the number of the following variables except last 'data_' */

  /* command type */
  int64_t        type_;

  /* user callback pointer casted into long 
     may not be portable for 64 bit arch */
  int64_t        arg_;

  /* size of network data to follow */
  int64_t        size_;

  /* padding */
  int64_t        unused_;




  /*********************/
  /* real network data */
  daqNetData  data_;

};

#endif

  
