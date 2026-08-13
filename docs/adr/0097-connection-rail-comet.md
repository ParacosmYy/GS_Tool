# ADR-0097：连接状态 rail 彗尾与中心光点

- 状态：Accepted
- 日期：2026-08-11
- 范围：`presentation/connection_status_surface.py`

## 背景

连接页状态 rail 已经拥有共享帧驱动的状态节点，但运动中的状态缺少连续性，视觉上更像离散的点亮变化。需要增加轻量的二次元动效，同时保持状态事实、生命周期和主题边界不变。

## 决策

在既有 `ConnectionStatusRail` 的动画绘制分支内增加三枚渐隐彗尾光点、一个外环和一个中心光点：

- 位置只由既有 `_phase`、节点位置和 `_animated` 推导；
- 颜色继续来自 `ThemeSpec` 的状态语义色；
- 不新增 `QTimer`、业务状态、公开 API、事件总线、资源或传输依赖；
- 保留现有生命周期停止与静态回退策略。

这样 rail 仍由 presentation owner 自绘，应用层继续只提供既有连接状态；未来 OTA/debug 页面可以复用相同的视觉契约，而不耦合 UART、网络、RTT 或 J-Link 实现。

## 状态与边界

`opening`、`open`、`closing` 继续使用共享帧产生动态光点；`discovered`、`closed`、`error` 保持静态。暂停、低动效、隐藏、最小化和关闭路径继续由现有生命周期控制，不能因装饰动效阻塞用户操作或改变连接行为。

## 验证与风险

真实组合根离屏向量覆盖三主题、六状态、运行/停止两种帧策略，并检查无障碍/鼠标透明属性和近白色像素；静态门禁、compileall、ruff 与 provenance manifest 校验通过。未运行可见 GUI/EXE startup、真实 UART/网络/BLE/RTT/J-Link、OTA 或硬件验收。该切片仅修改 Python/Qt presentation，不适用 MCU vendor public source，不声明 MISRA/ISO/硬件合规。

六个前置角色和独立复核均在限定等待窗口内超时并关闭，超时不视为通过；父代理完成五轴审查、架构复用检查、行为保持简化评估和非破坏性验证记录。

