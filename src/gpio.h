#ifndef GPIO_H_
#define GPIO_H_

typedef enum Gpio {
    GPIO0 = 0,
    GPIO1 = 1,
    GPIO2 = 2,
    GPIO3 = 3,
    GPIO4 = 4,
    GPIO5 = 5,
    GPIO6 = 6,
    GPIO7 = 7,
    GPIO8 = 8,
    GPIO9 = 9,
    GPIO10 = 10,
    GPIO11 = 11,
    GPIO12 = 12,
    GPIO13 = 13,
} Gpio;

/// configure the data direction to out for the specified pin.
void gpio_out(Gpio gpio);

/// Turn `pin` on port D on.
void gpio_on(Gpio gpio);

/// Turn `pin` on port D off.
void gpio_off(Gpio gpio);

/// Toggle `pin` on port D.
void gpio_toggle(Gpio gpio);

/// Flash GPIO LED on 250 millis, off 250 millis
void flash_gpio_led(Gpio gpio);

#endif
