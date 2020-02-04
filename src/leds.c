#include "leds.h"

/* global LED configuration */
Led _yellow;
Led _red;
Led _green;

/* Scope is limited to this file. Used to indicate if LED is initialized. */
static uint8_t bYellowInit = 0;
static uint8_t bRedInit = 0;
static uint8_t bGreenInit = 0;

/* configure the data direction for the specified on-board led.
 */
void configure_led(Led * color) {
    SET_BIT(*color->io.ddr, color->io.pin);
}

void initialize_led(int color) {
    switch(color) {
        case (YELLOW) :
        _yellow = (Led) {
            .io = (IO_struct) { &DDRC, &PORTC, YELLOW, &PINC },
            .inverted = 0,
        };
        configure_led(&_yellow);
        bYellowInit = 1;
        break;

        case(GREEN):
        _green = (Led) {
            .io = (IO_struct) { &DDRD, &PORTD, GREEN, &PIND },
            .inverted = 1,
        };
        configure_led(&_green);
        bGreenInit = 1;
        break;

        case(RED):
        _red = (Led) {
            .io = (IO_struct) { &DDRB, &PORTB, RED, &PINB },
            .inverted = 1,
        };
        configure_led(&_red);
        bRedInit = 1;
        break;
    }
}

void led_on(Led * color) {
    if (!color->inverted) {
        SET_BIT(*color->io.port, color->io.pin);
    } else {
        CLEAR_BIT(*color->io.port, color->io.pin);
    }
}

void led_off(Led * color) {
    if (!color->inverted) {
        CLEAR_BIT(*color->io.port, color->io.pin);
    } else {
        SET_BIT(*color->io.port, color->io.pin);
    }
}

void led_toggle(Led * color) {
    TOGGLE_BIT(*color->io.port, color->io.pin);
}

/* Flash the designated on-board led for 250ms on, then 250ms off.
 * Assumes led is initialized */
void flash_led(Led * color) {
    if (!color->inverted) {
        SET_BIT(*color->io.port, color->io.pin);
    } else {
        CLEAR_BIT(*color->io.port, color->io.pin);
    }
    _delay_ms(250);
    TOGGLE_BIT(*color->io.port, color->io.pin);
    _delay_ms(250);
}

/* Flash all the initialized leds for a sanity check light show */
void light_show(void) {
    int i;
    for (i = 0; i < 2; i++) {
        if (bYellowInit) flash_led(&_yellow);
        if (bRedInit) flash_led(&_red);
        if (bGreenInit) flash_led(&_green);
    }
}
