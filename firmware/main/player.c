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
#include "board.h"
#include "encoder.h"
#include "track.h"

#define TICK_MS 20

static const char *TAG = "player";

typedef enum { MODE_MANUAL, MODE_STORY, MODE_TEACH } mode_t_;

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
static char    s_pl_name[32] = "";   // active playlist ("" = none)
static char    s_pl[PLAYLIST_MAX][32];
static int     s_pl_n = 0, s_pl_i = 0;

// Hand-recorded stories. A story with a "track" plays it closed-loop when there's an encoder: the shaft
// follows the recorded positions (time scaled by the master speed), instead of the beats' open-loop speeds.
static track_t s_track;              // the loaded story's track (n = 0: none)
static bool    s_follow = false;     // playing s_track with the encoder
static float   s_tr_ms = 0;          // time into the take
#if HAS_ENCODER
static int32_t s_tr_base = 0;        // encoder count where this pass of the take started
#endif
static float   s_tr_err = 0;         // target - measured, counts
static int     s_stall_ms = 0;
static char    s_fault[80] = "";     // why the last followed story was stopped
// Teach mode: the driver is released and the encoder is sampled every TEACH_EVERY ticks into s_rec.
#define TEACH_EVERY (1000 / TRACK_HZ / TICK_MS)
static int32_t *s_rec = NULL;
static int     s_rec_n = 0;
#if HAS_ENCODER
static int     s_rec_tick = 0;
static int32_t s_rec_base = 0;
#endif
static bool    s_rec_full = false;

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
    cJSON *j = cJSON_Parse(txt);          // a track is one string item, so this stays small
    free(txt);
    char err[80];
    story_t *tmp = malloc(sizeof *tmp);   // ~3 KB: too big for the player task's stack
    if (!tmp) { cJSON_Delete(j); return ESP_ERR_NO_MEM; }
    bool ok = story_from_json(j, tmp, err, sizeof err);
    if (!ok) { ESP_LOGW(TAG, "%s: %s", name, err); cJSON_Delete(j); free(tmp); return ESP_ERR_INVALID_ARG; }
    strncpy(tmp->name, name, sizeof(tmp->name) - 1);
    track_t tr;
    if (!track_from_json(j, &tr, err, sizeof err)) { ESP_LOGW(TAG, "%s: %s; playing its beats", name, err); memset(&tr, 0, sizeof tr); }
    cJSON_Delete(j);
    s_story = *tmp;
    free(tmp);
    track_free(&s_track);
    s_track = tr;
    s_loaded = true;
    return ESP_OK;
}

static void start_step(int i)
{
    s_step = i;
    s_step_elapsed = 0;
    s_step_from = s_story_speed;
}

// Leave teach mode: the driver takes commands again and the take is dropped (player_teach_stop takes it first).
static void teach_end_locked(void)
{
    if (s_mode != MODE_TEACH) return;
    servo_release(false);
    free(s_rec);
    s_rec = NULL;
    s_mode = MODE_MANUAL;
    s_manual = 0;
}

static void follow_begin(void)
{
#if HAS_ENCODER
    s_follow = s_track.n > 1 && encoder_present();
#else
    s_follow = false;
#endif
    s_tr_ms = 0;
    s_tr_err = 0;
    s_stall_ms = 0;
#if HAS_ENCODER
    s_tr_base = encoder_count();
#endif
}

static esp_err_t play_locked(const char *name)
{
    teach_end_locked();
    esp_err_t err = ESP_OK;
    if (name && name[0] && (!s_loaded || strcmp(name, s_story.name) != 0)) err = load_story(name);
    if (err == ESP_OK && s_loaded) {
        if (s_mode != MODE_STORY || !s_paused) { s_loops = 0; s_resting = false; s_story_speed = 0; start_step(0); follow_begin(); s_fault[0] = 0; }
        s_mode = MODE_STORY;
        s_paused = false;
        ESP_LOGI(TAG, "playing %s (%d steps%s)", s_story.name, s_story.count, s_follow ? ", following its hand-recorded track" : "");
    } else if (err == ESP_OK) {
        err = ESP_ERR_NOT_FOUND;
    }
    return err;
}

esp_err_t player_play(const char *name)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_pl_n = 0;
    s_pl_name[0] = 0;
    esp_err_t err = play_locked(name);
    xSemaphoreGive(s_lock);
    return err;
}

