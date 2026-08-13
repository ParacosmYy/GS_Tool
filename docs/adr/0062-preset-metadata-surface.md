# ADR 0062：自定义连接配置元数据使用主题 surface

日期：2026-08-10

状态：accepted

## 背景

主连接页已经建立 `section / muted field / hint / state` 的主题层级，但保存/编辑自定义连接配置的 dialog 仍将“名称”和“备注”
以裸 `QLabel` 放在 dialog 根布局中。这样字段层级不明确，且容易在系统 palette 或主题变化时与输入框形成过亮对比。

## 决策

在 `ConnectionPresetEditorDialog` 中增加一个 `QFrame#presetMetadataFields[role="surface"]`，只承载名称和备注两个既有
`QLineEdit`。字段标签通过局部 `_field_label()` 设置 `role="muted"`，复用现有 `QWidget[role="surface"]` 与
`QLabel[role="muted"]` stylesheet，不新增主题 token 或独立资源。

该 surface 只改变布局分组与视觉层级；`ConnectionPresetMetadata`、随机 key、空名称/长度校验、保存/取消按钮、初始 `_label` focus、
Tab/accessibility、`dialog_transition.py` 和 controller/store 的业务边界不变。surface 不读取 ViewModel、不保存数据、不接收业务
signal，也不创建 timer。

## 未采用方案

- 不新增全局 dialog 基类或 form factory：当前只需一个 dialog 的局部 presentation helper。
- 不新增一套 metadata 专用 QSS/token：既有 surface/muted 语义已能表达需要的层级。
- 不把 label 文案或校验移入 controller：dialog 仍是 metadata editor 的 presentation owner，controller 仍负责 DTO 与持久化 action。
- 不为静态字段增加动画：dialog 入口淡入已经由 UI-1.74 统一拥有，字段 surface 不应制造第二套过渡。

## 验证

- 真实 `create_application()` + `create_main_window()` 的短时 Qt offscreen 组合根 vector：star_trail、moonlit_ocean、sakura_night
  三套主题均确认 metadata surface、两个 muted 字段标签、初始 label focus、有效 metadata 投影、空名称校验和 hide 后 effect 清理。
- `.venv\Scripts\python.exe -m compileall -q src`：pass；项目静态门禁：pass。
- 独立质量复核代理 `019febad-460e-7ac2-9e75-1b64c0b01172` 在窗口内超时并关闭；未将超时写成通过，父代理完成五轴审查。
- 未启动完整 GUI/EXE、未运行 unit tests、未创建测试/夹具/模拟器、未接入真实设备；嵌入式 C/C++ 适用性：N/A。
