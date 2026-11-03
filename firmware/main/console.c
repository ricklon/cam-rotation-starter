// Serial console on the board's USB-C (USB Serial/JTAG): Wi-Fi setup and diagnostics without the web app.
// Type `help` for the list. Settings changes are saved to NVS at once; network changes apply after `restart`.
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "console.h"
#include "board.h"
#include "settings.h"
#include "servo.h"
#include "controls.h"
#include "player.h"
#include "net.h"
#include "esp_console.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static void print_json(cJSON *j)
{
    char *s = cJSON_Print(j);
    if (s) { printf("%s\n", s); free(s); }
    cJSON_Delete(j);
}

static int save_or_complain(void)
{
    if (settings_save() != ESP_OK) { printf("could not save settings\n"); return 1; }
    return 0;
}

// ---------- status ----------

static int cmd_status(int argc, char **argv)
{
    int64_t up = esp_timer_get_time() / 1000000;
    printf("board %s, drive %s, up %lldh %02lldm %02llds, free heap %lu bytes\n", BOARD_NAME, DRIVE_NAME,
           up / 3600, (up / 60) % 60, up % 60, (unsigned long)esp_get_free_heap_size());

    cJSON *n = net_status();
    const char *sta = cJSON_GetObjectItem(n, "sta_ssid")->valuestring;
    const char *ip = cJSON_GetObjectItem(n, "sta_ip")->valuestring;
    printf("access point \"%s\", http://%s.local\n", cJSON_GetObjectItem(n, "ap_ssid")->valuestring,
           cJSON_GetObjectItem(n, "hostname")->valuestring);
    if (!sta[0]) {
        printf("home network: none set (wifi join <name> <password>)\n");
    } else {
        wifi_ap_record_t ap;
        if (ip[0] && esp_wifi_sta_get_ap_info(&ap) == ESP_OK)
            printf("home network \"%s\": joined as %s, signal %d dBm, channel %d\n", sta, ip, ap.rssi, ap.primary);
        else
            printf("home network \"%s\": not connected\n", sta);
    }
    cJSON_Delete(n);

    printf("knob %d %%%s, button %s, direction %d\n", controls_knob(), g_settings.knob_enabled ? "" : " (disabled)",
           controls_button_down() ? "down" : "up", controls_dir());
    printf("motor target %.0f, speed %.0f", servo_get_target(), servo_get_speed());
#if CONFIG_CAM_DRIVE_N20
    printf(", duty %d %%\n", servo_duty_pct());
#else
    printf(", pulse %d us\n", servo_pulse_us());
#endif
    cJSON *p = player_status();
    const cJSON *mode = cJSON_GetObjectItem(p, "mode");
    const cJSON *story = cJSON_GetObjectItem(p, "story");
    printf("player %s%s%s\n", cJSON_IsString(mode) ? mode->valuestring : "?",
           cJSON_IsString(story) && story->valuestring[0] ? ", story " : "",
           cJSON_IsString(story) ? story->valuestring : "");
    cJSON_Delete(p);
    return 0;
}

// ---------- wifi ----------

static int wifi_scan(void)
{
    wifi_mode_t mode;
    esp_wifi_get_mode(&mode);
    if (mode == WIFI_MODE_AP) esp_wifi_set_mode(WIFI_MODE_APSTA);   // scanning needs the station interface
    wifi_scan_config_t cfg = { .show_hidden = false };
    esp_err_t err = esp_wifi_scan_start(&cfg, true);
    uint16_t n = 0;
    wifi_ap_record_t *recs = NULL;
    if (err == ESP_OK) {
        esp_wifi_scan_get_ap_num(&n);
        if (n > 30) n = 30;
        recs = calloc(n ? n : 1, sizeof *recs);
        if (recs) esp_wifi_scan_get_ap_records(&n, recs);
    }
    if (mode == WIFI_MODE_AP) esp_wifi_set_mode(WIFI_MODE_AP);
    if (err != ESP_OK || !recs) { printf("scan failed: %s\n", esp_err_to_name(err)); free(recs); return 1; }
    printf("%d networks (2.4 GHz only; this chip can't see 5 GHz):\n", n);
    for (int i = 0; i < n; i++)
        printf("  %4d dBm  ch %2d  %-6s  %s\n", recs[i].rssi, recs[i].primary,
               recs[i].authmode == WIFI_AUTH_OPEN ? "open" : "secure", (const char *)recs[i].ssid);
    free(recs);
    return 0;
}

