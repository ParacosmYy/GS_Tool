# ADR 0128：BLE GATT 控件组合绑定边界

日期：2026-08-11  
状态：accepted / incremental migration

## 背景

UART 与网络端点 panel 已分别收敛为 typed wiring bundle。BLE GATT 仍有扫描过滤、设备、
服务缓存、特征、通知和写入模式等控件，被 connection、terminal、BLE action、commands、
lifecycle、preset 和组合 root 分散读取；同时通知 pending/ref/timer 又是有真实生命周期的
状态，不能被错误地塞进控件 bundle。

## 决策

新增 `presentation/connection_bindings.py` 中 frozen/slots 的 `BleControlBindings`，只保存
BLE panel/title/hint 与下列 Qt widget 引用：扫描/连接 timeout、扫描按钮、name/service filter、
device combo、pair/cache、characteristic combo/properties、read/notify、write mode。

- `controllers/connection_builder.py` 是唯一构造与 signal 接线 owner；
- `ble_selection.py`、`controllers/ble.py`、`connection_runtime.py`、`connection.py`、
  `connection_presets.py`、`composition.py`、`commands.py`、`lifecycle.py` 和
  `terminal_runtime.py` 通过 `ble_bindings_for()` 消费；
- `_ble_notification_pending`、`_ble_notification_ref`、`_ble_notification_timer` 保持在
  既有 BLE action/lifecycle owner，不进入 bundle；
- bundle 不携带 ViewModel、BLE backend、设备句柄、密钥、characteristic snapshot、callback、
  timer、扫描策略或业务状态；初始化早期缺失时 projection 安全返回或配置路径显式报错；
- 不改变 BLE signal、扫描/连接 gate、设备与特征选择、通知确认/超时回滚、写模式 normalize、
  `itemData`、accessibility、Tab 顺序或主题。

## 结果与验证

`ARCH6T_BLE_VECTOR_PASS 30` 覆盖 3 主题 × 980/1180 × 四工作区与 BLE runtime；模拟设备和
characteristic 的扫描结果投影、服务/特征投影、写模式读取、BLE panel 显隐均通过。每组
horizontal maximum=0、exact-white=0、near-white=0；compileall、Ruff 和 BLE widget owner
审计通过。

## 审查记录

架构师角色线程 `019ff11a-1ba7-7fb1-ba14-7d9442051e7f` 已调用，在两个限定等待窗口内未返回，
随后关闭，未计为独立通过。父代理完成 owner、状态隔离、依赖方向、Qt 生命周期、行为保持、
accessibility、主题、性能与简化审查。复用 ARCH-6r/6s accessor 模式，没有新增 timer、依赖
或测试资产。

嵌入式 C/C++/固件适用性：N/A。本轮只修改 Python/PySide6 presentation；没有适用的公开一手
厂商目标资料、硬件操作或刷写动作，不作 MISRA/ISO/认证合规声明。
