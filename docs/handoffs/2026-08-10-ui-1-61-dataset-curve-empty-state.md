# 2026-08-10 UI-1.61 Dataset 曲线空态画布交接

日期：2026-08-10  
范围：`DatasetCurveWidget` 空态主题化、共享 signal rail、最新 onefile EXE 交付。

## 结果

- 未选择数值 series、等待 sample、无可绘制数值三种状态统一为主题化 canvas：glyph、标题、说明、节点 rail 均使用 `ThemeSpec` 语义色。
- `set_frame()`/`stop()` 只驱动空态装饰；真实 points 仍由原有 `CurveSnapshot` 绘制。
- 保留 snapshot debounce `QTimer`、`flush()`、`set_suspended()`、`shutdown()`、焦点环、AccessibleDescription 和真实数据曲线契约。
- `_dataset_curve` 接入 `lifecycle.py` 的共享 MotionController fan-out；未创建局部动效时钟。

## 实际修改

- `src/serialforge/presentation/dataset_curve.py`
- `src/serialforge/presentation/controllers/lifecycle.py`
- `docs/ARCHITECTURE.md`
- `docs/CONSTRAINTS.md`
- `docs/adr/0048-dataset-curve-empty-state.md`
- `tasks/plan.md`
- `tasks/todo.md`

## 架构角色与复核

```text
架构角色  019feb28-f305-7ca1-b2cc-1411f691ed00  called before source edit; wait timed out; closed
独立质量复核  019feb2b-43aa-7103-bee9-e180a2d6bc2e  called after implementation; wait timed out; closed
父代理 bounded audit  GO：snapshot/data path、QTimer、suspend/shutdown、真实 points 和主题绘制边界保持；未发现跨模块耦合或越界绘制问题
简化评估  不抽取通用状态机；Dataset 曲线空态仍是单一 presentation 边界，通用化会复制数据契约
```

## 验证证据

```text
pwsh -NoProfile -ExecutionPolicy Bypass -File .\scripts\check.ps1  PASS
  source line limit: 141 files <= 1000
  theme token audit: 3 themes, 22 semantic tokens, 19 selectors, legacy_qss_literals=339
  Ruff/compileall: PASS

UI161_CURVE_EMPTY_SMOKE          PASS
  shared motion membership; frame/stop; empty/waiting snapshots; flush; geometry

UI161_CURVE_EMPTY_PIXEL_AUDIT    PASS
  star_trail/moonlit_ocean/sakura_night; 1180x780; near_white=0

provenance.py verify              PASS
```

离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论。未运行持续 GUI、Windows 原生 HIDPI/读屏、真实 UART/TCP/BLE/RTT/J-Link、OTA、硬件、签名或正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

嵌入式 C/C++ 适用性：N/A。本轮只修改 Python/PySide6 presentation 代码，没有 MCU、RTOS、BSP/HAL、驱动或厂商要求适用；因此嵌入式厂商公开源核验、固件烧录和硬件验证均不适用/未执行。

## 最新打包产物

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)
- source revision：`local-ui-1.61`
- size：`47,867,798` bytes
- canonical/root SHA-256：`593C35BFC87C530069351F11C1E122214601EC0561D671DE5B00DE582F2903E2`
- archive listing SHA-256：`82ACA88552876F18E270756D758EDE0C05A64BEA1C3D6BB9B935D0B5224897BE`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- signature：`NotSigned`
- release eligibility：`false`
- hardware acceptance：`not_run`

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
