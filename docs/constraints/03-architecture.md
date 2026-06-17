# 03 - 架构原则与分层执行口径（重构版）

> 适用范围：涉及架构决策、跨模块调用、接口变更、新模块引入。

---

## 一、架构基线与禁忌

### 1.1 设计原则

1. 单向依赖：下层不依赖上层，上层可依赖下层。
2. 分离职责：控制器只编排，不做底层解析；底层不处理 UI。
3. 接口优先：跨模块通信优先接口。
4. 证据驱动：不能以“写了代码”替代状态更新。

### 1.2 不能做的事（硬禁忌）

- `MainWindow` 与 `PanelManager` 承担业务逻辑
- `ui` 直接操纵串口核心对象/具体协议对象
- `protocols` 中出现 QWidget、QWidget 派生或 `ui/` 依赖
- `core` include 具体协议目录（只能依赖 `ISerialProtocol`/registry）
- `workers` 直接更新 UI
- 跨模块重复实现 CRC、Hex、RingBuffer、日志、设置管理
- 仅有 stub/TODO/空方法就标为 `U3`/`D3`

## 一-a、架构量化验收（可闭环）

- `I/O-依赖越界`：每次变更目标 ≤0 例。
- `分层越界`：每次提交必须为 0。
- `接口化覆盖率`：跨层调用必须通过 interface。
- `Python 主线落点完整度`：新增产品源码必须位于 `python/embeddebug/`，测试位于 `tests/python/`。
- `测试映射覆盖`：至少对应一个关键链路单测。

### 1-b、最小闭环条款（每次架构考核必做）

- 每轮架构相关考核必须至少执行一次 `uv run test-embeddebug-py`。
- 每轮架构相关考核必须有一次 Python/PyQt smoke：`uv run start-embeddebug --smoke`。
- 若启动失败，架构考核记阻断并不得记 `+1`。

---

## 二、分层模型（当前仓库）

### 2.1 模块层级

| 层 | 目录 | 职责边界 |
|----|------|----------|
| L0 | `src/interfaces/` | 纯接口与契约 |
| L1 | `src/shared/` | 跨模块常量、枚举、轻量值对象 |
| L2 | `src/utils/` | 通用工具（CRC、编码、日志、通道） |
| L3 | `src/connection/`、`src/protocol/`、`src/serial/` | 连接与协议处理 |
| L4 | `src/terminal/`、`src/chart/`、`src/rtt/` | 呈现与数据消费 |
| L5 | `src/ota/`、`src/automation/`、`src/dashboard/`、`src/plugin/` | 场景编排 |
| L5-A | `src/apps/serial_station/` | 串口工站独立 app（内部再分层） |
| L6 | `src/core/` | 应用协调、导航、会话、主题、入口 |
| PY-A | `python/embeddebug/` | Python/PyQt 默认主线；不反向依赖 C++ UI |

### 2.2 依赖矩阵（简化）

- L6 可依赖全部下层；下层不得依赖 L6。
- 同层禁止直接 include，必须通过接口或事件。
- `serial_station` 内部固定：`ui -> controller -> core/protocols/services`，`workers -> core -> signal -> controller`。
- `python/embeddebug/` 是 PRD-136 后的默认主线：Python 可以通过 golden fixtures、配置文件或明确兼容格式对齐 C++ 行为，不得 import、生成或修改 C++ 运行时对象作为正常业务路径。
- C++ `src/` 不得依赖 `python/`；C++/CMake 打包链路不再驱动默认启动、fallback 或验收入口。

---

## 三、架构准入检查（每次跨层改动前）

1. 这个改动属于哪个层？（单层优先）
2. 修改了谁的边界？是否触碰另层职责？
3. 影响了哪组接口？是否有接口层替代直接 include？
4. 谁提供状态、谁渲染状态、谁持久化状态？
5. 有无对应测试（至少一类）？证据路径是什么？
6. E/U/D 目标本轮是否变化？是否真实提升？

不满足时本次改动不允许进入生产代码。

---

## 四、关键接口契约（示例约束）

> 新增/修改跨模块接口前必须放入 `src/interfaces/` 或模块内 `interface/`，并写最小测试。

- `IConnection`：连接抽象（open/close/send/state/error）
- `IPanelProvider`：面板注册与创建
- `IDataSink`：数据分发目标
- `IProtocolParser`：协议解析能力
- `IDevice`：设备能力描述
- `IStationProtocol`（serial_station）：仅用于内部协议抽象兼容

### 4.1 接口变更要求
- 返回错误不可被吞掉（至少 signal 或结果对象）
- 生命周期清晰（owner/parent 明确）
- 不能把 QWidget/QWidget* 作为共享层业务参数

---

