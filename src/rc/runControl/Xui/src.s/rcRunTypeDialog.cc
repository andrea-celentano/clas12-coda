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
//      Implementation of run type dialog
//
// Author:  
//      Jie Chen
//      CEBAF Data Acquisition Group
//
// Revision History:
//   $Log: rcRunTypeDialog.cc,v $
//   Revision 1.6  1998/06/18 12:20:42  heyes
//   new GUI ready I think
//
//   Revision 1.5  1998/04/08 18:31:33  heyes
//   new look and feel GUI
//
//   Revision 1.4  1997/10/15 16:08:32  heyes
//   embed dbedit, ddmon and codaedit
//
//   Revision 1.3  1997/08/01 18:38:17  heyes
//   nobody will believe this!
//
//   Revision 1.2  1997/06/16 12:26:50  heyes
//   add dbedit and so on
//
//   Revision 1.1.1.1  1996/10/11 13:39:26  chen
//   run control source
//
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*for fchmod*/
#include <sys/types.h>
#include <sys/stat.h>

#include <Xm/Form.h>
#include <Xm/PushBG.h>
/*
#include <Xm/FileSB.h>
#include <Xm/DialogS.h>
*/
#include <rcDbaseHandler.h>
#include <rcRunTypeOption.h>
#include <XcodaErrorDialog.h>
#ifdef USE_CREG
#include <codaRegistry.h>
#endif
#include "rcRunTypeDialog.h"
#include "rcXpmComdButton.h"

#include <XcodaFileSelDialog.h>


rcRunTypeDialog::rcRunTypeDialog (Widget parent,
				  char* name,
				  char* title,
				  rcClientHandler& handler)
:XcodaFormDialog (parent, name, title), netHandler_ (handler),
 option_ (0), errDialog_ (0), fileDialog_ (0),
 ok_ (0), config_ (0)
{
#ifdef _TRACE_OBJECTS
  printf ("                   Create rcRunTypeDialog Class Object\n");
#endif
  // empty
}

rcRunTypeDialog::~rcRunTypeDialog (void)
{
#ifdef _TRACE_OBJECTS
  printf ("                   Delete rcRunTypeDialog Class Object\n");
#endif
  // empty
  // option_ and dialog_ will be destroyed by Xt destroy mechanism
}

