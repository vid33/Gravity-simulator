#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <g++/iostream.h>
#include <g++/fstream.h>
#include "baseinclude.h"
#include "timer.h"

int IT_S_OK = 0;

/*Macro to convert 0 to	255 color names to 0-1*/
#define UNITY(c) (((float) c)/255)

extern XtAppContext globalContext;
extern Display* globalDisplay;
extern Window animationWindow;
extern Widget animationArea, toplevel, top;
extern XVisualInfo *animationVisual;
extern Colormap animationColormap;
extern GLXContext animationContext;
extern GLboolean doubleBuffer, moving, made_current;


/*Widgets (buttons) whose parameters will need to be changed from here*/
extern Widget exitButton;
extern Widget configureButton;		
extern Widget calculateTrajectoryButton;
extern Widget animationButton;
extern Widget pauseAnimationButton;
extern Widget stopAnimationButton;

/*The slide widgets from which animation parameters are taken:*/
extern Widget frameIntervalScale;
extern Widget skipFramesScale;

/*The details test Widget which is written to if it is open.*/
extern Widget detailsText;

/*Global and extern variables.*/

/*This is needed for a very strange reason. Look at the 
  Calculate button callbacks in gui.c for more detail.*/
extern int INTERRUPT_SIGNAL; 

extern int realStepNumber;
extern int object_no;
extern double maxXYZposition;
int animationStepNumber;
int currentStep = 0;
int interval;
int skipFrames;

/*State variables - or whatever one calls this*/
int RESIZE_NOTIFY = 1;
int PAUSE_ON = 0;
int ANIMATION_RUNNING = 0;
int CALCULATION_DONE = 0;
extern int DETAILS_WINDOW_OPEN;

/*The animation timer*/
TimerStruct globalTimer;

/*Timer used when a zooo/translate/rotate button is held down*/
TimerStruct buttonTimer;
int TIMER_STEP;
double ZOOM_FACTOR = 1;

/*Global variables for the viewport*/
int newWidth, newHeight; 
int newOriginX = 0; int  newOriginY = 0;
int translationX = 0; int translationY = 0;

/*Declared here because needed in drawingCallback, and the actual
function is at the end of this file.*/
void animate();


/*A function which writes current positions forces... to the
details window if it is open. It is called from the main animate,
loop.*/ 

void writeDetails(double buffer[])
{
  XmTextPosition textPosition;

  /*Get enough space for the whole information*/
  char planetInformation[(object_no*4*4*4*4*2*11) + 20];
  strcpy(planetInformation, "All values are in SI units.\n");

  ifstream posFile("/tmp/.Sizif");
  ifstream massFile("/tmp/.Sizif-mass");
  ifstream accelerationFile("/tmp/.Sizif-acceleration");
  ifstream velocityFile("/tmp/.Sizif-velocity");

  int currentLine;
  double xyzBuffer[object_no*3];

  char localValue[15];
  char localMass[15];
  double localDouble;
  int readMass;

  /*   for (int i = 0; i < object_no; i++) {
     for(int j = 0; j < 3; j++) {
       position3D[i][j] = (buffer[(i*3)+j]/maxXYZposition); 
     }
   }*/

  for (int i = 0; i < object_no; i++){
    textPosition = 80;
  
    currentLine = i;
    /*Set the position in the text widget.*/
    XmTextSetInsertionPosition(detailsText, textPosition);

    strcat(planetInformation, "Object ");
    sprintf(localValue, "%d", i);
    strcat(planetInformation, localValue);
    strcat(planetInformation, "\n");

    strcat(planetInformation, "mass:\n");
    massFile.seekg(0);
    massFile.seekg(currentLine*(sizeof(double)));
    massFile.read(buffer, (sizeof(double)) );
    sprintf(localMass, "%.6g", buffer[0]);
    strcat(planetInformation, localMass);
    strcat(planetInformation, "\n");
    strcat(planetInformation, "x, y, and z, positions:\n");

    for (int blah = 0; blah < 3; blah++) {
      localDouble = buffer[(i*3)+blah];
      sprintf(localValue, "%.6g", localDouble);
      /*cout << buffer[i*3+blah] << "\t" << localValue << "\n";*/
      strcat(planetInformation, localValue);
      strcat(planetInformation, "\n");
    }

    cout << planetInformation;

    /*   currentLine = 3*i;
    velocityFile.seekg(0);
    velocityFile.seekg(currentLine*(sizeof(double)) );
    velocityFile.read(xyzBuffer, (3*sizeof(double)) );
    for (int blah = 0; blah < 3; blah++) {
      sprintf(localValue, "%.6g", xyzBuffer[blah]);
      strcat(planetInformation, localValue);
      strcat(planetInformation, "\n");
    }

    currentLine = 3*i;
    accelerationFile.seekg(0);
    accelerationFile.seekg(currentLine*(sizeof(double)));
    accelerationFile.read(xyzBuffer, (3*sizeof(double)) );
    for (int blah = 0; blah < 3; blah++) {
      sprintf(localValue, "%.6g", xyzBuffer[blah]);
      strcat(planetInformation, localValue);
      strcat(planetInformation, "\n");
    }

    for (int blah = 0; blah < 3; blah++) {
      xyzBuffer[i] = xyzBuffer[blah]*buffer[0];
      sprintf(localValue, "%.6g", xyzBuffer[blah]);
      strcat(planetInformation, localValue);
      strcat(planetInformation, "\n");
    }*/

    /*cout << planetInformation << "\n";*/

    /*sprintf(localMass, "\n%.6g", buffer[0]);
    sprintf(localMass, "%.6g", buffer[0]);
    cout << localMass;*/
    /*XmTextInsert(detailsText, textPosition, localMass);
    XtFree(localMass);*/
  }
}  

