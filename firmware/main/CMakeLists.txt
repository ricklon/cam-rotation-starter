idf_component_register(
    SRCS "app_main.c" "settings.c" "servo.c" "controls.c" "player.c" "storage.c" "net.c" "dns_server.c" "web.c"
    INCLUDE_DIRS "."
    REQUIRES esp_driver_ledc esp_driver_gpio esp_adc esp_wifi esp_netif esp_event nvs_flash esp_http_server json lwip esp_timer
)
# Pack ../data (web build + stories) into the "storage" LittleFS partition; flashed with `idf.py flash`.
littlefs_create_partition_image(storage ../data FLASH_IN_PROJECT)
