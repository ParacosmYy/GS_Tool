# EmbedDebug - 项目开发约束文档

## 构建环境

| 项目 | 路径/版本 |
|------|-----------|
| Qt 6.8.3 | `E:/Tool/DevEnv/Qt/6.8.3/mingw_64` |
| GCC 14.2.0 | `E:/Tool/DevEnv/x86_64-14.2.0-release-win32-seh-msvcrt-rt_v12-rev2/mingw64/bin` |
| CMake 4.0.1 | `E:/Tool/DevEnv/cmake-4.0.1-windows-x86_64/cmake-4.0.1-windows-x86_64/bin` |
| Ninja 1.13.2 | PATH中 |
| GDB 16.2 | PATH中 |
| J-Link V932 | `E:/Embedded/Tool/SEGGER_IOT/JLink_V932` |

## 构建命令

```bash
# 配置 (只需执行一次)
cmake -G Ninja -B build -DCMAKE_PREFIX_PATH=E:/Tool/DevEnv/Qt/6.8.3/mingw_64

# 编译
cmake --build build

# 部署Qt DLL
E:/Tool/DevEnv/Qt/6.8.3/mingw_64/bin/windeployqt.exe build/EmbedDebug.exe

# 运行
./build/EmbedDebug.exe 或双击 EmbedDebug.bat
```

## VS Code IntelliSense 配置

`.vscode/c_cpp_properties.json` 中的 includePath 需要包含:
- `${workspaceFolder}/src` — 项目源码根目录
- `E:/Tool/DevEnv/Qt/6.8.3/mingw_64/include/**` — Qt头文件
- `E:/Tool/DevEnv/x86_64-14.2.0-release-win32-seh-msvcrt-rt_v12-rev2/mingw64/include/c++/14.2.0` — GCC标准库头文件

defines 需要包含: `UNICODE`, `_UNICODE`, `QT_CORE_LIB`, `QT_GUI_LIB`, `QT_WIDGETS_LIB`, `QT_SERIALPORT_LIB`, `QT_CHARTS_LIB`, `QT_NETWORK_LIB`

compilerPath: `E:/Tool/DevEnv/x86_64-14.2.0-release-win32-seh-msvcrt-rt_v12-rev2/mingw64/bin/g++.exe`

intelliSenseMode: `gcc-x64`, cppStandard: `c++17`

## 编码规范

- C++17 标准，简单直接，避免高级模板特性
- 详细注释（开发者是C++/Qt新手）
- 头文件引用使用相对src目录的路径，如 `#include "core/Constants.h"`
- Qt信号/槽用新式 connect 语法（函数指针），不用 SIGNAL/SLOT 宏
- 命名：类名 PascalCase，方法 camelCase，成员变量 m_ 前缀，常量 k 前缀
- 每个类一对 .h/.cpp 文件，放在对应的子目录中

## 项目结构

```
src/
├── core/        # MainWindow, ConnectionManager, ThemeManager
├── connection/  # IConnection接口, SerialConnection, (Tcp/Udp待实现)
├── terminal/    # TerminalWidget(QPainter), TerminalModel
├── serial/      # SerialConfigPanel, QuickCommandBar, TimedSender
├── protocol/    # FrameParser, FrameVisualEditor (Phase 3)
├── chart/       # ChartWidget (Phase 3)
├── ota/         # X/Y/ZMODEM (Phase 5)
├── rtt/         # J-Link RTT (Phase 6)
└── utils/       # CRC, HexConverter, RingBuffer, SettingsManager
```

## Git 规范

- 分支: `feat/embed-debug`
- 远程: `https://github.com/ParacosmYy/GS_Tool.git`
- commit 消息用中文，说明改了什么和为什么
- 不要提交 build/ 目录
- .vscode/settings.json 和 c_cpp_properties.json 需要提交
