#include <avr/interrupt.h>

#include "common.h"
#include "timer.h"
#include "gpio.h"

void do_nothing(void) { /* do nothing */ }

/****************************************************************************
   Timer 0
****************************************************************************/

void (*TIMER0_INTERRUPT)(void) = do_nothing;

/// Initialize timer 0.
/// 16_000_000 / (64 * 250) = 1000 interrupts / second
void timer0_init(void) {
    // Enable CTC Mode
    CLEAR_BIT(TCCR0A, WGM00 /*0*/);
    SET_BIT(TCCR0A, WGM01 /*1*/);
    CLEAR_BIT(TCCR0B, WGM02 /*3*/);

    // Set Clock Select To Prescaler 64
    SET_BIT(TCCR0B, CS00 /*0*/);
    SET_BIT(TCCR0B, CS01 /*1*/);
    CLEAR_BIT(TCCR0B, CS02 /*2*/);

    // Clear on overflow over 250 (1 millisecond)
    OCR0A = 250 - 1;
    // Enable interrupt
    SET_BIT(TIMSK0, OCIE0A /*1*/);
}

/// Set action for timer0.
void timer0_action(void (*timer0_interrupt)(void)) {
    TIMER0_INTERRUPT = timer0_interrupt;
}

/// Called after 1 millisecond of time has passed
ISR(TIMER0_COMPA_vect) {
    TIMER0_INTERRUPT();
}

/****************************************************************************
   Timer 1: USED FOR MOTOR
****************************************************************************/

// /// Initialize timer 1.
// /// 16_000_000 / (256 * 15625) = 4 interrupts / second
// void timer1_init(void) {
//     // Set Mode to 14 (Fast PWM - Top = ICR1)
//     CLEAR_BIT(TCCR1A, WGM10 /*0*/);
//     SET_BIT(TCCR1A, WGM11 /*1*/);
//     SET_BIT(TCCR1B, WGM12 /*3*/);
//     SET_BIT(TCCR1B, WGM13 /*4*/);
// 
//     // Set Clock Select To Prescaler 8
//     CLEAR_BIT(TCCR1B, CS10 /*0*/);
//     SET_BIT(TCCR1B, CS11 /*1*/);
//     CLEAR_BIT(TCCR1B, CS12 /*2*/);
// 
//     // Set Top Value to 4096
//     ICR1 = 4096;
// 
//     // Set COM bits
//     CLEAR_BIT(TCCR1A, COM1B0 /*4*/);
//     SET_BIT(TCCR1A, COM1B1 /*5*/);
// 
//     // Set match value to 0 (off)
//     OCR1B = 0;
// }

/****************************************************************************
   Timer 3
****************************************************************************/

/// Initialize timer 3.
/// 16_000_000 / (256 * 15625) = 4 interrupts / second
void timer3_init(void) {
    // Enable CTC Mode (Using OCR3A)
    CLEAR_BIT(TCCR3A, WGM30 /*0*/);
    CLEAR_BIT(TCCR3A, WGM31 /*1*/);
    SET_BIT(TCCR3B, WGM32 /*3*/);
    CLEAR_BIT(TCCR3B, WGM33 /*4*/);

    // Set Clock Select To Prescaler 256
    CLEAR_BIT(TCCR3B, CS30 /*0*/);
    CLEAR_BIT(TCCR3B, CS31 /*1*/);
    SET_BIT(TCCR3B, CS32 /*2*/);

    // Clear on overflow over 15625 (.5 millisecond)
    OCR3A = 15625 - 1;

    // Enable interrupt (OCIE3A)
    SET_BIT(TIMSK3, OCIE3A /*1*/);
}

ISR(TIMER3_COMPA_vect) { }

/****************************************************************************
   End Of File
****************************************************************************/
