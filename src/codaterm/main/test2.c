/* file_browser.c -- use a ScrolledText object to view the
** contents of arbitrary files chosen by the user from a
** FileSelectionDialog or from a single-line text widget.
*/

#include <X11/Xos.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/FileSB.h>
#include <Xm/MainW.h>
#include <Xm/RowColumn.h>
#include <Xm/LabelG.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

main (int argc, char *argv[])
{
    Widget         top, main_w, rc, text_w, label_w;
    XtAppContext   app;
    XmString       file, open, exit;
    Arg            args[10];
    int            n;

    //XtSetLanguageProc (NULL, NULL, NULL);

    /* initialize toolkit and create toplevel shell */
    top = XtVaOpenApplication (&app, "Demos", NULL, 0, &argc, argv, NULL,
                                sessionShellWidgetClass, NULL);



	/*
    main_w = XmCreateMainWindow (top, "main_w", NULL, 0);
    rc = XmCreateRowColumn (main_w, "work_area", NULL, 0);
	*/

#if 0
	n = 0;
    XtSetArg (args[n], XmNorientation, XmVERTICAL/*XmHORIZONTAL*/); n++;
    XtSetArg (args[n], XmNresizeHeight, True); n++;
    rc = XmCreateRowColumn (top, "work_area", args, n);
#endif


    n = 0;
    rc = XmCreateFrame (top, "work_area", args, n);



	/*
    n = 0;
    //XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    label_w = XmCreateLabel (rc, "Filename:", args, n);
    XtManageChild (label_w);
	*/


    n = 0;
    XtSetArg (args[n], XmNframeChildType, XmFRAME_TITLE_CHILD); n++;
    XtSetArg (args[n], XmNchildVerticalAlignment, XmALIGNMENT_CENTER); n++;
    label_w = XmCreateLabelGadget (rc, "!!! TITLE !!!", args, n);
    XtManageChild (label_w);



    /* Create ScrolledText -- this is work area for the MainWindow */
#if 1
    n = 0;
    XtSetArg (args[n], XmNrows, 12);                     n++;
    XtSetArg (args[n], XmNcolumns, 70);                  n++;
    XtSetArg (args[n], XmNeditMode, XmMULTI_LINE_EDIT);  n++;
    XtSetArg (args[n], XmNtopAttachment, label_w/*XmATTACH_FORM*/); n++;
    text_w = XmCreateScrolledText (rc, "text_w", args, n);
    XtManageChild (text_w);
#endif



    XtManageChild (rc);
    XtRealizeWidget (top);
    XtAppMainLoop (app);
}
