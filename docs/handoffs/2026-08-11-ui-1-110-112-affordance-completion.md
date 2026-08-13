# UI-1.110～UI-1.112 动作与字段 affordance 完整化

日期：2026-08-11  
范围：回放/连接/录制动作、UART 字段、发送区、批量命令编辑器、自定义连接配置对话框、确认对话框与快捷命令 QAction。  
父代理：Codex；父代理是本轮唯一写入者  
共享 checkout：D:\Workplace\Agent_Workplace\SerialForge；未检测到 .git，未创建/操作 worktree

## 结果与边界

- UI-1.110 为回放开始/暂停/停止、连接/刷新、独占/DTR/RTS、录制动作补齐 owner-local tooltip，并在状态变化时同步 accessible description 与 tooltip。
- UI-1.111 为 UART 字节间超时明确“秒”和 0 的不设定语义；发送输入、发送格式、CRLF 和发送按钮分别解释输入/格式化/动作边界，避免一个通用提示覆盖多个字段。
- UI-1.112 为批量命令编辑器、自定义连接 preset 编辑器、确认对话框和快捷命令菜单 QAction 补齐取消/保存/确认/填入的副作用说明；批量步骤 CRLF 只改变 payload，不自动执行或发送。
- 不新增业务状态、signal、timer、线程、依赖、设备 I/O、公开 API 或跨层 affordance registry；连接、发送、回放、记录、OTA contract-only 和 RTT/J-Link attach-only 边界保持不变。

## 角色与审查证据

- 每个增量及其修正均先调用 product、architecture、UI、development、validation、packaging 六个 Luna/max/Fast 只读角色；父代理是唯一写入者。所有角色均在限定等待窗口内超时后关闭，超时不视为通过。
- UI-1.111 独立复核调用 019fecd9-b22c-7c43-93d1-1c0410f274d1；UI-1.112 独立复核调用 019fecde-16d7-7430-a5e1-0257c9874db2。两者均超时关闭，未被计为通过；父代理完成五轴审查、架构边界、行为保持的简化评估与验证结果整合。
- 本轮未修改嵌入式 C/C++、固件、MCU、BSP/HAL、RTOS、ISR/DMA、驱动或目标协议实现；嵌入式厂商资料不适用于本轮 Python/PySide6 presentation 文案改动，不声明 MISRA、ISO 26262 或硬件合规。

## 验证

scripts/check.ps1                                      PASS (154 files <=1000; 3 themes; 22 semantic tokens; legacy_qss_literals=0)
uv run --locked --extra dev python -m compileall -q src scripts  PASS
uv run --locked --extra dev ruff check src scripts              PASS
UI110_ACTION_TOOLTIP_VECTOR_PASS                       actions=9 replay_states=6 recording_states=5 connection_states=4 synced=1
UI111_SEND_UART_AFFORDANCE_VECTOR_PASS                  inter_byte=0-unbounded semantics preserved states=4 field_roles=4 synced=1
UI112_DIALOG_ACTION_AFFORDANCE_VECTOR_PASS              themes=3 dialog_types=3 quick_action_states=2
UI110_THEME_ACTION_RENDER_PASS                          buttons=40 themes=3 sizes=2 near_white=0 close_lifecycle=pass
UI111_THEME_RENDER_AFFORDANCE_PASS                      buttons=40 combos=27 themes=3 sizes=2 near_white=0 close_lifecycle=pass
uv run --locked --extra dev python scripts/provenance.py verify --manifest dist/release/0.1.0/core/onefile/PROVENANCE.json  PASS

所有 Qt 组合根脚本均使用 offscreen、未调用 show；退出路径关闭窗口、等待线程池并关闭 application service。未运行可见 GUI/EXE startup、读屏、HIDPI、真实 UART/网络/BLE/RTT/J-Link、OTA、签名或正式发行验收。

## 打包

artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (pending-user-close; two running instances hold the old file)
source revision: local-ui-1.112
size: 47,932,571 bytes
SHA-256: EB975C422B68D618C6B0E09669D8843689752C0153E540193F213B664FE0254E
archive listing SHA-256: C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run

旧根目录文件仍为已运行的 UI-1.109 副本，不能在进程持有句柄时强制覆盖。关闭 PID 46108、49236 后，使用 canonical 文件覆盖根目录并再次比较 SHA-256 即可完成最后交付动作。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
