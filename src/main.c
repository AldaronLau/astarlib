#include "common.h"

#include <avr/io.h>
#include <util/delay.h>

#include "leds.h"
#include "buttons.h"

// (.4 Hz Blinking, .8 Hz Toggling)
#define YELLOW_PERIOD 1250
// 2 HZ blinking (4 Hz toggling)
#define GREEN_PERIOD 250
// Amount to sleep in while loop.
#define TICK_PERIOD 50

volatile static uint8_t PRESSED_A = 0;
volatile static uint8_t PRESSED_C = 0;

volatile static uint8_t RELEASED_A = 0;
volatile static uint8_t RELEASED_C = 0;

typedef enum {
    STATE_OFF = 0,
    STATE_ON = 1,
    STATE_BLINKING = 2,
} State;

/****************************************************************************
   ALL INITIALIZATION
****************************************************************************/
void initialize_system(void) {
    // initalize green and yellow only.
    // initialization defines the IO_structs and sets DDR
    initialize_led(GREEN);
    initialize_led(YELLOW);

    // The "sanity check".
    // When you see this pattern of lights you know the board has reset
    light_show();

    // initalize only buttonA and buttonC because they are connected to PCINT
    // NOTE: button C and the RED led are on the same line.
    initialize_button(BUTTONA);
    initialize_button(BUTTONC);
}

static void press_a(void) {
    PRESSED_A = 1;
}

static void press_c(void) {
    PRESSED_C = 1;
}

static void release_a(void) {
    RELEASED_A = 1;
}

static void release_c(void) {
    RELEASED_C = 1;
}

/****************************************************************************
   MAIN
****************************************************************************/

int main(void) {
    // This prevents the need to reset after flashing
    USBCON = 0;

    initialize_system();

    //*************************************************************//
    //*******         THE CYCLIC CONTROL LOOP            **********//
    //*************************************************************//

    setup_button_action(&_interruptA, 0 /*press*/, press_a);
    setup_button_action(&_interruptC, 0 /*press*/, press_c);

    setup_button_action(&_interruptA, 1 /*release*/, release_a);
    setup_button_action(&_interruptC, 1 /*release*/, release_c);

    enable_pcint(&_interruptA);
    enable_pcint(&_interruptC);

    sei();

    State state_a = STATE_OFF;
    State state_c = STATE_OFF;

    uint32_t yellow_tick = 0;
    uint32_t green_tick = 0;

    uint8_t yellow_expired = 0;
    uint8_t green_expired = 0;

    while(1) {
        if (yellow_tick >= YELLOW_PERIOD) {
            yellow_expired = 1;
            yellow_tick -= YELLOW_PERIOD;
        }

        if (green_tick >= GREEN_PERIOD) {
            green_expired = 1;
            green_tick -= GREEN_PERIOD;
        }

        // Check state A
        switch(state_a) {
            case STATE_OFF:
                if (RELEASED_A) {
                    state_a = STATE_ON;
                    led_on(&_yellow);
                    RELEASED_A = 0;
                }
                break;
            case STATE_ON:
                if (RELEASED_A) {
                    yellow_tick = 0;
                    yellow_expired = 1;
                    state_a = STATE_BLINKING;
                    RELEASED_A = 0;
                }
                break;
            case STATE_BLINKING:
                if (RELEASED_A) {
                    state_a = STATE_OFF;
                    led_off(&_yellow);
                    RELEASED_A = 0;
                } else if(yellow_expired) {
                    led_toggle(&_yellow);
                    yellow_expired = 0;
                }
                yellow_tick += TICK_PERIOD;
                break;
        }

        // Check state C
        switch(state_c) {
            case STATE_OFF:
                if (RELEASED_C) {
                    state_c = STATE_ON;
                    led_on(&_green);
                    RELEASED_C = 0;
                }
                break;
            case STATE_ON:
                if (RELEASED_C) {
                    green_tick = 0;
                    green_expired = 1;
                    state_c = STATE_BLINKING;
                    RELEASED_C = 0;
                }
                break;
            case STATE_BLINKING:
                if (RELEASED_C) {
                    state_c = STATE_OFF;
                    led_off(&_green);
                    RELEASED_C = 0;
                } else if(green_expired) {
                    led_toggle(&_green);
                    green_expired = 0;
                }
                green_tick += TICK_PERIOD;
                break;
        }

        // Wait
        _delay_ms(TICK_PERIOD);
    }
} /* end main() */
