#include "buddy_proto.h"

#include <stdio.h>
#include <string.h>

/*
 * MVP：轻量字符串扫描，避免依赖 cJSON（实机可换 cJSON / jsmn）。
 * 仅处理 REFERENCE.md 中的 heartbeat / status 形状。
 */

static int extract_json_string(const char *json, const char *key, char *out, size_t out_len)
{
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char *p = strstr(json, pattern);
    if (!p) {
        return -1;
    }
    p = strchr(p + strlen(pattern), ':');
    if (!p) {
        return -1;
    }
    p++;
    while (*p == ' ' || *p == '\t') {
        p++;
    }
    if (*p != '"') {
        return -1;
    }
    p++;
    size_t i = 0;
    while (*p && *p != '"' && i + 1 < out_len) {
        if (*p == '\\' && p[1]) {
            p++;
        }
        out[i++] = *p++;
    }
    out[i] = '\0';
    return (int)i;
}

static int extract_json_int(const char *json, const char *key, int *out)
{
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char *p = strstr(json, pattern);
    if (!p) {
        return -1;
    }
    p = strchr(p + strlen(pattern), ':');
    if (!p) {
        return -1;
    }
    return sscanf(p + 1, "%d", out) == 1 ? 0 : -1;
}

static bool extract_json_bool(const char *json, const char *key, bool *out)
{
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char *p = strstr(json, pattern);
    if (!p) {
        return false;
    }
    p = strchr(p + strlen(pattern), ':');
    if (!p) {
        return false;
    }
    p++;
    while (*p == ' ' || *p == '\t') {
        p++;
    }
    if (strncmp(p, "true", 4) == 0) {
        *out = true;
        return true;
    }
    if (strncmp(p, "false", 5) == 0) {
        *out = false;
        return true;
    }
    return false;
}

static void parse_prompt_block(const char *json, buddy_prompt_t *prompt)
{
    prompt->valid = false;
    const char *block = strstr(json, "\"prompt\"");
    if (!block) {
        return;
    }
    extract_json_string(block, "id", prompt->id, sizeof(prompt->id));
    extract_json_string(block, "tool", prompt->tool, sizeof(prompt->tool));
    extract_json_string(block, "hint", prompt->hint, sizeof(prompt->hint));
    prompt->valid = prompt->id[0] != '\0';
}

buddy_proto_result_t buddy_proto_parse_line(const char *line, buddy_snapshot_t *out_snap)
{
    if (!line || !out_snap) {
        return BUDDY_PROTO_ERR;
    }
    if (strstr(line, "\"type\":\"heartbeat\"") == NULL && strstr(line, "\"type\": \"heartbeat\"") == NULL) {
        if (strstr(line, "\"type\":\"status\"") || strstr(line, "\"type\": \"status\"")) {
            return BUDDY_PROTO_IGNORED;
        }
        if (strstr(line, "\"type\":\"pong\"") || strstr(line, "\"type\": \"pong\"")) {
            return BUDDY_PROTO_IGNORED;
        }
        return BUDDY_PROTO_IGNORED;
    }

    memset(out_snap, 0, sizeof(*out_snap));
    extract_json_int(line, "total", &out_snap->total);
    extract_json_int(line, "running", &out_snap->running);
    extract_json_int(line, "waiting", &out_snap->waiting);
    extract_json_string(line, "msg", out_snap->msg, sizeof(out_snap->msg));

    int tok = 0;
    if (extract_json_int(line, "tokens", &tok) == 0) {
        out_snap->tokens = tok;
    }
    if (extract_json_int(line, "tokens_today", &tok) == 0) {
        out_snap->tokens_today = tok;
    }
    bool est = false;
    if (extract_json_bool(line, "tokens_estimated", &est)) {
        out_snap->tokens_estimated = est;
    }

    parse_prompt_block(line, &out_snap->prompt);

    /* TODO: 解析 entries[] 数组；MVP 保留空 entries，由 UI 显示 msg */
    out_snap->entry_count = 0;

    return BUDDY_PROTO_OK;
}

int buddy_proto_format_permission(const char *prompt_id, bool approve, char *buf, size_t buf_len)
{
    if (!prompt_id || !buf || buf_len < 32) {
        return -1;
    }
    const char *decision = approve ? "approve" : "deny";
    int n = snprintf(buf, buf_len, "{\"type\":\"permission\",\"id\":\"%s\",\"decision\":\"%s\"}", prompt_id,
                     decision);
    if (n < 0 || (size_t)n >= buf_len) {
        return -1;
    }
    return n;
}
