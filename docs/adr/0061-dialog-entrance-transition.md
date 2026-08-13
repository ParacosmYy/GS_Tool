# ADR 0061：自定义对话框使用一次性入口淡入

日期：2026-08-10

状态：accepted

## 背景

批量命令编辑器和自定义连接 preset editor 已经拥有独立的主题、空态和校验层级，但打开时仍直接出现，缺少与主题切换、
连接面板过渡一致的轻量入口反馈。过渡必须保持调试工具的可读性和键盘路径，不能把 dialog 生命周期变成业务状态。

## 决策

新增 `presentation/dialog_transition.py` 作为两个自定义 dialog 的唯一过渡 owner。dialog 的 `showEvent` 调用
`start_dialog_transition()`，`hideEvent` 调用 `stop_dialog_transition()`；正常动效下创建一组短时
`QGraphicsOpacityEffect/QPropertyAnimation`，参数固定为 150ms、`0.90 -> 1.0`、`OutCubic`。它复用现有
`motion_policy.decorative_motion_enabled()`；低动效、显式暂停或环境 reduced-motion 时静态显示。

该模块只持有 dialog 上的临时 animation/effect 引用，完成或 hide 时恢复 opacity、解除 effect 并销毁 animation；已有
graphics effect 不覆盖。它不读取 ViewModel、不进入 draft/validation、focus、Tab/accessibility、尺寸或 native dialog 路径，
不创建常驻 timer、第二套 MotionController 或外部视觉资源。

## 未采用方案

- 不为所有 `QDialog` 建立全局基类或事件过滤器：native `QMessageBox`/`QFileDialog` 不需要该过渡，且全局 hook 会扩大耦合。
- 不把淡入接入 `MotionController`：这是单次生命周期过渡，不是环境信号场或业务进度，使用低频共享时钟会增加无关 fan-out。
- 不在 ViewModel 中保存 transition state：动画是 presentation effect，不应改变 draft、校验、连接或命令状态。
- 不覆盖已有 graphics effect：其它 owner 可能持有自己的视觉效果，局部 helper 遇到已有 effect 时保持原状。

## 验证

- 真实 `create_application()` + `create_main_window()` 的短时 Qt offscreen 组合根 vector：三套主题的两个自定义 dialog 均确认
  正常动效 animation active；reduced-motion 的 command/preset 与显式暂停的 preset 均确认 static；hide 后清理通过。
- `.venv\Scripts\python.exe -m compileall -q src`：pass。
- 独立质量复核代理 `019feba2-e2bb-7223-a7f4-5a4c3755d454` 在窗口内超时并关闭；未将超时写成通过，父代理完成 correctness、
  readability/simplicity、architecture、security、performance 五轴审查。
- 未启动完整 GUI/EXE、未运行 unit tests、未创建测试/夹具/模拟器、未接入真实设备；嵌入式 C/C++ 适用性：N/A。

