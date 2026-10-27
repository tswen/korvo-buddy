#include "status_led.h"

#include "board_map.h"

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
    s_current = color;
    /*
     * On device, after espressif/esp32_s31_korvo_1 is linked:
     *   led_indicator_handle_t leds[BSP_LED_NUM];
     *   bsp_led_indicator_create(leds, NULL, BSP_LED_NUM);
     *   drive BSP_LED_RGB_IO (GPIO KORVO_LED_RGB_GPIO) with color.effect
     * Do not bit-bang GPIO8: a user-guide sentence names that pin, the BSP
     * header names GPIO37.
     */
    (void)KORVO_LED_RGB_GPIO;
}

status_led_color_t status_led_current(void)
{
    return s_current;
}
