#ifndef LEDS_H_
#define LEDS_H_

#include "common.h"
#include <util/delay.h>
#include <inttypes.h>

// These function as enumerated types (using pin numbers as values)
#define GREEN 5
#define YELLOW 7
#define RED 0

typedef struct {
    IO_struct io;
    uint8_t inverted;
} Led;

// Global variables. They MUST be defined using initialize_led(<COLOR>).
extern Led _yellow;
extern Led _red;
extern Led _green;

/* initialize the data struct for the specific on-board led.
 */
void initialize_led(int color);

/* Flash the designated on-board led for 250ms on, then 250ms off.
 * Assumes led is initialized */
void flash_led(Led * color);

/* turn on specified on board LED. indicate if inverted (0 turns led on)
*/
void led_on(Led * color);

/* turn off specified on board LED. indicate if inverted (1 turns led off)
*/
void led_off(Led * color);

/* toggle the led between on and off
*/
void led_toggle(Led * color);

#endif
