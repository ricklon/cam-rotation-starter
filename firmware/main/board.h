// Pin map for the Seeed XIAO ESP32-C6 build. Matches the wiring page (rev B).
#pragma once
#include "driver/gpio.h"

#define PIN_SERVO         GPIO_NUM_0   // D0  servo CTRL (through R1)
#define PIN_BTN           GPIO_NUM_1   // D1  push button to GND (internal pull-up)
#define PIN_KNOB          GPIO_NUM_2   // D2  pot wiper (ADC), pot across 3V3/GND
#define PIN_DIR_FWD       GPIO_NUM_21  // D3  switch outer pin A to GND
#define PIN_DIR_REV       GPIO_NUM_22  // D4  switch outer pin B to GND

#define PIN_LED           GPIO_NUM_15  // XIAO ESP32-C6 user LED (active low)
#define LED_ON_LEVEL      0

// XIAO ESP32-C6 RF switch: GPIO3 low enables it, GPIO14 low selects the on-board antenna.
#define PIN_RF_SWITCH_EN  GPIO_NUM_3
#define PIN_ANT_SELECT    GPIO_NUM_14
