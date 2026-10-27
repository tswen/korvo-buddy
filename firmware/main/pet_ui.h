#pragma once

#include "buddy_types.h"

/** 描述 800×480 LVGL 布局；实机由 BSP + esp_lvgl_port 创建 widget */
void pet_ui_init(void);
void pet_ui_deinit(void);

/** 根据状态与快照刷新各区域（桩：仅记录，实机更新 LVGL） */
void pet_ui_render(buddy_state_t state, const buddy_snapshot_t *snap);

/** 亮度 0–100 */
void pet_ui_set_brightness(int percent);
void pet_ui_adjust_brightness(int delta);
