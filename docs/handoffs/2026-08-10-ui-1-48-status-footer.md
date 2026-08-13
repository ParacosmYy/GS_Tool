# UI-1.48 交接：QStatusBar 状态 footer signal rail

日期：2026-08-10  
父代理：Codex；共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；本轮父代理是唯一源码写入者。  
当前状态：源码、静态检查、离屏视觉验证、onefile 打包和根目录 EXE 覆盖均完成；产物仍是 engineering build。

## 用户结果、范围与非范围

底部 native `QStatusBar` 右侧现在有一个 116×18 的主题化 session/RX signal rail，用于补足连接状态、短时接收活动和
应用 fault 的视觉锚点。原生 status message 仍是唯一权威文字和无障碍表达。

本轮只涉及 presentation/bootstrap/lifecycle 视觉接线；不改变 UART/TCP/UDP/BLE/RTT、协议解析、记录、OTA/debug、ViewModel
状态源、连接动作或任何真实设备行为。未把装饰轨道解释成吞吐、百分比或传输进度。

## 实际修改文件

- `src/serialforge/presentation/status_footer_surface.py`：新增 presentation-only `StatusFooterSurface`；只接受
  `set_state()`、`set_fault()`、`set_activity()`、`set_frame()`、`stop()`，固定尺寸、鼠标透明、不可聚焦、空 accessibility，
  不读取 ViewModel、不创建 `QTimer`。
- `src/serialforge/presentation/controllers/bootstrap.py`：创建 surface 并通过 `QStatusBar.addPermanentWidget()` 挂载；native
  status message 和现有 status bar 行为保留。
- `src/serialforge/presentation/controllers/lifecycle.py`：投影已有 `SessionState`、`ErrorInfo` 是否存在与 RX activity；加入
  `_motion_surfaces`，复用共享 `MotionController` frame/stop。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`：记录边界、依赖方向、生命周期、可访问性与静态回退规则。
- `docs/adr/0035-status-footer-signal-surface.md`：记录决策与替代方案。
- `tasks/plan.md`、`tasks/todo.md`：记录 UI-1.48 范围和完成的源码/离屏阶段。

## 六角色审查记录

```text
产品       019feab3-099d-7281-83b1-f53e33281310  called; repeated wait timed out; closed
架构       019feab3-09eb-7550-994d-cb15ccbcb781  called before source edit; repeated wait timed out; closed; parent bounded audit GO
UI 设计    019feab3-0a3e-7893-83ef-ab9353d52388  called; repeated wait timed out; closed
开发       019feab3-0a85-7232-a151-83a52dba5e56  called read-only; repeated wait timed out; closed; parent remained sole writer
验证       019feab3-0ad2-7962-bf99-bd9bfc6dc679  called; repeated wait timed out; closed
打包流程   019feab3-0b22-7d83-a923-a080653a0285  completed; confirmed provenance fields and packaging risks
独立复核   019feab8-563c-7602-998b-6a46c5794a9b  called read-only; repeated wait timed out; closed; parent five-axis review recorded
```

父代理架构整合：`StatusFooterSurface` 不拥有状态源、controller、timer 或业务依赖；`bootstrap` 只装配，`lifecycle` 只投影；
native status message 不被替换。父代理简化审查确认没有重复 renderer 或可安全合并的第二抽象。

## 验证证据

```text
scripts/check.ps1       PASS 130 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff
compileall               PASS src
UI148_IMPORT_COMPILE     PASS 67 presentation modules; qapplication_instance=False; AST parse
UI148_SURFACE            PASS star_trail/moonlit_ocean/sakura_night; active/fault_static/closed_static; 116x18
UI148_STATUSBAR          PASS three themes; 420x90; exact_white=0; native QStatusBar integration
```

离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论；上述是短时 offscreen evidence，不是持续 GUI 运行证明。
未运行持续 GUI/EXE 启动、Windows 原生键盘/读屏/HIDPI、真实 UART/BLE、RTT/J-Link、OTA、硬件、签名或正式发行验收；未创建、修改或
运行 unit test、mock、fixture、harness 或 test-only 资产。

嵌入式 C/C++ applicability：N/A。本轮是 Python/PySide6 desktop presentation 改动，无适用 public vendor source；不声称
MISRA、ISO 26262、ASIL、ASPICE、汽车或其他认证合规。

## 构建产物与最终交付

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，已在 checkout 内安全校验目标后覆盖，字节一致
- source revision：`local-ui-1.48`
- size：`47,817,520` bytes
- canonical/root SHA-256：`1DC3A5424FC3202DA7708BCA53BFF755317728911B23D1A2D7B74BC680A8C314`
- archive listing SHA-256：`A061158AF68D7B02117AF008AF439548E6675CE67B64AFF99810F5E1AEFF1E8F`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

未决项：持续 GUI/EXE 启动、Windows 原生键盘/读屏/HIDPI、真实 UART/BLE/RTT/J-Link、OTA、硬件、签名和正式发行验收仍需相应授权与环境；
它们不是本轮 offscreen/静态/工程包证据可以替代的项目。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
