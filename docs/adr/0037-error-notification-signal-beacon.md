# ADR 0037：错误通知 signal beacon 与清除复位

日期：2026-08-10  
状态：已接受（UI-1.50）

## 背景

errorBar 已经由 `ErrorInfo` 控制可见性，显示结构化错误摘要/详情并提供清除按钮，但错误出现时只有文字表面，没有与
SerialForge 其他状态带一致的几何告警锚点。直接改写错误文案或引入第二个错误状态源会破坏现有错误传播和无障碍契约。

## 决策

- 新增 `presentation/error_surface.py:ErrorSignalSurface`，固定 28×28，鼠标透明、不可聚焦、空 accessibility，放在 errorBar
  文字左侧；它只绘制静态 error ring/cross 和共享 frame 下的低干扰 pulse。
- `lifecycle.on_error_changed()` 在既有 `ErrorInfo` 非空/清除分支显式调用 `set_active(True/False)`；error label、detail tooltip、
  clear button 和 container visibility 不变。
- `lifecycle._motion_surfaces()` 统一转发 frame/stop；低动效、暂停、隐藏、最小化、关闭时 pulse 停止但 active beacon 保留静态回退。

## 放弃的选项

- 不自绘错误文字、不把 beacon 加入 accessibility tree、不改变 `ErrorInfo`、错误栏清除或 statusBar message 语义。
- 不创建 timer、闪烁线程或事件过滤器；不把 pulse 频率解释为严重性、数量、重试次数或传输进度。
- 不让 error surface 直接读取 ViewModel/domain/application；lifecycle 仍是唯一 presentation projection owner。

## 依赖与生命周期

```text
ErrorInfo changed
      ↓ lifecycle.on_error_changed()
error label/container + clear button (authoritative)
      └─ ErrorSignalSurface.set_active()
MotionController.frame_changed → lifecycle._motion_surfaces → set_frame/stop
```

`ErrorSignalSurface` 只依赖 presentation Qt/theme boundary；bootstrap 的 error bar builder 只负责装配，清除行为仍走现有 ViewModel slot。

## 验证

```text
scripts/check.ps1       PASS 132 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff
compileall               PASS src
UI150_ERROR              PASS three themes; active/static/stop/clear; error label and clear button retained
UI150_PIXEL              PASS three themes; errorBar 424x56 exact_white=0
```

离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论；未运行持续 GUI、Windows 原生读屏/HIDPI、真实传输、硬件、
签名或正式发行验收。未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

嵌入式 C/C++ 适用性：N/A。本项目是 Python/PySide6 Windows 桌面应用，无 MCU、固件或适用公共 vendor profile；不声称任何认证合规。
