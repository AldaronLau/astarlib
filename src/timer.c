#include <avr/interrupt.h>

#include "common.h"
#include "timer.h"
#include "gpio.h"

volatile uint64_t ms_timer = 0;

/// Initialize timer 0.
/// 16_000_000 / (64 * 250) = 1000 interrupts / second
void timer0_init(void) {
    // Enable CTC Mode
    CLEAR_BIT(TCCR0A, 0); // WGM00
    SET_BIT(TCCR0A, 1); // WGM01
    CLEAR_BIT(TCCR0B, 3); // WGM02

    // Set Clock Select To Prescaler 64
    SET_BIT(TCCR0B, 0); // CS00
    SET_BIT(TCCR0B, 1); // CS01
    CLEAR_BIT(TCCR0B, 2); // CS02

    // Clear on overflow over 250 (1 millisecond)
    OCR0A = 250 - 1;
    // Enable interrupt (OCIE0A)
    SET_BIT(TIMSK0, 1);
}

/// Initialize timer 0.
/// 16_000_000 / (256 * 15625) = 4 interrupts / second
void timer1_init(void) {
    // Enable CTC Mode (Using OCR1A)
    CLEAR_BIT(TCCR1A, 0); // WGM10
    CLEAR_BIT(TCCR1A, 1); // WGM11
    SET_BIT(TCCR1B, 3); // WGM12
    CLEAR_BIT(TCCR1B, 4); // WGM13

    // Set Clock Select To Prescaler 256
    CLEAR_BIT(TCCR1B, 0); // CS10
    CLEAR_BIT(TCCR1B, 1); // CS11
    SET_BIT(TCCR1B, 2); // CS12

    // Clear on overflow over 15625 (.5 millisecond)
    OCR1A = 15625 - 1;

    // Enable interrupt (OCIE1A)
    SET_BIT(TIMSK1, 1);
}

/// Called after 1 millisecond of time has passed
ISR(TIMER0_COMPA_vect) {
    ms_timer += 1;
}

/// Called after 250 milliseconds of time have passed
ISR(TIMER1_COMPA_vect) {
    gpio_toggle(GPIO_PIN2);
}
