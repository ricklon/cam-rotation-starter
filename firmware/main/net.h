// Wi-Fi: always an access point (CamBox-XXXX); optionally also joins a network (station).
#pragma once
#include "cJSON.h"
#include <stdbool.h>
#include "esp_err.h"

void      net_start(void);
cJSON    *net_status(void);
// Switch to saved network i: it moves to the top of the list (saved) and the box rejoins now.
// ESP_ERR_INVALID_STATE: no network was saved at power-up, so the switch waits for a restart.
esp_err_t net_use(int i);
// Stop trying to join while a scan runs (the radio refuses to scan mid-connect). False: nothing to pause.
bool      net_pause(void);
void      net_resume(void);
