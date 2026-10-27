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
# 模拟心跳，打印 3 帧后退出
cursor-bridge --demo --once 3

# 从 JSONL 读事件（Cursor hook 可往这个文件追加）
cursor-bridge --events examples\events.jsonl --once 3

# BLE：扫描名称前缀 Cursor-S31，写 NUS RX，订 NUS TX
cursor-bridge --ble --name-prefix Cursor-S31

# 串口回退，115200 8N1
cursor-bridge --serial COM7
```

没有 `--events` 且没有 `--ble` / `--serial` 时默认走 `--demo`，NDJSON 打到 stdout。

## 参数

| 参数 | 说明 |
|---|---|
| `--demo` | 循环 idle / running / waiting |
| `--events FILE` | JSONL，一行一个事件 |
| `--ble` | bleak 连接 NUS |
| `--name-prefix` | 默认 `Cursor-S31` |
| `--serial PORT` | 例如 `COM7` |
| `--once N` | 发 N 帧心跳后退出 |
| `--interval SEC` | 心跳间隔，默认 1 秒 |

## Token「约」

没有 Cursor token 接口时，按日志字符数粗算（约 4 字符 1 token），`tokens_estimated: true`，固件在今日数字旁显示「约」。计数按本机自然日，过午夜只清 `tokens_today`。事件里若同时带 `tokens` 和 `tokens_today`，改记实测并关掉「约」。

## 自测

```powershell
cd host\cursor-bridge
$env:PYTHONPATH = "."
python -m unittest tests.test_bridge
```

## 下一步

Cursor Hooks 把真实工具批准写成 JSONL（字段见 `examples/events.jsonl`）。`PermissionGate` 已回 `status`；把 `approved` / `denied` 送进 Cursor 批准通道仍要你自己接。BLE 侧在固件启用 NimBLE NUS 后即可联调，主机写入逻辑已经在 `BleakTransport` 里。