static int cmd_wifi(int argc, char **argv)
{
    if (argc == 1) {
        printf("home network: %s%s%s\n", g_settings.sta_ssid[0] ? "\"" : "none",
               g_settings.sta_ssid, g_settings.sta_ssid[0] ? "\"" : "");
        printf("password: %s\n", g_settings.sta_pass[0] ? "set" : "none");
        printf("use: wifi scan | wifi join <name> [password] | wifi forget   (quote names with spaces)\n");
        return 0;
    }
    if (!strcmp(argv[1], "scan")) return wifi_scan();
    if (!strcmp(argv[1], "join") && (argc == 3 || argc == 4)) {
        if (strlen(argv[2]) > 32 || (argc == 4 && strlen(argv[3]) > 64)) { printf("name or password too long\n"); return 1; }
        strcpy(g_settings.sta_ssid, argv[2]);
        strcpy(g_settings.sta_pass, argc == 4 ? argv[3] : "");
        if (save_or_complain()) return 1;
        printf("saved \"%s\". Type `restart` to join it (the CamBox access point stays on).\n", g_settings.sta_ssid);
        return 0;
    }
    if (!strcmp(argv[1], "forget")) {
        g_settings.sta_ssid[0] = 0;
        g_settings.sta_pass[0] = 0;
        if (save_or_complain()) return 1;
        printf("home network forgotten. Type `restart` to apply.\n");
        return 0;
    }
    printf("use: wifi | wifi scan | wifi join <name> [password] | wifi forget\n");
    return 1;
}

static int cmd_ap(int argc, char **argv)
{
    if (argc == 2 && !strcmp(argv[1], "default")) {
        g_settings.ap_ssid[0] = 0;
        g_settings.ap_pass[0] = 0;
    } else if (argc == 2 || argc == 3) {
        if (strlen(argv[1]) > 32 || (argc == 3 && strlen(argv[2]) > 64)) { printf("name or password too long\n"); return 1; }
        if (argc == 3 && strlen(argv[2]) < 8) { printf("the password needs 8 or more characters (or leave it out for an open network)\n"); return 1; }
        strcpy(g_settings.ap_ssid, argv[1]);
        strcpy(g_settings.ap_pass, argc == 3 ? argv[2] : "");
    } else {
        printf("use: ap <name> [password] | ap default\n");
        return 1;
    }
    if (save_or_complain()) return 1;
    printf("saved. Type `restart` to apply.\n");
    return 0;
}

// ---------- settings ----------

static int cmd_get(int argc, char **argv)
{
    cJSON *j = settings_to_json(false);
    if (argc == 2) {
        cJSON *it = cJSON_DetachItemFromObjectCaseSensitive(j, argv[1]);
        cJSON_Delete(j);
        if (!it) { printf("no setting \"%s\" (type `get` for the list)\n", argv[1]); return 1; }
        char *s = cJSON_PrintUnformatted(it);
        printf("%s\n", s ? s : "");
        free(s);
        cJSON_Delete(it);
        return 0;
    }
    print_json(j);
    return 0;
}

static int cmd_set(int argc, char **argv)
{
    if (argc != 3) { printf("use: set <name> <value>   e.g. set slew 200, set knob_enabled false\n"); return 1; }
    if (strstr(argv[1], "pass")) { printf("use `wifi join` or `ap` for passwords\n"); return 1; }
    cJSON *probe = settings_to_json(false);
    bool known = cJSON_HasObjectItem(probe, argv[1]);
    cJSON_Delete(probe);
    if (!known) { printf("no setting \"%s\" (type `get` for the list)\n", argv[1]); return 1; }
    cJSON *val = cJSON_Parse(argv[2]);                 // numbers, true/false
    if (!val || cJSON_IsObject(val) || cJSON_IsArray(val)) { cJSON_Delete(val); val = cJSON_CreateString(argv[2]); }
    cJSON *j = cJSON_CreateObject();
    cJSON_AddItemToObject(j, argv[1], val);
    settings_apply_json(j);                            // clamps every field
    cJSON_Delete(j);
    if (save_or_complain()) return 1;
    char *argv_get[] = { "get", argv[1] };
    printf("%s = ", argv[1]);
    return cmd_get(2, argv_get);
}

