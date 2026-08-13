# UI-1.47 交接：刷新/扫描按钮共享忙碌信号轨

日期：2026-08-10  
范围：UART 端口刷新、BLE 扫描的忙碌视觉反馈；不改变 transport、扫描、刷新或业务状态语义。

## 交付结果

- `presentation/action_surface.py` 新增 `BusyActionButton`，继续继承原生 `QPushButton`，保留点击、焦点、禁用、QSS、
  AccessibleName 和 AccessibleDescription 语义。
- `connection_builder.py` 只替换刷新和 BLE 扫描两个按钮；`connection.py` 只投影已有
  `discovery_busy` / `ble_scan_busy`；`lifecycle.py` 复用已有唯一 `MotionController` 帧分发。
- 发现两个 action surface 的绘制逻辑重复后，提取私有 `_paint_signal_rail()`，保留两个控件各自的主题色、几何和透明度参数。
- 动画关闭、低动效、暂停、隐藏、最小化和关闭时保留静态 activity rail；没有新增控件级 timer、线程、业务状态、进度或传输行为。

## 架构与审查记录

- 架构角色：`019feaa7-7b72-7d01-b980-e0a6f8a58fab`，初始边界审查调用后重复等待超时并关闭；父代理完成有界审计并允许实现。
- 架构角色：`019feaac-3741-7103-8c64-5d4c1d4bdade`，针对重复 renderer 提取的只读审查调用后重复等待超时并关闭；父代理审计确认提取保持边界。
- 独立质量审查：`019feaaa-4f4b-78c1-ba24-323b466a5147`，只读审查调用后重复等待超时并关闭；父代理完成五轴审查：依赖方向、行为保持、生命周期、可访问性、可简化性均无阻塞项。
- 简化结论：共享私有 renderer 消除重复 painter；`BusyActionButton` 不拥有 timer、状态源或 transport 依赖，按钮 API 与原生 Qt 交互保持不变。

## 验证证据

```text
scripts/check.ps1       PASS 129 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff
UI147_LAYOUT             PASS 980x680; exact_white=0; no horizontal overflow
UI147_BUSY               PASS three themes; native button type; busy text/enabled/colored rail
UI147_REDUCED            PASS busy remains visible with animated=False/static rail
UI147_IMPORT/COMPILE     PASS 129 modules; compileall; AST pass
UI147_PACKAGE            PASS onefile manifest; canonical/root size and SHA equal
PROVENANCE               PASS source=local-ui-1.47; signature=NotSigned; release_eligible=false
EMBEDDED_REVIEW          N/A for Python/PySide6; no MCU/C/C++/firmware/vendor profile applicable
```

离屏环境报告缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论。未运行持续 GUI、EXE 启动、Windows 原生键盘/读屏/HIDPI、
真实 UART/BLE 扫描、硬件或正式签名发布验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

## 构建产物

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)
- source revision：`local-ui-1.47`
- size：`47,814,301` bytes
- canonical/root SHA-256：`85CAB8E0977BD3FA2A849CA64E891A64E19F3EFFF52D74E6480C746D5740A15D`
- archive listing SHA-256：`314DF07FC1EC7E15C6DF32BBA8776AC83999BA78A371764F3399EFB32D7FD94A`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
