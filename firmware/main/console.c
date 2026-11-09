// Serial console on the board's USB-C (USB Serial/JTAG): Wi-Fi setup and diagnostics without the web app.
// Type `help` for the list. Settings changes are saved to NVS at once; `wifi use` switches networks live,
// other network changes apply after `restart`.
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "console.h"
#include "board.h"
#include "settings.h"
#include "servo.h"
#include "controls.h"
#include "player.h"
#include "encoder.h"
#include "net.h"
#include "esp_console.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_mac.h"
#include "esp_chip_info.h"
#include "esp_app_desc.h"
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

// ---------- info ----------

static int cmd_info(int argc, char **argv)
{
    cJSON *n = net_status();
    uint8_t sta[6], ap[6];
    esp_read_mac(sta, ESP_MAC_WIFI_STA);
    esp_read_mac(ap, ESP_MAC_WIFI_SOFTAP);
    printf("name      %s (http://%s.local)\n", cJSON_GetObjectItem(n, "hostname")->valuestring,
           cJSON_GetObjectItem(n, "hostname")->valuestring);
    printf("ap        %s\n", cJSON_GetObjectItem(n, "ap_ssid")->valuestring);
    cJSON_Delete(n);
    printf("mac       sta %02x:%02x:%02x:%02x:%02x:%02x, ap %02x:%02x:%02x:%02x:%02x:%02x\n",
           sta[0], sta[1], sta[2], sta[3], sta[4], sta[5], ap[0], ap[1], ap[2], ap[3], ap[4], ap[5]);
    printf("board     %s, drive %s\n", BOARD_NAME, DRIVE_NAME);

    const esp_app_desc_t *d = esp_app_get_description();
    printf("firmware  %s %s (built %s %s)\n", d->project_name, d->version, d->date, d->time);
    printf("idf       %s\n", esp_get_idf_version());

    esp_chip_info_t ci;
    esp_chip_info(&ci);
    printf("chip      esp32c6 rev v%d.%d, %d core(s)\n", ci.revision / 100, ci.revision % 100, ci.cores);
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
    if (!g_settings.net_count) {
        printf("home network: none saved (wifi add <name> <password>)\n");
    } else if (!sta[0]) {
        printf("home network: not connected (%d saved; type `wifi`)\n", g_settings.net_count);
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
    if (encoder_present())
        printf("encoder %ld counts, %.2f turns, %.1f deg, %.1f rpm\n", (long)encoder_count(), encoder_turns(),
               encoder_degrees(), encoder_rpm());
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
    bool paused = net_pause();
    wifi_scan_config_t cfg = { .show_hidden = false };
    esp_err_t err;
    for (int i = 0; (err = esp_wifi_scan_start(&cfg, true)) == ESP_ERR_WIFI_STATE && i < 10; i++)
        vTaskDelay(pdMS_TO_TICKS(100));             // the disconnect takes a moment to land
    uint16_t n = 0;
    wifi_ap_record_t *recs = NULL;
    if (err == ESP_OK) {
        esp_wifi_scan_get_ap_num(&n);
        if (n > 30) n = 30;
        recs = calloc(n ? n : 1, sizeof *recs);
        if (recs) esp_wifi_scan_get_ap_records(&n, recs);
    }
    if (mode == WIFI_MODE_AP) esp_wifi_set_mode(WIFI_MODE_AP);
    if (paused) net_resume();
    if (err != ESP_OK || !recs) { printf("scan failed: %s\n", esp_err_to_name(err)); free(recs); return 1; }
    printf("%d networks (2.4 GHz only; this chip can't see 5 GHz):\n", n);
    for (int i = 0; i < n; i++)
        printf("  %4d dBm  ch %2d  %-6s  %s\n", recs[i].rssi, recs[i].primary,
               recs[i].authmode == WIFI_AUTH_OPEN ? "open" : "secure", (const char *)recs[i].ssid);
    free(recs);
    return 0;
}

static const char *WIFI_USE =
    "use: wifi | wifi scan | wifi add <name> [password] | wifi use <n|name> | wifi join <name> [password]\n"
    "     wifi remove <n|name> | wifi forget   (quote names with spaces)\n";

static void wifi_list(void)
{
    cJSON *n = net_status();
    const char *cur = cJSON_GetObjectItem(n, "sta_ssid")->valuestring;
    if (!g_settings.net_count) printf("no saved networks (wifi add <name> <password>)\n");
    else printf("saved networks, tried in this order (* = joined now):\n");
    for (int i = 0; i < g_settings.net_count; i++)
        printf("  %d %s %s%s\n", i + 1, cur[0] && !strcmp(cur, g_settings.nets[i].ssid) ? "*" : " ",
               g_settings.nets[i].ssid, g_settings.nets[i].pass[0] ? "" : "  (open)");
    cJSON_Delete(n);
}

// A list number (1-based) or a saved name; -1 if neither.
static int wifi_find(const char *arg)
{
    char *end;
    long k = strtol(arg, &end, 10);
    if (*arg && !*end) return (k >= 1 && k <= g_settings.net_count) ? (int)k - 1 : -1;
    for (int i = 0; i < g_settings.net_count; i++) if (!strcmp(g_settings.nets[i].ssid, arg)) return i;
    return -1;
}

// Add a network, or update the password of one already saved. Returns its index, or -1.
static int wifi_add(int argc, char **argv)
{
    if (strlen(argv[2]) > 32 || (argc == 4 && strlen(argv[3]) > 64)) { printf("name or password too long\n"); return -1; }
    int i = 0;
    while (i < g_settings.net_count && strcmp(g_settings.nets[i].ssid, argv[2])) i++;   // same name: update it
    if (i == g_settings.net_count) {
        if (i >= WIFI_MAX_NETS) { printf("already %d networks saved; wifi remove one first\n", WIFI_MAX_NETS); return -1; }
        g_settings.net_count++;
    }
    strcpy(g_settings.nets[i].ssid, argv[2]);
    strcpy(g_settings.nets[i].pass, argc == 4 ? argv[3] : "");
    return save_or_complain() ? -1 : i;
}

static int wifi_use(int i)
{
    esp_err_t err = net_use(i);
    if (err == ESP_ERR_INVALID_STATE) { printf("saved as first choice. Type `restart` to join it.\n"); return 0; }
    if (err != ESP_OK) { printf("could not switch: %s\n", esp_err_to_name(err)); return 1; }
    printf("switching to \"%s\"... type `status` in a few seconds to see the address\n", g_settings.nets[0].ssid);
    return 0;
}

static int cmd_wifi(int argc, char **argv)
{
    if (argc == 1 || (argc == 2 && !strcmp(argv[1], "list"))) { wifi_list(); if (argc == 1) printf("%s", WIFI_USE); return 0; }
    if (!strcmp(argv[1], "scan")) return wifi_scan();
    if ((!strcmp(argv[1], "add") || !strcmp(argv[1], "join")) && (argc == 3 || argc == 4)) {
        int i = wifi_add(argc, argv);
        if (i < 0) return 1;
        if (!strcmp(argv[1], "join")) return wifi_use(i);
        printf("saved \"%s\" as number %d. `wifi use %d` switches to it now.\n", g_settings.nets[i].ssid, i + 1, i + 1);
        return 0;
    }
    if (!strcmp(argv[1], "use") && argc == 3) {
        int i = wifi_find(argv[2]);
        if (i < 0) { printf("no saved network \"%s\" (type `wifi` for the list)\n", argv[2]); return 1; }
        return wifi_use(i);
    }
    if (!strcmp(argv[1], "remove") && argc == 3) {
        int i = wifi_find(argv[2]);
        if (i < 0) { printf("no saved network \"%s\" (type `wifi` for the list)\n", argv[2]); return 1; }
        printf("removed \"%s\"\n", g_settings.nets[i].ssid);
        memmove(&g_settings.nets[i], &g_settings.nets[i + 1], (g_settings.net_count - i - 1) * sizeof g_settings.nets[0]);
        memset(&g_settings.nets[--g_settings.net_count], 0, sizeof g_settings.nets[0]);
        return save_or_complain();
    }
    if (!strcmp(argv[1], "forget")) {
        memset(g_settings.nets, 0, sizeof g_settings.nets);
        g_settings.net_count = 0;
        if (save_or_complain()) return 1;
        printf("all saved networks forgotten. Type `restart` to apply.\n");
        return 0;
    }
    printf("%s", WIFI_USE);
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

// ---------- encoder ----------

// `encoder` prints position and speed once; `encoder watch` prints them 5 times a second for 10 s, so you can
// turn the shaft by hand or run the motor and watch the count; `encoder zero` makes here count 0.
static int cmd_encoder(int argc, char **argv)
{
    if (!encoder_present()) { printf("no encoder in this build (Super Mini N20 only)\n"); return 1; }
    if (argc == 2 && !strcmp(argv[1], "zero")) { encoder_zero(); printf("encoder zeroed\n"); return 0; }
    int n = (argc == 2 && !strcmp(argv[1], "watch")) ? 50 : 1;
    if (argc > 2 || (argc == 2 && n == 1)) { printf("use: encoder | encoder watch | encoder zero\n"); return 1; }
    for (int i = 0; i < n; i++) {
        if (i) vTaskDelay(pdMS_TO_TICKS(200));
        printf("%9ld counts  %8.2f turns  %5.1f deg  %6.1f rpm\n", (long)encoder_count(), encoder_turns(),
               encoder_degrees(), encoder_rpm());
    }
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
        { .command = "info", .help = "Name, MAC, firmware version and chip: for telling boards apart", .func = cmd_info },
        { .command = "wifi", .help = "Saved networks: wifi | scan | add <name> [password] | use <n|name> | join <name> [password] | remove <n|name> | forget", .func = cmd_wifi },
        { .command = "ap", .help = "CamBox access point: ap <name> [password] | ap default", .func = cmd_ap },
        { .command = "get", .help = "Show settings: get | get <name>", .func = cmd_get },
        { .command = "set", .help = "Change and save a setting: set <name> <value>", .func = cmd_set },
        { .command = "motor", .help = "Run the motor by hand: motor <-100..100>", .func = cmd_motor },
        { .command = "stop", .help = "Stop the motor and any story", .func = cmd_stop },
        { .command = "play", .help = "Play a story: play <name>", .func = cmd_play },
        { .command = "encoder", .help = "N20 Hall encoder: encoder | encoder watch | encoder zero", .func = cmd_encoder },
        { .command = "log",.help = "Log level: log none|error|warn|info|debug", .func = cmd_log },
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
