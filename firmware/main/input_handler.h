#pragma once

#include "buddy_types.h"

typedef void (*buddy_input_cb_t)(buddy_input_action_t action, void *user);

void input_handler_init(buddy_input_cb_t cb, void *user);

/** Host and tests push a logical key. Device code calls this from the BSP callback. */
void input_handler_inject(buddy_input_action_t action);

/**
 * Classify a raw ADC sample using the public bsp_button.c windows.
 * Returns 0 and writes *out on a hit, -1 otherwise.
 * BSP_BUTTON_MODE is reported as APPROVE (silkscreen PLAY).
 */
int input_classify_adc(int raw, buddy_input_action_t *out);

const char *input_action_name(buddy_input_action_t action);

void input_handler_poll(void);
