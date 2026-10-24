#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "web.h"
#include "board.h"
#include "player.h"
#include "servo.h"
#include "settings.h"
#include "storage.h"
#include "net.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "web";
static httpd_handle_t s_server;

// ---------- helpers ----------

static esp_err_t send_json(httpd_req_t *req, cJSON *j)
{
    char *s = cJSON_PrintUnformatted(j);
    cJSON_Delete(j);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    esp_err_t r = httpd_resp_sendstr(req, s ? s : "{}");
    free(s);
    return r;
}

static esp_err_t send_err(httpd_req_t *req, const char *status, const char *msg)
{
    httpd_resp_set_status(req, status);
    cJSON *j = cJSON_CreateObject();
    cJSON_AddStringToObject(j, "error", msg);
    return send_json(req, j);
}

static char *read_body(httpd_req_t *req, size_t max)
{
    if (req->content_len == 0 || req->content_len > max) return NULL;
    char *buf = malloc(req->content_len + 1);
    if (!buf) return NULL;
    size_t got = 0;
    while (got < req->content_len) {
        int r = httpd_req_recv(req, buf + got, req->content_len - got);
        if (r <= 0) { free(buf); return NULL; }
        got += r;
    }
    buf[got] = 0;
    return buf;
}

static cJSON *full_status(void)
{
    cJSON *j = player_status();
    cJSON_AddItemToObject(j, "net", net_status());
    cJSON_AddStringToObject(j, "drive", DRIVE_NAME);
    cJSON_AddNumberToObject(j, "trim_us", g_settings.trim_us);
    cJSON_AddNumberToObject(j, "free_heap", esp_get_free_heap_size());
    return j;
}

// ---------- REST ----------

static esp_err_t h_status(httpd_req_t *req) { return send_json(req, full_status()); }

static esp_err_t h_settings_get(httpd_req_t *req) { return send_json(req, settings_to_json(false)); }

static esp_err_t h_settings_post(httpd_req_t *req)
{
    char *b = read_body(req, 2048);
    if (!b) return send_err(req, "400 Bad Request", "body missing or too large");
    cJSON *j = cJSON_Parse(b);
    free(b);
    if (!j) return send_err(req, "400 Bad Request", "invalid JSON");
    bool net_changed = cJSON_HasObjectItem(j, "ap_ssid") || cJSON_HasObjectItem(j, "ap_pass") ||
                       cJSON_HasObjectItem(j, "sta_ssid") || cJSON_HasObjectItem(j, "sta_pass") ||
                       cJSON_HasObjectItem(j, "hostname");
    settings_apply_json(j);
    cJSON_Delete(j);
    servo_preview_trim(0);
    if (settings_save() != ESP_OK) return send_err(req, "500 Internal Server Error", "could not save settings");
    cJSON *out = settings_to_json(false);
    cJSON_AddBoolToObject(out, "restart_needed", net_changed);
    return send_json(req, out);
}

static esp_err_t h_restart(httpd_req_t *req)
{
    httpd_resp_sendstr(req, "{\"ok\":true}");
    vTaskDelay(pdMS_TO_TICKS(300));
    esp_restart();
    return ESP_OK;
}

static esp_err_t h_stories_list(httpd_req_t *req) { return send_json(req, storage_list_stories()); }

static const char *story_name_from_uri(httpd_req_t *req)
{
    const char *p = req->uri + strlen("/api/stories/");
    static char name[40];
    size_t n = strcspn(p, "?");
    if (n >= sizeof name) return NULL;
    memcpy(name, p, n);
    name[n] = 0;
    return storage_valid_name(name) ? name : NULL;
}

static esp_err_t h_story_get(httpd_req_t *req)
{
    const char *name = story_name_from_uri(req);
    if (!name) return send_err(req, "400 Bad Request", "story names use letters, digits, - and _");
    char *txt = storage_read_story(name);
    if (!txt) return send_err(req, "404 Not Found", "no such story");
    httpd_resp_set_type(req, "application/json");
    esp_err_t r = httpd_resp_sendstr(req, txt);
    free(txt);
    return r;
}

