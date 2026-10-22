#include <string.h>
#include <stdio.h>
#include "net.h"
#include "settings.h"
#include "dns_server.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_mac.h"
#include "esp_log.h"
#include "mdns.h"

static const char *TAG = "net";
static esp_netif_t *s_ap, *s_sta;
static char s_ap_ssid[33];
static char s_sta_ip[16] = "";
static int  s_sta_retries = 0;

static void on_event(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        s_sta_ip[0] = 0;
        if (s_sta_retries++ < 20) esp_wifi_connect();
        else ESP_LOGW(TAG, "giving up on %s; access point still running", g_settings.sta_ssid);
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *e = data;
        snprintf(s_sta_ip, sizeof s_sta_ip, IPSTR, IP2STR(&e->ip_info.ip));
        s_sta_retries = 0;
        ESP_LOGI(TAG, "joined %s as %s", g_settings.sta_ssid, s_sta_ip);
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_AP_STACONNECTED) {
        ESP_LOGI(TAG, "a device joined %s", s_ap_ssid);
    }
}

void net_start(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    bool use_sta = g_settings.sta_ssid[0] != 0;
    s_ap = esp_netif_create_default_wifi_ap();
    if (use_sta) s_sta = esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, on_event, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, on_event, NULL));

    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP);
    if (g_settings.ap_ssid[0]) strncpy(s_ap_ssid, g_settings.ap_ssid, sizeof s_ap_ssid - 1);
    else snprintf(s_ap_ssid, sizeof s_ap_ssid, "CamBox-%02X%02X", mac[4], mac[5]);

    wifi_config_t ap = { 0 };
    strncpy((char *)ap.ap.ssid, s_ap_ssid, sizeof ap.ap.ssid);
    ap.ap.ssid_len = strlen(s_ap_ssid);
    ap.ap.channel = 6;
    ap.ap.max_connection = 4;
    if (strlen(g_settings.ap_pass) >= 8) {
        strncpy((char *)ap.ap.password, g_settings.ap_pass, sizeof ap.ap.password);
        ap.ap.authmode = WIFI_AUTH_WPA2_PSK;
    } else {
        ap.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(use_sta ? WIFI_MODE_APSTA : WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap));
    if (use_sta) {
        wifi_config_t sta = { 0 };
        strncpy((char *)sta.sta.ssid, g_settings.sta_ssid, sizeof sta.sta.ssid);
        strncpy((char *)sta.sta.password, g_settings.sta_pass, sizeof sta.sta.password);
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta));
    }
    ESP_ERROR_CHECK(esp_wifi_start());

    esp_netif_ip_info_t ip;
    esp_netif_get_ip_info(s_ap, &ip);
    dns_server_start(ip.ip.addr);

    if (mdns_init() == ESP_OK) {
        mdns_hostname_set(g_settings.hostname);
        mdns_instance_name_set("CamBox cam drive");
        mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);
    }
    ESP_LOGI(TAG, "access point \"%s\" (%s) at " IPSTR ", http://%s.local",
             s_ap_ssid, ap.ap.authmode == WIFI_AUTH_OPEN ? "open" : "WPA2", IP2STR(&ip.ip), g_settings.hostname);
}

cJSON *net_status(void)
{
    cJSON *j = cJSON_CreateObject();
    cJSON_AddStringToObject(j, "ap_ssid", s_ap_ssid);
    cJSON_AddStringToObject(j, "sta_ssid", g_settings.sta_ssid);
    cJSON_AddStringToObject(j, "sta_ip", s_sta_ip);
    cJSON_AddStringToObject(j, "hostname", g_settings.hostname);
    return j;
}
