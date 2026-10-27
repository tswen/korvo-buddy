#pragma once

#include "buddy_types.h"

void buddy_state_init(void);
buddy_state_t buddy_state_get(void);
const buddy_snapshot_t *buddy_state_snapshot(void);

/** 根据最新 snapshot 与链路健康更新状态机 */
void buddy_state_tick(uint32_t now_ms);

/** 应用心跳解析结果。新的心跳会把 ERROR 拉回由快照推导的状态。 */
void buddy_state_on_heartbeat(const buddy_snapshot_t *snap, uint32_t now_ms);

/** 主机 status ok 且 id 匹配时清掉等待中的 prompt。 */
void buddy_state_on_status_ack(const char *id, bool ok, uint32_t now_ms);

void buddy_state_on_link_lost(void);
void buddy_state_on_parse_error(void);