## 五、Serial Station 强制边界（再次强调）

以 `python/embeddebug/serial_station/` 为当前唯一新落地路径，禁止新功能回流旧 `src/serial/` 或 C++ 串口目录。

内部边界：

- UI 只发意图信号到 `SerialStationController`，不 parse、不中转字节
- `core/` 处理连接状态、字节读写、分发，不做业务协议语义判断
- `protocols/` 只做命令构建 + frame/stream parser
- `services/` 只做日志、导出、回放、档案
- `workers/` 不直接触碰 UI，通过 signal 回到 controller
- `SerialProtocolRegistry` 是协议注册入口，不允许外层自行 `new` 具体协议
- `core/` 不 include `protocols/<具体协议>/...`
- 插件/扩展能力必须与 registry + loader 契约对齐

### 5.1 Serial Station 交付阈值（阶段性）

- `E3`：Python 源码存在 + 关键单测
- `E4`：路径稳定 + pytest/pytest-qt 覆盖关键链路（含错误路径）
- `U3`：主流程闭环可完成（连接/发送/接收/日志）
- `D2`：替身或虚拟链路验证；`D3/D4` 需真实链路记录

### 5.2 Python/PyQt Serial Station 并行边界

Python/PyQt 迁移按 `PRD-135` 与 `PRD-136` 执行，落点为 `python/embeddebug/serial_station/`。它必须镜像 C++ Serial Station 的分层语义，但不共享 C++ QObject 或 QWidget。PRD-136 后默认用户入口已切换为 Python/PyQt。

```text
python/embeddebug/serial_station/
  app/           # QApplication、启动入口、smoke
  ui/            # PyQt widgets/views，只收集意图和展示状态
  controllers/   # UI 意图编排，不做 byte-level parser
  core/          # session、dispatcher、event bus、transport coordination
  protocols/     # build_command/feed/reset、VOFA+ RawData/FireWater/JustFloat
  services/      # log/export/replay/profile/settings
  workers/       # 后台 I/O 与批处理，不持有 QWidget
  drivers/       # QSerialPort/fake/virtual transport adapter
```

硬性规则：

- PyQt UI 只通过 controller 发意图，不直接操作 transport、protocol parser 或文件导出。
- Python `core/` 只处理 session、bytes、dispatcher 和 transport 状态，不写具体协议语义。
- Python `protocols/` 只处理命令构建、帧定义和流式解析，不 import PyQt widgets。
- Python `services/` 只处理日志、导出、回放、档案和设置，不直接触碰 QWidget 或串口线程。
- Python `workers/` 不直接更新 UI；跨线程结果必须回到 controller 或明确的事件通道。
- Python 协议 parity 必须先通过 golden fixtures，再接 UI。
- Python 运行时包不得放入 `tools/`；`tools/` 只保留工程辅助脚本。
- PyQt6 依赖加入前必须有 GPLv3/商业授权决策记录。

---

## 六、架构反例（禁止清单）

- controller 内部硬编码协议算法
- UI 直接 include 串口底层实现头
- panel 里持有文件系统导出动作
- `core/` 中出现“协议帧解析细节”
- 同层模块直接互相 include（尤其 chart ↔ terminal ↔ protocol）

---

## 七、重构与迁移建议（优先顺序）

1. 先补齐接口边界 + 单测，再补新能力
2. `serial_station` 优先做闭环验证，不先堆积协议扩展数量
3. 先统一公共组件和 theme/setting 通道，再推进多源连接
4. 每迁移一个目录，清点受影响状态与旧目录兼容性

---

## 八、审查清单（每次 PR/Commit 前）

- [ ] 依赖方向满足矩阵
- [ ] `core` 未侵入协议/解析细节
- [ ] 协议未依赖 UI/Service
- [ ] 文件写入仅在 service 或持久化模块
- [ ] controller 不含状态机式 byte-level 解析
- [ ] worker 不直接修改 UI
- [ ] 新能力有接口或文档入口
- [ ] E/U/D 更新有证据
- [ ] 串口工站触碰对应专项文档并保持一致

## 九、与项目评分闭环的架构映射

- 每次 commit 仅当上面审查项全部通过才允许加分。
- 架构级失败项会记为 P0 阻断：`I/O-依赖越界` 或 `分层越界`。
- 任何一次跨层修复成功后，更新本文件版本号与 `docs/constraints/01-project-overview.md` 的阶段记录。

## 十、架构交付前最小启动验收

1. 变更涉及跨层或边界修改时，先执行 `uv run test-embeddebug-py`。
2. 再执行 `uv run start-embeddebug --smoke` 与 `cmd /c EmbedDebug.bat --smoke`。
3. 若无法启动，立即阻断本轮加分并记录修复清单。
