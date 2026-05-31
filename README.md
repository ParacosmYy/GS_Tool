<div align="center">

# EmbedDebug

**企业级嵌入式调试集成工具**

*串口终端 · SEGGER RTT 查看器 · OTA固件升级 · 协议解析 · 波形显示*

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows-0078D6.svg)]()
[![Qt](https://img.shields.io/badge/Qt-6.8-41CD52.svg)]()
[![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C.svg)]()
[![Build](https://img.shields.io/badge/build-CMake%2BNinja-064F8C.svg)]()

</div>

---

## 功能概览

EmbedDebug 是一款专为嵌入式开发者设计的统一调试工具，将串口调试、TCP/UDP通信、协议帧解析、实时波形、OTA固件升级整合到一个应用中，告别在 WindTerm、sscom、SEGGER RTT Viewer 之间频繁切换的烦恼。

### 统一连接接口

| 连接类型 | 状态 | 说明 |
|:---------|:----:|:-----|
| 串口 | ✅ | 多串口同时连接，完整参数配置(波特率/数据位/停止位/校验/流控) |
| TCP Client | ✅ | 连接远程设备的TCP服务 |
| TCP Server | ✅ | 监听端口等待设备连接 |
| UDP | ✅ | UDP单播和广播通信 |
| SEGGER RTT | 📋 计划中 | 通过J-Link SDK动态加载实现RTT数据查看 |

### 串口终端

- **自绘制引擎** — 基于QPainter的高性能终端，避免QTextEdit性能瓶颈
- **三种显示模式** — 文本 / HEX / 混合模式，随时切换
- **时间戳** — 毫秒级精度的收发时间戳
- **快捷指令** — 可配置的指令按钮栏，一键发送常用命令
- **定时发送** — 循环发送，可配置间隔和命令队列
- **发送历史** — 最近50条命令记录，一键重发
- **搜索过滤** — 关键词、HEX模式、正则表达式搜索
- **多串口** — 同时连接多个串口，Tab切换

### 协议帧解析

- **可视化帧编辑器** — 拖拽式字段配置(帧头/长度/数据/校验/帧尾)
- **实时解析** — 状态机解析器，实时提取字段值
- **多协议** — 运行时切换帧格式定义
- **错误检测** — CRC校验失败、帧长度异常高亮显示
- **导入导出** — JSON格式帧定义，方便团队共享

### 实时波形

- **Qt Charts集成** — 流畅的实时数据可视化
- **自动映射** — 协议解析器的数值字段自动成为可选通道
- **手动公式** — 自定义从原始字节中提取数据的规则
- **多通道** — 同时显示多个数据流

### OTA固件升级

| 协议 | 块大小 | 特性 |
|:-----|:------:|:-----|
| XMODEM-Checksum | 128 B | 算术校验和 |
| XMODEM-CRC | 128 B | CRC16校验 |
| XMODEM-1K | 1024 B | CRC16，更大块传输 |
| YMODEM | 可变 | Block 0携带文件名+大小，批量传输 |
| ZMODEM | 可变 | CRC32校验，连续发送，断点续传 |

- **自动HEX→BIN** — 丢入Intel HEX文件，自动转换并发送
- **进度追踪** — 传输百分比、速率、ETA实时显示
- **OTA历史** — 所有升级记录带时间戳和成功/失败状态

### 数据持久化

- **配置保存** — 串口设置、面板布局、窗口位置自动保存
- **会话管理** — 保存/恢复完整工作区
- **日志录制** — 自动或手动录制，时间戳对齐的多数据流日志(.edl格式)
- **日志回放** — 按原始时间间隔或加速回放录制会话
- **数据导出** — TXT / CSV / BIN，支持时间范围选择

### 界面与主题

- **IDE风格布局** — 左侧导航树 + 右侧内容面板
- **多主题** — 暗色终端风 / 现代深色 / 浅色，QSS可扩展
- **双语** — 中文 / 英文，基于Qt Linguist国际化

---

## 架构设计

```
┌─────────────────────────────────────────────────────┐
│                    MainWindow                        │
│  ┌──────────┐  ┌──────────────────────────────────┐ │
│  │  导航树   │  │           内容面板               │ │
│  │          │  │  ┌────────────────────────────┐  │ │
│  │ ▶ 串口   │  │  │      TerminalWidget        │  │ │
│  │ ▶ TCP    │  │  │      (QPainter自绘引擎)     │  │ │
│  │ ▶ RTT    │  │  └────────────────────────────┘  │ │
│  │ ▶ OTA    │  │  ┌────────────────────────────┐  │ │
│  │          │  │  │    QuickCommandBar          │  │ │
│  │          │  │  ├────────────────────────────┤  │ │
│  │          │  │  │    发送栏 / 配置面板        │  │ │
│  └──────────┘  │  └────────────────────────────┘  │ │
│                └──────────────────────────────────┘ │
├─────────────────────────────────────────────────────┤
│  ConnectionManager ── IConnection (抽象接口)         │
│    ├── SerialConnection  (QSerialPort)               │
│    ├── TcpConnection     (QTcpSocket/QTcpServer)     │
│    ├── UdpConnection     (QUdpSocket)                │
│    └── RttConnection     (J-Link SDK / QLibrary)     │
├─────────────────────────────────────────────────────┤
│  数据流: IConnection ──▶ TerminalModel               │
│           ├── TerminalWidget   (终端显示)             │
│           ├── FrameParser      (协议解码)             │
│           ├── ChartWidget      (波形图)               │
│           ├── DataLogger       (录制/回放)            │
│           └── OtaManager       (固件升级)             │
└─────────────────────────────────────────────────────┘
```

**核心设计决策**:
- **IConnection抽象** — 所有功能不依赖具体连接类型
- **信号/槽数据总线** — `dataReceived`信号扇出到所有消费者
- **状态机解析** — 帧解析器和OTA协议均使用干净的状态机模式
- **QPainter渲染** — 终端绕过QTextEdit，在高波特率下零拷贝显示

### 设计模式

| 模式 | 应用场景 |
|:-----|:---------|
| 策略模式 | OTA协议切换 (XMODEM/YMODEM/ZMODEM) |
| 观察者模式 | 数据流分发 (Qt信号/槽) |
| 工厂模式 | 创建不同类型的连接 |
| 状态模式 | 连接状态管理、OTA传输状态机 |
| 单例模式 | SettingsManager、ThemeManager |
| 命令模式 | 快捷指令、发送历史 |
| 适配器模式 | J-Link SDK适配 |

---

## 技术栈

| 组件 | 技术 |
|:-----|:-----|
| 语言 | C++17 |
| UI框架 | Qt 6.8 (Widgets) |
| 图表 | Qt Charts |
| 串口 | Qt SerialPort |
| 网络 | Qt Network |
| 构建 | CMake 4.0 + Ninja |
| 编译器 | MinGW GCC 14.2 |
| 调试器 | GDB 16.2 |
| IDE | VS Code + CMake Tools |

---

## 快速开始

### 环境要求

- Windows 10/11
- [MinGW GCC 14.2+](https://github.com/niXman/mingw-builds-binaries/releases)
- [CMake 3.20+](https://cmake.org/download/)
- [Ninja](https://ninja-build.org/)
- Qt 6.8 (通过 [aqtinstall](https://github.com/miurahr/aqtinstall) 安装)

### 安装Qt（如尚未安装）

```bash
pip install aqtinstall
aqt install-qt windows desktop 6.8.3 win64_mingw ^
    --outputdir E:\Tool\DevEnv\Qt ^
    --modules qtserialport qtcharts qtnetworkauth
```

### 构建

```bash
git clone https://github.com/ParacosmYy/GS_Tool.git
cd GS_Tool
git checkout feat/embed-debug

# 配置
cmake -G Ninja -B build ^
    -DCMAKE_PREFIX_PATH=E:/Tool/DevEnv/Qt/6.8.3/mingw_64

# 编译
cmake --build build

# 部署Qt运行时DLL
E:\Tool\DevEnv\Qt\6.8.3\mingw_64\bin\windeployqt.exe build\EmbedDebug.exe
```

### 运行

**双击项目根目录的 `EmbedDebug.bat`** 即可启动。

或手动运行:
```bash
cd build && EmbedDebug.exe
```

---

## 项目结构

```
src/
├── core/               # 应用核心
│   ├── MainWindow      # 主窗口(IDE风格布局)
│   ├── ConnectionManager  # 统一连接生命周期管理
│   ├── ConnectionFactory   # 连接工厂(工厂模式)
│   └── ThemeManager    # 多主题QSS加载器(单例)
├── connection/         # 抽象连接层(基础设施层)
│   ├── IConnection     # 接口: open/close/write/signals
│   ├── SerialConnection  # QSerialPort实现
│   ├── TcpConnection   # QTcpSocket/QTcpServer实现
│   └── UdpConnection   # QUdpSocket实现
├── terminal/           # 高性能终端(表现层)
│   ├── TerminalWidget  # QPainter自绘控件
│   ├── TerminalModel   # 线程安全行缓冲
│   └── TerminalSearchBar  # 搜索栏(正则/HEX)
├── serial/             # 串口专用功能(表现层)
│   ├── SerialConfigPanel  # 端口/波特率/数据位配置UI
│   ├── QuickCommandBar # 一键指令按钮栏
│   ├── TimedSender     # 循环定时发送引擎
│   ├── SendHistory     # 发送历史管理
│   └── DataStatistics  # 实时数据统计
├── protocol/           # 协议帧解析(业务层)
│   ├── FrameDefinition # 帧格式数据模型(JSON序列化)
│   ├── FrameParser     # 状态机帧解析器
│   ├── FrameVisualEditor  # 可视化帧格式编辑器
│   ├── ProtocolView    # 解析结果表格展示
│   └── IntelHexParser  # Intel HEX文件解析器
├── chart/              # 实时波形(表现层)
│   └── ChartWidget     # Qt Charts封装(滑动窗口)
├── ota/                # OTA固件升级(业务层)
│   ├── OtaManager      # OTA调度管理器(策略模式)
│   ├── OtaWidget       # OTA操作面板(进度/速率/ETA)
│   ├── OtaHistoryModel # OTA历史记录模型(持久化)
│   └── protocols/
│       ├── XModemTransfer  # XMODEM (Checksum/CRC/1K)
│       ├── YModemTransfer  # YMODEM (Block 0 + 批量)
│       └── ZModemTransfer  # ZMODEM (CRC32 + 连续发送)
├── rtt/                # SEGGER RTT (计划中)
│   ├── JLinkBridge     # J-Link DLL动态加载(适配器模式)
│   └── ...
└── utils/              # 公共工具(基础设施层)
    ├── CRC             # CRC8/CRC16-CCITT/CRC16-Modbus/CRC32
    ├── HexConverter    # HEX编码/解码
    ├── RingBuffer      # 线程安全环形缓冲区(模板)
    ├── DataLogger      # 数据日志录制/回放(.edl格式)
    ├── DataExporter    # 数据导出(txt/csv/bin)
    └── SettingsManager # JSON配置持久化(单例)
```

---

## 开发路线图

| 阶段 | 功能 | 状态 |
|:----:|:-----|:----:|
| 1 | 串口基础收发 + HEX + 终端 | ✅ 已完成 |
| 2 | 快捷指令、定时发送、搜索过滤、导出、多串口、主题 | ✅ 已完成 |
| 3 | 协议帧解析 + 实时波形 + Intel HEX解析 | ✅ 已完成 |
| 4 | TCP/UDP Client + Server | ✅ 已完成 |
| 5 | OTA升级 (XMODEM/YMODEM/ZMODEM) | ✅ 已完成 |
| 6 | SEGGER RTT查看器 (J-Link SDK) | 📋 计划中 |
| 7 | 集成联调、国际化、性能优化、打包发布 | 📋 计划中 |

---

## 贡献指南

1. Fork本仓库
2. 创建功能分支 (`git checkout -b feat/your-feature`)
3. 提交有意义的commit message
4. 推送到分支 (`git push origin feat/your-feature`)
5. 发起Pull Request

---

## 许可证

MIT License — 详见 [LICENSE](LICENSE)

---

<div align="center">

*基于 Qt 6 · C++17 · CMake 构建*

*为嵌入式工程师设计，由嵌入式工程师打造。*

</div>
