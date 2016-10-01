/*
Code for generic periodic Xt timers. Since they are
on the Xt level, they are a lot more accurate than
for example Motif timers.
*/

#include "timer.h"
#include <X11/Xlib.h>

/*
This callback maintains the generic periodic timers
*/

static void PeriodicCallback(XtPointer clientdata,
			     XtIntervalId* timer_id)
{
  TimerStruct* timer_struct;

  if (clientdata == NULL) {
    return;
  }

  timer_struct = (TimerStruct*) clientdata;

  /*Increment the count*/
  timer_struct -> times_called += 1;

  /*Call user callback*/

  if (timer_struct->user_callback != NULL) {
    (*timer_struct->user_callback)
      (timer_struct->user_data,
      timer_struct->times_called);
  }

  /*Set up the timer again*/

  if (timer_struct->times_called < timer_struct->max_times) {
    timer_struct->timer_id = 
      XtAppAddTimeOut(timer_struct->app_context,
		      timer_struct->interval,
		      (XtTimerCallbackProc) PeriodicCallback,
		      clientdata);
  }

} 
  /*Set up a periodic timer from a preallocated timer structure.*/

  void AddPeriodic(TimerStruct* timer_struct, XtAppContext app_context,
		   unsigned long interval, int max_times,
		   TimerFunc user_callback,  XtPointer user_data)
    {
      /*Fill in the timer structure*/

      timer_struct->app_context = app_context;
      timer_struct->interval = interval;
      timer_struct->times_called = 0;
      timer_struct->max_times = max_times;
      timer_struct->user_callback = user_callback;
      timer_struct->user_data = user_data;

      /*Set up the Xt timer */
      timer_struct->timer_id =
	XtAppAddTimeOut(app_context, interval,
			(XtTimerCallbackProc) PeriodicCallback,
		        (XtPointer) timer_struct);
    }

  void RemovePeriodic(TimerStruct* timer_struct)
    {
      if (timer_struct != (TimerStruct*) NULL) {
	/*Remove Xt timer*/
	if (timer_struct->timer_id !=0) {
	  XtRemoveTimeOut(timer_struct->timer_id);
	}

	timer_struct->timer_id = 0;
      }
    }







