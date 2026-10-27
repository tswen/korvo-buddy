#pragma once

#include "buddy_types.h"

void buddy_state_init(void);
buddy_state_t buddy_state_get(void);
const buddy_snapshot_t *buddy_state_snapshot(void);

/** 根据最新 snapshot 与链路健康更新状态机 */
void buddy_state_tick(uint32_t now_ms);

/** 应用心跳解析结果 */
void buddy_state_on_heartbeat(const buddy_snapshot_t *snap, uint32_t now_ms);

void buddy_state_on_link_lost(void);
void buddy_state_on_parse_error(void);
