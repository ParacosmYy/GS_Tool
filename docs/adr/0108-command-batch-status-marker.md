# ADR-0108：批量命令状态 marker 与共享帧呼吸环

日期：2026-08-11  
状态：Accepted  
范围：SerialForge presentation / command management

## 背景

批量命令页已经有可访问的状态文案和步骤 rail，但在密集工作区中仅依靠文字和底部轨道，
状态入口不够醒目。需要提升扫描效率，同时保持“已提交”只表示本地发送队列接受，不能暗示
设备 ACK、真实吞吐或刷写进度。

## 决策

- 保持 `controllers/commands.py` 为 `CommandBatchSnapshot` → `CommandBatchSurfaceProjection`
  的唯一 projection owner；不新增 DTO 字段和业务状态。
- 在 `CommandBatchSurfaceLabel` 的原生 label 绘制完成后，使用既有 state 映射绘制左侧 semantic
  marker；`running` 只消费既有 shared `(phase, animated)` 绘制低对比度呼吸环。
- base stylesheet 只增加左侧 padding 和字重，避免 marker 与中文重叠；三主题颜色继续由
  `ThemeSpec`/variant QSS 提供，不把状态判断写入 QSS。
- 保留 `NoFocus`、鼠标透明、AccessibleDescription、步骤 rail、`stop()` 和 reduced-motion /
  hidden / minimized / close 静态回退；不新增局部 timer、MotionController、signal、线程、I/O
  或 OTA/AES/RTT/J-Link 依赖。

## 验证

静态门禁、compileall、Ruff、provenance verify 和三主题 offscreen vector 通过；生产字体为
`Microsoft YaHei UI`，marker/rail 在 `720x120` surface 中可见，NoFocus、鼠标透明、projection
和主题背景断言通过，截图已人工查看。

架构师线程 `019fed34-6372-7050-ab70-437f34378271` 在限定窗口内超时，未计为独立通过；父代理
完成 owner、token、Qt 绘制、可访问性、简化和生命周期审查。未修改嵌入式 C/C++；embedded
applicability=N/A。
