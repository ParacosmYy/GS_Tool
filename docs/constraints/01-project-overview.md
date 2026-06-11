# 01 - 项目概况与构建环境

> 本文档是 EmbedDebug 约束体系的第1模块。每次开发前必须了解。

---

## 一、项目概况

| 项 | 值 |
|----|-----|
| 应用名称 | EmbedDebug |
| 项目路径 | `D:\Workplace\Embedded_workplace\User_workplace\GS_Tool` |
| 当前版本 | 0.1.0 |
| 评分起点 | 1分 / 目标 1000分 |
| Git分支 | `feat/embed-debug` |
| Git远程 | `https://github.com/ParacosmYy/GS_Tool.git` |

---

## 二、构建环境

| 项目 | 路径/版本 |
|------|-----------|
| Qt 6.8.3 | `E:/Tool/DevEnv/Qt/6.8.3/mingw_64` |
| GCC 14.2.0 (MinGW) | `E:/Tool/DevEnv/x86_64-14.2.0-release-win32-seh-msvcrt-rt_v12-rev2/mingw64/bin` |
| CMake ≥ 3.24 (当前 4.0.1) | `E:/Tool/DevEnv/cmake-4.0.1-windows-x86_64/cmake-4.0.1-windows-x86_64/bin` |
| Ninja 1.13.2 | PATH中 |
| GDB 16.2 | PATH中 |
| J-Link V932 | `E:/Embedded/Tool/SEGGER_IOT/JLink_V932` |

### Qt模块依赖

| 模块 | 用途 |
|------|------|
| Qt Core | 核心非GUI功能（QObject, 事件循环, 文件IO） |
| Qt Widgets | UI控件体系（QWidget, 布局, 对话框） |
| Qt Gui | 图形基础（QPainter, 图片, 字体） |
| Qt SerialPort | 串口通信（QSerialPort） |
| Qt Network | TCP/UDP/SSL网络通信 |
| Qt Svg | SVG图标渲染（Lucide图标集） |
| Qt Charts | 数据波形图表（QChart, QLineSeries） |

### 构建命令

```bash
cmake -G Ninja -B build -DCMAKE_PREFIX_PATH=E:/Tool/DevEnv/Qt/6.8.3/mingw_64
cmake --build build
E:/Tool/DevEnv/Qt/6.8.3/mingw_64/bin/windeployqt.exe build/EmbedDebug.exe
```

### 启动底线

`EmbedDebug.bat` 是用户侧最低运行入口，必须保持双击可启动。若 `build/EmbedDebug.exe` 不存在，`EmbedDebug.bat` 必须在当前机器可用环境下自动尝试配置/构建；失败时必须在弹窗输出具体缺失组件和修复命令。

- bat 只能从 `build/EmbedDebug.exe` 启动应用。
- 如果缺少 Qt 运行库，bat 应尝试执行 `windeployqt` 或给出明确错误信息。
- 任何修改 CMake、输出目录、可执行文件名、资源路径、Qt 部署路径、启动脚本的任务，收口前必须验证 `EmbedDebug.bat`。
- 不能验证时必须写明原因，不能只写“未测试”。

### 构建目录唯一性

项目只允许一个构建目录：`build/`。

- 禁止创建或引用 `build2/`、`build-debug/`、`build-release/`、`cmake-build-*` 等平行构建目录。
- 所有构建、部署、启动、测试命令都必须指向 `build/`。
- `EmbedDebug.bat` 不允许兼容第二构建目录；如果 `build/EmbedDebug.exe` 不存在，应直接报错。

### VS Code IntelliSense

`.vscode/c_cpp_properties.json` 配置:
- includePath: `${workspaceFolder}/src`, `E:/Tool/DevEnv/Qt/6.8.3/mingw_64/include/**`, GCC标准库头文件
- defines: `UNICODE`, `_UNICODE`, `QT_CORE_LIB`, `QT_GUI_LIB`, `QT_WIDGETS_LIB`, `QT_SERIALPORT_LIB`, `QT_CHARTS_LIB`, `QT_NETWORK_LIB`
- compilerPath: `E:/Tool/DevEnv/.../mingw64/bin/g++.exe`
- intelliSenseMode: `gcc-x64`, cppStandard: `c++17`

---

