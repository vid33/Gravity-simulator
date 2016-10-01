/*
input.c
use: interactive configuration of input
*/

#include "baseinclude.h"
#include <g++/fstream.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h> /*for checking if input file is ok or not*/

/*
The main variables which are needed for the calculation.
*/

double simulationTime;
int stepNumber;
char *inputFile;

/*This holds the label on the unit button. The int make it easier to know
 which unit is current.*/
char *timeUnit = "Seconds"; 
int currentTimeUnit = 0;

/*
The declarations below are needed for putting the data as the arguments
some of the callback functions
*/

char inputfile[100];
char SimulationTime[50];
char StepNumber[50];

/*
The number of variables the user is asked for which are input file,  
time of simulation, and number of steps.
*/

#define NUMITEMS 3

/*The text widgets which prompt for step number simulaiton time and filename.*/
Widget inputText[3];
 
/*
The chars below are to save the values of the user input once the
user closes the input window.
*/

char *inputSimulationTime;
char *inputAnimationTime;
char *inputStepNumber;

/*
Used to check if user input is O.K.
*/

int check = 1;

/*The widgets whose state may need to be changed if the 
input is  O.K.*/
extern Widget calculateTrajectoryButton;
extern Widget animationButton;
extern Widget backAnimationButton;
extern Widget forwardAnimationButton;

/*Characters which will hold the states of the buttons*/

int calculateButtonState, exitButtonState, animationButtonState;

/*The button which generates the configure popup and which is made insensitive
while the popup is open*/

extern Widget configureButton;

/*
NOTE: the main function is at the end. The first functions check
the input etc.
*/

/*
The functions below check if user input is O.K. and gives an error
dialog if it is not. They are probably more extensive than they need
to be, but , without considering the means, at least  this part 
of the program is user-friendly.
*/

void checkInputDouble(Widget w, Widget clientdata, XtPointer calldata)
{ 

/*
Creates an overflow error. The integer retrieved from errno is
used to notice overflow errors coming from user input.
*/

  errno = 0;
  double createInfinityError;
  createInfinityError = strtod("123e1299888898", NULL); /*cool large number*/
  int infinityError = errno; errno = 0;

  int i;
  char E = 'e'; char dot = '.'; char plus = '+'; char minus = '-'; 

  int dotCount = 0; int ECount = 0; int PositionOfE = 0;


  char *local;
  double localcheck;
  double isDigitCheck, isCharacterCheck;

  local = XmTextGetString(clientdata);
  const int buffer =  strlen(local);
  char localstring[buffer];
  strcpy(localstring, local);

/*
This very complicated looking loop checks for any non-digit characters,
and then checks for pluses, minuses and 'E's used in any exponential 
expressions. It also checks whether the decimal point as given by the
user is acceptable.
*/

  for (i = 0; i < buffer; i++ ) {      
      isCharacterCheck = isalpha(localstring[i]);     
      isDigitCheck = isdigit(localstring[i]);     

/*
This causes an input error message, if non-digits excluding 'E', 'plus ',
'minus'  and 'dot'are encountered.
*/

    if ((isDigitCheck == 0) && (localstring[i] != E) && (localstring[i] != dot)&& (localstring[i] != plus) && (localstring[i] != minus)) {
      check = 0;
      break;
    }

/*
The rest of the code in this loop, checks if 'E's 'pluses' 'minuses', and 
'dot' (decimal point) which may be in the inputed expression, make the 
expression acceptable.
*/

    /* For 'E'*/                                        
      if (((localstring[i] == E) && (check == 1)) && (ECount == 0)) { 
	check = 1;
        ECount = ECount + 1; /*Insure there is only one E*/
        PositionOfE = i;
      } 
        
      /*for 'pluses' and 'minuses'*/
      if (((localstring[i] == plus) || (localstring[i] == minus)) && 
	  (check == 1)) {
	if (localstring[i-1] == E) {
            check = 1;
	  }
        else {
            check = 0;
            break;
        }
      } 
      
/* Checks for: 1. the error if the input contains more than 1 decimal 
before the exponent, 2. for a decimal after the exponent, 3. for a 
decimal just before the exponent or at the end of the expression  */

      if (PositionOfE == 0) { 
        if ((localstring[i] == dot) && (localstring[i+1] == E)) {
          check = 0;
          break;
	}
        if ((localstring[i] == dot) && (i == buffer)) {
          check = 0;
          break;
	}
	if (localstring[i] == dot) { 
          dotCount = dotCount + 1;
          if (dotCount == 2) {
            check = 0;
            break;
	  }
        }
      }
      if (PositionOfE != 0) {
	if (localstring[i] == dot) {
          check = 0;
          break;
        }
      }
  }
  
/*
Last bit of checking: involves only the strod error-checking ability
*/

  localcheck = strtod(localstring, NULL);
  int localError = errno;
  if ((localError == infinityError) || (check == 0)){
    check = 0;
  }
  if ((localcheck == 0) || (check == 0)) {
     check = 0;
    }
  else {
    check = 1;
    }
}

