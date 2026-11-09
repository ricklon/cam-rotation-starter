// Hand-recorded motion ("track") for stories: the output shaft's position, sampled TRACK_HZ times a second
// while someone turns it by hand (N20 with encoder). In a story's JSON it is
//   "track": {"hz": 25, "cpr": 4200, "d": "0,3,12,40,..."}
// where "d" holds the change in encoder counts from each sample to the next, as text: one cJSON string
// rather than thousands of number items, which would not fit the heap. The story also carries ordinary
// beats worked out from the track, so it still plays (approximately) without an encoder.
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "cJSON.h"

#define TRACK_HZ   25
#define TRACK_MAX  (120 * TRACK_HZ)        // samples after the first: a 120 s take

typedef struct {
    int      hz;
    int      n;          // number of positions, 0 = no track
    int32_t *pos;        // n cumulative positions in counts, pos[0] = 0 (malloc'd)
} track_t;

// Reads story["track"] if there is one (out->n = 0 if not). false + err if it is malformed.
bool   track_from_json(const cJSON *story, track_t *out, char *err, int errlen);
void   track_free(track_t *t);
int    track_ms(const track_t *t);        // length of the take

// A story for a take: {"name", "loop", "steps": beats from the motion, "track": {...}}. pos[0] must be 0.
cJSON *track_story_json(const int32_t *pos, int n, int hz, const char *name, bool loop);
