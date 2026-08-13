# ADR 0131：命令批处理控件绑定边界

- 日期：2026-08-11
- 状态：accepted
- 范围：presentation wiring；不改变命令业务协议

## 背景

命令管理页的批处理 combo、动作按钮、状态 rail、结果表和空态卡片由
`controllers/terminal.py` 创建，但此前由 commands、connection、selection、composition
和 lifecycle controller 直接读取 `MainWindow` 的动态 Qt 属性。动态属性让控件构建、跨页
状态投影和共享动效生命周期形成隐式耦合，也使后续 OTA/debug 工具站扩展容易继续把控件
塞进主窗口 facade。

## 决策

新增 `presentation/command_bindings.py` 的 frozen/slots
`CommandBatchControlBindings`。`controllers/bootstrap.py` 在
`workspace.py` 完成命令页构建后唯一组装 bundle；所有跨 controller 读取通过
`command_batch_bindings_for()` 完成。

bundle 只保存以下 Qt wiring：批处理 combo、new/edit/delete/run/stop 按钮、状态 rail、
结果表和空态卡片。批处理 catalog、snapshot、ViewModel、执行/停止策略、timer、callback、
transport handle 和连接事实仍由原 owner 持有。`terminal.py` 在迁移期可以保留动态字段，
但它们只服务于构建阶段装配；新的跨 controller 代码不得引用这些字段。

## 依赖与时序

`workspace.py` 先构建命令页，随后 composition root 创建 command bundle，再连接 ViewModel
信号并执行首屏 projection。因此 selector、connection gate、Tab 顺序和 lifecycle fan-out
在首个用户事件前都能读取 typed bundle。accessor 对初始化早期缺失安全返回；命令渲染和
连接状态 projection 在缺失时不执行部分更新。

## 行为保持

本决策不新增状态源、线程、timer、事件总线、设备 I/O、动作或主题 token。它不改变批处理
选择、编辑、删除、执行、停止、空态/结果表互斥可见性、共享 120Hz MotionController、
可访问性或 980/1180 响应式约束。OTA、AES 安全、XMODEM/YMODEM/TFTP、RTT/J-Link 仍保持
现有 contract-only/attach-only 边界。

## 验证与审查记录

- `scripts/check.ps1`、compileall、Ruff、source-limit、theme-token audit：通过。
- Qt offscreen 真实组合根：bundle identity、空态/结果表几何、980×720 station layout：通过。
- 未创建、修改或运行测试专用资产；未执行可见 GUI、EXE 启动、真实传输或硬件操作。
- 架构师线程 `019ff141-f6a2-74e2-b885-57d12f3c11fa` 已调用，但在限定窗口内超时关闭，未形成独立报告；父代理完成依赖方向、初始化时序、Qt 生命周期、accessibility、性能与行为保持型简化审查。
- 本轮为 Python/PySide6 presentation 变更，嵌入式 C/C++/固件 public-vendor-source applicability：N/A；不作 MISRA、ISO 26262 或正式发行合规声明。
