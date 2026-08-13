# ADR 0060：实时观测工具栏显示字段使用主题次级层级

日期：2026-08-10

状态：accepted

## 背景

实时观测工具栏中“显示”标签仍是没有语义属性的原生 `QLabel`。它紧邻显示模式 combo，可能继承系统 palette 的高亮文字，
使 section、字段和状态 rail 的层级不稳定，尤其在三套深色主题切换后容易形成白色视觉回退。

## 决策

在 `presentation/controllers/terminal.py` 增加局部 `_field_label(text)` helper，只创建 `QLabel` 并设置
`role="muted"`，将“显示”标签接入既有 `QLabel[role="muted"]` 主题规则。`实时观测` 保持 `role="section"`；暂停、记录、
接收活动保持 `role="status"`。

该 helper 不拥有显示模式、暂停、记录、接收数据或快捷键状态，不读取 ViewModel、不接收 signal、不创建 timer，也不改变
`_display_mode.currentIndexChanged`、`_rerender_preview`、MotionController、焦点、Tab 顺序或布局列。

## 未采用方案

- 不给所有 terminal QLabel 统一改成 muted：section/status/error/subtle 是不同的稳定语义，批量替换会降低可读性。
- 不改全局 QLabel 默认样式：无法限定到显示字段，可能覆盖状态 surface。
- 不增加 display mode 动画：模式切换只改变预览格式，不应引入第二个业务状态或常驻 timer。

## 验证

- 真实 `create_application()` + `create_main_window()` 的短时 Qt offscreen 组合根 vector：三套主题均确认“显示”为 `muted`、
  “实时观测”为 `section`、display mode combo 有 2 项、暂停/记录/接收状态为 `status`。
- `.venv\Scripts\python.exe -m compileall -q src`：pass。
- 独立质量复核代理 `019feb9a-75d9-7892-86df-dd257735b192` 在窗口内超时并关闭；未将超时写成通过，父代理完成五轴审查。
- 未启动完整 GUI/EXE、未运行 unit tests、未创建测试/夹具/模拟器、未接入真实设备；嵌入式 C/C++ 适用性：N/A。
