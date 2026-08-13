# ADR 0126：UART 控件组合绑定边界

日期：2026-08-11  
状态：accepted / incremental migration

## 背景

`MainWindow` 已经收敛为组合 shell，但连接页的 UART 控件仍以多个动态
`window._...` 字段被多个 controller 读取。这会把控件构造细节传播到连接、
协议、生命周期、preset 和终端逻辑，增加重命名、初始化顺序和后续工具站扩展的风险。

## 决策

新增 `presentation/connection_bindings.py`，以 frozen/slots 的
`UartControlBindings` 保存一个 UART 表单所需的 Qt widget 引用，并以
`uart_bindings_for()` 提供不依赖 `MainWindow` 类型的安全读取边界。

- `controllers/connection_builder.py` 是唯一构造 owner，并负责把已有控件装入 bundle；
- `connection.py`、`connection_runtime.py`、`composition.py`、`connection_presets.py`、
  `lifecycle.py`、`protocol_config.py` 和 `terminal_runtime.py` 只通过 bundle 读取 UART 控件；
- bundle 只包含 presentation wiring，不包含 ViewModel、domain/application DTO、业务状态、
  callback、timer、设备句柄、密钥或传输策略；
- 初始化早期 bundle 尚不存在时，投影型 controller 安全返回；需要配置的路径给出明确的
  `ConfigurationError`/`ValueError`，不伪造端口或配置；
- builder 内保留旧字段作为构造阶段局部 facade，待后续 feature bundle 迁移完成后再删除，
  避免一次性重写连接页行为。

## 结果与验证

该切片不改变 UART 选项、preset、Modbus timing、连接 gate、section 显隐、Tab 顺序、焦点、
accessibility 或主题行为。真实组合根的 UART preset/配置构建、panel 隐藏/恢复、focus restore、
三主题 × 980/1180 × 四 Tab 共 24 组均通过；每组 horizontal maximum=0，exact-white/near-white=0。
`compileall`、Ruff、`scripts/check.ps1`、源码行数门禁和 onefile provenance 通过。

## 审查记录

架构师角色线程 `019ff103-cb0d-7610-8306-66dffee0b675` 已调用；在两个限定等待窗口内未返回，
随后关闭。独立嵌入式 assurance reviewer 线程 `019ff108-4743-7c21-989d-861a1dd8a496` 同样未在
限定窗口内返回；父代理完成 owner、依赖方向、生命周期、行为保持、可访问性、主题对称性和
简化复核。复用既有 `workspace_bindings_for()` 的 typed wiring 模式，没有新增时钟或状态源。

嵌入式 C/C++/固件适用性：N/A。本轮只修改 Python/PySide6 presentation，暂无适用的公开一手
厂商目标资料；未运行硬件、刷写或部署，不作 MISRA/ISO/认证合规声明。
