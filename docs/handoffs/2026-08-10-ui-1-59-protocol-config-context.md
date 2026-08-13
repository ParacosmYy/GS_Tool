# UI-1.59：协议配置草稿/应用摘要

日期：2026-08-10  
范围：SerialForge presentation 协议配置页

## 交付结果

协议配置面板现在提供一条紧凑的配置上下文摘要，直接显示当前 framing、checksum、最大帧长和应用阶段：

- `草稿 · 待应用`：编辑控件发生变化，但尚未点击应用；
- `已应用 · 等待接收/活跃`：沿用既有 `protocolStatus.state`；
- `暂不可用`：沿用既有 blocked 状态，不重新判断传输/历史 gate。

摘要宽度限制 190–520 px，使用四节点 config rail 表达阶段，不进入键盘焦点、不拦截鼠标、不创建自身 timer。

## 架构边界

- `ProtocolConfigContextProjection` 是 immutable presentation DTO。
- `controllers/protocol_context.py` 只读取原生 framing/checksum/max-frame 控件和 `protocolStatus.property("state")`。
- `protocol_config.py` 继续拥有 editor、apply/reset、parser gate、status 文案与状态投影。
- `protocol.py` 只负责装配；`composition.py` 显式暴露字段；`lifecycle.py` 统一 MotionController 的 frame/stop。
- `theme_stylesheet_base.py` 与 `theme_variant_shell.py` 显式覆盖 waiting/active/draft/blocked/history/idle，避免系统 palette 白色回退。

## 架构审查与简化评估

```text
架构角色  019feb15-695e-78b0-b1af-6dff06e93d01  called before source edit; wait timed out; closed
独立质量复核  019feb1a-b071-7880-807e-d2929e8b5aef  called after implementation; wait timed out; closed
父代理 bounded audit  GO：没有重复 parser gate、没有本地 timer、Qt parent/lifecycle/QSS 接入完整
安全简化评估  deferred：不抽取通用 ConfigSummarySurface；当前单一职责边界更容易审查，抽象会扩大耦合
```

本轮为 Python/PySide6 UI 变更，不涉及 MCU、RTOS、BSP、驱动、固件或厂商硬件要求；嵌入式公开厂商资料适用性为 N/A。

## 验证证据

```text
pwsh -NoProfile -ExecutionPolicy Bypass -File .\scripts\check.ps1
PASS: source line limit 140 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff/compileall

inline offscreen UI159_SMOKE
PASS: projection states waiting/draft/active/blocked; accessibility; shared frame/stop; width <= 520; near_white=0

inline offscreen UI159_PROTOCOL_PAGE_SMOKE
PASS: protocol tab; context geometry 520x32; star_trail/moonlit_ocean/sakura_night; 1180x780; near_white=0

python scripts/provenance.py verify --manifest .\dist\release\0.1.0\core\onefile\PROVENANCE.json
PASS: manifest verified
```

离屏环境输出 Qt 缺少 `PySide6/lib/fonts` 警告，截图中文方框不代表 Windows 字体结论。未运行持续 GUI、Windows 原生字体/HIDPI/读屏、真实设备、硬件、签名或正式发行验收；未创建、修改或运行测试专用代码/资产。

## 当前 onefile 交付

- canonical：[SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)
- source revision：`local-ui-1.59`
- size：`47,856,384` bytes
- canonical/root SHA-256：`ECA8633C71DB6673B62CDC3EA063666E228224ED8757FD360C2735A66F2EE8A6`
- archive listing SHA-256：`349AAC2DB436582FD1D60DCDE3018FA41C169CA3ADBA96BFC1FA90DEA777CB99`
- signature：`NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
