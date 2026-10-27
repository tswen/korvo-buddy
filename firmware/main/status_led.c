#include "status_led.h"

#include "board_hooks.h"
#include "board_map.h"

#include <string.h>

__attribute__((weak)) void status_led_board_apply(status_led_color_t color)
{
    (void)color;
}

static status_led_color_t s_current;

status_led_color_t status_led_for_state(buddy_state_t state)
{
    switch (state) {
    case BUDDY_STATE_IDLE:
        return (status_led_color_t){0, 180, 180, "solid"};
    case BUDDY_STATE_RUNNING:
        return (status_led_color_t){0, 180, 40, "pulse"};
    case BUDDY_STATE_WAITING:
        return (status_led_color_t){220, 140, 0, "blink_fast"};
    case BUDDY_STATE_ERROR:
        return (status_led_color_t){200, 0, 0, "solid"};
    case BUDDY_STATE_SLEEP:
    default:
        return (status_led_color_t){0, 0, 40, "breathe_slow"};
    }
}

void status_led_apply(status_led_color_t color)
{
    bool same = s_current.effect != NULL && color.effect != NULL && s_current.r == color.r &&
                s_current.g == color.g && s_current.b == color.b && strcmp(s_current.effect, color.effect) == 0;
    s_current = color;
    if (!same) {
        status_led_board_apply(color);
    }
    (void)KORVO_LED_RGB_GPIO;
}

status_led_color_t status_led_current(void)
{
    return s_current;
}
