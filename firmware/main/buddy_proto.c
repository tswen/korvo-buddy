#include "buddy_proto.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *find_key(const char *json, const char *key)
{
    size_t klen = strlen(key);
    const char *p = json;
    while ((p = strstr(p, key)) != NULL) {
        if (p > json && p[-1] == '"' && p[klen] == '"') {
            return p - 1;
        }
        p += klen;
    }
    return NULL;
}

static const char *skip_ws_colon(const char *p)
{
    if (!p) {
        return NULL;
    }
    p = strchr(p, ':');
    if (!p) {
        return NULL;
    }
    p++;
    while (*p == ' ' || *p == '\t') {
        p++;
    }
    return p;
}

static int extract_json_string(const char *json, const char *key, char *out, size_t out_len)
{
    const char *p = skip_ws_colon(find_key(json, key));
    if (!p || *p != '"' || out_len == 0) {
        return -1;
    }
    p++;
    size_t i = 0;
    while (*p && *p != '"') {
        char c = *p++;
        if (c == '\\' && *p) {
            char e = *p++;
            if (e == 'n') {
                c = '\n';
            } else if (e == 't') {
                c = '\t';
            } else {
                c = e;
            }
        }
        if (i + 1 < out_len) {
            out[i++] = c;
        }
    }
    out[i] = '\0';
    return (int)i;
}

static int extract_json_i64(const char *json, const char *key, int64_t *out)
{
    const char *p = skip_ws_colon(find_key(json, key));
    if (!p) {
        return -1;
    }
    char *end = NULL;
    long long v = strtoll(p, &end, 10);
    if (end == p) {
        return -1;
    }
    *out = (int64_t)v;
    return 0;
}

