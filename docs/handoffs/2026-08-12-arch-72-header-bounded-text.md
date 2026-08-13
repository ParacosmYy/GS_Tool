# ARCH-72 / UI-1.145 Header bounded status text

日期：2026-08-12  
范围：顶部 header 长状态文案的布局边界与可恢复信息。

## 结果

`presentation/bounded_text_label.py` 新增 `BoundedTextLabel`。它继承现有 `QLabel`，只负责：

- 依据当前可用宽度执行单行右侧省略；
- 在 resize、字体变化和 style 变化后重新计算可见文本；
- 通过 `full_text`、tooltip 和 accessible description 保留完整值；
- 提供 bounded size hint 和可收缩 minimum width，避免长文本成为父布局的最小宽度。

`workspace.py` 将 header 的连接上下文、数据来源、连接状态替换为该组件；
`HeaderChromeBindings` 的 typed wiring 反映新控件类型。`lifecycle.py` 继续调用原有 `setText()`，
组件内部吸收显示省略，不复制 SessionState、来源判断或 ViewModel 状态。

本轮没有新增 QTimer、动画、事件总线、业务状态、连接动作、主题状态或 OTA/debug 依赖；
120Hz shared MotionController、workspace focus 单一主过渡、reduced-motion、Tab/accessibility、
三主题和 980/1240 响应式边界保持。

## 验证证据

```text
BOUNDED_LABEL_PASS narrow=elided/full_text+tooltip+accessible preserved wide=full
ARCH72_HEADER_PASS status=(13,60,496,46) motion=(517,60,165,46) theme=(690,60,245,46)
ARCH72_SOURCE_METADATA_PASS source value included in tooltip and accessible description
ARCH72_CTA_FOCUS_PASS focus=True lower_surfaces_hidden tabs_max=16777215
ARCH72_MATRIX_PASS star_trail/moonlit_ocean/sakura_night × 980/720/1240/820 × 4 tabs
ARCH72_MOTION_PASS 120 PreciseTimer [8,8,9,8,8,9,8,8,9,8,8,9]
scripts/check.ps1: pass
source line limit: pass (166 files <= 1000)
theme token audit: pass (3 themes, 22 semantic tokens, 19 selectors, legacy_qss_literals=0)
```

视觉截图使用 `configure_application_font()` 选择本机 `Microsoft YaHei UI`，确认长端点、来源和
会话状态在顶部状态簇内省略且不与动效/主题控制重叠。离屏运行仍报告 PySide6 缺少 bundled
fonts directory 的既有 warning；这不代表 Windows 系统字体缺失。真实 GUI、HIDPI、读屏、设备、
J-Link/RTT、OTA、安全后端和硬件验收未执行。

## 架构师与审查记录

本轮按项目约束调用 Luna/max 只读架构师，线程在时限内超时后关闭，未将超时当作独立结论；
父代理基于真实组合根、调用链、生命周期、布局几何和行为保持型简化检查完成集成审查。
该变更是 Python/Qt presentation-only，不涉及 embedded C/C++、MCU/BSP/HAL/RTOS/ISR/DMA、
OTA 固件、Flash/NVM、功耗或电机控制；公共 MCU 厂商源适用性为 N/A，无需声明 MISRA、ISO 26262
或任何认证合规性。

## 包交付

onefile 产物使用 `local-arch-72` 生成，并覆盖仓库根目录 `SerialForge.exe` 与
`SerialForge-latest.exe`。三者 byte-identical：

```text
size: 47,980,962 bytes
SHA256: 6925FDEAD6A98131F5ADD20BD46E9082DF5532AB3C733EAD4A67B2E2F2FD162C
archive listing SHA-256: C9C66CBFDA9102E0A13C6A95DD168ECDF3DC358009DFA125EBCB9D4CF636A732
provenance: pass; source revision local-arch-72
signature: NotSigned; release_eligible=false; hardware_acceptance=not_run
```
