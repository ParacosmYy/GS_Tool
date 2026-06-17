# 架构骨架索引

> 本目录只保留 Python/PyQt 主线架构说明。历史原生设计文档已从活跃架构入口移除，避免后续迭代误回旧工程线。

## 文档列表

- [目标结构](target-structure.md)
- [冻结目录](frozen-dirs.md)
- [模块边界](module-boundaries.md)
- [迁移路线图](migration-roadmap.md)
- [Serial Station 架构约束](../serial_station_architecture.md)

## 使用方式

- 新增目录前先看 `target-structure.md`。
- 遇到历史分叉先看 `frozen-dirs.md`。
- 不确定模块归属时先看 `module-boundaries.md`。
- 需要把能力迁到 Python/PyQt 主线时，先看 `migration-roadmap.md`。
- 涉及串口上位机重构、串口协议新增、串口收发框架拆分时，必须先看 `docs/serial_station_architecture.md`。
