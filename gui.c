#include "baseinclude.h"
#include "animation.h"
#include "datacontrol.h"
#include "details.h"
#include "editor.h"
#include "input.h"
#include "file.h"

/*The widgets of the main window GUI*/
Widget topForm;

Widget animationArea;
Widget topLabel;
Widget sizifLabel;
Widget animationTimeLabel1, animationTimeLabel2;

Widget animationFrame;
Widget topAnimationFrame;
Widget animationForm;
Widget animationControlsForm;
Widget animationControlsFrame;
Widget toggleForm;
Widget toggleFrame;
Widget fileFrame;
Widget fileForm;

Widget exitButton;
Widget configureButton;		
Widget editButton;
Widget addFilesButton;
Widget calculateTrajectoryButton;
Widget animationButton;
Widget pauseAnimationButton;
Widget stopAnimationButton;
Widget backAnimationButton;
Widget forwardAnimationButton;
Widget separator1;
Widget rotateButtonL, rotateButtonR, rotateButtonU, 
  rotateButtonD, rotateButtonC;
Widget translateButtonL, translateButtonR, translateButtonU, 
  translateButtonD, translateButtonC;
Widget zoomInButton, zoomDefaultButton, zoomOutButton,
  zoomInInButton, zoomOutOutButton;
Widget detailsButton;

Widget skipFramesScale;
Widget frameIntervalScale;

Widget constantsRadioBox;
Widget GToggle;
Widget mueToggle;

Widget calculationRadioBox;
Widget highCalculationToggle;
Widget lowCalculationToggle;
Widget fullOutputCheckBox;

Widget fileOptionMenu;

extern Widget toplevel;
extern Widget top;

/*
The main variables which are needed for the calculation.
*/

extern double simulationTime;
extern int stepNumber;
extern int realStepNumber;
extern char *inputFile;
int FILECHANGED;
int MAX_INTERVAL_BETWEEN_FRAMES = 100; /*ms*/
int MAX_FRAMES_SKIPPED = 10;

/*
The declarations below are needed for putting the data as the arguments
some of the callback functions
*/

extern char inputfile[100];
extern char SimulationTime[50];
extern char StepNumber[50];

/*
NOTE: The main gui function is at the end. The first functions
quit the program.
*/


/*
These two function are used to quit the program
*/

void reallyQuit(Widget w, XtPointer clientdata, XtPointer calldata)
{
/*Clean up: remove the temporary files.*/
system("rm -f /tmp/.Sizif");
system("rm -f /tmp/.Sizif-mass");
system("rm -f /tmp/.Sizif-velocity");
system("rm -f /tmp/.Sizif-acceleration");
exit(0);
}

/*
This is the 'are you sure you want to quit' function which is called
before the 'really quit' function is called
*/

void bye(Widget w, XtPointer clientdata, XtPointer calldata)
{
  static Widget quitDlg = NULL;
  static char *msgstr = "Are you sure you want to quit?";
 
  quitDlg = XmCreateQuestionDialog(w, "R U Sure?", NULL, 0);
  XtVaSetValues(quitDlg, 
		XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL,
		XtVaTypedArg, XmNmessageString, XmRString,
		msgstr, 
		strlen(msgstr), 
		NULL);

  /*Set the title of the window; if I do it any other way,
    Motif keeps the stupid "_popup" at the end of the title*/
  XtVaSetValues(XtParent(quitDlg),
		XmNtitle, "Really quit?",
		NULL);

  /*
  Get rid of the 'help' button which is on the default Question
  dialog
  */

  XtUnmanageChild(XmMessageBoxGetChild(quitDlg, XmDIALOG_HELP_BUTTON));

  XtAddCallback(quitDlg, XmNokCallback, reallyQuit, NULL);
  XtManageChild(quitDlg);
}


/*A function which changes the minimum animation time label.*/

void changeAnimationTimeLabel(Widget w, XtPointer clientdata, 
			      XtPointer calldata)
{
  /*Get the values from the scales*/
  int localSkipFrames, localInterval;

  XmScaleGetValue(skipFramesScale, &localSkipFrames);
  XmScaleGetValue(frameIntervalScale, &localInterval);

  XmString newLabel, localString1, localString2; 
  int localAnimTime, realAnimationStepNumber; 
  char localChar; char normalString[10];

  realAnimationStepNumber = (int)( ((double)realStepNumber)/
			((double)(localSkipFrames+1)) );

  localAnimTime = (int)(0.001*localInterval*realAnimationStepNumber);

  localString2 = XmStringCreateLocalized("s");
  sprintf(normalString, "%d", localAnimTime);
  localString1 = XmStringCreateLocalized(normalString);
  newLabel = XmStringConcat(localString1, localString2);
      
  XtVaSetValues(animationTimeLabel2, 
		XmNlabelString, newLabel,
		NULL);
		    
  XmStringFree(localString1); XmStringFree(localString2);
  XmStringFree(newLabel);
}


