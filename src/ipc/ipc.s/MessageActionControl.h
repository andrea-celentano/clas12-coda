#ifndef __MESSAGE_ACTION_CONTROL__
#define __MESSAGE_ACTION_CONTROL__

#include <stdio.h>
#include <strings.h>

#include "MessageAction.h"



class MessageActionControl : public MessageAction {

  private:

  //static const int NFORMATS = 3;
  //std::string formats[NFORMATS] = {"command","status","statistics"};
    static const int NFORMATS = 1;
    std::string formats[NFORMATS] = {"command"};
    int formatid;

    int debug;
    int status;
    int statistics;
    int done;
    std::string myname;
    std::string command;


  public:

    MessageActionControl(std::string myname_, int debug_ = 0)
    {
      myname = myname_;
      done = 0;
      status = 0;
      statistics = 0;
      debug = debug_;
    }

    ~MessageActionControl(){}

	void setDone(int done_)
    {
      done = done_;
    }

	int getDone()
    {
      return(done);
    }

	void setDebug(int debug_)
    {
      debug = debug_;
    }

	int getDebug()
    {
      return(debug);
    }


    int check(std::string fmt)
    {
	  printf("\ncheckControl: fmt >%s<, debug=%d\n",fmt.c_str(),debug);
      int n = 0;

      std::vector<std::string> list = fmtsplit(fmt, std::string(":"));
      for(std::vector<std::string>::const_iterator s=list.begin(); s!=list.end(); ++s)
	  {
        printf("n=%d\n",n);
		std::cout << *s << " " << s->c_str() << std::endl;

		if(n==0) /* first field in 'fmt' is format, compare it with out formats */
		{
          formatid = -1;
          for(int i=0; i<NFORMATS; i++)
	      {
            std::string f = formats[i];
		    std::cout << "==" << f.c_str() << " " << s->c_str() << " " << strlen(f.c_str()) << std::endl;
            if( !strncmp(f.c_str(),s->c_str(),strlen(f.c_str())) )
		    {
              formatid = i;
              printf(" -> set formatid=%d\n",formatid);
              break;
		    }
	      }
          if(formatid==-1) return(0); /* unknown format */
		}
        else if(n==1) /* sender name; ignore if it is us */
		{
          if( !strncmp(myname.c_str(),s->c_str(),strlen(myname.c_str())) )
		  {
            printf("it is our own message - ignore\n");
            return(0);
		  }
		}

        n++;
	  }
      printf("\ncheckControl: formatid=%d\n",formatid);

      return(1);
    }


	/* in old system we had following commands:
      QUIT    		 call quit callback handler
      LOG_IN_DATA [ON|OFF]	 start logging incoming data messages
      LOG_OUT_DATA [ON|OFF]	 start logging outgoing data messages
      LOG_IN_STATUS [ON|OFF]	 start logging incoming status messages
      LOG_OUT_STATUS [ON|OFF]	 start logging outgoing status messages
      GMD_RESEND    		 resend all gmd messages (not sure if this works...)
      STATUS_POLL    		 send status_poll_result to monitoring group
      RECONNECT               disconnect and reconnect to server
	*/
    void decode(IpcConsumer& server)
    {
      if(debug) std::cout << "MessageActionControl: decode CONTROL message" << std::endl;      

      server >> command;
      if(debug) printf("Received command >%s<\n",command.c_str());

      if(strncasecmp(command.c_str(),"quit",4)==0)
      {
        if(debug) printf("set done=1\n");
        done = 1;
	  }
      else if(strncasecmp(command.c_str(),"status",6)==0)
      {
        if(debug) printf("reporting status\n");
        status = 1;
	  }
      else if(strncasecmp(command.c_str(),"statistics",10)==0)
      {
        if(debug) printf("reporting statistics\n");
        statistics = 1;
	  }

    }

    void process(){
      if(debug) std::cout << "MessageActionControl: process CONTROL message" << std::endl;

      if(status) {sendStatus();status=0;}
      if(statistics) {sendStatistics(0, 0, 0.0, 0.0);statistics=0;}

    }

    void sendCommand(char *destination, char *command)
	{
      if(debug) std::cout << "MessageActionControl: sendCommand " << command << "to " << destination << std::endl;     
      IpcServer &server = IpcServer::Instance();
      server << clrm << "command:"+myname << command /*<< time(0)*/;
      server << endm;
	}

    void sendStatus()
	{
      if(debug) std::cout << "MessageActionControl: sendStatus" << std::endl;     
      IpcServer &server = IpcServer::Instance();
      server << clrm << "status:"+myname << myname << 0/*<< time(0)*/;
      server << endm;
	}

    void sendStatistics(int32_t nev_received, int32_t nev_processed,
                        double rate_received, double rate_processed)
	{
      if(debug) std::cout << "MessageActionControl: sendStatistics" << std::endl;     
      IpcServer &server = IpcServer::Instance();
      server << clrm << "statistics:"+myname << myname << 0/*<< time(0)*/;
      server << nev_received;
      server << nev_processed;
      server << rate_received;
      server << rate_processed;
      server << endm;
	}

};

#endif
