# 目标结构

> 这是 EmbedDebug 的兼容迁移目标结构。它描述“未来应落在哪里”，不是要求一次性移动所有现有源码。

## Canonical 目录

- `src/shared/` - 共享常量、枚举、轻量值类型
- `src/interfaces/` - 纯虚接口与契约
- `src/core/` - 应用协调、基础 UI、导航、主题、会话
- `src/features/` - 未来功能归属入口，只放骨架和说明
- `src/connection/` - 连接接入实现
- `src/protocol/` - 协议解析与协议桥
- `src/terminal/` - 终端显示与搜索
- `src/chart/` - 波形与图表呈现
- `src/ota/` - 固件升级
- `src/automation/` - 自动化与触发器
- `src/dashboard/` - 仪表盘控件
- `src/plugin/` - 插件系统
- `src/utils/` - 通用工具

## 新增规则

1. 新能力优先落入 canonical 目录。
2. 如果归属不清晰，先在 `src/features/` 放归属说明，再决定是否创建实现目录。
3. 不新增 `2`、`new`、`old`、`bak` 之类的平行实现目录。
