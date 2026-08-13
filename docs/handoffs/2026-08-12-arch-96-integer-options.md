# ARCH-96 / UI-1.169 交接：有限整数选项与紧凑布局

日期：2026-08-12

## 交付范围

- 协议最大帧长、UDP 最大报文、TCP Server 最大客户端数、批量命令步骤延时改为不可编辑的
  常用有限选项。
- UART/网络/BLE/RTT 的既有超时选项化继续保留；UART 端口和网络 host/port 等自由输入没有被
  强行改成有限选项。
- `BoundedIntCombo` 只属于 presentation 层，保留数字控件兼容读取接口；动态协议上限重建期间
  抑制中间 signal；控件宽度限制修正了网络页的横向撑宽。
- 没有新增动画时钟、线程、事件总线、业务状态、设备 I/O、OTA/AES/RTT/J-Link coupling。

## 架构与复核

ARCH-96 架构师 `019ff32a-7b39-7c50-a28b-ab39345c386c` 与独立 reviewer
`019ff332-291b-7273-a6fd-11faa93e7aa5` 在等待窗口内超时关闭，未形成外部结论，未伪造 PASS。
父代理完成 correctness、architecture、security、performance、readability 五轴 review，执行了
行为保持简化评估，并修正了 `setRange()` rebuild 的 signal fan-out 与 combo 宽度回归。

本轮只涉及 Python/PySide6 presentation；embedded C/C++ public-vendor-source applicability 为
N/A，不作固件标准或认证声明。

## 验证证据

- `ARCH96_COMPOSITION_PASS horizontal_max=[0, 0, 0, 0] udp=65507 server_clients=16 protocol=12345 command_delay=1234 motion=120Hz/8ms`
- 基础 selector、网络/协议/命令交互、固定 MAVLink 上限 `280` disabled、range signal suppression
  均通过。
- `uv run ruff check src scripts`、`uv run python -m compileall -q src`、`scripts/check.ps1` 通过。
- 当前 offscreen 环境仍输出 PySide6 缺少字体目录 warning；这是运行环境字体部署问题，不是主题
  白底或布局溢出证据。EXE 启动、真实显示器 FPS 和硬件验收未运行。

## 打包状态

源码、离屏验收与打包完成；`local-arch-96` onefile 已覆盖 canonical、仓库根目录
`SerialForge.exe` 和 `SerialForge-latest.exe`。三者均为 `48,013,765` bytes，SHA-256 为
`192CED543D4860E8DE2659377A67B5CC3E539D37AE62066D242C4E61F9EC9DBC`；archive listing SHA-256 为
`39F8F8912A12D60FDF1FA521C17D6654D96B5D227D245BD628160C745E7540DA`，provenance verify 通过。
签名为 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。
