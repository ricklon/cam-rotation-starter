#include <stdlib.h>
#include "controls.h"
#include "board.h"
#include "settings.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define POLL_MS         10
#define DEBOUNCE_MS     30
#define LONG_MS         2000
#define DOUBLE_GAP_MS   350
#define KNOB_DEADBAND   3     // % change needed before the knob takes over again

static const char *TAG = "controls";
static ctl_cb_t s_cb;
static adc_oneshot_unit_handle_t s_adc;
static adc_channel_t s_knob_ch;
static volatile int s_knob = 0, s_dir_raw = 0;
static volatile int s_virt_dir = 1;   // web "virtual switch", used only while no physical switch is wired; not saved

bool controls_button_down(void) { return gpio_get_level(PIN_BTN) == 0; }

int controls_dir(void)
{
    if (!g_settings.switch_installed) return s_virt_dir;
    return s_dir_raw;
}

bool controls_set_virtual_dir(int d)
{
    if (g_settings.switch_installed) return false;   // a wired switch is the safety override
    s_virt_dir = d < 0 ? -1 : (d > 0 ? 1 : 0);
    return true;
}

int controls_knob(void) { return s_knob; }

static int read_dir_pins(void)
{
    bool fwd = gpio_get_level(PIN_DIR_FWD) == 0;
    bool rev = gpio_get_level(PIN_DIR_REV) == 0;
    if (fwd && !rev) return 1;
    if (rev && !fwd) return -1;
    return 0;   // center (both open) or a wiring fault (both closed)
}

static int read_knob_pct(void)
{
    int sum = 0, raw = 0;
    for (int i = 0; i < 8; i++) {
        if (adc_oneshot_read(s_adc, s_knob_ch, &raw) == ESP_OK) sum += raw;
    }
    int pct = (sum / 8) * 100 / 4095;
    // small dead zones at both ends so "off" and "full" are easy to hit
    if (pct < 2) pct = 0;
    if (pct > 98) pct = 100;
    return pct;
}

static void emit(ctl_event_t ev, int v) { if (s_cb) s_cb(ev, v); }

static void controls_task(void *arg)
{
    // button state machine
    bool stable = false, last_raw = false;
    int  bounce_ms = 0, held_ms = 0, since_release_ms = 0, clicks = 0;
    bool long_fired = false;
    // switch debounce
    int dir_cand = read_dir_pins(), dir_ms = 0;
    s_dir_raw = dir_cand;
    // knob tracking
    int knob_reported = read_knob_pct();
    s_knob = knob_reported;
    int knob_tick = 0;

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(POLL_MS));

        // ---- button
        bool raw = controls_button_down();
        if (raw != last_raw) { bounce_ms = 0; last_raw = raw; }
        else if (bounce_ms < DEBOUNCE_MS) bounce_ms += POLL_MS;
        if (bounce_ms >= DEBOUNCE_MS && raw != stable) {
            stable = raw;
            if (stable) { held_ms = 0; long_fired = false; }
            else if (!long_fired) { clicks++; since_release_ms = 0; }
        }
        if (stable) {
            held_ms += POLL_MS;
            if (!long_fired && held_ms >= LONG_MS) { long_fired = true; clicks = 0; emit(CTL_BTN_LONG, 0); }
        } else if (clicks > 0) {
            since_release_ms += POLL_MS;
            if (clicks >= 2) { clicks = 0; emit(CTL_BTN_DOUBLE, 0); }
            else if (since_release_ms >= DOUBLE_GAP_MS) { clicks = 0; emit(CTL_BTN_SHORT, 0); }
        }

        // ---- direction switch
        int d = read_dir_pins();
        if (d != dir_cand) { dir_cand = d; dir_ms = 0; }
        else if (dir_ms < DEBOUNCE_MS) {
            dir_ms += POLL_MS;
            if (dir_ms >= DEBOUNCE_MS && d != s_dir_raw) {
                s_dir_raw = d;
                if (g_settings.switch_installed) emit(CTL_DIR, d);
            }
        }

        // ---- knob (every 50 ms)
        if (++knob_tick >= 5) {
            knob_tick = 0;
            int k = read_knob_pct();
            s_knob = k;
            if (g_settings.knob_enabled && abs(k - knob_reported) >= KNOB_DEADBAND) {
                knob_reported = k;
                emit(CTL_KNOB, k);
            }
        }
    }
}

void controls_init(ctl_cb_t cb)
{
    s_cb = cb;
    gpio_config_t io = {
        .pin_bit_mask = (1ULL << PIN_BTN) | (1ULL << PIN_DIR_FWD) | (1ULL << PIN_DIR_REV),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io));

    adc_unit_t unit;
    ESP_ERROR_CHECK(adc_oneshot_io_to_channel(PIN_KNOB, &unit, &s_knob_ch));
    adc_oneshot_unit_init_cfg_t ucfg = { .unit_id = unit };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&ucfg, &s_adc));
    adc_oneshot_chan_cfg_t ccfg = { .atten = ADC_ATTEN_DB_12, .bitwidth = ADC_BITWIDTH_12 };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(s_adc, s_knob_ch, &ccfg));

    xTaskCreate(controls_task, "controls", 3072, NULL, 8, NULL);
    ESP_LOGI(TAG, "button D1, knob D2 (ADC ch %d), switch D3/D4", (int)s_knob_ch);
}
