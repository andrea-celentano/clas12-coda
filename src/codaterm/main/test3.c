/* simple_text.c -- Create a minimally configured Text widget */

#include <Xm/Text.h>

main (int argc, char *argv[])
{
    Widget             toplevel;
    XtAppContext       app;
    Widget             text_w;
    Arg                args[2];

    XtSetLanguageProc (NULL, NULL, NULL);
    toplevel = XtVaOpenApplication (&app, "Comment", NULL, 0, &argc, argv, NULL, 
                                    sessionShellWidgetClass, NULL);

    XtSetArg (args[0], XmNvalue, "bla");
    text_w = XmCreateText (toplevel, "text", args, 1);
    XtManageChild (text_w);
    XtRealizeWidget (toplevel);
    XtAppMainLoop (app);
}
