#ifndef PD_H_
#define PD_H_

#include <stdint.h>
#include <stdbool.h>

extern volatile bool IS_LOGGING;

/// Run the PD-Control Loop.
void pd_loop(void);

/// Add offset to the setpoint.
void pd_add_setpoint(int32_t offset);

/// Set KP (gain)
void pd_set_gain(int32_t gain);

/// Set KD (damp)
void pd_set_damp(int32_t damp);

/// Print out the current state of the PID.
void view_current_values(void);

#endif
