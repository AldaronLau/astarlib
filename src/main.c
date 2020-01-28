#include "common.h"

#include <avr/io.h>
#include <util/delay.h>

#include "leds.h"
#include "buttons.h"

#define INVERTED 1

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

    // FILL THIS IN TO MEET FUNCTIONAL REQUIREMENTS:

    // Always toggle yellow LED every 1000ms
    // Toggle greed LED every 500ms, but only if is in an on state
    // Press and release button A switches green to on state.
    // Press and release button C switches green to off state.

    // Assume both buttons start in a not pressed state.

    uint8_t green_on = 1;
    uint8_t time = 0;

    uint8_t buttonA_old = 0;
    uint8_t buttonC_old = 0;

    uint8_t buttonA_state;
    uint8_t buttonC_state;

    uint8_t green_toggle = 1;

    while(1) {
        buttonA_state = is_button_pressed(&_buttonA);
        buttonC_state = is_button_pressed(&_buttonC); 

        if (!buttonA_state && buttonA_old) {
            green_on = 1;
        }
        if (!buttonC_state && buttonC_old) {
            green_on = 0;
        }

        if (time == 0) {
            led_toggle(&_yellow);
        }

        if (green_on) {
            if (time % 5 == 0) {
                if (green_toggle) {
                    led_on(&_green, 1);
                    green_toggle = 0;
                } else {
                    led_off(&_green, 1);
                    green_toggle = 1;
                }
            }
        } else {
            led_off(&_green, 1 /*INVERTED*/);
        }

        buttonA_old = buttonA_state;
        buttonC_old = buttonC_state;

        _delay_ms(100);
        time = (time + 1) % 10;
    } // end while(1)
} /* end main() */
