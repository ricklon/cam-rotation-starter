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
// ESP32-C6 Super Mini. Avoids USB (12, 13), UART0 (16 RX, 17 TX), the in-package flash and every
// strapping pin (5, 8, 9, 15) except GPIO4. The driver pins are the same GPIOs as on the XIAO, on
// the bottom edge; the panel pins are on the top edge. The pot's pins go straight into the strips of
// GPIO4 and GPIO3 (drives/n20/docs/perfboard.md), so GPIO3 is the pot's ground.
#define BOARD_NAME        "supermini-c6"
#if CONFIG_CAM_DRIVE_N20
#define PIN_AIN1          GPIO_NUM_18  // DRV8833 IN3 (forward PWM; BIN1)
#define PIN_AIN2          GPIO_NUM_20  // DRV8833 IN4 (reverse PWM; BIN2)
#define PIN_NSLEEP        GPIO_NUM_19  // DRV8833 EEP (nSLEEP); low keeps the driver off. Module jumper J1 open.
#else
#define PIN_SERVO         GPIO_NUM_18  // servo CTRL (through R1)
#endif
#define PIN_BTN           GPIO_NUM_1   // push button to GND (internal pull-up)
// GPIO4 is a strapping pin (MTMS), but on the C6 it only picks the JTAG source, and only when the
// JTAG_SEL_ENABLE eFuse is burned (it isn't by default). Otherwise its level at reset is ignored,
// so the wiper's position at boot doesn't matter.
#define PIN_KNOB          GPIO_NUM_4   // pot wiper (ADC1 ch4)
#define PIN_KNOB_GND      GPIO_NUM_3   // pot's low end; driven low as an output, so the pot runs 3V3 -> GPIO3
#define KNOB_REVERSED     1            // the pot's clockwise end is on GPIO3, so invert: clockwise is faster
// No direction switch on this board (the N20 rev C build has the knob and the button only): switch_installed is
// always false, and direction is forward. Its old pins, GPIO2 and GPIO0, now take the N20's Hall encoder.
#define BOARD_HAS_DIR_SWITCH 0
#if CONFIG_CAM_DRIVE_N20
// Hall encoder on the N20 (drives/n20/README.md, "Encoder motor"), powered from 3V3. Counted in x4 quadrature by
// the PCNT unit with the glitch filter on; the internal pull-ups back up the encoder board's own.
#define HAS_ENCODER       1
#define PIN_ENC_A         GPIO_NUM_0   // encoder C1 (green), channel A
#define PIN_ENC_B         GPIO_NUM_2   // encoder C2 (yellow), channel B
#define ENC_COUNTS_PER_REV 4200        // 7 pulses per motor turn x 150:1 gearbox x 4 edges, per output-shaft turn
#define ENC_REVERSED      0            // 1 if the count runs backwards while the motor turns forward
#endif

#define PIN_LED           GPIO_NUM_15  // on-board LED; check the level on the bench (15 is a strapping pin, fine as an output after boot)
#define LED_ON_LEVEL      1
#define HAS_RF_SWITCH     0            // fixed ceramic antenna, no RF switch
#else
#if CONFIG_CAM_BOARD_XIAO_C6_BARE
#define BOARD_NAME        "xiao-c6-bare"
#else
#define BOARD_NAME        "xiao-c6"
#endif
#if CONFIG_CAM_DRIVE_N20
// The N20 uses the module's channel B (IN3/IN4 -> OUT3/OUT4): those pins sit right under the
// XIAO's bottom row, so the three driver wires drop straight down (drives/n20/docs/wiring.md).
#define PIN_AIN1          GPIO_NUM_18  // D10 DRV8833 IN3 (forward PWM; BIN1)
#define PIN_AIN2          GPIO_NUM_20  // D9  DRV8833 IN4 (reverse PWM; BIN2)
#define PIN_NSLEEP        GPIO_NUM_19  // D8  DRV8833 EEP (nSLEEP); low keeps the driver off. Module jumper J1 open.
#else
#define PIN_SERVO         GPIO_NUM_0   // D0  servo CTRL (through R1)
#endif
#if CONFIG_CAM_BOARD_XIAO_C6_BARE
// Bare XIAO (no panel fitted): the on-board BOOT button (GPIO9) is the panel button, and there is
// no knob or direction switch; use the web virtual panel for those. GPIO9 is a strapping pin: held
// low at reset it enters the USB bootloader, so a boot-time hold can't reset Wi-Fi. A 10 s hold while
// running does instead (BOARD_BTN_IS_BOOT).
#define PIN_BTN           GPIO_NUM_9   // BOOT button to GND (external pull-up on the board)
#define BOARD_BTN_IS_BOOT 1
#define BOARD_HAS_KNOB    0
#define BOARD_HAS_DIR_SWITCH 0
#define PIN_KNOB          GPIO_NUM_2   // unused (no knob); kept so the build links
#define KNOB_REVERSED     0
#else
#define PIN_BTN           GPIO_NUM_1   // D1  push button to GND (internal pull-up)
#define PIN_KNOB          GPIO_NUM_2   // D2  pot wiper (ADC), pot across 3V3/GND
#define KNOB_REVERSED     0
#define PIN_DIR_FWD       GPIO_NUM_21  // D3  switch outer pin A to GND
#define PIN_DIR_REV       GPIO_NUM_22  // D4  switch outer pin B to GND
#define BOARD_HAS_DIR_SWITCH 1
#endif

#define PIN_LED           GPIO_NUM_15  // XIAO ESP32-C6 user LED (active low)
#define LED_ON_LEVEL      0

// XIAO ESP32-C6 RF switch: GPIO3 low enables it, GPIO14 low selects the on-board antenna.
#define HAS_RF_SWITCH     1
#define PIN_RF_SWITCH_EN  GPIO_NUM_3
#define PIN_ANT_SELECT    GPIO_NUM_14
#endif

// Defaults for boards that don't say otherwise.
#ifndef BOARD_HAS_KNOB
#define BOARD_HAS_KNOB    1
#endif
#ifndef BOARD_BTN_IS_BOOT
#define BOARD_BTN_IS_BOOT 0
#endif

#ifndef HAS_ENCODER
#define HAS_ENCODER       0            // only the Super Mini N20 build has an encoder wired
#endif
