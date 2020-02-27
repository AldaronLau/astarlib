#ifdef VIRTUAL_SERIAL
#include <VirtualSerial.h>
#else
#error VirtualSerial not defined!
#endif

#include <stdio.h>
#include <stdbool.h>

#include "common.h"

#include <avr/io.h>
#include <util/delay.h>

#include "leds.h"
#include "buttons.h"
#include "gpio.h"
#include "timer.h"
#include "scheduler.h"

static volatile bool BUTTON_A = false;
static volatile int BUTTON_A_COUNTER = 0;

static bool SLOW_FREQ = false;

// Task 1
static void toggle_red(void) {
    gpio_toggle(GPIO0);
}

// Task 2
static void toggle_yellow(void) {
    gpio_toggle(GPIO2);
}

// Task 3
static void poll_button_a(void) {
    if(BUTTON_A) {
        BUTTON_A_COUNTER += 1;
    }
}

// Task 4
static void poll_for_key(void) {
    char c;

    while((c=fgetc(stdin)) != EOF) {
        printf("Hello! Hit any key, including %c (%d) to see this again!\r\n", c, c);
        switch(c) {
            case 'f': {
                if (SLOW_FREQ) {
                    ICR1 = 4096 - 1;
                    OCR1B = 4096 / 2;
                } else {
                    ICR1 = 65536L - 1L;
                    OCR1B = 65536L / 2;
                }
                SLOW_FREQ = !SLOW_FREQ;
            }
            default: break;
        }
    }
}

// Task 5
static void print_press_count(void) {
    printf("Number of button A presses = %d!\r\n", BUTTON_A_COUNTER);
}

#define TASKS_LEN 5
Task TASK_LIST[TASKS_LEN] = {
    (Task) {
        .ready = false,
        .ms_period = 500,
        .next_release = 0,
        .callback = toggle_red,
    },
    (Task) {
        .ready = false,
        .ms_period = 250,
        .next_release = 0,
        .callback = toggle_yellow,
    },
    (Task) {
        .ready = false,
        .ms_period = 1000,
        .next_release = 0,
        .callback = poll_button_a,
    },
    (Task) {
        .ready = false,
        .ms_period = 2000,
        .next_release = 0,
        .callback = poll_for_key,
    },
    (Task) {
        .ready = false,
        .ms_period = 3000,
        .next_release = 0,
        .callback = print_press_count,
    }
};

/****************************************************************************
   LIGHT SHOW
****************************************************************************/

/* Flash all the initialized leds for a sanity check light show */
static void light_show(void) {
    int i;
    for (i = 0; i < 2; i++) {
        flash_led(&_yellow);
        flash_led(&_red);
        flash_led(&_green);
        flash_gpio_led(GPIO0);
        flash_gpio_led(GPIO2);
        flash_gpio_led(GPIO4);
        flash_gpio_led(GPIO10);
    }
}

/****************************************************************************
   CALLBACKS
****************************************************************************/

static void press_a(void) {
    BUTTON_A = true;
}

static void release_a(void) {
    for (int i = 0; i < 10; i++) {
        gpio_toggle(GPIO4);
        _delay_ms(200);
    }
    BUTTON_A = false;
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
    gpio_out(GPIO0);
    gpio_out(GPIO2);
    gpio_out(GPIO4);
    gpio_out(GPIO10);
    // Setup timers 0 and 1 for interrupts
    timer0_init();
    timer1_init();
    timer3_init();
    // Send tasks to the scheduler.
    scheduler_init(TASK_LIST, TASKS_LEN);
    // Initialize on-board LEDs for the "sanity check"
    initialize_led(RED);
    initialize_led(YELLOW);
    initialize_led(GREEN);
    // The "sanity check".
    // When you see this pattern of lights you know the board has reset
    light_show();
}

/****************************************************************************
   MAIN
****************************************************************************/

int main(void) {
    // Setup the hardware
    initialize_system();
    // Turn on all of the GPIO LEDs
    gpio_on(GPIO0);
    gpio_on(GPIO2);
    gpio_on(GPIO4);
    gpio_on(GPIO10);

    // Finish Initialization
    SetupHardware(); //This setups the USB hardware and stdio
    sei(); // Enable Interrupts

    // Main Loop.
    while(1) {
        // USB communication
        USB_Mainloop_Handler();

        // Scheduler
        for(unsigned int i = 0; i < TASKS_LEN; i++) {
            if (TASK_LIST[i].ready) {
                TASK_LIST[i].callback();
                TASK_LIST[i].ready = false;
                // "Continue" the outer loop.
                break;
            }
        }
    }
} /* end main() */
