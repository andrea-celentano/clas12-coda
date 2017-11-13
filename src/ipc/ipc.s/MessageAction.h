#ifndef __MESSAGE_ACTION__
#define __MESSAGE_ACTION__

#include <string>
#include <iostream>

class MessageAction {

  public:
    MessageAction(){}
    virtual ~MessageAction(){}
	virtual void init(){}
	virtual void close(){}
    virtual int  check(std::string fmt) {return(0);}
    virtual void decode(IpcConsumer& recv){}
    virtual void process(){}

	std::vector<std::string> fmtsplit(std::string str, std::string delimiter)
	{
	  std::vector<std::string> list;
	  std::string s = str;
      size_t pos = 0;
	  std::string token;
      while((pos = s.find(delimiter)) != std::string::npos)
	  {
        token = s.substr(0,pos);
		//std::cout<<"token="<<token<<std::endl;
        list.push_back(token);
        s.erase(0, pos + delimiter.length());
	  }
	  //std::cout << "s=" << s <<std::endl;
	  list.push_back(s);

      return list;
	}

};

#endif
