idf_component_register(
    SRCS
        "main.c"
        "buddy_state.c"
        "buddy_proto.c"
        "pet_ui.c"
        "input_handler.c"
        "ble_transport_stub.c"
    INCLUDE_DIRS "."
    REQUIRES
        # 实机启用 BSP 时在 idf_component.yml 添加 esp32_s31_korvo_1 后取消注释:
        # esp32_s31_korvo_1
)
