# Korvo 爪爪 / Hardware Buddy 风格线协议

社区桥接协议，灵感来自 [Anthropic Hardware Buddy](https://www.anthropic.com/news/hardware-buddy) 的 **Nordic UART Service (NUS) + 换行分隔 JSON (NDJSON)** 思路。**非** Cursor 或 Anthropic 官方支持；Claude Desktop 配对设备在 UUID 与 JSON 形状上可对齐，主机桥接实现不同。

## 传输层

| 项 | 值 |
|---|---|
| 首选 | BLE GATT，NUS |
| 回退 | USB 串口（CDC），同 NDJSON，UTF-8，`\n` 分行 |
| 方向 | 主机 → 设备：心跳/状态；设备 → 主机：权限响应、可选按键事件 |
| 编码 | 单行 JSON，无 BOM；单条建议 ≤ 4 KiB |

### NUS UUID（128-bit，与常见 NUS 一致）

| 角色 | UUID |
|---|---|
| Service | `6E400001-B5A3-F393-E0A9-E50E24DCCA9E` |
| RX（Central 写 → 设备收） | `6E400002-B5A3-F393-E0A9-E50E24DCCA9E` |
| TX（设备通知 → Central 读） | `6E400003-B5A3-F393-E0A9-E50E24DCCA9E` |

### BLE 广播

- 默认本地名：`Cursor-S31-xxxx`（`xxxx` = MAC 后两字节十六进制，小写）
- 可选后续模式：`Claude-S31-xxxx`（同一固件，仅改名与主机配对逻辑）

串口回退：建议 `115200 8N1`，设备发 `{"type":"ready","proto":1}\n` 后进入 NDJSON 流。

---

## 消息类型概览

所有消息含 `"type"` 字符串。未知 `type` 应忽略并打日志，不断开链路。

| type | 方向 | 说明 |
|---|---|---|
| `heartbeat` | 主机 → 设备 | 周期快照（建议 1–2 Hz；`waiting>0` 时可 4 Hz） |
| `permission` | 设备 → 主机 | 用户对工具提示的批准/拒绝 |
| `status` | 主机 → 设备 | 对 `permission` 的确认（可选明细） |
| `ping` / `pong` | 双向 | 保活（可选） |

---

## `heartbeat`（主机 → 设备）

将 Cursor Agent 会话状态映射为单帧 UI 快照。

```json
{
  "type": "heartbeat",
  "ts": 1730000000,
  "total": 3,
  "running": 1,
  "waiting": 1,
  "msg": "等待批准：run_terminal_cmd",
  "entries": [
    {"t": "11:02:01", "line": "Running grep in firmware/"},
    {"t": "11:02:05", "line": "Tool: Shell — npm test"}
  ],
  "tokens": 128450,
  "tokens_today": 42000,
  "tokens_estimated": true,
  "prompt": {
    "id": "prm_8f3a2c",
    "tool": "run_terminal_cmd",
    "hint": "npm test -- --run"
  }
}
```

### 字段

| 字段 | 类型 | 必填 | 说明 |
|---|---|---|---|
| `ts` | number | 建议 | Unix 秒，设备用于时钟漂移检测 |
| `total` | int | 是 | 可见 Agent/任务槽总数（MVP 常为 1） |
| `running` | int | 是 | 正在执行（非等待用户）的数量 |
| `waiting` | int | 是 | 等待用户批准工具的数量 |
| `msg` | string | 否 | 单行状态摘要 |
| `entries` | array | 否 | 最近日志行，`t` 本地时间字符串，`line` 正文（各 ≤ 120 字符建议） |
| `tokens` | int | 否 | 累计 token（会话或账户级，由桥接定义） |
| `tokens_today` | int | 否 | 本地自然日累计（主机按午夜重置） |
| `tokens_estimated` | bool | 否 | `true` 时 UI 显示「约」 |
| `prompt` | object | 条件 | 当 `waiting > 0` 时应出现 |
| `prompt.id` | string | 是* | 与主机侧待批请求一致 |
| `prompt.tool` | string | 是* | 工具名，如 `run_terminal_cmd` |
| `prompt.hint` | string | 否 | 短描述，供 800×480 截断显示 |

设备状态机建议：

- `waiting > 0` → `BUDDY_STATE_WAITING`，全屏等待 overlay，`PLAY`/`SET` 有效
- `running > 0` 且 `waiting == 0` → `RUNNING`
- 否则若近期有心跳 → `IDLE`；超时无心跳 → `SLEEP`
- 解析错误 / 链路断开 → `ERROR`（可自动恢复）

---

## `permission`（设备 → 主机）

```json
{
  "type": "permission",
  "id": "prm_8f3a2c",
  "decision": "approve"
}
```

| 字段 | 说明 |
|---|---|
| `id` | 必须匹配最近 `heartbeat.prompt.id` |
| `decision` | `approve` \| `deny` |

映射（ESP32-S31-Korvo-1 V1.1）：

- **PLAY** → `approve`
- **SET** → `deny`

公开 BSP 枚举是 `BSP_BUTTON_SET`、`BSP_BUTTON_MODE`、`BSP_BUTTON_VOLP`、`BSP_BUTTON_VOLM`（ADC1 CH0，窗口见 `firmware/main/board_map.h`）。丝印为 PLAY / SET / VOL+ / VOL-。固件暂把 `BSP_BUTTON_MODE` 当作 PLAY。头文件没有 PLAY 这个标识，上板后核对。

触摸 UI 可发送同一条 JSON。

---

## `status`（主机 → 设备）

```json
{
  "type": "status",
  "id": "prm_8f3a2c",
  "ok": true,
  "detail": "approved"
}
```

| `detail` 示例 | 含义 |
|---|---|
| `approved` / `denied` | 已转发给 Cursor |
| `stale` | `id` 已过期 |
| `error` | 桥接失败 |

设备收到 `ok: true` 后清除等待 overlay；`stale` 可短提示后回到 `IDLE`。

---

## Cursor vs Claude Desktop 配对差异

| 维度 | Cursor（本仓库 `host/cursor-bridge`） | Claude Desktop + 官方/社区 Buddy |
|---|---|---|
| 事件来源 | Cursor Hooks、日志 tail、API/文件监视（MVP 多为模拟） | Claude 应用内工具批准通道 |
| 广播名 | `Cursor-S31-xxxx` | 常为 `Claude-…` 或厂商固件默认 |
| `permission` 落地 | 需接到 Cursor 批准 UI/API（MVP 可只打日志） | Desktop 已理解同类 JSON |
| Token 字段 | 常缺失，桥接估算 + `tokens_estimated: true` | 若应用提供则可为实测 |
| 同一固件 | 可共用 NUS + JSON；仅换主机桥接与 BLE 过滤名 |

---

## 版本与兼容

- `proto` 版本：固件 `ready` 与主机配置中记为 `1`
- 新增字段：向后兼容，旧固件忽略
-  breaking 变更：递增 `proto`，文档记录

---

## 参考

- [ESP32-S31-Korvo-1 用户指南](https://docs.espressif.com/projects/esp-dev-kits/zh_CN/latest/esp32s31/esp32-s31-korvo-1/user_guide.html)
- BSP：`espressif/esp32_s31_korvo_1`（Component Registry）
- Nordic UART Service 惯例 UUID（见上表）
