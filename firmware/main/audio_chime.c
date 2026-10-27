#include "audio_chime.h"

static buddy_state_t s_prev = BUDDY_STATE_SLEEP;

void audio_chime_reset(void)
{
    s_prev = BUDDY_STATE_SLEEP;
}

bool audio_chime_on_state(buddy_state_t state)
{
    bool edge = s_prev != BUDDY_STATE_WAITING && state == BUDDY_STATE_WAITING;
    s_prev = state;
    /*
     * M1 does not open ES8389. The edge is only a log cue.
     * Later: bsp_audio_codec_speaker_init(), then esp_codec_dev_write.
     * ESP32-S31 leaves MCLK unconnected; the BSP recommends 16 kHz.
     */
    return edge;
}