/*A utilit function which draws a sphere with the radius and
  coordinates as arguments*/

void SolidSphere (GLdouble radius, GLdouble xpos, GLdouble ypos, 
		  GLdouble zpos)
{
    GLUquadricObj *quadObj;
    glEnable(GL_DEPTH_TEST);
    glTranslatef(xpos, ypos, zpos);
    quadObj = gluNewQuadric ();
    gluQuadricDrawStyle (quadObj, (GLenum)GLU_LINE);
    gluSphere(quadObj, radius, 8, 8);
    glTranslatef((-1*xpos), (-1*ypos), (-1*zpos));
}


/*Two utility functions which disable.enable the main buttons while animating*/

void disableNormalUse() {
  /*Make the exit, calculate and configure, buttons active. Now 
    the user can do anything.*/
  XtVaSetValues(exitButton,
		XmNsensitive, False,
		NULL);
  XtVaSetValues(configureButton,
		XmNsensitive, False,
		NULL);
  XtVaSetValues(skipFramesScale,
		XmNsensitive, False,
		NULL);
  XtVaSetValues(frameIntervalScale,
		XmNsensitive, False,
		NULL);
}

void enableNormalUse() {
  /*Make the exit, calculate and configure, buttons active. Now 
    the user can do anything.*/
  XtVaSetValues(exitButton,
		XmNsensitive, True,
		NULL);
  XtVaSetValues(configureButton,
		XmNsensitive, True,
		NULL);
  XtVaSetValues(skipFramesScale,
		XmNsensitive, True,
		NULL);
  XtVaSetValues(frameIntervalScale,
		XmNsensitive, True,
		NULL);
}

