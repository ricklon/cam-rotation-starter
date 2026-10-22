#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "player.h"
#include "servo.h"
#include "storage.h"
#include "settings.h"
#include "controls.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_timer.h"

#define TICK_MS 20

static const char *TAG = "player";

typedef enum { MODE_MANUAL, MODE_STORY } mode_t_;

static SemaphoreHandle_t s_lock;
static story_t s_story;
static bool    s_loaded = false;
static mode_t_ s_mode = MODE_MANUAL;
static bool    s_paused = false;
static bool    s_resting = false;
static int     s_step = 0;
static int     s_step_elapsed = 0;   // ms into the current step (ramp + hold)
static float   s_step_from = 0;      // speed at the start of the current step's ramp
static int     s_loops = 0;
static int     s_rest_left = 0;
static float   s_story_speed = 0;    // keyframe speed before master/direction
static float   s_manual = 0;         // -100..100
static int     s_master = 100;       // %

static float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

bool story_from_json(const cJSON *j, story_t *out, char *err, int errlen)
{
    memset(out, 0, sizeof(*out));
    if (!cJSON_IsObject(j)) { snprintf(err, errlen, "story must be a JSON object"); return false; }
    const cJSON *name = cJSON_GetObjectItemCaseSensitive(j, "name");
    if (cJSON_IsString(name)) strncpy(out->name, name->valuestring, sizeof(out->name) - 1);
    const cJSON *loop = cJSON_GetObjectItemCaseSensitive(j, "loop");
    out->loop = cJSON_IsBool(loop) ? cJSON_IsTrue(loop) : true;
    const cJSON *steps = cJSON_GetObjectItemCaseSensitive(j, "steps");
    if (!cJSON_IsArray(steps) || cJSON_GetArraySize(steps) == 0) { snprintf(err, errlen, "story needs a non-empty \"steps\" array"); return false; }
    const cJSON *st;
    cJSON_ArrayForEach(st, steps) {
        if (out->count >= STORY_MAX_STEPS) { snprintf(err, errlen, "too many steps (max %d)", STORY_MAX_STEPS); return false; }
        story_step_t *s = &out->steps[out->count];
        const cJSON *lb = cJSON_GetObjectItemCaseSensitive(st, "label");
        const cJSON *sp = cJSON_GetObjectItemCaseSensitive(st, "speed");
        const cJSON *ms = cJSON_GetObjectItemCaseSensitive(st, "ms");
        const cJSON *rp = cJSON_GetObjectItemCaseSensitive(st, "ramp");
        if (!cJSON_IsNumber(sp) || !cJSON_IsNumber(ms)) {
            snprintf(err, errlen, "step %d needs numeric \"speed\" and \"ms\"", out->count + 1);
            return false;
        }
        if (cJSON_IsString(lb)) strncpy(s->label, lb->valuestring, sizeof(s->label) - 1);
        s->speed = clampf((float)sp->valuedouble, -100, 100);
        s->ms = ms->valueint < 0 ? 0 : (ms->valueint > 600000 ? 600000 : ms->valueint);
        s->ramp_ms = cJSON_IsNumber(rp) ? (rp->valueint < 0 ? 0 : (rp->valueint > 60000 ? 60000 : rp->valueint)) : 300;
        out->count++;
    }
    return true;
}

static esp_err_t load_story(const char *name)
{
    char *txt = storage_read_story(name);
    if (!txt) return ESP_ERR_NOT_FOUND;
    cJSON *j = cJSON_Parse(txt);
    free(txt);
    char err[80];
    story_t tmp;
    bool ok = story_from_json(j, &tmp, err, sizeof err);
    cJSON_Delete(j);
    if (!ok) { ESP_LOGW(TAG, "%s: %s", name, err); return ESP_ERR_INVALID_ARG; }
    strncpy(tmp.name, name, sizeof(tmp.name) - 1);
    s_story = tmp;
    s_loaded = true;
    return ESP_OK;
}

static void start_step(int i)
{
    s_step = i;
    s_step_elapsed = 0;
    s_step_from = s_story_speed;
}

esp_err_t player_play(const char *name)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
    esp_err_t err = ESP_OK;
    if (name && name[0] && (!s_loaded || strcmp(name, s_story.name) != 0)) err = load_story(name);
    if (err == ESP_OK && s_loaded) {
        if (s_mode != MODE_STORY || !s_paused) { s_loops = 0; s_resting = false; s_story_speed = 0; start_step(0); }
        s_mode = MODE_STORY;
        s_paused = false;
        ESP_LOGI(TAG, "playing %s (%d steps)", s_story.name, s_story.count);
    } else if (err == ESP_OK) {
        err = ESP_ERR_NOT_FOUND;
    }
    xSemaphoreGive(s_lock);
    return err;
}

void player_toggle_pause(void)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
    if (s_mode == MODE_STORY) s_paused = !s_paused;
    xSemaphoreGive(s_lock);
    if (s_mode != MODE_STORY) {
        // nothing playing: start the autoplay story, or the first story on the device
        const char *name = g_settings.autoplay_story;
        cJSON *list = NULL;
        if (!name[0] && !s_loaded) {
            list = storage_list_stories();
            cJSON *first = cJSON_GetArrayItem(list, 0);
            name = cJSON_IsString(first) ? first->valuestring : "";
        }
        player_play(name);
        cJSON_Delete(list);
    }
}

void player_stop(void)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_mode = MODE_MANUAL;
    s_paused = false;
    s_resting = false;
    s_manual = 0;
    s_story_speed = 0;
    xSemaphoreGive(s_lock);
}

