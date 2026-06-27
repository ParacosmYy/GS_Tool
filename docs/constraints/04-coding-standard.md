# 04 - Python/PyQt 编码标准（约束版）

> 适用范围：`python/embeddebug/` 下 Python/PyQt 运行时代码。native 源码主线已移除。

---

## 一、语言与风格

- 标准：Python 3.10+，UTF-8 编码。
- import 顺序：标准库 -> 第三方 -> 项目内部。
- PyQt signal/slot 连接必须集中在 UI/controller 装配处，避免跨层随意连接。
- 公开类和复杂公开方法必须有简短 docstring。
- 命名：
  - 类名 `UpperCamelCase`
  - 函数与变量 `snake_case`
  - 常量 `UPPER_SNAKE_CASE`
  - 布尔变量 `is_`、`has_`、`can_` 前缀优先

## 二、错误处理与返回约定

- 错误不能仅 `qDebug()`；必须有：
  - 返回值/结果对象
  - 日志输出（便于复现）
  - 失败信号或错误路径（对用户可见）
- 关键接口返回 `bool` 时要有错误上下文（error string 或失败码）。
- 避免空 catch，避免吞异常；不确定行为需明确失败点。

### 2.1 量化错误处理约束

- 新增关键接口必须带显式失败路径。
- 同一错误场景至少有 1 个回归测试。
- 异常/失败必须可在日志中检索到。

## 三、边界与生命周期

- QWidget/QObject 生命周期遵循 parent 体系，非 QWidget 使用普通对象、dataclass 或明确 owner。
- 严禁 UI 层持有底层 transport/protocol 的可变内部状态。
- 跨线程传输只允许 signal/slot、队列或明确事件通道，不允许裸共享 UI 状态。

### 3.1 结构安全阈值

- 新增类需给出 owner/parent 关系。
- 线程边界变更需要对应日志与测试。
- 同步等待超过 10ms 的逻辑需写明原因。

## 四、性能与复杂度

- 不允许在 GUI 线程执行重计算或阻塞 IO。
- 解析/日志写入需考虑批处理阈值，避免单次小数据反复分配。
- 避免 O(n²) 的帧拼接策略，在可能时使用 ring buffer / 预分配容器。
- 仅在必要时使用全局单例，优先依赖注入或明确 owner。

### 4.1 可量化性能红线

- 连续 1000 条帧处理不得出现明显掉帧，平均处理时延需稳定。
- UI 线程不得直接进行阻塞性解析与写盘。
- 日志处理优先批量落盘，单次写入最小化。

## 五、结构规模约束（硬约束）

- `.py` 理想不超过 300 行；超过需拆分或写明理由。
- 单个函数建议不超过 80 行；超过需通过"重构理由"注释或拆分。
- 函数内复杂分支必须配套单测场景（成功、失败、边界）。
- 文件行数由 `uv run check-constraints`（[tools/check_constraints.py](../../tools/check_constraints.py)）机械检测，超限 commit 直接 fail；门禁定义见 [09-closed-loop §一.2](09-closed-loop.md)。

### 5.1 可量化评分映射

- 任何超过限制但未拆分的变更不得进入加分闭环（`uv run check-constraints` 会拦）。
- 尺寸与复杂度不达标时，commit 分数只能是 `+0`。
- 达标后才进入后续验收评分。

## 六、测试边界（可维护可迭代核心，强制）

> 本节是项目长期可维护的命脉。**测试文件要少、框架要正确分层、按域聚合**。
> 详细规则与机械守护见 [CLAUDE.md §测试文件组织规则](../../CLAUDE.md) 和 [09-closed-loop §一.2 门禁](09-closed-loop.md)。

### 6.1 开发最低要求

- 新增/修改核心逻辑必须至少一条 pytest、pytest-qt 或替身测试验证
- 关键错误路径必须有回归测试（失败连接、格式错误、空输入、超限输入）
- 跨层调用需最小接口替身验证
- UI 逻辑分离可在 pytest-qt 中用 fake 信号验证

### 6.2 测试三层框架（必须正确分层）

| 层 | 目录 | 特性要求 | 禁止 |
|----|------|----------|------|
| unit | `tests/python/unit/` | 毫秒级、纯函数/类、无 Qt 无 IO 无网络 | 禁止 import PyQt、禁止打开文件/网络、禁止 sleep>0 |
| integration | `tests/python/integration/` | 跨模块协作、fixture 加载、替身 transport | 禁止实例化 QApplication（那是 ui_smoke 的职责） |
| ui_smoke | `tests/python/ui_smoke/` | QApplication 实例化、窗口装配、objectName/tr() 合规 | 禁止做深度业务断言（那是 unit/integration 的职责） |

**分层原则**：一个测试只能属于一层。混层 = 拖慢全层 + 难定位回归。

### 6.3 测试文件要少（域聚合）

- 同域测试合并到单文件（如 `test_controller_state_core.py` 含 connection/callback/workbench state）
- 每个文件 ≥3 个测试函数（<3 个的孤儿文件必须并入同域既有文件）
- 同域散落 >3 个文件视为技术债，必须合并
- 新增测试默认追加到同域既有文件，**不是新建文件**

### 6.4 测试可量化指标

- 新增每个接口至少 1 个正向和 1 个异常路径测试
- 每个 commit 新增文件的测试覆盖率不得降级
- unit 全量 < 30 秒（超时说明混入 IO/Qt，必须拆层）
- 单个测试文件执行 < 1 秒（unit 层）

### 6.5 测试可迭代三原则

- **快**：unit 毫秒级，integration 秒级，ui_smoke 可接受十秒级；超时即分层错误
- **独立**：不依赖测试执行顺序、不依赖共享可变状态
- **可定位**：测试名描述被测行为（`test_parse_justfloat_returns_none_on_short_frame`），禁止 `test_1` / `test_ok` 这类无信息命名

## 七、代码评审红线

- UI 逻辑和数据逻辑混写
- UI 直接依赖 `QByteArray` 解析细节
- `core/` 直接触发 UI 导出/绘图/交互
- `protocols/` 操作设置窗、文件写入、数据库或 QWidget
- 跨模块重复实现公共能力（CRC、Hex、日志、settings）

## 八、提交前自检

- include 顺序正确
- 注释齐全
- 错误路径有证据
- 线程边界清晰
- 文件规模与复杂度满足本约束

## 九、最小高效验收（每轮）

1. `uv run test-embeddebug-py`
2. `uv run start-embeddebug --smoke`
3. `cmd /c EmbedDebug.bat --smoke`
4. 启动失败时不允许本轮提交加分
