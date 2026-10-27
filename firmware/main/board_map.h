#pragma once

/*
 * Pin and button facts taken from the public BSP header and bsp_button.c:
 *   https://github.com/espressif/esp-bsp/tree/master/bsp/esp32_s31_korvo_1
 *
 * BSP_LED_RGB_IO            = GPIO37
 * Buttons                   = ADC1 channel 0, created by bsp_iot_button_create()
 * BSP enum                  = SET, MODE, VOLP, VOLM
 * Silkscreen (user guide)   = PLAY, SET, VOL+, VOL-
 *
 * This tree treats BSP_BUTTON_MODE as the PLAY key (approve). Confirm on the
 * board before shipping a binary: the public header does not say "PLAY".
 *
 * One user-guide paragraph says the addressable RGB LED is GPIO8. The BSP
 * header says GPIO37. Follow the BSP symbol; do not bit-bang a guessed pin
 * until bsp_led_indicator_create() is linked.
 */

#define KORVO_LED_RGB_GPIO 37
#define KORVO_LCD_WIDTH 800
#define KORVO_LCD_HEIGHT 480

#define KORVO_BTN_ADC_CHANNEL 0

/* Ranges copied from bsp_button.c (iot_button ADC min/max, midpoint in comments). */
#define KORVO_ADC_SET_MIN 244
#define KORVO_ADC_SET_MAX 444
#define KORVO_ADC_MODE_MIN 643
#define KORVO_ADC_MODE_MAX 843
#define KORVO_ADC_VOLM_MIN 1118
#define KORVO_ADC_VOLM_MAX 1318
#define KORVO_ADC_VOLP_MIN 1600
#define KORVO_ADC_VOLP_MAX 1800
