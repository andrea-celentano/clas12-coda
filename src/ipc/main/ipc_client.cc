
/* ipc_client.cc - ipc client testing program */


#include "ipc_lib.h"

#include "MessageActionControl.h"
#include "MessageActionTest.h"

/* smartsockets - activemq

 application - session ?
 unique_id   - 

.............- topic/queue


*/


/* if 'ONE_SERVER' defined, both sending and receiving will be done using 'server',
   otherwise 'server' will be used for sending and 'recver' for receiving */
#define ONE_SERVER


#ifdef ONE_SERVER
IpcServer &server = IpcServer::Instance();
#else
IpcProducer &server = IpcProducer::Instance();
IpcConsumer &recver = IpcConsumer::Instance();
#endif


int
main(int argc AMQCPP_UNUSED, char* argv[] AMQCPP_UNUSED)
{
  int i;

    std::cout << "=====================================================\n";
    std::cout << "Starting the example:" << std::endl;
    std::cout << "-----------------------------------------------------\n";

    //============================================================
    // set to true to use topics instead of queues
    // Note in the code above that this causes createTopic or
    // createQueue to be used in both consumer an producer.
    //============================================================
    int numMessages = 1;
    int ret;

#ifdef ONE_SERVER
    server.init(getenv("EXPID"), NULL, NULL, (char *)"server");
#else
    server.send_init(getenv("EXPID"), NULL, NULL, (char *)"server");
    recver.recv_init(getenv("EXPID"), NULL, NULL, (char *)"recver", NULL, (char *)"server");
#endif

    MessageActionControl  *control = new MessageActionControl((char *)"server");
    MessageActionTest        *test = new MessageActionTest();

#ifdef ONE_SERVER
    server.addActionListener(control);
    server.addActionListener(test);
#else
    recver.addActionListener(control);
    recver.addActionListener(test);
#endif

    long long startTime = System::currentTimeMillis();


    for(i=0; i<numMessages; i++)
	{

	  std::string str = "qwerty";

      server << clrm << "testtest";
      server << (int32_t)333 << (uint8_t)5 << (float)7.7 << (double)987987987076.99;
      server << str << (const char *)"abcd";



      int array[5]={9, 8, 7, 6, 5};
      int* arrayPtr = &array[0];
      float farray[5]={9.9, 8.8, 7.7, 6.6, 5.5};
      std::vector<double> v({2.5, 3.6, 4.5, 5.5, 6.5});


      server << SetSize(4) << SetOffset(1) << arrayPtr;

      server << SetSize(2) << SetOffset(1) << array;

      server << SetSize(2) << SetOffset(1) << farray;

      server << SetSize(4) << SetOffset(1) << v;


      server << SetSize(4) << SetOffset(1) << arrayPtr;


      server << endm;
	}

    server << clrm << (char *)"control" << "quit" << endm;


    //printf("done1=%d\n",control->done);
    sleep(1);
    //printf("done2=%d\n",control->done);

    long long endTime = System::currentTimeMillis();
    double totalTime = (double)(endTime - startTime) / 1000.0;


#ifdef ONE_SERVER
    server.close();
#else
    server.send_close();
    recver.recv_close();
#endif



    std::cout << "Time to completion = " << totalTime << " seconds." << std::endl;
    std::cout << "-----------------------------------------------------\n";
    std::cout << "Finished with the client." << std::endl;
    std::cout << "=====================================================\n";

	exit(0);
}

