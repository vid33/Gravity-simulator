#include "baseinclude.h"
#include "file.h"
#include "model.h"
#include "animation.h"

/*Do display busy cursor during longer calculations:*/
#include <X11/cursorfont.h>

#include <g++/iostream.h>
#include <stdlib.h>
#include <math.h>
#include <g++/fstream.h>

/*Widgets whose state may need to be changed after the calculation*/

extern Widget toplevel;
extern Display *globalDisplay;

extern Widget animationButton;
extern Widget backAnimationButton;
extern Widget forwardAnimationButton;
extern Widget calculateTrajectoryButton;
extern Widget skipFramesScale;
extern Widget frameIntervalScale;
extern Widget configureButton;
extern Widget exitButton;
extern Widget skipFramesScale;
extern Widget frameIntervalScale;
extern Widget animationTimeLabel2;

/*To find out what calculations need to be performed.*/
extern Widget highCalculationToggle;
extern Widget fullOutputCheckBox;

/*
Some variables needed by the calculateAndAnimate function, which are 
either extern or determined in the send* functions. 
*/

extern int stepNumber;
extern double simulationTime;
double maxXYZposition;
extern int object_no;
extern char *inputFile;


/*Declarations of the working dialog functions used in datacontrol,
  and their global variables. The functions are at the end of the file*/
void TimeoutCursors(Boolean on);
Boolean CheckForInterrupt();
static Boolean stopped; /*True if user wants to stop the calculation.*/
static Widget workingDialog;

/*A variable which will hold the number of positions recorded in
/tmp/.Sizif*/
int realStepNumber;

/*Variables which determine the state of things.*/
int FULL_OUTPUT;

/*This needs to be declared globaly because it needs to be visible to
a function in animation.c. The reason is to prevent a very strange bug:
this prevents it, but I don't know why. Look at the calculate button
callbacks in gui.c for more detail.*/
int INTERRUPT_SIGNAL = 0; 

