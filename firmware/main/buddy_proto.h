#pragma once

#include <stddef.h>

#include "buddy_types.h"

typedef enum {
    BUDDY_PROTO_OK = 0,
    BUDDY_PROTO_IGNORED,
    BUDDY_PROTO_ERR,
} buddy_proto_result_t;

/**
 * 解析单行 NDJSON（不含换行）。
 * 成功解析 heartbeat 时写入 out_snap（不含 last_heartbeat_ms，由状态机填充）。
 */
buddy_proto_result_t buddy_proto_parse_line(const char *line, buddy_snapshot_t *out_snap);

/**
 * 构造 permission 行写入 buf，返回长度（不含 \\n）。失败返回 -1。
 */
int buddy_proto_format_permission(const char *prompt_id, bool approve, char *buf, size_t buf_len);
