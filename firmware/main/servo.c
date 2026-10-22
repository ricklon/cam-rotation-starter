#include <math.h>
#include "servo.h"
#include "board.h"
#include "settings.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define SERVO_HZ      50
#define PERIOD_US     (1000000 / SERVO_HZ)
#define RES_BITS      LEDC_TIMER_14_BIT
#define RES_MAX       ((1 << 14) - 1)
#define TICK_MS       20

static const char *TAG = "servo";
static volatile float s_target = 0, s_speed = 0;
static volatile int s_pulse = 0;
static volatile int s_preview_trim = 0;   // 0 = use settings

static void write_pulse(int us)
{
    uint32_t duty = us <= 0 ? 0 : (uint32_t)((uint64_t)us * RES_MAX / PERIOD_US);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    s_pulse = us;
}

static void servo_task(void *arg)
{
    TickType_t last = xTaskGetTickCount();
    int idle_ms = 0;
    for (;;) {
        vTaskDelayUntil(&last, pdMS_TO_TICKS(TICK_MS));
        float step = g_settings.slew * (TICK_MS / 1000.0f);
        float diff = s_target - s_speed;
        if (fabsf(diff) <= step) s_speed = s_target;
        else s_speed += diff > 0 ? step : -step;

        int trim = s_preview_trim ? s_preview_trim : g_settings.trim_us;
        bool stopped = fabsf(s_speed) < 0.5f && fabsf(s_target) < 0.5f;
        idle_ms = stopped ? idle_ms + TICK_MS : 0;

        if (stopped && !s_preview_trim && g_settings.idle_release_ms > 0 && idle_ms >= g_settings.idle_release_ms) {
            if (s_pulse != 0) write_pulse(0);   // release: no pulses, no creep
        } else {
            int us = trim + (int)lroundf(s_speed / 100.0f * g_settings.range_us);
            if (us != s_pulse) write_pulse(us);
        }
    }
}

void servo_init(void)
{
    ledc_timer_config_t t = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = RES_BITS,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = SERVO_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&t));
    ledc_channel_config_t c = {
        .gpio_num = PIN_SERVO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&c));
    write_pulse(0);
    xTaskCreate(servo_task, "servo", 3072, NULL, 10, NULL);
    ESP_LOGI(TAG, "ready on GPIO%d", PIN_SERVO);
}

void servo_set_target(float speed)
{
    if (speed > 100) speed = 100;
    if (speed < -100) speed = -100;
    s_target = speed;
}

float servo_get_speed(void)  { return s_speed; }
float servo_get_target(void) { return s_target; }
int   servo_pulse_us(void)   { return s_pulse; }
void  servo_preview_trim(int trim_us) { s_preview_trim = trim_us; }
