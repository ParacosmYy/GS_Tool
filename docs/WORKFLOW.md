# SerialForge 开发工作流

## 每轮六角色

每个功能迭代固定启动六个角色，角色可以由只读子代理承担：

1. 产品：范围、用户场景、验收结果；
2. 架构：分层、端口、依赖方向、并发和扩展边界；
3. UI 设计：界面布局、交互状态、键盘操作和信息层级；
4. 开发：唯一源码写入者，按明确文件范围实现；
5. 验证：静态、启动、手工、硬件和打包证据，不创建测试专用资产；
6. 打包/流程：依赖、许可证、PyInstaller、版本和交付检查。

产品、架构、UI、验证、打包角色只返回证据和建议。开发角色必须知道自己不是唯一使用共享目录的代理，不能回滚别人的修改，只能写入被明确分配的文件。

## 项目级强制协作规则

本文件与根目录 `AGENTS.md` 一起构成执行规则；若规则冲突，以更严格的规则和用户当前明确要求为准。

- 每个开发周期固定先做六角色只读评审：产品、架构、UI 设计、开发、验证、打包/流程；默认子代理
  为 `luna_max`，复杂并发/安全/调用链问题才逐级升级到 `terra_max`，最后才使用 `sol_medium`；
- 父代理是唯一整合者和最终责任人；每周期最多一个源码写入者，其他子代理不能修改共享 checkout，
  不能创建 worktree、不能回滚用户改动、不能把建议冒充验证；
- 用户已经明确不希望持续启动软件，所以默认不启动 GUI/EXE/服务；只执行静态、编译、导入、已有
  构建或短时进程内纯数据验证。GUI/offscreen、打包启动、真实硬件和外部发布均需单独授权；
- 不创建、修改或运行 unit test、mock、fixture、harness 或其他测试专用资产；默认不运行既有单元
  测试。验证优先使用现有脚本、编译诊断、静态分析、手工向量、回环和授权硬件；
- UI 快速迭代必须保持 presentation-only：动效/背景可暂停、支持 reduced-motion、低干扰并可回退；
  动效不承载连接/业务事实。多个视觉候选未被用户选定前，不固化美术方向；
- 嵌入式 C/C++ 改动必须先走 `$mcu`、`$embedded-enterprise-workflow`、
  `$embedded-code-review-simplifier`，记录公开一手 vendor source、独立 review、简化评估和
  非破坏性验证；Python desktop 轮次记录 applicability=N/A，不能声明固件或认证合规。

## Handoff 交接门（强制）

每次“交给下一轮/下一位协作者”、暂停、阻塞或完成一个切片，都必须更新
`docs/handoffs/current.md`，并新增一份不覆盖历史的 `docs/handoffs/YYYY-MM-DD-<scope>.md`。交接格式见
[`docs/handoffs/TEMPLATE.md`](handoffs/TEMPLATE.md)。至少包含：

1. 用户目标、当前结果、范围和明确非范围；
2. 实际修改文件和每个文件的职责；
3. 六角色 agent id、结论、关键发现和父代理整合动作；
4. 架构/依赖/并发/安全决策与行为保持的简化说明；
5. 已运行命令、实际结果、是否启动软件、硬件/发布授权状态；
6. 未运行验证、原因、剩余风险、阻塞条件、下一步和待用户选择；
7. 若涉及嵌入式，公开来源适用性、review、simplification、R&D validation 和完成标记。

`docs/handoffs/current.md` 是最新状态索引，不替代 ADR、约束或审查记录；历史 handoff 只追加不覆盖。

## 变更门禁

### Gate 0 — 定位

读取 `AGENTS.md`、本文件和相关设计文档，检查现有目录与状态。禁止创建 worktree。

### Gate 1 — 定义

写清用户结果、范围、非范围、错误/恢复行为、性能预期和验证证据。跨层、依赖、线程、持久化或授权变化必须新增/更新 ADR。

### Gate 2 — 六角色审查

输出必须包含：发现严重性、文件/符号证据、假设、未决风险、建议和是否可以开始编码。高风险 RTT、BLE、网络监听或动态代码加载不得凭直觉放行。

### Gate 3 — 单写入实现

只允许一个开发者修改源码范围。优先最小完整切片；UI、应用、领域、基础设施和插件代码不能互相越层。

