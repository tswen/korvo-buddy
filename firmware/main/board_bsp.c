/*
 * ESP32-S31-Korvo-1 + SUB3 bring-up via espressif/esp32_s31_korvo_1.
 * Linked by the IDF app only. Host tests do not compile this file.
 */

#include "board_hooks.h"
#include "input_handler.h"

#include "bsp/esp32_s31_korvo_1.h"
#include "esp_log.h"
#include "led_convert.h"
#include "led_indicator.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static const char *TAG = "korvo_bsp";

static const lv_font_t *font_text(void)
{
#if defined(LV_FONT_SIMSUN_16_CJK) && LV_FONT_SIMSUN_16_CJK
    return &lv_font_simsun_16_cjk;
#elif defined(LV_FONT_MONTSERRAT_20) && LV_FONT_MONTSERRAT_20
    return &lv_font_montserrat_20;
#else
    return LV_FONT_DEFAULT;
#endif
}

static const lv_font_t *font_face(void)
{
#if defined(LV_FONT_MONTSERRAT_28) && LV_FONT_MONTSERRAT_28
    return &lv_font_montserrat_28;
#else
    return font_text();
#endif
}

typedef struct {
    bool ready;
    lv_obj_t *pet;
    lv_obj_t *eye_l;
    lv_obj_t *eye_r;
    lv_obj_t *arm;
    lv_obj_t *face_anim;
    lv_obj_t *face_state;
    lv_obj_t *header;
    lv_obj_t *tokens;
    lv_obj_t *msg;
    lv_obj_t *logs;
    lv_obj_t *footer;
    lv_obj_t *overlay;
    lv_obj_t *overlay_title;
    lv_obj_t *overlay_body;
} korvo_ui_t;

static korvo_ui_t s_ui;

static lv_obj_t *panel(lv_obj_t *parent, int x, int y, int w, int h, uint32_t bg)
{
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, h);
    lv_obj_set_style_bg_color(obj, lv_color_hex(bg), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_radius(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, 10, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    return obj;
}

static lv_obj_t *text_label(lv_obj_t *parent, const lv_font_t *font, uint32_t color)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
    lv_label_set_text(label, "");
    return label;
}

static const char *state_cn(buddy_state_t state)
{
    switch (state) {
    case BUDDY_STATE_IDLE:
        return "IDLE  待命";
    case BUDDY_STATE_RUNNING:
        return "RUNNING  执行中";
    case BUDDY_STATE_WAITING:
        return "WAITING  等待批准";
    case BUDDY_STATE_ERROR:
        return "ERROR  链路错误";
    case BUDDY_STATE_SLEEP:
    default:
        return "SLEEP  休眠";
    }
}

static void place_face(buddy_state_t state);

static uint32_t face_bg(buddy_state_t state)
{
    switch (state) {
    case BUDDY_STATE_IDLE:
        return 0x145E66;
    case BUDDY_STATE_RUNNING:
        return 0x146B32;
    case BUDDY_STATE_WAITING:
        return 0x8A5608;
    case BUDDY_STATE_ERROR:
        return 0x7A1E1E;
    case BUDDY_STATE_SLEEP:
    default:
        return 0x1A3358;
    }
}

static const char *face_glyph(buddy_state_t state)
{
    switch (state) {
    case BUDDY_STATE_IDLE:
        return "o     o";
    case BUDDY_STATE_RUNNING:
        return ">  >  >";
    case BUDDY_STATE_WAITING:
        return "o/";
    case BUDDY_STATE_ERROR:
        return "x     x";
    case BUDDY_STATE_SLEEP:
    default:
        return "z  z  z";
    }
}

