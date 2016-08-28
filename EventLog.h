#ifndef EventLogH
#define EventLogH

#define EVENT_RUN 1
#define EVENT_LOSTCONNECTION 2
#define EVENT_ABORT 3


void newEvent(char marker, char *text);
extern void clearAllEvents();
void repaintEventLog();

#endif