static esp_err_t h_story_put(httpd_req_t *req)
{
    const char *name = story_name_from_uri(req);
    if (!name) return send_err(req, "400 Bad Request", "story names use letters, digits, - and _");
    char nm[40];
    strcpy(nm, name);
    char *b = read_body(req, 16384);
    if (!b) return send_err(req, "400 Bad Request", "body missing or too large");
    cJSON *j = cJSON_Parse(b);
    free(b);
    story_t tmp;
    char err[80] = "invalid JSON";
    if (!j || !story_from_json(j, &tmp, err, sizeof err)) { cJSON_Delete(j); return send_err(req, "400 Bad Request", err); }
    cJSON_ReplaceItemInObject(j, "name", cJSON_CreateString(nm));
    if (!cJSON_HasObjectItem(j, "name")) cJSON_AddStringToObject(j, "name", nm);
    char *s = cJSON_Print(j);
    cJSON_Delete(j);
    esp_err_t e = s ? storage_write_story(nm, s, strlen(s)) : ESP_ERR_NO_MEM;
    free(s);
    if (e != ESP_OK) return send_err(req, "500 Internal Server Error", "could not save story");
    cJSON *ok = cJSON_CreateObject();
    cJSON_AddStringToObject(ok, "saved", nm);
    return send_json(req, ok);
}

static esp_err_t h_story_delete(httpd_req_t *req)
{
    const char *name = story_name_from_uri(req);
    if (!name || storage_delete_story(name) != ESP_OK) return send_err(req, "404 Not Found", "no such story");
    return send_json(req, cJSON_CreateObject());
}

static esp_err_t h_play(httpd_req_t *req)
{
    char *b = read_body(req, 256);
    cJSON *j = b ? cJSON_Parse(b) : NULL;
    free(b);
    const cJSON *n = cJSON_GetObjectItemCaseSensitive(j, "name");
    esp_err_t e = player_play(cJSON_IsString(n) ? n->valuestring : "");
    cJSON_Delete(j);
    if (e != ESP_OK) return send_err(req, "404 Not Found", "story not found or invalid");
    return send_json(req, full_status());
}

static esp_err_t h_pause(httpd_req_t *req) { player_toggle_pause(); return send_json(req, full_status()); }
static esp_err_t h_stop(httpd_req_t *req)  { player_stop();         return send_json(req, full_status()); }
static esp_err_t h_next(httpd_req_t *req)  { player_next();         return send_json(req, full_status()); }

// ---------- WebSocket: live control + status stream ----------
// client -> device: {"t":"speed","v":-100..100} | {"t":"stop"} | {"t":"master","v":0..100} | {"t":"trim","v":us} | {"t":"trim_end"}
// device -> client: status object every 200 ms

static esp_err_t h_ws(httpd_req_t *req)
{
    if (req->method == HTTP_GET) return ESP_OK;   // handshake done
    httpd_ws_frame_t f = { .type = HTTPD_WS_TYPE_TEXT };
    if (httpd_ws_recv_frame(req, &f, 0) != ESP_OK || f.len == 0 || f.len > 256) return ESP_OK;
    uint8_t buf[260];
    f.payload = buf;
    if (httpd_ws_recv_frame(req, &f, f.len) != ESP_OK) return ESP_OK;
    buf[f.len] = 0;
    cJSON *j = cJSON_Parse((char *)buf);
    const cJSON *t = cJSON_GetObjectItemCaseSensitive(j, "t");
    const cJSON *v = cJSON_GetObjectItemCaseSensitive(j, "v");
    if (cJSON_IsString(t)) {
        if (!strcmp(t->valuestring, "speed") && cJSON_IsNumber(v)) player_manual((float)v->valuedouble);
        else if (!strcmp(t->valuestring, "stop")) player_stop();
        else if (!strcmp(t->valuestring, "master") && cJSON_IsNumber(v)) player_set_master(v->valueint);
        else if (!strcmp(t->valuestring, "trim") && cJSON_IsNumber(v)) { player_stop(); servo_preview_trim(v->valueint); }
        else if (!strcmp(t->valuestring, "trim_end")) servo_preview_trim(0);
    }
    cJSON_Delete(j);
    return ESP_OK;
}

static void ws_broadcast_task(void *arg)
{
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(200));
        size_t n = CONFIG_LWIP_MAX_SOCKETS;
        int fds[CONFIG_LWIP_MAX_SOCKETS];
        if (httpd_get_client_list(s_server, &n, fds) != ESP_OK || n == 0) continue;
        char *s = NULL;
        for (size_t i = 0; i < n; i++) {
            if (httpd_ws_get_fd_info(s_server, fds[i]) != HTTPD_WS_CLIENT_WEBSOCKET) continue;
            if (!s) { cJSON *j = full_status(); s = cJSON_PrintUnformatted(j); cJSON_Delete(j); }
            if (!s) break;
            httpd_ws_frame_t f = { .type = HTTPD_WS_TYPE_TEXT, .payload = (uint8_t *)s, .len = strlen(s) };
            httpd_ws_send_frame_async(s_server, fds[i], &f);
        }
        free(s);
    }
}

// ---------- static files ----------

