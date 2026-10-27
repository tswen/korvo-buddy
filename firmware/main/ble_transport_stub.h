#pragma once

typedef void (*buddy_rx_line_cb_t)(const char *line, void *user);

void ble_transport_init(buddy_rx_line_cb_t on_line, void *user);
void ble_transport_start_advertising(const char *name_prefix);
int ble_transport_send_line(const char *line);
