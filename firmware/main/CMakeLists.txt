idf_component_register(
    SRCS
        "main.c"
        "buddy_state.c"
        "buddy_proto.c"
        "pet_ui.c"
        "input_handler.c"
        "ble_transport_stub.c"
        "status_led.c"
        "audio_chime.c"
        "board_bsp.c"
    INCLUDE_DIRS "."
    REQUIRES esp32_s31_korvo_1
)
