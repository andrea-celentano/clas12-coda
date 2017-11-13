#ifndef __MESSAGE_ACTION_TEST__
#define __MESSAGE_ACTION_TEST__

#include "MessageAction.h"

class MessageActionTest : public MessageAction {

  private:

    static const int NFORMATS=1;
    std::string formats[NFORMATS] = {"testtest"};
    int formatid;

    int debug;

      uint32_t in;
	  uint8_t bt;
      float fl;
      double db;
	  std::string str;
      char ch[10];

      int ilen, flen, dlen;
      int32_t idata[10];
      float   fdata[10];
      double  ddata[10];


  public:
    MessageActionTest(){}

    MessageActionTest(int debug_)
    {
      debug = debug_;
    }

    ~MessageActionTest(){}

    int check(std::string fmt)
    {
	  printf("check: fmt >%s<\n",fmt.c_str());
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

      recv >> in >> bt >> fl >> db >> str >> ch;
	  std::cout<<std::endl<<"DECODEMESSAGE "<<in<<" "<<+bt<<" "<<fl<<" "<<db<<" '"<<str.c_str()<<"' '"<<ch<<"' "<<std::endl<<std::endl;



	  
	  try {

        ilen=0;
        while(ilen!=-1)
        {
          recv >> idata >> GetSize(&ilen);
          std::cout<< "ilen= "<<ilen<<std::endl;
          for(int i=0; i<ilen; i++) std::cout << "=--------- idata["<<i<<"]="<<idata[i]<< std::endl;
        }

        ilen=0;
        while(ilen!=-1)
        {
          recv >> idata >> GetSize(&ilen);
          std::cout<< "ilen= "<<ilen<<std::endl;
          for(int i=0; i<ilen; i++) std::cout << "=--------- idata["<<i<<"]="<<idata[i]<< std::endl;
        }

        flen=0;
        while(flen!=-1)
        {
          recv >> fdata >> GetSize(&flen);
          std::cout<< "flen= "<<flen<<std::endl;
          for(int i=0; i<flen; i++) std::cout << "=--------- fdata["<<i<<"]="<<fdata[i]<< std::endl;
       }

        dlen=0;
        while(dlen!=-1)
        {
          recv >> ddata >> GetSize(&dlen);
          std::cout<< "dlen= "<<dlen<<std::endl;
          for(int i=0; i<dlen; i++) std::cout << "=--------- ddata["<<i<<"]="<<ddata[i]<< std::endl;
        }
		
	  }
      catch (const MessageEOFException& ex) {
		printf("\nEnd Of Message reached !!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");
	  }
      catch (const std::exception& ex) {
        printf("EXCEPTION %s\n", ex.what());
        //return;
        //throw;
	  }
	  


	  printf("\n\n\n\n");
    }


    void process()
    {
      //std::cout << "MessageActionTest: process TEST message" << std::endl;      
    }
};

#endif