void
rcRunTypeDialog::createFormChildren (void)
{
  Arg arg[20];
  int ac = 0;

  ac = 0;  

  XtSetValues (_w, arg, ac);
  // create option menu first
  option_ = new rcRunTypeOption (_w, "runtype","RunType:",netHandler_, this);
  option_->init ();
  
  ac = 0;  

  // create action form
  XtSetArg (arg[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
  XtSetArg (arg[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
  XtSetArg (arg[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
  Widget actionForm = XtCreateManagedWidget ("runTypeActionForm",
				      xmFormWidgetClass, _w, arg, ac);

  XtManageChild (actionForm);

  ac = 0;

  // create push buttons
  rcXpmComdButton *ok     = new rcXpmComdButton(actionForm,"Ok",    NULL,"select run type",NULL,netHandler_);
  rcXpmComdButton *config = new rcXpmComdButton(actionForm,"Config",NULL,"select run config",NULL,netHandler_);


  ok->init();
  config->init();

  
  ok_     = ok->baseWidget();
  config_ = config->baseWidget();


  ac = 0;
  XtSetArg (arg[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
  XtSetArg (arg[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
  XtSetArg (arg[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
  XtSetValues (config_, arg, ac);



  /*sergey: 'Ok' displayed already, do not need following ???
  ac = 0;
  XtSetArg (arg[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
  XtSetArg (arg[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
  XtSetArg (arg[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
  XtSetArg (arg[ac], XmNrightAttachment, XmATTACH_WIDGET); ac++;
  XtSetArg (arg[ac], XmNrightWidget, ok_); ac++;
  XtSetValues (ok_, arg, ac);
  */



  ac = 0;


  // set resource for option menu
  XtSetArg (arg[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
  XtSetArg (arg[ac], XmNtopOffset, 10); ac++;
  XtSetArg (arg[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
  XtSetArg (arg[ac], XmNbottomWidget, actionForm); ac++;
  XtSetArg (arg[ac], XmNbottomOffset, 10); ac++;
  XtSetArg (arg[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
  XtSetArg (arg[ac], XmNleftOffset, 10); ac++;
  XtSetValues (option_->baseWidget(), arg, ac);
  ac = 0;



  // add callbacks
  XtAddCallback (ok_, XmNactivateCallback,
		 (XtCallbackProc)&(rcRunTypeDialog::okCallback),
		 (XtPointer)this);

  XtAddCallback (config_, XmNactivateCallback,
		 (XtCallbackProc)&(rcRunTypeDialog::configCallback),
		 (XtPointer)this);

  // manage all widgets
  option_->manage ();

  // set up default button
  defaultButton (ok_);
}

void
rcRunTypeDialog::startMonitoringRunTypes (void)
{
  option_->startMonitoringRunTypes (); /* sergey: displays 'runTypes' database table here */
}

void
rcRunTypeDialog::endMonitoringRunTypes (void)
{
  option_->endMonitoringRunTypes ();
}

void
rcRunTypeDialog::configure (void)
{
  if (option_->currentRunType () != 0)
  {
    rcClient& client = netHandler_.clientHandler ();
#ifdef _CODA_DEBUG
    printf(">>> rcRunTypeDialog::configure >%s<\n",option_->currentRunType());
#endif
    daqData data ("RCS", "command", option_->currentRunType());
    if (client.sendCmdCallback (DACONFIGURE, data,
		 (rcCallback)&(rcRunTypeDialog::configureCallback),
		 (void *)this) != CODA_SUCCESS)
	{
      reportErrorMsg ("Cannot communication with the RunControl Server\n");
	}
  }
}



void
rcRunTypeDialog::popup (void)
{
  option_->setAllEntries ();
  XcodaFormDialog::popup (); // popup run type configuration dialog
}




void
rcRunTypeDialog::okCallback (Widget, XtPointer data, XmAnyCallbackStruct *)
{
  rcRunTypeDialog* dialog = (rcRunTypeDialog *)data;
  char fname[256], *fn, *ptr;
  int len, ret;

#ifdef _CODA_DEBUG
  printf("rcRunTypeDialog::okCallback\n");fflush(stdout);
  printf("dialog = 0x%08x\n",dialog);fflush(stdout);
#endif

  /* get file name selected in 'Config' button */
  if(dialog->fileDialog_)
  {
    fn = dialog->fileDialog_->selectedFileName();

    if(fn) strcpy(fname,fn);
    else strcpy(fname,"none");

    /* cleanup filename so next popup returns 0 if file not selected */
    dialog->fileDialog_->deleteFileName();

    len = strlen(fname);
#ifdef _CODA_DEBUG
    printf(">>%s<<\n",(char *)&fname[len-4]);
#endif
    if( !strncmp((char *)&fname[len-4],"NONE",4) ) strcpy(fname,"none");
  }
  else
  {
#ifdef _CODA_DEBUG
    printf("rcRunTypeDialog::okCallback 2\n");fflush(stdout);
#endif
    strcpy(fname,"none");
  }

#ifdef _CODA_DEBUG
  printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> FILE1 >%s<\n",fname);
#endif


  /* resolve INCLUDE statements in the file */
  if( (ret=dialog->parseConfigFile(fname)) < 0 )
  {
    printf("ERROR in parseRunConfigFile: ret=%d\n",ret);
    strcpy(fname,"none");
  }
  else /* change extension from 'trg' to 'cnf' */
  {
    ptr = strrchr(fname,'.');
    if(ptr != NULL)
    {
      *ptr = '\0';
      strcat(fname,".cnf");
    }
    else
    {
      printf("ERROR: there is no '.' in file name >%s<\n",fname);
      strcpy(fname,"none");
    }
  }


  
  /*update database*/
  /*dialog->sendUpdateConfFile(fname);*/ /* does nothing !!! */
  dialog->updateConfFile(fname);

  dialog->popdown ();
  dialog->configure (); /*sergey: triggers rcServer activity*/
}




#define STRLEN 256

/* parse run config file *.trg resolving 'include' etc, and create new file in the same directory
with name <fname>.cnf; 'fname' assumed to be full path name */
int
rcRunTypeDialog::parseConfigFile(char *fname)
{
  FILE *fin, *fout, *fd;
  char fnameout[STRLEN];
  char filename[STRLEN], filename_full[STRLEN];
  char str_tmp[STRLEN];
  char keyword[STRLEN];
  char *clonparms;
  char *expid;
  char *ch, *ptr;

  clonparms = getenv("CLON_PARMS");

  if((fin=fopen(fname,"r")) == NULL)
  {
    printf("\nparseConfigFile: Can't open input run config file >%s<\n",fname);
    return(-1);
  }

  strcpy(fnameout,fname);
  ptr = strrchr(fnameout,'.');
  if(ptr != NULL)
  {
    *ptr = '\0';
    strcat(fnameout,".cnf");
  }
  else
  {
    printf("ERROR: there is no '.' in file name >%s<\n",fnameout);
    return(-1);
  }

  if((fout=fopen(fnameout,"w")) == NULL)
  {
    printf("\nparseConfigFile: Can't open output run config file >%s<\n",fnameout);
    return(-2);
  }

  printf("\nparseConfigFile: parsing run config file >%s<\n",fname);
  printf("\nparseConfigFile: writing run config file >%s<\n",fnameout);


  while ((ch = fgets(str_tmp, STRLEN, fin)) != NULL)
  {
    sscanf (str_tmp, "%s %s", keyword, filename);

	/*
    printf("keyword >%s<\n",keyword);
	*/

    /* Start parsing real config inputs */
    if(strcmp(keyword,"include")==0 || strcmp(keyword,"INCLUDE")==0)
    {

      if(strlen(filename)!=0) /* filename specified */
      {
        if ( filename[0]=='/' || (filename[0]=='.' && filename[1]=='/') )
	    {
          sprintf(filename_full, "%s", filename);
	    }
        else
	    {
          sprintf(filename_full, "%s/%s", clonparms, filename);
	    }

        if((fd=fopen(filename_full,"r")) == NULL)
        {
          printf("\nReadConfigFile: Can't open config file >%s<\n",filename_full);
          return(-1);
        }
        else
		{
          printf("\nReadConfigFile: including config file >%s<\n",filename_full);
	    }


        /* write 'include statement commenting it out */
        strcpy(str_tmp,"\n# ");
        strcat(str_tmp,keyword);
        strcat(str_tmp," ");
        strcat(str_tmp,filename);
        strcat(str_tmp,"\n\n");
        if( fputs(str_tmp, fout) == EOF)
	    {
          printf("ERROR writing %s\n",fnameout);
          return(-3);
	    }
        /*else
	    {
          printf("Write >%s<\n",str_tmp);
		}*/

        /* open and copy included file contents */
        while ((ch = fgets(str_tmp, STRLEN, fd)) != NULL)
		{
          if( fputs(str_tmp, fout) == EOF)
	      {
            printf("ERROR writing %s\n",fnameout);
            return(-3);
	      }
          /*else
	      {
            printf("Write >%s<\n",str_tmp);
		  }*/
		}

		fclose(fd);

      }
      else
	  {
        printf("ERROR included file is not specified\n");
        return(-4);
	  }

    }
    else /* just copy string 'as is' */
	{
      if( fputs(str_tmp, fout) == EOF)
	  {
        printf("ERROR writing %s\n",fnameout);
        return(-3);
	  }
      /*else
	  {
        printf("Write >%s<\n",str_tmp);
	  }*/
    }


  } /* end of while */

  fclose(fin);

  if(chmod(fnameout,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH) != 0) /*open file for everybody*/
  {
    printf("ERROR: cannot change mode on output run config file\n");
    printf("ERROR: cannot change mode on output run config file\n");
    printf("ERROR: cannot change mode on output run config file\n");
  }

  fclose(fout);

}




void
rcRunTypeDialog::updateConfFile (char *fname)
{
  rcDbaseHandler* handler = rcDbaseHandler::dbaseHandler ();
  char *confname = option_->currentRunType();
#ifdef _CODA_DEBUG
  printf("CONF1 >%s<\n",confname);
#endif
  handler->connect(getenv("MYSQL_HOST"));
  handler->updateConfFileName(confname,fname);
  handler->close();

  return;
}









/*sergey: 2 funcs

void
rcRunTypeDialog::sendUpdateConfFile (char *fname)
{
printf("> rcRunTypeDialog::sendUpdateConfFile reached\n");fflush(stdout);
  // get client handler
  rcClient& client = netHandler_.clientHandler ();

printf("????????????????????? sendUpdateConfFile >%s<\n",fname);


  daqData data (client.exptname (), "confFile", fname);
  if (client.setValueCallback (data, 
			       (rcCallback)&(rcRunTypeDialog::simpleCallback),
			       (void *)this) != CODA_SUCCESS)
  {
    return;
  }
}



void
rcRunTypeDialog::simpleCallback (int status, void* arg, daqNetData* )
{
printf("> rcRunTypeDialog::simpleCallback reached\n");fflush(stdout);
  rcRunTypeDialog* obj = (rcRunTypeDialog *)arg;
  
  if (status != CODA_SUCCESS)
  {
    obj->reportErrorMsg ("Setting update config file to the server failed !");
printf("rcRunTypeDialog::simpleCallback: Setting update config file to the server failed !\n");
    return;
  }
  else
  {
printf("rcRunTypeDialog::simpleCallback: Setting update config file to the server Ok !\n");
  }
}
*/



































void
rcRunTypeDialog::configCallback (Widget parent, XtPointer data, XmAnyCallbackStruct *)
{
printf("> rcRunTypeDialog::configCallback reached\n");fflush(stdout);

  rcRunTypeDialog* dialog = (rcRunTypeDialog *)data;
  //dialog->popdown ();

  /*sergey: file chooser */

  /*
  if (!(dialog->fileDialog_))
  {
    printf("rcRunTypeDialog::selectConfigFile() created\n");
    dialog->fileDialog_ = new XcodaFileSelDialog(parent,"run config file","aaaaa");
    dialog->fileDialog_->init();
  }
  */

  /*always recreate file dialog, so it picks newly created files in directory*/
  if(dialog->fileDialog_) delete dialog->fileDialog_;
  printf("rcRunTypeDialog::selectConfigFile() created\n");
  dialog->fileDialog_ = new XcodaFileSelDialog(parent,"run config file","aaaaa");
  dialog->fileDialog_->init();



  dialog->fileDialog_->popup();

}


void
rcRunTypeDialog::reportErrorMsg (char* error)
{
  if (!errDialog_) {
    errDialog_ = new XcodaErrorDialog (_w,"runTypeError", "Configuration Error");
    errDialog_->init ();
  }
  if (errDialog_->isMapped ())
    errDialog_->popdown ();
  errDialog_->setMessage (error);
  errDialog_->popup ();
}


void
rcRunTypeDialog::configureCallback (int status, void* arg, daqNetData* data)
{
  rcRunTypeDialog* obj = (rcRunTypeDialog *)arg;

  if (status != CODA_SUCCESS)
    obj->reportErrorMsg ("Configuring a run failed !!!");
}



