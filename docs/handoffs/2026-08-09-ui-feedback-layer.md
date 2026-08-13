# SerialForge UI-1.2 组件反馈层交接

日期：2026-08-09  
范围：presentation-only 分区标题、准备提示、空态、只读结果表、键盘路径和动态边界  
状态：in progress（静态门已验证；最终二次元视觉方向、GUI/EXE、重新打包和硬件仍未验收）  
父代理：Codex  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；未检测到 `.git`，未创建或操作 Git/Codex worktree

## 用户目标与约束

用户要求持续美化 SerialForge UI 组件和动态效果，所有验收要点交付成功后才停止。本轮继续遵守：只改当前 checkout 的 presentation；不启动 GUI/EXE/后台服务；不连接硬件；不创建、修改或运行测试专用资产；最终二次元美术方向未选定前不固化角色、插画、GIF、粒子或资源风格。

## 本轮结果

- 顶部/连接页增加 `UART 参数`、`网络端点`、`BLE GATT 设备` 等可见分区标题，并显示“下一步：选择或输入 UART 端口”“下一步：扫描并选择 BLE 设备”等准备提示。
- 活动连接时连接按钮可执行断开；关闭连接仍在 `CLOSING` 时保持禁用。空 UART 端口既不会启用连接按钮，入口被直接调用时也会显示明确错误。
- 核心键盘路径设置显式 Tab 顺序，覆盖传输选择、端口/刷新、UART/网络/BLE 参数、连接、工作区、终端和发送栏。
- 组件帧表和批量结果表设置只读、行选择、交替行色、列宽策略、可访问描述；组件长字段/Raw Hex 保留完整 tooltip。
- 组件表新增“暂无组件帧”“当前筛选无匹配”等空态，并显示“显示 N / 缓存 M”；批量结果和发送历史也有可见空态/占位文案。
- “历史回放”改为“原始记录回放”，与“发送历史”区分；错误详情直接显示在错误栏并设置 accessible description，不再只依赖 Tooltip。
- 动效自然结束时由 `MotionController` 发出一次 `animated=False` 静态帧；状态灯和信号场不会停在最后一帧发光。
- 曲线 projection 对极大整数和不可表示数值安全跳过；正常有限数值语义不变。

## 实际改动文件

- `src/serialforge/presentation/main_window.py`
- `src/serialforge/presentation/command_batch_editor.py`
- `src/serialforge/presentation/widgets.py`
- `src/serialforge/presentation/curve.py`
- `src/serialforge/presentation/qt.py`
- `src/serialforge/presentation/theme.py`
- `README.md`、`docs/ROADMAP.md`、`handoff.md`

未修改 domain/application/infrastructure、ViewModel 信号语义、transport、protocol、recorder、replay、queue 或硬件行为；未新增运行时依赖、图片/GIF/字体/QRC 或 PyInstaller 数据路径。

## 六角色只读评审

| 角色 | agent id | 结果 | 父代理处理 |
|---|---|---|---|
| 产品 | `019fe69f-c17a-76b1-b39f-0d4ca15e5f54` | revise | 建议把“能做什么/为什么不能做/下一步”变成显式准备提示，统一空态并区分发送历史与原始回放；已采纳。 |
| 架构 | `019fe69f-c1c2-7863-84fb-b850d52cf0b1` | revise | 指出自然结束静态帧、组件表重建和过滤空态风险；本轮修复静态帧与展示层空态，未扩展到 ViewModel coalescer。 |
| UI 设计 | `019fe69f-c1fc-76b2-a3b4-378d397993d1` | revise | 建议可见分区标题、焦点路径、空态、非 Tooltip 错误反馈和表格等价文本；已采纳。 |
| 开发 | `019fe69f-c23d-7622-80cf-43a8a2782df5` | revise | 发现活动连接按钮、结果表只读、空 UART 门、曲线极端值和批量空步骤编辑残留；均已修复，二次复核只发现并随后补齐“刷新端口”Tab 链遗漏。 |
| 验证 | `019fe69f-c27a-71c3-96b3-3effbfc0efca` | revise | 静态分层、编译、导入、动效闸门通过；指出 accessibility 元数据、GUI/offscreen、HIDPI 和真实读屏仍未验证。 |
| 打包/流程 | `019fe69f-c2bb-7a71-9523-1553d042abed` | revise | 确认无资源/依赖/hidden-import/打包路径变化；旧四包为历史证据，本轮未重包。 |

