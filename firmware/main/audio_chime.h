#pragma once

#include <stdbool.h>

#include "buddy_types.h"

/** True on the rising edge into WAITING. ES8389 playback stays on the BSP. */
bool audio_chime_on_state(buddy_state_t state);

void audio_chime_reset(void);
