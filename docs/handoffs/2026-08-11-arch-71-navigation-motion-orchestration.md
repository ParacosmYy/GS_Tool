# ARCH-71 / UI-1.144 workspace 导航动效编排交接

日期：2026-08-11  
父代理：Codex  共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`

## 本轮目标

解决 Tab 导航时 workspace 同一表面同时运行 220ms focus 几何过渡和 180ms page opacity
fade 的叠加感。保留 120Hz 共享动效时钟和同一 focus mode 内的轻量页面反馈，让模式变化
只有一个清晰的主过渡。

## 实现

`workspace_runtime.animate_workspace_transition()` 现在消费既有
`window._workspace_focus_transition` 生命周期：如果 focus geometry group 有效，就先通过
既有 `stop_workspace_transition()` 清理旧 page effect，然后跳过新的 page fade。Tab 仍处于
同一 focus mode 时不命中该条件，原有 180ms page fade 保持。

本轮只修改 workspace runtime，不新增 timer、event bus、业务状态、splitter、ViewModel/
application 依赖或第二套动画策略；focus transition、reduced-motion、隐藏/最小化/关闭
静态回退和 MotionController 仍由原 owner 管理。

此外，focus 入口会先隐藏 live observation、terminal、send 三块并清零 maximum-height，只动画
tabs 扩展；返回总览仍从 0 高度 bounded reveal，避免过渡帧把内部控件压扁。

## 真实组合根验证

```text
NAV_FOCUS_PRIMARY_PASS focus_active=True page_fade=None
NAV_SAME_MODE_FADE_PASS focus_active=None page_fade_active=True
NAV_RETURN_PRIMARY_PASS focus_active=True page_fade=None
NAV_REDUCED_MOTION_PASS
MOTION_INVARIANT_PASS 120 PreciseTimer 56
FOCUS_ENTER_NO_SQUEEZE_PASS [0, 0, 0] 350 True
FOCUS_ENTER_SETTLED_PASS 514 [0, 0, 0]
FOCUS_RETURN_REVEAL_PASS [0, 0, 0] True
FOCUS_RETURN_SETTLED_PASS 115 [96, 169, 98]
FOCUS_RAPID_REVERSAL_PASS
compileall: pass
ruff: pass
```

离屏环境仍报告既有 PySide6 fonts directory warning，不代表 Windows 系统字体缺失。真实
Windows GUI、HIDPI、读屏、设备 I/O、UART/网络/BLE/RTT/J-Link 和目标板硬件验收未执行。

## 复核与简化

- 架构师：Luna/max/Fast 只读线程已调用，但在时限内超时并关闭；未将超时视为通过，也未伪造报告。
- correctness：模式切换只保留 focus group，same-mode route 保留 page fade；focus 进入时下方 surface
  不参与高度收缩，返回时 bounded reveal；所有动画对象仍由既有 stop/finish owner 清理。
- architecture：`workspace_runtime` 是唯一导航动画 owner；不把编排逻辑放入 widget、bootstrap、
  ViewModel 或业务服务。
- readability：一个有效 transition guard 表达“主过渡优先”语义，复用已有清理函数，不引入新的状态枚举。
- performance：没有新增 QTimer、线程或 motion surface；120Hz/PreciseTimer/8-9ms cadence 不变，
  模式切换减少一次同区域 opacity effect 和三组无意义的收缩动画。
- security：无输入、密钥、网络、持久化或依赖变更。
- simplification：没有进一步抽 helper；当前条件只使用一次且直接邻接 page fade owner，避免为了少几行
  代码制造新的跨模块抽象。

## 包状态

`local-arch-71` onefile 已生成并覆盖根目录 `SerialForge.exe` 与 `SerialForge-latest.exe`。

```text
PACKAGE_ARCH71_FINAL                         pass
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
root-latest: SerialForge-latest.exe (byte-identical)
size: 47,978,127 bytes
SHA256: D3D73A8EFC8D4A1531C335CC1D5280C9F24A047A4484D4DBB523A1C1E715E5B6
archive listing SHA-256: FD2322723A63AD7B425D9F9B763D96DB2730E0C558B642548AF0BAEC86323969
provenance: pass; source revision local-arch-71
signature: NotSigned; release_eligible=false; hardware_acceptance=not_run
ROOT_EXE_OVERWRITE_PASS: canonical/root/root-latest size and SHA-256 identical
```

## Assurance

本轮只改 Python/Qt presentation，public vendor source applicability 为 N/A；不涉及 embedded C/C++、
MCU、vendor SDK、RTOS、ISR/DMA、OTA firmware 或硬件，不宣称 MISRA/ISO 26262/认证合规。未创建或
修改单元测试、mock、fixture、harness 或 test-only asset。
