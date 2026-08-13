# UI-1.3 交接归档：状态层级、来源语义与 renderer 生命周期

日期：2026-08-09  
范围：SerialForge presentation-only UI 美化与动态边界  
状态：静态检查完成；运行时、硬件和发行门未授权，均 `not-run`

## 目标与边界

用户目标是持续美化 UI 组件和动态效果，所有要点交付后再停止。本轮继续沿用既有边界：只修改 presentation/交接记录，不启动 GUI/EXE/服务，不连接真实传输或硬件，不创建、修改或运行测试专用资产，不操作 worktree；二次元视觉方向尚未选择，不固化角色、背景、GIF、粒子或具体美术资源。

当前 checkout 未检测到 `.git`，因此没有 commit/status/diff 基线；本文的文件范围是当前 checkout 的静态观察，不是 Git diff 推导。

## 交付内容

### 状态与来源语义

- 顶部来源 badge 和 pipeline summary 区分实时可派生来源、实时 raw-only、历史回放、历史文件无可用 RX。
- Protocol、Component、Dataset、Curve 控件和操作入口统一使用 `_derived_source_supported()` gate。
- 历史文件无 RX 时，组件、Dataset、曲线状态和空态不会误报为实时 raw-only，并提供换文件/停止回放后连接 UART/TCP Client 的行动提示。
- 从历史无 RX 直接进入 UART/TCP Client OPEN/OPENING 时，历史上下文会被清理，Protocol/Component/Dataset/Curve 控件重新启用并恢复当前 Dataset 配置/样本投影。
- 历史回放不允许应用 Modbus RTU timed：回放不提供物理线缆静默间隔，host gap 不被伪装成 wire timing。

### 动态和生命周期

- `MotionController` 继续作为顶部装饰动效的单一共享时钟，支持低动效、显式暂停、自然结束静态帧、隐藏/最小化/关闭停止。
- Component rows 与 Dataset curve 使用 bounded/latest-wins 刷新；非当前协议页、隐藏、最小化、关闭时停止或丢弃迟到 renderer 更新。
- 协议帧 preview 在隐藏/非当前页缓存最近有界快照，恢复可见时再渲染；终端保留有界 raw buffer，窗口隐藏/最小化时暂停 Qt 文档重绘，恢复时重绘。
- Dataset 曲线有 keyboard focus ring；curve snapshot 对极大整数、非法字段类型和非有限值保持安全拒绝/跳过。

### BLE 与反馈

- BLE write mode 按 characteristic capability 自动归一化。
- 扫描结果不再自动选择设备；用户必须显式选择，已选设备消失时不会静默切换到另一台。
- notification/indication 请求一次只允许一个 pending；服务刷新、会话关闭、错误和 bounded timeout 会释放 UI 锁；迟到且无当前 pending 的订阅事件被忽略。
- pending 期间 BLE read/notify/write 与批量命令入口均禁用并显示原因；真实 BLE operation id、service epoch、worker 关闭时序仍属于 domain/application/infrastructure 专项，未在本轮越界修改。

### 访问性与主题

- 主题保持无外部资源：暗色 surface/border/text/focus/disabled 状态、QMenu、Tab、危险按钮和 curve focus ring 均以内嵌 QSS/绘制完成。
- Component table、batch result table、batch editor table 对完整单元文本设置 `AccessibleTextRole`，可见摘要仍保持有界。
- Protocol 状态支持换行；依赖 framing 的 disabled 控件、BLE 控件和来源空态均补充 tooltip/AccessibleDescription。
- RTT channel 加入显式 Tab 顺序；工作区 QTabWidget 有 accessible name/description。

## 文件范围

本轮直接候选文件：

- `src/serialforge/presentation/main_window.py`
- `src/serialforge/presentation/theme.py`
- `src/serialforge/presentation/widgets.py`
- `src/serialforge/presentation/dataset_curve.py`
- `src/serialforge/presentation/curve.py`
- `src/serialforge/presentation/command_batch_editor.py`
- `src/serialforge/presentation/qt.py`
- `README.md`
- `handoff.md`
- 本归档文件

静态检查未见 domain/application/infrastructure 反向导入 Qt，也未见 presentation 直接导入 `serial`、`bleak`、`socket`。没有 Git 基线，不能将这项观察表述为确切 diff 事实。

## 六角色只读审查

最后一轮角色均为 `luna_max / max / Fast`，且未修改文件：

- 产品/UX：`019fe71c-4c99-7ef3-9f79-3a878fc0f890`；发现并推动修复历史无 RX → 实时连接后的控件恢复和空态行动提示。
- 架构/生命周期：`019fe71c-4ce4-71e3-96cf-f259f0699230`；确认 presentation renderer gate，指出 BLE/backend operation token 和 replay control event 风险留在非本轮层级。
- UI/accessibility/motion：`019fe71c-4d1f-7470-8dcb-404bd983352c`；发现并推动修复 curve focus、表格完整辅助文本、RTT Tab、QMenu 和协议 disabled 提示。
- 开发/代码正确性：`019fe72a-faad-7d23-a3b7-b79edb0e5007`；当前快照静态结论无 P0/P1，提出的初次 BLE 自动首选和 pending 批量入口提示偏差已修复。
- 验证：`019fe71c-4da1-7e91-9684-cdd8f15e4df7`；要求 fresh 静态证据，并校正 QApplication 结论为“模块导入不创建实例”。
- 交付/流程：`019fe71c-4de8-7663-8c91-65025e3e4c2f`；要求更新根 handoff、建立本归档、排除旧 build/dist、记录无 Git 基线。

前一轮六角色记录仍保留在根 handoff；所有角色都未启动 GUI/EXE、服务、硬件、测试或打包。

## Fresh 静态验证

最新源码更新后执行的非破坏命令和结果：

- `.venv\Scripts\ruff.exe check --no-cache src scripts` → `All checks passed!`
- `.venv\Scripts\ruff.exe format --check src scripts` → `55 files already formatted`
- `python -B` 内存 AST（`src` 与 `scripts`）→ `parsed 55 files`
- 新进程导入全部 `serialforge` 非 `__init__` 模块，并检查 `QApplication.instance()` 前后均为空 → `imported 53 serialforge modules; QApplication instance: none`
- 内存 BLE enum、UTF-8 上限、CurvePoint 极大整数、CurveSnapshot 非字符串 field 向量 → `passed`

“无 QApplication”在本文中只表示导入模块不创建 `QApplication` 实例；`presentation.qt` 仍会导出 Qt 类型符号。

## 未运行与风险

- 未运行 tests、`scripts/check.ps1`、GUI/offscreen、截图、HIDPI、真实焦点遍历、Narrator/NVDA、对比度和实际动效帧率。
- 未运行真实 UART/TCP/UDP/BLE/RTT、网络服务、EXE、重新打包或硬件；不得据此宣称运行时、协议、BLE、硬件或发行通过。
- `build/`、`dist/` 中的 provenance/EXE 早于本轮源码，只能作 `observed-historical`，不能代表当前 UI-1.3 交付；本轮未删除、覆盖或重建。
- Replay 极快 EOF 状态反序、EventBus control event 丢失、BLE service epoch/operation token、BLE worker close race 属于应用/基础设施后续专项；本轮明确不改这些层。
- 二次元视觉方向仍待用户选择；当前实现是无资源、方向无关的可替换基础层。

## 后续

用户选择美术方向后，下一轮再由六角色确定色板、密度、动效强度、资源来源和静态回退；若要执行 GUI/EXE/硬件/打包验证，需用户另行授权。
