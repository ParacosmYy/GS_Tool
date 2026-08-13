# ADR 0129：终端/发送控件组合绑定边界

日期：2026-08-11  
状态：accepted / incremental migration

## 背景

UART、网络和 BLE 连接表单已经分别使用 typed wiring bundle。终端观察、原始记录、发送控制、
快捷命令入口和发送历史仍由多个 controller 直接读取 `MainWindow._*` Qt 字段，导致
terminal runtime、connection gate、commands、lifecycle 和 focus transition 共享隐式 facade。

## 决策

新增 `presentation/terminal_bindings.py` 中 frozen/slots 的 `TerminalControlBindings`，只保存
实时观察、终端画布、记录、发送、快捷命令和历史控件的 Qt 引用。

- `controllers/bootstrap.py` 在全部 terminal/send widget 构造完成后一次性组装 bundle；
- `terminal_runtime.py`、`connection.py`、`commands.py`、`command_selection.py`、`send_context.py`、
  `composition.py`、`lifecycle.py` 和 `workspace_focus_transition.py` 通过
  `terminal_bindings_for()` 消费；
- `_preview_buffer`、`_preview_render_pending`、`_preview_render_timer`、history/quick-command
  snapshot、recording/session state、MotionController、ViewModel、callbacks 和业务策略不进入 bundle；
- bundle 缺失时 projection 安全返回，发送路径显式报告初始化错误；
- 保留构造 owner 内的迁移期 `window._*` compatibility fields，禁止新消费者重新读取这些 facade。

## 结果与验证

`ARCH6U_TERMINAL_VECTOR_PASS 24` 覆盖 3 主题 × 980/1180 × 四工作区，确认 terminal/send
bundle 身份、发送上下文、Hex/CRLF 控件、可见 scroll page、exact-white=0 和 near-white=0；
compileall、Ruff 与动态 widget owner 审计通过。行为保持包括 Tab 顺序、Ctrl+Enter、暂停显示、
记录、历史加载、快捷命令、发送 gate、MotionController 和关闭生命周期。

`local-arch-6u` onefile 已通过 provenance verify，并覆盖 canonical、根目录 `SerialForge.exe` 与
`SerialForge-latest.exe`；三者均为 `47,969,870` bytes，SHA-256
`485AFE4B7A581E27154BFA1043290D872FDB862CA61F35219945D88FA72839A8`，archive listing SHA-256
`E01F1A9B8689B822A7806272966DEF1637F1F5F6828ED7EBC8F739E925816876`。

## 审查记录

架构师线程 `019ff129-d626-78b0-9852-fe743f2b7d99` 已调用但在限定窗口内超时关闭，未计为
独立通过；父代理完成 owner、依赖方向、状态隔离、Qt 生命周期、行为保持、可访问性、主题、
响应式、性能与简化审查。复用既有 typed accessor 模式，没有新增 timer、事件总线、后端依赖或
测试资产。

嵌入式 C/C++/固件适用性：N/A。本轮只有 Python/PySide6 presentation 变更；没有适用的公开
一手厂商目标资料、硬件操作或刷写动作，不作 MISRA/ISO/认证合规声明。
