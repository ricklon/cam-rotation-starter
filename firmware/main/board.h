// Pin maps for the two supported boards: the Seeed XIAO ESP32-C6 and the ESP32-C6 Super Mini
// (menuconfig -> CamBox -> Board, or sdkconfig.supermini). Matches the wiring pages
// (MG90S rev B, N20 rev C).
#pragma once
#include "driver/gpio.h"

#include "sdkconfig.h"

#if CONFIG_CAM_DRIVE_N20
#define DRIVE_NAME        "n20"
#else
#define DRIVE_NAME        "mg90s"
#endif

#if CONFIG_CAM_BOARD_SUPERMINI_C6
// ESP32-C6 Super Mini. Avoids the strapping pins (4, 5, 8, 9, 15), USB (12, 13), UART0 (16 RX,
// 17 TX) and the in-package flash. The driver pins are the same GPIOs as on the XIAO, on the
// bottom edge; the panel pins are on the top edge, the knob on an ADC1 pin.
#define BOARD_NAME        "supermini-c6"
#if CONFIG_CAM_DRIVE_N20
#define PIN_AIN1          GPIO_NUM_18  // DRV8833 IN3 (forward PWM; BIN1)
#define PIN_AIN2          GPIO_NUM_20  // DRV8833 IN4 (reverse PWM; BIN2)
#define PIN_NSLEEP        GPIO_NUM_19  // DRV8833 EEP (nSLEEP); low keeps the driver off. Module jumper J1 open.
#else
#define PIN_SERVO         GPIO_NUM_18  // servo CTRL (through R1)
#endif
#define PIN_BTN           GPIO_NUM_1   // push button to GND (internal pull-up)
#define PIN_KNOB          GPIO_NUM_2   // pot wiper (ADC1 ch2), pot across 3V3/GND
#define PIN_DIR_FWD       GPIO_NUM_3   // switch outer pin A to GND
#define PIN_DIR_REV       GPIO_NUM_0   // switch outer pin B to GND

#define PIN_LED           GPIO_NUM_15  // on-board LED; check the level on the bench (15 is a strapping pin, fine as an output after boot)
#define LED_ON_LEVEL      1
#define HAS_RF_SWITCH     0            // fixed ceramic antenna, no RF switch
#else
#define BOARD_NAME        "xiao-c6"
#if CONFIG_CAM_DRIVE_N20
// The N20 uses the module's channel B (IN3/IN4 -> OUT3/OUT4): those pins sit right under the
// XIAO's bottom row, so the three driver wires drop straight down (drives/n20/docs/wiring.md).
#define PIN_AIN1          GPIO_NUM_18  // D10 DRV8833 IN3 (forward PWM; BIN1)
#define PIN_AIN2          GPIO_NUM_20  // D9  DRV8833 IN4 (reverse PWM; BIN2)
#define PIN_NSLEEP        GPIO_NUM_19  // D8  DRV8833 EEP (nSLEEP); low keeps the driver off. Module jumper J1 open.
#else
#define PIN_SERVO         GPIO_NUM_0   // D0  servo CTRL (through R1)
#endif
#define PIN_BTN           GPIO_NUM_1   // D1  push button to GND (internal pull-up)
#define PIN_KNOB          GPIO_NUM_2   // D2  pot wiper (ADC), pot across 3V3/GND
#define PIN_DIR_FWD       GPIO_NUM_21  // D3  switch outer pin A to GND
#define PIN_DIR_REV       GPIO_NUM_22  // D4  switch outer pin B to GND

#define PIN_LED           GPIO_NUM_15  // XIAO ESP32-C6 user LED (active low)
#define LED_ON_LEVEL      0

// XIAO ESP32-C6 RF switch: GPIO3 low enables it, GPIO14 low selects the on-board antenna.
#define HAS_RF_SWITCH     1
#define PIN_RF_SWITCH_EN  GPIO_NUM_3
#define PIN_ANT_SELECT    GPIO_NUM_14
#endif
