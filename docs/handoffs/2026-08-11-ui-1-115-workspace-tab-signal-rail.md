# UI-1.115 工作区 Tab signal underline 与 halo

日期：2026-08-11  
范围：工作区四个原生 Tab 的选中态动态视觉反馈。  
父代理：Codex；父代理是本轮唯一写入者  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；未检测到 `.git`，未创建/操作 worktree

## 结果与边界

- 新增 `presentation/workspace_tab_surface.py:AnimatedWorkspaceTabBar`，继承原生 `QTabBar`，在 native paint 后绘制当前 Tab 的主题 signal underline/halo。
- `workspace.py` 通过 `setTabBar()` 显式注入；`lifecycle.py` 只将该 widget 纳入已有 shared frame/stop fan-out。
- 原生 `QTabWidget` 的 currentChanged、键盘、焦点、icon、滚动按钮、Tab 文本和 QSS 行为保持；surface 不拥有导航信号、ViewModel 状态或业务 gate。
- 无新增 timer、MotionController、registry、线程、I/O、跨层 callback 或业务 DTO；`stop()`/reduced-motion/paused/hidden/minimized/close 继续静态回退。

## 架构与审查证据

- 已按用户要求调用 Luna/max/Fast 架构师角色，但本轮在限定等待窗口内超时，未将超时记作通过；父代理完成 owner 边界、行为保持、复用/简化、绘制边界和行数审查。
- 独立质量审查线程同样超时并关闭，未计为通过；没有伪造独立 review 结论。
- 本轮仅修改 Python/PySide6 presentation，不包含 MCU、固件、嵌入式 C/C++、BSP/HAL、RTOS、ISR/DMA、驱动或硬件协议实现。公开厂商资料不适用，不声明 MISRA、ISO 26262、硬件或安全认证合规。

## 验证

```text
scripts/check.ps1                                      PASS (156 files <=1000; 3 themes; 22 semantic tokens; 19 selectors; legacy_qss_literals=0)
.venv/Scripts/python.exe -m compileall -q src          PASS
.venv/Scripts/ruff.exe check src                       PASS
UI115_WORKSPACE_TAB_RAIL_VECTOR_PASS                  themes=3 tabs=4 native_navigation=pass stop=pass
UI115_AUDIT_RENDER_PASS                                1180x780 PNG
.venv/Scripts/python.exe scripts/provenance.py verify  PASS
```

离屏 vector 使用真实 composition root 和 `MainWindow`，逐主题切换四个 Tab，验证 `setTabBar()`、current index、Tab geometry、frame/stop；视觉审计生成临时 PNG 仅用于本轮检查。未启动可见持续 GUI、真实设备、网络、OTA、J-Link 或硬件；未创建或运行测试专用资产。PySide6 环境输出缺少字体目录提示，但不影响断言。

## 打包

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (pending-user-close; PID 46108、49236 持有旧文件)
source revision: local-ui-1.115
size: 47,937,677 bytes
SHA-256: 8B10ABABAB674833AE4FCA663D9ACD81AF59225F53C046F11BC130410249B786
archive listing SHA-256: D11AE67CCE33F851AC4DA36450BFAADF5247F1FF5776A428677A2F27E662E49A
Python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

根目录覆盖仍因两个旧运行实例持有文件句柄而未完成；未强制结束进程。关闭实例后可将 canonical 文件复制为根目录 `SerialForge.exe`，再核对根目录 SHA-256 与上方一致。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
