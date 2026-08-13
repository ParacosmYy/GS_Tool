# ARCH-124 / UI-1.197 命令上下文带交接

日期：2026-08-12

## 交付范围

本轮将命令页的发送历史与批量命令选择收敛到 `presentation/command_context_band.py::CommandContextBand`。宽度足够时两个 cell 两列并排，
窄宽度时自动单列；批量命令的五个操作按钮保留在独立 action row。owner 只重排 builder 已创建的控件，不接管业务、ViewModel、signals、
bindings、transport、scroll owner、焦点/Tab 顺序或共享动效时钟。

`QLayout.SizeConstraint.SetNoConstraint` 允许响应式布局进入 compact mode；minimum/size hint 由 cell 的 Qt sizing contract 推导。
wrapper 没有 `role=surface`，没有新增 QSS/theme selector，避免上下文带出现嵌套卡片和白色视觉层。

## 架构与独立审查

- 架构师 `019ff5a5-cb5d-75b3-9fa6-c41bc2a66802`：APPROVE owner、API、边界。
- Terra 架构 follow-up `019ff5a8-f256-7002-b8fc-86de2dab2dd0`：APPROVE Qt size constraint、cell hint、无嵌套 surface 边界。
- Luna 架构 follow-up `019ff5b5-7c29-7fa3-9c27-14b251e0e951`：APPROVE compact/current-column height contract。
- 文案架构 follow-up `019ff5c0-2320-7ec1-99d1-2b7d90444be1`：APPROVE docstring-only 修正。
- 独立代码审查 `019ff5bc-af7c-7f91-bc2d-891292e96db0`：`APPROVE WITH ADVISORIES`；无 Critical/Required。
- 独立简化审查 `019ff5bc-b25e-7743-9a2b-f0ae71f185f8`：无必须简化项；仅记录可选的文案和极端本地化/HIDPI 后续关注。

## 验证证据

- 几何：compact minimum width `160px`；wide threshold `330px`；320/329px 单列，330px 起双列；style relayout 后列数稳定。
- workspace：三主题 `star_trail`、`moonlit_ocean`、`sakura_night`；980×720 与 1180×820；band `height=92px`、两列、focus nodes `10`；
  cells 无重叠，accessible name/object identity、hide/show/close 通过。
- 动效：10.2 秒共享 scheduler `1224 frames / 120.000Hz`；未增加 timer、QPropertyAnimation 或 per-widget clock。
- 静态：compileall、Ruff、`scripts/check.ps1`、source-limit（183 files ≤1000 lines）、theme audit（3 themes / 22 tokens / 19 selectors / 0 legacy literals）通过。
- 包：`local-arch-124` onefile 已完成；canonical/root/root-latest 均为 `48,052,563` bytes，SHA-256 为
  `B52DD4ADFC7C06CEAF9E2B46BCA1FB9AFB2DE6AA996B286AAF80ED9E12107B16`，archive listing SHA-256 为
  `060BCD1C9D88F7C19657FA6A66794FB333F24BE0B8676605BACFBB4A121F9D1D`，provenance verify 通过；签名 `NotSigned`、
  `release_eligible=false`、`hardware_acceptance=not_run`。

## 限制

本轮是 Python/PySide6 presentation-only，embedded C/C++ public-vendor-source applicability=N/A；不声称 MISRA、ISO 26262 或认证合规。
尚未做真实 Windows GUI/HIDPI、显示器实际 120fps、EXE 冷启动、硬件连接、OTA/RTT 实连与签名发行验收；Qt offscreen 证据不能外推这些结果。
