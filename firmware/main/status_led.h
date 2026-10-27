#pragma once

#include <stdint.h>

#include "buddy_types.h"

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    const char *effect; /* breathe_slow, solid, pulse, blink_fast */
} status_led_color_t;

/** Desired WS2812 color for a buddy state. Hardware write is separate. */
status_led_color_t status_led_for_state(buddy_state_t state);

/** Remember the last requested color. BSP path calls bsp_led_indicator_create(). */
void status_led_apply(status_led_color_t color);
status_led_color_t status_led_current(void);
