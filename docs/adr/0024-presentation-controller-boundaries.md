# ADR 0024：Presentation 控制器边界与 MainWindow 瘦身

状态：已接受；按增量切片迁移，迁移期间保留兼容性接线，行为不变。  
日期：2026-08-10

## 背景

`src/serialforge/presentation/main_window.py` 已增长到约 4,600 行，同时承担窗口初始化、布局构建、
连接配置、BLE/server 交互、终端/记录/发送、协议解析、Component、Dataset、Curve、回放、批量命令、
状态投影、动效和生命周期。继续把新 UI 状态直接塞进这个文件会让模块边界失真，难以单独审查和替换。

用户要求主文件主要负责初始化与接口接线，并保证模块高内聚、低耦合。本 ADR 只改变 presentation 内部
组织，不改变 ViewModel、application/domain/infrastructure 端口或当前运行语义。

## 决策

- `main_window.py` 作为 composition shell：负责创建 `QMainWindow`、应用主题、创建各 presentation
  controller、连接跨控制器回调、统一生命周期和关闭顺序；目标是不再承载具体工作区业务流程。
- 在 `src/serialforge/presentation/controllers/` 下按稳定变化边界拆分：
  - `header.py`：品牌、Session 状态、错误横幅、低动效/动效控制与状态灯；
  - `workspace.py`：Tab、滚动页面、切换淡入和可见性/最小化策略；
  - `connection.py`：UART/TCP/UDP/BLE/RTT 配置、发现、BLE 服务/通知、TCP Server peer 和连接 gate；
  - `terminal.py`：终端工具栏、接收预览、暂停/记录、发送栏和快捷历史；
  - `protocol.py`：协议编辑、帧预览、Component、Dataset、Curve 和派生来源状态；
  - `replay.py`：历史回放控制及历史来源投影；
  - `commands.py`：快捷命令、批量命令编辑/执行/停止及结果表面。
- 稳定的跨控制器边界放在 `presentation/contracts.py`：使用类型化 DTO、`Callable` 回调端口和最小
  生命周期接口；禁止把整个 `MainWindow` 作为“上帝对象”传给每个 controller。
- controller 只依赖 `SessionViewModel`、自己的 Qt parent 和明确 callbacks；controller 之间不直接
  访问彼此的控件。跨区状态通过 ViewModel signal、不可变快照和 shell 回调传播。
- Qt 对象由 shell 或 controller 的 parent 所有；每个 controller 提供幂等 `close()`/`suspend()`（按
  需要）入口，保证窗口关闭、隐藏、最小化和低动效边界继续有效。
- 迁移采用“先抽取构建与契约，再抽取事件处理，再删除兼容别名”的顺序。迁移期间允许 shell 暂时保存
  兼容性 widget 引用，但新 controller 不得新增对这些别名的依赖。
- `viewmodels.py` 继续是 application 到 Qt 的唯一状态适配边界；controller 不导入 OTA adapter、
  debug backend、socket、pyserial、vendor SDK、密钥或领域业务实现。

## 备选方案

### 多个 mixin 直接拼接 MainWindow

拒绝：虽然能减少单文件行数，但所有 mixin 继续共享隐式 `self` 字段和方法，依赖关系不可见，容易把
connection、protocol 和 command 状态重新耦合成一个隐形上帝对象。

### 一个 `MainWindowContext` 包含所有控件和服务

拒绝：这只是把大类搬进一个大字典/对象，消费者仍可随意跨边界读取控件；接口不可验证，也无法单独替换
工作区。

### 一次性重写整个窗口

拒绝：约 4,600 行同时重排会混合结构变更与行为变更，难以定位 Qt 初始化顺序、关闭 fence、BLE 异步
回调和历史来源 gate 的回归。采用可编译的垂直切片。

## 后果

- 每个工作区可以独立维护主题、无障碍、状态投影和生命周期；新增 OTA/debug UI 时不会再进入主窗口。
- 迁移期间文件数量会增加，短期存在兼容引用；每个切片必须通过静态检查并在交接文档记录删除了哪些耦合。
- controller 之间的回调契约需要谨慎设计，避免用通用 `object` 或无语义 `dict` 逃避类型边界。
- 真实 GUI、读屏、窄宽度、HIDPI 和动效性能仍需用户授权后验收；静态拆分不能代替运行时证据。

## 增量迁移顺序

1. 建立 `contracts.py` 和 controllers 包，先抽取协议/遥测面板构建与状态表面；
2. 抽取终端/发送/记录和命令批量面板；
3. 抽取连接/BLE/server 配置与异步回调；
4. 抽取 header/workspace 动效和统一生命周期；
5. 将 MainWindow 收敛为 shell，删除兼容别名，补齐边界审计与打包。
