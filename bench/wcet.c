#ifdef VIRTUAL_SERIAL
#include <VirtualSerial.h>
#else
#error VirtualSerial not defined!
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <avr/interrupt.h>

#define SET_BIT(reg, pin) (reg |= (1<<pin))
#define CLEAR_BIT(reg, pin) (reg &= ~(1<<pin))
#define TOGGLE_BIT(reg, pin) (reg ^= (1<<pin))

volatile uint64_t MS_TICKS = 0;

// Scheduler
void do_nothing(void) {}
typedef struct Task {
    bool ready;
    uint64_t ms_period;
    uint64_t next_release;
    void (*callback)(void);
} Task;
Task TASK_LIST[5] = {
    (Task) {
        .ready = false,
        .ms_period = 1,
        .next_release = 1,
        .callback = do_nothing,
    },
    (Task) {
        .ready = false,
        .ms_period = 1,
        .next_release = 1,
        .callback = do_nothing,
    },
    (Task) {
        .ready = false,
        .ms_period = 1,
        .next_release = 1,
        .callback = do_nothing,
    },
    (Task) {
        .ready = false,
        .ms_period = 1,
        .next_release = 1,
        .callback = do_nothing,
    },
    (Task) {
        .ready = true,
        .ms_period = 1,
        .next_release = 1,
        .callback = do_nothing,
    },
};
// Scheduler ISR:
// static Task* ISR_TASK_LIST = TASK_LIST;
// static unsigned int ISR_TASK_LEN = 5;

// Encoder ISR:
volatile int8_t global_m2a;
volatile int8_t global_m2b;
volatile int32_t global_counts_m2 = 0;
volatile int8_t global_error_m2 = 0;
volatile int16_t global_last_m2a_val;
volatile int16_t global_last_m2b_val;
#define chA_control DDRB
#define chA_pin DDB4
#define chB_control DDRB
#define chB_pin DDB5
#define enc_power_control DDRD
#define enc_power_pin DDD1
#define enc_power_output PORTD

// Potentiometer
typedef enum {
    ANALOG_DC_A0 = 7,
    ANALOG_DC_A1 = 6,
    ANALOG_DC_A2 = 5,
    ANALOG_DC_A3 = 4,
    ANALOG_DC_A4 = 1,
    ANALOG_DC_A5 = 0,
    ANALOG_DC_A6 = 8,
    ANALOG_DC_A7 = 10,
    ANALOG_DC_A8 = 11,
    ANALOG_DC_A9 = 12,
    ANALOG_DC_A10 = 13,
    ANALOG_DC_A11 = 9,
} AnalogDC;
volatile uint64_t pot = 0;

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

/// Initialize analog to digital converter.
void adc_init(void) {
    // AREF = AVcc
    ADMUX = (1<<REFS0);

    // ADC Enable and prescaler of 128
    // 16000000/128 = 125000
    ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

/// Read and convert value from analog channel.
uint16_t adc_read(AnalogDC ch) {
    // Clear the bottom 3 bits before ORing.
    ADMUX &= 0xF8;
    if (ch >= 8) {
        ADCSRB |= (1<<MUX5);
        ADMUX |= (ch - 8);
    } else {
        ADCSRB &= ~(1<<MUX5);
        ADMUX |= ch;
    }

    // Start single conversion
    // Write '1' to ADSC
    ADCSRA |= (1<<ADSC);

    // Wait for conversion to complete (ADSC becomes '0' again).
    while(ADCSRA & (1<<ADSC));

    return ADC;
}

uint64_t get_ms_ticks(void) {
    cli();
    uint64_t now = MS_TICKS;
    sei();
    return now;
}

void setupEncoder(void) {
  // Set the encoders as input
  chA_control &= ~(1 << chA_pin );
  chB_control &= ~(1 << chB_pin );

  // Enable the interrupts for the 2 encoder channels
  // PCMSK0 |= (1 << chA_INT );
  // PCMSK0 |= (1 << chB_INT );

  // enable PCINT interrupts
  // PCICR |= (1 << PCIE0 );

  // Powering the encoder through general I/O. This sets signal high to provide power to device.
  enc_power_control |= (1 << enc_power_pin );
  enc_power_output  |= (1 << enc_power_pin );
}

void init(void) {
    // Auto reset after code deploy
    USBCON = 0;
    // Enable millisecond timer.
    timer0_init();
    // This setups the USB hardware and stdio
    SetupHardware();


}

uint32_t bench(void) {
    uint32_t now = get_ms_ticks();
    uint64_t i;
    for (i = 0; i < 100000; i+= 1) {
        // Scheduler (ISR)
        /*unsigned int i;
        for(i = 0; i < ISR_TASK_LEN; i += 1) {
            if(MS_TICKS >= ISR_TASK_LIST[i].next_release) {
                ISR_TASK_LIST[i].ready = true;
                ISR_TASK_LIST[i].next_release = MS_TICKS + ISR_TASK_LIST[i].ms_period;
            } else {
                // Do anyway to guarantee code path is worst case
                ISR_TASK_LIST[i].ready = true;
                ISR_TASK_LIST[i].next_release = MS_TICKS + ISR_TASK_LIST[i].ms_period;
            }
        }*/

        // Scheduler (Main)
        /*unsigned int i;
        for(i = 0; i < 5; i++) {
            if (TASK_LIST[i].ready) {
                TASK_LIST[i].callback();
                TASK_LIST[i].ready = false;
                // "Continue" the outer loop.
                break;
            }
        }
        // Re-enable TASK_LIST[4].ready, so that it continues on worst code path
        TASK_LIST[4].ready = true;*/

        // Encoder (ISR)
        /*// Make a copy of the current reading from the encoders
        uint8_t tmpB = PINB;
        // Get value of each channel, making it either a 0 or 1 valued integer
        uint8_t m2a_val = (tmpB & (1 << chA_pin )) >> chA_pin;
        uint8_t m2b_val = (tmpB & (1 << chB_pin )) >> chB_pin;
        // Adding or subtracting counts is determined by how these change between interrupts
        int8_t plus_m2 = m2a_val ^ global_last_m2b_val;
        int8_t minus_m2 = m2b_val ^ global_last_m2a_val;
        // Add or subtract encoder count as appropriate
        if(plus_m2) { global_counts_m2 += 1; }
        if(minus_m2) { global_counts_m2 -= 1; }
        // If both values changed, something went wrong - probably missed a reading
        if(m2a_val != global_last_m2a_val && m2b_val != global_last_m2b_val) {
            global_error_m2 = 1;
            PORTD ^= (1 << PORTD5);
        }
        // Save for next interrupt
        global_last_m2a_val = m2a_val;
        global_last_m2b_val = m2b_val;*/

        pot = ((uint64_t)adc_read(ANALOG_DC_A7)) * 32768;
    }
    uint32_t finish = get_ms_ticks();
    // Average worst case execution time in 100ths of microseconds.
    return finish - now;
}

int main(void) {
    char c;

    init();
    setupEncoder();
    adc_init();

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
                    uint32_t wcet = bench();
                    printf("avg. WCET = %lu.%2lu micros\r\n", wcet / 100,
                        wcet % 100);
                }
                default: break;
            }
        }
    }

    return 0;
}
