#ifndef SCHEDULER
#define SCHEDULER

#include <stdint.h>
#include <stdbool.h>

typedef struct Task {
    bool ready;
    uint64_t ms_period;
    uint64_t next_release;
    void (*callback)(void);
} Task;

/// Initialize millisecond task scheduler.
void scheduler_init(Task* isr_task_list, unsigned int isr_task_len);

#endif
