# UI-1.52 交接：连接主按钮 activity rail

日期：2026-08-10  
父代理：Codex；共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；父代理是本轮唯一源码写入者。  
当前状态：源码、静态检查、连接状态离屏验证、三主题截图、onefile 打包和根目录 EXE 覆盖均完成；产物仍是 engineering build。

## 用户结果、范围与非范围

连接主按钮现在复用 `BusyActionButton` 展示既有 `SessionState.OPENING/CLOSING` 的 activity rail；`OPEN/CLOSED/ERROR/DISCOVERED`
保持静态。按钮文字、enabled gate、原生 click、AccessibleName/Description 和连接 action callback 保持既有语义。

本轮不改变 session worker、transport、重试、网络授权、ViewModel 状态源、OTA/debug 或真实设备行为。

## 实际修改文件

- `src/serialforge/presentation/controllers/connection_builder.py`：连接主按钮使用既有 `BusyActionButton`。
- `src/serialforge/presentation/controllers/connection.py`：将 OPENING/CLOSING 投影为 `set_busy()`，其他状态静态。
- `src/serialforge/presentation/controllers/lifecycle.py`：将连接按钮加入唯一 MotionController fan-out。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0039-connection-button-activity-rail.md`、`tasks/plan.md`、`tasks/todo.md`：同步边界与任务。

## 架构审查与父代理整合

```text
架构角色  019fead1-7801-7a50-bcab-b52709c161dc  called before source edit; repeated wait timed out twice; closed
父代理    bounded audit GO：复用 shared BusyActionButton/MotionController；connection.py 保持 SessionState projection owner
```

独立简化结论：无需新增 abstraction；共享 painter/lifecycle 已是最小实现，按钮不读取 ViewModel、不创建 timer、不表达连接百分比。

## 验证证据

```text
scripts/check.ps1       PASS 132 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff
compileall               PASS src
UI152_CONNECTION         PASS CLOSED/OPENING/CLOSING/OPEN/ERROR; native BusyActionButton; text/accessibility retained
UI152_MOTION             PASS shared frame; busy rail; static/stop fallback
UI152_PIXEL              PASS three themes; 980x680; near_white=0
```

离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论。未运行持续 GUI/EXE 启动、真实连接 worker、UART/BLE/RTT/J-Link、
OTA、硬件、Windows 原生键盘/读屏/HIDPI、签名或正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。
嵌入式 C/C++ applicability：N/A；本轮是 Python/PySide6 desktop presentation 改动。

## 构建产物与最终交付

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，已在 checkout 内安全校验目标后覆盖，字节一致
- source revision：`local-ui-1.52`
- size：`47,825,940` bytes
- canonical/root SHA-256：`B150C618B8929943D24B87FC46F1B6FD7F3BF0DD1C8755AF2C3EA355F055458E`
- archive listing SHA-256：`3D64C4E65BB5425A6D3F39F9E93AF8A3F38CD9921CB37EF9E05500B336F2AD13`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

未决项：持续 GUI/EXE 启动、Windows 原生键盘/读屏/HIDPI、真实 UART/BLE/RTT/J-Link、OTA、硬件、签名和正式发行验收仍需相应授权与环境；
这些项目不能由静态、offscreen 或 engineering build 证据替代。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
