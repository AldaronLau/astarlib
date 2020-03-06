#include "common.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/*volatile uint64_t ENCODER_A_COUNT = 0;

volatile uint64_t ENCODER_B_COUNT = 0;

// Yellow: Encoder A (B4)
ISR(PCINT4_vect){
    ENCODER_A_COUNT += 1;
}

// White: Encoder B (B5)
ISR(PCINT5_vect){
    ENCODER_B_COUNT += 1;
}*/
