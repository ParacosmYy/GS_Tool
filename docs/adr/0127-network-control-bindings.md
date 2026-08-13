# ADR 0127：网络端点控件组合绑定边界

日期：2026-08-11  
状态：accepted / incremental migration

## 背景

UART 表单已经通过 `UartControlBindings` 收敛，但 TCP Client、TCP Server、UDP 和 RTT
仍由多个 controller 直接读取网络 panel 的动态 `window._...` 字段。尤其是 TCP Server
peer selector、allowlist 与端点摘要横跨连接 gate、发送命令、preset、生命周期和终端事件，
容易让构造细节泄漏为隐式共享依赖。

## 决策

新增 `presentation/connection_bindings.py` 中 frozen/slots 的 `NetworkControlBindings`，
把同一个“网络端点”面板的纯 Qt wiring 作为一个组合边界。它包含共享远端/本地端点、超时、
UDP 限制、RTT selector、TCP Server allowlist/peer selector、panel/title/label/hint 引用。

- `controllers/connection_builder.py` 是唯一构造和 signal 接线 owner；
- `connection.py`、`connection_runtime.py`、`connection_presets.py`、`composition.py`、
  `lifecycle.py`、`commands.py` 和 `terminal_runtime.py` 通过 `network_bindings_for()` 消费；
- bundle 只持有 Qt widget 引用，不持有 `server_target_explicit`、默认值标志、Session、
  transport config、ViewModel、callback、timer、peer snapshot 或网络探测策略；
- `server_target_explicit`、`server_defaults_applied` 和 `rtt_defaults_applied` 继续由既有
  owner 保持为 presentation projection state，不能塞进 bundle；
- 初始化早期 bundle 缺失时，projection 安全返回或配置路径显式报错，不伪造端点；
- 不拆出 TCP/UDP/RTT 三套近似 bundle，避免把一个共享表单拆成重复 wiring；后续若出现独立
  的 BLE surface，再以不同 feature boundary 单独迁移。

## 结果与验证

本切片保持 TCP/UDP/RTT/Server 的配置字段、preset、allowlist 校验、LAN 确认失效、peer
selector、发送目标、连接 gate、默认端点、section 显隐、Tab 顺序、accessibility 和主题不变。
`ARCH6S_NETWORK_VECTOR_PASS 30` 覆盖 3 主题 × 980/1180 × 四工作区与五种网络相关模式；
每组 horizontal maximum=0、exact-white=0、near-white=0。compileall、Ruff 和动态 widget
owner 审计通过。

## 审查记录

架构师角色线程 `019ff110-8695-7383-b627-5c85caee76c4` 已调用，在两个限定等待窗口内未返回，
随后关闭，未计为独立通过。父代理完成 owner、依赖方向、状态隔离、Qt 生命周期、行为保持、
accessibility、主题对称性、性能和简化审查。该切片复用 ARCH-6r 的 typed binding 模式，
没有新增计时器、事件总线、依赖或测试资产。

嵌入式 C/C++/固件适用性：N/A。本轮只修改 Python/PySide6 presentation；没有适用的公开一手
厂商目标资料、硬件操作或刷写动作，不作 MISRA/ISO/认证合规声明。
