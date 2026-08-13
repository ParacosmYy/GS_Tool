# ADR 0035：QStatusBar 状态 footer signal surface

日期：2026-08-10  
状态：已接受（UI-1.48）

## 背景

SerialForge 已经在顶栏、连接带和终端观测带显示状态，但底部 native `QStatusBar` 只有纯文字 message。
实时 RX activity、连接阶段和应用错误在底部没有统一的低干扰视觉锚点；直接修改 message 文案或新增独立业务状态都会
破坏状态栏原生可读性和分层边界。

## 决策

- 新增 `presentation/status_footer_surface.py:StatusFooterSurface`，作为 `QStatusBar.addPermanentWidget()` 的永久装饰子控件。
- 控件固定为 116×18，鼠标透明、不可聚焦、空 accessibility；native status message 仍是唯一可访问和可读的权威文案。
- lifecycle 显式投影已有 `SessionState`、`ErrorInfo` 是否存在和短时 RX activity；控件只消费这些投影与共享
  `MotionController` `(phase, animated)`，不读取 ViewModel、不复制 status 文本、不解释为吞吐/进度。
- animation 只在共享 clock 出帧且 activity 为 true 时显示移动 pulse；reduced-motion、显式暂停、隐藏、最小化和关闭时保持
  静态轨道。fault 通过静态 error accent 表达，清除时由 lifecycle 显式复位。

## 放弃的选项

- 不替换 native `QStatusBar`，避免改变 Qt status message、布局、键盘/读屏和窗口边界语义。
- 不在 footer 内创建 `QTimer`、保存 ViewModel、统计字节或伪造百分比；所有事实继续来自既有状态与信号。
- 不把装饰图形作为唯一状态表达；文字 message 和既有错误栏保持权威。

## 依赖与生命周期

```text
SessionState / ErrorInfo / RX activity
             ↓ lifecycle projection
QStatusBar message (authoritative text)
             └─ StatusFooterSurface(state/fault/activity)
                 ↑ MotionController.frame_changed
```

`bootstrap.py` 只创建并挂载 surface；`lifecycle.py` 负责 `set_state/set_fault/set_activity` 及统一 frame/stop fan-out。
surface 不持有 controller，不引入 application/domain/infrastructure 依赖。

## 验证

```text
scripts/check.ps1          PASS 130 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff
UI148_IMPORT_COMPILE       PASS 67 presentation modules; qapplication_instance=False; AST parse
UI148_STATUSBAR            PASS star_trail/moonlit_ocean/sakura_night; 420x90; exact_white=0
UI148_SURFACE              PASS active/fault_static/closed_static; 116x18; all three themes
```

离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论；未运行持续 GUI、EXE 启动、Windows 原生读屏/HIDPI、
真实 UART/BLE、硬件、签名或正式发行验收。未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

嵌入式 C/C++ 适用性：N/A。本项目是 Python/PySide6 Windows 桌面应用，无 MCU、固件或适用公共 vendor profile；不声称任何认证合规。
