# Korvo 爪爪（korvo-buddy）

**English:** A community desk-pet firmware for **ESP32-S31-Korvo-1** that mirrors [Anthropic Hardware Buddy](https://www.anthropic.com/news/hardware-buddy)-style **BLE NDJSON** to show **Cursor Agent** status (running / waiting for approval / recent logs / daily tokens) and approve or deny tool prompts with **PLAY** / **SET**. Not an official Cursor or Anthropic product.

桌宠固件 + PC 桥接骨架：在 Korvo 上显示 Cursor Agent 状态，并用按键批准/拒绝工具调用。

## 硬件

| 项 | 说明 |
|---|---|
| 主板 | [ESP32-S31-Korvo-1 V1.1](https://docs.espressif.com/projects/esp-dev-kits/zh_CN/latest/esp32s31/esp32-s31-korvo-1/user_guide.html)（ESP32-S31-WROOM-3，16MB Flash + 16MB PSRAM） |
| LCD | ESP32-S3-LCD-EV-Board-SUB3，4.3" 800×480 RGB565，触摸 GT1151 |
| 按键 | 丝印 PLAY = BSP `BSP_BUTTON_MODE` 批准；SET 拒绝；VOL± 改界面亮度。BSP 的背光 API 在这块板上返回不支持 |
| LED | WS2812，`BSP_LED_RGB_IO` = GPIO37 |
| 音频 | ES8389 等待音未接，只记进入 WAITING 的边沿 |

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
3. **设备**：刷入 `firmware/`。LVGL 显示状态、日志和今日 token（估算时带「约」）。WAITING 时 PLAY 批准、SET 拒绝。BLE 仍是桩，心跳可从串口/测试注入。

```mermaid
flowchart LR
  Cursor --> Bridge[cursor-bridge]
  Bridge -->|BLE NUS| FW[firmware]
  FW -->|permission| Bridge
```

## 固件编译（ESP-IDF 6.1，本机已用官方 display 例程点亮 SUB3）

串口：CP2102N，`/dev/ttyUSB0`。`esp32s31` 在 IDF 6.1 里要 `--preview`。

```bash
. ~/idf/esp-idf6.1/export.sh
cd firmware
idf.py --preview set-target esp32s31
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

`build` 或 `flash` 若仍提示目标处于预览，给那两条也加上 `--preview`。BSP 依赖已写在 `firmware/main/idf_component.yml`。细节和丝印对照见 [firmware/README.md](firmware/README.md)。

没有板子、也没有 IDF 时：

```bash
make -C firmware/host_test test
cd host/cursor-bridge && PYTHONPATH=. python3 -m unittest tests.test_bridge
```

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
| M0 | 协议、状态机、NDJSON、Python 桥 |
| M1（本仓库） | BSP 屏 + LVGL 宠物界面 + ADC 按键 + WS2812。BLE 仍是桩，提示音未接喇叭 |
| M2 | NimBLE NUS 与 `cursor-bridge` 联调 |
| M3 | Cursor Hooks → 真实 `permission` 转发 |
| 可选 | `Claude-` 广播名、ES8389 等待音 |

## 免责声明

社区项目，与 Cursor、Anthropic、Espressif 无官方背书。协议意图与 Hardware Buddy 公开思路对齐，便于日后 Claude Desktop 类主机复用同一 JSON。
