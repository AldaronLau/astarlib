#include "common.h"

#include <avr/io.h>
#include <util/delay.h>

#include "leds.h"
#include "buttons.h"
#include "gpio.h"
#include "timer.h"

/****************************************************************************
   CALLBACKS
****************************************************************************/

static void press_a(void) {
    // Do nothing
}

static void release_a(void) {
    for (int i = 0; i < 10; i++) {
        gpio_toggle(GPIO_PIN4);
        _delay_ms(200);
    }
}

/****************************************************************************
   INITIALIZATION
****************************************************************************/

void initialize_system(void) {
    // This prevents the need to reset after flashing
    USBCON = 0;
    // Setup Button A for interrupts
    initialize_button(BUTTONA);
    setup_button_action(&_interruptA, 0 /*press*/, press_a);
    setup_button_action(&_interruptA, 1 /*release*/, release_a);
    enable_pcint(&_interruptA);
    // Enable output on the GPIO for the 3 LEDS
    gpio_out(GPIO_PIN0);
    gpio_out(GPIO_PIN2);
    gpio_out(GPIO_PIN4);
//    gpio_out(GPIO_PIN10);
    SET_BIT(DDRB, 6);
    // Setup timers 0 and 1 for interrupts
    timer0_init();
    timer1_init();
    timer3_init();
    // Initialize on-board LEDs for the "sanity check"
    initialize_led(RED);
    initialize_led(YELLOW);
    initialize_led(GREEN);
    // The "sanity check".
    // When you see this pattern of lights you know the board has reset
    // light_show();
}

/****************************************************************************
   MAIN
****************************************************************************/

int main(void) {
    uint64_t last = 500;

    // Setup the hardware
    initialize_system();
    // Turn on all of the GPIO LEDs
    gpio_on(GPIO_PIN0);
    gpio_on(GPIO_PIN2);
    gpio_on(GPIO_PIN4);
    //gpio_on(GPIO_PIN10);
    // SET_BIT(PORTB, 6);
    // Background self-scheduling of Red LED blinking.
    // sei(); // Enable Interrupts
    while(1) {
        if(ms_timer >= last) {
            gpio_toggle(GPIO_PIN0);
            last += 500;
        }
    }
} /* end main() */