// Move to the next story in the active playlist, skipping any that no longer load. Keeps the current
// speed so the motor doesn't stop between stories. Returns false (and stops) if none can be played.
static bool playlist_advance_locked(void)
{
    float keep = s_story_speed;
    for (int tries = 0; tries < s_pl_n; tries++) {
        s_pl_i = (s_pl_i + 1) % s_pl_n;
        if (play_locked(s_pl[s_pl_i]) == ESP_OK) { s_story_speed = keep; s_step_from = keep; return true; }
    }
    s_pl_n = 0;
    s_pl_name[0] = 0;
    s_mode = MODE_MANUAL;
    s_manual = 0;
    s_story_speed = 0;
    return false;
}

esp_err_t player_play_playlist(const char *name)
{
    char *txt = storage_read_playlist(name);
    if (!txt) return ESP_ERR_NOT_FOUND;
    cJSON *j = cJSON_Parse(txt);
    free(txt);
    const cJSON *arr = cJSON_GetObjectItemCaseSensitive(j, "stories");
    char list[PLAYLIST_MAX][32];
    int n = 0;
    const cJSON *it;
    cJSON_ArrayForEach(it, arr) {
        if (n >= PLAYLIST_MAX) break;
        if (cJSON_IsString(it) && storage_valid_name(it->valuestring)) strcpy(list[n++], it->valuestring);
    }
    cJSON_Delete(j);
    if (n == 0) return ESP_ERR_INVALID_ARG;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    memcpy(s_pl, list, sizeof s_pl);
    s_pl_n = n;
    s_pl_i = n - 1;                       // advance lands on the first story
    strncpy(s_pl_name, name, sizeof s_pl_name - 1);
    s_pl_name[sizeof s_pl_name - 1] = 0;
    s_mode = MODE_MANUAL;                 // start from the top even if a story was paused
    esp_err_t err = playlist_advance_locked() ? ESP_OK : ESP_ERR_NOT_FOUND;
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
    teach_end_locked();
    s_mode = MODE_MANUAL;
    s_paused = false;
    s_resting = false;
    s_manual = 0;
    s_story_speed = 0;
    s_pl_n = 0;
    s_pl_name[0] = 0;
    xSemaphoreGive(s_lock);
}

void player_next(void)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
    if (s_pl_n > 0) { playlist_advance_locked(); xSemaphoreGive(s_lock); return; }
    xSemaphoreGive(s_lock);
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
    teach_end_locked();
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
        if (s_mode == MODE_TEACH) break;   // the shaft is being turned by hand
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
        if (s_pl_n > 0) { playlist_advance_locked(); return; }   // playlists play each story once
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

#if HAS_ENCODER
// Stop a followed story that can't keep up (stalled, or an encoder counting the wrong way).
static void follow_fault(const char *why)
{
    snprintf(s_fault, sizeof s_fault, "%s", why);
    ESP_LOGW(TAG, "%s: stopped (%s)", s_story.name, why);
    s_mode = MODE_MANUAL;
    s_manual = 0;
    s_story_speed = 0;
    s_pl_n = 0;
    s_pl_name[0] = 0;
}

// The beat whose time span holds t (for the status display).
static void follow_beat(float t)
{
    int acc = 0;
    for (int i = 0; i < s_story.count; i++) {
        acc += s_story.steps[i].ramp_ms + s_story.steps[i].ms;
        if (t < acc || i == s_story.count - 1) { s_step = i; return; }
    }
}

