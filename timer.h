#ifndef timeout_h_
#define timeout_h_ 1

#include <X11/Intrinsic.h>

typedef void (*TimerFunc)(XtPointer userdata, int times_called);

typedef struct {
  XtAppContext  app_context;
  unsigned long interval;
  XtIntervalId  timer_id;
  int           times_called;
  int           max_times;

  /*User data*/
  TimerFunc     user_callback;
  XtPointer     user_data;
} TimerStruct;

/*Timer functions*/

extern void AddPeriodic(TimerStruct*  timer_struct,
			XtAppContext app_context,
			unsigned long interval,
			int max_times,
			TimerFunc user_callback,
			XtPointer user_data);

extern void RemovePeriodic(TimerStruct* timer_struct);

#endif
