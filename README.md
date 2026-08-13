# SerialForge

> Windows-first 的嵌入式调试控制台（embedded debug console）。基于 PySide6 与 pyserial，提供串口通信、协议解析（Modbus RTU / MAVLink）、OTA 升级（AES）、J-Link RTT 与可选 BLE 能力，采用与 QuillForge 一致的分层架构。

![Platform](https://img.shields.io/badge/platform-Windows-0078D4)
![Language](https://img.shields.io/badge/language-Python%203.12-3776AB)
![UI](https://img.shields.io/badge/ui-PySide6%20%2F%20pyserial-41CD52)
![CI](https://img.shields.io/badge/CI-GitHub%20Actions-2088FF)
![License](https://img.shields.io/badge/license-Proprietary-yellow)

---

## 1. 项目简介

SerialForge 是面向嵌入式开发者的一体化调试控制台，将常用的串口交互、协议观察、固件 OTA 与片上调试收敛到统一界面。核心能力：

- **串口控制台**：基于 `pyserial` 的发送/接收、hex/ASCII 视图、可配置 UART 参数与流控。
- **协议解析**：内置 Modbus RTU、MAVLink 等协议预设（`profiles/`），支持 Dataset 预览与组件表。
- **OTA 升级**：经 AES 加密的固件推送（contract-only 边界，伪造产物需经批准）。
- **J-Link RTT**：通过契约接入的实时终端（contract-only / attach-only 边界）。
- **BLE（可选）**：`bleak` 提供的低功耗蓝牙传输（可选依赖）。
- **高刷新动效**：全窗口唯一 `MotionController` 调度器，统一 120Hz 动画时间域。

> 原 82KB 开发者手册已归档至 [`docs/README.developer.md`](docs/README.developer.md)，包含设计原则、协议细节与完整切片路线。

## 2. 目录结构

```text
SerialForge/
├─ src/serialforge/       # 应用源码
│  ├─ application/        # 用例（use cases）
│  ├─ domain/             # 领域模型与不变式
│  ├─ infrastructure/     # 适配器（串口传输、文件服务等）
│  ├─ presentation/       # PySide6 表现层（响应式 owner / MotionController / 主题）
│  ├─ debug/              # 调试会话
│  ├─ ota/                # OTA 升级（AES）
│  └─ __main__.py         # 入口（main）
├─ docs/                  # ARCHITECTURE / DEPENDENCIES / PROTOCOLS / ADR / 性能
├─ packaging/             # PyInstaller 配置与 NOTICE
├─ profiles/              # 协议预设 JSON（mavlink-common-heartbeat / modbus-rtu-rx-adu）
├─ scripts/               # check.ps1 / package.ps1 / run.ps1 及源码限额检查
├─ .github/workflows/     # CI（GitHub Actions）
├─ pyproject.toml         # 依赖与工具配置（ruff / hatchling / uv）
├─ uv.lock                # 锁定依赖
├─ .python-version        # Python 3.12
└─ README.md              # 本文件（企业级总览）
```

## 3. 架构说明

与 QuillForge 一致的分层契约，依赖方向指向稳定内层：

```text
Presentation ──> Application ──> Domain
      │               │
      └──────> Ports <┘
                    ▲
             Infrastructure

Plugins / Transports ──> Public API ──> Application commands and events
```

关键约束：

- **表现层几何 owner**：窄窗口/响应式修复收敛为「单一几何 owner」，只接收已创建控件并按 Qt sizing contract 重排，不创建业务控件、不持有 ViewModel/DTO/signals/timer/`MotionController`。
- **边界纪律**：OTA/AES、J-Link RTT、BLE 为 **contract-only / attach-only** 边界——在未批准时不得伪造产物或执行原生烧录。
- **单一调度器**：全窗口唯一 `MotionController` 负责动画调度；新增能力不得引入第二个 timer、线程或 `MotionController`。
- **质量门禁**：`scripts/check.ps1` 与源码限额脚本（`check_source_limits.py` / `check_theme_tokens.py`）强制导入方向与主题 token 约束。

## 4. 环境依赖

| 类别 | 依赖 | 版本约束 |
|---|---|---|
| 运行时 | Python | `>=3.12,<3.13`（见 `.python-version`） |
| UI 运行时 | PySide6 | `>=6.9,<7` |
| 串口 | pyserial | `>=3.5,<4` |
| 可选 BLE | bleak | `>=0.22,<4`（可选 `ble` 依赖组） |
| 开发工具 | pyinstaller / ruff | `pyinstaller>=6,<7` · `ruff>=0.14,<1` |
| 依赖与构建 | uv / hatchling | `uv` 管理环境与锁文件；`hatchling` 构建 |
| 操作系统 | Windows | Windows-first（原生窗口/渲染/洁净机器为验证边界） |

> 依赖统一置于 `pyproject.toml` 与 `uv.lock`，禁止安装到全局 Python 环境。

## 5. 部署方式（构建与打包）

### 5.1 开发运行

```powershell
uv sync            # 创建虚拟环境并安装依赖
uv run serialforge # 启动调试控制台
```

### 5.2 质量门禁与源码限额

```powershell
.\scripts\check.ps1                 # ruff + 导入边界 + 闸门
python scripts/check_source_limits.py   # 单文件规模限额
python scripts/check_theme_tokens.py    # 主题 token 约束
```

### 5.3 打包为 EXE 与 CI

```powershell
.\scripts\package.ps1 # 经 PyInstaller 产出 SerialForge.exe
```

CI 由 `.github/workflows/` 提供，在推送/PR 时执行 lint 与构建校验。打包产物 `SerialForge.exe` / `SerialForge-latest.exe` 已被 `.gitignore` 排除。

## 6. 使用说明

### 6.1 常规启动

```powershell
uv run serialforge
# 或直接使用打包产物
.\SerialForge.exe
```

### 6.2 连接与协议

- 在连接页配置传输（串口/UART 参数、流控、超时）或从预设加载；
- 在协议页加载 `profiles/` 中的协议预设（如 `modbus-rtu-rx-adu.json`、`mavlink-common-heartbeat.json`）进行解析与 Dataset 预览；
- 命令页支持历史、批量操作与上下文带。

### 6.3 高级能力

- **OTA 升级**：经 AES 加密通道推送固件（需遵守 contract-only 边界）。
- **J-Link RTT**：通过契约接入的实时终端（attach-only，不伪造原生行为）。
- **BLE**：在已安装 `bleak` 的环境启用低功耗蓝牙传输。

## 7. 贡献规范

- **提交规范**：遵循 Conventional Commits；提交须标注受影响模块、导入方向与契约边界。
- **代码风格**：ruff 规则集 `E, F, I, UP, B`；`line-length = 100`；`quote-style = double`。
- **依赖方向**：表现层/基础设施层不得反向依赖领域层；稳定内层导入边界由门禁脚本强制。
- **表现层修复边界**：响应式/几何修复必须是「单一几何 owner」，不得穿透 builder/controller、不得持有业务状态或 `MotionController`。
- **安全与边界**：OTA/AES/RTT/J-Link/BLE 为 contract-only / attach-only；不宣称签名、洁净机器或发布验收完成（须说明实际验证范围）。
- **规模限额**：单文件规模受源码限额脚本约束，超限须先拆分再继续。

---

> 详细文档入口：
> - 架构与切片：[`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md)
> - 依赖与协议：[`docs/DEPENDENCIES.md`](docs/DEPENDENCIES.md) · [`docs/PROTOCOLS.md`](docs/PROTOCOLS.md)
> - 开发者手册（原完整 README）：[`docs/README.developer.md`](docs/README.developer.md)
> - 工作流与路线：[`docs/WORKFLOW.md`](docs/WORKFLOW.md) · [`docs/ROADMAP.md`](docs/ROADMAP.md)
