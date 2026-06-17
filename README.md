# EmbedDebug

> 面向嵌入式研发、协议调试、硬件联调与产测排障的 Python/PyQt 桌面上位机。项目目标是形成比 VOFA+ 更完整、更工程化、更适合长期交付的串口与多源数据调试平台。

![Python](https://img.shields.io/badge/Python-3.10%2B-3776AB?style=flat-square)
![PyQt6](https://img.shields.io/badge/UI-PyQt6-41CD52?style=flat-square)
![uv](https://img.shields.io/badge/Toolchain-uv-DE5FE9?style=flat-square)
![PyInstaller](https://img.shields.io/badge/Package-PyInstaller-00599C?style=flat-square)
![Windows](https://img.shields.io/badge/Platform-Windows%2010%2B-0078D4?style=flat-square)

## 当前状态

| 维度 | 状态 | 证据 |
|---|---|---|
| 工程状态 | `E4`，Python 测试与启动 smoke 可复现 | `uv run test-embeddebug-py` |
| 用户状态 | `U3`，Serial Station 主流程已有可见入口 | `uv run start-embeddebug` |
| 设备状态 | `D2`，TCP/UDP 已有替身或 loopback 验证，真实硬件仍需补证 | `tests/python/ui_smoke/test_serial_station_udp_ui.py` |
| 评分进度 | `609 / 1000` | `docs/tracking/SCORE_TRACKING.md` |

## 产品能力

| 能力 | 当前口径 | 入口 |
|---|---|---|
| PyQt 桌面工作台 | 默认主线，支持 smoke 启动 | `EmbedDebug.bat` |
| Serial Station | UART/TCP/UDP 主流程已有可见入口；Profile 加载 System 日志、协议切换 System 日志、连接生命周期 System 日志、连接控件状态、命令输入写入、Profile 名称输入、Profile 下拉选项、端点 Profile 回填、端点默认文本、端点控件文案、串口配置选项、串口 Profile 参数回填、串口连接字段读取、日志视图内容、日志筛选选项、System/Error 诊断日志筛选、System/Error 日志显示与统计、System/Error 导出回放方向保持、controller 错误日志入库、日志过滤规则、串口端口空态、命令历史选项、TCP/UDP 端点校验、普通状态文本、结果状态反馈、Profile 标签反馈、日志行与日志统计反馈、RX 注入反馈已公共化，UDP UI loopback 收发已验证，断开、刷新端口、发送、快捷键、Profile 恢复、错误状态反馈和窗口生命周期可诊断 | `tests/python/ui_smoke/test_serial_station_udp_ui.py` |
| 协议解析 | RawData / FireWater / JustFloat 已纳入测试，协议选择动作已独立 | `python/embeddebug/serial_station/ui/protocol_actions.py` |
| 日志与回放 | 支持结构化记录、过滤、搜索、统计、清空、导出、重放和失败可恢复反馈 | `python/embeddebug/serial_station/ui/log_actions.py` |
| 波形预览 | 基于 pyqtgraph 接入轻量趋势视图，测量展示动作已独立 | `python/embeddebug/serial_station/ui/measurement_actions.py` |
| Windows 交付 | PyInstaller onedir 打包 | `uv run package-embeddebug` |

## 架构原则

```text
用户入口
  EmbedDebug.bat -> uv run start-embeddebug

应用层
  app/                 QApplication、启动参数、smoke

Serial Station
  ui/                  只收集用户意图与展示状态
  controllers/         编排流程、错误映射、状态同步
  core/                会话、字节收发、dispatcher、codec
  protocols/           帧定义、命令构建、流式解析
  services/            日志、导出、回放、Profile
  drivers/             UART、TCP、UDP、替身 transport
  workers/             后台任务，不直接更新 UI

工程交付
  pytest / pytest-qt / uv scripts / PyInstaller
```

核心边界：UI 不解析字节流，controller 不写协议细节，core 不依赖具体协议，services 不反向驱动界面，所有入口收敛到 Python/PyQt 主线。

当前 controller 已将测量缓冲、命令历史、日志状态、连接生命周期和发送/注入 I/O 状态拆成小 helper，UI 主布局已将连接工具栏拆出独立 helper，Controller 与 Serial Station UI smoke 回归测试已按行为域拆分，保持主编排与 Python 测试文件低于 300 行，后续功能增量必须继续维持这个边界。

## 快速启动

```powershell
uv sync
uv run start-embeddebug
```

用户入口 smoke：

```powershell
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```

## 质量门禁

```powershell
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```

脚本面约束：仓库只保留 `EmbedDebug.bat` 作为用户侧 Windows 启动入口；测试、打包、验证和工程辅助统一通过 `uv run ...` 暴露。

## 打包交付

```powershell
uv run package-embeddebug --version local --clean
uv run verify-package-embeddebug --package-dir dist\EmbedDebugPy-local-windows-x64
```

交付目录固定为：

```text
dist/EmbedDebugPy-<version>-windows-x64/
```

`dist/`、`build/`、`.venv/`、PyInstaller workpath、缓存与二进制产物不得提交。

## 仓库结构

```text
GS_Tool/
|-- python/embeddebug/
|   |-- app/                  # PyQt 应用入口
|   |-- serial_station/       # 上位机核心工作台
|   |-- devtools/             # 测试、打包、验证命令
|   `-- shared/               # 共享值对象与基础能力
|-- tests/python/             # 单测、集成测试、UI smoke
|-- tests/fixtures/           # 协议样本、日志、回放样本
|-- docs/constraints/         # 工程约束
|-- docs/prd/                 # 活跃 PRD
|-- EmbedDebug.bat            # 唯一用户启动脚本
|-- pyproject.toml            # uv scripts 与依赖声明
`-- uv.lock                   # 锁定依赖图
```

## 开发纪律

- 新产品代码只进入 `python/embeddebug/`，测试进入 `tests/python/`。
- 新能力必须有 PRD、测试证据和三轴状态。
- 单个 Python 运行时文件不得超过 300 行。
- README 只描述已有证据支撑的能力；涉及真实硬件时必须标明验证状态。
- 提交前按 [CLAUDE.md](CLAUDE.md) 与 [docs/constraints](docs/constraints/) 执行约束加载、测试和收口。

## License

MIT License.