void player_next(void)
{
    cJSON *list = storage_list_stories();
    int n = cJSON_GetArraySize(list), idx = -1;
    for (int i = 0; i < n && s_loaded; i++) {
        if (strcmp(cJSON_GetArrayItem(list, i)->valuestring, s_story.name) == 0) { idx = i; break; }
    }
    if (n > 0) player_play(cJSON_GetArrayItem(list, (idx + 1) % n)->valuestring);
    cJSON_Delete(list);
}

void player_manual(float speed)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_mode = MODE_MANUAL;
    s_paused = false;
    s_manual = clampf(speed, -100, 100);
    xSemaphoreGive(s_lock);
}

void player_set_master(int pct)
{
    s_master = pct < 0 ? 0 : (pct > 100 ? 100 : pct);
}

void player_on_control(ctl_event_t ev, int value)
{
    switch (ev) {
    case CTL_BTN_SHORT:  player_toggle_pause(); break;
    case CTL_BTN_DOUBLE: player_next(); break;
    case CTL_BTN_LONG:   player_stop(); break;
    case CTL_KNOB:
        if (s_mode == MODE_STORY) player_set_master(value);
        else player_manual((float)value);   // magnitude; the switch sets direction
        break;
    case CTL_DIR:
        if (value == 0 && s_mode == MODE_STORY) s_paused = true;      // center pauses a story
        else if (value != 0 && s_mode == MODE_STORY && s_paused) s_paused = false;
        break;
    }
}

static void advance_story(int dt)
{
    if (!s_loaded || s_story.count == 0) return;
    if (s_resting) {
        s_rest_left -= dt;
        s_story_speed = 0;
        if (s_rest_left <= 0) { s_resting = false; start_step(0); }
        return;
    }
    story_step_t *st = &s_story.steps[s_step];
    s_step_elapsed += dt;
    if (st->ramp_ms > 0 && s_step_elapsed < st->ramp_ms) {
        float f = (float)s_step_elapsed / st->ramp_ms;
        s_story_speed = s_step_from + (st->speed - s_step_from) * f;
    } else {
        s_story_speed = st->speed;
    }
    if (s_step_elapsed >= st->ramp_ms + st->ms) {
        if (s_step + 1 < s_story.count) { start_step(s_step + 1); return; }
        s_loops++;
        if (!s_story.loop) { s_mode = MODE_MANUAL; s_manual = 0; s_story_speed = 0; ESP_LOGI(TAG, "story finished"); return; }
        if (g_settings.rest_every > 0 && s_loops % g_settings.rest_every == 0 && g_settings.rest_ms > 0) {
            s_resting = true;
            s_rest_left = g_settings.rest_ms;
            s_story_speed = 0;
            return;
        }
        start_step(0);
    }
}

static void player_task(void *arg)
{
    TickType_t last = xTaskGetTickCount();
    for (;;) {
        vTaskDelayUntil(&last, pdMS_TO_TICKS(TICK_MS));
        xSemaphoreTake(s_lock, portMAX_DELAY);
        int dir = controls_dir();
        float cmd;
        if (s_mode == MODE_STORY) {
            if (!s_paused && dir != 0) advance_story(TICK_MS);
            cmd = (s_paused || dir == 0) ? 0 : s_story_speed * s_master / 100.0f;
        } else {
            cmd = s_manual;
        }
        servo_set_target(cmd * dir);
        xSemaphoreGive(s_lock);
    }
}

cJSON *player_status(void)
{
    cJSON *j = cJSON_CreateObject();
    xSemaphoreTake(s_lock, portMAX_DELAY);
    cJSON_AddStringToObject(j, "mode", s_mode == MODE_STORY ? "story" : "manual");
    cJSON_AddBoolToObject(j, "paused", s_paused);
    cJSON_AddBoolToObject(j, "resting", s_resting);
    cJSON_AddStringToObject(j, "story", s_loaded ? s_story.name : "");
    cJSON_AddNumberToObject(j, "step", s_step);
    cJSON_AddNumberToObject(j, "steps", s_loaded ? s_story.count : 0);
    cJSON_AddStringToObject(j, "step_label", (s_loaded && s_step < s_story.count) ? s_story.steps[s_step].label : "");
    cJSON_AddNumberToObject(j, "loops", s_loops);
    cJSON_AddNumberToObject(j, "manual", s_manual);
    cJSON_AddNumberToObject(j, "master", s_master);
    xSemaphoreGive(s_lock);
    cJSON_AddNumberToObject(j, "dir", controls_dir());
    cJSON_AddNumberToObject(j, "knob", controls_knob());
    cJSON_AddNumberToObject(j, "speed", roundf(servo_get_speed() * 10) / 10);
    cJSON_AddNumberToObject(j, "target", roundf(servo_get_target() * 10) / 10);
    cJSON_AddNumberToObject(j, "pulse_us", servo_pulse_us());
    cJSON_AddNumberToObject(j, "uptime_s", (double)(esp_timer_get_time() / 1000000));
    return j;
}

void player_init(void)
{
    s_lock = xSemaphoreCreateMutex();
    xTaskCreate(player_task, "player", 4096, NULL, 9, NULL);
    if (g_settings.autoplay && g_settings.autoplay_story[0]) {
        if (player_play(g_settings.autoplay_story) == ESP_OK) ESP_LOGI(TAG, "gallery mode: autoplay %s", g_settings.autoplay_story);
        else ESP_LOGW(TAG, "autoplay story \"%s\" not found", g_settings.autoplay_story);
    }
}
