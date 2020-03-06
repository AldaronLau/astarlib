#ifdef VIRTUAL_SERIAL
#include <VirtualSerial.h>
#else
#error VirtualSerial not defined!
#endif

#include <stdint.h>
#include <stdio.h>
#include <avr/interrupt.h>

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
    // This setups the USB hardware and stdio
    SetupHardware();
}

uint64_t bench(void) {
    uint64_t now = get_ms_ticks();
    uint64_t i;
    for (i = 0; i < 100000; i+= 1) {
        __asm__ __volatile__("nop");
    }
    uint64_t finish = get_ms_ticks();
    // Average worst case execution time in microseconds.
    return finish - now;
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
                case 'w': {
                    printf("Benching...\r\n");
                    USB_Mainloop_Handler();
                    uint64_t wcet = bench();
                    printf(
                        "avg. WCET = %lu.%2lu micros\r\n",
                        (unsigned long) wcet / 100,
                        (unsigned long) wcet % 100
                    );
                }
                default: break;
            }
        }
    }

    return 0;
}
