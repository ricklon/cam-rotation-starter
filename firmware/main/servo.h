// Motor output: speed -100..100 with slew limiting. servo.c drives a continuous-rotation
// servo (stop trim, pulse width); motor_drv8833.c drives an N20 gearmotor through a DRV8833.
#pragma once
#include <stdbool.h>

void  servo_init(void);
void  servo_set_target(float speed);   // -100..100, ramped by settings.slew
float servo_get_speed(void);           // current (ramped) speed
float servo_get_target(void);
int   servo_pulse_us(void);            // servo: last pulse sent, 0 = released; N20: always 0
int   servo_duty_pct(void);            // N20: signed PWM duty -100..100, 0 = braked or asleep; servo: always 0
void  servo_preview_trim(int trim_us); // servo: apply a trim immediately (not saved); N20: ignored
void  servo_set_immediate(float speed); // closed-loop use only (hand-recorded stories): no slew, -100..100
void  servo_release(bool free);         // true: let the shaft turn freely by hand (N20: driver asleep, outputs open;
                                        // servo: no pulses) and ignore targets, until called with false
