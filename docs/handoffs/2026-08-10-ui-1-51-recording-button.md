# UI-1.51 交接：原始记录按钮 activity rail

日期：2026-08-10  
父代理：Codex；共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；父代理是本轮唯一源码写入者。  
当前状态：源码、静态检查、recording 状态离屏验证、onefile 打包和根目录 EXE 覆盖均完成；产物仍是 engineering build。

## 用户结果、范围与非范围

原始记录按钮现在复用 `BusyActionButton` 展示 `RecordingState.STARTING/ACTIVE/STOPPING` 的 activity rail；按钮文字仍按既有逻辑在
“开始原始记录/停止原始记录”之间切换，`ERROR/STOPPED` 保持静态非 busy。

本轮不改变 recording worker、文件写入、JSONL 格式、错误传播、ViewModel、transport、OTA/debug 或真实设备行为。

## 实际修改文件

- `src/serialforge/presentation/controllers/terminal.py`：record button 使用既有 `BusyActionButton`，保留 click、焦点、文本、QSS、
  AccessibleName/Description 和 tab wiring。
- `src/serialforge/presentation/controllers/terminal_runtime.py`：将三个既有 recording busy 状态投影到 `set_busy()`；文案与 recordState
  projection 保持原逻辑。
- `src/serialforge/presentation/controllers/lifecycle.py`：将 record button 加入唯一 MotionController `_motion_surfaces`。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0038-recording-button-activity-rail.md`、`tasks/plan.md`、`tasks/todo.md`：同步架构边界和任务。

## 六角色审查记录

```text
产品       019feac9-c929-7c43-936b-49a8107705bc  called; repeated wait timed out; closed
架构       019feac9-c979-7f70-92a9-91a2ba99fa20  called before source edit; repeated wait timed out; closed; parent bounded audit GO
UI 设计    019feac9-c9c8-77e2-a25c-f36fbc960ae2  called; repeated wait timed out; closed
开发       019feac9-ca10-7990-80ea-0aee7d8394f7  called read-only; repeated wait timed out; closed
验证       019feac9-ca5d-7953-9241-77ef5f3a9998  called; repeated wait timed out; closed
打包流程   019feac9-cab0-7a00-b593-c7f88bcaa75d  called; repeated wait timed out; closed
独立复核   019feacc-4119-79c3-b3ef-ef88ab7ead14  called read-only; repeated wait timed out; closed; parent five-axis review recorded
```

父代理审查：已有 BusyActionButton shared renderer/lifecycle 足够，未新增 recording 专用 widget；recording state 仍由 terminal runtime 负责，
视觉忙碌不表示写盘百分比或传输完成。

## 验证证据

```text
scripts/check.ps1       PASS 132 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff
compileall               PASS src
UI151_RECORD             PASS idle/starting/active/stopping/error; native BusyActionButton; text/accessibility retained
UI151_MOTION             PASS shared frame; busy rail; static/reduced stop fallback
```

离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论；这些是短时 offscreen evidence，不是持续 GUI/EXE 启动证明。
未运行真实 recording worker/文件写入、Windows 原生键盘/读屏/HIDPI、真实 UART/BLE/RTT/J-Link、OTA、硬件、签名或正式发行验收；未创建、修改
或运行 unit test、mock、fixture、harness 或 test-only 资产。

嵌入式 C/C++ applicability：N/A。本轮是 Python/PySide6 desktop presentation 改动，无适用 public vendor source；不声称 MISRA、
ISO 26262、ASIL、ASPICE、汽车或其他认证合规。

## 构建产物与最终交付

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，已在 checkout 内安全校验目标后覆盖，字节一致
- source revision：`local-ui-1.51`
- size：`47,824,961` bytes
- canonical/root SHA-256：`636E471AFE973EE703254BD6A0D3D5BEC63E59C29762B67A68F220D0BB004315`
- archive listing SHA-256：`3D64C4E65BB5425A6D3F39F9E93AF8A3F38CD9921CB37EF9E05500B336F2AD13`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

未决项：持续 GUI/EXE 启动、Windows 原生键盘/读屏/HIDPI、真实 UART/BLE/RTT/J-Link、OTA、硬件、签名和正式发行验收仍需相应授权与环境；
这些项目不能由静态、offscreen 或 engineering build 证据替代。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
