#include <stdio.h>

#include "pd.h"
#include "motor.h"

typedef struct {
    int32_t setpoint;
    int32_t kp;
    int32_t kd;
    uint32_t time;
    // For debugging
    int32_t vm; // Measured Velocity
    int32_t pm; // Measured Position
    int32_t torque;
    int32_t last_encoder_count;
} PDControl;

static PDControl PD_CONTROL = (PDControl) {
    .setpoint = 0,
    .kp = 0,
    .kd = 0,
    .time = 0,
    .vm = 0,
    .pm = 0,
    .torque = 0,
    .last_encoder_count = 0,
};

volatile bool IS_LOGGING = false;

void view_current_values(void) {
    printf("Kd: %ld.%02ld, Kp: %ld.%02ld, Vm: %ld.%02ld, Pr: %ld.%02ld, Pm: %ld.%02ld, T: %ld.%02ld\n\r",
        PD_CONTROL.kd / 32768L,
        (((PD_CONTROL.kd % 32768L) * 100L) / 32768L),
        PD_CONTROL.kp / 32768L,
        (((PD_CONTROL.kp % 32768L) * 100L) / 32768L),
        PD_CONTROL.vm / 32768L,
        (((PD_CONTROL.vm % 32768L) * 100L) / 32768L),
        (360L * PD_CONTROL.setpoint) / 2249L,
        ((((360L * PD_CONTROL.setpoint) % 2249L) * 100L) / 32768L),
        PD_CONTROL.pm / 32768L,
        (((PD_CONTROL.pm % 32768L) * 100L) / 32768L),
        PD_CONTROL.torque / 32768L,
        (((PD_CONTROL.torque % 32768L) * 100L) / 32768L)
    );
}

// Multiply two int16_t's as if they are from -1.0 to 1.0 instead of
// -32,768 to 32,767 (fixed-point multiply)
static int32_t mul(int32_t a, int32_t b) {
    int64_t a_big = (int64_t) a;
    int64_t b_big = (int64_t) b;
    int64_t result_big = a_big * b_big;

    // Divide by max of 16 bit int.
    result_big /= 32768LL;

    int32_t result;
    if(result_big <= (int64_t) INT32_MIN) {
        result = INT32_MIN;
    } else if(result_big >= (int64_t) INT32_MAX) {
        result = INT32_MAX;
    } else {
        result = (int32_t) result_big;
    }

    return result;
}

/// Run the PD-Control Loop.
void pd_loop(void) {
    // Get encoder count.
    int32_t count = global_counts_m2;

    // Calculate error from encoder count to setpoint.
    int32_t error = PD_CONTROL.setpoint - count;
    int32_t error_over_time = error / (PD_CONTROL.time + 1); // FIXME: What time unit?

    // Send power signal to motor
    int32_t pkp = mul(PD_CONTROL.kp, -error);
    int32_t dkd = mul(PD_CONTROL.kd, error_over_time);
    int32_t power = pkp + dkd;

    // Divided by time (1).
    PD_CONTROL.vm = 32768L * (count - PD_CONTROL.last_encoder_count);
    PD_CONTROL.pm = count;
    PD_CONTROL.torque = power;
    PD_CONTROL.last_encoder_count = count;

    if(power <= -19660) {
        motorBackward();
        OCR1B = 19660 / 8;
    } else if(power >= 32768) {
        motorForward();
        OCR1B = 19660 / 8;
    } else if (power < 0) {
        motorBackward();
        OCR1B = (uint16_t)(-power / 8);
    } else {
        motorForward();
        OCR1B = (uint16_t)(power / 8);
    }

    // Increment time counter
    PD_CONTROL.time += 1;

    if(IS_LOGGING) {
        view_current_values();
    }
}

/// Add offset to the setpoint.
void pd_add_setpoint(int32_t offset) {
    PD_CONTROL.setpoint += offset;
    PD_CONTROL.time = 0;
}

/// Set KP (gain)
void pd_set_gain(int32_t gain) {
    PD_CONTROL.kp = gain;
}

/// Set KD (damp)
void pd_set_damp(int32_t damp) {
    PD_CONTROL.kd = damp;
}
