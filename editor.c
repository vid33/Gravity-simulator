#include "baseinclude.h"
#include <g++/fstream.h>
#include <ctype.h>
#include <errno.h>
#include <X11/Xos.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#define FILE_SAVE 0
#define FILE_CLOSE 1

#define EDIT_CUT 0
#define EDIT_COPY 1
#define EDIT_PASTE 2
#define EDIT_CLEAR 3

#define SEARCH_FIND_NEXT 0
#define SEARCH_SHOW_ALL 1
#define SEARCH_REPLACE 2
#define SEARCH_CLEAR 3

/*Global declarations:*/

extern Display* globalDisplay;

/*The base of the text edior will be a top level shell.*/
Widget editorShell;

Widget text_edit;
Widget search_text;
Widget replace_text;
Widget text_output;

/*Current input file filename:*/
extern char *inputFile;

/*
The callbacks of the editor:
*/


/*Callback routine for cancel button.*/

void popdown_cb(Widget w, XtPointer clientdata, XtPointer calldata)
{
  XtUnmanageChild(w);
}


/*If a menu item from the file pulldown menu was selected.*/

void file_cb(Widget w, XtPointer clientdata, XtPointer calldata)
{
  char buf[256],  *text;
  struct stat statb;
  long len;
  FILE *fp;
  int reason = (int)clientdata;

  if (reason == FILE_CLOSE) {
    XtUnmapWidget(editorShell);
  }

  XmTextSetString(text_output, NULL);

  if (reason == FILE_SAVE) {
      if (!(fp = fopen (inputFile, "w"))) {
      perror(inputFile);
      sprintf(buf, "Can't save to %s.", inputFile);
      XmTextSetString(text_output, buf);
      return;
    }
    /*saving -- get text from Text widget.*/
    text = XmTextGetString(text_edit);
    len = XmTextGetLastPosition (text_edit);
    /*Write to file and check for error.*/
    if (fwrite (text, sizeof(char), len, fp) != len) {
      strcpy(buf, "Warning: did not write entire file!");
    }
    else {
      /*make sure a newline terminates file.*/
      if (text[len-1] != '\n')
	fputc('\n', fp);
      sprintf(buf, "Saved %ld bytes to %s.", len, inputFile);
    }
  }
  /*Set output message:*/
  XmTextSetString(text_output, buf);

  fclose(fp);
  /*I wanted to XtFree(text);, but then the program core dumps when
   closing the editor.*/
}


/*If a menu from the search pulldown menu was selected.*/

void search_cb(Widget w, XtPointer clientdata, XtPointer calldata)
{
  char *search_pat, *p, *string, *new_pat, buf[256];
  XmTextPosition pos = 0;
  int len, nfound = 0;
  int search_len, pattern_len;
  int reason = (int)clientdata;
  Boolean found = False;

  XmTextSetString(text_output, NULL); /*clear message area*/
  
  if (reason == SEARCH_CLEAR) {
    pos = XmTextGetLastPosition(text_edit);
    XmTextSetHighlight(text_edit, 0, pos, XmHIGHLIGHT_NORMAL);
    return;
  }
  if (!(string = XmTextGetString(text_edit)) || !*string) {
    XmTextSetString(text_output, "No text to search.");
    return;
  }
  if (!(search_pat = XmTextGetString(search_text)) || !*search_pat) {
    XmTextSetString (text_output, "Specify a search pattern.");
    XtFree(string);
   return;
  }

  new_pat = XmTextGetString(replace_text);
  search_len = strlen(search_pat);
  pattern_len = strlen(new_pat);

  if (reason == SEARCH_FIND_NEXT) {
    pos = XmTextGetCursorPosition(text_edit) + 1;
    found = XmTextFindString(text_edit, pos, search_pat, 
			     XmTEXT_FORWARD, &pos);
    if (!found)
      found = XmTextFindString(text_edit, 0, search_pat,
			       XmTEXT_FORWARD, &pos);
    if(found)
      nfound++;
  }
  else{ /*reason == SEARCH_SHOW_ALL || reason == SEARCH_REPLACE*/
    do {
      found = XmTextFindString(text_edit, pos, search_pat,
			       XmTEXT_FORWARD, &pos);
      if (found) {
	nfound++;
	if (reason == SEARCH_SHOW_ALL)
	  XmTextSetHighlight (text_edit, pos, pos + search_len,
			      XmHIGHLIGHT_SELECTED);
	else
	  XmTextReplace (text_edit, pos, pos + search_len, new_pat);
	pos++;
      }
    }
    while (found);
  }

  if (nfound == 0)
    XmTextSetString(text_output, "Pattern not found.");
  else {
    switch (reason) {
    case SEARCH_FIND_NEXT:
      sprintf(buf, "Pattern found at position %ld.", pos);
      XmTextSetInsertionPosition(text_edit, pos);
      break;
    case SEARCH_SHOW_ALL:
      sprintf(buf, "Found %d occurences.", nfound);
      break;
    case SEARCH_REPLACE:
      sprintf (buf, "Made %d replacements.", nfound);
    }
    XmTextSetString(text_output, buf);
  }
  XtFree(string);
  XtFree(search_pat);
  XtFree(new_pat);
}

