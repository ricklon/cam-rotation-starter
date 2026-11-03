// LittleFS on the "storage" partition: web app under /data/www, stories under /data/stories.
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include "esp_err.h"
#include "cJSON.h"

#define FS_BASE      "/data"
#define WWW_DIR      FS_BASE "/www"
#define STORIES_DIR  FS_BASE "/stories"
#define PLAYLISTS_DIR FS_BASE "/playlists"

esp_err_t storage_mount(void);
bool      storage_valid_name(const char *name);         // [A-Za-z0-9_-]{1,31}
cJSON    *storage_list_stories(void);                   // JSON array of names
char     *storage_read_story(const char *name);         // malloc'd JSON text or NULL
esp_err_t storage_write_story(const char *name, const char *json, size_t len);
esp_err_t storage_delete_story(const char *name);

// Playlists: ordered lists of story names, {"name":"x","stories":["a","b"]}, stored under /data/playlists.
cJSON    *storage_list_playlists(void);
char     *storage_read_playlist(const char *name);
esp_err_t storage_write_playlist(const char *name, const char *json, size_t len);
esp_err_t storage_delete_playlist(const char *name);