### Gate 4 — 独立复核与简化

检查 UI 阻塞、线程关闭、队列上限、错误传播、资源释放、隐式全局状态、重复抽象和无用依赖。能用一个清晰端口解决的，不引入大型插件系统。

### Gate 5 — 验证

默认执行：

```powershell
uv sync --locked --extra dev
.\scripts\check.ps1
```

需要 EXE 时再执行：

```powershell
.\scripts\package.ps1
```

真实 UART/BLE/J-Link 验证必须记录设备型号、驱动、配置、结果和未运行项目。没有硬件时只能报告静态/启动/打包证据。

仓库内的 `.github/workflows/windows-quality.yml` 只运行 Windows 上的 locked sync、compileall、
Ruff 和 core/BLE × onedir/onefile 四矩阵打包，并对每个隔离模式运行 provenance manifest verification；
不创建或运行单元测试、mock、fixture 或其他 test-only asset。真实硬件、完整许可证 bundle 和签名发布
仍由具备相应环境的人工门单独执行。

M5c 额外要求使用临时 JSONL/内存向量验证严格 replay codec、RX-only、历史来源隔离、时间轴、
暂停/停止、背压和超长行边界；这些向量不落库、不加入测试专用目录。历史回放必须继续复用
protocol/component/dataset worker，不能在 UI 中直接解析 JSONL 或生成派生值。

M5d 的曲线验证使用临时 Dataset DTO/内存向量与 Qt offscreen 渲染：检查相对时间轴、有限数值
过滤、错误/空数据提示、512 点上限、100 ms latest-wins 刷新、历史来源和关闭生命周期；不引入
图表依赖，也不把自绘 widget 变成 application worker。

M5e 的批量命令验证使用临时 immutable batch/内存发送向量和 Qt offscreen 编辑器：检查 16/32/512 B/16 KiB
边界、顺序、CRLF、延时取消、失败停止、历史复用、TCP Server 显式 Peer、BLE typed write、RTT 禁止路径和
关闭清理；执行语义只记录本地 enqueue，不宣称线缆完成，不创建宏脚本或测试专用资产。

M5f 的协议预设验证使用 immutable catalog 和临时 `ProtocolConfig` 向量：检查目录数量/字段边界、
预设配置匹配、自定义配置回退、选择预设不改变 parser、显式应用才 reset parser，以及 Qt offscreen
下的控件状态、tooltip 和非 UART/TCP Client scope 文案。预设不新增运行时依赖，不创建协议脚本或
test-only asset；Modbus RTU/MAVLink/NMEA 完整 codec 仍列为未覆盖项。

M5g 的 NMEA 验证使用临时 bytes 向量：已知 `$...*HH` 正帧、坏 checksum、坏 hex、缺失/重复 `*`、
非 `$` 起始、空 body、CRLF、跨读取分片、`INVALID_FORMAT`/`INVALID_CHECKSUM` 区分、raw truth 和
后缀去除；再执行 Qt offscreen 预设应用、静态门、onedir/onefile 启动/关闭。NMEA 标准正文不复制
进仓库；真实 GNSS/UART、设备版本差异、RMC/GGA 字段解析和标准许可仍是未运行项。

M5h 的 Modbus RTU 验证使用临时完整 ADU bytes：标准 request `01 03 00 00 00 0A C5 CD`、坏 CRC、
保留地址、非法 function、异常 function/exception code、截断、超限、codec dump/load、row/status、
字段 error 阻断 Dataset 和 raw truth；必须明确 CRC 不再由通用 framing 消费。再执行 Qt offscreen、
locked static/compile、onedir/onefile 启动/关闭和包内容检查。不能用普通 UART/TCP read chunk 证明
t1.5/t3.5；真实串口、设备寄存器映射、request/response、主从事务和 timing-aware framer 另列未运行项。

