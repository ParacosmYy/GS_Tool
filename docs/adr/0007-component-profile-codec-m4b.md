# ADR 0007：M4b 声明式 component profile 与字段 worker

## 背景

M4a 已经能生成有界 `DecodedFrame`，但把字段解析放进 Qt 会让 UI 承担 codec、异常和高频
数据压力；把 JSON/profile 直接交给 transport 又会破坏传输与组件的职责边界。

## 决策

- `domain.components` 定义 versioned `ComponentProfile`、`FieldSpec`、`ComponentFrameRow`
  和无动态执行的 `BinaryComponentCodec`；
- profile schema v1 只允许固定 offset/length、Hex、UTF-8、uint、int、Float32、端序、scale
  和 unit；profile/字段/显示结果均有上限；
- `application.components.ComponentPipelineWorker` 独占字段解码线程和 bounded queue；
- application event bus 观察 raw stream 与 protocol frame，分别桥接到 protocol/component worker，
  presentation 只消费结果；
- UI 只提供显式 JSON profile 加载、有效/错误过滤、bounded table 和当前 rows CSV 导出；
- 不在本 ADR 引入任意 Python、表达式、动态插件、JSON/TLV 通用解释器、曲线或回放。

## 取舍与风险

声明式固定字段覆盖常见传感器和状态帧，牺牲了任意协议的表达能力；后续必须为 TLV、字段
迁移和复杂 transform 建立单独版本化 ADR。CSV 只是当前表格视图，不替代 JSONL raw truth。

## 验收证据

使用 inline vectors 验证 profile round-trip、字段端序/scale、未知 kind 拒绝、component worker
事件和 raw-to-protocol-to-component bridge；Ruff、compileall、offscreen Qt 和最终 PyInstaller
门沿用本轮记录。真实设备、长时间压力和 clean Windows 仍未运行。
