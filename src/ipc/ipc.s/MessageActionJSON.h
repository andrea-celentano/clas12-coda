#ifndef __MESSAGE_ACTION_JSON__
#define __MESSAGE_ACTION_JSON__

#include "MessageAction.h"


#include "RCDB/WritingConnection.h"
#include "json/json.hpp"
using json = nlohmann::json;


class MessageActionJSON : public MessageAction {



  private:

    static const int NFORMATS=1;
    std::string formats[NFORMATS] = {"json"};
    int formatid;

    int debug;
    std::string str;


  public:

    MessageActionJSON() {debug = 0;}

    MessageActionJSON(int debug_) {debug = debug_;}

    ~MessageActionJSON() {}

    int check(std::string fmt)
    {
	  printf("\ncheckJSON: fmt >%s<\n",fmt.c_str());
	  /*
      std::vector<std::string> list = fmtsplit(fmt, std::string(":"));
      for(std::vector<std::string>::const_iterator s=list.begin(); s!=list.end(); ++s)
	  {
		std::cout << *s << " ";
	  }
	  std::cout << endl;
	  */
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
	  recv >> str;

	  if(debug) printf("\nMessageActionJSON received >%s<\n",str.c_str());

    }



    void process()
    {
      using namespace std;

      if(debug) return;

      std::cout << "MessageActionJSON: process JSON message: " << str.c_str() << std::endl;      

      rcdb::WritingConnection connection("mysql://rcdb:e1tocome@clondb1/rcdb");

      std::string sql_statement = str;

      cout << "json>" << sql_statement << endl << endl;

      int run_number;
      int ich;
      char *ch, runstr[20];
	  std::string json_string;

      json_string = str;
      /* extract json 
      ch = (char *)&sql_statement[4];
      //printf("ch >%s<\n",ch);
      ich = -1;
      for(int i=0; i<strlen(ch); i++)
	  {
        if(ch[i]=='[')
	    {
          ich = i;
          //printf("ich=%d\n",ich);
          break;
	    }
      }
      if(ich==-1) printf("ERROR: json string does not contains any '{'\n");
	  else
	  {
	    json_string = (char *)&ch[ich];
	  }
	  */
      cout << "json_string>" << json_string << endl;


      json j3 = json::parse(json_string);
      auto j1 = j3[0];
      auto j2 = j3[1];


      for (auto& element : j1)
      {
        std::cout << element << '\n';
      }

	  /* get name */
      string name = j1["name"];
      printf("name >%s<\n",name.c_str());

      /*get run number*/
      run_number = j1["run_number"];
      printf("run_number=%d\n",run_number);


      char *cond[] = {
            "event_count",
            "events_rate",
            "temperature",
            "beam_energy",
            "test",
            "beam_current",
            "torus_scale",
            "solenoid_scale",
            "daq_trigger",
            "target_position",
            "daq_comment",
            "is_valid_run_end",
            "status"
	  };


      /* for 'run_log', add conditions one by one */
      if(!strncmp(name.c_str(),"run_log",7))
	  {
        //connection.AddRunStartTime(run_number, j1["run_start_time"]);
        //connection.AddRunEndTime(run_number, j1["run_end_time"]);
  
	    for(int i=0; i<(sizeof(cond)/sizeof(char *)); i++)
	    {
          if (j1[cond[i]].type() == json::value_t::number_float)
          {
            printf("Add condition >%s<\n",cond[i]);
            connection.AddCondition(999, cond[i], j1[cond[i]].get<float>());
          }
          else if (j1[cond[i]].type() == json::value_t::number_unsigned) //number_integer does not recognize int32_t from run_log_begin
          {
            printf("Add condition >%s<\n",cond[i]);
            //connection.AddCondition(999, cond[i], j1[cond[i]].get<int>());
            connection.AddCondition(999, cond[i], (long int)j1[cond[i]]);
          }
          else if (j1[cond[i]].type() == json::value_t::boolean)
          {
            printf("Add condition >%s<\n",cond[i]);
            //connection.AddCondition(999, cond[i], j1[cond[i]].get<std::bool>());
            connection.AddCondition(999, cond[i], (bool)j1[cond[i]]);
          }
          else if (j1[cond[i]].type() == json::value_t::string)
          {
            printf("Add condition >%s<\n",cond[i]);
            connection.AddCondition(999, cond[i], j1[cond[i]].get<std::string>());
          }
          else
		  {
            printf("Cannot add condition >%s< - unknown type\n",cond[i]);
		  }
	    }
	  


	    /*
        beam_current  number     7
        beam_energy   number     7
        daq_comment   string     3
        daq_trigger   string     3
        event_count   number     6
        events_rate   number     7
        is_valid_run_end boolean    4
        name          string     3
        run_end_time  string     3
        run_number    number     6
        run_start_time string     3
        solenoid_scale number     7
        status        number     6
        target_position number     7
        temperature   number     6
        test          number     7
        torus_scale   number     7

        for (json::iterator it = j1.begin(); it != j1.end(); ++it)
        {
          std::cout<< std::left<< std::setw (13)<<it.key()<<" "
                   << std::left<< std::setw (10)<< it.value().type_name() <<" "
                   << (int)it.value().type() << '\n';
        }
	    */


	  }


      cout<<endl<<j2.dump()<<endl<<endl;
      connection.AddCondition(run_number, "json_cnd", j2.dump());

#if 0
	
      auto j = json::parse(json_string);
 
      // iterate the array
      for (json::iterator it = j.begin(); it != j.end(); ++it)
      {
        std::cout << *it << '\n';
      }

      // range-based for
      for (auto& element : j)
      {
        std::cout << element << '\n';
      }
      int32_t ev_cnt = j["event_count"];
      auto trigger = j["daq_trigger"].get<std::string>();
	  /*
      int32_t vec[4];
      vec[0] = j["c_vector"][0];
      vec[2] = j["c_vector"][2];
	  */
      auto vec = j["c_vector"].get<std::vector<int>>();
      cout<<endl<<"ev_cnt="<<ev_cnt<<"  trigger="<<trigger<<"   vec="<<vec[0]<<" "<<vec[2]<<endl;

	  /*
      // other stuff
      j.size();     // 3 entries
      j.empty();    // false
      j.type();     // json::value_t::array
      j.clear();    // the array is empty again

      // convenience type checkers
      j.is_null();
      j.is_boolean();
      j.is_number();
      j.is_object();
      j.is_array();
      j.is_string();
	  */
#endif

    }

};

#endif