double datacontrol(Widget w, char *clientdata, XtPointer calldata)
{
  ofstream vidfile("/tmp/.Sizif");
  int OPTION = atoi(clientdata);
  int UPDATE_COUNT = 0;
  short int workingDialogNecessary;

  /*Reset the realStepNumber variable.*/
  realStepNumber = 0;

  double maxXposition = 0; double maxYposition = 0; 
  double  maxZposition = 0;

  double deltaTime;
  double massTot;

  double mass[object_no];

  double position[object_no][3];
  double force[object_no][3];
  double velocity[object_no][3];
  double acceleration[object_no][3];
  double time;

  double local = double(stepNumber);
  deltaTime = simulationTime/local; 

  /*A variable which will hold the position information just before
    it is written to /tmp/xxxx/ */
  double buffer[3];

  getAllVariables(&mass[0], position, velocity, object_no, inputFile);

  massTot=centerOfMass(&mass[0], position, object_no);

  zeroMomentum(velocity, &mass[0], massTot, object_no);

  /*If full output was selected, the output will have to additionaly
    contain velocities, accelerations, and forces. Each will be held
    in a separate temporary file.*/
  XtVaGetValues(fullOutputCheckBox, 
		XmNset, &FULL_OUTPUT,
		NULL);

 ofstream accelerationFile("/tmp/.Sizif-acceleration");
 ofstream velocityFile("/tmp/.Sizif-velocity");
 ofstream massFile("/tmp/.Sizif-mass");

  if (FULL_OUTPUT ==  1) {    
    /*Write the masses to file already here.*/
    double localBuffer[1];
    for (int i = 0; i < object_no; i++) {
      localBuffer[0] = mass[i];
      massFile.write(localBuffer, sizeof(double));
    } 
  }

  /*If low caluclation is selected, the number of position points
    in the output file will be around ten thousand at the maximum.
    This allows the user to specify a high step number without getting
    huge temporary output files.*/
  int currentSkipStep = 0;
  int SKIP_STEPS = 0;
  Boolean highCalculation;
  double DstepsToSkip; int stepsToSkip;
  XtVaGetValues(highCalculationToggle,
		XmNset, &highCalculation,
		NULL);
  if (!highCalculation) {
    DstepsToSkip = ((double)stepNumber)/15000;
    if (DstepsToSkip > 1.5) {
      stepsToSkip = (int)DstepsToSkip;
      SKIP_STEPS = 1;
    }
  }
    
  /*
    Decide if the calculation is large enough for a
    working dialog to be necessary. The number of calculations
    is proportional to the number_of_steps to the power of
    the number of objects (minus one); I find that things get
    lengthy on my pentium 133 when there are two objects 
    with 100,000 calculation step, so 100,000 is the value
    which should not be exceeded by step_no^oject_no.
  */

  double calculationStepReference = pow(stepNumber, (object_no-1));
  /*The second statement is in the unlikely case that there is
    an overflow in the first number, eg. 100000 steps with 100 objects.*/
  if ((calculationStepReference >= 100000) || (stepNumber >= 100000)) {
    workingDialogNecessary = 1;
  }
  else {
    workingDialogNecessary = 0;
  }

  if (workingDialogNecessary == 1){
    TimeoutCursors(True);
  }

  /*To mark when a new step loop is started. Needed for the skip steps
   thing.*/
  int enteredNewLoop;

  for (int totalStep = 0; totalStep < stepNumber; totalStep++) {
    enteredNewLoop = 1;
    model(force, acceleration, position, velocity, &mass[0], 
	  time, deltaTime, stepNumber, object_no);
    for (int blah = 0; blah < object_no; blah++){
      if ((fabs(position[blah][0])) > maxXposition) 
	maxXposition = fabs(position[blah][0]);
      if ((fabs(position[blah][1])) > maxYposition) 
	maxYposition = fabs(position[blah][1]);
      if ((fabs(position[blah][2])) > maxZposition) 
	maxZposition = fabs(position[blah][2]);

      /*For larger calculations, check for interrupt events etc.*/
      if (workingDialogNecessary == 1) {
	/*Check for events  after every 1000 steps*/
	UPDATE_COUNT = UPDATE_COUNT + 1;
	if (UPDATE_COUNT == 100) {
	  UPDATE_COUNT = 0;
	  if (CheckForInterrupt ()) {
	    INTERRUPT_SIGNAL = 1;
	    break;
	  }
	  
	}
      }
    
    /*Search for maximum distance from the origin.*/
      if ((maxXposition >= maxYposition) && (maxXposition >= maxZposition)){
	maxXYZposition = maxXposition;
      }
      if ((maxYposition >= maxXposition) && (maxYposition >= maxZposition)) {
	maxXYZposition = maxYposition;
      }
      if ((maxZposition >= maxYposition) && (maxZposition >= maxXposition)) {
	maxXYZposition = maxZposition;
      }

      /*
	Write results to a temporary file, which can then be read 
	to do the animation. If full output was selected, write
	the accelerations and velocities to appropriate files as well.
	*/

      if (SKIP_STEPS == 0) {
	for (int i = 0; i < 3; i++){
	  buffer[i] = position[blah][i];
	}
	vidfile.write(buffer, 3*sizeof(double));

	if (FULL_OUTPUT == 1) {
	  for (int i = 0; i < 3; i++) {
	    buffer[i] = acceleration[blah][i];
	  }
	  accelerationFile.write(buffer, 3*sizeof(double));
	  for (int i = 0; i < 3; i++) {
	    buffer[i] = velocity[blah][i];
	  }
	  velocityFile.write(buffer, 3*sizeof(double));
	}
	if (blah == (object_no-1)) {
	  realStepNumber = realStepNumber + 1;
	}
      }

      if (SKIP_STEPS == 1) {
	if (enteredNewLoop == 1) {
	  currentSkipStep = currentSkipStep + 1;
	  enteredNewLoop = 0;
	}
	if (currentSkipStep == stepsToSkip) {
	  for (int i = 0; i < 3; i++) {
	    buffer[i] = position[blah][i];
	  }
	  vidfile.write(buffer, 3*sizeof(double));
	  if (FULL_OUTPUT == 1) {
	    for (int i = 0; i < 3; i++) {
	      buffer[i] = acceleration[blah][i];
	    }
	    accelerationFile.write(buffer, 3*sizeof(double));    
	    for (int i = 0; i < 3; i++) {
	      buffer[i] = velocity[blah][i];
	    }
	    velocityFile.write(buffer, 3*sizeof(double));
	  }
	  if (blah == (object_no-1)) {
	    currentSkipStep = 0;
	    realStepNumber = realStepNumber + 1;
	  }
	}
      }
    }
    if (INTERRUPT_SIGNAL == 1) {
      break;
    }
  }

  if (workingDialogNecessary == 1) {
    TimeoutCursors(False);
  }  

  vidfile.close();
  massFile.close();
  accelerationFile.close();
  velocityFile.close();

  if (FULL_OUTPUT == 0) {
    system("rm -f /tmp/.Sizif-mass");
    system("rm -f /tmp/.Sizif-acceleration");
    system("rm -f /tmp/.Sizif-velocity");
  }

  if (INTERRUPT_SIGNAL == 0) {
    /*Make the appropriate  animation controls sensitive.*/
    XtVaSetValues(animationButton, 
		  XmNsensitive, True,
		  NULL);
    XtVaSetValues(backAnimationButton,
		  XmNsensitive, True,
		  NULL);
    XtVaSetValues(forwardAnimationButton,
		  XmNsensitive, True,
		  NULL);
		  
    /*Make the calculation button control insensitive*/
    XtVaSetValues(calculateTrajectoryButton,
		  XmNsensitive, False,
		  NULL); 

    /*Set the maximum value of the skip frames slide
      to three percent of the step number. Also put the
      current values to minimum and allow input.*/

    int maxSkipFrames = (int)(0.03*realStepNumber);

    XtVaSetValues(skipFramesScale,
		  XmNmaximum, maxSkipFrames,
		  XmNvalue, 0,
		  XmNsensitive, True,
		  NULL);

    XtVaSetValues(frameIntervalScale,
		  XmNvalue, 10,
		  XmNsensitive, True,
		  NULL); 
  /*Make the minimum animation time label display the correct animation
    time.*/  
    XmString newLabel, localString1, localString2; 
    int localInt; char localChar; char normalString[10];
      
    localInt = (int)(realStepNumber*0.01);
    localString2 = XmStringCreate("s", "Tag");
    sprintf(normalString, "%d", localInt);
    localString1 = XmStringCreate(normalString, "Tag");
    newLabel = XmStringConcat(localString1, localString2);
      
    XtVaSetValues(animationTimeLabel2, 
		  XmNlabelString, newLabel,
		  NULL);
		    
    XmStringFree(localString1); XmStringFree(localString2);
    XmStringFree(newLabel);
  }
  else {
    XtVaSetValues(calculateTrajectoryButton,
		  XmNsensitive, True,
		  NULL);

    /*Make the scales insensitive.*/

    XtVaSetValues(skipFramesScale,
		  XmNmaximum, 10,
		  XmNvalue, 0,
		  XmNsensitive, False,
		  NULL);

    XtVaSetValues(frameIntervalScale,
		  XmNvalue, 10,
		  XmNsensitive, False,
		  NULL); 

    /*Reset the animation time label to "empty"*/
    XmString newLabel; 
      
    newLabel = XmStringCreate("empty", "Tag");
      
    XtVaSetValues(animationTimeLabel2, 
		  XmNlabelString, newLabel,
		  NULL);
		    
    XmStringFree(newLabel);
  
    /*Remove the temporary file.*/
    system("rm -f /tmp/.Sizif");
  }

}

