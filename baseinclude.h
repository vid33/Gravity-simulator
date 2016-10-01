#ifndef BASEINCLUDE_H
#define BASEINCLUDE_H

/*
C/C++ include files
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <g++/iostream.h>
#include <g++/fstream.h>


/*
Low level Xlib include files
*/

#include <X11/Xos.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/keysym.h>


/*
Motif include files. The reason for including XmAll.h is that I
had problems with default Motif dialogs and include files, and was 
too lazy to figure out why. The other files are included to make it more 
appearent what my GUI consists of
*/

#include <Xm/XmAll.h> 
#include <Xm/Form.h>
#include <Xm/PushB.h>	
#include <Xm/Label.h>
#include <Xm/Text.h>
#include <Xm/RowColumn.h>
#include <Xm/DrawingA.h>

/*
Include files for Mesa/OpenGL libraries, which are used for the
planet animation

#include <GL/GLwDrawA.h>*/
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <gltk.h>

#endif
