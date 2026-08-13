# UI-1.116 主窗口 action signal rail 统一

日期：2026-08-11  
范围：连接、协议、终端、发送、回放和批量页面的普通动作按钮视觉统一。  
父代理：Codex；父代理是本轮唯一写入者  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；未检测到 `.git`，未创建/操作 worktree

## 结果与边界

- 16 个普通动作复用 `ActionRailButton`：连接配置保存/删除、BLE 读取、协议应用/重置、回放开始/停止、错误清除、终端清空、发送、快捷命令保存、发送历史清除、批量新建/编辑/删除/执行。
- `BusyActionButton` 的连接、刷新、BLE 扫描、原始记录、回放暂停和批量停止保持原有 busy projection。
- 各 owner 继续创建自身按钮和绑定 callback；`lifecycle.py` 只显式纳入 frame/stop fan-out。没有新增 timer、MotionController、registry、状态源、DTO、线程、I/O 或跨层 callback。
- 原生 `QPushButton` 继承关系、文本、objectName、enabled gate、焦点/Tab、signal payload、连接/发送/记录/回放/批量业务语义保持。

## 架构与审查证据

- 已按用户要求调用 Luna/max/Fast 架构师角色，但本轮在限定等待窗口内超时，未将超时记作通过；父代理完成 owner 边界、行为保持、复用/简化、lifecycle 和行数审查。
- 独立质量审查线程同样超时并关闭，未计为通过；没有伪造独立 review 结论。
- 本轮仅修改 Python/PySide6 presentation，不包含 MCU、固件、嵌入式 C/C++、BSP/HAL、RTOS、ISR/DMA、驱动或硬件协议实现。公开厂商资料不适用，不声明 MISRA、ISO 26262、硬件或安全认证合规。

## 验证

```text
scripts/check.ps1                                      PASS (156 files <=1000; 3 themes; 22 semantic tokens; 19 selectors; legacy_qss_literals=0)
.venv/Scripts/python.exe -m compileall -q src          PASS
.venv/Scripts/ruff.exe check src                       PASS
UI116_ACTION_RAIL_VECTOR_PASS                          themes=3 actions=16 lifecycle=shared stop=pass
UI116_AUDIT_RENDER_PASS                                1180x780 command-page PNG; actions_in_lifecycle=pass
.venv/Scripts/python.exe scripts/provenance.py verify  PASS
```

离屏 vector 使用真实 composition root，逐主题验证 16 个动作均为 `ActionRailButton`、被 lifecycle 收集并能 stop；视觉审计覆盖命令管理页和共享主题表面。前一次审计脚本曾因错误调用 `window._motion_surfaces()` 失败，已修正为 `lifecycle._motion_surfaces(window)` 并以 `finally` 清理上下文，随后审计通过。未启动持续 GUI、真实设备、网络、OTA、J-Link 或硬件；未创建或运行测试专用资产。PySide6 环境输出缺少字体目录提示，但不影响断言。

## 打包

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (pending-user-close; PID 46108、49236 持有旧文件)
source revision: local-ui-1.116
size: 47,937,619 bytes
SHA-256: D80631FE5B2F9DCF213ADCAB16083BBAC7BAEA49EDB1CC39A683F7BFCE494448
archive listing SHA-256: D11AE67CCE33F851AC4DA36450BFAADF5247F1FF5776A428677A2F27E662E49A
Python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

根目录覆盖仍因两个旧运行实例持有文件句柄而未完成；未强制结束进程。关闭实例后可将 canonical 文件复制为根目录 `SerialForge.exe`，再核对根目录 SHA-256 与上方一致。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
