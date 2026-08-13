# ADR 0041：实时接收数据 activity rail

日期：2026-08-10

## 状态

已接受，UI-1.54。

## 决策

将实时观测带中原有的接收数据 `QLabel` 替换为 `DataActivitySurface(QLabel)`。它先完成 QLabel 原生文字/QSS 绘制，再在底部
padding 内绘制低对比度实时/历史 activity rail。文字、`objectName`、AccessibleName/Description、tooltip 和布局契约保持不变。

`terminal_runtime.update_data_activity()` 继续唯一拥有用户可读文案，并从既有 presentation facts 生成 bounded
`DataActivityProjection(source, latest_bytes, window_bytes)`。`lifecycle.set_data_activity_motion()` 继续拥有 active 复位，
`MotionController` 继续提供唯一 frame；surface 不读取 ViewModel、不复制 RX buffer、不创建 timer。

## 边界

- 不绘制吞吐率、百分比、设备进度或接收总量；字节值只用于确定性装饰形态；
- source 仅区分既有“实时/历史”上下文，不新增历史状态源；
- active、frame、stop 均由现有 lifecycle 接线投影；
- reduced-motion、暂停、隐藏、最小化和关闭保留静态轨道，文字始终可读。

## 验证要求

覆盖实时/历史、零字节/非零字节、active/static/frame/stop、native QLabel text/accessibility/objectName、三主题、980×680
near-white 和最终 provenance 构建。
