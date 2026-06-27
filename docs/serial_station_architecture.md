# Serial Station 架构与交付标准（重构版）

> 适用范围：任何涉及 Python/PyQt `python/embeddebug/serial_station/` 的新增、重构、协议扩展、测试与发布动作。

---

## 一、目标与边界

Serial Station 是串口上位机的重构落点。目标是把“连接、协议、服务、展示”拆开，避免在单层堆积。

- Python/PyQt 主线目录：`python/embeddebug/serial_station/`
- 对外唯一协议入口：`ISerialProtocol` + `SerialProtocolRegistry`
- 不允许恢复旧 native 串口目录或主窗口核心层实现
- 每次改动必须能映射到三轴状态与可执行验收条款
- 自 PRD-136/B23 起，Python/PyQt 是 `EmbedDebug.bat -> uv run start-embeddebug` 的默认生产方向；遗留 native 打包链路、源码树和测试入口已移除。

### 成功口径

- 能在同一工作台完成：配置端口 → 连接 → 发送/接收 → 日志 → 回放/导出。
- E/U/D 状态变化必须有证据，不允许仅凭源码存在提升状态。
- 闭环失败率持续下降并可审计。

## 二、内部五层分工

### ui/
- 只负责展示和意图收集，不触碰连接字节细节。
- 仅通过信号调用 controller。

### controller/
- 协调层，负责 UI 事件、协议/服务调用编排、状态和错误映射。
- 不持有协议内部状态机细节。

### core/
- 串口 open/close/send/bytes 收发、会话状态、dispatcher 入口。
- 依赖 `ISerialProtocol` 抽象，不得 include 具体协议。

### protocols/
- 命令构建、frame 定义、流式解析。
- 不依赖 UI、service、workers。

### services/
- 日志、导出、回放、档案。
- 不直接触碰 QWidget 与串口线程。

### workers/（保留时）
- 后台耗时任务和异步执行，结果通过 signal 回 controller。
- 不能直接更新 UI。

## 三、硬性边界（硬规则）

1. UI 不直接调用 `SerialManager`。
2. `core` 不 include `protocols/<name>/`。
3. 协议目录不 include UI 或 `SerialStationWindow`。
4. 文件读写只在 `services/`，日志导出在 service。
5. 新增协议必须补 `tests/python/` 对应 parser + build 测试。
6. 新增控件/协议能力必须落文档，并更新三轴状态。

## 四、目录与文件约束（串口专项）

### 4.1 Python/PyQt 主线

```text
python/embeddebug/serial_station/
  app/
    main.py
  ui/
    ...
  controllers/
    ...
  core/
    ...
  protocols/
    base.py
    raw_data.py
    fire_water.py
    just_float.py
  services/
    ...
  workers/
    ...
  drivers/
    ...
```

Python 测试统一落：

```text
tests/python/
  unit/
  integration/
  ui_smoke/
tests/fixtures/serial_station/
  vofa/
  logs/
  replay/
```

Python/PyQt lane 规则：

- 启动、测试、打包、验证通过 uv script 暴露，不使用未登记的裸脚本入口。
- 默认命令：`start-embeddebug`、`test-embeddebug-py`、`package-embeddebug`、`verify-package-embeddebug`。
- 兼容命令：`start-embeddebug-py`、`package-embeddebug-py`、`verify-package-embeddebug-py`。
- `start-embeddebug` 和 `EmbedDebug.bat` 默认进入 Python/PyQt。旧入口不再作为用户路径、fallback 或验收路径。
- Python 生产代码不得放入 `tools/`。
- PyInstaller 只用于 Python lane，优先 `onedir`，workpath 不得使用仓库 `build/`。
- PyQt6 依赖进入 `pyproject.toml` 前必须记录 GPLv3 或商业授权路线。

## 五、对标主流工具能力差距的量化闭环

### 5.1 已具备（当前）

- UART 配置、连接、发送、接收、日志、回放、导出闭环
- 规则化协议层与 pytest 测试雏形
- 配置档案化与会话记录

### 5.2 仍缺（高优先）

- 插件化生态：缺少运行时热插拔能力
- 多源接入：目前多以单 UART 为主
- 高级可视化套件：FFT/直方图/游标面板能力不统一
- 自动化引擎：缺少脚本化条件触发与外部 API 控制
- 会话模板化：复测流程和项目模板共享能力不足
- 告警联动：无统一告警策略与过滤器编排

### 5.3 缺口量化验收目标（可测）

