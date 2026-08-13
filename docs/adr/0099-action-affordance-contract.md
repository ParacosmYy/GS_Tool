# ADR-0099：关键动作与 UART 超时 affordance 契约

日期：2026-08-11  
状态：accepted（UI-1.109）

## 背景

真实组合根审计发现，错误提示、终端预览、发送历史、快捷命令、自定义连接配置以及 UART 读写超时虽然已有可读文本或 accessible name，但部分控件缺少一致的 tooltip / accessible description。用户和读屏工具无法在动作发生前确认副作用边界，尤其容易把“保存/清除”误解为发送、断开或删除原始数据。

## 决策

- 每个控件继续由其 presentation owner 声明 `accessibleName`、tooltip 与 `accessibleDescription`，不引入全局 affordance registry 或跨层文案服务。
- 清除错误、清空终端预览、清除发送历史、打开快捷命令、保存快捷命令、保存/删除自定义连接配置必须说明实际影响与不影响的连接、发送、原始记录范围。
- UART 读超时和写超时继续由 `QDoubleSpinBox` 提供既有 bounded value；只补充“秒”和连接后生效的说明，不改变范围、默认值、DTO 或 runtime callback。
- accessibility 文案只解释现有动作和数据边界，不承诺自动连接、自动发送、设备探测、OTA 或 J-Link 行为。

## 边界与验证

本切片不新增业务状态、timer、signal、DTO、依赖、线程、设备 I/O 或公开 API；`MainWindow`、application/domain、OTA/debug contract-only/attach-only 边界保持不变。真实组合根验证覆盖 9 个目标控件、27 个 combo、UART 唯一可编辑边界、三主题及 980×680 / 1180×780 渲染，近白像素为 0，关闭生命周期通过。

