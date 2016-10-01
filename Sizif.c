#include "baseinclude.h"
#include "animation.h"
#include "gui.h"

/*
Xlib/Xt and GL globals needed by my silly program
*/

Display* globalDisplay;
Window animationWindow;
XtAppContext globalContext;
XVisualInfo *animationVisual;
Colormap animationColormap;
GLXContext animationContext;
GLboolean doubleBuffer = GL_TRUE, moving = GL_FALSE, made_current = GL_FALSE;


static int dblBuf[] = {
    GLX_DOUBLEBUFFER, GLX_RGBA, GLX_DEPTH_SIZE, 16,
    GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1, GLX_BLUE_SIZE, 1,
    None
};
static int *snglBuf = &dblBuf[1];

/*Pixmap used to redraw the drawing area of my animation*/
Pixmap redrawAnimation; 
/*Pixmap which will hold the grod only*/
Pixmap grid;

/* 
Graphics context for the drawable in which
the animation is performed, defausl colormap, and main XColor structure.
*/

GC animationGC;
GC negativeAnimationGC; /*used to clear the pixmap*/
GC planetGC; /*used to draw planets*/

XColor backgroundColor;
long unsigned int BackgroundColor;
XColor planetColor,planetBackgroundColor;
long unsigned int PlanetColor, PlanetBackgroundColor;

extern Widget animationArea;
Widget toplevel,top;

int main(int argc, char **argv)
{

  /*
  Below are the GUI declarations
  */

  char *objectNumber;

  /*'Low level' Xt Xlib decparations*/

  XGCValues xgcvalues;
  XGCValues negativeXgcValues;
  XGCValues planetXgcValues;
  int screen;
  int n = 0;

  /*Motif declarations*/

  Widget topForm;

  toplevel = XtVaAppInitialize (&globalContext, "KBH", NULL,
	     0, &argc, argv, NULL,
	     XmNmappedWhenManaged, False,
	     XmNminHeight, 600,
	     XmNminWidth, 650,
             NULL);

  top = XtVaCreateManagedWidget ("mw",
	xmMainWindowWidgetClass, toplevel,
	XmNheight, 650,
        XmNwidth, 700,
        NULL);
          
  gui();

  globalDisplay = XtDisplay(animationArea);
  animationWindow = XtWindow(animationArea);
  screen = DefaultScreen(globalDisplay);

  /*find an OpenGL capable visual*/
  animationVisual = glXChooseVisual(globalDisplay, screen, dblBuf);
  if (animationVisual == NULL) {
    animationVisual = glXChooseVisual(globalDisplay, screen, snglBuf);
    if (animationVisual == NULL)
      XtAppError(globalContext, 
		 "Sisyphus is sad without a visual he wants.");
    doubleBuffer = GL_FALSE;
  }

  /*create an openGL rendering context which will be used for animation*/
  animationContext = glXCreateContext(globalDisplay, animationVisual,
				      None, GL_TRUE);
  if (animationContext == NULL) {
    XtAppError(globalContext, 
	       "Sisyphus is sad without his animation context.");
  }

  /*Create a colormap since it may be that the default one is not the one
    which is used.*/
  animationColormap = XCreateColormap(globalDisplay, 
		      RootWindow(globalDisplay, animationVisual->screen),
		      animationVisual->visual, AllocNone);

  /*Make this the default colormap of the toplevel window 
   (before toplevel is realized).*/
  XtVaSetValues(top, XtNvisual, animationVisual->visual, 
		XtNdepth, animationVisual->depth,
		XtNcolormap, animationColormap, NULL);

  /*XtAddEventHandler(toplevel, StructureNotifyMask, False,
		    map_state_changed, NULL);*/


  /*To create resize events when window is made smaller.
   This has to be done by using XChangeWindowAttributes, before
   the window is managed.*/
  /*  XSetWindowAttributes attributes;
     unsigned long attr_mask;
  attributes.bit_gravity = ForgetGravity;
  attr_mask |= CWBitGravity;
  XChangeWindowAttributes(XtDisplay(animationArea), XtWindow(animationArea), 
  attr_mask, &attributes);*/	

  /*Now realize toplevel*/

  XtRealizeWidget(toplevel);
  XtManageChild(animationArea);

  /*Once toplevel is realized, bind the openGL rendering context
    to the animationWindow*/
  glXMakeCurrent(globalDisplay, XtWindow(animationArea), animationContext);
  made_current = GL_TRUE;

  /*Set up OpenGL state.*/
 glClearDepth(1.0);
    glClearColor(1.0, 1.0, 1.0, 1.0);
    rotateDefault(top, NULL, NULL);

  XtMapWidget(toplevel);
  XtAppMainLoop(globalContext);

  return(0);
}








