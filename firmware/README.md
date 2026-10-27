# firmware — Korvo 爪爪

ESP-IDF 应用，目标 **esp32s31**。屏、ADC 按键、WS2812 走公开 BSP [`espressif/esp32_s31_korvo_1`](https://components.espressif.com/components/espressif/esp32_s31_korvo_1) `^1.0.1`。BLE 仍是 NUS 行缓冲桩。ES8389 等待音只记边沿，不打开 codec。

没有开发板时，主机 gcc 测协议和状态机（不编译 `board_bsp.c`）。

## 丝印和 BSP 枚举

| 丝印 | BSP 枚举 | 动作 |
|---|---|---|
| PLAY | `BSP_BUTTON_MODE` | 批准 |
| SET | `BSP_BUTTON_SET` | 拒绝 |
| VOL+ | `BSP_BUTTON_VOLP` | 亮度 +5（界面上的百分比） |
| VOL- | `BSP_BUTTON_VOLM` | 亮度 −5 |

公开头文件没有 PLAY 这个名字。`bsp_button.c` 的四个 ADC 窗口（ADC1 CH0）和丝印一一对应，MODE 就是 PLAY。`board_bsp.c` 按这个接。

灯是 `BSP_LED_RGB_IO`（GPIO37）上的 WS2812，经 `bsp_led_indicator_create()`。颜色：休眠暗蓝呼吸、待命青、执行绿、等待琥珀快闪、错误红。

## 亮度

`bsp_display_brightness_set()` 在这份 BSP 里会打日志并返回 `ESP_ERR_NOT_SUPPORTED`（`BSP_LCD_BACKLIGHT` 是 `GPIO_NUM_NC`）。VOL 仍会改屏幕上的百分比。背光硬件没接到这个 API。

## 提示音

进入 WAITING 时 `audio_chime_on_state()` 返回 true，主循环打一条 log。没有调用 `bsp_audio_codec_speaker_init()`。ESP32-S31 的 MCLK 未接，BSP 建议 16 kHz。

## 实板编译（已在本机用官方 display 例程验证的工具链）

串口是 CP2102N，设备节点 `/dev/ttyUSB0`。IDF 6.1 里 `esp32s31` 还要带 `--preview`。

```bash
. ~/idf/esp-idf6.1/export.sh
cd firmware
idf.py --preview set-target esp32s31
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

若 `build` / `flash` 仍提示目标处于预览，给那两条也加上 `--preview`。

`sdkconfig.defaults` 对齐官方 `examples/display/sdkconfig.bsp.esp32_s31_korvo_1`（PSRAM、RGB bounce buffer、LVGL 9），并打开 `CONFIG_LV_FONT_SIMSUN_16_CJK`，用来画「约」和中文状态。某条 CONFIG 若报未知，删掉那一行；SPIRAM 和 `CONFIG_BSP_LCD_*` 是点亮 SUB3 用的，留着。

依赖在 `main/idf_component.yml`，第一次构建会从组件注册表拉 BSP。

## 主机侧自测（无需 IDF、无需板子）

```bash
make -C firmware/host_test test
cd host/cursor-bridge && PYTHONPATH=. python3 -m unittest tests.test_bridge
```

## 模块

| 文件 | 作用 |
|---|---|
| `board_bsp.c` | `bsp_display_start`、LVGL 800×480、按键、WS2812 |
| `pet_ui.c` | 文本帧（主机测试）+ 调用板上 UI |
| `buddy_state.c` / `buddy_proto.c` | 状态机和 NDJSON |
| `ble_transport_stub.c` | NUS 行缓冲，尚未 NimBLE |
| `audio_chime.c` | WAITING 边沿，无音频输出 |