/*
The function below, which checks whether the integer data from the user input
is acceptable, is simple compared to the 'checkInputDouble'. All it does is
that it checks if the input is an integer. This also disables the user from
having exponential input, but at the moment I can't be bothered to fix that.
The number of steps is hardly likely to be high enough for exponential notation
to be really useful. Also, no overflow checking is performed since it's
unlikely that a user will type in such a number.
*/

void checkInputInt(Widget w, Widget clientdata, XtPointer calldata)
{
  int i;
  char *local;
  double localcheck;
  double isDigitCheck;
  local = XmTextGetString(clientdata);
  const int buffer =  strlen(local);
  char localstring[buffer];
  strcpy(localstring, local);

  for (i = 0; i < buffer; i++ ) {      
      isDigitCheck = isdigit(localstring[i]);          
     
    if (isDigitCheck == 0) {
      check = 0;
      break;
    }
  }
  
/*
Last bit of checking: involves only the strol error-checking ability
*/

  localcheck = strtol(localstring, NULL, 10);
  if ((localcheck == 0) || (check == 0)) {
     check = 0;
    }
  else {
    check = 1;
    }
}

void checkInputFile(Widget w, Widget clientdata, XtPointer calldata)
{
  char *local;
  local = XmTextGetString(clientdata);
  struct stat s_buf;
  if (check == 0) {
    check = 0;
    }
  else {
    /*check the file via stat. -1 means not writable the second
      thing means it's a directory.*/
    if(( (stat(local, &s_buf) == -1) )
       || ( ((s_buf.st_mode  & S_IFMT) == S_IFDIR) ) ) { 
    check = 0;
    }
      else {
        check = 1;
      }
  }
} 

/*
The function below is the error dialog
*/

void errorDialog(Widget w, XtPointer clientdata, XtPointer calldata)
{
  if (check == 0) {
    Widget ErrorDialog;
    XmString errorMessage;
    errorMessage = XmStringGenerate("The input was not usable",NULL, 
				XmCHARSET_TEXT, NULL);

    char *popupName = "Something's Wrong";

    ErrorDialog = XmCreateInformationDialog(w, NULL, NULL, 0);
    XtVaSetValues(ErrorDialog,
		  XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL,
		  XmNmessageString, errorMessage,
		  XmNallowShellResize, False,
		  NULL);
    
    /*Change the title of the window. If I do it any other way
      the title is of the form "XX_popup", where XX is what I set the title
      to be.*/

    XtVaSetValues(XtParent(ErrorDialog),
		  XmNtitle, popupName,
		  NULL);

    XmStringFree (errorMessage);

    /*Get rid of the cancel and OK buttons*/
    XtUnmanageChild (
		     XmMessageBoxGetChild (ErrorDialog, XmDIALOG_HELP_BUTTON));
    XtUnmanageChild (
		     XmMessageBoxGetChild (ErrorDialog, XmDIALOG_CANCEL_BUTTON));

    XtManageChild(ErrorDialog);
  }
}


