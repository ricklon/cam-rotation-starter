// N20 gearmotor through a DRV8833 (channel A): same interface as servo.c.
// Slow decay: one input held high, the other PWM'd inverted, so the off phase brakes
// (both high) instead of coasting. That keeps speed close to proportional to duty.
#include <math.h>
#include <stdlib.h>
#include "servo.h"
#include "board.h"
#include "settings.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define PWM_HZ        20000                // above hearing
#define RES_BITS      LEDC_TIMER_10_BIT
#define RES_MAX       ((1 << 10) - 1)
#define CH_IN1        LEDC_CHANNEL_0
#define CH_IN2        LEDC_CHANNEL_1
#define TICK_MS       20

static const char *TAG = "motor";
static volatile float s_target = 0, s_speed = 0;
static volatile int s_duty = 0;            // signed %, as last written
static volatile bool s_awake = false;

static void write_in(ledc_channel_t ch, uint32_t duty)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, ch, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, ch);
}

static void set_awake(bool on)
{
    if (on == s_awake) return;
    if (!on) { write_in(CH_IN1, 0); write_in(CH_IN2, 0); }    // coast, then sleep
    gpio_set_level(PIN_NSLEEP, on);
    if (on) vTaskDelay(pdMS_TO_TICKS(2));                       // DRV8833 wakes in about 1 ms
    s_awake = on;
}

// duty -100..100; 0 brakes (both inputs high)
static void write_duty(int duty)
{
    uint32_t off = (uint32_t)(RES_MAX - abs(duty) * RES_MAX / 100);
    if (duty >= 0) { write_in(CH_IN1, RES_MAX); write_in(CH_IN2, off); }
    else           { write_in(CH_IN2, RES_MAX); write_in(CH_IN1, off); }
    s_duty = duty;
}

static void motor_task(void *arg)
{
    TickType_t last = xTaskGetTickCount();
    int idle_ms = 0;
    for (;;) {
        vTaskDelayUntil(&last, pdMS_TO_TICKS(TICK_MS));
        float step = g_settings.slew * (TICK_MS / 1000.0f);
        float diff = s_target - s_speed;
        if (fabsf(diff) <= step) s_speed = s_target;
        else s_speed += diff > 0 ? step : -step;

        bool stopped = fabsf(s_speed) < 0.5f && fabsf(s_target) < 0.5f;
        idle_ms = stopped ? idle_ms + TICK_MS : 0;

        if (stopped && g_settings.idle_release_ms > 0 && idle_ms >= g_settings.idle_release_ms) {
            if (s_awake) { set_awake(false); s_duty = 0; }      // release: driver asleep, motor free
        } else {
            int duty = (int)lroundf(s_speed);
            if (!s_awake) { set_awake(true); write_duty(duty); }
            else if (duty != s_duty) write_duty(duty);
        }
    }
}

void servo_init(void)
{
    gpio_config_t g = { .pin_bit_mask = 1ULL << PIN_NSLEEP, .mode = GPIO_MODE_OUTPUT };
    ESP_ERROR_CHECK(gpio_config(&g));
    gpio_set_level(PIN_NSLEEP, 0);

    ledc_timer_config_t t = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = RES_BITS,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = PWM_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&t));
    const int pins[2] = { PIN_AIN1, PIN_AIN2 };
    for (int i = 0; i < 2; i++) {
        ledc_channel_config_t c = {
            .gpio_num = pins[i],
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = i == 0 ? CH_IN1 : CH_IN2,
            .timer_sel = LEDC_TIMER_0,
            .duty = 0,
            .hpoint = 0,
        };
        ESP_ERROR_CHECK(ledc_channel_config(&c));
    }
    xTaskCreate(motor_task, "motor", 3072, NULL, 10, NULL);
    ESP_LOGI(TAG, "DRV8833 ready: AIN1 GPIO%d, AIN2 GPIO%d, nSLEEP GPIO%d", PIN_AIN1, PIN_AIN2, PIN_NSLEEP);
}

void servo_set_target(float speed)
{
    if (speed > 100) speed = 100;
    if (speed < -100) speed = -100;
    s_target = speed;
}

float servo_get_speed(void)  { return s_speed; }
float servo_get_target(void) { return s_target; }
int   servo_pulse_us(void)   { return 0; }
int   servo_duty_pct(void)   { return s_duty; }
void  servo_preview_trim(int trim_us) { (void)trim_us; }
