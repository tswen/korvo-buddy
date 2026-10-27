# Korvo 爪爪（korvo-buddy）

**English:** A community desk-pet firmware for **ESP32-S31-Korvo-1** that mirrors [Anthropic Hardware Buddy](https://www.anthropic.com/news/hardware-buddy)-style **BLE NDJSON** to show **Cursor Agent** status (running / waiting for approval / recent logs / daily tokens) and approve or deny tool prompts with **PLAY** / **SET**. Not an official Cursor or Anthropic product.

桌宠固件 + PC 桥接骨架：在 Korvo 上显示 Cursor Agent 状态，并用按键批准/拒绝工具调用。

## 硬件

| 项 | 说明 |
|---|---|
| 主板 | [ESP32-S31-Korvo-1 V1.1](https://docs.espressif.com/projects/esp-dev-kits/zh_CN/latest/esp32s31/esp32-s31-korvo-1/user_guide.html)（ESP32-S31-WROOM-3，16MB Flash + 16MB PSRAM） |
| LCD | ESP32-S3-LCD-EV-Board-SUB3，4.3" 800×480 RGB565，触摸 GT1151 |
| 按键 | ADC：PLAY=批准，SET=拒绝，VOL±=亮度/音量 |
| LED | WS2812 @ GPIO37 |
| 音频 | ES8389 扬声器（等待提示音，MVP 桩） |

BSP（实机编译）：[`espressif/esp32_s31_korvo_1`](https://components.espressif.com/components/espressif/esp32_s31_korvo_1)（ESP-IDF ≥ 5.4）。

## 仓库结构

```
REFERENCE.md          # NUS + NDJSON 协议（Hardware Buddy 风格）
docs/
  architecture.md     # 端到端架构图
  ui-wireframe.md     # 800×480 线框与状态动画
firmware/             # ESP-IDF 应用骨架（状态机、协议、UI/BLE 桩）
host/cursor-bridge/   # Python 主机桥（bleak + 串口回退）
```

## 如何协作

1. **主机**：在 PC 上运行 `host/cursor-bridge`，把 Cursor 会话映射为 `heartbeat`（见 `REFERENCE.md`）。
2. **链路**：默认 BLE 名 `Cursor-S31-xxxx`；无蓝牙时用 USB 串口同一 JSON 格式。
3. **设备**：刷入 `firmware/`，解析心跳、渲染 LVGL（待接 BSP）、`WAITING` 时 PLAY/SET 发 `permission`。

```mermaid
flowchart LR
  Cursor --> Bridge[cursor-bridge]
  Bridge -->|BLE NUS| FW[firmware]
  FW -->|permission| Bridge
```

## 固件编译（实板）

需安装 [ESP-IDF 5.4+](https://docs.espressif.com/projects/esp-idf/) 并支持 `esp32s31` 目标。

```bash
cd firmware
# 编辑 main/idf_component.yml，启用 esp32_s31_korvo_1
idf.py set-target esp32s31
idf.py build
idf.py -p PORT flash monitor
```

当前仓库为 **MVP 桩**：`ble_transport_stub`、`pet_ui`、`input_handler` 含 TODO，在无 BSP 的环境可能仅作结构参考；接好 BSP 后实现 NimBLE NUS、LVGL 与按键。

## 主机桥接

见 [host/cursor-bridge/README.md](host/cursor-bridge/README.md)。快速演示（打印 NDJSON）：

```bash
cd host/cursor-bridge
python -m venv .venv && source .venv/bin/activate
pip install -e .
cursor-bridge --demo
```

## MVP 路线图

| 阶段 | 内容 |
|---|---|
| M0（本仓库） | 协议文档、状态机、NDJSON 解析桩、Python 桥骨架 |
| M1 | 启用 Korvo BSP：LCD + LVGL 布局 + WS2812 |
| M2 | NimBLE NUS 与 `cursor-bridge` 联调 |
| M3 | Cursor Hooks → 真实 `permission` 转发 |
| 可选 | `Claude-` 广播名、ES8389 等待音 |

## 免责声明

社区项目，与 Cursor、Anthropic、Espressif 无官方背书。协议意图与 Hardware Buddy 公开思路对齐，便于日后 Claude Desktop 类主机复用同一 JSON。
