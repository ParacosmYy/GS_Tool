# ARCH-6s：网络端点控件组合绑定

日期：2026-08-11  
父代理：Codex  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`

## 交付

在 `presentation/connection_bindings.py` 新增 frozen/slots `NetworkControlBindings` 与
`network_bindings_for()`。`connection_builder.py` 唯一组装同一网络端点面板的 Qt 引用；
connection/runtime、preset、composition、lifecycle、commands 和 terminal runtime 不再
直接读取网络动态 widget 字段。TCP Client、TCP Server、UDP、RTT 共用该面板边界，BLE 保持
独立，后端与 OTA/debug contract 边界不变。

## 行为保持

保留远端/本地 host/port、连接/读/写超时、UDP 最大报文、RTT channel、TCP Server allowlist、
LAN 确认、最大 client、peer 发送目标、网络默认端点、preset、server readiness、发送目标、
连接 gate、Tab 顺序、focus/accessibility、主题和无横向溢出语义。`server_target_explicit`、
默认值标志等状态仍由原 owner 持有，不进入 wiring bundle。

## 验证证据

- `ARCH6S_COMPILE_PASS`
- `ARCH6S_RUFF_PASS`
- `NETWORK_DYNAMIC_WIDGET_OWNER_ONLY_PASS`
- `ARCH6S_NETWORK_VECTOR_PASS 30 themes=3 network=5 transport_modes`
- 3 主题 × 980/1180 × 4 workspace：`hmax=0`、`exact-white=0`、`near-white=0`
- `local-arch-6s` onefile 已完成：canonical/root/root-latest 均为 `47,965,543` bytes，SHA-256
  `E9F3D230B75FFA54D61F5190556CE3393F308A4CC75E53B303DEA01E74BD71CA`，archive listing SHA-256
  `C8FE3A7CE1807A4E6B164CEB0EACB4D51120E15B9F8937FD4BBC8C604F57EBE4`，provenance verify pass；
  `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。根目录两个 EXE 已覆盖并
  与 canonical 字节一致。

运行时仍报告既有 PySide6 fonts 目录 warning，但系统字体离屏渲染正常；GUI/EXE 启动、读屏、
真实设备、硬件、刷写和部署验收未运行。

## 角色与简化审查

架构师线程 `019ff110-8695-7383-b627-5c85caee76c4` 已调用但超时关闭；父代理完成 owner、
依赖、状态隔离、初始化顺序、Qt 生命周期、行为保持、可访问性、主题和性能审查。复用上一
切片的 accessor，不新增 transport abstraction、timer、状态源或测试资产。

嵌入式 C/C++/固件适用性：N/A；无 public vendor source applicability；无硬件授权操作。
