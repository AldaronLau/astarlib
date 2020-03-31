#ifndef TIMER_H_
#define TIMER_H_

#include <stdint.h>

/// Initialize timer 0.
void timer0_init(void);

/// Set action for timer0.
void timer0_action(void (*timer0_interrupt)(void));

/// Initialize timer 1.
void timer1_init(void);

/// Initialize timer 3.
void timer3_init(void);

#endif
