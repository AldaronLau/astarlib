#include "pd.h"

typedef struct {
    int32_t setpoint;
    int16_t kp;
    int16_t kd;
    uint32_t time;
} PDControl;

static PDControl PD_CONTROL = (PDControl) {
    .setpoint = 0,
    .kp = 0,
    .kd = 0,
    .time = 0,
};

// Multiply two int16_t's as if they are from -1.0 to 1.0 instead of
// -32,768 to 32,767 (fixed-point multiply)
static int16_t mul(int16_t a, int16_t b) {
    int32_t a_big = (int32_t) a;
    int32_t b_big = (int32_t) b;
    int32_t result_big = a_big * b_big;

    // Put into range of 16 bit int.
    result_big /= 32768;

    int16_t result = (int16_t) result_big;

    return result;
}

// Do a saturating add on two int16_t's
static int16_t add(int16_t a, int16_t b) {
    int32_t a_big = (int32_t) a;
    int32_t b_big = (int32_t) b;
    int32_t result_big = a_big + b_big;

    if(result_big >= 32767) {
        return 32767;
    } else {
        return (int16_t) result_big;
    }
}

/// Run the PD-Control Loop.
void pd_loop(void) {
    // Get encoder count.
    
    // FIXME

    // Calculate error from encoder count to setpoint.
    int32_t error = ;

    // FIXME

    // Send power signal to motor
    int16_t pkp = mul(PD_CONTROL.kp, error);
    int16_t dkd = mul(PD_CONTROL.kd, error_over_time);
    int16_t power = add(pkp, dkd);

    if(power == -32768) {
        OCR1B = 32768;
    } else if (power < 0) {
        motorBackward();
        OCR1B = (uint16_t)(-power);
    } else {
        motorForward();
        OCR1B = (uint16_t) power;
    }

    // Increment time counter
    PD_CONTROL.time += 1;
}

/// Add offset to the setpoint.
void pd_add_setpoint(int32_t offset) {
    PD_CONTROL.setpoint += offset;
}

/// Set KP (gain)
void pd_set_gain(int16_t gain) {
    PD_CONTROL.kp = gain;
}

/// Set KD (damp)
void pd_set_damp(int16_t damp) {
    PD_CONTROL.kd = damp;
}
