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
     * Edge is the cue. Playback, when the BSP is linked:
     *   esp_codec_dev_handle_t spk = bsp_audio_codec_speaker_init();
     *   esp_codec_dev_open / esp_codec_dev_write a short tone, then close.
     * MCLK is unconnected on ESP32-S31; the BSP notes 16 kHz.
     */
    return edge;
}