void drawingCallback(Widget w)
{
  glLoadIdentity();

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
    glEnable(GL_DEPTH_TEST);
    glColor3f(0, 0, 0);/*Black*/

   glBegin(GL_LINES);
        glVertex3f(-0.95, 0, 0);
        glVertex3f(0.95, 0, 0);
   glEnd();

   glBegin(GL_LINES);
        glVertex3f(0, -0.95, 0);
	glVertex3f(0, 0.95, 0);
   glEnd();

   glBegin(GL_LINES);
         glVertex3f(0, 0, -0.95);
	 glVertex3f(0, 0, 0.95);
   glEnd();

   /*make the lines into arrows by adding four sided pyramids, 
     in this 3-d case.*/

   /*Positive y-axis:*/
   /*Sides of pyramid*/
   glColor3f(UNITY(255), UNITY(0), UNITY(0));/*Red*/
   glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0, 0.98, 0);
        glVertex3f(0.015, 0.91, 0.015); 
        glVertex3f(-0.015, 0.91, 0.015); 
        glVertex3f(-0.015, 0.91, -0.015); 
        glVertex3f(0.015, 0.91, -0.015);  
	glVertex3f(0.015, 0.91, 0.015); 
  glEnd();
   /*Base of pyramid*/
   glBegin(GL_QUADS);
     glVertex3f(0.015, 0.91, 0.015); 
     glVertex3f(-0.015, 0.91, 0.015); 
     glVertex3f(-0.015, 0.91, -0.015);     
     glVertex3f(0.015, 0.91, -0.015);  
   glEnd();

   /*Negative y-axis*/
   /*Sides of pyramid*/
   glColor3f(UNITY(219), UNITY(112), UNITY(147));/*Pale violet red*/
   glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0, -0.98, 0);
        glVertex3f(0.015, -0.91, 0.015); 
        glVertex3f(-0.015, -0.91, 0.015); 
        glVertex3f(-0.015, -0.91, -0.015); 
        glVertex3f(0.015, -0.91, -0.015);  
	glVertex3f(0.015, -0.91, 0.015); 
  glEnd();
   /*Base of pyramid*/
   glBegin(GL_QUADS);
     glVertex3f(0.015, -0.91, 0.015); 
     glVertex3f(-0.015, -0.91, 0.015); 
     glVertex3f(-0.015, -0.91, -0.015);     
     glVertex3f(0.015, -0.91, -0.015);  
   glEnd();

   /*Negative x-axis*/
   /*Sides of pyramid*/
   glColor3f(UNITY(124), UNITY(252), UNITY(0));/*Lawn green*/
   glBegin(GL_TRIANGLE_FAN);
        glVertex3f(-0.98,0, 0);
        glVertex3f(-0.91, 0.015, 0.015); 
        glVertex3f(-0.91, -0.015, 0.015); 
        glVertex3f(-0.91,-0.015, -0.015); 
        glVertex3f(-0.91, 0.015, -0.015);  
	glVertex3f(-0.91, 0.015, 0.015); 
  glEnd();
   /*Base of pyramid*/
   glBegin(GL_QUADS);
     glVertex3f(-0.91, 0.015, 0.015); 
     glVertex3f(-0.91,-0.015, 0.015); 
     glVertex3f(-0.91, -0.015, -0.015);     
     glVertex3f(-0.91, 0.015, -0.015);  
   glEnd();

   /*Positive x-axis*/
   glColor3f(UNITY(34), UNITY(139), UNITY(34));/*Forrest green*/
   /*Sides of pyramid*/
   glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0.98,0, 0);
        glVertex3f(0.91, 0.015, 0.015); 
        glVertex3f(0.91, -0.015, 0.015); 
        glVertex3f(0.91,-0.015, -0.015); 
        glVertex3f(0.91, 0.015, -0.015);  
	glVertex3f(0.91, 0.015, 0.015); 
  glEnd();
   /*Base of pyramid*/
   glBegin(GL_QUADS);
     glVertex3f(0.91, 0.015, 0.015); 
     glVertex3f(0.91,-0.015, 0.015); 
     glVertex3f(0.91, -0.015, -0.015);     
     glVertex3f(0.91, 0.015, -0.015);  
   glEnd();

   /*Positive z-axis*/
   glColor3f(UNITY(100), UNITY(100), UNITY(100));/*Black*/
   /*Sides of pyramid*/
   glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0, 0, 0.98);
        glVertex3f(0.015, 0.015, 0.91); 
        glVertex3f(-0.015, 0.015, 0.91); 
        glVertex3f(-0.015, -0.015, 0.91); 
        glVertex3f(0.015, -0.015, 0.91);  
	glVertex3f(0.015, 0.015, 0.91); 
  glEnd();
   /*Base of pyramid*/
   glBegin(GL_QUADS);
     glVertex3f(0.015, 0.015, 0.91); 
     glVertex3f(-0.015, 0.015, 0.91); 
     glVertex3f(-0.015, -0.015, 0.91);     
     glVertex3f(0.015, -0.015, 0.91);  
   glEnd();

   /*Negative z-axis*/
   glColor3f(UNITY(0), UNITY(0), UNITY(0));/*Grey*/
   /*Sides of pyramid*/
   glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0, 0, -0.98);
        glVertex3f(0.015, 0.015, -0.91); 
        glVertex3f(-0.015, 0.015, -0.91); 
        glVertex3f(-0.015, -0.015, -0.91); 
        glVertex3f(0.015, -0.015, -0.91);  
	glVertex3f(0.015, 0.015, -0.91); 
  glEnd();
   /*Base of pyramid*/
   glBegin(GL_QUADS);
     glVertex3f(0.015, 0.015, -0.91); 
     glVertex3f(-0.015, 0.015, -0.91); 
     glVertex3f(-0.015, -0.015, -0.91);     
     glVertex3f(0.015, -0.015, -0.91);  
   glEnd();

   animate();
  
}

