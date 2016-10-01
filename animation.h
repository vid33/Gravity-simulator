#ifndef ANIMATION_H
#define ANIMATION_H

/*
Functions used to draw &redraw the 3-d grid for the animation.
*/

extern void drawingCallback(Widget w);
extern void resize(Widget w, XtPointer calldata, XtPointer clientdata);
extern void rotateDefault(Widget w, XtPointer calldata, XtPointer clientdata);
extern void rotateAboutX(Widget w, XtPointer calldata, XtPointer clientdata);
extern void rotateAboutZ(Widget w, XtPointer calldata, XtPointer clientdata);
extern void rotateContinuousAboutX(Widget w, XtPointer calldata,
				   XtPointer clientdata);
extern void rotateContinuousAboutZ(Widget w, XtPointer calldata,
				   XtPointer clientdata);
extern void removeButtonTimer(Widget w, XtPointer clientdata, 
				XtPointer calldata);
extern void zoom(Widget w, XtPointer clientdata, XtPointer calldata);
extern void zoomContinuous(Widget w, XtPointer clientdata, XtPointer calldata);
extern void translate(Widget w, XtPointer clientdata, XtPointer calldata);
extern void translateContinuous(Widget w, XtPointer clientdata,
				XtPointer calldata);
extern void animate();

extern void startAnimation(Widget w, XtPointer clientdata, XtPointer calldata);
extern void stopAnimation(Widget w, XtPointer clientdata, XtPointer calldata);
extern void pauseAnimation(Widget w, XtPointer clientdata, XtPointer calldata);
extern void backAnimation(Widget w, XtPointer clientdata, XtPointer calldata);
extern void forwardAnimation(Widget w, XtPointer clientdata, XtPointer calldata);

extern void sendAnimationTime(Widget w, char *clientdata, XtPointer calldata);
extern void resizeNotify(Widget w, XtPointer clinetdata, XtPointer calldata);

void functionWithoutAKnownReason(Widget w, XtPointer x, XtPointer y);
#endif




