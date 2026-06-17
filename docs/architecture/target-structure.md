# 目标结构

> 这是 EmbedDebug 的 Python/PyQt 主线目标结构。它描述“未来应落在哪里”，不是要求一次性移动所有历史文档。

## Canonical 目录

- `python/embeddebug/app/` - 应用入口、窗口装配、导航、主题、生命周期
- `python/embeddebug/shared/` - 共享常量、枚举、轻量值类型
- `python/embeddebug/interfaces/` - Python 协议、抽象接口与契约
- `python/embeddebug/features/` - 未来功能归属入口，只放骨架和说明
- `python/embeddebug/serial_station/` - 串口上位机工站，按 ui/controllers/core/protocols/services/drivers/workers 分层
- `python/embeddebug/serial_station/drivers/` - 连接接入实现
- `python/embeddebug/serial_station/protocols/` - 协议解析与协议桥
- `python/embeddebug/serial_station/ui/terminal/` - 终端显示与搜索
- `python/embeddebug/serial_station/ui/plotting/` - 波形与图表呈现
- `python/embeddebug/serial_station/services/` - 日志、导出、回放、设备档案
- `python/embeddebug/tools/` - 应用内工具能力
- `tests/python/` - Python/PyQt 单测、契约测试和启动烟测
- `tools/` - uv 启动、体检、打包、发布辅助脚本

## 新增规则

1. 新能力优先落入 canonical Python 包目录。
2. 如果归属不清晰，先在 `python/embeddebug/features/` 放归属说明，再决定是否创建实现目录。
3. 不新增 `2`、`new`、`old`、`bak` 之类的平行实现目录。
4. 串口上位机重构、新协议扩展和串口收发框架拆分优先落入 `python/embeddebug/serial_station/`，不要恢复旧原生功能桶。
