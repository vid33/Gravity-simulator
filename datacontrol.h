#ifndef DATACONTROL_H
#define DATACONTROL_H

extern void sendSimulationTime(Widget w, char *clientdata, XtPointer calldata);
extern void sendStepNumber(Widget w, char *clientdata, XtPointer calldata);  

extern void datacontrol(Widget w, char *clientdata, XtPointer calldata);

extern void makeMainControlsSensitive(Widget w, XtPointer clientdata,
				      XtPointer calldata);
extern void makeMainControlsInsensitive(Widget w, XtPointer clientdata,
					XtPointer calldata);

#endif
