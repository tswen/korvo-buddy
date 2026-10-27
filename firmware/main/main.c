/*
 * Korvo 爪爪 — 固件入口
 * 状态机 + NDJSON 协议桩 + UI/输入/BLE 桩
 */

#include "ble_transport_stub.h"
#include "buddy_proto.h"
#include "buddy_state.h"
#include "input_handler.h"
#include "pet_ui.h"

#include <stdio.h>
#include <string.h>

#if __has_include("freertos/FreeRTOS.h")
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
static const char *TAG = "korvo_buddy";
#define BUDDY_DELAY_MS(ms) vTaskDelay(pdMS_TO_TICKS(ms))
static uint32_t buddy_now_ms(void)
{
    return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}
#else
#define ESP_LOGI(tag, fmt, ...) ((void)0)
#define BUDDY_DELAY_MS(ms) ((void)(ms))
static uint32_t s_fake_ms;
static uint32_t buddy_now_ms(void)
{
    return s_fake_ms += 100;
}
#endif

static void on_proto_line(const char *line, void *user)
{
    (void)user;
    buddy_snapshot_t snap;
    buddy_proto_result_t r = buddy_proto_parse_line(line, &snap);
    if (r == BUDDY_PROTO_OK) {
        buddy_state_on_heartbeat(&snap, buddy_now_ms());
    } else if (r == BUDDY_PROTO_ERR) {
        buddy_state_on_parse_error();
    }
}

static void on_input(buddy_input_action_t action, void *user)
{
    (void)user;
    const buddy_snapshot_t *snap = buddy_state_snapshot();

    if (action == BUDDY_INPUT_VOL_UP) {
        pet_ui_adjust_brightness(5);
        return;
    }
    if (action == BUDDY_INPUT_VOL_DOWN) {
        pet_ui_adjust_brightness(-5);
        return;
    }

    if (buddy_state_get() != BUDDY_STATE_WAITING || !snap->prompt.valid) {
        return;
    }

    bool approve = (action == BUDDY_INPUT_APPROVE);
    char buf[160];
    int n = buddy_proto_format_permission(snap->prompt.id, approve, buf, sizeof(buf));
    if (n > 0) {
        ble_transport_send_line(buf);
        ESP_LOGI(TAG, "permission sent: %s", buf);
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Korvo 爪爪 starting (proto stub)");

    buddy_state_init();
    pet_ui_init();
    ble_transport_init(on_proto_line, NULL);
    ble_transport_start_advertising("Cursor-S31");
    input_handler_init(on_input, NULL);

    /* TODO: WS2812 GPIO37 状态色 — led_indicator / BSP */

    while (1) {
        uint32_t now = buddy_now_ms();
        input_handler_poll();
        buddy_state_tick(now);
        pet_ui_render(buddy_state_get(), buddy_state_snapshot());
        BUDDY_DELAY_MS(50);
    }
}
