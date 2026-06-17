# EmbedDebug

> 面向嵌入式研发、硬件联调、协议验证与产测排障的 Python/PyQt 桌面调试工作台。目标是做成比 VOFA+ 更完整、更可扩展、更适合工程交付的上位机。

![Python](https://img.shields.io/badge/Python-3.10%2B-3776AB?style=flat-square)
![PyQt6](https://img.shields.io/badge/UI-PyQt6-41CD52?style=flat-square)
![uv](https://img.shields.io/badge/Toolchain-uv-DE5FE9?style=flat-square)
![PyInstaller](https://img.shields.io/badge/Package-PyInstaller-00599C?style=flat-square)
![Windows](https://img.shields.io/badge/Platform-Windows%2010%2B-0078D4?style=flat-square)

EmbedDebug 将串口连接、协议解析、数据观察、日志回放、波形预览和 Windows 交付收敛到一条可测试、可打包、可追溯的 Python 主线。默认运行链路为 `EmbedDebug.bat -> uv run start-embeddebug`。

## 核心能力

| 能力 | 当前口径 | 证据入口 |
|---|---|---|
| PyQt 桌面入口 | 默认主线，可 smoke 启动 | `EmbedDebug.bat`、`python/embeddebug/app/` |
| Serial Station | UART 主流程可用，TCP client 已有 UI 入口；真实硬件仍需补证 | `python/embeddebug/serial_station/` |
| 协议解析 | RawData / FireWater / JustFloat 已纳入测试 | `tests/python/unit/test_protocols.py` |
| 日志与回放 | 支持结构化记录、导出、重放 | `services/`、`tests/python/unit/test_services.py` |
| 波形预览 | 基于 pyqtgraph 渐进接入 | `ui/waveform_preview.py` |
| Windows 打包 | PyInstaller onedir 交付 | `uv run package-embeddebug` |

## 架构

```text
用户入口
  EmbedDebug.bat / uv run start-embeddebug / packaged exe
      |
      v
PyQt 应用层
  app/main.py
  主窗口装配、主题、导航、生命周期
      |
      v
Serial Station 工作台
  ui/            用户意图与界面状态
  controllers/   流程编排、错误映射、状态同步
  core/          会话、收发、dispatcher、codec
  protocols/     帧定义、命令构建、流式解析
  services/      日志、导出、回放、Profile
  drivers/       串口、TCP 与替身设备适配
  workers/       后台任务，不直接更新 UI
      |
      v
工程交付层
  pytest / fixtures / uv scripts / PyInstaller
```

设计边界很明确：UI 不处理字节流，Controller 不写协议细节，Core 不依赖具体协议，Services 不反向驱动界面。

## 快速启动

```powershell
uv sync
uv run start-embeddebug
```

用户入口烟测：

```powershell
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```

## 测试与质量门禁

```powershell
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```

当前对外状态口径：

- 工程状态：`E4`，Python 测试与启动 smoke 可复现。
- 用户状态：`U3`，Serial Station 主流程已有入口与可见反馈。
- 设备状态：`D2`，TCP 已有本机 loopback 替身验证；真实硬件验证未完成，不能宣传为硬件全闭环。

## Windows 交付

```powershell
uv run package-embeddebug --version local --clean
uv run verify-package-embeddebug --package-dir dist\EmbedDebugPy-local-windows-x64
```

交付目录：

```text
dist/EmbedDebugPy-<version>-windows-x64/
```

最小交付内容：

```text
EmbedDebugPy.exe
_internal/PyQt6/Qt6/plugins/platforms/qwindows.dll
THIRD_PARTY_NOTICES.md
PYTHON_DEPENDENCIES.txt
```

`dist/`、`build/`、`.venv/`、PyInstaller workpath、缓存和二进制产物不得提交。

## 目录结构

```text
GS_Tool/
|-- python/embeddebug/
|   |-- app/                  # PyQt 应用入口
|   |-- serial_station/       # 串口工作台
|   |-- devtools/             # 测试、打包、验证命令
|   `-- shared/               # 共享值对象与基础能力
|-- tests/python/             # 单测、集成测试、UI smoke
|-- tests/fixtures/           # 协议样本、日志、回放样本
|-- tools/                    # uv script 适配与工程辅助
|-- docs/constraints/         # 工程约束
|-- EmbedDebug.bat            # 用户启动入口
|-- pyproject.toml            # uv scripts 与依赖声明
`-- uv.lock                   # 锁定依赖图
```

## 开发约束

- 新能力必须进入 `python/embeddebug/`，测试进入 `tests/python/`。
- 入口、依赖、打包统一通过 `uv run ...`。
- 不恢复遗留 native 主线，不新增 fallback。
- README 只描述已有证据支撑的能力；涉及真实硬件时必须标明验证状态。
- 提交前按 [CLAUDE.md](CLAUDE.md) 与 [docs/constraints](docs/constraints/) 执行约束加载、测试和收口。

## License

MIT License.
