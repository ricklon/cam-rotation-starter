// Physical controls: push button (D1), speed knob (D2), ON-OFF-ON direction switch (D3/D4).
#pragma once
#include <stdbool.h>

typedef enum {
    CTL_BTN_SHORT,   // start / pause
    CTL_BTN_DOUBLE,  // next story
    CTL_BTN_LONG,    // stop (2 s hold)
    CTL_KNOB,        // value 0..100, only sent when the knob actually moves
    CTL_DIR,         // value -1 reverse, 0 stop (center), +1 forward
} ctl_event_t;

typedef void (*ctl_cb_t)(ctl_event_t ev, int value);

void controls_init(ctl_cb_t cb);
int  controls_dir(void);          // -1 / 0 / +1 (the virtual switch when settings.switch_installed is off)
bool controls_set_virtual_dir(int d);   // web virtual switch; false (ignored) while a physical switch is wired
int  controls_knob(void);         // 0..100
bool controls_button_down(void);  // raw, for the boot-time Wi-Fi reset check
