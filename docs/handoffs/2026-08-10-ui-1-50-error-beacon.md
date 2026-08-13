# UI-1.50 交接：errorBar 故障 beacon

日期：2026-08-10  
父代理：Codex；共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；父代理是本轮唯一源码写入者。  
当前状态：源码、静态检查、三主题 active/static/clear 离屏验证、onefile 打包和根目录 EXE 覆盖均完成；产物仍是 engineering build。

## 用户结果、范围与非范围

errorBar 现在有一个 28×28 的主题化故障 beacon。`ErrorInfo` 非空时显示静态环/叉标并可由共享 MotionController 显示低频 pulse；
暂停、低动效、隐藏、最小化、关闭时保持静态回退或停止。错误文字、详情、clear button、可见性和错误恢复语义完全保留。

本轮只修改 presentation；不改变错误码、ViewModel、transport、协议、OTA/debug、真实设备或发送行为。

## 实际修改文件

- `src/serialforge/presentation/error_surface.py`：presentation-only `ErrorSignalSurface`，只提供
  `set_active()`、`set_frame()`、`stop()`；固定 28×28，鼠标透明、不可聚焦、空 accessibility，无 timer。
- `src/serialforge/presentation/controllers/terminal.py`：在现有 errorBar layout 中加入 beacon，保留 label/clear button。
- `src/serialforge/presentation/controllers/lifecycle.py`：既有 ErrorInfo empty/non-empty 分支投影 active/clear，纳入统一 frame/stop fan-out。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0037-error-notification-signal-beacon.md`、`tasks/plan.md`、`tasks/todo.md`：同步规则。

## 六角色审查记录

```text
产品       019feac3-a720-7050-b270-cb46df70ee34  called; repeated wait timed out; closed
架构       019feac3-a760-79c1-b8e5-40cb05331f70  called before source edit; repeated wait timed out; closed; parent bounded audit GO
UI 设计    019feac3-a7af-7880-91b2-51a8560759ad  called; repeated wait timed out; closed
开发       019feac3-a801-7522-9220-b2c03cedf23b  called read-only; repeated wait timed out; closed
验证       019feac3-a84c-7230-bf45-4670e7a96ee6  called; repeated wait timed out; closed
打包流程   019feac3-a896-7463-aa4a-2106fac31b9b  called; repeated wait timed out; closed
独立复核   019feac6-bab1-7890-a0fe-13d9ff297a64  called read-only; repeated wait timed out; closed; parent five-axis review recorded
```

父代理审查：错误信号只消费 ErrorInfo 的已有可见性投影，错误 label/clear button 仍是权威交互；不新增状态源、timer、事件过滤器或
错误恢复流程，静态 beacon 是 pulse 的安全回退。

## 验证证据

```text
scripts/check.ps1       PASS 132 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff
compileall               PASS src
UI150_ERROR              PASS star_trail/moonlit_ocean/sakura_night; active/static/stop/clear; label and clear retained
UI150_PIXEL              PASS three themes; errorBar 424x56; exact_white=0
```

离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论；这些是短时 offscreen evidence，不是持续 GUI/EXE 启动证明。
未运行 Windows 原生键盘/读屏/HIDPI、真实 UART/BLE/RTT/J-Link、OTA、硬件、签名或正式发行验收；未创建、修改或运行 unit test、mock、
fixture、harness 或 test-only 资产。

嵌入式 C/C++ applicability：N/A。本轮是 Python/PySide6 desktop presentation 改动，无适用 public vendor source；不声称 MISRA、
ISO 26262、ASIL、ASPICE、汽车或其他认证合规。

## 构建产物与最终交付

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，已在 checkout 内安全校验目标后覆盖，字节一致
- source revision：`local-ui-1.50`
- size：`47,823,121` bytes
- canonical/root SHA-256：`D4A81A89B4CE6FF31FBB5DA93C4D93CD57921E05C70DD5A674BC05E947392B3C`
- archive listing SHA-256：`3D64C4E65BB5425A6D3F39F9E93AF8A3F38CD9921CB37EF9E05500B336F2AD13`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

未决项：持续 GUI/EXE 启动、Windows 原生键盘/读屏/HIDPI、真实 UART/BLE/RTT/J-Link、OTA、硬件、签名和正式发行验收仍需相应授权与环境；
这些项目不能由静态、offscreen 或 engineering build 证据替代。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
