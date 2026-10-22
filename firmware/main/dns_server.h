// Tiny captive-portal DNS: answers every A query with the access point's address.
#pragma once
#include <stdint.h>

void dns_server_start(uint32_t ap_ip_network_order);