六角色均为只读、`luna_max / max / Fast`；父代理是唯一写入者。所有角色未启动 GUI/EXE/服务、未操作硬件、未创建或运行测试专用资产。

## 架构、简化与行为保持

- 保留 `MotionController` 为唯一装饰动效时钟；自然结束发布静态帧，不增加第二个 timer。
- 空态、准备提示、只读表格、列宽和 tooltip 只改变 presentation；不改变数据快照、执行状态或 ViewModel 事件。
- `can_start/can_stop` 分离：连接按钮在可断开状态可用，`CLOSING` 时关闭；UART 空端口被 UI 门阻止，领域校验仍保留。
- 曲线新增 `_finite_number()`，将 `OverflowError` 边界转为跳过计数；有限值、时间序列容量和 latest-wins 刷新保持不变。
- 本轮为 Python desktop 变更，嵌入式 C/C++ assurance applicability=N/A；不据此声称 MISRA、ISO、认证或硬件合规。

## 验证证据

| 命令/检查 | 实际输出 | 状态 |
|---|---|---|
| `.venv\Scripts\ruff.exe check src scripts` | `All checks passed!` | verified |
| `.venv\Scripts\ruff.exe format --check src scripts` | `55 files already formatted` | verified |
| Python `-B` 内存编译 | `compiled 55 files` | verified |
| Python `-B` 全包导入 | `imported 54 serialforge modules without QApplication` | verified |
| `QHeaderView.ResizeMode` 导入 | `ResizeToContents Stretch` | verified |
| 极大整数曲线向量 | `curve extreme-integer guard: passed` | verified |
| AST presentation/动效/键盘/空态/表格/连接门审计 | `presentation boundary, motion, keyboard, empty-state, read-only-table, connection-gate, and curve-boundary audit: passed` | verified |

AST 审计确认 presentation 未直接导入 `serial`、`bleak`、`socket`，非 presentation 未导入 Qt；`MotionController` 同时有 active/inactive 两种帧发布；未发现旧 `pulse()` 调用。

## 未运行项目、授权与风险

- GUI/offscreen、最小尺寸 `980×680`、HIDPI、真实 Tab 循环、读屏、QSS 绘制、颜色对比、隐藏/最小化/关闭时序和运行时帧率：`not-run`，因用户约束未授权。
- EXE 启动、当前源码重新打包、四矩阵 provenance/archive/签名：`not-run`；既有 `dist/release/0.1.0` 只能标为 `observed-historical`。
- UART/BLE/Wi-Fi/TCP/UDP/J-Link 和真实设备：`not-run`，未操作硬件。
- unit test、mock、fixture、harness 和测试专用资产：未创建、未修改、未运行。
- accessibility 元数据仍不是实际读屏证据；部分高级输入控件仍依赖可见标签/Qt 默认推断，后续可继续补齐。
- 组件表在极高吞吐下仍可能全量重建；本轮未改 ViewModel 或引入 `QAbstractTableModel`，运行时性能仍待授权验证。

## 下一步与用户选择

1. 用户选择一个最终视觉候选并确定色板、密度、动效强度、资源来源/许可证和静态回退，下一轮再接入可替换 backdrop。
2. 获得 GUI/offscreen 授权后验证真实焦点循环、读屏、最小窗口、高 DPI、空态/错误态和动效生命周期。
3. 获得打包授权后用当前源码重跑 core/BLE × onedir/onefile；若加入资源，再记录资源哈希、许可证和失败回退。

当前不能宣称完整 UI、最终二次元背景、GUI/EXE、真实硬件或正式发行通过。