/*
The functions below take the user input and store it into the
variables which are needed for the calculations to be performed. This is
done only if the checks are succesfull: thus the if statements.
*/

void saveSimulationTime(Widget w,  Widget clientdata, XtPointer calldata){
  if (check == 1) {
    inputSimulationTime = XmTextGetString(clientdata);
    strcpy(SimulationTime, inputSimulationTime);
    simulationTime = strtod(SimulationTime, NULL);

    /*Put simulation time into seconds.*/
    /*seconds*/
    if (currentTimeUnit == 0) { 
      simulationTime = simulationTime;
    }
    /*hours:*/
    if (currentTimeUnit == 1) {
      simulationTime = simulationTime*60*60;
    }
    /*days*/
    if (currentTimeUnit == 2) {
      simulationTime = simulationTime*60*60*24;
    }  
    /*years*/
    if (currentTimeUnit == 3) {
      simulationTime = simulationTime*60*60*24*365;
    }
    /*centuries*/
    if (currentTimeUnit == 4) {
      simulationTime = simulationTime*60*60*24*365*100;
    }
  }
}

void saveStepNumber(Widget w,  Widget clientdata, XtPointer calldata)
{  if (check == 1) {
  inputStepNumber = XmTextGetString(clientdata); 
  strcpy(StepNumber, inputStepNumber);  
  stepNumber = atoi(StepNumber);
  }
}

void saveInputFile(Widget w,  Widget clientdata, XtPointer calldata)
{ if (check == 1) {
  inputFile = XmTextGetString(clientdata);
  strcpy(inputfile, inputFile);
  }
}

/*
Used to close the input window and after all the saves and checks
have been performed.
*/ 

void close(Widget w, Widget clientdata, XtPointer calldata)
{
  if (check == 1) {
      /*Make the calculate button sensitive*/
      XtVaSetValues (calculateTrajectoryButton,
		     XmNsensitive, True,
		     NULL);
      /*Make the appropriate animation buttons insensitive*/
      XtVaSetValues (animationButton,
		     XmNsensitive, False,
		     NULL);
      XtVaSetValues(forwardAnimationButton,
		    XmNsensitive, False,
		    NULL);
      XtVaSetValues(backAnimationButton,
		    XmNsensitive, False,
		    NULL);


      XtUnmanageChild(clientdata);
  }

check = 1; /*Resets the check value to 1.*/ 
}

/*
Used by the cancel button; closes the input window without doing
anything with the user input. Also disable all the animation
buttons; not really necessary, but otherwise the user would
have said O.K.. Also, if I have more possible inputs in the
future, this will be a way of erasing an imput.
*/

void closeNoCheck(Widget w, Widget clientdata, XtPointer calldata)
{ 
  XtUnmanageChild(clientdata);
}


/*The functions below are for the file browser used to help
  find input files.*/

void exit(Widget w, Widget clientdata, XtPointer calldata)
{ 
  XtUnmanageChild(clientdata);
}

