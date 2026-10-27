#include "ble_transport_stub.h"

#include <stdio.h>
#include <string.h>

#define BLE_LINE_MAX 1024

static buddy_rx_line_cb_t s_on_line;
static void *s_user;
static char s_rx[BLE_LINE_MAX];
static size_t s_rx_len;
static char s_tx[BLE_LINE_MAX];
static char s_adv[32];

void ble_transport_init(buddy_rx_line_cb_t on_line, void *user)
{
    s_on_line = on_line;
    s_user = user;
    s_rx_len = 0;
    s_tx[0] = '\0';
    s_adv[0] = '\0';
    /*
     * NimBLE NUS (see REFERENCE.md):
     *   service 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
     *   RX      6E400002-B5A3-F393-E0A9-E50E24DCCA9E  central write → ble_transport_feed
     *   TX      6E400003-B5A3-F393-E0A9-E50E24DCCA9E  notify ← ble_transport_send_line
     * USB CDC fallback uses the same feed/send functions at 115200 8N1.
     */
}

int ble_transport_format_name(const char *prefix, const uint8_t mac[6], char *out, size_t out_len)
{
    if (!prefix || !mac || !out) {
        return -1;
    }
    int n = snprintf(out, out_len, "%s-%02x%02x", prefix, mac[4], mac[5]);
    if (n < 0 || (size_t)n >= out_len) {
        return -1;
    }
    return n;
}

void ble_transport_start_advertising(const char *name_prefix)
{
    if (!name_prefix) {
        name_prefix = "Cursor-S31";
    }
    snprintf(s_adv, sizeof(s_adv), "%s", name_prefix);
}

const char *ble_transport_adv_name(void)
{
    return s_adv;
}

static void emit_line(void)
{
    if (s_rx_len == 0) {
        return;
    }
    s_rx[s_rx_len] = '\0';
    if (s_rx_len > 0 && s_rx[s_rx_len - 1] == '\r') {
        s_rx[s_rx_len - 1] = '\0';
    }
    if (s_on_line && s_rx[0] != '\0') {
        s_on_line(s_rx, s_user);
    }
    s_rx_len = 0;
}

void ble_transport_feed(const uint8_t *data, size_t len)
{
    if (!data) {
        return;
    }
    for (size_t i = 0; i < len; i++) {
        char c = (char)data[i];
        if (c == '\n') {
            emit_line();
            continue;
        }
        if (s_rx_len + 1 >= sizeof(s_rx)) {
            s_rx_len = 0;
            continue;
        }
        s_rx[s_rx_len++] = c;
    }
}

int ble_transport_send_line(const char *line)
{
    if (!line) {
        return -1;
    }
    int n = snprintf(s_tx, sizeof(s_tx), "%s", line);
    if (n < 0 || (size_t)n >= sizeof(s_tx)) {
        return -1;
    }
    return n;
}

const char *ble_transport_last_tx(void)
{
    return s_tx;
}
