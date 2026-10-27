#include "buddy_state.h"

#include <string.h>

static buddy_snapshot_t s_snap;
static buddy_state_t s_state = BUDDY_STATE_SLEEP;

void buddy_state_init(void)
{
    memset(&s_snap, 0, sizeof(s_snap));
    s_state = BUDDY_STATE_SLEEP;
}

buddy_state_t buddy_state_get(void)
{
    return s_state;
}

const buddy_snapshot_t *buddy_state_snapshot(void)
{
    return &s_snap;
}

static buddy_state_t derive_state(const buddy_snapshot_t *snap, uint32_t now_ms)
{
    if (snap->last_heartbeat_ms == 0) {
        return BUDDY_STATE_SLEEP;
    }
    if ((now_ms - snap->last_heartbeat_ms) > BUDDY_HEARTBEAT_TIMEOUT_MS) {
        return BUDDY_STATE_SLEEP;
    }
    if (snap->waiting > 0) {
        return BUDDY_STATE_WAITING;
    }
    if (snap->running > 0) {
        return BUDDY_STATE_RUNNING;
    }
    return BUDDY_STATE_IDLE;
}

void buddy_state_on_heartbeat(const buddy_snapshot_t *snap, uint32_t now_ms)
{
    s_snap = *snap;
    s_snap.last_heartbeat_ms = now_ms;
    s_state = derive_state(&s_snap, now_ms);
}

void buddy_state_tick(uint32_t now_ms)
{
    if (s_state == BUDDY_STATE_ERROR) {
        return;
    }
    s_state = derive_state(&s_snap, now_ms);
}

void buddy_state_on_link_lost(void)
{
    memset(&s_snap, 0, sizeof(s_snap));
    s_state = BUDDY_STATE_SLEEP;
}

void buddy_state_on_parse_error(void)
{
    s_state = BUDDY_STATE_ERROR;
}
