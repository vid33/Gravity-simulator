#include "baseinclude.h"
#include <g++/fstream.h>
#include <ctype.h>
#include <errno.h>
#include <X11/Xos.h>
#include <stdio.h>

/*Global declarations:*/

extern Display* globalDisplay;
Widget detailsShell;
Widget detailsText;
extern Widget detailsButton;
int DETAILS_WINDOW_OPEN = 0;

/*Callback which closes the window.*/
void closeDetails(Widget w, XtPointer clientdata, XtPointer calldata)
{
  /*Set the detailButton to sensitive, and detailswin open to flase.*/
  XtVaSetValues(detailsButton, 
		XmNsensitive, True,
		NULL);
  DETAILS_WINDOW_OPEN = 0;

  XtUnmapWidget(detailsShell);
}

/*The window which holds the details of current animation
position, acceleration or whatever else it should hold.*/

void details(Widget parent, XtPointer clientdata, char *app_class)
{
  Widget main_window, mainForm;
  Widget closeButton;

  /*Set the detailButton to insensitive, and detailswin open to true.*/
  XtVaSetValues(detailsButton, 
		XmNsensitive, False,
		NULL);
  DETAILS_WINDOW_OPEN = 1;

  detailsShell = XtVaAppCreateShell("Details", 
				    app_class,
				    topLevelShellWidgetClass,
				    XtDisplay(parent),
				    XmNtransientFor, parent,
				    XmNheight, 600,
				    XmNwidth, 550,
				    NULL);

  main_window = XtVaCreateWidget("main_window",
				 xmMainWindowWidgetClass, detailsShell, NULL);

  /*Create a form.*/
  mainForm = XtVaCreateWidget ("form",
	     xmFormWidgetClass, main_window, NULL);

  Arg args[15]; int n = 0;

  XtSetArg(args[n], XmNrows, 10); n++;
  XtSetArg(args[n], XmNcolumns, 80); n++;
  XtSetArg(args[n], XmNeditable, False); n++;
  XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_POSITION); n++;
  XtSetArg(args[n], XmNtopPosition, 3); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
  XtSetArg(args[n], XmNleftPosition, 3); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
  XtSetArg(args[n], XmNrightPosition, 97); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_POSITION); n++;
  XtSetArg(args[n], XmNbottomPosition, 90); n++;
  XtSetArg(args[n], XmNbackground, 
	   WhitePixel(globalDisplay, DefaultScreen(globalDisplay)) );n++;
  
  detailsText = XmCreateScrolledText(mainForm, "text_edit", args, n);

  closeButton = XtVaCreateManagedWidget("Close",
		xmPushButtonWidgetClass, mainForm,
	        XmNheight, 40,
	        XmNwidth, 90,
	        XmNleftAttachment, XmATTACH_POSITION,
                XmNleftPosition, 1,	
	        XmNbottomAttachment, XmATTACH_POSITION,
                XmNbottomPosition, 99,
	        NULL);

  XtAddCallback(closeButton, XmNactivateCallback,
		closeDetails, NULL);

  XtManageChild(detailsText);
  XtManageChild(mainForm);
  XtManageChild(main_window);

  XtRealizeWidget(detailsShell);
}







