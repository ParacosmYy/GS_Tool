# 2026-08-10 UI-1.62 链路传输模式 glyph 交接

日期：2026-08-10  
范围：链路页传输模式视觉识别、共享动效接线、最新 onefile EXE 交付。

## 结果

- 新增 `TransportModeSurface`，在“传输”下拉框旁绘制 UART、TCP Client、TCP Server、UDP、BLE GATT、J-Link RTT 六种几何 glyph。
- glyph 只镜像 `_transport_combo.currentData()` 的有界模式值；ComboBox 仍保留选择、键盘顺序、AccessibleName/Description 和连接动作 owner。
- glyph 使用唯一共享 `MotionController` 的 frame/stop；没有本地 timer、业务状态、ViewModel/SessionState 读取、自动连接或配置修改。
- 现有 header context、preset summary、connection hint、SessionState、面板显隐、980/1180 响应式行为均保持。

## 实际修改

- `src/serialforge/presentation/transport_mode_surface.py`
- `src/serialforge/presentation/controllers/connection_builder.py`
- `src/serialforge/presentation/controllers/connection_runtime.py`
- `src/serialforge/presentation/controllers/lifecycle.py`
- `docs/ARCHITECTURE.md`
- `docs/CONSTRAINTS.md`
- `docs/adr/0049-transport-mode-glyph.md`
- `tasks/plan.md`
- `tasks/todo.md`

## 架构角色与复核

```text
架构角色  019feb32-d9d8-77c3-a95a-21efc687f99f  called before source edit; wait timed out; closed
独立质量复核  019feb38-22e0-7fa1-8daa-f05dca1d65d5  called after implementation; wait timed out; closed
父代理 bounded audit  GO：六模式映射、最小 QPainter 边界、shared frame/stop、无连接副作用和 980/1180 布局均通过；未发现 Critical/Required
简化评估  不增加 transport context 状态机；一个固定 glyph renderer 足够表达模式识别，避免复制既有文本事实
```

## 验证证据

```text
pwsh -NoProfile -ExecutionPolicy Bypass -File .\scripts\check.ps1  PASS
  source line limit: 142 files <= 1000
  theme token audit: 3 themes, 22 semantic tokens, 19 selectors, legacy_qss_literals=339
  Ruff/compileall: PASS

UI162_TRANSPORT_MODE_SMOKE       PASS
  six mode mappings; UART/network/BLE panel visibility; shared motion membership; frame/stop; 980/1180 horizontal-scroll gate

UI162_TRANSPORT_MOTION_DELTA     PASS
  phase changes altered 342 glyph pixels

UI162_TRANSPORT_PIXEL_AUDIT      PASS
  star_trail/moonlit_ocean/sakura_night; 1180x780; near_white=0

provenance.py verify              PASS
```

离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论。未运行持续 GUI、Windows 原生 HIDPI/读屏、真实 UART/TCP/BLE/RTT/J-Link、OTA、硬件、签名或正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

嵌入式 C/C++ 适用性：N/A。本轮只修改 Python/PySide6 presentation 代码，没有 MCU、RTOS、BSP/HAL、驱动或厂商要求适用；嵌入式厂商公开源核验、固件烧录和硬件验证均不适用/未执行。

## 最新打包产物

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)
- source revision：`local-ui-1.62`
- size：`47,871,279` bytes
- canonical/root SHA-256：`8331F98A3ED790D589C556641EBE3E31B924B535ED4A5748D8ED716A893AEECD`
- archive listing SHA-256：`40BF58857031C5CDF9DA8E661A47E0C28F54E9E66339ABE103E85778B7E5B60D`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- signature：`NotSigned`
- release eligibility：`false`
- hardware acceptance：`not_run`

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
