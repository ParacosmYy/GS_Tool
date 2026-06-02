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
| GCC 14.2.0 | `E:/Tool/DevEnv/x86_64-14.2.0-release-win32-seh-msvcrt-rt_v12-rev2/mingw64/bin` |
| CMake 4.0.1 | `E:/Tool/DevEnv/cmake-4.0.1-windows-x86_64/cmake-4.0.1-windows-x86_64/bin` |
| Ninja 1.13.2 | PATH中 |
| GDB 16.2 | PATH中 |
| J-Link V932 | `E:/Embedded/Tool/SEGGER_IOT/JLink_V932` |

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