void rotateDefault(Widget w, XtPointer clientdata, XtPointer calldata)
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glRotated(30, 1.0, 0, 0);
  glRotated(30, 0, -1.0, 0);  
  glPushMatrix();
  glMatrixMode(GL_MODELVIEW);  
  drawingCallback(animationArea);
}


void rotateAboutX(Widget w, XtPointer clientdata, XtPointer calldata)
{
  double angle; 
  char *angleChar;
  angleChar = (char *)clientdata;
  angle = strtod(angleChar, NULL);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glRotated(angle, 1.0, 0, 0);
  glPushMatrix();
  glMatrixMode(GL_MODELVIEW); 
  drawingCallback(animationArea);
}


void rotateAboutZ(Widget w, XtPointer clientdata, XtPointer calldata)
{
  double angle; 
  char *angleChar;
  angleChar = (char *)clientdata;
  angle = strtod(angleChar, NULL);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();  /*make the previous matrix current*/
  glRotated((GLdouble)angle, 0, -1.0, 0);
  glPushMatrix();/*make top two matrices on the stack the current matrix*/
  glMatrixMode(GL_MODELVIEW); 
  drawingCallback(animationArea);
}


void removeButtonTimer(Widget w, XtPointer clientdata, XtPointer calldata)
{
  RemovePeriodic(&buttonTimer);
}

/*When the buttons is held down for longer than 0.5sec this is called,
  It rottates the coordinate system continuously.*/
void timerControlX(XtPointer clientdata, int id) 
{
  /*Start rotating only after half a second*/
  char *deg; deg = (char *)clientdata;
  if (TIMER_STEP >= 10) {
    rotateAboutX(top, deg, NULL);
  }
  TIMER_STEP = TIMER_STEP + 1;
}


void rotateContinuousAboutX(Widget w, XtPointer clientdata, XtPointer calldata)
{
  int interval = 30;  /*milliseconds*/
  char *deg; deg = (char *)clientdata;
  TIMER_STEP = 0;
  AddPeriodic(&buttonTimer, globalContext, interval, 1000, timerControlX, deg);
}

/*When the buttons is held down for longer than 0.5sec this is called,
  It rottates the coordinate system continuously.*/
void timerControlZ(XtPointer clientdata, int id) 
{
  /*Start rotating only after half a second*/
  char *deg; deg = (char *)clientdata;
  if (TIMER_STEP >= 10) {
    rotateAboutZ(top, deg, NULL);
  }
  TIMER_STEP = TIMER_STEP + 1;
}


void rotateContinuousAboutZ(Widget w, XtPointer clientdata, XtPointer calldata)
{
  int interval = 30;  /*milliseconds*/
  char *deg; deg = (char *)clientdata;
  TIMER_STEP = 0;
  AddPeriodic(&buttonTimer, globalContext, interval, 1000, timerControlZ, deg);
}


