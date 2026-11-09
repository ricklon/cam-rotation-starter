// Persistent settings, stored as one JSON blob in NVS.
#pragma once
#include <stdbool.h>
#include "esp_err.h"
#include "cJSON.h"

#define WIFI_MAX_NETS 4

typedef struct {
    char ssid[33];
    char pass[65];         // empty = open network
} wifi_net_t;

typedef struct {
    // Servo calibration
    int  trim_us;          // pulse that stops the continuous servo (default 1500)
    int  range_us;         // pulse offset at full speed (default 400 -> 1100..1900)
    int  slew;             // max speed change, % per second (smooth starts/stops)
    int  idle_release_ms;  // stop sending pulses after this long at zero (0 = never)
    // Hand-recorded stories (N20 with encoder): following the recorded motion
    int  max_rpm;          // output-shaft rpm at 100 % duty on 5 V: turns recorded speeds into beats, and feeds forward
    int  min_duty;         // % added to any non-zero follow command to get past the gearbox's friction
    int  follow_gain;      // % of duty per 100 counts (about 8.6 degrees) the shaft is off the recording
    // Physical controls
    bool knob_enabled;
    bool switch_installed; // false: treat direction as forward when the switch is absent
    // Gallery
    bool autoplay;
    char autoplay_story[32];
    int  rest_every;       // rest after this many loops (0 = never)
    int  rest_ms;
    // Network
    char ap_ssid[33];      // empty = CamBox-XXXX from the MAC
    char ap_pass[65];      // empty = open network
    wifi_net_t nets[WIFI_MAX_NETS];   // networks to join, tried in order; none = access point only
    int  net_count;
    char hostname[32];     // mDNS name, e.g. cambox -> cambox.local
} settings_t;

extern settings_t g_settings;

void       settings_load(void);
esp_err_t  settings_save(void);
void       settings_reset_network(void);
cJSON     *settings_to_json(bool include_secrets);
void       settings_apply_json(const cJSON *j);
