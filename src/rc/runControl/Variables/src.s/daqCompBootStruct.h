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
//      CODA run control : component auto boot values
//
// Author:  
//      Jie Chen
//      CEBAF Data Acquisition Group
//
// Revision History:
//   $Log: daqCompBootStruct.h,v $
//   Revision 1.1.1.1  1996/10/11 13:39:30  chen
//   run control source
//
//
#ifndef _CODA_DAQ_COMPBOOT_STRUCT_H
#define _CODA_DAQ_COMPBOOT_STRUCT_H

#include <daqArbStruct.h>

#include <stdint.h>

class daqCompBootStruct: public daqArbStruct
{
public:
  daqCompBootStruct  (void);
  ~daqCompBootStruct (void);

  /* return duplicated object */
  daqArbStruct* dup (void);

  // return size information
  size_t size (void);

  // return run time id information
  /*long*/int64_t   id   (void);

  // encode information
  void   encode (char* buffer, size_t &bufsize);
  
  // decode information
  void   decode (char* buffer, size_t size);

  // clean up
  void   cleanUp (void);

  // add new components and auto boot info to the struct
  void   insertInfo (char* component, int autoboot);

  // get components and auto boot information
  /*long*/int64_t   compBootInfo (char** &components, /*long*/int64_t* &autoboot);

  // print out all information
  void   dumpAll (void);

  // class name
  const char* className (void) const {return "daqCompBootStruct";}

protected:

  // encode data area
  void   encodeData (void);
  // restore data
  void   restoreData (void);

private:

  // data area
  static int maxNumComps;
  static int maxCompNameLen;

  /* all components name (used in rcClientHandler, rcCompBootDialog, daqRun) */
  int64_t/*long*/    id_;             // run time decoding id
  int64_t/*long*/    numComponents_;





#if 0
  int64_t/*long*/   *autoboot_;
#endif
  union auto_ptr {
    int64_t/*long*/   *autoboot_;
    int64_t  filler;
  } a_; /* used in this class only */


  /* sergey
  char**  components_;
  */
  union comp_ptr {
    char**  components_;
    int64_t  filler;
  } c_; /* used in this class only */





  // one has to count virtual function table pointer size
  // to align in 8 byte boundary; 'static' variables does not count;
  // inheritated from class 'daqArbStruct' count and it's size must
  // be subtracted to get our size only (see 'realsize' in 'daqCompBootStruct::decode()'
};
#endif
