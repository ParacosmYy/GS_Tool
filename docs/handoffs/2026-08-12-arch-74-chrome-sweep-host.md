# ARCH-74 / UI-1.147 Explicit chrome sweep host

日期：2026-08-12  
范围：主题切换 fade target、header sweep host 与窗口 resize 生命周期边界。

## 结果

`start_theme_transition(window, *, target, sweep_host)` 现在显式区分两个职责：`target` 是 `appRoot`，
只承载全局 opacity fade；`sweep_host` 是 bootstrap 唯一组装的 header chrome，必须属于 root 子树，
`ThemeTransitionSurface` 只使用 host 本地坐标绘制。窗口 resize 由现有 lifecycle façade 先停止一次性
主题过渡，再交给 Qt 基类重新布局，避免 180ms 动画继续使用旧 host geometry。

没有新增 timer、MotionController、业务状态、事件总线、主题状态或外部依赖；连接、协议、记录、发送、
终端和 OTA/debug contract-only 语义不变。

## 验证证据

```text
ARCH74_HOST_GEOMETRY_PASS size=980x720 host=(16,16,948,115) overlay=(-180,0,180,115) terminal_top=413 send_top=587
ARCH74_HOST_GEOMETRY_PASS size=1240x820 host=(16,16,1208,115) overlay=(-180,0,180,115) terminal_top=483 send_top=687
ARCH74_RESIZE_CONSTRUCTOR_PASS host=(0,0,100,30)
ARCH74_RESIZE_ABORT_PASS from=980x720 to=1240x820 terminal=(16,483,1208,192) send=(16,687,1208,98)
ARCH74_RESIZE_ABORT_PASS from=1240x820 to=980x720 terminal=(16,413,948,162) send=(16,587,948,98)
ARCH74_RESIZE_CLEANUP_PASS
ARCH74_THEME_MATRIX_PASS themes=3
ARCH74_MOTION_GATE_PASS reduced=True paused=True
ARCH74_THEME_LIFECYCLE_PASS hidden=True close=True
ARCH74_MOTION_PASS hz=120 timer=PreciseTimer intervals=[8,9,8,8,9,8,8,9,8,8,9,8]
scripts/check.ps1: pass
source line limit: pass (166 files <= 1000)
theme token audit: pass (3 themes, 22 semantic tokens, 19 selectors, legacy_qss_literals=0)
```

验证器中两次前置错误已定位为 Qt `QRect` 类型转换和 combo/lifecycle 状态设置问题，修正后没有产品代码
改动；所有 inline runtime smoke 均通过 `finally` 清理 application services。环境仍有 PySide6 bundled
fonts directory warning，已通过 `configure_application_font()` 选择系统字体。

## 架构、审查与适用性

架构师对显式 `target/sweep_host` 契约和 resize stop 方案均给出 conditional approve；独立 Luna/max 最终
五轴审查返回 Required=0、Optional=0，可打包。审查确认 host ancestor 校验、root effect 恢复、
stop/finish identity guard、resize 前清理、MainWindow 薄 façade、单一 MotionController 与无新增外部依赖。
可选但未扩大范围的后续简化是合并 stop/finish 的重复 teardown helper。

本轮为 Python/Qt presentation-only，不涉及 embedded C/C++、MCU/BSP/HAL/RTOS/ISR/DMA、OTA 固件、
Flash/NVM、功耗或电机控制；公共 MCU 厂商源适用性为 N/A，不声明 MISRA、ISO 26262 或任何认证合规性。
真实 GUI、HIDPI、读屏、设备、OTA 安全后端、J-Link/RTT 和硬件验收未执行。

## 包交付

```text
PACKAGE_ARCH74_FINAL                         pass
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
root-latest: SerialForge-latest.exe (byte-identical)
size: 47,981,373 bytes
SHA256: 42C2451F5315CB8273CEEA45985ED6E595CF7E495B8E26E8434B41B2B4B45E34
archive listing SHA-256: C9C66CBFDA9102E0A13C6A95DD168ECDF3DC358009DFA125EBCB9D4CF636A732
provenance: pass; source revision local-arch-74
signature: NotSigned; release_eligible=false; hardware_acceptance=not_run
ROOT_EXE_OVERWRITE_PASS: canonical/root/root-latest size and SHA-256 identical
```
