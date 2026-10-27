#include "audio_chime.h"
#include "ble_transport_stub.h"
#include "buddy_proto.h"
#include "buddy_state.h"
#include "input_handler.h"
#include "pet_ui.h"
#include "status_led.h"

#include <stdio.h>
#include <string.h>

static int g_fail;

#define CHECK(cond)                                                                                                    \
    do {                                                                                                               \
        if (!(cond)) {                                                                                                 \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);                                            \
            g_fail++;                                                                                                  \
        }                                                                                                              \
    } while (0)

static const char *SAMPLE =
    "{\"type\":\"heartbeat\",\"ts\":1730000000,\"total\":1,\"running\":0,\"waiting\":1,"
    "\"msg\":\"等待批准\",\"entries\":[{\"t\":\"11:02:01\",\"line\":\"Running grep\"},{\"t\":\"11:02:05\","
    "\"line\":\"Tool: Shell\"}],\"tokens\":128450,\"tokens_today\":42000,\"tokens_estimated\":true,"
    "\"prompt\":{\"id\":\"prm_8f3a2c\",\"tool\":\"run_terminal_cmd\",\"hint\":\"npm test\"}}";

static void test_proto(void)
{
    buddy_line_t msg;
    CHECK(buddy_proto_parse(SAMPLE, &msg) == BUDDY_LINE_HEARTBEAT);
    CHECK(msg.snap.waiting == 1);
    CHECK(msg.snap.tokens_today == 42000);
    CHECK(msg.snap.tokens == 128450);
    CHECK(msg.snap.tokens_estimated);
    CHECK(msg.snap.entry_count == 2);
    CHECK(strcmp(msg.snap.entries[0].line, "Running grep") == 0);
    CHECK(strcmp(msg.snap.entries[1].t, "11:02:05") == 0);
    CHECK(msg.snap.prompt.valid);
    CHECK(strcmp(msg.snap.prompt.id, "prm_8f3a2c") == 0);
    CHECK(strcmp(msg.snap.prompt.tool, "run_terminal_cmd") == 0);

    char perm[128];
    CHECK(buddy_proto_format_permission(msg.snap.prompt.id, true, perm, sizeof(perm)) > 0);
    CHECK(strstr(perm, "\"decision\":\"approve\"") != NULL);

    CHECK(buddy_proto_parse("{\"type\":\"status\",\"id\":\"prm_8f3a2c\",\"ok\":true,\"detail\":\"approved\"}", &msg) ==
          BUDDY_LINE_STATUS);
    CHECK(msg.status_ok);
    CHECK(strcmp(msg.status_detail, "approved") == 0);
    CHECK(buddy_proto_parse("{not json", &msg) == BUDDY_LINE_ERR);
    CHECK(buddy_proto_parse("{\"type\":\"pong\"}", &msg) == BUDDY_LINE_OTHER);
}

static void test_state_and_ui(void)
{
    buddy_state_init();
    audio_chime_reset();
    pet_ui_init();
    CHECK(buddy_state_get() == BUDDY_STATE_SLEEP);

    buddy_line_t msg;
    buddy_proto_parse(SAMPLE, &msg);
    buddy_state_on_heartbeat(&msg.snap, 1000);
    CHECK(buddy_state_get() == BUDDY_STATE_WAITING);
    CHECK(audio_chime_on_state(buddy_state_get()) == true);
    CHECK(audio_chime_on_state(buddy_state_get()) == false);

    status_led_color_t led = status_led_for_state(BUDDY_STATE_WAITING);
    CHECK(led.r > 200 && led.g > 100 && led.b == 0);
    CHECK(strcmp(led.effect, "blink_fast") == 0);

    char frame[1024];
    CHECK(pet_ui_format(frame, sizeof(frame), BUDDY_STATE_WAITING, buddy_state_snapshot()) > 0);
    CHECK(strstr(frame, "约") != NULL);
    CHECK(strstr(frame, "需要你的批准") != NULL);
    CHECK(strstr(frame, "Running grep") != NULL);

    size_t zones = 0;
    const pet_zone_t *z = pet_ui_zones(&zones);
    CHECK(zones == 5);
    CHECK(z[0].w == 800 && z[1].w == 320 && z[2].x == 320);

    buddy_state_on_status_ack("prm_8f3a2c", true, 1100);
    CHECK(buddy_state_get() == BUDDY_STATE_IDLE);
    CHECK(!buddy_state_snapshot()->prompt.valid);

    buddy_state_tick(1100 + 16000);
    CHECK(buddy_state_get() == BUDDY_STATE_SLEEP);
}

static int s_got_approve;

static void on_input(buddy_input_action_t action, void *user)
{
    (void)user;
    if (action == BUDDY_INPUT_APPROVE) {
        s_got_approve++;
    }
}

static void test_input_and_ble(void)
{
    buddy_input_action_t act;
    CHECK(input_classify_adc(743, &act) == 0 && act == BUDDY_INPUT_APPROVE);
    CHECK(input_classify_adc(344, &act) == 0 && act == BUDDY_INPUT_DENY);
    CHECK(input_classify_adc(1700, &act) == 0 && act == BUDDY_INPUT_VOL_UP);
    CHECK(input_classify_adc(1218, &act) == 0 && act == BUDDY_INPUT_VOL_DOWN);
    CHECK(input_classify_adc(10, &act) != 0);

    input_handler_init(on_input, NULL);
    input_handler_inject(BUDDY_INPUT_APPROVE);
    input_handler_poll();
    CHECK(s_got_approve == 1);

    pet_ui_set_brightness(80);
    pet_ui_adjust_brightness(-100);
    CHECK(pet_ui_get_brightness() == 5);

    char name[32];
    uint8_t mac[6] = {0x02, 0, 0, 0, 0xa1, 0xb2};
    CHECK(ble_transport_format_name("Cursor-S31", mac, name, sizeof(name)) > 0);
    CHECK(strcmp(name, "Cursor-S31-a1b2") == 0);
}

static int s_lines;

static void on_line(const char *line, void *user)
{
    (void)user;
    if (strstr(line, "heartbeat")) {
        s_lines++;
    }
}

static void test_feed(void)
{
    ble_transport_init(on_line, NULL);
    const char *chunk = "{\"type\":\"heartbeat\",\"total\":1,\"running\":1,\"waiting\":0}\n{\"type\":\"pong\"}\n";
    ble_transport_feed((const uint8_t *)chunk, strlen(chunk));
    CHECK(s_lines == 1);
    CHECK(ble_transport_send_line("{\"type\":\"permission\",\"id\":\"a\",\"decision\":\"deny\"}") > 0);
    CHECK(strstr(ble_transport_last_tx(), "deny") != NULL);
}

int main(void)
{
    test_proto();
    test_state_and_ui();
    test_input_and_ble();
    test_feed();
    if (g_fail) {
        fprintf(stderr, "%d checks failed\n", g_fail);
        return 1;
    }
    printf("host_test ok\n");
    return 0;
}
