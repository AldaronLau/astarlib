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

/// Initialize timer 1.
/// 16_000_000 / (256 * 15625) = 4 interrupts / second
void timer1_init(void) {
    // Set Mode to 14 (Fast PWM - Top = ICR1)
    CLEAR_BIT(TCCR1A, 0); // WGM10
    SET_BIT(TCCR1A, 1); // WGM11
    SET_BIT(TCCR1B, 3); // WGM12
    SET_BIT(TCCR1B, 4); // WGM13

    // Set Clock Select To Prescaler 1024
    SET_BIT(TCCR1B, 0); // CS10
    CLEAR_BIT(TCCR1B, 1); // CS11
    SET_BIT(TCCR1B, 2); // CS12

    // Set Top Value to 65535
    ICR1 = 0xFFFF;

    // Set COM bits
    CLEAR_BIT(TCCR1A, 4); // COM1B0
    SET_BIT(TCCR1A, 5); // COM1B1

    // Set match value to 15728 (24% duty cycle)
    OCR1B = 15728;
}

/// Initialize timer 3.
/// 16_000_000 / (256 * 15625) = 4 interrupts / second
void timer3_init(void) {
    // Enable CTC Mode (Using OCR3A)
    CLEAR_BIT(TCCR3A, 0); // WGM30
    CLEAR_BIT(TCCR3A, 1); // WGM31
    SET_BIT(TCCR3B, 3); // WGM32
    CLEAR_BIT(TCCR3B, 4); // WGM33

    // Set Clock Select To Prescaler 256
    CLEAR_BIT(TCCR3B, 0); // CS30
    CLEAR_BIT(TCCR3B, 1); // CS31
    SET_BIT(TCCR3B, 2); // CS32

    // Clear on overflow over 15625 (.5 millisecond)
    OCR3A = 15625 - 1;

    // Enable interrupt (OCIE3A)
    SET_BIT(TIMSK3, 1);
}

/// Called after 1 millisecond of time has passed
ISR(TIMER0_COMPA_vect) {
    ms_timer += 1;
}

/// Called after 250 milliseconds of time have passed
ISR(TIMER3_COMPA_vect) {
    gpio_toggle(GPIO_PIN2);
}
