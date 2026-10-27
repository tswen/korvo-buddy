# firmware — Korvo 爪爪

ESP-IDF 应用，目标 **esp32s31** + **esp32_s31_korvo_1** BSP。

## 模块

| 文件 | 作用 |
|---|---|
| `main.c` | 主循环：输入、状态机、UI 刷新 |
| `buddy_state.c` | SLEEP / IDLE / RUNNING / WAITING / ERROR |
| `buddy_proto.c` | NDJSON 解析（heartbeat）、permission 序列化 |
| `pet_ui.c` | 800×480 LVGL 布局桩 |
| `input_handler.c` | PLAY/SET/VOL 桩 |
| `ble_transport_stub.c` | NimBLE NUS 桩 |

协议见仓库根目录 [REFERENCE.md](../REFERENCE.md)。

## 构建

1. 取消 `main/idf_component.yml` 中 `esp32_s31_korvo_1` 注释  
2. `main/CMakeLists.txt` 的 `REQUIRES` 加入该组件  
3. `idf.py set-target esp32s31 && idf.py build`
