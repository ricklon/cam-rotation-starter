// Hall encoder on the N20: x4 quadrature counting with the PCNT peripheral.
// Two PCNT channels on one unit, each counting the edges of one phase with the other phase as its
// direction level (the IDF rotary-encoder arrangement), so every edge of A and B counts once, up or
// down. The glitch filter drops pulses shorter than 1 us; at full speed (about 15000 motor rpm,
// 1750 Hz per channel) an edge comes every 140 us or so. The hardware counter is 16-bit, so the
// unit accumulates across its limits (accum_count) into a 32-bit count: over 500000 output turns.
#include "encoder.h"
#include "board.h"

#if HAS_ENCODER
#include <math.h>
#include "driver/pulse_cnt.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "encoder";

#define ENC_LIMIT      30000            // PCNT counts to its limit, then folds it into the accumulated count
#define ENC_GLITCH_NS  1000
#define SAMPLE_MS      100

static pcnt_unit_handle_t s_unit;
static volatile int32_t s_zero;         // raw count that reads as 0
static volatile float s_rpm;

static int32_t raw_count(void)
{
    int v = 0;
    if (s_unit) pcnt_unit_get_count(s_unit, &v);
    return ENC_REVERSED ? -v : v;
}

static void speed_task(void *arg)
{
    int32_t last = raw_count();
    int64_t t_last = esp_timer_get_time();
    float rpm = 0;
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(SAMPLE_MS));
        int32_t now = raw_count();
        int64_t t = esp_timer_get_time();
        float dt = (t - t_last) / 1e6f;
        if (dt > 0) {
            float inst = (float)(now - last) / ENC_COUNTS_PER_REV / dt * 60.0f;
            rpm += 0.3f * (inst - rpm);                    // about 0.3 s time constant at 100 ms samples
            if (fabsf(rpm) < 0.05f && now == last) rpm = 0;
            s_rpm = rpm;
        }
        last = now;
        t_last = t;
    }
}

void encoder_init(void)
{
    pcnt_unit_config_t ucfg = {
        .low_limit = -ENC_LIMIT,
        .high_limit = ENC_LIMIT,
        .flags.accum_count = 1,
    };
    esp_err_t err = pcnt_new_unit(&ucfg, &s_unit);
    if (err != ESP_OK) { ESP_LOGE(TAG, "no PCNT unit: %s", esp_err_to_name(err)); s_unit = NULL; return; }
    pcnt_glitch_filter_config_t fcfg = { .max_glitch_ns = ENC_GLITCH_NS };
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(s_unit, &fcfg));

    pcnt_chan_config_t acfg = { .edge_gpio_num = PIN_ENC_A, .level_gpio_num = PIN_ENC_B };
    pcnt_chan_config_t bcfg = { .edge_gpio_num = PIN_ENC_B, .level_gpio_num = PIN_ENC_A };
    pcnt_channel_handle_t a, b;
    ESP_ERROR_CHECK(pcnt_new_channel(s_unit, &acfg, &a));
    ESP_ERROR_CHECK(pcnt_new_channel(s_unit, &bcfg, &b));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(a, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(b, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    // The encoder board pulls C1 and C2 up to its VCC (3V3) already; the internal pull-ups are a fallback,
    // so the inputs don't float if it doesn't, or when the motor is unplugged.
    gpio_pullup_en(PIN_ENC_A);
    gpio_pullup_en(PIN_ENC_B);

    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(s_unit, -ENC_LIMIT));
    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(s_unit, ENC_LIMIT));
    ESP_ERROR_CHECK(pcnt_unit_enable(s_unit));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(s_unit));
    ESP_ERROR_CHECK(pcnt_unit_start(s_unit));
    s_zero = 0;
    xTaskCreate(speed_task, "encoder", 2560, NULL, 3, NULL);
    ESP_LOGI(TAG, "counting A=GPIO%d B=GPIO%d, %d counts per output turn", PIN_ENC_A, PIN_ENC_B, ENC_COUNTS_PER_REV);
}

bool encoder_present(void) { return s_unit != NULL; }
int32_t encoder_count(void) { return raw_count() - s_zero; }
float encoder_turns(void) { return (float)encoder_count() / ENC_COUNTS_PER_REV; }
float encoder_rpm(void) { return s_rpm; }
void encoder_zero(void) { s_zero = raw_count(); }

float encoder_degrees(void)
{
    int32_t c = encoder_count() % ENC_COUNTS_PER_REV;
    if (c < 0) c += ENC_COUNTS_PER_REV;
    return c * 360.0f / ENC_COUNTS_PER_REV;
}

cJSON *encoder_status(void)
{
    if (!s_unit) return NULL;
    int32_t c = encoder_count();
    cJSON *j = cJSON_CreateObject();
    cJSON_AddNumberToObject(j, "count", c);
    cJSON_AddNumberToObject(j, "turns", roundf((float)c / ENC_COUNTS_PER_REV * 100) / 100);
    cJSON_AddNumberToObject(j, "deg", roundf(encoder_degrees() * 10) / 10);
    cJSON_AddNumberToObject(j, "rpm", roundf(s_rpm * 10) / 10);
    cJSON_AddNumberToObject(j, "cpr", ENC_COUNTS_PER_REV);
    return j;
}

#else

void encoder_init(void) {}
bool encoder_present(void) { return false; }
int32_t encoder_count(void) { return 0; }
float encoder_turns(void) { return 0; }
float encoder_degrees(void) { return 0; }
float encoder_rpm(void) { return 0; }
void encoder_zero(void) {}
cJSON *encoder_status(void) { return NULL; }

#endif
