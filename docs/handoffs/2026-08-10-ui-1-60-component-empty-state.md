# UI-1.60：Component 空态卡片

日期：2026-08-10  
范围：SerialForge presentation Component telemetry surface

## 交付结果

Component 无数据时现在显示可行动的空态卡片，而不是孤立的空表头加普通文字：

- 卡片标题/提示继续来自 controller 的完整文案；presentation surface 只负责拆分显示层级。
- “加载 Profile / Codec”按钮放在空态附近，发出无参数 `load_requested` intent，复用既有 `on_load_component_codec`。
- 空态时隐藏 `QTableWidget` 表头；有可见 rows 时恢复原生表格，不改变过滤、选择、导出和 codec 行为。
- glyph 与 CTA 使用共享 MotionController frame/stop，低动效、暂停、隐藏、最小化和关闭都静态回退。

## 架构边界

- `ComponentEmptyStateSurface` 只提供 `setText()`、`text()`、`clear()` 兼容接口和 `load_requested` intent。
- `derived_data.py` 是 visible rows/table-empty 互斥显隐 owner；它不把空态卡片当成新状态源。
- `protocol_config.py` 只把既有 derived source gate 传给 `set_action_enabled()`；卡片不复制 gate。
- `protocol.py` 只装配并连接既有 callback；`lifecycle.py` 统一动效生命周期。

## 架构审查与简化评估

```text
架构角色  019feb21-0879-76c1-b406-65782c04f14c  called before source edit; wait timed out; closed
独立质量复核  019feb24-3d9d-75f3-b888-246830ade834  called after implementation; wait timed out; closed
父代理 bounded audit  GO：没有 gate/state 复制、没有本地 timer、Qt parent/lifecycle/QSS 接入完整
安全简化评估  deferred：不抽取通用空态状态机，保持 Component 单一变化边界
```

本轮为 Python/PySide6 UI 变更，不涉及 MCU、RTOS、BSP、驱动、固件或厂商硬件要求；嵌入式公开厂商资料适用性为 N/A。

## 验证证据

```text
pwsh -NoProfile -ExecutionPolicy Bypass -File .\scripts\check.ps1
PASS: source line limit 141 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff/compileall

inline offscreen UI160_COMPONENT_EMPTY_SMOKE
PASS: componentEmptyState; table hidden; CTA signal; action gate visibility; shared frame/stop; accessibility

inline offscreen UI160_COMPONENT_SCROLL_PIXEL
PASS: star_trail/moonlit_ocean/sakura_night; 1180x780; near_white=0

python scripts/provenance.py verify --manifest .\dist\release\0.1.0\core\onefile\PROVENANCE.json
PASS: manifest verified; canonical/root hash match
```

离屏环境输出 Qt 缺少 `PySide6/lib/fonts` 警告，截图中文方框不代表 Windows 字体结论。未运行持续 GUI、Windows 原生字体/HIDPI/读屏、真实设备、硬件、签名或正式发行验收；未创建、修改或运行测试专用代码/资产。

## 当前 onefile 交付

- canonical：[SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)
- source revision：`local-ui-1.60`
- size：`47,865,160` bytes
- canonical/root SHA-256：`C0962904AB5678D525B44F1702E907B4798E198171103081BCC2C38557BC83BB`
- archive listing SHA-256：`82ACA88552876F18E270756D758EDE0C05A64BEA1C3D6BB9B935D0B5224897BE`
- signature：`NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
