// Hand-recorded motion for stories: parsing the compact "track" and turning a take into a story.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "track.h"
#include "board.h"
#include "settings.h"
#include "player.h"

#if HAS_ENCODER
#define CPR ENC_COUNTS_PER_REV
#else
#define CPR 4200
#endif

void track_free(track_t *t)
{
    free(t->pos);
    memset(t, 0, sizeof *t);
}

int track_ms(const track_t *t) { return t->n > 1 ? (int)((int64_t)(t->n - 1) * 1000 / t->hz) : 0; }

bool track_from_json(const cJSON *story, track_t *out, char *err, int errlen)
{
    memset(out, 0, sizeof *out);
    const cJSON *tr = cJSON_GetObjectItemCaseSensitive(story, "track");
    if (!tr || cJSON_IsNull(tr)) return true;
    const cJSON *hz = cJSON_GetObjectItemCaseSensitive(tr, "hz");
    const cJSON *d = cJSON_GetObjectItemCaseSensitive(tr, "d");
    if (!cJSON_IsNumber(hz) || hz->valueint < 1 || hz->valueint > 100 || !cJSON_IsString(d)) {
        snprintf(err, errlen, "\"track\" needs \"hz\" (1-100) and a \"d\" string");
        return false;
    }
    int commas = 0;
    for (const char *p = d->valuestring; *p; p++) commas += *p == ',';
    if (commas + 1 > TRACK_MAX + 1) { snprintf(err, errlen, "track too long (max %d samples)", TRACK_MAX + 1); return false; }
    int32_t *pos = malloc((commas + 2) * sizeof *pos);
    if (!pos) { snprintf(err, errlen, "out of memory for the track"); return false; }
    // "d" holds n-1 changes, so the n positions start at 0; a leading "0," from older takes is fine too
    int n = 0;
    pos[n++] = 0;
    const char *p = d->valuestring;
    while (*p) {
        char *end;
        long v = strtol(p, &end, 10);
        if (end == p || labs(v) > 100000) { free(pos); snprintf(err, errlen, "track: bad number at sample %d", n); return false; }
        pos[n] = pos[n - 1] + (int32_t)v;
        n++;
        p = end;
        if (*p == ',') p++;
        else if (*p) { free(pos); snprintf(err, errlen, "track: unexpected '%c'", *p); return false; }
    }
    if (n < 2) { free(pos); snprintf(err, errlen, "track is empty"); return false; }
    out->hz = hz->valueint;
    out->n = n;
    out->pos = pos;
    return true;
}

// Douglas-Peucker on (time, position), with the vertical error: keep[] marks the samples that stay.
static void simplify(const int32_t *pos, int n, float tol, uint8_t *keep)
{
    memset(keep, 0, n);
    keep[0] = keep[n - 1] = 1;
    int stack[64][2], sp = 0;
    stack[sp][0] = 0; stack[sp][1] = n - 1; sp++;
    while (sp) {
        sp--;
        int a = stack[sp][0], b = stack[sp][1];
        if (b - a < 2) continue;
        float worst = 0; int wi = -1;
        for (int i = a + 1; i < b; i++) {
            float lin = pos[a] + (float)(pos[b] - pos[a]) * (i - a) / (b - a);
            float e = fabsf(pos[i] - lin);
            if (e > worst) { worst = e; wi = i; }
        }
        if (wi < 0 || worst <= tol) continue;
        keep[wi] = 1;
        if (sp + 2 > 64) continue;          // deep enough: the tolerance loop below catches the rest
        stack[sp][0] = a; stack[sp][1] = wi; sp++;
        stack[sp][0] = wi; stack[sp][1] = b; sp++;
    }
}

// How far the straight runs between the kept samples stray from the recording, in counts.
static float max_error(const int32_t *pos, int n, const uint8_t *keep)
{
    float worst = 0;
    int a = 0;
    for (int b = 1; b < n; b++) {
        if (!keep[b]) continue;
        for (int i = a + 1; i < b; i++) {
            float e = fabsf(pos[i] - (pos[a] + (float)(pos[b] - pos[a]) * (i - a) / (b - a)));
            if (e > worst) worst = e;
        }
        a = b;
    }
    return worst;
}

cJSON *track_story_json(const int32_t *pos, int n, int hz, const char *name, bool loop)
{
    if (n < 2) return NULL;
    uint8_t *keep = malloc(n);
    if (!keep) return NULL;
    // Beats: the take simplified to at most STORY_MAX_STEPS straight runs, each at its average speed.
    float tol = CPR / 48.0f;                 // 7.5 degrees to start with
    int kept;
    for (;;) {
        simplify(pos, n, tol, keep);
        kept = 0;
        for (int i = 0; i < n; i++) kept += keep[i];
        if (kept - 1 <= STORY_MAX_STEPS) break;
        tol *= 1.5f;
    }
    if (n - 1 > STORY_MAX_STEPS) {
        // A motion too wiggly for 64 straight runs gets flattened by the loop above. 64 equal slices, each at
        // its average speed, may follow it better: keep whichever is closer to the recording.
        uint8_t *even = calloc(n, 1);
        if (even) {
            for (int k = 0; k <= STORY_MAX_STEPS; k++) even[(int)((int64_t)k * (n - 1) / STORY_MAX_STEPS)] = 1;
            if (max_error(pos, n, even) < max_error(pos, n, keep)) memcpy(keep, even, n);
            free(even);
        }
    }
    cJSON *j = cJSON_CreateObject();
    if (name && name[0]) cJSON_AddStringToObject(j, "name", name);
    cJSON_AddBoolToObject(j, "loop", loop);
    cJSON *steps = cJSON_AddArrayToObject(j, "steps");
    const float cps_full = g_settings.max_rpm * (float)CPR / 60.0f;   // counts per second at 100 %
    int a = 0, beat = 0;
    for (int b = 1; b < n; b++) {
        if (!keep[b]) continue;
        int ms = (int)((int64_t)(b - a) * 1000 / hz);
        float cps = (float)(pos[b] - pos[a]) * 1000.0f / (ms > 0 ? ms : 1);
        float sp = cps / cps_full * 100.0f;
        if (sp > 100) sp = 100;
        if (sp < -100) sp = -100;
        int ramp = ms / 4 > 300 ? 300 : ms / 4;
        cJSON *s = cJSON_CreateObject();
        char lb[24];
        snprintf(lb, sizeof lb, "Beat %d", ++beat);
        cJSON_AddStringToObject(s, "label", lb);
        cJSON_AddNumberToObject(s, "speed", lroundf(sp));
        cJSON_AddNumberToObject(s, "ms", ms - ramp);
        cJSON_AddNumberToObject(s, "ramp", ramp);
        cJSON_AddItemToArray(steps, s);
        a = b;
    }
    free(keep);
    // The track itself: the change from each sample to the next, as text.
    size_t cap = (size_t)n * 8 + 16, len = 0;
    char *d = malloc(cap);
    if (!d) { cJSON_Delete(j); return NULL; }
    d[0] = 0;
    for (int i = 1; i < n; i++)
        len += snprintf(d + len, cap - len, i > 1 ? ",%ld" : "%ld", (long)(pos[i] - pos[i - 1]));
    cJSON *tr = cJSON_AddObjectToObject(j, "track");
    cJSON_AddNumberToObject(tr, "hz", hz);
    cJSON_AddNumberToObject(tr, "cpr", CPR);
    cJSON_AddStringToObject(tr, "d", d);
    free(d);
    return j;
}
