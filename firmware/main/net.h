// Wi-Fi: always an access point (CamBox-XXXX); optionally also joins a network (station).
#pragma once
#include "cJSON.h"

void   net_start(void);
cJSON *net_status(void);
