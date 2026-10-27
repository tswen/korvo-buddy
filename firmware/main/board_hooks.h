#pragma once

#include "buddy_types.h"
#include "status_led.h"

/*
 * Strong definitions live in board_bsp.c and are linked only by the IDF build.
 * Host gcc tests keep the weak empty bodies in the matching .c files.
 */

void pet_ui_board_init(void);
void pet_ui_board_render(buddy_state_t state, const buddy_snapshot_t *snap);
void pet_ui_board_set_brightness(int percent);

void input_board_init(void);
void status_led_board_apply(status_led_color_t color);

/* Referenced from app_main so the linker pulls board_bsp.c. */
void korvo_board_bind(void);
