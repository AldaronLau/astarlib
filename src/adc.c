#include "adc.h"

#include <avr/io.h>

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
