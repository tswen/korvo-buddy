/*
 * Korvo 爪爪 — firmware entry.
 * State machine, NDJSON, LVGL pet UI, ADC keys, WS2812.
 * BLE NUS and the ES8389 chime stay stubs.
 */

#include "audio_chime.h"
#include "ble_transport_stub.h"
#include "board_hooks.h"
#include "buddy_proto.h"
#include "buddy_state.h"
#include "input_handler.h"
#include "pet_ui.h"
#include "status_led.h"

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

static void publish_face(void)
{
    buddy_state_t state = buddy_state_get();
    const buddy_snapshot_t *snap = buddy_state_snapshot();
    if (audio_chime_on_state(state)) {
        ESP_LOGI(TAG, "wait chime edge (ES8389 not opened in this skeleton)");
    }
    status_led_apply(status_led_for_state(state));
    pet_ui_render(state, snap);
}

static void on_proto_line(const char *line, void *user)
{
    (void)user;
    buddy_line_t msg;
    buddy_line_kind_t kind = buddy_proto_parse(line, &msg);
    uint32_t now = buddy_now_ms();
    if (kind == BUDDY_LINE_HEARTBEAT) {
        buddy_state_on_heartbeat(&msg.snap, now);
    } else if (kind == BUDDY_LINE_STATUS) {
        buddy_state_on_status_ack(msg.status_id, msg.status_ok, now);
    } else if (kind == BUDDY_LINE_ERR) {
        buddy_state_on_parse_error();
    }
    publish_face();
}

static void on_input(buddy_input_action_t action, void *user)
{
    (void)user;
    const buddy_snapshot_t *snap = buddy_state_snapshot();

    if (action == BUDDY_INPUT_VOL_UP) {
        pet_ui_adjust_brightness(5);
        publish_face();
        return;
    }
    if (action == BUDDY_INPUT_VOL_DOWN) {
        pet_ui_adjust_brightness(-5);
        publish_face();
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
    ESP_LOGI(TAG, "Korvo 爪爪 starting");
    korvo_board_bind();

    buddy_state_init();
    audio_chime_reset();
    pet_ui_init();
    ble_transport_init(on_proto_line, NULL);
    ble_transport_start_advertising("Cursor-S31");
    input_handler_init(on_input, NULL);
    publish_face();

    while (1) {
        input_handler_poll();
        buddy_state_tick(buddy_now_ms());
        publish_face();
        BUDDY_DELAY_MS(50);
    }
}