M5i 的 MAVLink 验证使用临时完整 v1/v2 packet bytes：CRC-16/MCRF4XX 向量、common message id 0/
CRC_EXTRA 50、缺失 mapping 的 `UNVERIFIED`、坏 CRC、signed v2 的 13 B signature 未认证、未知
incompat flag、sysid/compid 0、截断、超长和多余字节；再检查 schema dump/load、router fields、
非 `VALID` 字段 error 阻断 Dataset、UART-only profile scope 和 raw truth。执行 Qt offscreen、locked
static/compile、onedir/onefile 启动/关闭及 archive import 检查。不得用普通 UART/TCP read chunk 证明
MAVLink stream resync；真实 UART/飞控、方言完整 CRC_EXTRA mapping、签名密钥认证、TX 编码和飞控
message semantics 另列未运行项。映射来源必须记录官方 URL 和 revision；示例 `master` 不能作为生产锁定证据。

M5j 的 generation barrier 验证使用临时内存事件向量和实际 worker：检查 protocol generation 随
configure/reset 递增、`occurred_at` 从 live ingress 保留、component/Dataset event 传播完整上游
generation、低于 upstream fence 的 stale event 不入队，以及 fresh event 可继续通过；再执行 ViewModel
generation-chain 过滤的 Qt offscreen/API 检查、locked static/compile 和最终双模式启动/关闭。不得用
generation barrier 冒充 Replay session segment、真实 UART/BLE/RTT、Modbus/MAVLink stream boundary
或发行许可证验证；这些保持独立未运行项。

M5k 的 replay segment 验证使用临时内存 JSONL/bytes 和实际 protocol worker：检查同一原始
`RawRecord.session_id` 可以跨 chunk 保持 partial state；session 变化时旧 tail 只产生一个
`INCOMPLETE` frame，新 segment 不拼接旧 bytes；FIFO 已接受 ingress 不被 replay 线程的直接 reset
丢弃；连续重复出现同一 UUID 仍按连续段变化处理。再执行 locked static/compile、Qt offscreen
回放/关闭和默认 onedir/onefile 启动门。不得把该切片冒充实时 Modbus/MAVLink timing/resync、
真实 UART/BLE/RTT 或正式许可证验证。

M5l 的 unified-error 验证使用 Qt offscreen 当前 composition：调用 MainWindow 的本地校验入口和
ViewModel 的结构化错误入口，确认两者都由同一 `ErrorInfo` 状态驱动；检查 detail tooltip、清除
后 `None` 信号、成功操作清除、错误栏隐藏、关闭后无残留进程。不得把这个 presentation 切片
冒充 session-scoped error aggregation、真实 UART 错误恢复、实时协议 timing 或许可证审查。

M5m 的 realtime stream boundary 验证使用临时进程内 bytes/DTO 向量与实际 protocol worker：
检查 MAVLink 噪声、半包、坏长度后的保守 re-anchor、无 CRC_EXTRA 的 `UNVERIFIED`、显式
profile upgrade；检查 Modbus t3.5、t1.5～t3.5、无 timing quality 合并、256 B 上限和
`GapObservation` quality；检查 queue drop 后 source continuity epoch 先结束旧 partial 再
处理新数据。再执行 locked compile/Ruff 门；不启动 GUI/EXE，不创建测试文件，不把 host read gap
冒充 wire timestamp。当前 presentation 接入还需在获得授权后单独做 Qt offscreen/GUI 门，
检查新 framing 的 preset 应用、Modbus timing 提示、固定 256/280 B 上限和 incomplete/gap/resync
展示；真实 per-byte UART timing、Modbus transaction、MAVLink dialect/signature 和硬件验收另列
后续门。

G0 的 packaging gate 必须使用 scripts/package.ps1 单一入口，并检查版本源、core/BLE 内容隔离、
PyInstaller archive listing、locked dependency tree、PE file/product version、NotSigned 状态、
SHA256SUMS、NOTICE、第三方 inventory 和 release_eligible=false。core 不得出现 Bleak/WinRT，
任何变体不得出现 J-Link/SEGGER/probe-rs vendor binary；不因该门引入 RTT 驱动或工具。

### Gate 6 — 交付

检查实际 EXE、启动、退出、资源、版本和第三方许可证说明。PyInstaller 单文件只是分发格式，不等于安装器、签名或供应商 SDK 授权。

## 子代理输出格式

```text
Role: <product|architecture|ui|development|verification|packaging>
Scope: <exact files or question>
Result: <pass|revise|blocked>
Evidence: <files, symbols, commands, observations>
Findings: <severity and detail>
Unresolved risks: <none or list>
Parent action: <integrated|deferred|escalated>
```
