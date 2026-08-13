# ADR 0043：连接快速配置可视摘要 surface

日期：2026-08-10

## 状态

已接受，UI-1.56。

## 决策

连接带内新增 `ConnectionPresetContextSurface`，把已有 `ConnectionPreset` 的内置/自定义来源、传输类型和有界描述变成可见的
快速配置摘要。原有 `connection_hint` 仍作为下一步提示、tooltip 与无障碍文案的事实表达；选择 preset 只投影摘要并填充表单，
不自动连接或持久化。

surface 使用 `ThemeSpec` 语义 token 绘制无白色回退的容器和底部 signal rail；已选 preset 时消费共享 `MotionController` frame，
低动效、暂停、隐藏、最小化和关闭时保持静态。

## 边界

- `connection_preset_surface.py` 负责 combo selection 到 surface 的 projection；
- `connection_builder.py` 只负责组合 surface，并继续暴露原 `connection_hint` QLabel contract；
- `controllers/connection_presets.py` 继续负责 preset 应用/保存/删除动作；
- `lifecycle.py` 只负责共享 frame/stop fan-out；
- surface 不读取 ViewModel、不创建 timer、不改变连接 gate、preset codec 或 domain port。

## 验证要求

覆盖未选择/内置/自定义/清空、多主题、共享 frame/stop、原有 hint label 身份与 accessibility、1180×780 near-white，以及最终
onefile/root EXE provenance。
