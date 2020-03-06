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
#include "motor.h"
#include "adc.h"

// Max out at a 60% duty cycle.
#define MAX_MOTOR_SPEED 2399

static volatile bool BUTTONA_STATE = false;
static volatile bool BUTTONC_STATE = false;

static volatile int32_t CURR_REF_POS = 0;
static volatile int8_t CURR_DIR = 0 /* Not moving */;
#define REF_POS_LEN 16
static volatile int16_t REF_POS[REF_POS_LEN] = {
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0
};
static volatile int8_t REF_POS_HEAD = 0;
static volatile int8_t REF_POS_TAIL = 0;

static int8_t ref_pos_fifo_incr(int8_t a) {
    a += 1;
    if (a >= 16) {
        a = 0;
    }
    return a;
}

static void ref_pos_fifo_push(int16_t value) {
    int8_t new_tail = ref_pos_fifo_incr(REF_POS_TAIL);
    if (new_tail == REF_POS_HEAD) {
        // Queue is full.
        /* do nothing, ignore */
        return;
    }
    REF_POS[REF_POS_TAIL] = value;
    REF_POS_TAIL = new_tail;
}

static int16_t ref_pos_fifo_pop(void) {
    if (REF_POS_HEAD == REF_POS_TAIL) {
        // Queue is empty
        /* Don't move motor */
        return 0;
    }
    int16_t value = REF_POS[REF_POS_HEAD];
    REF_POS_HEAD = ref_pos_fifo_incr(REF_POS_HEAD);
    return value;
}

// // //

static void motor_control(void) {
    printf(
        "Input: %ld, Target: %ld\r\n",
        (int32_t) global_counts_m2,
        (int32_t) CURR_REF_POS
    );
    switch (CURR_DIR) {
        case -1: {
            if (global_counts_m2 <= CURR_REF_POS) {
                CURR_DIR = 0;
                OCR1B = 0;
            }
            break;
        }
        case 1: {
            if (global_counts_m2 >= CURR_REF_POS) {
                CURR_DIR = 0;
                OCR1B = 0;
            }
            break;
        }
        case 0: default: {
            int16_t diff = ref_pos_fifo_pop();
            uint64_t pot = (adc_read(ANALOG_DC_A7) / 2) + 512;
            printf("POT = %ld\r\n", (int32_t) pot);
            if(diff != 0) {
                printf("DIFF = %d\r\n", diff);
                CURR_REF_POS += diff;
                if (diff > 0) {
                    CURR_DIR = 1;
                    motorBackward();
                    OCR1B = (uint16_t) ((MAX_MOTOR_SPEED * pot) / 1024);
                } else if (diff < 0) {
                    CURR_DIR = -1;
                    motorForward();
                    OCR1B = (uint16_t) ((MAX_MOTOR_SPEED * pot) / 1024);
                }
            }
            break;
        };
    }
}

static void poll_buttons(void) {
    bool new_buttona_state = button_pressed(BUTTONA);
    bool new_buttonc_state = button_pressed(BUTTONC);

    if(!BUTTONA_STATE && new_buttona_state) {
        // Add to end of queue unless queue is full.
        ref_pos_fifo_push(2249 /* 360 degrees = 2248.86 */);
        printf("Button A, queue push len: %d-%d\r\n", REF_POS_HEAD, REF_POS_TAIL);
    }
    if(!BUTTONC_STATE && new_buttonc_state) {
        printf("Button C, queue push len: %d-%d\r\n", REF_POS_HEAD, REF_POS_TAIL);
        // Add to end of queue
        ref_pos_fifo_push(-2249 /* -360 degrees = -2248.86 */);
    }
    BUTTONA_STATE = new_buttona_state;
    BUTTONC_STATE = new_buttonc_state;
}

static void poll_keys(void) {
    char c;

    while((c=fgetc(stdin)) != EOF) {
        // printf("Hello! Hit any key, including %c (%d) to see this again!\r\n", c, c);
        switch(c) {
            /*case 'f': {
            }*/
            default: break;
        }
    }
}

#define TASKS_LEN 3
Task TASK_LIST[TASKS_LEN] = {
    (Task) {
        .ready = false,
        .ms_period = 100,
        .next_release = 0,
        .callback = motor_control,
    },
    (Task) {
        .ready = false,
        .ms_period = 100,
        .next_release = 0,
        .callback = poll_buttons,
    },
    (Task) {
        .ready = false,
        .ms_period = 250,
        .next_release = 0,
        .callback = poll_keys,
    },
};

/****************************************************************************
   LIGHT SHOW
****************************************************************************/

/* Flash all the initialized leds for a sanity check light show */
static void light_show(void) {
    int i;
    for (i = 0; i < 2; i++) {
        flash_led(&_yellow);
        // flash_led(&_red);
        flash_led(&_green);
        flash_gpio_led(GPIO0);
        // flash_gpio_led(GPIO2);
        flash_gpio_led(GPIO4);
        // flash_gpio_led(GPIO10);
    }
}

/****************************************************************************
   CALLBACKS
****************************************************************************/

/*static void press_a(void) {
    BUTTON_A = true;
}

static void release_a(void) {
}

static void press_c(void) {
    BUTTON_C = true;
}

static void release_c(void) {
}*/

/****************************************************************************
   INITIALIZATION
****************************************************************************/

void initialize_system(void) {
    // This prevents the need to reset after flashing
    USBCON = 0;
    // Setup Button A for interrupts
    initialize_button(BUTTONA);
    initialize_button(BUTTONC);
    // setup_button_action(&_interruptA, 0 /*press*/, press_a);
    // setup_button_action(&_interruptA, 1 /*release*/, release_a);
    // setup_button_action(&_interruptC, 0 /*press*/, press_c);
    // setup_button_action(&_interruptC, 1 /*release*/, release_c);
    // enable_pcint(&_interruptA);
    // Enable output on the GPIO for the 3 LEDS
    gpio_out(GPIO0);
    // gpio_out(GPIO2);
    gpio_out(GPIO4);
    // Enable output on the motor.
    gpio_out(GPIO10);
    // Setup timers 0 and 1 for interrupts
    timer0_init();
    // timer1_init();
    timer3_init();
    // Send tasks to the scheduler.
    scheduler_init(TASK_LIST, TASKS_LEN);
    // Initialize on-board LEDs for the "sanity check"
    // initialize_led(RED);
    initialize_led(YELLOW);
    initialize_led(GREEN);
    // The "sanity check".
    // When you see this pattern of lights you know the board has reset
    light_show();
    // Set up potentiometer.
    adc_init();
    // Set up motor and motor encoder.
    setupMotor2();
    setupEncoder();
}

/****************************************************************************
   MAIN
****************************************************************************/

int main(void) {
    // Setup the hardware
    initialize_system();
    // Turn on all of the GPIO LEDs
    gpio_on(GPIO0);
    // gpio_on(GPIO2);
    gpio_on(GPIO4);
    // gpio_on(GPIO10);

    // AStar pin 11
    DDRB |= ( 1 << DDB7 );
    PORTB |= ( 1 << PORTB7 );

    // Finish Initialization
    SetupHardware(); //This setups the USB hardware and stdio
    sei(); // Enable Interrupts

    // Main Loop.
    while(1) {
        // USB communication
        USB_Mainloop_Handler();

        // Scheduler
        unsigned int i;
        for(i = 0; i < TASKS_LEN; i++) {
            if (TASK_LIST[i].ready) {
                TASK_LIST[i].callback();
                TASK_LIST[i].ready = false;
                // "Continue" the outer loop.
                break;
            }
        }
    }
} /* end main() */