void getFile(Widget w, Widget clientdata, XtPointer calldata)
{
  Widget fileErrorDialog;

  Widget filenameText = XmFileSelectionBoxGetChild(clientdata, 
						     XmDIALOG_TEXT);
  char *local;
  XtVaGetValues(filenameText, 
		XmNvalue, &local, NULL);

  /*Check if the file is readable. More extensive checks will come later.*/ 
  struct stat s_buf;

  /*check the file via stat. -1 means not writable the second
    thing means it's a directory.*/
  if(( (stat(local, &s_buf) == -1) )
     || ( ((s_buf.st_mode  & S_IFMT) == S_IFDIR) ) ) {
    XmString errorMessage;
    errorMessage = XmStringGenerate("Could not readable, or a directory.",
				    NULL, 
				    XmCHARSET_TEXT, NULL);

    char *popupName = "Something's Wrong";

    fileErrorDialog = XmCreateInformationDialog(w, NULL, NULL, 0);
    XtVaSetValues(fileErrorDialog,
		  XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL,
		  XmNmessageString, errorMessage,
		  XmNallowShellResize, False,
		  NULL);
    
    /*Change the title of the window. If I do it any other way
      the title is of the form "XX_popup", where XX is what I set the title
      to be.*/

    XtVaSetValues(XtParent(fileErrorDialog),
		  XmNtitle, popupName,
		  NULL);

    XmStringFree (errorMessage);

    /*Get rid of the cancel and OK buttons*/
    XtUnmanageChild (XmMessageBoxGetChild 
		     (fileErrorDialog, XmDIALOG_HELP_BUTTON));
    XtUnmanageChild (XmMessageBoxGetChild 
		     (fileErrorDialog, XmDIALOG_CANCEL_BUTTON));

    XtManageChild(fileErrorDialog);
  }
  else {
    /*Put the filename into the text box of configure dialog.*/
    XtVaSetValues(inputText[2], XmNvalue, local, NULL);
    XtUnmanageChild(clientdata);
  }
}


void browse(Widget w, XtPointer clientdata, XtPointer calldata)
{
   Widget FileDialog;
 
   char *popupName = "Input File";

   FileDialog = XmCreateFileSelectionDialog(w, NULL, NULL, 0);
   XtVaSetValues(FileDialog,
		 XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL,
		 XmNallowShellResize, False,
		 XmNwidth, 300,
		 XmNheight, 450,
		 NULL);
    
   /*Set the window name*/
   XtVaSetValues(XtParent(FileDialog),
		 XmNtitle, popupName,
		 NULL);

   /*Get rid of the help*/
   XtUnmanageChild (XmFileSelectionBoxGetChild
		    (FileDialog, XmDIALOG_HELP_BUTTON));

   /*Set the callbacks:*/
   XtAddCallback(FileDialog, XmNcancelCallback, exit, FileDialog);
   XtAddCallback(FileDialog, XmNokCallback, getFile, FileDialog);

   /*Set the filter to ~/*.szf*/
   Widget filter = XmFileSelectionBoxGetChild(FileDialog, 
					      XmDIALOG_FILTER_TEXT);
   XtVaSetValues(filter,
		 XmNvalue, "~/*.szf",
		 NULL);
   XmString dirmask; 
   dirmask = XmStringCreateLocalized("~/*.szf");
   XmFileSelectionDoSearch(FileDialog, dirmask);
   XmStringFree(dirmask);

   XtManageChild(FileDialog);
}  


/*The functions below determine the unit of simulationTime.*/

void changeUnit(Widget w, XtPointer clientdata, XtPointer calldata)
{
  char *timeLabelList[5] = 
  {
    "Seconds", 
    "Hours", "Days", "Years", "Centuries" 
  }; 

  if ( (currentTimeUnit+1) == 5) {
    currentTimeUnit = 0;
  }
  else {
    currentTimeUnit = currentTimeUnit + 1;
  }
  
  XmString newLabel;
  newLabel = XmStringCreateLocalized(timeLabelList[currentTimeUnit]);
  
  XtVaSetValues(w,
		XmNlabelString, newLabel,
		NULL);
  XmStringFree(newLabel);

  XtVaSetValues(w,
		XmNwidth, 77, /*Otherwise width changes.*/
		NULL);

  /*Also set the global char variable, so that the value is
    'remembered' after the conf window is closed*/
  timeUnit = timeLabelList[currentTimeUnit];
}


/*The main function containing the GUI of the user input window*/