## 三、模块清单

> src/ 下有主模块，基础层已经落地到 `shared/` 与 `interfaces/`，当前进入兼容迁移与骨架化收口阶段。

### 现有模块（12个）

| 模块 | 路径 | 职责 | 所属层级 |
|------|------|------|---------|
| `automation` | `src/automation/` | 脚本自动化引擎、触发器 | 业务层 |
| `chart` | `src/chart/` | 数据波形显示（折线/FFT/散点/直方） | 表现层 |
| `connection` | `src/connection/` | 连接抽象与多种连接方式实现 | 基础设施层 |
| `core` | `src/core/` | 应用协调 + 基础 UI（MainWindow, PanelManager, ThemeManager, 各Controller） | 应用协调层 |
| `dashboard` | `src/dashboard/` | 仪表盘模式（Gauge/LED/数值） | 表现层 |
| `ota` | `src/ota/` | 固件OTA升级（XMODEM/YMODEM/ZMODEM） | 业务层 |
| `plugin` | `src/plugin/` | 插件系统（DLL动态加载） | 业务层 |
| `protocol` | `src/protocol/` | 协议解析引擎（帧解析/Modbus/JustFloat/FireWater） | 业务层 |
| `rtt` | `src/rtt/` | SEGGER RTT连接（J-Link SDK适配） | 基础设施层 |
| `serial` | `src/serial/` | 串口功能UI（配置面板/快捷指令/信号线监控） | 表现层 |
| `terminal` | `src/terminal/` | 终端显示（自绘制控件/搜索/选择/过滤） | 表现层 |
| `utils` | `src/utils/` | 公共工具（CRC/RingBuffer/DataLogger/SettingsManager） | 基础设施层 |

### 收敛中的基础层

| 模块 | 路径 | 职责 | 状态 |
|------|------|------|------|
| `interfaces` | `src/interfaces/` | 纯虚接口定义（IConnection, IPanelProvider, IDataSink, IProtocolParser, IDevice） | 已定义，持续扩展 |
| `shared` | `src/shared/` | 共享常量+枚举正式层（`core/theme/Constants.h` 仅作兼容转发） | ✅ 已落地，持续收口 |

### 现阶段骨架口径

| 入口 | 口径 |
|------|------|
| 稳定入口 | `AGENTS.md` 只保留索引和稳定规则，详细约束入口为 `CLAUDE.md` |
| 主架构文档 | `docs/constraints/03-architecture.md` |
| 目录骨架文档 | `docs/constraints/07-directory-structure.md` |
| 串口工站专项架构 | `docs/serial_station_architecture.md` |
| 冻结历史分叉 | `animation2/`、`widgets2/`、`loader2/`、`fonts/`、`icons/`、`responsive/` |
| 未来迁移落点 | `src/features/`、`src/shared/`、`src/interfaces/`、`src/core/` 的各自 canonical 子目录 |

---

## 四、Serial Station 重构目标

`serial_station` 是后续串口上位机重构的独立 app 模块，目标是把“串口框架层”和“业务协议层”切开，避免继续把 UI、串口收发、协议解析和文件服务耦合在一起。

### 目标定位

| 项 | 约束 |
|----|------|
| 模块路径 | `src/apps/serial_station/` |
| 专项文档 | `docs/serial_station_architecture.md` |
| 入口方式 | 作为 C++/Qt 新工站模块接入现有主窗口或启动入口，不直接污染现有 WiFi/RF/UWB 等工站 |
| 核心边界 | UI 只通过 `SerialStationController` 协调；`core/` 只管串口 bytes；`protocols/` 只管协议；`services/` 只管日志/导出/回放 |

### 优先落地顺序

1. 先建 `protocols/ISerialProtocol.h`、`protocols/SerialProtocolRegistry.h/.cpp`、`core/SerialCodec.h/.cpp`、`core/SerialDispatcher.h/.cpp`。
2. 再建 `core/SerialPort.h/.cpp`、`core/SerialManager.h/.cpp`、`workers/SerialReaderWorker.h/.cpp`。
3. 最后接 `ui/` 面板和 `SerialStationController.h/.cpp`。
4. 第一批协议只保留 `ascii_text`、`modbus_rtu`、`custom_md`，并配套 QTest 测试。
