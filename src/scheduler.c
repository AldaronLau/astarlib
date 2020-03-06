#include "scheduler.h"
#include "timer.h"

static Task* ISR_TASK_LIST;
static unsigned int ISR_TASK_LEN = 0;
static uint64_t MILLIS = 0;

void millis_interrupt(void) {
    MILLIS += 1;

    unsigned int i;
    for(i = 0; i < ISR_TASK_LEN; i += 1) {
        if(MILLIS >= ISR_TASK_LIST[i].next_release) {
            ISR_TASK_LIST[i].ready = true;
            ISR_TASK_LIST[i].next_release = MILLIS + ISR_TASK_LIST[i].ms_period;
        }
    }
}

/// Initialize millisecond task scheduler.
void scheduler_init(Task* isr_task_list, unsigned int isr_task_len) {
    ISR_TASK_LIST = isr_task_list;
    ISR_TASK_LEN = isr_task_len;
    timer0_action(millis_interrupt);
}
