# ADR 0034：刷新与扫描按钮的共享忙碌信号轨

日期：2026-08-10  
状态：已接受（UI-1.47）

## 背景

UART 端口刷新和 BLE 扫描已有真实的 `discovery_busy` / `ble_scan_busy` 状态、按钮禁用和“刷新中…/扫描中…”文案，
但运行中只改变文字，缺少持续的视觉反馈。用户无法快速区分“操作仍在执行”和“界面没有响应”。项目已经有唯一的
`MotionController` 与 `ActionRailButton` 装饰边界，适合复用而不是新增控件计时器。

## 决策

- 在 `presentation/action_surface.py` 新增 `BusyActionButton`，仍继承原生 `QPushButton`，保留点击、焦点、disabled、QSS、
  AccessibleName/Description 和现有信号接线。
- `connection_builder.py` 只把 `_refresh_button` 与 `_ble_scan_button` 换成该 presentation 子类；不改变文本、slot、布局或业务动作。
- `connection.py` 继续作为 busy projection owner，仅调用 `set_busy(discovery_busy/ble_scan_busy)`；按钮不读取 ViewModel。
- `lifecycle.py` 把两个按钮接入既有 `_motion_surfaces` frame fan-out；按钮使用共享 `(phase, animated)` 绘制底部 activity rail。
- busy rail 在动画开启时沿轨移动，在低动效、暂停、隐藏、最小化和关闭时退回静态提示；不表示百分比、字节或真实扫描进度。

## 放弃的选项

- 不为每个按钮创建 `QTimer`、线程或动画对象；这会违反共享动效时钟和生命周期边界。
- 不在按钮中启动扫描/刷新、不读取 ViewModel、不复制 busy 状态源；controller 仍是事实投影 owner。
- 不替换为自绘按钮或 overlay；原生 QPushButton 的键盘、焦点、点击和 accessibility 语义更重要。
- 不用文字轮播/Unicode spinner 替代 painter，避免缺少字体、文案抖动和屏幕阅读器误读。

## 依赖与生命周期

```text
ViewModel busy signal
        ↓
connection.update_connection_controls
        ├─ button text/enabled/AccessibleDescription
        └─ BusyActionButton.set_busy(bool)
                         ↑
MotionController.frame_changed → lifecycle._motion_surfaces
```

`BusyActionButton` 只持有短暂 presentation phase/animated/busy flags，不持有业务 DTO；`stop()` 由已有 lifecycle 统一调用。

## 验证

```text
scripts/check.ps1       PASS 129 files <= 1000; theme audit; Ruff
UI147_BUSY              PASS three themes; busy text, disabled state, themed rail
UI147_REDUCED           PASS busy remains visible with animated=False/static rail
UI147_LAYOUT            PASS 980x680; no exact-white fallback or layout expansion
UI147_A11Y              PASS native button type and existing accessible labels/descriptions retained
```

离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论；未运行持续 GUI、真实扫描/端口刷新、Windows 键盘/读屏、
硬件或正式签名发布验收。未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

嵌入式 C/C++ 适用性：N/A。本项目是 Python/PySide6 Windows 桌面应用，无 MCU、固件或适用公共 vendor profile；不声称任何认证合规。
