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

// Replace the saved network list. An entry without "pass" keeps the password already saved for that SSID.
static void apply_networks(const cJSON *arr)
{
    wifi_net_t old[WIFI_MAX_NETS];
    int old_n = g_settings.net_count;
    memcpy(old, g_settings.nets, sizeof old);
    memset(g_settings.nets, 0, sizeof g_settings.nets);
    int n = 0;
    const cJSON *it;
    cJSON_ArrayForEach(it, arr) {
        if (n >= WIFI_MAX_NETS) break;
        const cJSON *ss = cJSON_GetObjectItemCaseSensitive(it, "ssid");
        const cJSON *pp = cJSON_GetObjectItemCaseSensitive(it, "pass");
        if (!cJSON_IsString(ss) || !ss->valuestring[0]) continue;
        wifi_net_t *d = &g_settings.nets[n];
        strncpy(d->ssid, ss->valuestring, sizeof d->ssid - 1);
        if (cJSON_IsString(pp)) strncpy(d->pass, pp->valuestring, sizeof d->pass - 1);
        else for (int i = 0; i < old_n; i++) if (!strcmp(old[i].ssid, d->ssid)) { strcpy(d->pass, old[i].pass); break; }
        n++;
    }
    g_settings.net_count = n;
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
    const cJSON *nets = cJSON_GetObjectItemCaseSensitive(j, "networks");
    if (cJSON_IsArray(nets)) {
        apply_networks(nets);
    } else {
        // Older saves and clients had one network: sta_ssid / sta_pass become the first entry.
        const cJSON *ss = cJSON_GetObjectItemCaseSensitive(j, "sta_ssid");
        if (cJSON_IsString(ss) && ss->valuestring[0]) {
            if (g_settings.net_count == 0) g_settings.net_count = 1;
            copy_str(g_settings.nets[0].ssid, sizeof g_settings.nets[0].ssid, j, "sta_ssid");
            copy_str(g_settings.nets[0].pass, sizeof g_settings.nets[0].pass, j, "sta_pass");
        }
    }
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
    cJSON_AddStringToObject(j, "hostname", g_settings.hostname);
    cJSON *arr = cJSON_AddArrayToObject(j, "networks");
    for (int i = 0; i < g_settings.net_count; i++) {
        cJSON *o = cJSON_CreateObject();
        cJSON_AddStringToObject(o, "ssid", g_settings.nets[i].ssid);
        if (include_secrets) cJSON_AddStringToObject(o, "pass", g_settings.nets[i].pass);
        else cJSON_AddBoolToObject(o, "pass_set", g_settings.nets[i].pass[0] != 0);
        cJSON_AddItemToArray(arr, o);
    }
    if (include_secrets) cJSON_AddStringToObject(j, "ap_pass", g_settings.ap_pass);
    else cJSON_AddBoolToObject(j, "ap_pass_set", g_settings.ap_pass[0] != 0);
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
    memset(g_settings.nets, 0, sizeof g_settings.nets);
    g_settings.net_count = 0;
    strcpy(g_settings.hostname, "cambox");
    settings_save();
}