static const char *mime_for(const char *path)
{
    const char *ext = strrchr(path, '.');
    if (!ext) return "application/octet-stream";
    if (!strcmp(ext, ".html")) return "text/html; charset=utf-8";
    if (!strcmp(ext, ".js"))   return "text/javascript";
    if (!strcmp(ext, ".css"))  return "text/css";
    if (!strcmp(ext, ".svg"))  return "image/svg+xml";
    if (!strcmp(ext, ".png"))  return "image/png";
    if (!strcmp(ext, ".json") || !strcmp(ext, ".webmanifest")) return "application/manifest+json";
    if (!strcmp(ext, ".ico"))  return "image/x-icon";
    return "application/octet-stream";
}

static esp_err_t send_file(httpd_req_t *req, const char *rel)
{
    char path[160], gz[164];
    snprintf(path, sizeof path, WWW_DIR "%s", rel);
    snprintf(gz, sizeof gz, "%s.gz", path);
    struct stat st;
    const char *open_path = NULL;
    bool gzipped = false;
    if (stat(gz, &st) == 0) { open_path = gz; gzipped = true; }
    else if (stat(path, &st) == 0 && !S_ISDIR(st.st_mode)) open_path = path;
    if (!open_path) return ESP_ERR_NOT_FOUND;

    FILE *f = fopen(open_path, "r");
    if (!f) return ESP_ERR_NOT_FOUND;
    httpd_resp_set_type(req, mime_for(path));
    if (gzipped) httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    // Vite puts content hashes in /assets/ file names, so those can be cached for a long time.
    httpd_resp_set_hdr(req, "Cache-Control", strncmp(rel, "/assets/", 8) == 0 ? "public, max-age=31536000, immutable" : "no-cache");
    char *chunk = malloc(2048);
    size_t r;
    while (chunk && (r = fread(chunk, 1, 2048, f)) > 0) {
        if (httpd_resp_send_chunk(req, chunk, r) != ESP_OK) break;
    }
    free(chunk);
    fclose(f);
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t h_static(httpd_req_t *req)
{
    char rel[128];
    size_t n = strcspn(req->uri, "?#");
    if (n >= sizeof rel - 12) n = sizeof rel - 12;
    memcpy(rel, req->uri, n);
    rel[n] = 0;
    if (strstr(rel, "..")) return send_err(req, "400 Bad Request", "bad path");
    if (!strcmp(rel, "/")) strcpy(rel, "/index.html");
    if (send_file(req, rel) == ESP_OK) return ESP_OK;
    // Unknown paths (including phones' captive-portal probes) get the app itself.
    if (send_file(req, "/index.html") == ESP_OK) return ESP_OK;
    httpd_resp_set_type(req, "text/plain");
    return httpd_resp_sendstr(req, "Web app not installed. Build web/ and flash the storage partition (see README).");
}

// ---------- start ----------

static void reg(const char *uri, httpd_method_t m, esp_err_t (*h)(httpd_req_t *))
{
    httpd_uri_t u = { .uri = uri, .method = m, .handler = h };
    httpd_register_uri_handler(s_server, &u);
}

void web_start(void)
{
    httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
    cfg.uri_match_fn = httpd_uri_match_wildcard;
    cfg.max_uri_handlers = 24;
    cfg.lru_purge_enable = true;
    cfg.stack_size = 8192;
    cfg.max_open_sockets = 7;
    ESP_ERROR_CHECK(httpd_start(&s_server, &cfg));

    reg("/api/status", HTTP_GET, h_status);
    reg("/api/settings", HTTP_GET, h_settings_get);
    reg("/api/settings", HTTP_POST, h_settings_post);
    reg("/api/restart", HTTP_POST, h_restart);
    reg("/api/stories", HTTP_GET, h_stories_list);
    reg("/api/stories/*", HTTP_GET, h_story_get);
    reg("/api/stories/*", HTTP_PUT, h_story_put);
    reg("/api/stories/*", HTTP_DELETE, h_story_delete);
    reg("/api/play", HTTP_POST, h_play);
    reg("/api/pause", HTTP_POST, h_pause);
    reg("/api/stop", HTTP_POST, h_stop);
    reg("/api/next", HTTP_POST, h_next);
    httpd_uri_t ws = { .uri = "/ws", .method = HTTP_GET, .handler = h_ws, .is_websocket = true };
    httpd_register_uri_handler(s_server, &ws);
    reg("/*", HTTP_GET, h_static);   // last: app files + captive-portal fallback

    xTaskCreate(ws_broadcast_task, "ws_push", 4096, NULL, 4, NULL);
    ESP_LOGI(TAG, "http server up");
}
