#include "common.h"

/// configure the data direction to out for the specified pin.
void gpio_out(uint8_t pin) {
    SET_BIT(DDRD, pin);
}

/// Turn `pin` on port D on.
void gpio_on(uint8_t pin) {
    SET_BIT(PORTD, pin);
}

/// Turn `pin` on port D off.
void gpio_off(uint8_t pin) {
    CLEAR_BIT(PORTD, pin);
}

/// Toggle `pin` on port D.
void gpio_toggle(uint8_t pin) {
    TOGGLE_BIT(PORTD, pin);
}