// One tick of following the track: returns the duty to apply, -100..100. Time runs at the master speed,
// so the knob slows the whole motion down rather than changing its shape.
static float follow_tick(int dt)
{
    const float cps_full = g_settings.max_rpm * (float)ENC_COUNTS_PER_REV / 60.0f;
    const float rate = s_master / 100.0f;
    const int last = s_track.n - 1;
    if (s_resting) {
        s_rest_left -= dt;
        if (s_rest_left <= 0) { s_resting = false; s_tr_ms = 0; }
        return 0;
    }
    s_tr_ms += dt * rate;
    float idx = s_tr_ms * s_track.hz / 1000.0f;
    if (idx >= last) {                                  // end of the take
        s_tr_base += s_track.pos[last];                 // carry on from where the take ended, not where the shaft is
        if (s_pl_n > 0) { playlist_advance_locked(); return 0; }
        s_loops++;
        if (!s_story.loop) { s_mode = MODE_MANUAL; s_manual = 0; s_story_speed = 0; ESP_LOGI(TAG, "story finished"); return 0; }
        s_tr_ms = 0;
        idx = 0;
        if (g_settings.rest_every > 0 && s_loops % g_settings.rest_every == 0 && g_settings.rest_ms > 0) {
            s_resting = true;
            s_rest_left = g_settings.rest_ms;
            return 0;
        }
    }
    int i = (int)idx;
    float f = idx - i;
    float d = (float)(s_track.pos[i + 1] - s_track.pos[i]);
    float target = s_tr_base + s_track.pos[i] + d * f;
    float vff = d * s_track.hz * rate;                  // counts per second the recording moves here
    float err = target - (float)encoder_count();
    s_tr_err = err;
    follow_beat(s_tr_ms);
    if (fabsf(err) > 2.0f * ENC_COUNTS_PER_REV) { follow_fault("the shaft is over 2 turns off the recording: check ENC_REVERSED"); return 0; }
    s_stall_ms = fabsf(err) > ENC_COUNTS_PER_REV / 2.0f ? s_stall_ms + dt : 0;
    if (s_stall_ms > 2000) { follow_fault("the shaft couldn't keep up for 2 s: stalled, or the cam is too heavy"); return 0; }
    if (fabsf(vff) < 1.0f && fabsf(err) < 15.0f) return 0;   // still in the recording and close enough: rest
    float cmd = vff / cps_full * 100.0f + g_settings.follow_gain * err / 100.0f;
    cmd += (cmd > 0 ? 1.0f : -1.0f) * g_settings.min_duty;
    return clampf(cmd, -100, 100);
}

static void teach_sample(void)
{
    if (!s_rec || s_rec_full) return;
    if (++s_rec_tick < TEACH_EVERY) return;
    s_rec_tick = 0;
    s_rec[s_rec_n++] = encoder_count() - s_rec_base;
    if (s_rec_n > TRACK_MAX) { s_rec_full = true; ESP_LOGI(TAG, "teach: take full (%d s)", TRACK_MAX / TRACK_HZ); }
}
#endif

