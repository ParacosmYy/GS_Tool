# UI-1.77 连接方式 Section 可见性同步交接

日期：2026-08-10  
工作区：`D:\Workplace\Agent_Workplace\SerialForge`  
父代理：Codex；本轮唯一写入者  

## 交付结果

修复连接配置页 transport 切换后的孤立 section 标题行：

- UART：`_uart_title` 与 `_uart_panel` 同步。
- TCP Client、TCP Server、UDP、J-Link RTT：`_network_title` 与 `_network_panel` 同步。
- BLE GATT：`_ble_title` 与 `_ble_panel` 同步。

三个标题保留 `role="section"`。修改只涉及 presentation builder/runtime，不改变连接 DTO、ViewModel、协议、焦点、Tab、accessibility、
既有面板 fade、MotionController、主题 token 或业务状态。

## 修改文件

- `src/serialforge/presentation/controllers/connection_builder.py`
- `src/serialforge/presentation/controllers/connection_runtime.py`
- `docs/ARCHITECTURE.md`
- `docs/CONSTRAINTS.md`
- `docs/adr/0064-transport-section-visibility.md`
- `tasks/plan.md`
- `tasks/todo.md`
- `README.md`

## 评审与简化

```text
产品角色       019febba-d48a-7ce3-8f92-c4280b62007f  called before source edit; wait timed out; closed
架构角色       019febba-d4d9-76d3-84b5-c21d67acea2c  called before source edit; wait timed out; closed
UI 设计角色    019febba-d521-7201-a652-e57bc2e906cb  called before source edit; wait timed out; closed
开发角色       019febba-d56d-7913-b11e-5c0ac3762849  called before source edit; wait timed out; closed
验证角色       019febba-d5bb-7c73-947e-2a1bf2997ed6  called before source edit; wait timed out; closed
打包/流程角色  019febba-d60d-7001-a0a0-db1a060ff5df  called before source edit; wait timed out; closed
独立质量复核   019febbc-a397-73c1-9531-e662d1ff7d99  called after implementation; wait timed out; closed
```

六角色与独立复核没有返回完整报告，父代理未将其视为通过，并完成五轴复核：标题/panel projection 正确；三个显式引用易读且无重复状态机；
builder/runtime 边界保持高内聚低耦合；无新增输入/网络/存储/密钥风险；无新增 timer 或性能负担。

简化结论：复用 `on_transport_changed()` 已存在的 `is_uart/is_ble` 布尔 projection 是最小完整修复，不需要抽象 helper、事件总线或新的动画 owner。

## 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src          PASS
UI177_TRANSPORT_SECTION_PASS theme=star_trail modes=6 parity=pass
UI177_TRANSPORT_SECTION_PASS theme=moonlit_ocean modes=6 parity=pass
UI177_TRANSPORT_SECTION_PASS theme=sakura_night modes=6 parity=pass
UI177_TRANSPORT_SECTION_VECTOR_PASS themes=3 modes=6 startup=pass lifecycle=pass
```

向量使用真实 composition root 与 `QApplication`/Qt offscreen，主窗口未 `.show()`；缺失 PySide6 虚拟字体目录的 Qt warning 已记录，
不据此宣称真实 Windows 字体/HIDPI 通过。未运行完整 GUI、读屏、EXE 启动、真实硬件/网络、OTA、签名或正式发行验收；未创建、修改或运行
unit test、mock、fixture、harness 或 test-only 资产。嵌入式 C/C++ 适用性：N/A。

## 打包

本轮已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.77` 重新生成并覆盖根目录文件：

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.77`
- size：`47,892,808` bytes
- SHA-256：`CBA24073D49B6CA1C6FDC82C873DEBE19FBCB619D2C2CC4580CD5406D9772A88`
- archive listing SHA-256：`29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
