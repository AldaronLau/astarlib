#ifndef ADC_H_
#define ADC_H_

#include <stdint.h>

/// Analog input pins.
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

void adc_init(void);
uint16_t adc_read(AnalogDC ch);

#endif
