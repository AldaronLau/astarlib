#ifdef VIRTUAL_SERIAL
#include <VirtualSerial.h>
#else
#error VirtualSerial not defined!
#endif

#define F_CPU 16000000UL

#include "../src/motor.h"
#include "../src/gpio.h"

#include <stdint.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define SET_BIT(reg, pin) (reg |= (1<<pin))
#define CLEAR_BIT(reg, pin) (reg &= ~(1<<pin))
#define TOGGLE_BIT(reg, pin) (reg ^= (1<<pin))

volatile uint64_t MS_TICKS = 0;

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

/// Called after 1 millisecond of time has passed
ISR(TIMER0_COMPA_vect) {
    MS_TICKS += 1;
}

uint64_t get_ms_ticks(void) {
    cli();
    uint64_t now = MS_TICKS;
    sei();
    return now;
}

void init(void) {
    // Auto reset after code deploy
    USBCON = 0;
    // Enable millisecond timer.
    timer0_init();
    // Enable output
    gpio_out(GPIO10);
    // Set up motor and motor encoder.
    setupMotor2();
    setupEncoder();
    // This setups the USB hardware and stdio
    SetupHardware();
}

uint32_t bench(void) {
    // Move motor
    motorForward();
    OCR1B = 600;

    cli();
    uint32_t begin = interrupt_counter;
    sei();

    // Wait for 1 second.
    _delay_ms(1000);

    cli();
    uint32_t finish = interrupt_counter;
    sei();

    OCR1B = 0;

    // Stop motor
    // Average worst case execution time in microseconds.
    return finish - begin;
}

int main(void) {
    char c;

    init();
    // Enable Interrupts
    sei();
    while(1) {
        // USB communication
        USB_Mainloop_Handler();
        // Get a character
        while((c=fgetc(stdin)) != EOF) {
            switch(c) {
                case 'b': {
                    printf("Benching...\r\n");
                    USB_Mainloop_Handler();
                    uint64_t freq = bench();
                    printf(
                        "Frequency = %lu interrupts / sec\r\n",
                        (unsigned long) freq
                    );
                }
                default: break;
            }
        }
    }

    return 0;
}
