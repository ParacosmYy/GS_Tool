# ADR 0020：Presentation-owned unified error（M5l）

状态：已接受；M5l 代码切片，真实设备和正式发行门后置。

## 背景

`SessionViewModel` 已经用 `ErrorInfo` 管理 transport、session、recording、replay、component 和
configuration 错误，并通过 `error_changed` 通知 MainWindow。但 MainWindow 的本地表单校验走
`_show_local_error()`，直接写 QLabel/status bar；ViewModel 的 `_clear_error()` 不知道这份本地状态，
导致错误栏可能在成功操作后残留或被两套状态交替覆盖。

## 决策

- `SessionViewModel` 是 presentation error state 的唯一 owner，内部持有 `ErrorInfo | None`；
- 新增 `show_local_error(message)`，复用 `ErrorCode.CONFIGURATION` 和 recoverable 标记，不新增
  domain 错误类别；
- `error_changed` 传递结构化 `ErrorInfo` 或 `None`，MainWindow 只负责渲染 message、detail tooltip
  和可见性；
- MainWindow `_show_local_error()` 只委托给 ViewModel；清除按钮仍调用 ViewModel 的唯一
  `clear_error()`/`_clear_error()`；
- 异步事件的 session/replay source gate、最后一个被接受的错误覆盖策略、transport/recorder/
  domain 错误边界保持不变；本轮不引入错误队列或 session-scoped 聚合器。

## 简化与边界

错误状态仍只保留一个当前槽位；无关异步错误可能覆盖当前错误，后续若需要需另开 scope/priority
设计。清除错误时 MainWindow 清空状态栏显示，但不改变 ViewModel 的正常 status 状态机。`detail`
只作为 UI tooltip，不在普通错误栏中展开；不把错误逻辑放入 transport、recorder、codec 或 domain。

## 验证要求

使用当前 composition 与 Qt offscreen 的 inline vector，不创建测试文件、mock、fixture 或 harness：

1. `show_local_error()` 产生 `ErrorInfo(CONFIGURATION)`，MainWindow 展示并显示错误栏；
2. ViewModel `clear_error()` 发出 `None`，错误栏、tooltip 和状态栏都清理；
3. MainWindow 本地校验入口与异步结构化错误共用同一状态，detail 可读；
4. 窗口关闭后 worker 无残留；静态、模块导入和现有包门继续通过。

真实 UART open/read/write、BLE、TCP/UDP acceptance、实时 Modbus/MAVLink stream boundary、J-Link
RTT、干净 Windows 和正式许可证仍未由本 ADR 证明。
