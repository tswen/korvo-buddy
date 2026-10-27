#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define BUDDY_LOG_ENTRY_MAX 8
#define BUDDY_LINE_MAX 120
#define BUDDY_PROMPT_ID_MAX 48
#define BUDDY_TOOL_NAME_MAX 64
#define BUDDY_MSG_MAX 160
#define BUDDY_HEARTBEAT_TIMEOUT_MS 15000

typedef enum {
    BUDDY_STATE_SLEEP = 0,
    BUDDY_STATE_IDLE,
    BUDDY_STATE_RUNNING,
    BUDDY_STATE_WAITING,
    BUDDY_STATE_ERROR,
} buddy_state_t;

typedef struct {
    char t[16];
    char line[BUDDY_LINE_MAX];
} buddy_log_entry_t;

typedef struct {
    char id[BUDDY_PROMPT_ID_MAX];
    char tool[BUDDY_TOOL_NAME_MAX];
    char hint[BUDDY_LINE_MAX];
    bool valid;
} buddy_prompt_t;

typedef struct {
    int total;
    int running;
    int waiting;
    char msg[BUDDY_MSG_MAX];
    buddy_log_entry_t entries[BUDDY_LOG_ENTRY_MAX];
    size_t entry_count;
    int64_t tokens;
    int64_t tokens_today;
    bool tokens_estimated;
    buddy_prompt_t prompt;
    uint32_t last_heartbeat_ms;
} buddy_snapshot_t;

typedef enum {
    BUDDY_INPUT_APPROVE = 0,
    BUDDY_INPUT_DENY,
    BUDDY_INPUT_VOL_UP,
    BUDDY_INPUT_VOL_DOWN,
} buddy_input_action_t;
