# UI-1.49 交接：发送输入框状态轨与焦点反馈

日期：2026-08-10  
父代理：Codex；共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；父代理是本轮唯一源码写入者。  
当前状态：源码、静态检查、三主题离屏状态验证、onefile 打包和根目录 EXE 覆盖均完成；产物仍是 engineering build。

## 用户结果、范围与非范围

发送输入框现在把已有 `send_band_state` 显示为 blocked/waiting/ready/busy/history 语义色轨道；ready/busy 且有焦点时可由共享
MotionController 显示低频 pulse，焦点边界有额外反馈。

输入框仍是原生 QLineEdit：文本、选择、光标、剪贴板、placeholder、回车、QSS、AccessibleName/Description 和发送 gate 保持不变。
本轮不改变 ViewModel、transport、协议、payload、OTA/debug 或真实设备行为。

## 实际修改文件

- `src/serialforge/presentation/send_input_surface.py`：presentation-only `SendInputSurface`，只提供
  `set_surface_state()`、`set_frame()`、`stop()`；原生绘制先执行，装饰后置；无 timer、事件过滤器和业务依赖。
- `src/serialforge/presentation/controllers/terminal.py`：仅替换输入框构造类型，保留已有 signal/keyboard/accessibility wiring。
- `src/serialforge/presentation/controllers/connection.py`：投影已有 `send_band_state`，不改变 enable/send hint/gate。
- `src/serialforge/presentation/controllers/lifecycle.py`：加入已有 `_motion_surfaces` frame/stop fan-out。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0036-send-input-surface-state-rail.md`、`tasks/plan.md`、`tasks/todo.md`：同步架构约束与 ADR。

## 六角色审查记录

```text
产品       019feabd-3d4c-7a30-9d7e-54c1dc475517  called; repeated wait timed out; closed
架构       019feabd-3d96-7d11-a9c1-0c96160dcc99  called before source edit; repeated wait timed out; closed; parent bounded audit GO
UI 设计    019feabd-3de6-7190-8dc1-821d1ac8c003  called; repeated wait timed out; closed
开发       019feabd-3e37-7283-ba1b-1ad7bc17bc5f  called read-only; repeated wait timed out; closed
验证       019feabd-3e86-7dd3-b95a-aa96c14694ed  called; repeated wait timed out; closed
打包流程   019feabd-3ecd-7a31-bfd4-fd39a2e0b055  called; repeated wait timed out; closed
独立复核   019feabf-42c3-73b3-9d09-51f8de2bf42e  called read-only; repeated wait timed out; closed; parent five-axis review recorded
```

父代理审查：依赖方向保持 presentation-only；`QLineEdit.paintEvent()` 先完成，输入交互不被自绘层接管；状态轨不承载业务事实；
不需要额外 renderer 抽象，保持单一小模块和共享 MotionController 生命周期。

## 验证证据

```text
scripts/check.ps1       PASS 131 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff
compileall               PASS src
UI149_FIELD              PASS star_trail/moonlit_ocean/sakura_night; five states; native QLineEdit; text=AA 55 preserved
UI149_PIXEL              PASS state screenshots and themed host rendering
UI149_SEND_BAND          PASS three themes; themed sendControlBand; exact_white=0; native surface type retained
```

离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论；这些是短时 offscreen evidence，不是持续 GUI/EXE 启动证明。
未运行 Windows 原生键盘/读屏/HIDPI、真实 UART/BLE/RTT/J-Link、OTA、硬件、签名或正式发行验收；未创建、修改或运行 unit test、mock、
fixture、harness 或 test-only 资产。

嵌入式 C/C++ applicability：N/A。本轮是 Python/PySide6 desktop presentation 改动，无适用 public vendor source；不声称 MISRA、
ISO 26262、ASIL、ASPICE、汽车或其他认证合规。

## 构建产物与最终交付

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，已在 checkout 内安全校验目标后覆盖，字节一致
- source revision：`local-ui-1.49`
- size：`47,822,058` bytes
- canonical/root SHA-256：`4998DCEBB54D7464CF19EB2CC19F0319F5013444C35E50C8CF57EE5435B4D35F`
- archive listing SHA-256：`77BE6FBE1BC0155AF3AFAA560B4D1C53803BC4AB2C2BA8D91EEE12AED9F953EE`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

未决项：持续 GUI/EXE 启动、Windows 原生键盘/读屏/HIDPI、真实 UART/BLE/RTT/J-Link、OTA、硬件、签名和正式发行验收仍需相应授权与环境；
这些项目不能由静态、offscreen 或 engineering build 证据替代。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
