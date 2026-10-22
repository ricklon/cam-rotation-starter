// Continuous-rotation servo on LEDC: speed -100..100 with stop trim and slew limiting.
#pragma once

void  servo_init(void);
void  servo_set_target(float speed);   // -100..100, ramped by settings.slew
float servo_get_speed(void);           // current (ramped) speed
float servo_get_target(void);
int   servo_pulse_us(void);            // last pulse sent, 0 = released
void  servo_preview_trim(int trim_us); // apply a trim immediately (not saved)
