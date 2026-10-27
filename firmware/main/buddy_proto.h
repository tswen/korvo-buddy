#pragma once

#include <stddef.h>

#include "buddy_types.h"

typedef enum {
    BUDDY_PROTO_OK = 0,
    BUDDY_PROTO_IGNORED,
    BUDDY_PROTO_ERR,
} buddy_proto_result_t;

typedef enum {
    BUDDY_LINE_EMPTY = 0,
    BUDDY_LINE_HEARTBEAT,
    BUDDY_LINE_STATUS,
    BUDDY_LINE_OTHER,
    BUDDY_LINE_ERR,
} buddy_line_kind_t;

typedef struct {
    buddy_line_kind_t kind;
    buddy_snapshot_t snap;
    char status_id[BUDDY_PROMPT_ID_MAX];
    bool status_ok;
    char status_detail[48];
} buddy_line_t;

/** Parse one NDJSON object (newline already stripped). */
buddy_line_kind_t buddy_proto_parse(const char *line, buddy_line_t *out);

/** Heartbeat-only helper. Returns BUDDY_PROTO_OK when kind is heartbeat. */
buddy_proto_result_t buddy_proto_parse_line(const char *line, buddy_snapshot_t *out_snap);

/**
 * Write a permission object without the trailing newline.
 * Returns byte count, or -1 if the buffer is too small.
 */
int buddy_proto_format_permission(const char *prompt_id, bool approve, char *buf, size_t buf_len);