void zoom(Widget w, XtPointer clientdata, XtPointer calldata)
{
  char *local; local = (char *)clientdata;
  int option =  atoi(local);
  Dimension       width, height;
  double widthDifference, heightDifference;

  /*First find out what to do: which callback called the funtion*/
  /*Zoom In*/
  if (option == 0) {
    ZOOM_FACTOR = ZOOM_FACTOR + 0.02;
  }
  /*Zoom Out. Don't allow too much zooming*/
  if (option == 1) {
    if ((ZOOM_FACTOR - 0.02) > 0.5) {
      ZOOM_FACTOR = ZOOM_FACTOR - 0.02;
    }
    else {
      ZOOM_FACTOR = 0.5;
    }
  } 
  /*Zoom In quickly*/
  if (option == 2) {
    ZOOM_FACTOR = ZOOM_FACTOR + 0.3;
  }
  /*Zoom Out Quickly. Again, don't allow too much*/
  if (option == 3) {
    if ((ZOOM_FACTOR - 0.3) > 0.5){
      ZOOM_FACTOR = ZOOM_FACTOR - 0.3;
    }
    else {
      ZOOM_FACTOR = 0.5;
    }
  }
  /*Zoom Default*/
  if (option == 4) {
    ZOOM_FACTOR = 1;
  }
  /*Reize Callback*/
  else {
    option == 5;
    ZOOM_FACTOR = ZOOM_FACTOR; /*ha*/
  }
  XtVaGetValues(animationArea, XmNwidth, &width, XmNheight, &height, NULL);
  newWidth = (GLsizei)(((double)width)*ZOOM_FACTOR);
  newHeight = (GLsizei)(((double)height)*ZOOM_FACTOR);


  /*Get the maximum viewport dimensions*/
  GLdouble maxWidthAndHeight[2];
  glGetDoublev(GL_MAX_VIEWPORT_DIMS, maxWidthAndHeight);

  /*Do the right thing if the new dimensions exceed the maximum ones.*/

  if ((newWidth >= maxWidthAndHeight[0]) || 
      (newHeight >= maxWidthAndHeight[1])) {
    if (option == 0) {
      ZOOM_FACTOR = ZOOM_FACTOR - 0.02;
    }
    if (option == 2) {
      ZOOM_FACTOR = ZOOM_FACTOR - 0.3;
    }
    /*Don't zoom, but the widths and heights need to be 
      recalculated anyway because of the translate function,
      which needs to have the correct values at all times.*/
    newWidth = (GLsizei)(((double)width)*ZOOM_FACTOR);
    newHeight = (GLsizei)(((double)height)*ZOOM_FACTOR);
    return;
  }

  widthDifference = width - newWidth;
  heightDifference = height - newHeight;
  newOriginX = (GLint)((widthDifference)/(2)) + 
    ((translationX));
  newOriginY = (GLint)((heightDifference)/(2)) + 
    ((translationY));

  glViewport(newOriginX, newOriginY, newWidth, newHeight);
  drawingCallback(animationArea);
}

void zoomTimerControl(XtPointer clientdata, int id) 
{
  /*Start rotating only after half a second*/
  char *deg; deg = (char *)clientdata;
  if (TIMER_STEP >= 10) {
    zoom(top, deg, NULL);
  }
  TIMER_STEP = TIMER_STEP + 1;
}


void zoomContinuous(Widget w, XtPointer clientdata, XtPointer calldata)
{
  int interval = 50;  /*milliseconds*/
  char *deg; deg = (char *)clientdata;
  TIMER_STEP = 0;
  AddPeriodic(&buttonTimer, globalContext, interval, 1000, 
	      zoomTimerControl, deg);
}


void translate(Widget w, XtPointer clientdata, XtPointer calldata) 
{
  char *local; local = (char *)clientdata;
  int option =  atoi(local);  
 
  Dimension       width, height;
  XtVaGetValues(animationArea, XmNwidth, &width, XmNheight, &height, NULL); 

  if (ZOOM_FACTOR == 1) {
    /*Prevent these variables from being empty*/
    newWidth = (int) width;
    newHeight = (int) height;
  }

  /*Find out what button called translation: which way to translate
    The user is allowed to fly off into free space because I think it's
    cool.*/

  /*Translate to the right.*/
  if (option == 0) {  
    newOriginX = newOriginX -2;
    translationX = translationX -2;
  }
  /*Translate to the left.*/
  if (option == 1) {  
    newOriginX = newOriginX + 2;
    translationX = translationX + 2;
  }

  /*Translate up.*/
  if (option == 2) {  
    newOriginY = newOriginY - 2;  
    translationY = translationY - 2;
  }

  /*Translate down.*/
  if (option == 3) {    
    newOriginY = newOriginY + 2;
    translationY = translationY + 2;
  }

  /*Recenter the picture*/
  if (option == 4) {
    char* center; center = "5";
    translationY = 0; translationX = 0;
    zoom(top, center, NULL);
    return;
  }
  /*Draw the new Picture*/
  glViewport(newOriginX, newOriginY, (GLint)newWidth, (GLint)newHeight);
  drawingCallback(animationArea);
}