void pet_ui_board_init(void)
{
    bsp_display_start();

    bsp_display_lock(0);
    lv_obj_t *scr = lv_disp_get_scr_act(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x12141A), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *header = panel(scr, 0, 0, 800, 40, 0x1C2030);
    s_ui.header = text_label(header, font_text(), 0xF2F4F8);
    lv_obj_align(s_ui.header, LV_ALIGN_LEFT_MID, 0, 0);

    s_ui.pet = panel(scr, 0, 40, 320, 400, 0x1A3358);
    s_ui.face_anim = text_label(s_ui.pet, font_face(), 0xF7F1D8);
    lv_obj_align(s_ui.face_anim, LV_ALIGN_CENTER, 0, -30);
    s_ui.face_state = text_label(s_ui.pet, font_text(), 0xF7F1D8);
    lv_obj_align(s_ui.face_state, LV_ALIGN_BOTTOM_MID, 0, 0);

    s_ui.eye_l = lv_obj_create(s_ui.pet);
    s_ui.eye_r = lv_obj_create(s_ui.pet);
    s_ui.arm = lv_obj_create(s_ui.pet);
    lv_obj_set_style_bg_color(s_ui.eye_l, lv_color_hex(0xF7F1D8), 0);
    lv_obj_set_style_bg_color(s_ui.eye_r, lv_color_hex(0xF7F1D8), 0);
    lv_obj_set_style_bg_color(s_ui.arm, lv_color_hex(0xE7C48A), 0);
    lv_obj_set_style_border_width(s_ui.eye_l, 0, 0);
    lv_obj_set_style_border_width(s_ui.eye_r, 0, 0);
    lv_obj_set_style_border_width(s_ui.arm, 0, 0);
    lv_obj_set_style_bg_opa(s_ui.eye_l, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_opa(s_ui.eye_r, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_opa(s_ui.arm, LV_OPA_COVER, 0);
    lv_obj_clear_flag(s_ui.eye_l, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(s_ui.eye_r, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(s_ui.arm, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(s_ui.arm, LV_OBJ_FLAG_HIDDEN);
    place_face(BUDDY_STATE_SLEEP);

    lv_obj_t *status = panel(scr, 320, 40, 480, 400, 0x171A22);
    s_ui.tokens = text_label(status, font_text(), 0xD7E6C8);
    lv_obj_set_width(s_ui.tokens, 440);
    lv_obj_align(s_ui.tokens, LV_ALIGN_TOP_LEFT, 0, 36);
    s_ui.msg = text_label(status, font_text(), 0xF2F4F8);
    lv_obj_set_width(s_ui.msg, 440);
    lv_obj_align(s_ui.msg, LV_ALIGN_TOP_LEFT, 0, 78);
    s_ui.logs = text_label(status, font_text(), 0xC5CAD6);
    lv_obj_set_width(s_ui.logs, 440);
    lv_obj_align(s_ui.logs, LV_ALIGN_TOP_LEFT, 0, 130);

    lv_obj_t *footer = panel(scr, 0, 440, 800, 40, 0x1C2030);
    s_ui.footer = text_label(footer, font_text(), 0xD0D4DE);
    lv_label_set_text(s_ui.footer, "PLAY 批准    SET 拒绝    VOL 亮度");
    lv_obj_align(s_ui.footer, LV_ALIGN_LEFT_MID, 0, 0);

    s_ui.overlay = panel(scr, 0, 0, 800, 480, 0x1A1206);
    lv_obj_set_style_bg_opa(s_ui.overlay, LV_OPA_90, 0);
    s_ui.overlay_title = text_label(s_ui.overlay, font_text(), 0xFFD27A);
    lv_obj_align(s_ui.overlay_title, LV_ALIGN_TOP_MID, 0, 80);
    s_ui.overlay_body = text_label(s_ui.overlay, font_text(), 0xFFF6E8);
    lv_obj_set_width(s_ui.overlay_body, 680);
    lv_obj_align(s_ui.overlay_body, LV_ALIGN_CENTER, 0, 10);
    lv_obj_add_flag(s_ui.overlay, LV_OBJ_FLAG_HIDDEN);

    bsp_display_unlock();
    bsp_display_backlight_on();
    s_ui.ready = true;
    ESP_LOGI(TAG, "LVGL pet UI ready (800x480)");
}

static void place_face(buddy_state_t state)
{
    bool closed = (state == BUDDY_STATE_SLEEP);
    bool waiting = (state == BUDDY_STATE_WAITING);
    lv_obj_set_size(s_ui.eye_l, closed ? 36 : 28, closed ? 6 : 28);
    lv_obj_set_size(s_ui.eye_r, closed ? 36 : 28, closed ? 6 : 28);
    lv_obj_set_pos(s_ui.eye_l, 70, closed ? 150 : 130);
    lv_obj_set_pos(s_ui.eye_r, 190, closed ? 150 : 130);
    lv_obj_set_style_radius(s_ui.eye_l, closed ? 2 : 14, 0);
    lv_obj_set_style_radius(s_ui.eye_r, closed ? 2 : 14, 0);
    if (waiting) {
        lv_obj_set_size(s_ui.arm, 18, 90);
        lv_obj_set_pos(s_ui.arm, 250, 70);
        lv_obj_clear_flag(s_ui.arm, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(s_ui.arm, LV_OBJ_FLAG_HIDDEN);
    }
}

void pet_ui_board_render(buddy_state_t state, const buddy_snapshot_t *snap)
{
    if (!s_ui.ready || !snap) {
        return;
    }
    if (!bsp_display_lock(30)) {
        return;
    }

    char line[192];
    snprintf(line, sizeof(line), "Korvo 爪爪    %s", state_cn(state));
    lv_label_set_text(s_ui.header, line);

    lv_obj_set_style_bg_color(s_ui.pet, lv_color_hex(face_bg(state)), 0);
    lv_label_set_text(s_ui.face_anim, face_glyph(state));
    lv_label_set_text(s_ui.face_state, state_cn(state));
    place_face(state);

    if (snap->tokens_estimated) {
        snprintf(line, sizeof(line), "今日 %lld 约     累计 %lld", (long long)snap->tokens_today,
                 (long long)snap->tokens);
    } else {
        snprintf(line, sizeof(line), "今日 %lld     累计 %lld", (long long)snap->tokens_today,
                 (long long)snap->tokens);
    }
    lv_label_set_text(s_ui.tokens, line);
    lv_label_set_text(s_ui.msg, snap->msg[0] ? snap->msg : " ");

    char logs[640];
    size_t used = 0;
    logs[0] = '\0';
    int head = snprintf(logs, sizeof(logs), "最近日志\n");
    if (head > 0) {
        used = (size_t)head;
    }
    size_t shown = snap->entry_count > 6 ? 6 : snap->entry_count;
    size_t start = snap->entry_count - shown;
    for (size_t i = 0; i < shown && used + 1 < sizeof(logs); i++) {
        int n = snprintf(logs + used, sizeof(logs) - used, "%s  %s\n", snap->entries[start + i].t,
                         snap->entries[start + i].line);
        if (n < 0 || (size_t)n >= sizeof(logs) - used) {
            break;
        }
        used += (size_t)n;
    }
    if (shown == 0) {
        snprintf(logs + used, sizeof(logs) - used, "(空)");
    }
    lv_label_set_text(s_ui.logs, logs);

    if (state == BUDDY_STATE_WAITING && snap->prompt.valid) {
        lv_label_set_text(s_ui.overlay_title, "需要你的批准");
        snprintf(line, sizeof(line), "%s\n%s\n\nPLAY 批准          SET 拒绝", snap->prompt.tool, snap->prompt.hint);
        lv_label_set_text(s_ui.overlay_body, line);
        lv_obj_clear_flag(s_ui.overlay, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(s_ui.overlay);
    } else {
        lv_obj_add_flag(s_ui.overlay, LV_OBJ_FLAG_HIDDEN);
    }

    bsp_display_unlock();
}

void pet_ui_board_set_brightness(int percent)
{
    static bool noted;
    esp_err_t err = bsp_display_brightness_set(percent);
    if (err == ESP_ERR_NOT_SUPPORTED && !noted) {
        ESP_LOGW(TAG, "BSP has no LCD backlight on this board; VOL updates the on-screen percent only");
        noted = true;
    }
}

static void on_button(void *button_handle, void *usr_data)
{
    (void)button_handle;
    input_handler_inject((buddy_input_action_t)(intptr_t)usr_data);
}

static void bind_key(button_handle_t btn, buddy_input_action_t action, const char *name)
{
    esp_err_t err = iot_button_register_cb(btn, BUTTON_PRESS_DOWN, NULL, on_button, (void *)(intptr_t)action);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "button cb %s failed: %s", name, esp_err_to_name(err));
    }
}

void input_board_init(void)
{
    button_handle_t btns[BSP_BUTTON_NUM] = {0};
    esp_err_t err = bsp_iot_button_create(btns, NULL, BSP_BUTTON_NUM);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "bsp_iot_button_create failed: %s", esp_err_to_name(err));
        return;
    }
    /* Silkscreen PLAY is BSP_BUTTON_MODE. SET / VOL+ / VOL- match the enum names. */
    bind_key(btns[BSP_BUTTON_MODE], BUDDY_INPUT_APPROVE, "PLAY/MODE");
    bind_key(btns[BSP_BUTTON_SET], BUDDY_INPUT_DENY, "SET");
    bind_key(btns[BSP_BUTTON_VOLP], BUDDY_INPUT_VOL_UP, "VOL+");
    bind_key(btns[BSP_BUTTON_VOLM], BUDDY_INPUT_VOL_DOWN, "VOL-");
    ESP_LOGI(TAG, "ADC buttons: MODE=approve SET=deny VOL=brightness");
}

static led_indicator_handle_t s_led;

static int effect_id(const char *effect)
{
    if (effect && strcmp(effect, "blink_fast") == 0) {
        return BSP_LED_BLINK_FAST;
    }
    if (effect && strcmp(effect, "pulse") == 0) {
        return BSP_LED_BREATHE_FAST;
    }
    if (effect && strcmp(effect, "breathe_slow") == 0) {
        return BSP_LED_BREATHE_SLOW;
    }
    return BSP_LED_ON;
}

void status_led_board_apply(status_led_color_t color)
{
    if (!s_led) {
        led_indicator_handle_t leds[BSP_LED_NUM] = {0};
        esp_err_t err = bsp_led_indicator_create(leds, NULL, BSP_LED_NUM);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "bsp_led_indicator_create failed: %s", esp_err_to_name(err));
            return;
        }
        s_led = leds[BSP_LED_1];
        ESP_LOGI(TAG, "WS2812 on BSP_LED_RGB_IO GPIO%d", (int)BSP_LED_RGB_IO);
    }

    static const int k_all[] = {
        BSP_LED_ON, BSP_LED_OFF, BSP_LED_BLINK_FAST, BSP_LED_BLINK_SLOW, BSP_LED_BREATHE_FAST, BSP_LED_BREATHE_SLOW,
    };
    for (size_t i = 0; i < sizeof(k_all) / sizeof(k_all[0]); i++) {
        led_indicator_stop(s_led, k_all[i]);
    }
    led_indicator_set_rgb(s_led, SET_IRGB(0, color.r, color.g, color.b));
    led_indicator_start(s_led, effect_id(color.effect));
}

void korvo_board_bind(void)
{
}