- 插件化：新增/禁用协议或可视化组件无需重启，目标 5 秒内生效。
- 多源接入：目标至少支持 UART + TCP + UDP 三源并发。
- 可视化：同一帧可并行投射到 2 个以上图表，刷新抖动小于 5%。
- 脚本：条件触发脚本从接收到执行总延迟低于 50ms。
- 会话模板：至少支持 3 套流程模板导入导出。
- 告警：支持至少 4 类错误码映射与可视化规则。

### 5.4 Python/PyQt VOFA+ parity target

Python/PyQt 迁移不是只替换 UI 技术栈，必须按 VOFA+ 能力追平：

- RawData / FireWater / JustFloat 一等协议。
- 数据、命令、参数绑定为一等对象。
- 多通道实时波形、统计、测量和分析视图逐步接入。
- 日志、导出、回放、Profile 通过 fixtures 或明确差异矩阵对齐目标上位机能力。
- 热路径必须使用 typed batch、NumPy ring buffer、批量信号和 pyqtgraph 定时刷新，不允许逐点信号或无界列表。

## 六、执行闭环（统一引用 09-closed-loop）

> ⚠️ **本节原定义的 "S-E-L-T-V-Q-R" + "P-U-L-G-S-B" 双段十三环已废除**。
>
> 历史问题：与 02-workflow（S-I-P-E-V-R-L 七环）、06-git-commit（A-I-R-C-L-M-P 七环）
> 三重编号冲突，AI 不知跑哪套。**统一权威定义见 [docs/constraints/09-closed-loop.md](constraints/09-closed-loop.md)**。

### 6.1 通用闭环（每次 Serial Station 改动都跑）

按 [09-closed-loop §一](constraints/09-closed-loop.md) 执行 5 视角自检 + 6 门禁：

1. **5 视角**：架构（分层边界）/ 实现（公共能力复用）/ 测试（parser+build）/ 产品（E/U/D）/ 用户（入口可达）
2. **6 门禁**：`uv run test-embeddebug-py` + `uv run start-embeddebug --smoke` + `cmd /c EmbedDebug.bat --smoke`（启动链路改动时）+ `uv run lint-embeddebug-py` + `uv run check-constraints` + 行数门禁

### 6.2 Serial Station 专项追加（在通用闭环之上）

无论闭环是否完整，Serial Station 改动还需额外满足：

1. **Scope**：明确本次范围是否只改 `serial_station/` 子域
2. **Edge**：检查不越界 `ui/controller/core/protocols/services/workers/drivers`
3. **Path**：确认文件只进入 `python/embeddebug/serial_station/`、`tests/python/` 或授权 fixtures
4. **Test**：新增/修改协议必须配 `tests/python/` 对应 **parser + build** 双测试
5. **License**：PyQt6、PyInstaller 和第三方依赖有授权/notice 记录
6. **Golden**：协议迁移先通过 `tests/fixtures/serial_station/` 的 golden fixtures
7. **Verify**：连接、发送、接收、日志、回放至少各有一次证据（替身/虚拟即可）

闭环不完整时，该轮 Serial Station 改动不进入 `+1` 计分。

### 6.3 门禁失败 → LOOP

按 [docs/superpowers/LOOP_PROTOCOL.md](superpowers/LOOP_PROTOCOL.md) 分层（详见 [09-closed-loop §三](constraints/09-closed-loop.md)）：
环境/启动失败 → Doctor；可复现缺陷 → Debug；重复/过大 → Simplify。
GO 循环 20 轮或 30 分钟触发安全刹车，必须进 LOOP。

## 七、验收清单（每次变更前后）

- [ ] 新增/修改 Python 文件位于 `python/embeddebug/serial_station/`、`tests/python/` 或授权 fixtures 路径
- [ ] 对应测试补齐（至少 parser/feed 和 controller/service）
- [ ] 变更文件未越界到其他层
- [ ] E/U/D 目标更新并可追溯
- [ ] `EmbedDebug.bat` 影响范围可启动验证
- [ ] 缺口量化目标有更新
- [ ] 快速启动 smoke：`uv run start-embeddebug --smoke` 与 `cmd /c EmbedDebug.bat --smoke`
- [ ] Python/PyQt 默认入口仍可启动
- [ ] Python/PyQt 新文件路径符合 `07-directory-structure.md`
- [ ] PyQt6 依赖变更前已有授权决策记录

## 八、与项目目标分的绑定

- 本专项每次可提交只在上述验收完整时记 `+1`。
- 任何一项缺口未修复但已声称达成，属于高风险项，视为阻断提交。
- 目标分日志中每 20 分必须出现一次 `Serial Station` 的量化进展记录。
