# cursor-bridge（Windows 为主）

将 Cursor Agent 会话状态映射为 [REFERENCE.md](../../REFERENCE.md) 中的 `heartbeat` NDJSON，经 BLE（NUS）或串口发给 Korvo 爪爪；接收 `permission` 并回 `status`。

**非官方** Cursor 集成；MVP 含模拟事件与桩 BLE 写入。

## 环境

- Windows 10/11（蓝牙需支持 BLE Central；笔记本内置蓝牙通常可用）
- Python 3.10+
- 可选：USB 数据线连接 Korvo 串口回退

## 安装（Windows PowerShell）

```powershell
cd host\cursor-bridge
python -m venv .venv
.\.venv\Scripts\Activate.ps1
pip install -e .
```

若 `bleak` 安装失败，先安装 [Microsoft C++ Build Tools](https://visualstudio.microsoft.com/visual-cpp-build-tools/) 或使用 `pip install bleak --only-binary :all:`。

## 运行

```powershell
# 模拟心跳（不接设备）
cursor-bridge --demo

# BLE：扫描名称前缀 Cursor-S31 并连接（桩：打印 NDJSON）
cursor-bridge --ble --name-prefix Cursor-S31

# 串口回退
cursor-bridge --serial COM7
```

## 配置

| 变量 / 参数 | 说明 |
|---|---|
| `--demo` | 循环演示 running / waiting |
| `--ble` | 使用 bleak 连接 NUS |
| `--serial PORT` | 115200 8N1 |
| `CURSOR_BRIDGE_LOG` | 可选日志文件路径（tail 作为 entries 来源，TODO） |

## Token「约」

无公开 Cursor token API 时，桥接按日志行数/heuristic 累加 `tokens_today`，并设 `tokens_estimated: true`。本地午夜（系统时区）重置计数。

## 下一步

- 对接 Cursor Hooks / 扩展，把真实 `prompt.id` 与批准 API 接到 `permission` 处理
- 实机联调：固件启用 NimBLE NUS 后去掉 `transport` 中的 print-only 桩
