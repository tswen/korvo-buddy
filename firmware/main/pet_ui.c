#include "pet_ui.h"

#include "board_hooks.h"
#include "board_map.h"

#include <stdio.h>
#include <string.h>

__attribute__((weak)) void pet_ui_board_init(void) {}
__attribute__((weak)) void pet_ui_board_render(buddy_state_t state, const buddy_snapshot_t *snap)
{
    (void)state;
    (void)snap;
}
__attribute__((weak)) void pet_ui_board_set_brightness(int percent)
{
    (void)percent;
}

static const pet_zone_t s_zones[] = {
    {0, 0, KORVO_LCD_WIDTH, 40, "header"},
    {0, 40, 320, 400, "pet"},
    {320, 40, 480, 400, "status"},
    {0, 440, KORVO_LCD_WIDTH, 40, "footer"},
    {0, 0, KORVO_LCD_WIDTH, KORVO_LCD_HEIGHT, "overlay"},
};

static int s_brightness = 80;
static char s_frame[1024];

const pet_zone_t *pet_ui_zones(size_t *count)
{
    if (count) {
        *count = sizeof(s_zones) / sizeof(s_zones[0]);
    }
    return s_zones;
}

const char *pet_ui_anim_name(buddy_state_t state)
{
    switch (state) {
    case BUDDY_STATE_IDLE:
        return "blink";
    case BUDDY_STATE_RUNNING:
        return "run";
    case BUDDY_STATE_WAITING:
        return "raise_hand";
    case BUDDY_STATE_ERROR:
        return "confused";
    case BUDDY_STATE_SLEEP:
    default:
        return "sleep_zzz";
    }
}

void pet_ui_init(void)
{
    s_brightness = 80;
    s_frame[0] = '\0';
    pet_ui_board_init();
    pet_ui_board_set_brightness(s_brightness);
}

void pet_ui_deinit(void)
{
    s_frame[0] = '\0';
}

void pet_ui_set_brightness(int percent)
{
    if (percent < 5) {
        percent = 5;
    }
    if (percent > 100) {
        percent = 100;
    }
    s_brightness = percent;
    pet_ui_board_set_brightness(s_brightness);
}

void pet_ui_adjust_brightness(int delta)
{
    pet_ui_set_brightness(s_brightness + delta);
}

int pet_ui_get_brightness(void)
{
    return s_brightness;
}

static const char *state_label(buddy_state_t state)
{
    switch (state) {
    case BUDDY_STATE_IDLE:
        return "IDLE";
    case BUDDY_STATE_RUNNING:
        return "RUNNING";
    case BUDDY_STATE_WAITING:
        return "WAITING";
    case BUDDY_STATE_ERROR:
        return "ERROR";
    case BUDDY_STATE_SLEEP:
    default:
        return "SLEEP";
    }
}

int pet_ui_format(char *buf, size_t len, buddy_state_t state, const buddy_snapshot_t *snap)
{
    if (!buf || len == 0) {
        return -1;
    }
    if (!snap) {
        buf[0] = '\0';
        return 0;
    }

    int n = snprintf(buf, len,
                     "header Korvo state=%s anim=%s bright=%d\n"
                     "pet %s\n"
                     "status run=%d wait=%d total=%d tokens_today=%lld%s tokens=%lld\n"
                     "msg %s\n",
                     state_label(state), pet_ui_anim_name(state), s_brightness, pet_ui_anim_name(state),
                     snap->running, snap->waiting, snap->total, (long long)snap->tokens_today,
                     snap->tokens_estimated ? " 约" : "", (long long)snap->tokens, snap->msg);
    if (n < 0 || (size_t)n >= len) {
        return -1;
    }

    size_t used = (size_t)n;
    size_t shown = snap->entry_count > 6 ? 6 : snap->entry_count;
    size_t start = snap->entry_count - shown;
    for (size_t i = 0; i < shown; i++) {
        const buddy_log_entry_t *e = &snap->entries[start + i];
        int w = snprintf(buf + used, len - used, "log %s %s\n", e->t, e->line);
        if (w < 0 || (size_t)w >= len - used) {
            return -1;
        }
        used += (size_t)w;
    }

    if (state == BUDDY_STATE_WAITING && snap->prompt.valid) {
        int w = snprintf(buf + used, len - used, "overlay 需要你的批准 tool=%s hint=%s\nfooter PLAY=批准 SET=拒绝\n",
                         snap->prompt.tool, snap->prompt.hint);
        if (w < 0 || (size_t)w >= len - used) {
            return -1;
        }
        used += (size_t)w;
    } else {
        int w = snprintf(buf + used, len - used, "footer PLAY=批准 SET=拒绝 VOL=亮度\n");
        if (w < 0 || (size_t)w >= len - used) {
            return -1;
        }
        used += (size_t)w;
    }
    return (int)used;
}

void pet_ui_render(buddy_state_t state, const buddy_snapshot_t *snap)
{
    if (pet_ui_format(s_frame, sizeof(s_frame), state, snap) < 0) {
        s_frame[0] = '\0';
    }
    pet_ui_board_render(state, snap);
}
