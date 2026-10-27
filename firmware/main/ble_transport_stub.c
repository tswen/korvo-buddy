#include "ble_transport_stub.h"

#include <string.h>

static buddy_rx_line_cb_t s_on_line;
static void *s_user;

void ble_transport_init(buddy_rx_line_cb_t on_line, void *user)
{
    s_on_line = on_line;
    s_user = user;
    /*
     * TODO: NimBLE NUS
     *   Service 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
     *   RX    6E400002-B5A3-F393-E0A9-E50E24DCCA9E
     *   TX    6E400003-B5A3-F393-E0A9-E50E24DCCA9E
     * 收到 central 写入后按 \\n 分包回调 s_on_line
     */
}

void ble_transport_start_advertising(const char *name_prefix)
{
    (void)name_prefix;
    /* TODO: 广播名 Cursor-S31-xxxx */
}

int ble_transport_send_line(const char *line)
{
    if (!line) {
        return -1;
    }
    (void)s_on_line;
    (void)s_user;
    /* TODO: NUS TX notify */
    return 0;
}