static void player_task(void *arg)
{
    TickType_t last = xTaskGetTickCount();
    for (;;) {
        vTaskDelayUntil(&last, pdMS_TO_TICKS(TICK_MS));
        xSemaphoreTake(s_lock, portMAX_DELAY);
        int dir = controls_dir();
        float cmd;
#if HAS_ENCODER
        if (s_mode == MODE_TEACH) {             // the driver is released: just sample
            teach_sample();
            xSemaphoreGive(s_lock);
            continue;
        }
        if (s_mode == MODE_STORY && s_follow) {
            cmd = (!s_paused && dir != 0) ? follow_tick(TICK_MS) : 0;
            if (s_mode == MODE_STORY && s_follow) servo_set_immediate(cmd);
            else servo_set_target(s_mode == MODE_STORY ? s_story_speed * s_master / 100.0f : s_manual);
            xSemaphoreGive(s_lock);
            continue;
        }
#endif
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
    cJSON_AddStringToObject(j, "mode", s_mode == MODE_STORY ? "story" : s_mode == MODE_TEACH ? "teach" : "manual");
    cJSON_AddBoolToObject(j, "paused", s_paused);
    cJSON_AddBoolToObject(j, "resting", s_resting);
    cJSON_AddStringToObject(j, "story", s_loaded ? s_story.name : "");
    cJSON_AddNumberToObject(j, "step", s_step);
    cJSON_AddNumberToObject(j, "steps", s_loaded ? s_story.count : 0);
    cJSON_AddStringToObject(j, "step_label", (s_loaded && s_step < s_story.count) ? s_story.steps[s_step].label : "");
    cJSON_AddNumberToObject(j, "loops", s_loops);
    cJSON_AddStringToObject(j, "playlist", s_pl_name);
    cJSON_AddNumberToObject(j, "pl_pos", s_pl_n ? s_pl_i + 1 : 0);
    cJSON_AddNumberToObject(j, "pl_n", s_pl_n);
    cJSON_AddNumberToObject(j, "manual", s_manual);
    cJSON_AddNumberToObject(j, "master", s_master);
    cJSON_AddBoolToObject(j, "follow", s_mode == MODE_STORY && s_follow);
    if (s_mode == MODE_STORY && s_follow) cJSON_AddNumberToObject(j, "follow_err", lroundf(s_tr_err));
    if (s_fault[0]) cJSON_AddStringToObject(j, "fault", s_fault);
    if (s_mode == MODE_TEACH) {
        cJSON *tj = cJSON_AddObjectToObject(j, "teach");
        cJSON_AddNumberToObject(tj, "ms", s_rec_n > 1 ? (s_rec_n - 1) * 1000 / TRACK_HZ : 0);
        cJSON_AddNumberToObject(tj, "max_ms", TRACK_MAX * 1000 / TRACK_HZ);
        cJSON_AddBoolToObject(tj, "full", s_rec_full);
    }
    xSemaphoreGive(s_lock);
    cJSON_AddNumberToObject(j, "dir", controls_dir());
    cJSON_AddBoolToObject(j, "switch_installed", g_settings.switch_installed);
    cJSON_AddNumberToObject(j, "knob", controls_knob());
    cJSON_AddNumberToObject(j, "speed", roundf(servo_get_speed() * 10) / 10);
    cJSON_AddNumberToObject(j, "target", roundf(servo_get_target() * 10) / 10);
    cJSON_AddNumberToObject(j, "pulse_us", servo_pulse_us());
    cJSON_AddNumberToObject(j, "duty", servo_duty_pct());
    cJSON_AddNumberToObject(j, "uptime_s", (double)(esp_timer_get_time() / 1000000));
    return j;
}

bool player_teaching(void) { return s_mode == MODE_TEACH; }

esp_err_t player_teach_start(void)
{
#if HAS_ENCODER
    if (!encoder_present()) return ESP_ERR_NOT_SUPPORTED;
    int32_t *buf = malloc((TRACK_MAX + 1) * sizeof *buf);
    if (!buf) return ESP_ERR_NO_MEM;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    teach_end_locked();
    free(s_rec);
    s_rec = buf;
    s_pl_n = 0;
    s_pl_name[0] = 0;
    s_paused = false;
    s_resting = false;
    servo_release(true);                  // driver asleep: the shaft turns freely by hand
    s_rec_base = encoder_count();
    s_rec[0] = 0;
    s_rec_n = 1;
    s_rec_tick = 0;
    s_rec_full = false;
    s_mode = MODE_TEACH;
    xSemaphoreGive(s_lock);
    ESP_LOGI(TAG, "teach: turn the shaft by hand (up to %d s)", TRACK_MAX / TRACK_HZ);
    return ESP_OK;
#else
    return ESP_ERR_NOT_SUPPORTED;
#endif
}

void player_teach_cancel(void)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
    teach_end_locked();
    xSemaphoreGive(s_lock);
}

cJSON *player_teach_stop(const char *name, bool loop, char *err, int errlen)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
    if (s_mode != MODE_TEACH || !s_rec) { xSemaphoreGive(s_lock); snprintf(err, errlen, "not recording"); return NULL; }
    int32_t *rec = s_rec;
    int n = s_rec_n;
    s_rec = NULL;
    teach_end_locked();
    xSemaphoreGive(s_lock);
    // Trim the still time before the first move and after the last, keeping half a second of each.
    const int32_t still = 20;              // counts, about 1.7 degrees of the output shaft
    int a = 0, b = n - 1;
    while (a < n - 1 && labs(rec[a] - rec[0]) <= still) a++;
    while (b > 0 && labs(rec[b] - rec[n - 1]) <= still) b--;
    if (a >= n - 1 || b <= 0 || a > b) { free(rec); snprintf(err, errlen, "the shaft didn't move"); return NULL; }
    a = a - TRACK_HZ / 2 < 0 ? 0 : a - TRACK_HZ / 2;
    b = b + TRACK_HZ / 2 > n - 1 ? n - 1 : b + TRACK_HZ / 2;
    int32_t base = rec[a];
    for (int i = a; i <= b; i++) rec[i] -= base;
    cJSON *story = track_story_json(rec + a, b - a + 1, TRACK_HZ, name, loop);
    free(rec);
    if (!story) snprintf(err, errlen, "out of memory");
    return story;
}

void player_init(void)
{
    s_lock = xSemaphoreCreateMutex();
    xTaskCreate(player_task, "player", 6144, NULL, 9, NULL);
    if (g_settings.autoplay && g_settings.autoplay_story[0]) {
        if (player_play(g_settings.autoplay_story) == ESP_OK) ESP_LOGI(TAG, "gallery mode: autoplay %s", g_settings.autoplay_story);
        else ESP_LOGW(TAG, "autoplay story \"%s\" not found", g_settings.autoplay_story);
    }
}
