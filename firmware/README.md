# firmware — Korvo 爪爪

ESP-IDF 应用骨架，目标 **esp32s31**。显示、按键、WS2812、喇叭走公开 BSP [`espressif/esp32_s31_korvo_1`](https://components.espressif.com/components/espressif/esp32_s31_korvo_1)（ESP-IDF ≥ 5.4）。本目录在没有 IDF 时仍可用主机 gcc 测协议和状态机。

## 模块

| 文件 | 作用 |
|---|---|
| `main.c` | 主循环：收行、状态机、UI 帧、灯色 |
| `buddy_state.c` | SLEEP / IDLE / RUNNING / WAITING / ERROR |
| `buddy_proto.c` | NDJSON：`heartbeat`（含 `entries[]`）、`status`、`permission` |
| `pet_ui.c` | 800×480 五区布局 + 文本帧（LVGL 调用写在 `pet_ui_init` 注释里） |
| `input_handler.c` | ADC 窗口 → PLAY 批准 / SET 拒绝 / VOL± 亮度 |
| `ble_transport_stub.c` | NUS 行缓冲；`ble_transport_feed` / `send_line` |
| `status_led.c` | 状态 → RGB + 效果名（GPIO37 = `BSP_LED_RGB_IO`） |
| `audio_chime.c` | 进入 WAITING 的边沿；ES8389 播放留给 BSP |
| `board_map.h` | 从公开 `esp-bsp` 抄来的灯脚和 ADC 窗口 |

协议见 [REFERENCE.md](../REFERENCE.md)。

## 按键

公开 `bsp_button.c` 四个 ADC 键（ADC1 CH0）：`BSP_BUTTON_SET`、`BSP_BUTTON_MODE`、`BSP_BUTTON_VOLP`、`BSP_BUTTON_VOLM`。用户指南丝印是 PLAY、SET、VOL+、VOL-。本仓库把 **MODE 当作 PLAY（批准）**。BSP 头文件没有写 PLAY 这个名字，上板后核对丝印再改 `input_handler.c`。

## 灯

`BSP_LED_RGB_IO` 是 **GPIO37**。有一页用户指南写 RGB 由 GPIO8 驱动。以 BSP 头文件为准，骨架不直接操作 GPIO。

## 不在本骨架里链接的 BSP 调用

实机取消 `main/idf_component.yml` 里的注释后使用：

- `bsp_display_start()`、`bsp_display_lock()` / `unlock()`、`bsp_display_brightness_set()`
- `bsp_iot_button_create()`
- `bsp_led_indicator_create()`、`bsp_led_set()`
- `bsp_audio_codec_speaker_init()`（MCLK 未接，BSP 建议 16 kHz）

## 主机侧自测（无需 IDF）

```bash
make -C firmware/host_test test
```

## 实板编译

```bash
cd firmware
# 编辑 main/idf_component.yml，启用 esp32_s31_korvo_1
# main/CMakeLists.txt 的 REQUIRES 加上 esp32_s31_korvo_1
idf.py set-target esp32s31
idf.py build
idf.py -p PORT flash monitor
```
