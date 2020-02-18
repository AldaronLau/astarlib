#ifndef GPIO_H_
#define GPIO_H_

#define GPIO_PIN0 2
#define GPIO_PIN2 1
#define GPIO_PIN4 4
#define GPIO_PIN10 6

/// configure the data direction to out for the specified pin.
void gpio_out(uint8_t pin);

/// Turn `pin` on port D on.
void gpio_on(uint8_t pin);

/// Turn `pin` on port D off.
void gpio_off(uint8_t pin);

/// Toggle `pin` on port D.
void gpio_toggle(uint8_t pin);

#endif
