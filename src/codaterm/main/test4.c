/* text_box.c -- demonstrate simple use of XmNactivateCallback
** for TextField widgets. Create a rowcolumn that has rows of Form
** widgets, each containing a Label and a Text widget. When
** the user presses Return, print the value of the text widget
** and move the focus to the next text widget.
*/

#include <Xm/TextF.h>
#include <Xm/LabelG.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>

char *labels[] = { "Name:", "Address:", "City:", "State:", "Zip:" };

main (int argc, char *argv[])
{
    Widget         toplevel, text_w, form, rowcol, label_w;
    XtAppContext   app;
    int            i;
    void           print_result(Widget, XtPointer, XtPointer);
    Arg            args[8];
    int            n;

    XtSetLanguageProc (NULL, NULL, NULL);
    toplevel = XtVaAppInitialize (&app, "Demos", NULL, 0, &argc, argv, NULL,     
                                    sessionShellWidgetClass, NULL);

    rowcol = XmCreateRowColumn (toplevel, "rowcol", NULL, 0);

    for (i = 0; i < XtNumber (labels); i++) {
        n = 0;
        XtSetArg (args[n], XmNfractionBase, 10);       n++;
        XtSetArg (args[n], XmNnavigationType, XmNONE); n++;
        form = XmCreateForm (rowcol, "form", args, n);

        n = 0;
        XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM);       n++;
        XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM);    n++;
        XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);      n++;
        XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
        XtSetArg (args[n], XmNrightPosition, 3);                   n++;
        XtSetArg (args[n], XmNalignment, XmALIGNMENT_END);         n++;
        XtSetArg (args[n], XmNnavigationType, XmNONE);             n++;
        label_w = XmCreateLabelGadget (form, labels[i], args, n);
        XtManageChild (label_w);

        n = 0;
        XtSetArg (args[n], XmNtraversalOn, True);                 n++;
        XtSetArg (args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
        XtSetArg (args[n], XmNleftPosition, 4);                   n++;
        XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM);    n++;
        XtSetArg (args[n], XmNnavigationType, XmTAB_GROUP);       n++;
        text_w = XmCreateTextField (form, "text_w", args, n);
        XtManageChild (text_w);

        /* When user hits return, print the label+value of text_w */
        XtAddCallback (text_w, XmNactivateCallback, print_result, 
                            (XtPointer) labels[i]);
        XtManageChild (form);
    }

    XtManageChild (rowcol);
    XtRealizeWidget (toplevel);
    XtAppMainLoop (app);
}

/* print_result() -- callback for when the user hits return in the
** TextField widget.
*/

void print_result (Widget text_w, XtPointer client_data,
                        XtPointer call_data)
{
    char *value = XmTextFieldGetString (text_w);
    char *label = (char *) client_data;

    printf ("%s %s\n", label, value);
    XtFree (value);

    XmProcessTraversal (text_w, XmTRAVERSE_NEXT_TAB_GROUP);
}
