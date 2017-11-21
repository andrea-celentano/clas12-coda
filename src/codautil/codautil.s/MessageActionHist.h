#ifndef __MESSAGE_ACTION_HIST__
#define __MESSAGE_ACTION_HIST__

#include "MessageAction.h"

#include "hbook.h"
#ifdef USE_ROOT
#include "CMHbook.h"
#endif

class MessageActionHist : public MessageAction {

  private:

    static const int NFORMATS=1;
    std::string formats[NFORMATS] = {"hist"};
    int formatid;

    int debug;
    int done;
    int status;
    int statistics;
    std::string myname;
    int len;
	std::string title;

    int packed;
    Hist hist;

#ifdef USE_ROOT
    CMHbook *hbook;
#endif

  public:

    MessageActionHist(){}


#ifdef USE_ROOT
    MessageActionHist(std::string myname_, int debug_ = 0, CMHbook *hbook_ = NULL)
    {
      myname = myname_;
      done = 0;
      status = 0;
      statistics = 0;
      debug = debug_;
      hbook = hbook_;
    }
#else
    MessageActionHist(std::string myname_, int debug_ = 0)
    {
      myname = myname_;
      done = 0;
      status = 0;
      statistics = 0;
      debug = debug_;
    }
#endif

    ~MessageActionHist(){}

    int check(std::string fmt)
    {
	  printf("check: testing fmt >%4.4s<\n",fmt.c_str());
      for(int i=0; i<NFORMATS; i++)
	  {
        std::string f = formats[i];
        if( !strncmp(f.c_str(),fmt.c_str(),strlen(f.c_str())) )
		{
          formatid = i;
          return(1);
		}
	  }

      formatid = 0;
      return(0);
    }


    void decode(IpcConsumer& recv)
    {
      int ind = 0;

      recv >> packed >> hist.id >> hist.entries >> hist.ntitle >> title;
      hist.title = strdup(title.c_str());

	  std::cout<<"id="<<hist.id<<" ntitle="<<hist.ntitle<<" title="<<hist.title<<std::endl;

      recv >> hist.nbinx >> hist.xmin >> hist.xmax >> hist.xunderflow >> hist.xoverflow >> hist.nbiny;

      printf("nbinx=%d nbiny=%d\n",hist.nbinx,hist.nbiny);
      if(hist.nbiny==0)
	  {
        hist.buf = (float *) calloc(hist.nbinx,sizeof(float));
        if(hist.buf==NULL)
        {
          printf("ERROR in calloc()\n");
		}
		else
		{
          for(int ibinx=0; ibinx<hist.nbinx; ibinx++)
          {
            recv >> hist.buf[ibinx];
            printf("[%d] --> %f\n",ibinx,hist.buf[ibinx]);
	      }
		}
	  }

	  printf("\n\n");
    }


    void process()
    {
      //std::cout << "MessageActionHist: process Hist message" << std::endl;      


#ifdef USE_ROOT
      hbook->CMhist2root(hist);
#endif

    }



};

#endif