/*Callback routine for the items in the edit menu.*/
void edit_cb(Widget w, XtPointer clientdata, XtPointer calldata)
{
  Boolean result = True;
  int reason = (int)clientdata;
  XEvent *event = ((XmPushButtonCallbackStruct *) calldata)->event;
  Time when;
  
  XmTextSetString(text_output, NULL);/*clear message area.*/

    if ( (event != NULL) && 
      reason == EDIT_CUT || reason == EDIT_COPY
      || reason == EDIT_CLEAR) {
    switch (event->type) {
       case ButtonRelease:
	 when = event->xbutton.time;
	 break;
       case KeyRelease:
	 when = event->xkey.time;
	 break;
       default:
	 when = CurrentTime;
	 break;
    }
  }

    switch (reason) {
    case EDIT_CUT:
      result = XmTextCut(text_edit, when);
      break;
    case EDIT_COPY:
      result = XmTextCopy(text_edit, when);
      break;
    case EDIT_PASTE:
      result = XmTextPaste(text_edit);
    case EDIT_CLEAR:
      XmTextClearSelection(text_edit, when);
      break;
  }
  
  if (result == False)
    XmTextSetString(text_output, "There is no selection.");
}


/*The main editor function:*/

void editor(Widget parent, XtPointer clientdata, char *app_class)
{
  Widget main_window, menubar, form, search_panel;
  XmString open; XmString save; 
  XmString close; XmString close_acc; 
  XmString file; XmString edit; XmString cut; XmString clear; 
  XmString copy;
  XmString paste; XmString search; XmString next; XmString find;
  XmString replace;

  /*Variables for opening the input file.*/
  char buf[256], *text;
  struct stat statb;
  long len;
  FILE *fp;

  editorShell = XtVaAppCreateShell("Input file editor", 
				   app_class,
				   topLevelShellWidgetClass,
				   XtDisplay(parent),
				   XmNtransientFor, parent,
				   XmNheight, 600,
				   XmNwidth, 550,
				   NULL);

  XmRepTypeInstallTearOffModelConverter();

  main_window = XtVaCreateWidget("main_window",
				 xmMainWindowWidgetClass, editorShell, NULL);
  
  /*Create a simple menu bar containing three menus.*/
  file = XmStringCreateLocalized("File");
  edit = XmStringCreateLocalized("Edit");
  search = XmStringCreateLocalized("Search");
  menubar = XmVaCreateSimpleMenuBar(main_window, "menubar", 
				    XmVaCASCADEBUTTON, file, 'F',
				    XmVaCASCADEBUTTON, edit, 'E',
				    XmVaCASCADEBUTTON, search, 'S',
				    NULL);
  XmStringFree(file);
  XmStringFree(edit);
  XmStringFree(search);
  
  /*First menu is the File menu; the callback is file_cb()*/
  save = XmStringCreateLocalized("Save");
  close = XmStringCreateLocalized("Close");
  close_acc = XmStringCreateLocalized("Ctrl+C");

  XmVaCreateSimplePulldownMenu(menubar, "file_menu", 0, file_cb,
			       XmVaPUSHBUTTON, save, 'S', NULL, NULL,
			       XmVaSEPARATOR,
			       XmVaPUSHBUTTON, close, 'x', "Ctrl<Key>c", 
			       close_acc,
			       NULL);


  XmStringFree(save);
  XmStringFree(close);
  XmStringFree(close_acc);


  /*Create the edit menu. I am not freeing these variables because if I do,
   this little program core dumps when the edit menu is touched. Could it be
   a Motif bug, or something else.*/
  copy = XmStringCreateLocalized("Copy");
  cut = XmStringCreateLocalized("Cut");
  clear = XmStringCreateLocalized("Clear");
  paste = XmStringCreateLocalized("Paste");
  
  XmVaCreateSimplePulldownMenu(menubar, "edit_menu", 1, edit_cb,
			       XmVaPUSHBUTTON, cut, "t", NULL, NULL,
			       XmVaPUSHBUTTON, copy, 'C', NULL, NULL,
			       XmVaPUSHBUTTON, paste, 'P', NULL, NULL,
			       XmVaSEPARATOR,
			       XmVaPUSHBUTTON, clear, 'l', NULL, NULL,
			       NULL);

  /*Create the search Menu.*/
  next = XmStringCreateLocalized("Find Next");
  find = XmStringCreateLocalized("Show All");
  replace = XmStringCreateLocalized("Replace Text");
  
  XmVaCreateSimplePulldownMenu(menubar, "search_menu", 2, search_cb,
			       XmVaPUSHBUTTON, next, 'N', NULL, NULL,
			       XmVaPUSHBUTTON, find, 'A', NULL, NULL,
			       XmVaPUSHBUTTON, replace, 'R', NULL, NULL,
			       XmVaSEPARATOR,
			       XmVaPUSHBUTTON, clear, 'C', NULL, NULL,
			       NULL);
  XmStringFree(next);
  XmStringFree(copy);
  XmStringFree(paste);

  XtManageChild(menubar);

  /*Create a form work area.*/
  form = XtVaCreateWidget ("form",
	 xmFormWidgetClass, main_window, NULL);

  /*Create a horizontal row-column inside the form.*/
  search_panel = XtVaCreateWidget("search_panel",
		 xmRowColumnWidgetClass, form,
		 XmNorientation, XmHORIZONTAL,
		 XmNpacking, XmPACK_TIGHT,
                 XmNtopAttachment, XmATTACH_FORM,
		 XmNleftAttachment, XmATTACH_FORM,
                 XmNrightAttachment, XmATTACH_FORM,
		 NULL);

  /*Create two text field widgets with labels.*/
  XtVaCreateManagedWidget("Search Pattern",
			  xmLabelGadgetClass, search_panel, NULL);
  search_text = XtVaCreateManagedWidget ("search_text",
		xmTextFieldWidgetClass, search_panel, NULL);
  XtVaCreateManagedWidget("       Replace Pattern:",
			  xmLabelGadgetClass, search_panel, NULL);
  replace_text = XtVaCreateManagedWidget("replace_text",
                 xmTextFieldWidgetClass, search_panel, NULL);
  XtManageChild(search_panel);

  text_output = XtVaCreateManagedWidget("text_output",
	        xmTextFieldWidgetClass, form,
		XmNeditable, False,
                XmNcursorPositionVisible, False,
                XmNshadowThickness, 0,
		XmNleftAttachment, XmATTACH_FORM,
                XmNrightAttachment, XmATTACH_FORM,
	        XmNbottomAttachment, XmATTACH_FORM,
		NULL);

  Arg args[9]; int n = 0;

  XtSetArg(args[n], XmNrows, 10); n++;
  XtSetArg(args[n], XmNcolumns, 80); n++;
  XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNtopWidget, search_panel); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNbottomWidget, text_output); n++;
  XtSetArg(args[n], XmNbackground, 
	   WhitePixel(globalDisplay, DefaultScreen(globalDisplay)) );n++;
  
  text_edit = XmCreateScrolledText(form, "text_edit", args, n);

  XtManageChild(text_edit);
  XtManageChild(form);
  XtManageChild(main_window);

  /*Open current input file.*/
    /*make sure the file is a regular text file and open it.*/
    if ( (stat(inputFile, &statb) == -1) || 
	 (statb.st_mode & S_IFMT) != S_IFREG ||
	 (!(fp = fopen (inputFile, "r"))) ) {
      perror(inputFile);
      sprintf (buf, "Can't read %s.", inputFile);
      XmTextSetString(text_output, buf);
      XtFree(inputFile);
      return;
    }

    /*Put the contents of the file in the text widget by allocating
      enough space for the entire filem reading the file into the space,
      and using XmTextSetString to show the file.*/
    len = statb.st_size;
    if ( !(text = XtMalloc ((unsigned)(len+1))) )
      sprintf (buf, "%s: XtMalloc(%ld) failed", len, inputFile);
    else {
      if (fread (text, sizeof(char), len, fp) != len)
	sprintf (buf, "Warning: did not read entire file!");
      else
	sprintf (buf, "Loaded %ld bytes from %s.", len, inputFile);
      text[len] = 0; /*NULL-terminate*/
      XmTextSetString (text_edit, text);
    }
  

  XmTextSetString (text_output, buf); /*purge output message*/

  /*free all allocated space.*/
  XtFree(text);
  fclose(fp);

  XtRealizeWidget(editorShell);

}






