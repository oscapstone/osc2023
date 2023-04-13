#ifndef TIMER_H
#define TIMER_H

/************************************************************************
 * Timer queue, which is the basic data type of the timer-queue.
 *
 * The 0 is test the item is used, 1 or 0
 * The first is the remain seconds this timer should be.
 * The second is the call back function.
 ***********************************************************************/
typedef struct {
  unsigned char used;
  int remain;
  int (*fn)(void *);
  void *arg;
} timer_Q;

int add_timer(int (*fn)(void *), int after, void *arg);
int set_timeout(char *message, int second);
int timer_walk(int sec);

int init_timer_Q(void);
int set_timer_read();

// Timer control functions
int enable_timer_int(void);
int disable_timer_int(void);

// Util functions
int delay(uint64_t);

#endif // TIMER_H
