# ADR 0026：终端空态导航意图与工作区 owner

状态：已接受（2026-08-10）  
范围：`presentation/terminal_surface.py`、`controllers/bootstrap.py`、`controllers/workspace_runtime.py`

## 背景

终端没有 RX 数据时，用户需要一个明确的下一步入口；但空态 renderer 不应读取连接配置、触碰
transport 或自动建立会话。此前空态只有文案，用户必须自己寻找连接 Tab，空态的可用性不足。

## 决策

- `TerminalEmptyState` 增加无参数 `connection_requested` presentation signal，并在 idle、waiting、
  transition 状态显示主题化 `primaryButton` CTA；paused、history 状态隐藏 CTA，避免暗示错误动作。
- CTA 只发出“请求打开链路连接页”的意图，不执行连接、不改变 ViewModel/session/transport 状态，
  不读取密钥、端点或设备句柄。
- `bootstrap.py` 用显式 `partial` 把信号接到 `workspace_runtime.select_workspace_tab(window, 0)`；
  `workspace_runtime.py` 是唯一导航 owner，负责 closing fence、Tab index 边界、切换首 Tab 和恢复焦点。
- 真实 `QTabWidget` 仍是可访问性和顺序的权威；CTA 自身提供 accessible name/description/tooltip，
  route beacon 仍是鼠标透明、不可聚焦的装饰层。
- 复用现有 `primaryButton` 语义样式，不新增颜色 token、timer、业务状态或主题分支；空态背景和
  CTA 必须在 980×680/1180×780 下保持完整，不出现近白回退。

## 取舍与被拒方案

### 让空态直接调用连接动作

拒绝：这会让 presentation surface 拥有 transport/session 副作用，并绕过用户确认、连接 gate 和
错误投影。导航与连接保持两个 owner，用户仍明确点击真正的“连接”按钮。

### 让 `TerminalEmptyState` 直接持有 `QTabWidget`

拒绝：会把 terminal surface 与 workspace 结构耦合，破坏高内聚边界。无参数 signal 让 surface
只表达意图，bootstrap 负责一次性装配。

### 增加一个控件级动画 timer

拒绝：CTA 使用现有 QSS 和共享生命周期；没有必要为了按钮增加第二套动效时钟。

## 后果

- 空态具备可发现、可键盘聚焦和可读屏的下一步动作，点击后能回到连接配置上下文。
- workspace navigation owner 增加一个小的显式 action contract，但连接业务路径不变。
- paused/history 空态仍是只读信息 surface；未来增加其他入口必须先定义状态语义，不得复用该 CTA
  伪装成 OTA、调试或自动连接动作。

## 验证与限制

- `scripts/check.ps1`：通过；124 个 Python 源文件均不超过 1000 行，主题 token 审计和 Ruff 通过。
- Qt offscreen inline smoke：980×680、1180×780，idle/waiting/transition/paused/history 可见性、
  实际按钮 click、首 Tab/焦点、会话状态不变、终端无横向滚动、三主题近白像素为 0：通过。
- 模块 import smoke：123 个包模块导入，`QApplication.instance() is None`：通过。
- offscreen 环境缺少 PySide6 fonts directory，截图中的中文方框只属于该环境限制，不代表 Windows
  运行时字体结论；未运行持续 GUI、EXE 启动、真实传输、RTT/J-Link、OTA 或硬件验收。

嵌入式适用性：本 ADR 仅涉及 Python/PySide6 presentation，MCU/BSP/HAL/C/C++、RTOS 与厂商资料
不适用；不宣称 MISRA、ISO、WCAG、认证或硬件合规。
