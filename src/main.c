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
#include "pd.h"

// Max out at 100% duty cycle.
#define MAX_MOTOR_SPEED (32768 / 8)

#define TRAJECTORY_LEN 16
static volatile int16_t TRAJECTORY[TRAJECTORY_LEN] = {
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0
};
static volatile uint16_t TRAJECTORY_IDX = 16;

static bool UI_TASK_NUMBER = false;
static char UI_TASK_ID = '\0';
static int UI_TASK_INDEX = 0;
static int64_t UI_TASK_VALUE = 0;
static int32_t UI_TRAJECTORY[TRAJECTORY_LEN] = {
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0
};
static uint16_t UI_TRAJECTORY_IDX = 16;

// // //

static void potentiometers(void) {
    // kP pot (pot goes up to 1024)
    uint64_t pot = ((uint64_t)adc_read(ANALOG_DC_A7)) * 32768;
    pd_set_gain(((int32_t)(pot / 1024)) * 16 /*max kP*/);

    // kD pot (pot goes up to 1024)
    pot = ((uint64_t)adc_read(ANALOG_DC_A11)) * 32768;
    pd_set_damp(((int32_t)(pot / 1024)) * 16 /*max kD*/);
}

static void motor_control(void) {
    pd_loop();
}

static void poll_keys(void) {
    char c;

    USB_Mainloop_Handler();
    while((c=fgetc(stdin)) != EOF) {
        if (!UI_TASK_NUMBER) {
            switch(c) {
                case 'R': {
                    // degrees += x
                    printf("Add how many degrees? ");
                    USB_Mainloop_Handler();
                    UI_TASK_NUMBER = true;
                    UI_TASK_VALUE = 0;
                    break;
                }
                case 'r': {
                    // degrees -= x
                    printf("Subtract how many degrees? ");
                    USB_Mainloop_Handler();
                    UI_TASK_NUMBER = true;
                    UI_TASK_VALUE = 0;
                    break;
                }
                case 'Z':
                case 'z': {
                    // Zero the encoder marker the current position to 0 degrees
                    cli();
                    global_counts_m2 = 0;
                    sei();
                    printf("Zeroed global counts\n\r");
                    USB_Mainloop_Handler();
                    break;
                }
                case 'V':
                case 'v': {
                    // Print the current values Kd Kp Vm Pr Pm and T
                    view_current_values();
                    USB_Mainloop_Handler();
                    break;
                }
                case 'S':
                case 's': {
                    // Start / Stop continuously logging data
                    IS_LOGGING = !IS_LOGGING;
                    break;
                }
                default: {
                    printf("(%d)\n\r", c);
                    break;
                }
            }
            UI_TASK_ID = c;
            UI_TASK_INDEX = 0;
        } else {
            switch(c) {
                case '\r': {
                    UI_TRAJECTORY_IDX -= 1;
                    UI_TRAJECTORY[UI_TRAJECTORY_IDX] = UI_TASK_VALUE;
                    UI_TASK_VALUE = 0;
                    UI_TASK_INDEX = 0;

                    switch(UI_TASK_ID) {
                        case 'r': {
                            uint16_t i;
                            for (i = 0; i < 16; i++) {
                                TRAJECTORY[i] = -UI_TRAJECTORY[i];
                            }
                            TRAJECTORY_IDX = UI_TRAJECTORY_IDX;
                            break;
                        }
                        case 'R': {
                            uint16_t i;
                            for (i = 0; i < 16; i++) {
                                TRAJECTORY[i] = UI_TRAJECTORY[i];
                            }
                            TRAJECTORY_IDX = UI_TRAJECTORY_IDX;
                            break;
                        }
                        default: break;
                    }
                    printf("\n\r");
                    USB_Mainloop_Handler();
                    UI_TASK_NUMBER = false;
                    break;
                }
                case '\x7f': // On my Linux machine, this is backspace
                case '\b': { // Mac OS?
                    if(UI_TASK_INDEX == 0) {
                        UI_TASK_NUMBER = false;
                        printf("\r                                         \r");
                        USB_Mainloop_Handler();
                    } else {
                        UI_TASK_INDEX -= 1;
                        printf("\b \b");
                        USB_Mainloop_Handler();
                        UI_TASK_VALUE /= 10;
                    }
                    break;
                }
                case ' ': {
                    printf(" ");
                    USB_Mainloop_Handler();
                    UI_TRAJECTORY_IDX -= 1;
                    UI_TRAJECTORY[UI_TRAJECTORY_IDX] = UI_TASK_VALUE;
                    UI_TASK_VALUE = 0;
                    UI_TASK_INDEX = 0;
                    break;
                }
                default: {
                    UI_TASK_VALUE *= 10;
                    if(c <= '9' && c >= '1') {
                        UI_TASK_VALUE += 1 + (c - '1');
                    }
                    UI_TASK_INDEX += 1;
                    printf("%c", c);
                    USB_Mainloop_Handler();
                    break;
                }
            }
        }
    }
}

static void trajectory(void) {
    if(TRAJECTORY_IDX == 16) {
        return;
    }
    int32_t value = TRAJECTORY[TRAJECTORY_IDX];
    value *= 2249;
    value /= 360;
    pd_add_setpoint(value);
    TRAJECTORY_IDX += 1;
}

#define TASKS_LEN 4
Task TASK_LIST[TASKS_LEN] = {
    (Task) {
        .ready = false,
        .ms_period = 100,
        .next_release = 0,
        .callback = trajectory, // Trajectory interpolation
    },
    (Task) {
        .ready = false,
        .ms_period = 100,
        .next_release = 0,
        .callback = potentiometers, // Potentiometers
    },
    (Task) {
        .ready = false,
        .ms_period = 100,
        .next_release = 0,
        .callback = motor_control, // PD Control
    },
    (Task) {
        .ready = false,
        .ms_period = 250,
        .next_release = 0,
        .callback = poll_keys, // Text UI
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
        flash_gpio_led(GPIO4);
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
    initialize_button(BUTTONC);
    // Enable output on the GPIO for 2 LEDS
    gpio_out(GPIO0);
    gpio_out(GPIO4);
    // Potentiometer power
    gpio_out(GPIO3);
    // Setup timers 0 and 1 for interrupts
    timer0_init();
    // timer1_init();
    timer3_init();
    // Send tasks to the scheduler.
    scheduler_init(TASK_LIST, TASKS_LEN);
    // Initialize on-board LEDs for the "sanity check"
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
    gpio_on(GPIO4);
    // Pot
    // gpio_on(GPIO3);
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