/*Callback when an item from the file options menu is selected*/

void option_callback(Widget menu_item, 
		     XtPointer clientdata, XtPointer calldata)
{
  int item_no = (int) clientdata;
}


/*The function in which the gui of the main window is determined.*/

void gui()
{  
  /* For the logo */

  Pixmap sizif;
  Pixel cur_color;
  char pixmapName[15] = "Sizif-blur.xpm";

  /*
    The characters which are send to the datacontrol function, and
    which determine how the datacontrol function behaves
    */
  char *CALCULATE = "1";
  char *WRITETOFILE = "2";

  /*The form which holds everything else:*/
  topForm = XtVaCreateManagedWidget("Form 1", 
	  xmFormWidgetClass, top,      
	  NULL);

  /*First I'll do the cool logo and the main label:*/

  /*Get the logo Pixmap*/ 
  sizif = XmGetPixmap(XtScreen (toplevel), pixmapName, 
          BlackPixelOfScreen (XtScreen (top)),
          WhitePixelOfScreen (XtScreen (top)));

  /*The code sets the font for the main label*/
  topLabel = XtVaCreateManagedWidget("Sizif 0.22",
	     xmLabelWidgetClass, topForm,
             XmNalignment, XmALIGNMENT_END,
             XmNleftAttachment, XmATTACH_POSITION,
             XmNleftPosition, 5,
             XmNrightAttachment, XmATTACH_POSITION,
             XmNrightPosition, 50,
             XmNtopAttachment, XmATTACH_POSITION,
             XmNtopPosition, 5,
             NULL);

  /*  The code below changes the font and size of the 
  main label.  */
  XFontStruct *font=NULL;
  XmFontList fontlist=NULL;
  char *namestring = NULL;
  namestring="-adobe-utopia-*-i-normal-*-50-*-*-*-*-*-*-*";
  font=XLoadQueryFont(XtDisplay(topLabel), namestring);
  fontlist=XmFontListCreate(font, XmSTRING_DEFAULT_CHARSET);
  XtVaSetValues(topLabel, XmNfontList,fontlist, NULL);

  /*  Manages the pixmap label*/
  sizifLabel = XtVaCreateManagedWidget("Sizif",
               xmLabelWidgetClass, topForm,
               XmNalignment, XmALIGNMENT_END,
               XmNleftAttachment, XmATTACH_FORM,
               XmNrightAttachment, XmATTACH_POSITION,
               XmNrightPosition, 95,
               XmNtopAttachment, XmATTACH_POSITION,
               XmNtopPosition, 5,
               XmNlabelType, XmPIXMAP,
               XmNlabelPixmap, sizif,
               NULL);

  /*Now do all the secondary forms and frames:*/


  /*A frame which goes around the animation window and the buttons
    which control the animation*/
  topAnimationFrame = XtVaCreateManagedWidget("animation Frame",
                   xmFrameWidgetClass, topForm,
                   XmNshadowType, XmSHADOW_OUT,
	           XmNleftAttachment, XmATTACH_POSITION,
                   XmNleftPosition, 3,
                   XmNrightAttachment, XmATTACH_POSITION,
                   XmNrightPosition, 55,
                   XmNtopAttachment, XmATTACH_POSITION,
                   XmNtopPosition, 25,
	           XmNbottomAttachment, XmATTACH_POSITION,
                   XmNbottomPosition, 80,
                   NULL); 

  /*A form inside the frame*/
  animationForm = XtVaCreateManagedWidget("animation form",
		   xmFormWidgetClass, topAnimationFrame,
		   XmNleftAttachment, XmATTACH_POSITION,
                   XmNleftPosition, 5,
                   XmNrightAttachment, XmATTACH_POSITION,
                   XmNrightPosition, 95,
                   XmNtopAttachment, XmATTACH_POSITION,
                   XmNtopPosition, 5,
	           XmNbottomAttachment, XmATTACH_POSITION,
                   XmNbottomPosition, 95,
		   NULL);

  /*The drawing area for animation will be held in this frame:*/
  animationFrame = XtVaCreateManagedWidget("animation Frame",
                   xmFrameWidgetClass, animationForm,
                   XmNshadowType, XmSHADOW_ETCHED_IN,
	           XmNleftAttachment, XmATTACH_POSITION,
                   XmNleftPosition, 7,
                   XmNrightAttachment, XmATTACH_POSITION,
                   XmNrightPosition, 93,
                   XmNtopAttachment, XmATTACH_POSITION,
                   XmNtopPosition, 7,
	           XmNbottomAttachment, XmATTACH_POSITION,
                   XmNbottomPosition, 93,
                   NULL);

  /*The frame and for which will hold the input file information.*/
 fileFrame = XtVaCreateManagedWidget("toggle Form",
		xmFrameWidgetClass, topForm,
		XmNshadowType, XmSHADOW_ETCHED_IN,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNtopPosition, 25,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 60,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNrightPosition, 97,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNbottomPosition, 42,
		NULL);
  
  fileForm = XtVaCreateManagedWidget("toggle form",
	       xmFormWidgetClass, fileFrame,
	       NULL);

  /*The frame and form which will hold th toggle controls: high calculation,
   low calculation, high graphics, low graphics.*/
  toggleFrame = XtVaCreateManagedWidget("toggle Form",
		xmFrameWidgetClass, topForm,
		XmNshadowType, XmSHADOW_ETCHED_IN,
		XmNtopAttachment, XmATTACH_POSITION,
		XmNtopPosition, 44,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 60,
		XmNrightAttachment, XmATTACH_POSITION,
		XmNrightPosition, 97,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNbottomPosition, 61,
		NULL);
  
  toggleForm = XtVaCreateManagedWidget("toggle form",
	       xmFormWidgetClass, toggleFrame,
	       NULL);

  animationControlsFrame = XtVaCreateManagedWidget("controls frame", 
                 xmFrameWidgetClass, topForm, 
                 XmNshadowType, XmSHADOW_ETCHED_IN,
		 XmNtopAttachment, XmATTACH_POSITION,
                 XmNtopPosition, 63,
		 XmNleftAttachment, XmATTACH_POSITION,
		 XmNleftPosition, 60,
		 XmNrightAttachment, XmATTACH_POSITION,
		 XmNrightPosition, 97,
		 XmNbottomAttachment, XmATTACH_POSITION,
		 XmNbottomPosition, 80,
		 NULL);

  animationControlsForm =  XtVaCreateManagedWidget("animation Form",
			   xmFormWidgetClass, animationControlsFrame,
                           NULL);


  /*Do all the controls:*/

  /*File list, and file controls.*/
  XmString optionMenuLabel;
  optionMenuLabel = XmStringCreateLocalized("Current file:");
  XmString file1; XmString file2;
  file1 = XmStringCreateLocalized("superfile");
  file2 = XmStringCreateLocalized("file2");
 
  fileOptionMenu = XmVaCreateSimpleOptionMenu(fileForm, "File option",
		   optionMenuLabel, 'D', 0, option_callback,
		   XmVaPUSHBUTTON, file1, 'L', NULL, NULL,
		   XmVaPUSHBUTTON, file2, 'S', NULL, NULL,
		   XmNleftAttachment, XmATTACH_POSITION,
		   XmNleftPosition, 5,
		   XmNtopAttachment, XmATTACH_POSITION,
		   XmNtopPosition, 5,
		   NULL);

  XmStringFree(file1); XmStringFree(file2);
  XmStringFree(optionMenuLabel);
  
  XtManageChild(fileOptionMenu);

  /*The radio boxes: full calculation, high/low graphics*/
  constantsRadioBox = XmCreateRadioBox(toggleForm, "Radio box", NULL, 0);
  XtVaSetValues(constantsRadioBox, 
		XmNtopAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_FORM,
		XmNrightAttachment, XmATTACH_POSITION,
	        XmNrightPosition, 45,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNbottomPosition, 60,
		NULL);
  GToggle = XtVaCreateManagedWidget("G",
		      xmToggleButtonGadgetClass, constantsRadioBox,
		      XmNspacing, 3,
		      XmNset, True,
		      NULL);
  /*Initially high graphics is on.*/
  mueToggle = XtVaCreateManagedWidget("m", /*equal to mue in font below.*/
	      xmToggleButtonGadgetClass, constantsRadioBox,
	      XmNspacing, 3,
	      NULL);

  /*Change the font of the mue Toggle to get the greek letter mue.*/
  XFontStruct *mueFont=NULL;
  XmFontList mueFontlist=NULL;
  char *mueNamestring = NULL;
  mueNamestring="-adobe-symbol-*-*-*-*-*-140-*-*-*-*-*-*";
  mueFont=XLoadQueryFont(XtDisplay(top), mueNamestring);
  mueFontlist=XmFontListCreate(mueFont, XmSTRING_DEFAULT_CHARSET);
  XtVaSetValues(mueToggle, XmNfontList, mueFontlist, NULL);
 
  XtManageChild(constantsRadioBox);

  calculationRadioBox = XmCreateRadioBox(toggleForm, "Radio box", NULL, 0);
  XtVaSetValues(calculationRadioBox,
		XmNtopAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 50,
		XmNrightAttachment, XmATTACH_FORM,
		XmNbottomAttachment, XmATTACH_POSITION,
		XmNbottomPosition, 60,
		NULL);

  /*Initially the lowCalToggle is on.*/
  lowCalculationToggle = XtVaCreateManagedWidget("Low Calculation",
			 xmToggleButtonGadgetClass, calculationRadioBox,
			 XmNspacing, 3,
			 XmNset, True,
			 NULL);
  highCalculationToggle = XtVaCreateManagedWidget("High Calculation",
			  xmToggleButtonGadgetClass, calculationRadioBox,
			  XmNspacing, 3,
			  NULL);
  fullOutputCheckBox = XtVaCreateManagedWidget("Full Output",
		       xmToggleButtonGadgetClass, toggleForm,
		       XmNtopAttachment, XmATTACH_POSITION,
		       XmNtopPosition, 62,
		       XmNleftAttachment, XmATTACH_POSITION,
		       XmNleftPosition, 50,
		       NULL);
  XtManageChild(calculationRadioBox);

  /*Details button.*/
  detailsButton = XtVaCreateManagedWidget("Details",
		  xmPushButtonWidgetClass, toggleForm,
		  XmNtopAttachment, XmATTACH_POSITION,
		  XmNtopPosition, 62,
		  XmNleftAttachment, XmATTACH_POSITION,
		  XmNleftPosition, 3,
		  XmNheight, 30,
		  XmNwidth, 100,
		  NULL);

  /*A slide which controls the number of frames which the 
    program will (try) to display per second during animation*/
  frameIntervalScale = XtVaCreateManagedWidget("controls Slide",
	               xmScaleWidgetClass, animationControlsForm,
	               XtVaTypedArg, XmNtitleString,
	               XmRString, 
	               "Interval\nbetween\nframes\n(ms)", 30,
	               XmNmaximum, MAX_INTERVAL_BETWEEN_FRAMES,
	               XmNminimum, 10,
	               XmNvalue, 10,
	               XmNdecimalPoints, 0,
	               XmNshowValue, True,
	               XmNorientation, XmVERTICAL,
	               XmNbottomAttachment, XmATTACH_POSITION,
	               XmNbottomPosition, 70,
	               XmNleftAttachment, XmATTACH_POSITION,
	               XmNleftPosition, 3,
	               XmNtopAttachment, XmATTACH_POSITION,
	               XmNtopPosition, 3,
      	               NULL);
		    
  /*The slide which prompts for the number of calculated frames
    which should be skipped between animated frames*/
  
  skipFramesScale = XtVaCreateManagedWidget("controls Slide",
		    xmScaleWidgetClass, animationControlsForm,
		    XtVaTypedArg, XmNtitleString,
		    XmRString, 
		    "Skip\nframes", 15,
		    XmNmaximum, MAX_FRAMES_SKIPPED,
		    XmNminimum, 0,
		    XmNvalue, 0,
		    XmNdecimalPoints, 0,
		    XmNshowValue, True,
		    XmNorientation, XmVERTICAL,
		    XmNtopAttachment, XmATTACH_POSITION,
		    XmNtopPosition, 3,
		    XmNbottomAttachment, XmATTACH_POSITION,
		    XmNbottomPosition, 70,
		    XmNleftAttachment, XmATTACH_WIDGET,
		    XmNleftWidget, frameIntervalScale,
		    NULL);
		       
  /*Create scale tick marks for both slides*/
  
  for (int i = 1; i < 10; i++) {
    if (i == 1) {
    XtVaCreateManagedWidget("100 -", xmLabelGadgetClass, 
			    frameIntervalScale, NULL);
    }
    if (i == 9) {
      XtVaCreateManagedWidget("10 -", xmLabelGadgetClass,
			      frameIntervalScale, NULL);
    }
    else {
      XtVaCreateManagedWidget("   -", xmLabelGadgetClass,
			      frameIntervalScale, NULL);
    }
  }

  for (int i = 0; i < 10; i++) {
    if (i == 0) {
    XtVaCreateManagedWidget("3% - ", xmLabelGadgetClass, 
			    skipFramesScale, NULL);
    }
    if (i == 9) {
      XtVaCreateManagedWidget("0% - ", xmLabelGadgetClass,
			      skipFramesScale, NULL);
    }
    else {
      XtVaCreateManagedWidget("   - ", xmLabelGadgetClass,
			      skipFramesScale, NULL);
    }
  }

  /*Now, after creating the tick marks, make the scales insensitive.*/
  XtVaSetValues(frameIntervalScale, 
	       XmNsensitive, False,
	       NULL);
  XtVaSetValues(skipFramesScale, 
	       XmNsensitive, False,
	       NULL);

  /*Create a label which will hold the animation time*/

  animationTimeLabel1 = XtVaCreateManagedWidget(
		       "Minimum animation time: ",
                       xmLabelWidgetClass, animationControlsForm,
		       XmNbottomAttachment, XmATTACH_POSITION,
		       XmNbottomPosition, 97,
		       XmNleftAttachment, XmATTACH_POSITION,
		       XmNleftPosition, 3, 
		       NULL);

  animationTimeLabel2 = XtVaCreateManagedWidget("empty",
			xmLabelWidgetClass, animationControlsForm,
			XmNbottomAttachment, XmATTACH_POSITION,
			XmNbottomPosition, 97,
			XmNleftAttachment, XmATTACH_WIDGET,
			XmNleftWidget, animationTimeLabel1,
			NULL);
		       
  /*NOTE: animationArea below is not managed. The reason is that its 
    bit_gravity has to be set to ForgetGravity to generate expose events 
    when the window is made smaller. This has to be done after the widget 
    has been realized, and befor it is managed. This is done in main();*/

  /*The drawing widget where the animation will happen:*/
  animationArea = XtVaCreateWidget("animationArea",
                  xmDrawingAreaWidgetClass, animationFrame,
		  XmNleftAttachment, XmATTACH_POSITION,
	          NULL);

  
  stopAnimationButton = XtVaCreateManagedWidget("S",
		        xmPushButtonWidgetClass, animationForm,
		        XmNtopAttachment, XmATTACH_FORM,
		        XmNleftAttachment, XmATTACH_FORM,
	                XmNbottomAttachment, XmATTACH_POSITION,
		        XmNbottomPosition, 7,
		        XmNrightAttachment, XmATTACH_POSITION,
		        XmNrightPosition, 7,
		        XmNsensitive, False,
		        NULL);

  animationButton = XtVaCreateManagedWidget("A",
		    xmPushButtonWidgetClass, animationForm,
		    XmNtopAttachment, XmATTACH_FORM,
		    XmNrightAttachment, XmATTACH_POSITION,
		    XmNrightPosition, 14,
		    XmNleftAttachment, XmATTACH_WIDGET,
		    XmNleftWidget, stopAnimationButton,
		    XmNbottomAttachment, XmATTACH_POSITION,
		    XmNbottomPosition, 7, 
		    XmNsensitive, False,
		    NULL);

  forwardAnimationButton = XtVaCreateManagedWidget("",
		           xmPushButtonWidgetClass, animationForm,
		           XmNtopAttachment, XmATTACH_FORM,
		           XmNrightAttachment, XmATTACH_POSITION,
			   XmNrightPosition, 21,
		           XmNleftAttachment, XmATTACH_POSITION,
		           XmNleftPosition, 14,
		           XmNbottomAttachment, XmATTACH_POSITION,
	                   XmNbottomPosition, 7, 
	                   XmNsensitive, False,
	                   NULL);

  pauseAnimationButton = XtVaCreateManagedWidget("P",
		         xmPushButtonWidgetClass, animationForm,
		         XmNleftAttachment, XmATTACH_FORM,
		         XmNbottomAttachment, XmATTACH_POSITION,
			 XmNbottomPosition, 14,
		         XmNtopAttachment, XmATTACH_POSITION,
		         XmNtopPosition, 7,
		         XmNrightAttachment, XmATTACH_POSITION,
		         XmNrightPosition, 7,
		         XmNsensitive, False,
		         NULL);

  backAnimationButton = XtVaCreateManagedWidget("",
		      xmPushButtonWidgetClass, animationForm,
		      XmNleftAttachment, XmATTACH_FORM,
		      XmNbottomAttachment, XmATTACH_POSITION,
		      XmNbottomPosition, 21,
		      XmNtopAttachment, XmATTACH_POSITION,
		      XmNtopPosition, 14,
		      XmNrightAttachment, XmATTACH_POSITION,
		      XmNrightPosition, 7,
		      XmNsensitive, False,
		      NULL);

  zoomDefaultButton = XtVaCreateManagedWidget("Z",
		      xmPushButtonWidgetClass, animationForm,
		      XmNtopAttachment, XmATTACH_FORM,
		      XmNrightAttachment, XmATTACH_FORM,
	              XmNbottomAttachment, XmATTACH_POSITION,
		      XmNbottomPosition, 7,
		      XmNleftAttachment, XmATTACH_POSITION,
		      XmNleftPosition, 93,
		      NULL);

  zoomOutButton = XtVaCreateManagedWidget("",
		  xmPushButtonWidgetClass, animationForm,
		  XmNtopAttachment, XmATTACH_FORM,
		  XmNleftAttachment, XmATTACH_POSITION,
		  XmNleftPosition, 86,
		  XmNrightAttachment, XmATTACH_POSITION,
		  XmNrightPosition, 93,
		  XmNbottomAttachment, XmATTACH_POSITION,
		  XmNbottomPosition, 7, 
		  NULL);

  zoomOutOutButton = XtVaCreateManagedWidget("",
		     xmPushButtonWidgetClass, animationForm,
		     XmNtopAttachment, XmATTACH_FORM,
		     XmNleftAttachment, XmATTACH_POSITION,
		     XmNleftPosition, 79,
		     XmNrightAttachment, XmATTACH_POSITION,
		     XmNrightPosition, 86,
		     XmNbottomAttachment, XmATTACH_POSITION,
		     XmNbottomPosition, 7, 
		     NULL);

  zoomInButton = XtVaCreateManagedWidget("",
		 xmPushButtonWidgetClass, animationForm,
		 XmNrightAttachment, XmATTACH_FORM,
	         XmNbottomAttachment, XmATTACH_POSITION,
	         XmNbottomPosition, 14,
	         XmNtopAttachment, XmATTACH_POSITION,
	         XmNtopPosition, 7,
	         XmNleftAttachment, XmATTACH_POSITION,
	         XmNleftPosition, 93,
  	         NULL);

  zoomInInButton = XtVaCreateManagedWidget("",
		   xmPushButtonWidgetClass, animationForm,
		   XmNrightAttachment, XmATTACH_FORM,
		   XmNbottomAttachment, XmATTACH_POSITION,
		   XmNbottomPosition, 21,
		   XmNtopAttachment, XmATTACH_POSITION,
		   XmNtopPosition, 14,
		   XmNleftAttachment, XmATTACH_POSITION,
		   XmNleftPosition, 93,
		   NULL);

  rotateButtonC = XtVaCreateManagedWidget("R",
		  xmPushButtonWidgetClass, animationForm,
		  XmNbottomAttachment, XmATTACH_FORM,
		  XmNrightAttachment, XmATTACH_FORM,
	          XmNtopAttachment, XmATTACH_POSITION,
		  XmNtopPosition, 93,
		  XmNleftAttachment, XmATTACH_POSITION,
		  XmNleftPosition, 93,
		  NULL);

  rotateButtonR = XtVaCreateManagedWidget("",
		  xmPushButtonWidgetClass, animationForm,
		  XmNbottomAttachment, XmATTACH_FORM,
		  XmNleftAttachment, XmATTACH_POSITION,
		  XmNleftPosition, 86,
		  XmNrightAttachment, XmATTACH_POSITION,
		  XmNrightPosition, 93,
		  XmNtopAttachment, XmATTACH_POSITION,
		  XmNtopPosition, 93, 
		  NULL);

  rotateButtonL = XtVaCreateManagedWidget("",
		  xmPushButtonGadgetClass, animationForm,
		  XmNbottomAttachment, XmATTACH_FORM,
		  XmNleftAttachment, XmATTACH_POSITION,
		  XmNleftPosition, 79,
		  XmNrightAttachment, XmATTACH_POSITION,
		  XmNrightPosition, 86,
		  XmNtopAttachment, XmATTACH_POSITION,
		  XmNtopPosition, 93,
		  NULL);

  rotateButtonD = XtVaCreateManagedWidget("",
		  xmPushButtonWidgetClass, animationForm,
		  XmNrightAttachment, XmATTACH_FORM,
		  XmNtopAttachment, XmATTACH_POSITION,
		  XmNtopPosition, 86,
		  XmNbottomAttachment, XmATTACH_POSITION,
		  XmNbottomPosition, 93,
		  XmNleftAttachment, XmATTACH_POSITION,
		  XmNleftPosition, 93, 
		  NULL);

  rotateButtonU = XtVaCreateManagedWidget("",
		  xmPushButtonWidgetClass, animationForm,
		  XmNrightAttachment, XmATTACH_FORM,
		  XmNtopAttachment, XmATTACH_POSITION,
		  XmNtopPosition, 79,
		  XmNbottomAttachment, XmATTACH_POSITION,
		  XmNbottomPosition, 86,
		  XmNleftAttachment, XmATTACH_POSITION,
		  XmNleftPosition, 93,
		  NULL);

  translateButtonC = XtVaCreateManagedWidget("T",
		     xmPushButtonWidgetClass, animationForm,
		     XmNbottomAttachment, XmATTACH_FORM,
		     XmNleftAttachment, XmATTACH_FORM,
	             XmNtopAttachment, XmATTACH_POSITION,
		     XmNtopPosition, 93,
		     XmNrightAttachment, XmATTACH_POSITION,
		     XmNrightPosition, 7,
		     NULL);

  translateButtonL = XtVaCreateManagedWidget("",
		     xmPushButtonWidgetClass, animationForm,
		     XmNbottomAttachment, XmATTACH_FORM,
		     XmNrightAttachment, XmATTACH_POSITION,
		     XmNrightPosition, 14,
		     XmNleftAttachment, XmATTACH_POSITION,
		     XmNleftPosition, 7,
		     XmNtopAttachment, XmATTACH_POSITION,
		     XmNtopPosition, 93, 
		     NULL);

  translateButtonR = XtVaCreateManagedWidget("",
		     xmPushButtonGadgetClass, animationForm,
		     XmNbottomAttachment, XmATTACH_FORM,
		     XmNrightAttachment, XmATTACH_POSITION,
		     XmNrightPosition, 21,
		     XmNleftAttachment, XmATTACH_POSITION,
		     XmNleftPosition, 14,
		     XmNtopAttachment, XmATTACH_POSITION,
		     XmNtopPosition, 93,
		     NULL);

  translateButtonD = XtVaCreateManagedWidget("",
		     xmPushButtonWidgetClass, animationForm,
		     XmNleftAttachment, XmATTACH_FORM,
		     XmNtopAttachment, XmATTACH_POSITION,
		     XmNtopPosition, 86,
		     XmNbottomAttachment, XmATTACH_POSITION,
		     XmNbottomPosition, 93,
		     XmNrightAttachment, XmATTACH_POSITION,
		     XmNrightPosition, 7, 
		     NULL);


  translateButtonU = XtVaCreateManagedWidget("",
		     xmPushButtonWidgetClass, animationForm,
		     XmNleftAttachment, XmATTACH_FORM,
		     XmNtopAttachment, XmATTACH_POSITION,
		     XmNtopPosition, 79,
		     XmNbottomAttachment, XmATTACH_POSITION,
		     XmNbottomPosition, 86,
		     XmNrightAttachment, XmATTACH_POSITION,
		     XmNrightPosition, 7,
		     NULL);

  calculateTrajectoryButton = XtVaCreateManagedWidget("Calculate",
                              xmPushButtonWidgetClass, topForm,
                              XmNheight, 40,
                              XmNwidth, 90,
                              XmNleftAttachment, XmATTACH_POSITION,
                              XmNleftPosition, 5,
                              XmNbottomAttachment, XmATTACH_POSITION,
                              XmNbottomPosition, 97,
			      XmNsensitive, False,
                              NULL);

  configureButton = XtVaCreateManagedWidget("New",
                    xmPushButtonWidgetClass, fileForm,
                    XmNleftAttachment, XmATTACH_POSITION,
                    XmNleftPosition, 5,	
                    XmNbottomAttachment, XmATTACH_POSITION,
                    XmNbottomPosition, 90,
		    XmNheight, 30,
		    XmNwidth, 70,
                    NULL); 

  editButton = XtVaCreateManagedWidget("Edit",
	       xmPushButtonWidgetClass, fileForm,
	       XmNleftAttachment,XmATTACH_POSITION,
	       XmNleftPosition, 37,
	       XmNbottomAttachment, XmATTACH_POSITION,
	       XmNbottomPosition, 90,
	       XmNheight, 30,
	       XmNwidth, 70,
	       NULL);

  addFilesButton = XtVaCreateManagedWidget("Add",
		   xmPushButtonWidgetClass, fileForm,
	           XmNleftAttachment,XmATTACH_POSITION,
	           XmNleftPosition, 69,
	           XmNbottomAttachment, XmATTACH_POSITION,
	           XmNbottomPosition, 90,
	           XmNheight, 30,
	           XmNwidth, 70,
		   NULL);

  exitButton = XtVaCreateManagedWidget("Exit",
	       xmPushButtonWidgetClass, topForm,
	       XmNheight, 40,
	       XmNwidth, 90,
	       XmNrightAttachment, XmATTACH_POSITION,
               XmNrightPosition, 95,	
	       XmNbottomAttachment, XmATTACH_POSITION,
               XmNbottomPosition, 97,
	       NULL);

  /*Create a separator to separate the calculation, conf. and exit
    buttons*/
  
  separator1 = XtVaCreateManagedWidget("Separate",
               xmSeparatorWidgetClass, topForm,
	       XmNorientation, XmHORIZONTAL,
	       XmNheight, 7,
	       XmNseparatorType, XmSHADOW_ETCHED_OUT,
               XmNleftAttachment, XmATTACH_POSITION,
	       XmNleftPosition, 3,
	       XmNrightAttachment, XmATTACH_POSITION,
	       XmNrightPosition, 97,
	       XmNtopAttachment, XmATTACH_POSITION,
	       XmNtopPosition, 87,
	       NULL);

  /*
    The callbacks:
  */

  XtAddCallback(configureButton, XmNactivateCallback,
		configure, NULL);

  XtAddCallback(detailsButton, XmNactivateCallback,
		details, NULL);

  XtAddCallback(calculateTrajectoryButton, XmNactivateCallback,
		getObjectNo, inputFile);
  XtAddCallback(calculateTrajectoryButton, XmNactivateCallback,
     datacontrol, CALCULATE);
  XtAddCallback(calculateTrajectoryButton, XmNactivateCallback,
    sendAnimationTime, NULL);
			
  /*As you can gather I don't know why this is needed. Without
    it the program core dumps when after an expose event. */
  XtAddCallback(calculateTrajectoryButton, XmNactivateCallback,
  functionWithoutAKnownReason, NULL);
		
  /*The button which brings up the editor.*/
  XtAddCallback(editButton, XmNactivateCallback,
		editor, NULL);
			
  /*The callbacks below are for the rotation buttons which rotate
    the coordinate system (what else).*/
  XtAddCallback(rotateButtonC, XmNactivateCallback,
		rotateDefault, NULL);
  char *oneDegree; oneDegree = "1"; 
  char *minusOneDegree; minusOneDegree = "-1";
  XtAddCallback(rotateButtonD, XmNactivateCallback,
		rotateAboutX, oneDegree);
  XtAddCallback(rotateButtonD, XmNarmCallback,
		rotateContinuousAboutX, oneDegree);
  XtAddCallback(rotateButtonD, XmNdisarmCallback,
		removeButtonTimer, NULL);

  XtAddCallback(rotateButtonU, XmNactivateCallback,
		rotateAboutX, minusOneDegree);
  XtAddCallback(rotateButtonU, XmNarmCallback,
		rotateContinuousAboutX, minusOneDegree);
  XtAddCallback(rotateButtonU, XmNdisarmCallback,
		removeButtonTimer, NULL);

  XtAddCallback(rotateButtonL, XmNactivateCallback,
		rotateAboutZ, oneDegree);
  XtAddCallback(rotateButtonL, XmNarmCallback,
		rotateContinuousAboutZ, oneDegree);
  XtAddCallback(rotateButtonL, XmNdisarmCallback,
		removeButtonTimer, NULL);

  XtAddCallback(rotateButtonR, XmNactivateCallback,
		rotateAboutZ, minusOneDegree);
  XtAddCallback(rotateButtonR, XmNarmCallback,
		rotateContinuousAboutZ, minusOneDegree);
  XtAddCallback(rotateButtonR, XmNdisarmCallback,
		removeButtonTimer, NULL);

  /*What foolows are callbacks for the zooming*/
  char *In, *Out, *InIn, *OutOut, *Default, *resizeCallback;
  In = "0"; Out = "1"; InIn = "2"; OutOut = "3"; Default = "4";
      resizeCallback = "5";
  XtAddCallback(zoomInButton, XmNactivateCallback,
		zoom, In);
  XtAddCallback(zoomInButton, XmNarmCallback,
		zoomContinuous, In);
  XtAddCallback(zoomInButton, XmNdisarmCallback,
		removeButtonTimer, NULL);
  
  XtAddCallback(zoomOutButton, XmNactivateCallback,
		zoom, Out);
  XtAddCallback(zoomOutButton, XmNarmCallback,
		zoomContinuous, Out);
  XtAddCallback(zoomOutButton, XmNdisarmCallback,
		removeButtonTimer, NULL);

  XtAddCallback(zoomInInButton, XmNactivateCallback,
		zoom, InIn);
  XtAddCallback(zoomOutOutButton, XmNactivateCallback,
		zoom, OutOut);
  XtAddCallback(zoomDefaultButton, XmNactivateCallback,
		zoom, Default);

  /*What follows are the tanslation callbacks*/
  char *Right, *Left, *Up, *Down, *Center;
  Right = "0"; Left = "1"; Up = "2"; Down = "3"; Center = "4";
      resizeCallback = "5";
  XtAddCallback(translateButtonR, XmNactivateCallback,
		translate, Right);
  XtAddCallback(translateButtonR, XmNarmCallback,
		translateContinuous, Right);
  XtAddCallback(translateButtonR, XmNdisarmCallback,
		removeButtonTimer, NULL);

  XtAddCallback(translateButtonL, XmNactivateCallback,
		translate, Left);
  XtAddCallback(translateButtonL, XmNarmCallback,
		translateContinuous, Left);
  XtAddCallback(translateButtonL, XmNdisarmCallback,
		removeButtonTimer, NULL);
 
  XtAddCallback(translateButtonU, XmNactivateCallback,
		translate, Up);
  XtAddCallback(translateButtonU, XmNarmCallback,
		translateContinuous, Up);
  XtAddCallback(translateButtonU, XmNdisarmCallback,
		removeButtonTimer, NULL);

  XtAddCallback(translateButtonD, XmNactivateCallback,
		translate, Down);
  XtAddCallback(translateButtonD, XmNarmCallback,
		translateContinuous, Down);
  XtAddCallback(translateButtonD, XmNdisarmCallback,
		removeButtonTimer, NULL);

  XtAddCallback(translateButtonC, XmNactivateCallback,
		translate, Center);
		
  /*Animation area expose and resize callbacks:*/
  XtAddCallback(animationArea, XmNexposeCallback, 
      (XtCallbackProc)drawingCallback, NULL);
  XtAddCallback(animationArea, XmNresizeCallback, 
	        resize, NULL);

  /*Slider callback. Changes the minimum animation time
    each time the user changes the value of either slider.*/
  XtAddCallback(frameIntervalScale, XmNvalueChangedCallback,
		changeAnimationTimeLabel, NULL);
  XtAddCallback(skipFramesScale, XmNvalueChangedCallback,
		changeAnimationTimeLabel, NULL);
  
  XtAddCallback(animationButton, XmNactivateCallback,
		sendAnimationTime, NULL);
  XtAddCallback(animationButton, XmNactivateCallback,
		startAnimation, NULL);
  XtAddCallback(stopAnimationButton, XmNactivateCallback,
		stopAnimation, NULL);
  XtAddCallback(pauseAnimationButton, XmNactivateCallback,
		pauseAnimation, NULL);
  XtAddCallback(forwardAnimationButton, XmNactivateCallback,
		forwardAnimation, NULL);
  XtAddCallback(backAnimationButton, XmNactivateCallback,
		backAnimation, NULL);
		
  XtAddCallback(exitButton, XmNactivateCallback,
		bye, (XtPointer) NULL);

}










