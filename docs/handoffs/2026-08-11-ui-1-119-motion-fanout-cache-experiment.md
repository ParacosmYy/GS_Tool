# UI-1.119 motion fanout cache 性能实验（撤回）

日期：2026-08-11  
父代理：Codex；本地 checkout 唯一写入者  
范围：仅评估 presentation 动效 surface 的 Tab 缓存，不修改业务状态、ViewModel、传输或设备。

## 架构决策

架构师线程 `019fed1f-ec2e-73d3-a4a4-a1c7783ce845` 在限定窗口内超时，未计为通过；父代理按
高内聚/低耦合边界做短时实现与审查。实验实现曾放在独立的 presentation registry：组合根完成后
登记全量 surface，workspace runtime 只在 `currentChanged` 时刷新全局/当前页集合；lifecycle 的
hidden/minimized/close/reduced-motion stop 仍遍历全量集合。

## 证据

- 全量 surface：56；全局：19；四页归属：`9/20/8/0`。
- 切页缓存 active 集合：`28/39/27/19`，cache index 与 Tab index 一致。
- 全量 stop 向量：`residual_animated=0`。
- 低层直接 fan-out（120 帧）：约 `7.25ms` → `4.96ms`，只说明少调用了隐藏 surface。
- 端到端 offscreen 1180×780、协议页、120 帧、每帧 `processEvents()`、5 轮：全量均值
  `781.29ms`，缓存均值 `821.50ms`；缓存更慢。

## 结论与后续

该方案不能证明用户端性能收益，registry 分组复杂度不值得进入生产代码，已完整撤回；源码回到
UI-1.118 canonical 状态，未因负收益重新打包。下一轮性能工作必须先定位可观测瓶颈，再选择更小
的验证切片；不得保留无端到端收益的缓存、逐帧可见性查询或第二套动效时钟。

## Assurance

本轮未修改嵌入式 C/C++、固件、BSP/HAL、RTOS、驱动、协议实现、OTA 或 RTT/J-Link 后端；
embedded applicability=N/A。未运行目标硬件、GUI EXE 正式启动、HIDPI/读屏或真实链路验收；
使用的是非破坏性 offscreen Qt 组合根、源码检查、compileall、Ruff 和 provenance 验证。
