#include "input_handler.h"

static buddy_input_cb_t s_cb;
static void *s_user;

void input_handler_init(buddy_input_cb_t cb, void *user)
{
    s_cb = cb;
    s_user = user;
    /*
     * TODO: 使用 esp32_s31_korvo_1 BSP 按键 API
     *   PLAY  -> BUDDY_INPUT_APPROVE
     *   SET   -> BUDDY_INPUT_DENY
     *   VOL+  -> BUDDY_INPUT_VOL_UP
     *   VOL-  -> BUDDY_INPUT_VOL_DOWN
     */
}

void input_handler_poll(void)
{
    /* TODO: 读取按键事件并调用 s_cb */
    (void)s_cb;
    (void)s_user;
}
