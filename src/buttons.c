#include "buttons.h"
#include "leds.h"

// comment this line out when you are done debugging
#define DEBUG

uint8_t button_mask = 0;

INTERRUPT_struct _interruptA = {
   .pcint = PCINT_BUTTONA,
   .mask = (1 << BUTTONA),
   .enabled = 0,
   .prev_state = (1 << BUTTONA),
   .release_fn = empty_function,
   .press_fn = empty_function,
};

INTERRUPT_struct _interruptC = {
   .pcint = PCINT_BUTTONC,
   .mask = (1 << BUTTONC),
   .enabled = 0,
   .prev_state = (1 << BUTTONC),
   .release_fn = empty_function,
   .press_fn = empty_function,
};

void empty_function(void) {}

/* Initialize the specified on-board button. Button B is not here because it has no interrupt, but you could add it.
* The corresponding pin is set as input. The pullup resistor is enabled.
*/
void initialize_button(int button) {
    if (BUTTONA == button) {
        CLEAR_BIT(DDRB, BUTTONA);
        // enable pull-up resistor
        SET_BIT(PORTB, BUTTONA);
    } else if (BUTTONC == button) {
        CLEAR_BIT(DDRB, BUTTONC);
        // enable pull-up resistor
        SET_BIT(PORTB, BUTTONC);
    }
}

void enable_pcint(INTERRUPT_struct *state) {
    // PCICR: Pin Change Interrupt Control Register
    // PCIE0: Pin Change Interrupt Enable Bit:
    //    Any change on any enabled PCINT7..0 can fire ISR.
    PCICR |= (1 << PCIE0);

    // PCMSK0: Pin Change Mask for Interrupt0, which is for all pins 0 through 7
    // Enable interrupts on Button A (PCINT3) and Button C (PCINT0)
    PCMSK0 |= (1 << state->pcint);

    state->enabled = 1;
    button_mask |= state->mask;
    state->prev_state = PINB & state->mask;
}

void setup_button_action(INTERRUPT_struct *state, int release,
    void (*callback)(void))
{
    if (release) {
        state->release_fn = callback;
    } else {
        state->press_fn = callback;
    }
}

/// Called when button state changes
ISR(PCINT0_vect) {
    // Read port B, mask for enabled buttons
    uint8_t pinb_now = (PINB & button_mask);

    // Check that pin change persists. ignore if not.
    _delay_ms(10);
    if (pinb_now ^ (PINB & button_mask)) {
        return;
    }

    // If interrupt A is enabled, check for changes on button A
    if (_interruptA.enabled) {
        uint8_t state = pinb_now & _interruptA.mask;
        // If there was a state change
        if (state != _interruptA.prev_state) {
            // If it was released, call the release_fn()
            if (state) {
                _interruptA.release_fn();
            }
            // Else, call the press_fn()
            else {
                _interruptA.press_fn();
            }
            // save state as prev_state
            _interruptA.prev_state = state;
        }
    }
    // If interrupt C is enabled, check for changes on button A
    if (_interruptC.enabled) {
        uint8_t state = pinb_now & _interruptC.mask;
        // If there was a state change
        if (state != _interruptC.prev_state) {
            // If it was released, call the release_fn()
            if (state) {
                _interruptC.release_fn();
            }
            // Else, call the press_fn()
            else {
                _interruptC.press_fn();
            }
            // save state as prev_state
            _interruptC.prev_state = state;
        }
    }
}
