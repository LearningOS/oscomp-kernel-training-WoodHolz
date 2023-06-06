#ifndef __TIMER_H
#define __TIMER_H

#include "types.h"
#include "spinlock.h"

extern struct spinlock tickslock;
extern uint ticks;
typedef long int time_t;

void timerinit();
void set_next_timeout();
void timer_tick();

struct timeval {
  uint64 sec;
  uint32 usec;
};

struct timezone {
  int tz_minuteswest;
  int tz_dsttime;
};
struct timespec {
  time_t tv_sec;        /* seconds */
  long   tv_nsec;       /* nanoseconds */
};


#endif
