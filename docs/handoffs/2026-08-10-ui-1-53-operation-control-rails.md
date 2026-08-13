# UI-1.53 交接：操作控制 activity rails

日期：2026-08-10  
父代理：Codex；共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；父代理是本轮唯一源码写入者。  
当前状态：源码、静态检查、batch/replay 状态离屏验证、三主题截图、onefile 打包和根目录 EXE 覆盖均完成；产物仍是 engineering build。

## 用户结果、范围与非范围

批量命令执行时，停止按钮现在显示共享 activity rail；历史回放播放时，暂停按钮显示共享 activity rail。批量 `IDLE/COMPLETED`、
回放 `PAUSED/EOF/STOPPED/ERROR/EMPTY` 均静态回退。按钮仍保留原生 click、enabled、文案和无障碍语义。

本轮不改变 batch/replay worker、文件解析、发送队列、传输、设备行为、ViewModel 状态源或真实进度语义。

## 实际修改文件

- `src/serialforge/presentation/controllers/terminal.py`：批量停止按钮使用 `BusyActionButton`。
- `src/serialforge/presentation/controllers/protocol.py`：回放暂停按钮使用 `BusyActionButton`。
- `src/serialforge/presentation/controllers/connection.py`：`batch_active` → stop button `set_busy()`。
- `src/serialforge/presentation/controllers/replay.py`：`ReplayState.PLAYING` → pause button `set_busy()`。
- `src/serialforge/presentation/controllers/lifecycle.py`：两个控件加入共享 frame/stop fan-out。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0040-operation-control-activity-rails.md`、`tasks/plan.md`、`tasks/todo.md`：同步边界。

## 架构审查与父代理整合

```text
架构角色  019fead9-910d-7823-b9d0-8aa757bf80ce  called before source edit; repeated wait timed out twice; closed
父代理    bounded audit GO：复用 shared BusyActionButton/MotionController；batch/replay owner projection unchanged
```

父代理简化结论：无需新增 renderer、timer 或抽象；busy 只表示本地操作正在进行，不表示设备完成度、回放总进度或批量确认。

## 验证证据

```text
scripts/check.ps1       PASS 132 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff
compileall               PASS src
UI153_BATCH              PASS IDLE/RUNNING/COMPLETED; BusyActionButton; enabled/accessibility retained
UI153_REPLAY             PASS EMPTY/PLAYING/PAUSED/EOF/STOPPED/ERROR; text/enabled retained
UI153_MOTION             PASS shared frame; busy rail; static/stop fallback
UI153_PIXEL              PASS three themes; replay and batch 980x680; near_white=0
```

离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论。未运行持续 GUI/EXE 启动、真实 batch/replay worker、UART/BLE/RTT/J-Link、
OTA、硬件、Windows 原生键盘/读屏/HIDPI、签名或正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。
嵌入式 C/C++ applicability：N/A；本轮是 Python/PySide6 desktop presentation 改动。

## 构建产物与最终交付

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，已在 checkout 内安全校验目标后覆盖，字节一致
- source revision：`local-ui-1.53`
- size：`47,825,866` bytes
- canonical/root SHA-256：`99439BB50574149CA2080814278547F3723929A3ED85384444973868B7868E2C`
- archive listing SHA-256：`3D64C4E65BB5425A6D3F39F9E93AF8A3F38CD9921CB37EF9E05500B336F2AD13`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

未决项：持续 GUI/EXE 启动、Windows 原生键盘/读屏/HIDPI、真实 UART/BLE/RTT/J-Link、OTA、硬件、签名和正式发行验收仍需相应授权与环境；
这些项目不能由静态、offscreen 或 engineering build 证据替代。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
