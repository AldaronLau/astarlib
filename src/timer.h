#ifndef TIMER
#define TIMER

#include <inttypes.h>

extern volatile uint64_t ms_timer;

/// Initialize timer 0.
void timer0_init(void);

/// Initialize timer 1.
void timer1_init(void);

#endif
