# 01 - 项目概况与构建环境

> 本文档是 EmbedDebug 约束体系的第1模块。每次开发前必须了解。

---

## 一、项目概况

| 项 | 值 |
|----|-----|
| 应用名称 | EmbedDebug |
| 项目路径 | `E:\Embedded\Tool\Serial_tool\User_Serial` |
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

### VS Code IntelliSense

`.vscode/c_cpp_properties.json` 配置:
- includePath: `${workspaceFolder}/src`, `E:/Tool/DevEnv/Qt/6.8.3/mingw_64/include/**`, GCC标准库头文件
- defines: `UNICODE`, `_UNICODE`, `QT_CORE_LIB`, `QT_GUI_LIB`, `QT_WIDGETS_LIB`, `QT_SERIALPORT_LIB`, `QT_CHARTS_LIB`, `QT_NETWORK_LIB`
- compilerPath: `E:/Tool/DevEnv/.../mingw64/bin/g++.exe`
- intelliSenseMode: `gcc-x64`, cppStandard: `c++17`

---

## 三、模块清单

> src/ 下共 12 个顶层模块，外加 2 个规划中的基础设施模块。

### 现有模块（12个）

| 模块 | 路径 | 职责 | 所属层级 |
|------|------|------|---------|
| `automation` | `src/automation/` | 脚本自动化引擎、触发器 | 业务层 |
| `chart` | `src/chart/` | 数据波形显示（折线/FFT/散点/直方） | 表现层 |
| `connection` | `src/connection/` | 连接抽象与13种连接方式实现 | 基础设施层 |
| `core` | `src/core/` | 核心编排（MainWindow, PanelManager, ThemeManager, 各Controller） | 表现层+业务层 |
| `dashboard` | `src/dashboard/` | 仪表盘模式（Gauge/LED/数值） | 表现层 |
| `ota` | `src/ota/` | 固件OTA升级（XMODEM/YMODEM/ZMODEM） | 业务层 |
| `plugin` | `src/plugin/` | 插件系统（DLL动态加载） | 业务层 |
| `protocol` | `src/protocol/` | 协议解析引擎（帧解析/Modbus/JustFloat/FireWater） | 业务层 |
| `rtt` | `src/rtt/` | SEGGER RTT连接（J-Link SDK适配） | 基础设施层 |
| `serial` | `src/serial/` | 串口功能UI（配置面板/快捷指令/信号线监控） | 表现层 |
| `terminal` | `src/terminal/` | 终端显示（自绘制控件/搜索/选择/过滤） | 表现层 |
| `utils` | `src/utils/` | 公共工具（CRC/RingBuffer/DataLogger/SettingsManager） | 基础设施层 |

### 规划中模块（2个）

| 模块 | 路径 | 职责 | 状态 |
|------|------|------|------|
| `interfaces` | `src/interfaces/` | 纯虚接口定义（IConnection, IPanelProvider, IDataSink, IProtocolParser, IDevice） | 🔄 规划中 |
| `shared` | `src/shared/` | 共享常量+枚举（ColorConstants, LayoutConstants等6个域头文件） | 🔄 规划中 |