void translateTimerControl(XtPointer clientdata, int id) 
{
  /*Start rotating only after half a second*/
  char *deg; deg = (char *)clientdata;
  if (TIMER_STEP >= 10) {
    translate(top, deg, NULL);
  }
  TIMER_STEP = TIMER_STEP + 1;
}


void translateContinuous(Widget w, XtPointer clientdata, XtPointer calldata)
{
  int interval = 50;  /*milliseconds*/
  char *deg; deg = (char *)clientdata;
  TIMER_STEP = 0;
  AddPeriodic(&buttonTimer, globalContext, interval, 1000, 
	      translateTimerControl, deg);
}

void resize(Widget w, XtPointer data, XtPointer callData)
{
    char *resizeCallback; resizeCallback = "5";
    if(made_current) {
      zoom(top, resizeCallback, NULL);
    }
}


void animate()
{
  /*If the animation is not running just draw the grid.
    This function is called by the resize and expose events.
    I did things this way because now the planets get redrawn
    during resize/expose events during animation. This prevents
    unwanted dissapearing of planets during slow animations.*/
  if (CALCULATION_DONE == 0) {    
    if (doubleBuffer) glXSwapBuffers(globalDisplay, XtWindow(animationArea));
    if(!glXIsDirect(globalDisplay, animationContext)){
        glFinish(); /* avoid indirect rendering latency from queuing */
    }
    return;
  }

 /*The file where all the position information is*/
  char *cent = "/tmp/.Sizif";
  ifstream central(cent);

  /*Here is a little problem I've encountered and can't explain away.
    If this function is not called as the last calculation callback,
    the application core dumps. It seems that something needs to be 
    drawn to the window. Unless there is a stupid bug I can't find,
    the problem may be in the compiler, Mesa, or Motif libraries. 
    A further problem is that a user may interrupt the calculation
    process, which causes the data file /tmp/.Sizif to be erased.
    This means that I have to solve the problem by drawing something
    other than the planets to the window, and then erasing it
    (Look at functionForNoReason(x,x,x) at the end of the file).
    Thus the code below.*/
  if(!bool(central.good())) {
    /*Some bluish color for useless Planet*/
    glColor3f(UNITY(96), UNITY(104), UNITY(168));				              /*draw a useless planet:*/
    SolidSphere(0.03, 0, 0, 0);
    if (doubleBuffer) glXSwapBuffers(globalDisplay, XtWindow(animationArea));
    if(!glXIsDirect(globalDisplay, animationContext)){
        glFinish(); /* avoid indirect rendering latency from queuing */
    }
    return;
  }  /*End of silly code.*/

  double position3D[object_no][3];

  /*used to go to the correct point in the file holding the position
    data*/
  int currentLine; 


  /*Used by both animate and details functions. Used to retrieve
    current positions of objects from the input file.*/
  int loc = 3*object_no;
  double buffer[loc];
 
  /*Now make the currentLine be the line for the new picture*/
  currentLine = ((currentStep*(skipFrames+1))*object_no*3); 

  /*go to the correct point in file*/
  central.seekg(currentLine*(sizeof(double)));


  /*get the data for the object*/
    central.read(buffer, (3*object_no*(sizeof(double))) );

  /*put the variables into a more convenient array than buffer,
   and divide by maximum (absolute value) XYZ distance*/
   for (int i = 0; i < object_no; i++) {
     for(int j = 0; j < 3; j++) {
       position3D[i][j] = (buffer[(i*3)+j]/maxXYZposition); 
     }
   }


  /*Some bluish color for Mr.Planet*/
  glColor3f(UNITY(96), UNITY(104), UNITY(168));				          
  /*draw the planets:*/
  for(int i = 0; i < object_no; i++) {
    SolidSphere(0.03, position3D[i][0], position3D[i][1], position3D[i][2]);
  }

  if ((ANIMATION_RUNNING == 1) && (PAUSE_ON == 0)) {
    currentStep = currentStep + 1;
  }

  /*A few things to do after the last step*/
   if (currentStep >= animationStepNumber) {
     ANIMATION_RUNNING = 0;

     /*forbid input to the pause and stop buttons*/
     XtVaSetValues(pauseAnimationButton,
		   XmNsensitive, False,
		   NULL);
     XtVaSetValues(stopAnimationButton,
		   XmNsensitive, False,
		   NULL);
     enableNormalUse();
     currentStep = 0;

     /*Call the drawing callback again in order to redraw planets
       in their initial positions*/
     drawingCallback(top);
   }

 if (doubleBuffer) glXSwapBuffers(globalDisplay, XtWindow(animationArea));
    if(!glXIsDirect(globalDisplay, animationContext))
        glFinish(); /* avoid indirect rendering latency from queuing */

  /*If the options window is open, write information to it.*/
  if (DETAILS_WINDOW_OPEN == 1) {
    writeDetails(&buffer[0]);
  }

}


