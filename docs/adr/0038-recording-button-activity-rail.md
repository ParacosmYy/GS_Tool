# ADR 0038：原始记录按钮 activity rail

日期：2026-08-10  
状态：已接受（UI-1.51）

## 背景

原始记录流程已经通过 `RecordingState.STARTING/ACTIVE/STOPPING/ERROR/IDLE` 提供明确状态，按钮文字和 `recordState` 标签也会
随 snapshot 更新，但按钮本身没有进行中的视觉反馈。项目已有用于 UART discovery/BLE scan 的 `BusyActionButton`，其 native
QPushButton 契约、共享 MotionController 和静态回退边界正好适用于同类异步 presentation action。

## 决策

- `terminal.py` 将原始记录按钮实例化为既有 `BusyActionButton`，不改变 object name、signal、click handler、文字或 accessible wiring。
- `terminal_runtime.on_recording_changed()` 只把已有 `RecordingState` 映射为 `set_busy(True)`（STARTING/ACTIVE/STOPPING）或 false（STOPPED/ERROR）；
  `recordState` 文案/状态仍由原逻辑负责。
- `lifecycle._motion_surfaces()` 纳入 `_record_button`，复用唯一 MotionController；低动效、暂停、隐藏、最小化、关闭时保留静态轨。
- `BusyActionButton` 的职责描述扩展为已有异步 presentation action，而不是增加第二套 recording renderer。

## 放弃的选项

- 不新增 `RecordingActionButton` 或第二份 painter；复用现有 shared `_paint_signal_rail()`，保持高内聚、低耦合。
- 不读取 ViewModel、创建 timer、修改 recording worker、写盘语义、记录吞吐或伪造百分比。
- 不将 button text/status label 替换为 spinner；文字和无障碍仍是状态事实表达。

## 依赖与生命周期

```text
RecordingState snapshot
        ↓ terminal_runtime.on_recording_changed()
record button text/description + BusyActionButton.set_busy()
        ↑ MotionController.frame_changed → lifecycle._motion_surfaces
```

`BusyActionButton` 只持有 presentation busy flag/phase/animated flag；recording controller 仍拥有状态 projection，application/domain
不依赖 Qt widget。

## 验证

```text
scripts/check.ps1       PASS 132 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff
compileall               PASS src
UI151_RECORD             PASS idle/starting/active/stopping/error; native BusyActionButton; text/accessibility retained
UI151_MOTION             PASS shared frame; busy rail; static/reduced stop fallback
```

离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论；未运行持续 GUI、真实 recording worker/文件写入、硬件、签名
或正式发行验收。未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

嵌入式 C/C++ 适用性：N/A。本项目是 Python/PySide6 Windows 桌面应用，无 MCU、固件或适用公共 vendor profile；不声称任何认证合规。
