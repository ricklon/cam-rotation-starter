#include <string.h>
#include "dns_server.h"
#include "lwip/sockets.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "dns";
static uint32_t s_ip;

static void dns_task(void *arg)
{
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) { ESP_LOGE(TAG, "socket failed"); vTaskDelete(NULL); return; }
    struct sockaddr_in addr = { .sin_family = AF_INET, .sin_port = htons(53), .sin_addr.s_addr = htonl(INADDR_ANY) };
    if (bind(sock, (struct sockaddr *)&addr, sizeof addr) < 0) { ESP_LOGE(TAG, "bind failed"); close(sock); vTaskDelete(NULL); return; }
    uint8_t buf[512];
    for (;;) {
        struct sockaddr_in from;
        socklen_t fl = sizeof from;
        int n = recvfrom(sock, buf, sizeof buf - 16, 0, (struct sockaddr *)&from, &fl);
        if (n < 12) continue;
        uint16_t qd = (buf[4] << 8) | buf[5];
        if (qd < 1 || (buf[2] & 0x80)) continue;          // need a question; ignore responses
        // walk the first question name
        int p = 12;
        while (p < n && buf[p] != 0) p += buf[p] + 1;
        p += 1 + 4;                                         // zero byte + QTYPE + QCLASS
        if (p > n) continue;
        uint16_t qtype = (buf[p - 4] << 8) | buf[p - 3];
        // header: response, authoritative, no error; one question, one answer for A queries
        buf[2] = 0x84; buf[3] = 0x00;
        buf[4] = 0; buf[5] = 1;                             // QDCOUNT 1
        buf[6] = 0; buf[7] = (qtype == 1) ? 1 : 0;          // ANCOUNT
        buf[8] = buf[9] = buf[10] = buf[11] = 0;
        int len = p;
        if (qtype == 1) {
            uint8_t ans[16] = { 0xC0, 0x0C, 0, 1, 0, 1, 0, 0, 0, 60, 0, 4 };
            memcpy(ans + 12, &s_ip, 4);
            memcpy(buf + len, ans, 16);
            len += 16;
        }
        sendto(sock, buf, len, 0, (struct sockaddr *)&from, fl);
    }
}

void dns_server_start(uint32_t ap_ip_network_order)
{
    s_ip = ap_ip_network_order;
    xTaskCreate(dns_task, "dns", 3072, NULL, 5, NULL);
}
