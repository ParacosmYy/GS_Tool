# ADR-0155：命令空态作为可伸缩工作区画布

日期：2026-08-12

状态：已采用（ARCH-104 / UI-1.177）

## 背景

命令管理页在没有批量命令时，控件和空态卡片集中在滚动页左上方，剩余高度没有明确的
presentation owner。视觉上容易被感知为组件挤在一起，或下面留下一块无语义空白。

## 决策

- `controllers/command_workspace_builder.py` 继续拥有命令页布局，把空态作为唯一可伸缩的
  vertical slot；删除 root trailing stretch，避免出现两个竞争的空白分配者。
- `command_batch_empty_state.py` 只负责 presentation 排版：使用 expanding vertical policy、
  垂直居中的 glyph，以及 copy 区前后 stretch，让标题、说明和 CTA 在画布中形成稳定焦点。
- 批量命令是否存在、空态的 visible 状态和 snapshot 语义仍由 `controllers/commands.py` 投影；
  空态组件不读取 ViewModel、不改变命令执行、不改变滚动 owner。
- 共享动效保持窗口级唯一 `MotionController`、`TARGET_HZ=120` 与 8ms `PreciseTimer` target。
  本轮验证过的 1ms/elapsed-budget 试验没有改善真实 Qt 主循环的回调 cadence，并增加唤醒压力，
  因此撤回，不进入交付代码。

## 验证与取舍

静态门禁 `scripts/check.ps1` 通过；三主题、980×720/1240×820、四个 workspace 的真实 Qt
offscreen 组合检查无 sibling overlap，command empty state 在 focus 模式下占据可伸缩画布，
overview 模式仍由既有 scroll owner 承载。最终截图见 `serialforge-arch104-final-*.png`。

真实 `app.exec()` 采样得到 `TARGET_HZ=120`、`interval=8ms`、`frames=116`、均值 `8.536ms`、
有效约 `117.16Hz`；这是当前 offscreen scheduler 证据，不是显示器精确 120fps 承诺。暂停、低
动效和隐藏状态均观察到 timer inactive。未运行 EXE 启动、真实高刷新显示器、HIDPI、硬件连接、
OTA/RTT 实连或签名验收。

本轮为 Python/PySide6 presentation-only，embedded C/C++ public-source applicability 为 N/A；
不作 MISRA、ISO 26262、ASIL、ASPICE 或认证声明。多次架构师调用在等待窗口内超时关闭，未形成
外部结论；父代理完成 correctness、readability、architecture、security、performance 五轴
review 与行为保持简化评估。
