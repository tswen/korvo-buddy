#include "input_handler.h"

#include "board_map.h"

typedef struct {
    const char *bsp_name;
    const char *silk;
    int min_raw;
    int max_raw;
    buddy_input_action_t action;
} korvo_key_t;

static const korvo_key_t s_keys[] = {
    {"BSP_BUTTON_SET", "SET", KORVO_ADC_SET_MIN, KORVO_ADC_SET_MAX, BUDDY_INPUT_DENY},
    {"BSP_BUTTON_MODE", "PLAY", KORVO_ADC_MODE_MIN, KORVO_ADC_MODE_MAX, BUDDY_INPUT_APPROVE},
    {"BSP_BUTTON_VOLM", "VOL-", KORVO_ADC_VOLM_MIN, KORVO_ADC_VOLM_MAX, BUDDY_INPUT_VOL_DOWN},
    {"BSP_BUTTON_VOLP", "VOL+", KORVO_ADC_VOLP_MIN, KORVO_ADC_VOLP_MAX, BUDDY_INPUT_VOL_UP},
};

static buddy_input_cb_t s_cb;
static void *s_user;
static int s_pending = 0;
static buddy_input_action_t s_pending_action;

void input_handler_init(buddy_input_cb_t cb, void *user)
{
    s_cb = cb;
    s_user = user;
    s_pending = 0;
    /*
     * Device path:
     *   button_handle_t btns[BSP_BUTTON_NUM];
     *   bsp_iot_button_create(btns, NULL, BSP_BUTTON_NUM);
     *   iot_button_register_cb(btns[i], BUTTON_PRESS_DOWN, ..., on_press, (void *)(intptr_t)i);
     * ADC windows live in s_keys, copied from public bsp_button.c.
     */
}

int input_classify_adc(int raw, buddy_input_action_t *out)
{
    for (size_t i = 0; i < sizeof(s_keys) / sizeof(s_keys[0]); i++) {
        if (raw >= s_keys[i].min_raw && raw <= s_keys[i].max_raw) {
            if (out) {
                *out = s_keys[i].action;
            }
            return 0;
        }
    }
    return -1;
}

const char *input_action_name(buddy_input_action_t action)
{
    switch (action) {
    case BUDDY_INPUT_APPROVE:
        return "approve";
    case BUDDY_INPUT_DENY:
        return "deny";
    case BUDDY_INPUT_VOL_UP:
        return "vol_up";
    case BUDDY_INPUT_VOL_DOWN:
        return "vol_down";
    default:
        return "unknown";
    }
}

void input_handler_inject(buddy_input_action_t action)
{
    s_pending_action = action;
    s_pending = 1;
}

void input_handler_poll(void)
{
    if (!s_pending || !s_cb) {
        s_pending = 0;
        return;
    }
    s_pending = 0;
    s_cb(s_pending_action, s_user);
}
