# UI-1.114 派生动作 signal rail 与 lifecycle fan-out

日期：2026-08-11  
范围：协议/组件/Dataset 页面四个既有派生动作的主题化动态 signal rail。  
父代理：Codex；父代理是本轮唯一写入者  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；未检测到 `.git`，未创建/操作 worktree

## 结果与边界

- `加载组件配置 / Codec`、`导出 CSV`、`加载 Dataset`、`导出 Dataset CSV` 四个既有按钮复用 `ActionRailButton`。
- `protocol.py` 继续负责创建按钮和注入 callback；`protocol_config.py` 继续负责 derived source enabled/disabled projection；`lifecycle.py` 只负责纳入已有 frame/stop fan-out。
- 无新增 timer、MotionController、状态源、动作 registry、DTO、线程、I/O 或业务判断；按钮回调、布局、焦点/Tab、主题 token、原始终端/记录、OTA contract-only 和 RTT/J-Link attach-only 边界不变。
- 禁用态动作专属 tooltip 与 accessible description 仍由 UI-1.113 的 immutable contract 和既有来源原因组成，动画不改变 gate。

## 架构与审查证据

- 已按用户要求尝试调用 Luna/max/Fast 架构师角色；本轮调用在限定等待窗口内超时，未将超时记作通过。父代理依据既有 `ActionRailButton` 与 lifecycle contract 完成高内聚/低耦合、行为保持、复用/简化和行数审查。
- 独立 UI 复核线程同样超时并关闭，未计为通过；没有伪造独立 review 通过结论。
- 本轮仅修改 Python/PySide6 presentation，不包含 MCU、固件、嵌入式 C/C++、BSP/HAL、RTOS、ISR/DMA、驱动或硬件协议实现。公开厂商资料不适用，不声明 MISRA、ISO 26262、硬件或安全认证合规。

## 验证

```text
scripts/check.ps1                                      PASS (155 files <=1000; 3 themes; 22 semantic tokens; 19 selectors; legacy_qss_literals=0)
.venv/Scripts/python.exe -m compileall -q src          PASS
.venv/Scripts/ruff.exe check src                       PASS
UI114_DERIVED_ACTION_RAIL_VECTOR_PASS                 themes=3 actions=4 lifecycle=shared stop=pass
.venv/Scripts/python.exe scripts/provenance.py verify  PASS
```

vector 覆盖真实 `build_protocol_panel()`、三套主题、enabled/disabled 文案同步、`ActionRailButton` 类型、frame 绘制入口、lifecycle 收集和 `stop()` 静态回退。只使用短时 Qt offscreen 组件，没有启动持续 GUI、EXE、真实设备、网络、OTA、J-Link 或硬件；未创建或运行测试专用资产。PySide6 环境输出缺少字体目录提示，但不影响断言。

## 打包

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (pending-user-close; PID 46108、49236 持有旧文件)
source revision: local-ui-1.114
size: 47,934,511 bytes
SHA-256: 7608148F5BE4E3F30F82379C78A94B749320E30809A2B32A5B7ECBEF8CBC14C6
archive listing SHA-256: 5137DE350B55E756B354569A7ADABD60F5C069FAC0074A37A25E20B5DD78A8A2
Python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

根目录覆盖尝试因两个旧运行实例持有文件句柄而被 Windows 拒绝；未强制结束进程。关闭实例后可将 canonical 文件复制为根目录 `SerialForge.exe`，再核对根目录 SHA-256 与上方一致。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
