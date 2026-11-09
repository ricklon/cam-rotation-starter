// Quadrature Hall encoder on the N20's motor shaft (Super Mini N20 build; board.h HAS_ENCODER).
// Counted in hardware by the PCNT peripheral, every edge of both channels (x4). Positions and speeds
// are of the output shaft, ENC_COUNTS_PER_REV counts per turn. Without an encoder every call is a
// no-op returning 0, and encoder_status() returns NULL.
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "cJSON.h"

void encoder_init(void);
bool encoder_present(void);       // built with an encoder (not whether one is plugged in)
int32_t encoder_count(void);      // counts since power-up or the last encoder_zero(); + is forward
float encoder_turns(void);        // output-shaft turns, count / ENC_COUNTS_PER_REV
float encoder_degrees(void);      // shaft angle within the turn, 0..360
float encoder_rpm(void);          // output-shaft speed, smoothed over about 0.3 s; + is forward
void encoder_zero(void);          // make the current position count 0
cJSON *encoder_status(void);      // {count, turns, deg, rpm, cpr}, or NULL without an encoder
