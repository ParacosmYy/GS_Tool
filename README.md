# GS_Tool / EmbedDebug

> 面向嵌入式工程师的一体化调试工作站：串口终端、协议分析、实时波形、OTA 升级和数据仪表盘。

## 仓库定位

`GS_Tool` 当前主项目是 **EmbedDebug**。它不是一个简单串口助手，而是尝试把嵌入式调试中常用的工具能力统一到一个 Qt 桌面应用里。

| 项目 | 说明 |
|------|------|
| 当前默认分支 | `feat/embed-debug` |
| 技术栈 | C++17 / Qt 6 / CMake / MinGW |
| 运行平台 | Windows 10+ |
| 目标用户 | 嵌入式工程师、固件开发者、调试工具开发者 |
| 核心价值 | 减少串口、波形、协议、OTA、数据记录工具之间频繁切换 |

## 核心功能

- 串口终端：文本、HEX、混合显示；
- 多连接抽象：Serial、TCP、UDP、TLS、WebSocket，后续扩展 BLE、CAN、MQTT；
- 协议分析：帧格式配置、CRC、Modbus、私有协议解析；
- 实时波形：多通道曲线、FFT、游标、直方图、散点图；
- OTA 升级：XMODEM / YMODEM / ZMODEM，HEX 转 BIN，进度追踪；
- 数据记录：会话保存、日志回放、CSV/图片导出；
- 自动化：触发规则、脚本录制、命令回放；
- 仪表盘：数值、进度条、LED、热力图等可视化组件。

## 架构分层

```text
L6 core/                 # 应用协调层：主窗口、控制器、会话、主题
L5 ota/ automation/      # 业务层：OTA、自动化、仪表盘、插件
L4 terminal/ chart/ rtt/ # 表现层：终端、波形、RTT
L3 connection/ protocol/ # 连接与协议层
L2 utils/                # 基础工具：CRC、转换、日志、导出
L1 shared/               # 常量、枚举、轻量类型
L0 interfaces/           # 纯接口契约
```

依赖方向只允许从高层指向低层，禁止反向依赖、横向乱调和跨层跳跃。

## 快速开始

环境要求：

| 依赖 | 推荐版本 |
|------|----------|
| Qt | 6.8.x |
| CMake | 3.20+ |
| Ninja | 1.11+ |
| MinGW GCC | 13+ |

构建：

```bash
git clone https://github.com/ParacosmYy/GS_Tool.git
cd GS_Tool
git checkout feat/embed-debug
cmake -G Ninja -B build -DCMAKE_PREFIX_PATH=E:/Tool/DevEnv/Qt/6.8.3/mingw_64
cmake --build build
```

运行：

```bash
cd build
EmbedDebug.exe
```

## 目录建议

```text
GS_Tool/
├── src/
│   ├── core/
│   ├── connection/
│   ├── protocol/
│   ├── terminal/
│   ├── chart/
│   ├── ota/
│   ├── dashboard/
│   ├── automation/
│   ├── utils/
│   └── interfaces/
├── docs/                 # 设计约束、PRD、跟踪记录
├── resources/            # 图标、主题、QSS
├── tests/                # 测试代码
├── CMakeLists.txt
└── README.md
```

## 开发约束

- `.cpp` 文件尽量不超过 500 行；
- `.h` 文件尽量不超过 200 行；
- UI 文字使用 `tr()`；
- QWidget 需要设置 `objectName`，方便 QSS 管理；
- 新增模块必须符合分层依赖；
- 重要功能先写 PRD 或设计说明，再实现。

## 后续计划

- 完成 CAN / BLE / MQTT / RTT 的真实设备集成；
- 完善主题系统和响应式布局；
- 增加安装包和发布流程；
- 补齐测试和 CI；
- 把协议分析、波形和 OTA 的使用示例补成独立文档。

## License

MIT License