static bool extract_json_bool(const char *json, const char *key, bool *out)
{
    const char *p = skip_ws_colon(find_key(json, key));
    if (!p) {
        return false;
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

static const char *skip_object(const char *p)
{
    int depth = 0;
    bool in_str = false;
    if (*p != '{') {
        return p;
    }
    for (; *p; p++) {
        if (in_str) {
            if (*p == '\\' && p[1]) {
                p++;
                continue;
            }
            if (*p == '"') {
                in_str = false;
            }
            continue;
        }
        if (*p == '"') {
            in_str = true;
        } else if (*p == '{') {
            depth++;
        } else if (*p == '}') {
            depth--;
            if (depth == 0) {
                return p + 1;
            }
        }
    }
    return p;
}

static void parse_prompt_block(const char *json, buddy_prompt_t *prompt)
{
    memset(prompt, 0, sizeof(*prompt));
    const char *key = find_key(json, "prompt");
    const char *p = skip_ws_colon(key);
    if (!p || *p != '{') {
        return;
    }
    const char *end = skip_object(p);
    size_t n = (size_t)(end - p);
    char tmp[384];
    if (n >= sizeof(tmp)) {
        n = sizeof(tmp) - 1;
    }
    memcpy(tmp, p, n);
    tmp[n] = '\0';
    extract_json_string(tmp, "id", prompt->id, sizeof(prompt->id));
    extract_json_string(tmp, "tool", prompt->tool, sizeof(prompt->tool));
    extract_json_string(tmp, "hint", prompt->hint, sizeof(prompt->hint));
    prompt->valid = prompt->id[0] != '\0';
}

static void parse_entries(const char *json, buddy_snapshot_t *snap)
{
    const char *key = find_key(json, "entries");
    const char *p = skip_ws_colon(key);
    if (!p || *p != '[') {
        return;
    }
    p++;
    while (*p && snap->entry_count < BUDDY_LOG_ENTRY_MAX) {
        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == ',') {
            p++;
        }
        if (*p == ']' || *p == '\0') {
            break;
        }
        if (*p != '{') {
            break;
        }
        const char *end = skip_object(p);
        size_t n = (size_t)(end - p);
        char tmp[384];
        if (n >= sizeof(tmp)) {
            n = sizeof(tmp) - 1;
        }
        memcpy(tmp, p, n);
        tmp[n] = '\0';
        buddy_log_entry_t *e = &snap->entries[snap->entry_count];
        if (extract_json_string(tmp, "line", e->line, sizeof(e->line)) >= 0) {
            extract_json_string(tmp, "t", e->t, sizeof(e->t));
            snap->entry_count++;
        }
        p = end;
    }
}

static buddy_line_kind_t parse_heartbeat(const char *line, buddy_line_t *out)
{
    int64_t total = 0;
    int64_t running = 0;
    int64_t waiting = 0;
    int got = 0;
    got += extract_json_i64(line, "total", &total) == 0;
    got += extract_json_i64(line, "running", &running) == 0;
    got += extract_json_i64(line, "waiting", &waiting) == 0;
    if (got == 0) {
        return BUDDY_LINE_ERR;
    }

    memset(&out->snap, 0, sizeof(out->snap));
    out->snap.total = (int)total;
    out->snap.running = (int)running;
    out->snap.waiting = (int)waiting;
    extract_json_string(line, "msg", out->snap.msg, sizeof(out->snap.msg));

    int64_t tok = 0;
    if (extract_json_i64(line, "tokens", &tok) == 0) {
        out->snap.tokens = tok;
    }
    if (extract_json_i64(line, "tokens_today", &tok) == 0) {
        out->snap.tokens_today = tok;
    }
    bool est = false;
    if (extract_json_bool(line, "tokens_estimated", &est)) {
        out->snap.tokens_estimated = est;
    }
    parse_prompt_block(line, &out->snap.prompt);
    parse_entries(line, &out->snap);
    return BUDDY_LINE_HEARTBEAT;
}

static buddy_line_kind_t parse_status(const char *line, buddy_line_t *out)
{
    if (extract_json_string(line, "id", out->status_id, sizeof(out->status_id)) < 0) {
        return BUDDY_LINE_ERR;
    }
    bool ok = false;
    if (!extract_json_bool(line, "ok", &ok)) {
        return BUDDY_LINE_ERR;
    }
    out->status_ok = ok;
    out->status_detail[0] = '\0';
    extract_json_string(line, "detail", out->status_detail, sizeof(out->status_detail));
    return BUDDY_LINE_STATUS;
}

buddy_line_kind_t buddy_proto_parse(const char *line, buddy_line_t *out)
{
    if (!out) {
        return BUDDY_LINE_ERR;
    }
    memset(out, 0, sizeof(*out));
    if (!line) {
        return BUDDY_LINE_ERR;
    }
    while (*line == ' ' || *line == '\t') {
        line++;
    }
    if (*line == '\0') {
        return BUDDY_LINE_EMPTY;
    }

    char type[32];
    if (extract_json_string(line, "type", type, sizeof(type)) < 0) {
        out->kind = (*line == '{') ? BUDDY_LINE_ERR : BUDDY_LINE_OTHER;
        return out->kind;
    }

    if (strcmp(type, "heartbeat") == 0) {
        out->kind = parse_heartbeat(line, out);
    } else if (strcmp(type, "status") == 0) {
        out->kind = parse_status(line, out);
    } else {
        out->kind = BUDDY_LINE_OTHER;
    }
    return out->kind;
}

buddy_proto_result_t buddy_proto_parse_line(const char *line, buddy_snapshot_t *out_snap)
{
    buddy_line_t msg;
    buddy_line_kind_t kind = buddy_proto_parse(line, &msg);
    if (kind == BUDDY_LINE_ERR) {
        return BUDDY_PROTO_ERR;
    }
    if (kind != BUDDY_LINE_HEARTBEAT) {
        return BUDDY_PROTO_IGNORED;
    }
    if (out_snap) {
        *out_snap = msg.snap;
    }
    return BUDDY_PROTO_OK;
}

int buddy_proto_format_permission(const char *prompt_id, bool approve, char *buf, size_t buf_len)
{
    if (!prompt_id || !buf || buf_len < 32 || strchr(prompt_id, '"') || strchr(prompt_id, '\\')) {
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
