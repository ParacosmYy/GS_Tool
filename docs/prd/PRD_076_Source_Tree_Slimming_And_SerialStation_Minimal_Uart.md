# PRD-076 - Source Tree Slimming And SerialStation Minimal UART

## 背景

当前仓库已经出现明显的源文件膨胀、历史分叉目录和重复能力目录。用户反馈编译变慢、上位机文件数量不符合预期，并且新串口上位机方向看起来缺少明确的 UART 配置入口。

只从现象看，问题不是单一编译参数导致，而是三类风险叠加：

1. `src/utils/` 与若干历史分叉目录数量异常，可能包含大量未收敛、未验证或生成式源码。
2. `CMakeLists.txt` 中显式引用的源码范围过大，导致配置、生成、AUTOMOC 和编译扫描成本升高。
3. 旧串口配置链路存在，但新的 `src/apps/serial_station/` 尚未形成可用的 UART 配置闭环。

## 目标

1. 先建立只读审计工具，量化源码规模、CMake 引用规模、冻结目录、生成式目录和 Serial Station 缺口。
2. 明确后续清理原则：先冻结、再验证引用、再从 CMake 收缩，最后才删除文件。
3. 明确新串口工站的最小 UART 配置闭环，避免继续在旧 `src/serial/` 里扩张。
4. 为后续 BATCH 并行清理提供稳定输入报告。

## 非目标

1. 本 PRD 不直接删除源码。
2. 本 PRD 不直接移动目录。
3. 本 PRD 不直接改 `CMakeLists.txt`。
4. 本 PRD 不直接实现新的 Serial Station UI 或 core 类。
5. 本 PRD 不改变旧串口配置面板的现有行为。

## 当前判断

### 文件数量

一个嵌入式串口上位机不应该长期维持异常巨大的 `src/` 文件规模。当前仓库更像是多轮生成、扩展和历史分叉叠加后的状态，已经不利于维护。

后续要区分三类文件：

| 类型 | 处理策略 |
|------|----------|
| 当前产品入口真实使用 | 保留，必要时重构 |
| 历史兼容但仍被引用 | 冻结，只做 bugfix，不加新功能 |
| 未引用、重复、生成式、实验性 | 先移出 CMake，再进入删除候选 |

### 重复轮子

重复轮子重点集中在以下方向：

- `widgets` / `widgets2`
- `animation` / `animation2`
- `loader` / `loader2`
- `font` / `fonts`
- `icon` / `icons` / `iconprovider`
- `layout` / `responsive`
- `shortcut` / `managers`
- `utils/` 下大量编号或生成式目录

清理时不能只按名字删除，必须结合 CMake 引用、include 引用、运行入口和测试结果判断。

### UART 配置

旧主线中 UART 配置并非不存在，至少包含：

- `src/serial/config/SerialConfigPanel*`
- `src/connection/serial_port/SerialConnection*`
- `src/core/connect/ConnectionController*`
- `src/core/panels/PanelManager*`

真正的缺口是新的 `src/apps/serial_station/` 尚未形成架构文档要求的最小 UART 配置闭环，也未接入 CMake 和主入口。

## 最小 UART 闭环目标

Serial Station 后续最小可用闭环应包含：

1. `SerialStationController`
2. `ui/SerialPortPanel`
3. `core/SerialPort`
4. `core/SerialManager`
5. `core/SerialSession`
6. `protocols/ISerialProtocol`
7. `protocols/SerialProtocolRegistry`
8. `protocols/ascii_text/AsciiTextProtocol`
9. 对应 QTest

最小配置字段：

- 端口名
- 波特率
- 数据位
- 校验位
- 停止位
- 流控
- DTR
- RTS
- 自动重连

## 验收标准

1. 存在只读审计脚本 `tools/source-tree-audit.ps1`。
2. 脚本可以输出 Markdown 报告到 `docs/reviews/simplify/source-tree-latest.md`。
3. 报告至少包含源码数量、CMake 引用数量、冻结目录命中、生成式 utils 目录、旧 UART 配置证据和新 Serial Station 缺口。
4. 本轮不修改生产源码，不影响 `EmbedDebug.bat` 启动链路。
5. 后续任何删除、移动、CMake 收缩必须基于该报告或更新后的审计报告执行。

## 后续建议

1. PRD-077：Serial Station 最小 UART 骨架。
2. PRD-078：CMake 源码引用收缩，只移除可证明未使用的生成式/实验性文件。
3. PRD-079：冻结目录合并计划，逐组处理 `widgets2`、`animation2`、`loader2` 等历史分叉。
