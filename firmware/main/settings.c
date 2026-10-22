#include <string.h>
#include <stdlib.h>
#include "settings.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

static const char *TAG = "settings";
settings_t g_settings;

static void defaults(settings_t *s)
{
    memset(s, 0, sizeof(*s));
    s->trim_us = 1500;
    s->range_us = 400;
    s->slew = 150;
    s->idle_release_ms = 1500;
    s->knob_enabled = true;
    s->switch_installed = true;
    s->autoplay = false;
    s->rest_every = 0;
    s->rest_ms = 5000;
    strcpy(s->hostname, "cambox");
}

static int clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

static void copy_str(char *dst, size_t n, const cJSON *j, const char *key)
{
    const cJSON *it = cJSON_GetObjectItemCaseSensitive(j, key);
    if (cJSON_IsString(it) && it->valuestring) {
        strncpy(dst, it->valuestring, n - 1);
        dst[n - 1] = 0;
    }
}

static void get_int(int *dst, const cJSON *j, const char *key, int lo, int hi)
{
    const cJSON *it = cJSON_GetObjectItemCaseSensitive(j, key);
    if (cJSON_IsNumber(it)) *dst = clampi(it->valueint, lo, hi);
}

static void get_bool(bool *dst, const cJSON *j, const char *key)
{
    const cJSON *it = cJSON_GetObjectItemCaseSensitive(j, key);
    if (cJSON_IsBool(it)) *dst = cJSON_IsTrue(it);
}

void settings_apply_json(const cJSON *j)
{
    if (!cJSON_IsObject(j)) return;
    get_int(&g_settings.trim_us, j, "trim_us", 1000, 2000);
    get_int(&g_settings.range_us, j, "range_us", 50, 900);
    get_int(&g_settings.slew, j, "slew", 10, 2000);
    get_int(&g_settings.idle_release_ms, j, "idle_release_ms", 0, 60000);
    get_bool(&g_settings.knob_enabled, j, "knob_enabled");
    get_bool(&g_settings.switch_installed, j, "switch_installed");
    get_bool(&g_settings.autoplay, j, "autoplay");
    copy_str(g_settings.autoplay_story, sizeof g_settings.autoplay_story, j, "autoplay_story");
    get_int(&g_settings.rest_every, j, "rest_every", 0, 1000);
    get_int(&g_settings.rest_ms, j, "rest_ms", 0, 600000);
    copy_str(g_settings.ap_ssid, sizeof g_settings.ap_ssid, j, "ap_ssid");
    copy_str(g_settings.ap_pass, sizeof g_settings.ap_pass, j, "ap_pass");
    copy_str(g_settings.sta_ssid, sizeof g_settings.sta_ssid, j, "sta_ssid");
    copy_str(g_settings.sta_pass, sizeof g_settings.sta_pass, j, "sta_pass");
    copy_str(g_settings.hostname, sizeof g_settings.hostname, j, "hostname");
    // WPA2 needs 8+ characters; anything shorter falls back to an open network.
    if (strlen(g_settings.ap_pass) > 0 && strlen(g_settings.ap_pass) < 8) g_settings.ap_pass[0] = 0;
}

cJSON *settings_to_json(bool include_secrets)
{
    cJSON *j = cJSON_CreateObject();
    cJSON_AddNumberToObject(j, "trim_us", g_settings.trim_us);
    cJSON_AddNumberToObject(j, "range_us", g_settings.range_us);
    cJSON_AddNumberToObject(j, "slew", g_settings.slew);
    cJSON_AddNumberToObject(j, "idle_release_ms", g_settings.idle_release_ms);
    cJSON_AddBoolToObject(j, "knob_enabled", g_settings.knob_enabled);
    cJSON_AddBoolToObject(j, "switch_installed", g_settings.switch_installed);
    cJSON_AddBoolToObject(j, "autoplay", g_settings.autoplay);
    cJSON_AddStringToObject(j, "autoplay_story", g_settings.autoplay_story);
    cJSON_AddNumberToObject(j, "rest_every", g_settings.rest_every);
    cJSON_AddNumberToObject(j, "rest_ms", g_settings.rest_ms);
    cJSON_AddStringToObject(j, "ap_ssid", g_settings.ap_ssid);
    cJSON_AddStringToObject(j, "sta_ssid", g_settings.sta_ssid);
    cJSON_AddStringToObject(j, "hostname", g_settings.hostname);
    if (include_secrets) {
        cJSON_AddStringToObject(j, "ap_pass", g_settings.ap_pass);
        cJSON_AddStringToObject(j, "sta_pass", g_settings.sta_pass);
    } else {
        cJSON_AddBoolToObject(j, "ap_pass_set", g_settings.ap_pass[0] != 0);
        cJSON_AddBoolToObject(j, "sta_pass_set", g_settings.sta_pass[0] != 0);
    }
    return j;
}

void settings_load(void)
{
    defaults(&g_settings);
    nvs_handle_t h;
    if (nvs_open("cambox", NVS_READONLY, &h) != ESP_OK) {
        ESP_LOGI(TAG, "no saved settings, using defaults");
        return;
    }
    size_t len = 0;
    if (nvs_get_str(h, "cfg", NULL, &len) == ESP_OK && len > 1) {
        char *buf = malloc(len);
        if (buf && nvs_get_str(h, "cfg", buf, &len) == ESP_OK) {
            cJSON *j = cJSON_Parse(buf);
            settings_apply_json(j);
            cJSON_Delete(j);
        }
        free(buf);
    }
    nvs_close(h);
    ESP_LOGI(TAG, "loaded: trim %d us, range %d us", g_settings.trim_us, g_settings.range_us);
}

esp_err_t settings_save(void)
{
    cJSON *j = settings_to_json(true);
    char *s = cJSON_PrintUnformatted(j);
    cJSON_Delete(j);
    if (!s) return ESP_ERR_NO_MEM;
    nvs_handle_t h;
    esp_err_t err = nvs_open("cambox", NVS_READWRITE, &h);
    if (err == ESP_OK) {
        err = nvs_set_str(h, "cfg", s);
        if (err == ESP_OK) err = nvs_commit(h);
        nvs_close(h);
    }
    free(s);
    return err;
}

void settings_reset_network(void)
{
    g_settings.ap_ssid[0] = 0;
    g_settings.ap_pass[0] = 0;
    g_settings.sta_ssid[0] = 0;
    g_settings.sta_pass[0] = 0;
    strcpy(g_settings.hostname, "cambox");
    settings_save();
}
