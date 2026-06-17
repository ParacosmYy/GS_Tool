# EmbedDebug

> 面向嵌入式研发、硬件联调、产测排障和协议验证现场的 Python/PyQt 桌面调试工作台。
> 当前主线以 `uv` 管理依赖与命令入口，以 PyQt6 承载桌面交互，以 PyInstaller 输出 Windows 可交付包。

![Python](https://img.shields.io/badge/Python-3.10%2B-3776AB?style=flat-square)
![PyQt6](https://img.shields.io/badge/UI-PyQt6-41CD52?style=flat-square)
![uv](https://img.shields.io/badge/Toolchain-uv-DE5FE9?style=flat-square)
![PyInstaller](https://img.shields.io/badge/Package-PyInstaller-00599C?style=flat-square)
![Windows](https://img.shields.io/badge/Platform-Windows%2010%2B-0078D4?style=flat-square)

## 一、项目定位

EmbedDebug 的目标不是再做一个单点串口助手，而是把嵌入式现场常见的连接、收发、解析、观察、记录、回放和交付动作收敛到一个可验证的工程工作台。

当前技术路线已经切换为 Python/PyQt 主线：

- 入口统一：用户从 `EmbedDebug.bat` 启动，实际进入 `uv run start-embeddebug`。
- 依赖统一：Python、PyQt6、pyqtgraph、PyInstaller 等由 `uv` 管理。
- 打包统一：Windows 交付包通过 `uv run package-embeddebug` 生成。
- 验证统一：测试、启动 smoke、包结构校验全部走 `uv run ...`。

C++/CMake 打包链路、native 源码树和 C++ 测试入口已从当前仓库形态移除；后续新增能力只能进入 Python/PyQt 主线。

## 二、企业级架构总览

```text
┌──────────────────────── 用户入口层 ────────────────────────┐
│ EmbedDebug.bat / uv run start-embeddebug / packaged exe      │
└──────────────────────────────┬──────────────────────────────┘
                               │
                               ▼
┌──────────────────────── PyQt 应用层 ────────────────────────┐
│ app/main.py / 主窗口装配 / 页面导航 / 全局状态与主题          │
└──────────────────────────────┬──────────────────────────────┘
                               │
                               ▼
┌──────────────────── Serial Station 工作台层 ─────────────────┐
│ ui/ 意图收集  ->  controllers/ 编排与状态映射                 │
│ core/ 会话收发 -> protocols/ 帧构建与流式解析                 │
│ services/ 日志、导出、回放、档案、历史                        │
│ drivers/ 串口设备适配  workers/ 后台任务                     │
└──────────────────────────────┬──────────────────────────────┘
                               │
                               ▼
┌──────────────────────── 数据与交付层 ────────────────────────┐
│ Golden fixtures / 结构化日志 / CSV 导出 / Profile / PyInstaller │
└─────────────────────────────────────────────────────────────┘
```

核心原则：

- UI 只表达用户意图，不直接处理串口字节流。
- Controller 做状态编排和错误映射，不承载协议细节。
- Core 只管理会话、收发、dispatcher 和传输状态。
- Protocols 只负责帧语义、命令构建和流式解析。
- Services 负责日志、导出、回放、档案和历史数据。
- 打包、启动、测试命令全部收束到 `pyproject.toml` 的 uv scripts。

## 三、当前主线能力

| 能力域 | 当前状态 | 证据入口 |
|---|---|---|
| Python/PyQt 启动链路 | 已切为默认主线 | `EmbedDebug.bat`、`uv run start-embeddebug` |
| Serial Station MVP | 已具备工作台闭环 | `python/embeddebug/serial_station/` |
| 协议核心 | RawData、FireWater、JustFloat 等迁移中 | `protocols/` 与 fixtures |
| 串口连接 | 支持真实串口连接路径 | `drivers/`、`core/`、UI smoke |
| 日志与检索 | 支持过滤、搜索、统计 | UI smoke 与服务测试 |
| 配置档案 | 支持 Profile 读写与恢复 | `services/`、集成测试 |
| 波形预览 | 基于 pyqtgraph/NumPy 渐进迁移 | UI smoke |
| Windows 打包 | PyInstaller onedir | `uv run package-embeddebug` |

## 四、快速开始

首次准备环境：

```powershell
uv sync
```

启动应用：

```powershell
uv run start-embeddebug
```

用户入口 smoke：

```powershell
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```

## 五、测试与验证

日常验证：

```powershell
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```

打包验证：

```powershell
uv run package-embeddebug --version verify --clean
uv run verify-package-embeddebug --package-dir dist\EmbedDebugPy-verify-windows-x64
```

Doctor 检查：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\doctor.ps1 -RunTests -RunPackageDryRun -RunLaunch
```

## 六、交付方式

PyInstaller 交付包输出到：

```text
dist/EmbedDebugPy-<version>-windows-x64/
```

包结构至少包含：

- `EmbedDebugPy.exe`
- `_internal/PyQt6/Qt6/plugins/platforms/qwindows.dll`
- `THIRD_PARTY_NOTICES.md`
- `PYTHON_DEPENDENCIES.txt`

`dist/`、临时 workpath、缓存和生成产物不得提交。

## 七、仓库结构

```text
GS_Tool/
|-- python/embeddebug/        # Python/PyQt 产品运行时代码
|   |-- app/                  # 应用入口
|   |-- serial_station/       # 串口工作台主线
|   |-- devtools/             # 打包、验证、测试入口
|   `-- shared/               # 共享值对象与基础能力
|-- tests/python/             # Python 单测、集成测试、UI smoke
|-- tests/fixtures/           # 协议 golden、日志、回放样本
|-- tools/                    # 工程管理、启动、审计、doctor 辅助脚本
|-- docs/constraints/         # 强制约束与收口规则
|-- docs/prd/                 # PRD
|-- docs/superpowers/specs/   # 批次 Specs
|-- EmbedDebug.bat            # 用户启动入口
|-- pyproject.toml            # uv scripts 与依赖声明
|-- uv.lock                   # 锁定依赖图
`-- README.md
```

## 八、工程治理

任何变更必须遵守仓库约束链路：

1. 先读 `CLAUDE.md`。
2. 再读 `docs/constraints/01-project-overview.md`。
3. 涉及架构、目录、UI、串口工作台时加载对应约束文档。
4. 新行为必须有 PRD/Specs。
5. 提交前必须运行可复现测试或 smoke，并记录不能验证的原因。
6. 用户已有改动不得擅自回退。

当前收口口径：

- 默认启动：`EmbedDebug.bat -> uv run start-embeddebug`
- 默认测试：`uv run test-embeddebug-py`
- 默认工具验证：`uv run test-embeddebug-tools`
- 默认打包：`uv run package-embeddebug`
- 默认包验证：`uv run verify-package-embeddebug`

## 九、路线图

| 优先级 | 方向 | 目标 |
|---|---|---|
| P0 | Serial Station 完整工作流 | 连接、发送、接收、日志、导出、回放形成稳定闭环 |
| P0 | VOFA+ 能力追平 | RawData、FireWater、JustFloat、多通道波形与测量分析 |
| P0 | 真实设备验证 | 从 smoke 和 fixtures 推进到虚拟串口/真实硬件证据 |
| P1 | UI 与交互一致性 | PyQt 控件层级、快捷键、状态反馈、错误恢复统一 |
| P1 | 交付硬化 | 包结构、依赖 notices、启动诊断和升级路径稳定 |

## License

MIT License.
