// The "director": decides the servo's target from stories, live web control and the physical controls.
//
// Rules
//  - The direction switch gates everything: center = stop, REV = reverse whatever is commanded.
//  - Manual mode: speed comes from the web slider or the knob, whichever moved last.
//  - Story mode: speed comes from the story's keyframes; the knob becomes a master speed (0..100 %).
//  - Button: short = play/pause, double = next story, long = stop.
#pragma once
#include <stdbool.h>
#include "esp_err.h"
#include "cJSON.h"
#include "controls.h"

#define STORY_MAX_STEPS 64
#define PLAYLIST_MAX    16

typedef struct {
    char  label[32];
    float speed;     // -100..100
    int   ms;        // how long to hold this beat (after the ramp)
    int   ramp_ms;   // time to ramp from the previous speed into this one
} story_step_t;

typedef struct {
    char name[32];
    bool loop;
    int  count;
    story_step_t steps[STORY_MAX_STEPS];
} story_t;

void      player_init(void);
bool      story_from_json(const cJSON *j, story_t *out, char *err, int errlen);

esp_err_t player_play(const char *name);
esp_err_t player_play_playlist(const char *name);   // stories in order, one pass each, then round again
void      player_toggle_pause(void);
void      player_stop(void);
void      player_next(void);
void      player_manual(float speed);   // live web control; leaves story mode
void      player_set_master(int pct);   // 0..100

void      player_on_control(ctl_event_t ev, int value);
cJSON    *player_status(void);

// Teach: record a story by turning the shaft by hand (N20 with encoder only; ESP_ERR_NOT_SUPPORTED otherwise).
// Start releases the motor and samples the encoder (up to 120 s); stop returns the take as a story
// ({"name","loop","steps","track"}, not saved) with the still time at each end trimmed, or NULL and err.
// A story with a "track" plays it closed-loop: the shaft follows the recorded positions.
esp_err_t player_teach_start(void);
cJSON    *player_teach_stop(const char *name, bool loop, char *err, int errlen);
void      player_teach_cancel(void);
bool      player_teaching(void);
