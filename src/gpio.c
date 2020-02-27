#include "common.h"
#include "gpio.h"
#include <util/delay.h>

const uint8_t GPIO_PINS[14] = {
    2, // 0 (Port D)
    9, // FIXME: 1
    1, // 2 (Port D)
    9, // FIXME: 3
    4, // 4 (Port D)
    9, // FIXME: 5
    9, // FIXME: 6
    9, // FIXME: 7
    9, // FIXME: 8
    9, // FIXME: 9
    6, // 10 (Port B)
    9, // FIXME: 11
    9, // FIXME: 12
    9 // FIXME: 13
};

const uint8_t GPIO_PORTS[14] = {
    3, // 0 (Port D)
    5, // FIXME: 1
    3, // 2 (Port D)
    5, // FIXME: 3
    3, // 4 (Port D)
    5, // FIXME: 5
    5, // FIXME: 6
    5, // FIXME: 7
    5, // FIXME: 8
    5, // FIXME: 9
    1, // 10 (Port B)
    5, // FIXME: 11
    5, // FIXME: 12
    5 // FIXME: 13
};

static inline volatile uint8_t* get_port(uint8_t port) {
    switch(port) {
        case 1:
            return &PORTB;
        case 2:
            return &PORTC;
        case 3:
            return &PORTD;
        default:
            return 0;
    }
}

static inline volatile uint8_t* get_ddr(uint8_t port) {
    switch(port) {
        case 1:
            return &DDRB;
        case 2:
            return &DDRC;
        case 3:
            return &DDRD;
        default:
            return 0;
    }
}

/// configure the data direction to out for the specified pin.
void gpio_out(Gpio gpio) {
    SET_BIT(*get_ddr(GPIO_PORTS[gpio]), GPIO_PINS[gpio]);
}

/// Turn `pin` on port D on.
void gpio_on(Gpio gpio) {
    SET_BIT(*get_port(GPIO_PORTS[gpio]), GPIO_PINS[gpio]);
//    SET_BIT(PORTD, pin);
}

/// Turn `pin` on port D off.
void gpio_off(Gpio gpio) {
    CLEAR_BIT(*get_port(GPIO_PORTS[gpio]), GPIO_PINS[gpio]);
//    CLEAR_BIT(PORTD, pin);
}

/// Toggle `pin` on port D.
void gpio_toggle(Gpio gpio) {
    TOGGLE_BIT(*get_port(GPIO_PORTS[gpio]), GPIO_PINS[gpio]);
//    TOGGLE_BIT(PORTD, pin);
}

/// Flash GPIO LED on 250 millis, off 250 millis
void flash_gpio_led(Gpio gpio) {
    gpio_on(gpio);
    _delay_ms(100);
    gpio_toggle(gpio);
    _delay_ms(100);
}