void animateCB(XtPointer user_data, int times_called)
{
  drawingCallback(top);
}

/*Function which starts the animation: the callback function of the
animation button*/

void startAnimation(Widget w, XtPointer clientdata, XtPointer calldata)
{ 

  /*Make pause state inactive*/

  PAUSE_ON = 0;

  /*Make the pause and stop buttons active*/
  XtVaSetValues(pauseAnimationButton,
		XmNsensitive, True,
		NULL);
  XtVaSetValues(stopAnimationButton,
		XmNsensitive, True,
		NULL);

  disableNormalUse();

  /*remove an existing timer wich may not have finished*/
  RemovePeriodic(&globalTimer);
  
  /*Start the new timer*/
  AddPeriodic(&globalTimer, globalContext,
	      interval, (animationStepNumber - currentStep),
	      (TimerFunc) animateCB,
	      (XtPointer) NULL);
  
  ANIMATION_RUNNING = 1;
}

void stopAnimation(Widget w, XtPointer clientdata, XtPointer calldata)
{ 
  /*remove an existing timer*/
  RemovePeriodic(&globalTimer);

  /*Make pause state inactive*/
  PAUSE_ON = 0;

  /*Make the pause and stop buttons inactive*/
  XtVaSetValues(pauseAnimationButton,
		XmNsensitive, False,
		NULL);
  XtVaSetValues(stopAnimationButton,
		XmNsensitive, False,
		NULL);

  enableNormalUse();

  /*If pause was on at the time of stopping, change 
    label to "P"*/
  if (PAUSE_ON == 1){
    XmString newLabel = XmStringGenerate("P", NULL,
					 XmCHARSET_TEXT, NULL);
    XtVaSetValues(pauseAnimationButton,
		  XmNx, 10,
		  XmNlabelString, newLabel,
		  NULL);
    XmStringFree(newLabel);
  }

  /*Reset the current step*/
  currentStep = 0;
 
  ANIMATION_RUNNING = 0;

  drawingCallback(top);
}

