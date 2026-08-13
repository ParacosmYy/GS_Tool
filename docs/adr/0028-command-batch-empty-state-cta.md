# ADR 0028：批量命令空态 CTA 与结果表互斥可见性

状态：已接受（2026-08-10）  
范围：`presentation/command_batch_empty_state.py`、`presentation/action_surface.py` 与命令管理工作区

## 背景

命令管理页在没有批量命令时原本只显示一行说明，用户必须回到上方工具条寻找“新建”。将 CTA 直接塞进
`controllers/commands.py` 会把 UI 结构与业务 action 混在一起；同时，空结果表的 header 和状态带会占用有限的
workspace viewport，使空态卡片即使创建成功也可能被裁切。

## 决策

- `CommandBatchEmptyState` 是独立的 presentation surface，负责 glyph、说明文案、可访问元数据和“新建批量命令”
  CTA；它只发出无参数 `new_requested` intent，不读取 ViewModel/domain/transport。
- `controllers/terminal.py` 显式把 `new_requested` 接到 `commands.py` 已存在的 `new_command_batch_action`；dialog parent、
  ViewModel 保存语义和错误入口保持在原 owner，不新增第二套 action。
- `ActionRailButton` 从 `terminal_surface.py` 抽为共享原生 `QPushButton` 装饰组件；`TerminalActionButton` 保留兼容导出，
  但只指向该实现。底部 rail 只消费共享 `(phase, animated)` 与 `ThemeSpec.accent`，不拥有 timer 或业务状态。
- `commands.py` 是唯一的 batch presentation projection owner：
  - `batch is None`：隐藏 `CommandBatchSurfaceLabel` 与空 `QTableWidget`，显示完整 `CommandBatchEmptyState`；
  - 选中 batch：恢复 status surface 与只读结果表，隐藏空态；
  - snapshot、selection、执行顺序、connection controls 和 batch gate 不因视觉切换改变。
- `connection.py` 同时 gate 顶部“新建”和空态 CTA；历史回放、批量执行和 BLE notification pending 期间两者都不可用。

## 被拒方案

### 在空态组件中直接调用 `ViewModel.save_command_batch`

拒绝：会让 presentation surface 持有业务依赖，绕过 `commands.py` 的 action owner、dialog parent 和现有 gate。

### 只隐藏结果表，保留空 status surface

拒绝：状态带在 empty projection 中只重复说明，仍会把 CTA 下半部分推到有限 viewport 外；空态卡片应完整承担首步引导。

### 为 CTA 或 glyph 添加控件级 QTimer

拒绝：会产生第二个动效时钟和重复生命周期；现有 `MotionController` 已能统一驱动终端、批量、主题和路由装饰。

## 后果

- 首次进入命令管理页即可看到明确的下一步操作，减少手动寻找控件的成本。
- 空态和结果表互斥占用同一工作区区域，避免空表 header 或重复状态带造成视觉裁切。
- 新增两个稳定变化边界文件，通用 action rail 不再与终端 surface 强耦合；代价是 lifecycle fan-out 增加一个
  presentation consumer，但没有新增时钟或业务状态。

## 验证与限制

- `scripts/check.ps1`：源码行数、theme token audit、Ruff 全部通过（126 files <= 1000）。
- Qt offscreen：空态 component signal、shared frame、空态/有 batch 的 status/table 互斥可见性、结果行恢复、
  pause/hide/close 静态回退均通过。
- 980/1180：空态卡片高度 98、CTA 宽度 106，按钮在 workspace viewport 内完整可见；三主题 near-white pixel count 均为 0。
- offscreen 环境仍提示 PySide6 fonts directory 缺失；中文方框不代表 Windows 运行时字体结论。未运行持续 GUI、EXE 启动、
  HIDPI/读屏、真实传输、OTA/RTT/J-Link 或硬件验收。

嵌入式适用性：本 ADR 仅涉及 Python/PySide6 presentation，不涉及 MCU、BSP/HAL/C++、RTOS 或厂商资料；不宣称
MISRA、ISO 26262、WCAG、认证或硬件合规。
