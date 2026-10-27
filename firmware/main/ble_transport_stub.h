#pragma once

#include <stddef.h>
#include <stdint.h>

typedef void (*buddy_rx_line_cb_t)(const char *line, void *user);

void ble_transport_init(buddy_rx_line_cb_t on_line, void *user);

/** name_prefix is "Cursor-S31" or "Claude-S31". MAC suffix is applied in format_name. */
void ble_transport_start_advertising(const char *name_prefix);

/** "Cursor-S31-a1b2" from the last two MAC bytes. Returns length or -1. */
int ble_transport_format_name(const char *prefix, const uint8_t mac[6], char *out, size_t out_len);

/** Push a GATT/serial chunk. Splits on \\n and invokes the RX callback per line. */
void ble_transport_feed(const uint8_t *data, size_t len);

int ble_transport_send_line(const char *line);

const char *ble_transport_last_tx(void);
const char *ble_transport_adv_name(void);
