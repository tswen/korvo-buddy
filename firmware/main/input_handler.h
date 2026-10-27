#pragma once

#include "buddy_types.h"

typedef void (*buddy_input_cb_t)(buddy_input_action_t action, void *user);

void input_handler_init(buddy_input_cb_t cb, void *user);

/** 轮询 ADC 按键；实机使用 BSP button 或 esp_adc 校准 */
void input_handler_poll(void);
