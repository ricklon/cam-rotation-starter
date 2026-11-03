#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include "storage.h"
#include "esp_littlefs.h"
#include "esp_log.h"

static const char *TAG = "storage";

esp_err_t storage_mount(void)
{
    esp_vfs_littlefs_conf_t conf = {
        .base_path = FS_BASE,
        .partition_label = "storage",
        .format_if_mount_failed = true,
        .dont_mount = false,
    };
    esp_err_t err = esp_vfs_littlefs_register(&conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "mount failed: %s", esp_err_to_name(err));
        return err;
    }
    mkdir(STORIES_DIR, 0775);
    mkdir(PLAYLISTS_DIR, 0775);
    size_t total = 0, used = 0;
    esp_littlefs_info(conf.partition_label, &total, &used);
    ESP_LOGI(TAG, "LittleFS %u / %u bytes used", (unsigned)used, (unsigned)total);
    return ESP_OK;
}

bool storage_valid_name(const char *name)
{
    size_t n = name ? strlen(name) : 0;
    if (n == 0 || n > 31) return false;
    for (size_t i = 0; i < n; i++) {
        char c = name[i];
        if (!(isalnum((unsigned char)c) || c == '-' || c == '_')) return false;
    }
    return true;
}

static void file_path(char *out, size_t n, const char *dir, const char *name)
{
    snprintf(out, n, "%s/%s.json", dir, name);
}

static cJSON *list_dir(const char *dir)
{
    cJSON *arr = cJSON_CreateArray();
    DIR *d = opendir(dir);
    if (!d) return arr;
    struct dirent *e;
    while ((e = readdir(d)) != NULL) {
        size_t n = strlen(e->d_name);
        if (n > 5 && strcmp(e->d_name + n - 5, ".json") == 0) {
            char name[40];
            snprintf(name, sizeof name, "%.*s", (int)(n - 5), e->d_name);
            cJSON_AddItemToArray(arr, cJSON_CreateString(name));
        }
    }
    closedir(d);
    return arr;
}

static char *read_file(const char *dir, const char *name)
{
    if (!storage_valid_name(name)) return NULL;
    char path[96];
    file_path(path, sizeof path, dir, name);
    FILE *f = fopen(path, "r");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (len <= 0 || len > 32768) { fclose(f); return NULL; }
    char *buf = malloc(len + 1);
    if (buf) { buf[fread(buf, 1, len, f)] = 0; }
    fclose(f);
    return buf;
}

static esp_err_t write_file(const char *dir, const char *name, const char *json, size_t len)
{
    if (!storage_valid_name(name)) return ESP_ERR_INVALID_ARG;
    char path[96], tmp[100];
    file_path(path, sizeof path, dir, name);
    snprintf(tmp, sizeof tmp, "%s.tmp", path);
    FILE *f = fopen(tmp, "w");
    if (!f) return ESP_FAIL;
    size_t w = fwrite(json, 1, len, f);
    fclose(f);
    if (w != len) { remove(tmp); return ESP_FAIL; }
    remove(path);
    return rename(tmp, path) == 0 ? ESP_OK : ESP_FAIL;
}

static esp_err_t delete_file(const char *dir, const char *name)
{
    if (!storage_valid_name(name)) return ESP_ERR_INVALID_ARG;
    char path[96];
    file_path(path, sizeof path, dir, name);
    return remove(path) == 0 ? ESP_OK : ESP_ERR_NOT_FOUND;
}

cJSON *storage_list_stories(void) { return list_dir(STORIES_DIR); }
char *storage_read_story(const char *name) { return read_file(STORIES_DIR, name); }
esp_err_t storage_write_story(const char *name, const char *json, size_t len) { return write_file(STORIES_DIR, name, json, len); }
esp_err_t storage_delete_story(const char *name) { return delete_file(STORIES_DIR, name); }

cJSON *storage_list_playlists(void) { return list_dir(PLAYLISTS_DIR); }
char *storage_read_playlist(const char *name) { return read_file(PLAYLISTS_DIR, name); }
esp_err_t storage_write_playlist(const char *name, const char *json, size_t len) { return write_file(PLAYLISTS_DIR, name, json, len); }
esp_err_t storage_delete_playlist(const char *name) { return delete_file(PLAYLISTS_DIR, name); }
