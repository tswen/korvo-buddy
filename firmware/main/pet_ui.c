#include "pet_ui.h"

#include <stdio.h>

/*
 * LVGL 布局区（见 docs/ui-wireframe.md）:
 *   - header_bar   (0,0,800,40)
 *   - pet_face     (0,40,320,400)
 *   - status_panel (320,40,480,400)
 *   - footer_hint  (0,440,800,40)
 *   - wait_overlay (全屏, WAITING 时可见)
 *
 * TODO: bsp_display_start() + lvgl_port_lock/unlock
 * TODO: 加载精灵 assets/pet_sleep.png 等
 */

static int s_brightness = 80;

void pet_ui_init(void)
{
    /* TODO: esp32_s31_korvo_1 BSP 显示与触摸初始化 */
    s_brightness = 80;
}

void pet_ui_deinit(void)
{
    /* TODO: 释放 LVGL / 显示资源 */
}

void pet_ui_set_brightness(int percent)
{
    if (percent < 0) {
        percent = 0;
    }
    if (percent > 100) {
        percent = 100;
    }
    s_brightness = percent;
    /* TODO: bsp_display_brightness_set() 或背光 PWM */
}

void pet_ui_adjust_brightness(int delta)
{
    pet_ui_set_brightness(s_brightness + delta);
}

void pet_ui_render(buddy_state_t state, const buddy_snapshot_t *snap)
{
    (void)s_brightness;
    /* 桩：开发阶段可接 ESP_LOG；避免无 IDF 时链接失败，此处为空操作 */
    (void)state;
    (void)snap;
#if 0
    ESP_LOGI("pet_ui", "state=%d waiting=%d msg=%s", state, snap->waiting, snap->msg);
#endif
}
