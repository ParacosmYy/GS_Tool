# ADR-0010：M5b typed transform 与 bounded dataset

状态：已接受（2026-08-09）

## 背景

M5a 已能把 UART/TCP Client RX frame 映射为 component field，但如果后续逻辑从 UI 的
`display` 字符串反向解析，单位、缩放、枚举和错误状态会与原始值耦合，也会把数据计算
拖进 Qt 线程。曲线和回放尚未实现，因此本轮需要一个可复用、可限流、可观察的中间边界。

## 决策

1. `ComponentFieldValue` 同时保留有限 scalar `value`；`display` 只负责展示。
2. 新增独立 Dataset schema v1。每个 series 绑定一个 component field，可声明最多 8 个
   内置 transform：`scale`、`offset`、`clamp`、`enum`；enum 只能是最后一项。
3. Dataset 在 `application/datasets.py` 中以独立 worker 运行，输入最多 128 项/256 KiB，
   内存窗口最多 1024 条，默认 256 条。重配置增加 generation 并清理旧队列/窗口。
4. 错误不静默丢失：字段不存在、上游 field error、非数字、非有限计算和未知 enum 分别
   生成 DatasetValue 的 error/warning；raw、component row、terminal、TX 和 recorder 不受影响。
5. 先只接入 UART/TCP Client RX 的 ComponentFramesDecodedEvent；UDP、TCP Server、BLE、
   RTT、TX 编码、数据库和图表留给后续有独立语义的切片。

## 结果

- 领域层提供 `TransformChain`、`DatasetConfig`、`DatasetSample` 和 typed ports/events；
- ViewModel 只消费 DatasetBatchEvent，UI 可显式加载 JSON、查看 bounded preview 和导出 CSV；
- 不新增运行时依赖，不执行表达式、脚本或动态导入；
- CSV 是当前窗口的派生导出，不替代 raw JSONL truth；
- 曲线组件未来可订阅同一 DatasetBatchEvent，不需要改造 transport 或 component codec。

## 未决事项

- EMA/滑动窗口统计、时间轴重采样、曲线渲染和回放需要独立容量/性能设计；
- 真实 UART/TCP 高吞吐、长时间运行和跨传输数据集仍需授权环境验证；
- schema 迁移策略仍后置，本轮只接受 schema v1 严格形状。
