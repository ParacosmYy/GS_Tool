# ARCH-6t：BLE GATT 控件组合绑定

日期：2026-08-11  
父代理：Codex  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`

## 交付

新增 `BleControlBindings` 与 `ble_bindings_for()`，由 `connection_builder.py` 唯一组装 BLE
panel Qt 引用。BLE selection、action、connection/runtime、connection gate、preset、组合、
commands、lifecycle 和 terminal runtime 不再直接读取 BLE 动态 widget 字段；通知 pending/ref/
timer 状态留在原 owner。UART、Network、OTA/debug 边界不变。

## 行为保持

保留 BLE 扫描过滤、设备选择、服务缓存、连接 timeout/pair、characteristic 选择、特征属性、
read、notify/indicate、写入模式 normalize、通知超时回滚、`itemData`、signal、accessibility、
Tab 顺序、连接 gate、三主题和响应式布局。

## 验证证据

- `ARCH6T_COMPILE_PASS`
- `ARCH6T_RUFF_PASS`
- `BLE_DYNAMIC_WIDGET_OWNER_ONLY_PASS`
- `ARCH6T_BLE_VECTOR_PASS 30 themes=3 devices=1 characteristics=1`
- 3 主题 × 980/1180 × 4 workspace：`hmax=0`、`exact-white=0`、`near-white=0`
- `local-arch-6t` package/provenance/root EXE 覆盖已完成：canonical/root/root-latest 均为
  `47,968,345` bytes，SHA-256 `019CB04DE1400D4A03F5B51C662C31B7A4C25195EB83C844FA1EB8ECD0B23024`，
  archive listing SHA-256 `C8FE3A7CE1807A4E6B164CEB0EACB4D51120E15B9F8937FD4BBC8C604F57EBE4`，
  provenance verify pass；签名 `NotSigned`，`release_eligible=false`，hardware acceptance `not_run`

运行时仍报告既有 PySide6 fonts 目录 warning，但系统字体离屏渲染正常；GUI/EXE 启动、读屏、
真实 BLE 设备、硬件、刷写和部署验收未运行。

## 角色与简化审查

架构师线程 `019ff11a-1ba7-7fb1-ba14-7d9442051e7f` 已调用但超时关闭；父代理完成 owner、
状态隔离、依赖、初始化顺序、Qt 生命周期、行为保持、可访问性、主题和性能审查。复用既有
typed accessor，不新增 BLE 状态源、timer、backend 依赖或测试资产。

追加的独立架构师终审线程 `019ff123-7d9f-7dd0-81a8-e03b42397730` 也在限定窗口内超时关闭，
未计为独立通过；父代理的审查结论与可重复验证证据作为最终依据。

## assurance gate

`ARCH6T_FINAL_VALIDATION_PASS`、`BLE_DYNAMIC_WIDGET_OWNER_ONLY_PASS`、
`EMBEDDED_VENDOR_SOURCE_APPLICABILITY=N/A`、
`SIMPLIFICATION_ASSESSMENT=passed_parent_review_no_behavior_change`、
`AUTHORIZED_NON_DESTRUCTIVE_VERIFICATION=static+offscreen+provenance+hash`。

嵌入式 C/C++/固件适用性：N/A；无 public vendor source applicability；无硬件授权操作。
