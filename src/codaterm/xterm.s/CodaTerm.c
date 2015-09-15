
/* CodaTerm.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
/*
#include <Xm/PushBG.h>
#include <Xm/SeparatoG.h>
*/


#include "Editor_configSel.h"
#include "Editor.h"
#include "Editor_graph.h"




Widget top_form;

/*
void
EditorSelectConfig (char *confn)
*/
void
CodaTermSelectConfig (char *confn)
{
  int  i = 0;
  char *currconfig;
  Arg arg[20];
  int ac = 0;
  Widget curr;
  char  temp[128];
  rcNetComp *daq_list[MAX_NUM_COMPS];
  int         num_comps;
  ConfigInfo *configs[EDITOR_MAX_CONFIGS];
  int         num_configs;

  printf("CodaTermSelectConfig: confn >%s<\n",confn);

  currconfig = confn;
  /*XcodaEditorResetGraphCmd();
  delete_everything (0, 0, 0);
  initConfigSel (sw_geometry.draw_area);
  */

  /* tell database handler */
  if (selectConfigTable (currconfig) < 0)
  {
    /*XcodaEditorShowConfigName (0);*/
    sprintf (temp, "Cannot select configuration %s", currconfig);
    printf("%s\n",temp);
    /*pop_error_message (temp, top_form);*/
    return;
  }
  else
  {
    /*XcodaEditorShowConfigName (currconfig);*/
    if (constructRcnetCompsWithConfig (currconfig, daq_list, &num_comps,
				                       configs, &num_configs) == 0)
    {
      printf("num_comps=%d, num_configs=%d\n",num_comps,num_configs);


	  /*
      XcodaEditorConstructGraphFromConfig(&coda_graph, daq_list, num_comps,
					  configs, num_configs);
      (*coda_graph.redisplay)(&coda_graph, sw_geometry.draw_area, 0);
	  */

      /* free all resources */
      for (i = 0; i < num_comps; i++) freeRcNetComp (daq_list[i]);
      for (i = 0; i < num_configs; i++) freeConfigInfo (configs[i]);
    }
    else
	{
      printf("ERROR in constructRcnetCompsWithConfig\n");
	}
  }
}


Widget
CodaTerm(Widget toplevel, int withExit)
{
  char   *dbasehost;
  Arg    args[10];
  int    ac;
  Pixel  bg, fg;

  /*dbasehost = 0;*/
  dbasehost = getenv("MYSQL_HOST");

  /* connect to a database server */
  if (connectToDatabase (dbasehost) == NULL)
  {
    fprintf (stderr, "Cannot connect to a CODA database server\n");
    printf("Cannot connect to a CODA database server\n");
    exit(0);/*return;*/
  }

  return(top_form);
}
