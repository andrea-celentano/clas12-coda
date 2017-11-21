
/* msg_to_epics_server_send.cc */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

using namespace std;
#include <strstream>
#include <iomanip>
#include <fstream>
#include <string>
#include <iostream>

#include "ipc.h"

#if 0

#include "json/json.hpp"
using json = nlohmann::json;

#include "ipc_lib.h"
IpcServer &server = IpcServer::Instance();

int
main()
{
  strstream message;

  printf("use IPC_HOST >%s<\n",getenv("IPC_HOST"));

  // connect to ipc server
  //server.init(getenv("EXPID"), NULL, NULL, "*", NULL, "*");
  //server.init(NULL, NULL, NULL, "HallB_DAQ", NULL, "HallB_DAQ");
  server.init("clasrun","clasprod","daq","HallB_DAQ", NULL, "HallB_DAQ");
  int iarray[6] = {6,7,8,9,13,123};
  float farray[5] = {1.,2.,55.,4.,5.};


	  //if(debug) printf("\n\nepics_msg_callback reached\n");
	  //if(debug) printf("numfields=%d\n",msg.NumFields());
	  //if(debug) printf("destination >%s<\n",msg.Dest());
	  //if(debug) printf("sender >%s<\n",msg.Sender());

	  //printf("Message:\n");

      msg >> sender >> host >> user >> time;
      if(debug) printf("  Sender >%s<\n",sender.c_str());
      if(debug) printf("  Host >%s<\n",host.c_str());
      if(debug) printf("  User >%s<\n",user.c_str());
      if(debug) printf("  Unixtime >%d<\n",time);

      msg >> caname >> catype >> nelem;

      if(debug) printf("  caname >%s<\n",caname.c_str());
      if(debug) printf("  catype >%s<\n",catype.c_str());
      if(debug) printf("  nelem >%d<\n",nelem);
      if(nelem > MAX_ELEM)
      {
        printf("WARN: nelem > %d, will set nelem=%d\n",MAX_ELEM,MAX_ELEM);
        nelem = MAX_ELEM;
      }

      if(      !strcmp(catype.c_str(),"char"))   for(ii=0; ii<nelem; ii++) {msg >> d_char[ii]; /*printf(" %c",d_char[ii]);*/}
      else if( !strcmp(catype.c_str(),"uchar"))  for(ii=0; ii<nelem; ii++) {msg >> d_uchar[ii]; /*printf(" %d",d_uchar[ii]);*/}
      else if( !strcmp(catype.c_str(),"short"))  for(ii=0; ii<nelem; ii++) {msg >> d_short[ii]; /*printf(" %d",d_short[ii]);*/}
      else if( !strcmp(catype.c_str(),"ushort")) for(ii=0; ii<nelem; ii++) {msg >> d_ushort[ii]; /*printf(" %d",d_ushort[ii]);*/}
      else if( !strcmp(catype.c_str(),"int"))    for(ii=0; ii<nelem; ii++) {msg >> d_int[ii]; /*printf(" %d",d_int[ii]);*/}
      else if( !strcmp(catype.c_str(),


  /*
  json j0 = {
	        {"EventRate",54321}
  };

  json j1 = {
            {"TestScalers", iarray}
  };

  json j2 = {
            {"TestVals", farray}
  };

  json j3;
  j3.push_back(j0);
  j3.push_back(j1);
  j3.push_back(j2);

  */


  /*
  json j0 = {
	{"EventRate",54921},
	{"TestScalers", iarray},
	{"TestVals", farray}
  };

  json j3;
  j3.push_back(j0);
  */





  json j3 = {
	{"TestScalers", iarray}
  };



  //message << "{\"items\": " << j3.dump() << "}" <<ends;
  message << j3.dump() <<ends;
  cout << "will send >" << message.str() << "<" << endl;
  server << clrm << message.str() << endm;




  /*  
  message << "{\"EventRate\":4502}" <<ends;
  cout << "will send >" << message.str() << "<" << endl;
  server << clrm << message.str() << endm;
  */

}


#else


#define MAXELEM 10

int
main()
{
  int32_t  iarray[MAXELEM] = {-1,-2,-3,-4,-5,-6,-7,-8,-9,-19};
  uint32_t uarray[MAXELEM] = {10,9,8,7,6,5,4,3,2,1};
  float    farray[MAXELEM] = {0.0,1.1,3.3,4.4,5.5,6.6,7.7,8.8,9.9,10.99};
  double   darray[MAXELEM] = {0.0,2.277,3.3333,4.455555,5.5333333,6.66666666,7.7888888888,8.83333333,9.92222222,10.991111111};
  uint8_t  carray[MAXELEM] = {18,19,33,66,77,25,25,25,27,28};

  /* params: (expid,session,myname,chname,chtype,nelem,data_array) */
  send_daq_message_to_epics("clasrun","clasprod","daq", "TestScalers", "int", MAXELEM, iarray);
  //send_daq_message_to_epics("*","*","*", "TestScalers", "int", MAXELEM, iarray);

  iarray[0] = 12345678;
  send_daq_message_to_epics("clasrun","clasprod","daq", "EventRate", "int", 1, iarray);

  send_daq_message_to_epics("clasrun","clasprod","daq", "TestVals", "float", MAXELEM, farray);

  send_daq_message_to_epics("clasrun","clasprod","daq", "DoubleVals", "double", MAXELEM, darray);
}

#endif