/*Below are the functions which create a working dialog for longer
  calculations, refreshes the screen, but allows the user only to
  stop the task, all other actions are ignored. Most of the code
  is courtesy of  The Motif Programming Manual.*/

void stop(Widget dialog, XtPointer clientdata, XtPointer calldata)
{
  stopped = True;
}


Boolean CheckForInterrupt()
{
  Window win = XtWindow(workingDialog);
  XEvent event;

  /*Make sure all requests go to the server.*/
  XFlush(globalDisplay);

  /*Let Motif process all pending exposure events for us.*/
  XmUpdateDisplay(toplevel);

  /*Check the event loop for events in the stop dialog.*/
  while (XCheckMaskEvent (globalDisplay,
			  ButtonPressMask | ButtonReleaseMask 
			  | ButtonMotionMask | PointerMotionMask  
			  | KeyPressMask,
			  &event)) {
    if (event.xany.window == win)
      XtDispatchEvent (&event); 
    else /*Throw event away and sound bell*/
      XBell (globalDisplay, 50);
  }

  return stopped;
}

void TimeoutCursors(Boolean on)
{
  static int locked;
  static Cursor cursor;
  XSetWindowAttributes attrs;
  XEvent event;
  Arg args[5];
  int n;
  XmString str;

  if (on)
    locked++;
  else
    locked--;
  if (locked > 1 || locked == 1 && on == 0)
    return; /*already locked*/

  stopped = False;
  if (!cursor)
    cursor = XCreateFontCursor (globalDisplay, XC_watch);

  /*If on is true turn on watch cursor, otherwise return cursor to 
    normal.*/

  attrs.cursor = on ? cursor : None;

  /*Change main application shell's cursor to timer*/
  XChangeWindowAttributes(globalDisplay, XtWindow(toplevel),
			  CWCursor, &attrs);

  XFlush(globalDisplay);

  /*Put up a working dialog with an interrupt button*/
  if (on) {
    n = 0;
    str = XmStringCreateLocalized("Calculations in progress.");
    XtSetArg(args[n], XmNmessageString, str); n++;
    workingDialog = XmCreateWorkingDialog (toplevel, "busy", args, n);
    XmStringFree(str);
    XtUnmanageChild(XmMessageBoxGetChild(workingDialog, 
						 XmDIALOG_OK_BUTTON));
    XtUnmanageChild(XmMessageBoxGetChild(workingDialog, 
						 XmDIALOG_HELP_BUTTON));
    str = XmStringCreateLocalized("Stop");
    XtVaSetValues(workingDialog, XmNcancelLabelString, str, NULL);

    /*Set the name of the working dialog:*/
    XtVaSetValues(XtParent(workingDialog),
		  XmNtitle, "Working...",
		  NULL);    
    XmStringFree(str);
    XtAddCallback(workingDialog, XmNcancelCallback, stop, NULL);
    XtManageChild(workingDialog);
  }
  else {
    /*Get rid of all the events that may have occured
      during the calculation.*/
    while (XCheckMaskEvent(globalDisplay,
			   ButtonPressMask | ButtonReleaseMask 
			   | ButtonMotionMask | PointerMotionMask 
			   | KeyPressMask, &event)) {
      /*do nothing*/ ;
    }
    XtDestroyWidget (workingDialog);
  }
}







