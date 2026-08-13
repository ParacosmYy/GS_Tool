# ADR 0045：发送表单上下文摘要与 canonical payload rail

日期：2026-08-10

## 状态

已接受，UI-1.58。

## 背景

发送栏已经有连接可用性和发送状态反馈，但用户仍需要在 mode、输入内容、CRLF 和真实 wire payload 之间来回推断。
坏 Hex 或超过 `CommandEntry` 64 KiB 上限的内容如果只在点击发送后暴露，反馈滞后且容易误以为发送按钮或链路异常。

## 决策

在 `presentation/` 增加 `SendContextSurface`，并由 `controllers/send_context.py` 将原生发送控件的当前值投影为不可变
`SendContextProjection`。摘要显示 UTF-8/HEX、payload 字节数、wire payload 字节数、CRLF 和格式/容量错误；底部 rail
只表达当前输入上下文，不表达设备吞吐、发送进度或设备已接收事实。

`project_send_form()` 复用 domain 的 `MAX_COMMAND_PAYLOAD_BYTES`，使提示与 `CommandEntry` canonical 约束一致。原生
`QComboBox`、`QLineEdit`、`QCheckBox` 的编辑、焦点、键盘、无障碍和发送行为保持不变；mode/CRLF 变化接入既有输入刷新回调，
`connection.py` 仍拥有 connected/batch/history/BLE/TCP 的发送 gate。

## 边界与后果

- `send_context_surface.py` 只负责 projection DTO、纯表单投影和 QLabel 绘制，不触碰 ViewModel、transport、发送动作或 payload 持久化；
- `controllers/send_context.py` 是表单到 surface 的唯一 projection owner；`terminal.py` 只负责装配，`connection.py` 只保留发送 gate；
- surface 复用 `lifecycle.py` 的共享 `MotionController` frame/stop，不创建控件级 timer；低动效、暂停、隐藏、最小化和关闭都有静态回退；
- 默认 QSS 与主题 override 显式覆盖 empty/ready/invalid 三态，避免系统 palette 白色回退；
- 为了契合 980px 响应式布局，摘要采用有限宽度和短文案，完整解释放在 `AccessibleDescription`/tooltip；
- 这只是 presentation 反馈，不替代未来 application 层的发送结果、设备 ACK、吞吐或安全策略。

## 放弃的方案

- 在 `send_current()` 中增加 UI 文案：会把 presentation 反馈和发送动作耦合，并且只覆盖提交后的错误路径；
- 让 `SendContextSurface` 直接读取 `MainWindow`/ViewModel：会扩大共享状态容器，破坏 surface 的高内聚边界；
- 为输入 rail 创建独立 QTimer：会绕开统一 MotionController 生命周期，增加低动效/隐藏/关闭竞态。

## 验证要求

覆盖空输入、合法 HEX+CRLF、坏 Hex、canonical payload 超限、共享 frame、低动效停止、三主题 1180×780 near-white=0、
`scripts/check.ps1`、compileall 和 onefile/root EXE provenance。offscreen 环境的 PySide6 fonts 目录提示不能作为 Windows 字体结论。
