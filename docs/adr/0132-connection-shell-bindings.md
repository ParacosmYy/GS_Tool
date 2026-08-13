# ADR 0132：链路控制外壳绑定边界

- 日期：2026-08-11
- 状态：accepted
- 关联切片：ARCH-6x / UI-1.140

## 背景

传输选择、连接快速配置、preset 上下文、状态 rail 和连接按钮属于同一块
presentation shell，但此前由多个 controller 通过 `window._...` 动态字段读取。
这让组合阶段顺序、生命周期和后续嵌入式工具站扩展容易产生隐式耦合。

## 决策

在 `presentation/connection_bindings.py` 增加 frozen/slots
`ConnectionShellBindings`，只保存以下 Qt 引用：control band、transport mode
surface、transport/preset combo、preset context、保存/删除动作、status rail 和
connect action。`connection_builder.py` 是唯一组装 owner，并在刷新 preset combo
之前完成 bundle 写入，确保后续 signal/refresh 路径可以读取完整壳体。

连接、runtime、preset、commands、composition、derived/protocol、replay、source
scope 和 lifecycle 通过 `connection_shell_bindings_for()` 读取。preset catalog/store、
session/ViewModel、连接策略、callbacks、timer、transport handle、peer/device snapshot
不进入 bundle。builder 内的动态字段只允许作为构建与 signal 接线阶段的兼容装配字段。

## 影响与验证边界

该变化只收敛 Qt wiring，不改变 `itemData()`、preset 不自动连接语义、连接 gate、
TCP/BLE/RTT/UART 行为、焦点/Tab/accessibility、主题、120Hz shared motion 或 OTA/debug
contract-only/attach-only 边界。初始化早期 accessor 缺失时 projection 安全返回；需要
执行连接、保存或删除的路径显式提示控件未初始化。

本轮不涉及嵌入式 C/C++、MCU、SDK、RTOS 或硬件；没有适用的厂商一手要求，也不声称
MISRA、ISO 26262、ASIL、ASPICE 或任何认证合规。验证使用静态检查、compileall、Ruff、
项目既有 `scripts/check.ps1` 和后续授权的 offscreen 组合根向量；不创建测试资产，不闪写、
擦除、部署或连接硬件。