void configure(Widget parent, XtPointer clientdata, char* app_class)
{
  /*The shell widget which pups up*/
  Widget Conf;

  Widget masterForm;
  Widget currentForm;
  Widget cancelButton;
  Widget okButton;
  Widget browseButton;
  Widget timeUnitButton;
  Widget rowColumn;
  Widget separator;

  int n=0;
  int i, j;

  char *label[NUMITEMS] = 
  {
    "Time of Simulation:", 
    "Step Number:", "Input File:" 
  };
  XmString xarray[NUMITEMS];

  Widget inputLabel[NUMITEMS];

  Conf = XmCreateQuestionDialog(configureButton,
	  NULL,
          NULL, n);

  XtVaSetValues(Conf,
		XmNheight, 210,
		XmNwidth, 415,
		XmNnoResize, True,
		XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL,
		NULL);

  /*Remove all the default widgets:*/
  XtUnmanageChild (XmMessageBoxGetChild (Conf, 
					 XmDIALOG_HELP_BUTTON));
  XtUnmanageChild (XmMessageBoxGetChild (Conf, 
					 XmDIALOG_CANCEL_BUTTON));
  XtUnmanageChild (XmMessageBoxGetChild (Conf, 
					 XmDIALOG_OK_BUTTON));
  XtUnmanageChild (XmMessageBoxGetChild (Conf, 
					 XmDIALOG_SYMBOL_LABEL));
  XtUnmanageChild (XmMessageBoxGetChild (Conf, 
					 XmDIALOG_MESSAGE_LABEL));
  XtUnmanageChild (XmMessageBoxGetChild (Conf, 
					 XmDIALOG_SEPARATOR));

  /*Change the name of the popup window*/
  char *newName = "Configure";
  XtVaSetValues(XtParent(Conf),
		XmNtitle, newName,
		NULL);

  masterForm = XtVaCreateManagedWidget("Form Conf", 
	       xmFormWidgetClass, Conf, 
	       NULL);

  rowColumn = XtVaCreateWidget("labelRC",
              xmRowColumnWidgetClass, masterForm,
	      XmNtopAttachment, XmATTACH_FORM,
	      XmNleftPosition, XmATTACH_FORM,
	      XmNrightPosition, XmATTACH_FORM,
	      XmNbottomAttachment, XmATTACH_POSITION,
	      XmNbottomPosition, 65,
              NULL);

  for (i = 0; i < NUMITEMS; i++)  {
    currentForm = XtVaCreateWidget("form dorm",
                   xmFormWidgetClass, rowColumn,
	           XmNfractionBase, 7,
	           NULL);

    inputLabel[i]= XtVaCreateManagedWidget(label[i],
                   xmLabelWidgetClass, currentForm, 
                   XmNtopAttachment, XmATTACH_FORM,
                   XmNbottomAttachment, XmATTACH_FORM,
                   XmNleftAttachment, XmATTACH_FORM,
                   XmNrightAttachment, XmATTACH_POSITION,
                   XmNrightPosition, 2,
                   XmNalignment, XmALIGNMENT_BEGINNING,
                   NULL);

  /* Here comes an ungraceful part of setting the primary input of each
  text widget (from the values defined at the top of the program)*/
    
    if (i == 0){
      inputText[i] = XtVaCreateManagedWidget("sex",
		     xmTextWidgetClass, currentForm, 
                     XmNvalue, inputSimulationTime,
		     XmNtraversalOn, True,
		     XmNrightAttachment, XmATTACH_POSITION,
		     XmNrightPosition, 5,
		     XmNleftAttachment, XmATTACH_POSITION,
		     XmNleftPosition, 2,  
                     NULL);
      timeUnitButton = XtVaCreateManagedWidget(timeUnit,
		       xmPushButtonWidgetClass, currentForm,
		       XmNtopAttachment , XmATTACH_FORM,
		       XmNbottomAttachment, XmATTACH_FORM,
		       XmNleftAttachment, XmATTACH_WIDGET,
		       XmNleftWidget, inputText[i],
                       XmNleftOffset, 3,
		       XmNwidth, 77,
		       NULL);
		      
    }  
    if (i == 1){
      inputText[i] = XtVaCreateManagedWidget("sex",
                     xmTextWidgetClass, currentForm, 
                     XmNvalue,  inputStepNumber,
       		     XmNtraversalOn, True,
      		     XmNrightAttachment, XmATTACH_POSITION,
		     XmNrightPosition, 5,
		     XmNleftAttachment, XmATTACH_POSITION,
		     XmNleftPosition, 2,  
                     NULL);  
    }
    if (i == 2){
      inputText[i] = XtVaCreateManagedWidget("sex",
                     xmTextWidgetClass, currentForm, 
		     XmNtraversalOn, True,
		     XmNrightAttachment, XmATTACH_POSITION,
		     XmNrightPosition, 5,
		     XmNleftAttachment, XmATTACH_POSITION,
		     XmNleftPosition, 2,  
                     XmNvalue, inputFile,
                     NULL);
  
      browseButton = XtVaCreateManagedWidget("Browse...",
		     xmPushButtonWidgetClass, currentForm,
		     XmNtopAttachment , XmATTACH_FORM,
		     XmNbottomAttachment, XmATTACH_FORM,
		     XmNleftAttachment, XmATTACH_WIDGET,
		     XmNleftWidget, inputText[i],
		     XmNleftOffset, 3,
		     XmNwidth, 77,
		     NULL);
    }        
       
    XtManageChild(currentForm);
  }

  XtManageChild(rowColumn);

  separator = XtVaCreateManagedWidget("Separate",
              xmSeparatorWidgetClass, masterForm,
	      XmNorientation, XmHORIZONTAL,
	      XmNheight, 7,
	      XmNseparatorType, XmSHADOW_ETCHED_OUT,
              XmNleftAttachment, XmATTACH_POSITION,
              XmNleftPosition, 0,
              XmNrightAttachment, XmATTACH_POSITION,
              XmNrightPosition, 100,
              XmNtopAttachment, XmATTACH_POSITION,
              XmNtopPosition, 69,
              NULL);

  okButton = XtVaCreateManagedWidget("OK",
             xmPushButtonWidgetClass, masterForm,
             XmNheight, 40,
             XmNwidth, 70,
             XmNleftAttachment, XmATTACH_POSITION,
             XmNleftPosition, 5,	
             XmNbottomAttachment, XmATTACH_POSITION,
             XmNbottomPosition, 100,
             NULL);

  cancelButton = XtVaCreateManagedWidget("Cancel",
          	 xmPushButtonWidgetClass, masterForm,
                 XmNheight, 40,
                 XmNwidth, 70,
                 XmNleftAttachment, XmATTACH_POSITION,
                 XmNleftPosition, 30,
                 XmNbottomAttachment, XmATTACH_POSITION,
                 XmNbottomPosition, 100,
                 NULL);

  /*The button which causes a file selection widget to pop up.*/
  XtAddCallback(browseButton, XmNactivateCallback,
		browse, NULL);
   
  /*The button which determines the units of the simulation time input*/
  XtAddCallback(timeUnitButton, XmNactivateCallback,
		changeUnit, NULL);

  /*
   When the OK button is pressed, first all the inputs are checked. 
   If they are OK, then all the rest of the callback functions
   are performed. The functions save the user input into     
   the variables the program needs to perform the calculations.
  */

  XtAddCallback(okButton, XmNactivateCallback,
        	checkInputDouble, inputText[0]);
  XtAddCallback(okButton, XmNactivateCallback,
	        checkInputInt, inputText[1]);
  XtAddCallback(okButton, XmNactivateCallback,
		checkInputFile, inputText[2]);

  XtAddCallback(okButton, XmNactivateCallback,
		saveSimulationTime, inputText[0]);
  XtAddCallback(okButton, XmNactivateCallback,
		saveStepNumber, inputText[1]);
  XtAddCallback(okButton, XmNactivateCallback,
		saveInputFile, inputText[2]); 
  XtAddCallback(okButton, XmNactivateCallback,
		errorDialog, NULL);
  XtAddCallback(okButton, XmNactivateCallback,
		close,	Conf);
		 
/*
The callback function for the cancel button
*/
 
  XtAddCallback(cancelButton, XmNactivateCallback,
		closeNoCheck, Conf);   

  XtManageChild(Conf);
  XtRealizeWidget(Conf); 
}
