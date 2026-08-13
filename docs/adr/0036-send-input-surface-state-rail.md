# ADR 0036：发送输入框状态轨与原生编辑契约

日期：2026-08-10  
状态：已接受（UI-1.49）

## 背景

发送区已经有 `sendControlBand` 和 `sendState` 的既有状态投影，但输入框本身仍是通用静态 `QLineEdit`。
用户无法仅通过输入框边界快速区分等待连接、可发送、处理中和历史回放上下文；直接改写 placeholder 或把发送状态塞进文本会
损害原生编辑体验和无障碍表达。

## 决策

- 新增 `presentation/send_input_surface.py:SendInputSurface`，继续继承原生 `QLineEdit`，只在 `super().paintEvent()` 完成后
  绘制底部状态轨和焦点反馈。
- `connection.py` 继续作为 `send_band_state` projection owner，向 input surface 投影已有
  `blocked/waiting/ready/busy/history` 值；surface 不读取 ViewModel，不改变 `setEnabled`、send button gate 或 hint 文案。
- `terminal.py` 只创建该子类；`lifecycle.py` 将它加入唯一 `MotionController` frame/stop fan-out。
- ready/busy 且有焦点时，轨道可在共享 frame 下显示低频 pulse；低动效、暂停、隐藏、最小化和关闭时静态显示。blocked/waiting/history
  仍有静态语义色，不伪造进度或成功。

## 原生契约与替代方案

- 保留 QLineEdit 文本、选择、光标、焦点、剪贴板、回车、placeholder、AccessibleName/Description、QSS 和键盘行为；不自绘文字，
  不接管事件过滤器。
- 不使用独立 `QTimer`、QPropertyAnimation 或业务状态；不把 pulse 作为发送结果或 wire progress。
- 不用 event filter/overlay 兄弟控件，避免焦点树、Tab 顺序和命中区域分裂。

## 依赖与生命周期

```text
connection.update_connection_controls()
        └─ send_band_state → SendInputSurface.set_surface_state()
MotionController.frame_changed → lifecycle._motion_surfaces → set_frame/stop
```

`SendInputSurface` 只依赖 presentation Qt/theme boundary；controller 和 transport 不反向持有该 widget。

## 验证

```text
scripts/check.ps1       PASS 131 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff
compileall               PASS src
UI149_FIELD              PASS three themes; five states; native QLineEdit subclass; text=AA 55 preserved
UI149_PIXEL              PASS state screenshots and no theme white fallback in rendered themed host
UI149_SEND_BAND          PASS three themes; themed sendControlBand; exact_white=0; native surface type retained
```

离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论；未运行持续 GUI、Windows 原生读屏/HIDPI、真实传输、硬件、
签名或正式发行验收。未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

嵌入式 C/C++ 适用性：N/A。本项目是 Python/PySide6 Windows 桌面应用，无 MCU、固件或适用公共 vendor profile；不声称任何认证合规。
