#pragma once

#include <stddef.h>

#include "buddy_types.h"

typedef struct {
    int x;
    int y;
    int w;
    int h;
    const char *id;
} pet_zone_t;

/** Fixed 800×480 zones. Count is 5 (header, pet, status, footer, overlay). */
const pet_zone_t *pet_ui_zones(size_t *count);

const char *pet_ui_anim_name(buddy_state_t state);

void pet_ui_init(void);
void pet_ui_deinit(void);

void pet_ui_set_brightness(int percent);
void pet_ui_adjust_brightness(int delta);
int pet_ui_get_brightness(void);

/** Fill buf with a text frame of the current screen. Returns bytes written. */
int pet_ui_format(char *buf, size_t len, buddy_state_t state, const buddy_snapshot_t *snap);

void pet_ui_render(buddy_state_t state, const buddy_snapshot_t *snap);