// ---------- motor and player ----------

static int cmd_motor(int argc, char **argv)
{
    if (argc != 2) { printf("use: motor <-100..100>   (motor 0 stops)\n"); return 1; }
    float v = strtof(argv[1], NULL);
    if (v < -100) v = -100;
    if (v > 100) v = 100;
    player_manual(v);
    printf("manual speed %.0f\n", v);
    return 0;
}

static int cmd_stop(int argc, char **argv) { player_stop(); printf("stopped\n"); return 0; }

static int cmd_play(int argc, char **argv)
{
    if (argc != 2) { printf("use: play <story>\n"); return 1; }
    esp_err_t err = player_play(argv[1]);
    if (err != ESP_OK) { printf("could not play \"%s\": %s\n", argv[1], esp_err_to_name(err)); return 1; }
    printf("playing %s\n", argv[1]);
    return 0;
}

// ---------- system ----------

static int cmd_log(int argc, char **argv)
{
    static const char *names[] = { "none", "error", "warn", "info", "debug", "verbose" };
    if (argc == 2)
        for (int i = 0; i < 6; i++)
            if (!strcmp(argv[1], names[i])) { esp_log_level_set("*", (esp_log_level_t)i); printf("log level %s\n", names[i]); return 0; }
    printf("use: log none|error|warn|info|debug   (log warn keeps the console quiet while you type)\n");
    return 1;
}

static int cmd_restart(int argc, char **argv)
{
    printf("restarting...\n");
    fflush(stdout);
    vTaskDelay(pdMS_TO_TICKS(200));
    esp_restart();
    return 0;
}

static int cmd_reset_network(int argc, char **argv)
{
    settings_reset_network();
    printf("Wi-Fi settings back to defaults. Type `restart` to apply.\n");
    return 0;
}

void console_start(void)
{
    static const esp_console_cmd_t cmds[] = {
        { .command = "status", .help = "Board, Wi-Fi, knob, button, motor and player at a glance", .func = cmd_status },
        { .command = "wifi", .help = "Home network: wifi | wifi scan | wifi join <name> [password] | wifi forget", .func = cmd_wifi },
        { .command = "ap", .help = "CamBox access point: ap <name> [password] | ap default", .func = cmd_ap },
        { .command = "get", .help = "Show settings: get | get <name>", .func = cmd_get },
        { .command = "set", .help = "Change and save a setting: set <name> <value>", .func = cmd_set },
        { .command = "motor", .help = "Run the motor by hand: motor <-100..100>", .func = cmd_motor },
        { .command = "stop", .help = "Stop the motor and any story", .func = cmd_stop },
        { .command = "play", .help = "Play a story: play <name>", .func = cmd_play },
        { .command = "log", .help = "Log level: log none|error|warn|info|debug", .func = cmd_log },
        { .command = "restart", .help = "Restart the board (applies network changes)", .func = cmd_restart },
        { .command = "reset-network", .help = "Put all Wi-Fi settings back to defaults", .func = cmd_reset_network },
    };
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t cfg = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    cfg.prompt = "cambox>";
    cfg.task_stack_size = 6144;
    esp_console_dev_usb_serial_jtag_config_t dev = ESP_CONSOLE_DEV_USB_SERIAL_JTAG_CONFIG_DEFAULT();
    if (esp_console_new_repl_usb_serial_jtag(&dev, &cfg, &repl) != ESP_OK) return;
    esp_console_register_help_command();
    for (size_t i = 0; i < sizeof cmds / sizeof cmds[0]; i++) esp_console_cmd_register(&cmds[i]);
    esp_console_start_repl(repl);
}
