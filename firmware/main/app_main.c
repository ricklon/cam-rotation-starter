// CamBox: drives a cam shaft with a continuous-rotation servo, from stories, a web app, and panel controls.
#include "board.h"
#include "settings.h"
#include "storage.h"
#include "servo.h"
#include "controls.h"
#include "player.h"
#include "net.h"
#include "web.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "cambox";

static void board_init(void)
{
    // XIAO ESP32-C6: enable the RF switch and select the on-board antenna.
    gpio_config_t out = {
        .pin_bit_mask = (1ULL << PIN_RF_SWITCH_EN) | (1ULL << PIN_ANT_SELECT) | (1ULL << PIN_LED),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&out);
    gpio_set_level(PIN_RF_SWITCH_EN, 0);
    gpio_set_level(PIN_ANT_SELECT, 0);
    gpio_set_level(PIN_LED, !LED_ON_LEVEL);
}

// LED: slow pulse = ready, steady = playing a story, fast blink = Wi-Fi reset.
static void led_task(void *arg)
{
    int t = 0;
    for (;;) {
        cJSON *s = player_status();
        const cJSON *mode = cJSON_GetObjectItem(s, "mode");
        const cJSON *paused = cJSON_GetObjectItem(s, "paused");
        bool playing = cJSON_IsString(mode) && mode->valuestring[0] == 's' && !cJSON_IsTrue(paused);
        cJSON_Delete(s);
        bool on = playing ? true : (t % 20) < 2;
        gpio_set_level(PIN_LED, on ? LED_ON_LEVEL : !LED_ON_LEVEL);
        t++;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

static void check_wifi_reset(void)
{
    // Hold the panel button while powering up for 5 s to reset Wi-Fi settings.
    if (!controls_button_down()) return;
    ESP_LOGW(TAG, "button held at boot: keep holding 5 s to reset Wi-Fi");
    for (int i = 0; i < 50; i++) {
        gpio_set_level(PIN_LED, (i & 1) ? LED_ON_LEVEL : !LED_ON_LEVEL);
        vTaskDelay(pdMS_TO_TICKS(100));
        if (!controls_button_down()) return;
    }
    settings_reset_network();
    ESP_LOGW(TAG, "Wi-Fi settings reset");
}

void app_main(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    board_init();
    settings_load();
    storage_mount();
    servo_init();
    player_init();                       // before controls: button events call into the player
    controls_init(player_on_control);
    check_wifi_reset();
    net_start();
    web_start();
    xTaskCreate(led_task, "led", 2560, NULL, 2, NULL);
    ESP_LOGI(TAG, "ready");
}
