# UI-1.54 交接：实时接收数据 activity rail

日期：2026-08-10  
父代理：Codex；共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；父代理是本轮唯一源码写入者。  
当前状态：源码、静态检查、数据状态离屏验证、三主题截图、onefile 打包和根目录 EXE 覆盖均完成；产物仍是 engineering build。

## 用户结果、范围与非范围

实时观测带的接收数据 QLabel 现在在原有文字下方绘制实时/历史 activity rail；active 时由共享 frame 驱动 pulse，静止/停止时保留静态轨道。
文字、字节数、窗口大小、source、tooltip、AccessibleDescription、`objectName` 和原生 QLabel 契约均保持不变。

本轮不复制 RX buffer、不绘制吞吐/百分比/设备进度、不改变接收/暂停/历史语义，也不新增 timer 或业务状态源。

## 实际修改文件

- `src/serialforge/presentation/data_activity_surface.py`：新增 `DataActivityProjection` 与 `DataActivitySurface(QLabel)`。
- `src/serialforge/presentation/controllers/terminal.py`：接收活动 widget 使用 surface，保留 `_data_activity_label` 名称。
- `src/serialforge/presentation/controllers/terminal_runtime.py`：从既有接收事实投影 source/最近字节/窗口字节。
- `src/serialforge/presentation/controllers/lifecycle.py`：投影 active 并加入共享 frame/stop fan-out。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0041-data-activity-surface.md`、`tasks/plan.md`、`tasks/todo.md`：同步边界。

## 架构审查与父代理整合

```text
架构角色  019feadf-ecb6-72f3-9f7a-d403d5ccedcf  called before source edit; repeated wait timed out twice; closed
父代理    bounded audit GO：surface 只装饰原生 QLabel；terminal_runtime/lifecycle 保持 projection 与 lifecycle owner
```

父代理简化结论：没有新增抽象或第二套时钟；字节数只生成 bounded deterministic decoration，不代表吞吐或进度。

## 验证证据

```text
scripts/check.ps1       PASS 133 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff
compileall               PASS src
UI154_DATA               PASS realtime/history; latest/window projection; native text/accessibility/objectName retained
UI154_MOTION             PASS active/static/frame/stop; shared MotionController fan-out
UI154_PIXEL              PASS three themes; 980x680; near_white=0
```

离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论。未运行持续 GUI/EXE 启动、真实 UART/BLE/RTT/J-Link、OTA、硬件、
Windows 原生键盘/读屏/HIDPI、签名或正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。
嵌入式 C/C++ applicability：N/A；本轮是 Python/PySide6 desktop presentation 改动。

## 构建产物与最终交付

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，已在 checkout 内安全校验目标后覆盖，字节一致
- source revision：`local-ui-1.54`
- size：`47,829,486` bytes
- canonical/root SHA-256：`DB64592CA9AE1A1789AB31B7255219DB3CD35138430356CD0F110FD24FE1BFAE`
- archive listing SHA-256：`A0F6FFE24B100AD6E33ABD750786ECECF8477293716843682F7ADE7E85399543`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

未决项：持续 GUI/EXE 启动、Windows 原生键盘/读屏/HIDPI、真实 UART/BLE/RTT/J-Link、OTA、硬件、签名和正式发行验收仍需相应授权与环境；
这些项目不能由静态、offscreen 或 engineering build 证据替代。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
