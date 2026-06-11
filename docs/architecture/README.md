# 架构骨架索引

> 这里记录兼容迁移阶段的目标骨架，不替代 `docs/constraints/03-architecture.md`。

## 文档列表

- 现有审查与设计文档:
  - `ARCH_040_NavIndicatorToast.md`
  - `ARCH_041_Animations.md`
  - `ARCH_044.md`
  - `ARCH_045_Review.md`
  - `DECOUPLING_PROPOSAL.md`
- 新的骨架说明文档:
  - [目标结构](target-structure.md)
  - [冻结目录](frozen-dirs.md)
  - [模块边界](module-boundaries.md)
  - [迁移路线图](migration-roadmap.md)
- 串口工站专项边界:
  - [Serial Station C++ 架构约束](../serial_station_architecture.md)

## 使用方式

- 新增目录前先看 `target-structure.md`
- 遇到历史分叉先看 `frozen-dirs.md`
- 不确定模块归属时先看 `module-boundaries.md`
- 需要把一个新能力从旧路径迁到新路径时，先看 `migration-roadmap.md`
- 涉及串口上位机重构、串口协议新增、串口收发框架拆分时，必须先看 `docs/serial_station_architecture.md`
