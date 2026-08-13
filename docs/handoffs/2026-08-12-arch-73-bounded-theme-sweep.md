# ARCH-73 / UI-1.146 Bounded theme sweep

日期：2026-08-12  
范围：主题切换动画的视觉遮挡与生命周期边界。

## 结果

实测旧主题 sweep 在 1240×820 根窗口内为 `180×797`，会覆盖 terminal/send 和下方数据 surface。
本轮只修改 `presentation/theme_transition.py`：

```python
sweep_height = min(_THEME_SWEEP_HEIGHT, max(1, target.height()))
```

`_THEME_SWEEP_HEIGHT = 220`。root opacity fade、180ms sweep、既有 `stop_theme_transition()`、
finish cleanup、reduced-motion/暂停/隐藏/最小化/关闭和快速主题切换语义不变；没有新增 timer、
MotionController、业务状态、事件总线、主题状态或外部依赖。overlay 继续透明鼠标，不改变 palette
生效时机和连接/协议/记录语义。

## 验证证据

```text
THEME_TRANSITION_BASELINE overlay=180x797
ARCH73_SWEEP_BOUNDS_PASS overlay=(-180,0,180,220) root_effect=0.86 sweep=Running
ARCH73_SWEEP_SETTLED_PASS overlay=None animation=None
ARCH73_HEADER_REGRESSION_PASS status=(13,60,496,46) motion=(517,60,165,46) theme=(690,60,245,46)
ARCH73_THEME_LIFECYCLE_PASS sweep_height<=220 settled=True
ARCH73_MATRIX_PASS 980/720 and 1240/820 × 4 tabs
ARCH73_CTA_PASS state=closed tabs_max=350
ARCH73_MOTION_PASS 120 PreciseTimer [8,8,9,8,8,9,8,8,9,8,8,9]
ARCH73_GEOMETRY_PASS 980x720 overlay=180x220 terminal_top=413 send_top=587; 1240x820 terminal_top=483 send_top=687
ARCH73_GEOMETRY_CLEANUP_PASS
scripts/check.ps1: pass
source line limit: pass (166 files <= 1000)
theme token audit: pass (3 themes, 22 semantic tokens, 19 selectors, legacy_qss_literals=0)
```

首轮组合根脚本把隐藏页面的 `settingsScroll` 横向范围误判为失败；诊断显示隐藏页 viewport 为
88px，当前可见 scroll area 横向 maximum 均为 0，修正验证前提后完整矩阵通过。离屏环境仍有
PySide6 bundled fonts directory warning；已通过 `configure_application_font()` 选择系统字体。
真实 GUI、HIDPI、读屏、设备、OTA、安全后端、J-Link/RTT 和硬件验收未执行。

## 架构师与审查记录

本轮按项目约束调用 Luna/max 只读架构师，首轮线程在限定窗口内超时后关闭，未把超时当作独立结论；
随后独立 Luna/max 只读审查返回 `risk=medium`，确认 stop/finish 的 active-animation 身份检查、
两条动画停止、opacity 恢复和 overlay 延迟删除路径，同时指出 root 顶部父级与 queued cleanup 的残余风险，
建议未来有明确 chrome 容器时再收敛父级。父代理用两个窗口尺寸的真实几何回归确认 terminal/send 均在
sweep 下方，并完成 theme owner、effect/animation 生命周期、遮挡范围、五轴质量和行为保持型简化复核。
本轮为 Python/Qt presentation-only，不涉及 embedded C/C++、MCU/BSP/HAL/RTOS/ISR/DMA、
OTA 固件、Flash/NVM、功耗或电机控制；公共 MCU 厂商源适用性为 N/A，不声明 MISRA、ISO 26262
或任何认证合规性。

## 包交付

onefile 使用 `local-arch-73` 生成，并覆盖根目录 `SerialForge.exe` 与 `SerialForge-latest.exe`；
三者 byte-identical：

```text
size: 47,981,132 bytes
SHA256: DBD554999C65C18DDDC66643EA336CD90EFDF7B41DC43629194BAA044752F886
archive listing SHA-256: C9C66CBFDA9102E0A13C6A95DD168ECDF3DC358009DFA125EBCB9D4CF636A732
provenance: pass; source revision local-arch-73
signature: NotSigned; release_eligible=false; hardware_acceptance=not_run
```