void pauseAnimation(Widget w, XtPointer clientdata, XtPointer calldata)
{ 
  if (PAUSE_ON == 0) {
    /*remove an existing timer*/
    RemovePeriodic(&globalTimer);

    /*Change the label of the button to "resume".*/

    XmString newLabel = XmStringGenerate("R", NULL,
					 XmCHARSET_TEXT, NULL);
    XtVaSetValues(pauseAnimationButton,
		  XmNx, 10,
		  XmNlabelString, newLabel,
		  NULL);
    XtVaSetValues(pauseAnimationButton,
		  XmNwidth, 55,
		  XmNheight, 25,
		  NULL);
    XmStringFree(newLabel);
  }

  if (PAUSE_ON == 1) {
    /*Change the label back to "P"*/
    XmString oldLabel = XmStringGenerate("P", NULL,
					 XmCHARSET_TEXT, NULL);
    XtVaSetValues(pauseAnimationButton,
		  XmNx, 10,
		  XmNlabelString, oldLabel,
		  NULL);
    XtVaSetValues(pauseAnimationButton,
		  XmNwidth, 55,
		  XmNheight, 25,
		  NULL);
    XmStringFree(oldLabel);

  /*Start the new timer*/
    AddPeriodic(&globalTimer, globalContext,
		interval, (animationStepNumber - currentStep),
		(TimerFunc) animateCB,
		(XtPointer) NULL);
  }

  /*Change the PAUSE_ON state*/
  if (PAUSE_ON == 1) {
    PAUSE_ON = 0;
  }
  else {
    PAUSE_ON = 1;
  }

  /*If the animation is not running, always keep the PAUSE_ON state off*/
  if (ANIMATION_RUNNING == 0) {
    PAUSE_ON = 0;
  }
}


void backAnimation(Widget w, XtPointer clientdata, XtPointer calldata)
{
  /*The task is to go back in the animation by about 2 percent*/
  int backStepNumber = (int) (0.02*animationStepNumber);

  /*Remove the current timer:*/
  RemovePeriodic(&globalTimer);

  /*In the unlikely event that the above is 0:*/
  if (backStepNumber == 0) {
    backStepNumber = 1;
  }

  if ((currentStep - backStepNumber) > 0) {
    currentStep = currentStep - backStepNumber;
  }
  else {
    currentStep = animationStepNumber - backStepNumber;
  }

  if ((PAUSE_ON == 1) || (ANIMATION_RUNNING == 0)) {
    drawingCallback(top);
    return;
  }

  if (ANIMATION_RUNNING == 1) {
    AddPeriodic(&globalTimer, globalContext,
		interval, (animationStepNumber - currentStep),
		(TimerFunc) animateCB,
		(XtPointer) NULL);
  }

}

void forwardAnimation(Widget w, XtPointer clientdata, XtPointer calldata) 
{
  /*The task is to go forward in the animation by about 2 percent*/
  int forwardStepNumber = (int) (0.02*animationStepNumber);

  /*Remove the current timer:*/
  RemovePeriodic(&globalTimer);

  /*In the unlikely event that the above is 0:*/
  if (forwardStepNumber == 0) {
    forwardStepNumber = 1;
  }

  if ((currentStep + forwardStepNumber) < animationStepNumber) {
    currentStep = currentStep + forwardStepNumber;
  }
  else {
    currentStep = forwardStepNumber;
  }

  if ((PAUSE_ON == 1) || (ANIMATION_RUNNING == 0)) {
    drawingCallback(top);
    return;
  }

  if (ANIMATION_RUNNING == 1) {
    AddPeriodic(&globalTimer, globalContext,
		interval, (animationStepNumber - currentStep),
		(TimerFunc) animateCB,
		(XtPointer) NULL);
  }

}	
  
/*
This function gets the animation time, and calculates the number of
steps per second.
*/

void sendAnimationTime(Widget w, char *clientdata, XtPointer calldata)
{
  /*Get the interval and the number of frames whch should be skipped
    from the slides. Wihen using the normal Xt GetValue function
    the appplication core dumps when called by the animation button.
    I am beginning to dislike Motif.*/
  XmScaleGetValue(skipFramesScale,  &skipFrames);
  XmScaleGetValue(frameIntervalScale,  &interval);

  /*Calculate the total number of frames from the above data.*/
   animationStepNumber = (int)( ((double)realStepNumber)/
			       ((double)(skipFrames+1)) );
			       
 /*Check for possible problems due to rounding.*/ 
   int local = animationStepNumber*(1+skipFrames);
  if (local > realStepNumber) {
    animationStepNumber = animationStepNumber - 1;
  }

}

void functionWithoutAKnownReason(Widget w, XtPointer x, XtPointer y)
{
  CALCULATION_DONE = 1;
  currentStep = 0;


/* XtUnmanageChild(animationArea);
   animate();*/
  if (INTERRUPT_SIGNAL == 1) {
    CALCULATION_DONE = 0;
    INTERRUPT_SIGNAL =0;
  }
 
}











