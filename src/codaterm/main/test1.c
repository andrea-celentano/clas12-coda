/********************************************************************
* This example code is from the book:
*
* The X Window System: Programming and Applications with Xt
* Second OSF/Motif Edition
* by
* Douglas Young
* Prentice Hall, 1994
*
* Copyright 1994 by Prentice Hall
* All Rights Reserved
*
* Permission to use, copy, modify, and distribute this software for
* any purpose except publication and without fee is hereby granted,
provided
* that the above copyright notice appear in all copies of the software.
*
*****************************************************************************/

/*******************************************************************
* editor.c: An example Motif interface used to discuss resources
*******************************************************************/
#include <Xm/Xm.h>
#include <Xm/DrawingA.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include <Xm/Form.h>

#include <Xm/Frame.h>

#include <Xm/PanedW.h>


void quitCallback ( Widget w, XtPointer clientData, XtPointer callData );
void yesCallback ( Widget w, XtPointer clientData, XtPointer callData );
void noCallback ( Widget w, XtPointer clientData, XtPointer callData );
void HandleBoardEvents ( Widget w, XtPointer clientData, XEvent *event, Boolean *flag );

Widget myterm[4];
Widget yes;
Widget no;
Arg wargs[1];
Display *display;
int screen;
long fgcolor, bgcolor;
XGCValues gcval;
GC draw;
Window window;
int pointx, pointy;
int FirstPt = TRUE;

void
main ( int argc, char **argv )
{
  Widget shell, canvas, panel, commands, options;
  XtAppContext app;
  int ac;
  Arg arg[20];
  int ii;
    

	/*
	* Initialize Xt.
	*/

/*
	if ( putenv("XENVIRONMENT=Editor.color") < 0)
		printf("can't set XENVIRONMENT\n");
*/

    ac = 0;
	XtSetArg(arg[ac], XmNallowShellResize, True); ac++;
	shell = XtAppInitialize ( &app, "Editor", NULL, 0, &argc, argv, NULL, arg, ac );




    ac = 0;

    //XtSetArg (arg[ac], XmNwidth,  300); ac++;
    //XtSetArg (arg[ac], XmNheight, 300); ac++;

    //XtSetArg (arg[ac], XmNpaneMinimum, 300); ac++; //does not do anything
    //XtSetArg (arg[ac], XmNpaneMaximum, 300); ac++; //does not do anything

    //XtSetArg (arg[ac], XmNrefigureMode, True); ac++; //default, does not do anything

    //XtSetArg (arg[ac], XmNsashHeight, 100); ac++; //height of slider box

    //XtSetArg (arg[ac], XmNallowResize, True); ac++; //does not do anything


	//XtSetArg (arg[ac], XmNresizeToPreferred, True); ac++;


	panel = XtCreateManagedWidget ( "panel", xmPanedWindowWidgetClass, shell, arg, ac );





XtManageChild(panel);



	
	for(ii=0; ii<4; ii++)
	{
	  ac = 0;
      XtSetArg (arg[ac], XmNwidth,  300); ac++;
      XtSetArg (arg[ac], XmNheight, 200); ac++;

      myterm[ii] = XtCreateManagedWidget ( "xterm", xmFrameWidgetClass, panel, arg, ac );
	}


XtManageChildren(myterm, 4);


    /***********************/
	XtRealizeWidget ( shell );
    /***********************/



	//XtVaSetValues(panel, XmNresizeToPreferred, True, NULL ); //do nothing
	//XtVaSetValues(panel, XmCPreferredPaneSize, 100, NULL ); //do nothing

	//XtVaSetValues(panel, XmNpacking, XmPACK_COLUMN, NULL); //do nothing

 	//XtVaSetValues(panel, XmNresizeHeight, 100, NULL); //do nothing

	/*XmNresizeWidth*/





	for(ii=0; ii<4; ii++)
	{

      XtVaSetValues(myterm[ii], XmNallowResize, True, NULL );

      XtVaSetValues(myterm[ii], XmNskipAdjust, True, NULL ); /* disable resizing for this 'myterm' - best I can do for now */

	}



	/*
XmNresizeToPreferred
XmNpreferredPaneSize
	*/

	/* do 'XtSetArg (arg[ac], XmNheight, 200); ac++;' above
	for(ii=0; ii<4; ii++)
	{

      XtVaSetValues(myterm[ii], XmNheight, 50, NULL );

	}
	*/








	//XtVaSetValues(panel, XmNallowResize, False, NULL ); // do nothing

	//XtVaSetValues(panel, XmNskipAdjust, True, NULL ); // do nothing


    //XtUnmapWidget(myterm[0]); // hides

	XtAppMainLoop ( app );
}

