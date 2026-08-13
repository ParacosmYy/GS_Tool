# ADR 0063：快速配置选择使用共享 activity rail 确认

日期：2026-08-10

状态：accepted

## 背景

连接页的快速配置选择会同步更新表单和 `ConnectionPresetContextSurface` 摘要，但用户缺少即时的视觉确认。摘要 surface 已经拥有
共享 frame 驱动的 rail 绘制，增加一段短时 activity window 可以改善选择反馈，同时不需要第二套动画时钟。

## 决策

在 `controllers/connection_builder.py:_dispatch_connection_preset()` 完成既有 selection projection 和 preset apply callback 后，
调用 `window._motion_controller.request_activity(420)`。`ConnectionPresetContextSurface` 保持只消费已有 `frame_changed`，其 rail
在 `_animated` 时绘制 bounded pulse；controller 不创建 timer、不保存 transient animation state，也不改变 preset/connection 业务状态。

`refresh_connection_preset_combo()` 仍通过 blocked signals hydration，因此初始化/重建 catalog 不伪造用户 activity。用户选择内置、自定义或清空
选项均可触发同一确认窗口；MotionController 自己负责 reduced-motion、暂停、隐藏、最小化和关闭回退。

## 未采用方案

- 不在 `ConnectionPresetContextSurface` 内创建 `QTimer`：presentation 的装饰时钟必须保持单一共享 owner。
- 不新增 `QPropertyAnimation` 或独立 selection transition：已有 rail + shared frame 足以表达一次 bounded acknowledgement。
- 不把“已选择”写入 SessionState/ViewModel：选择快速配置只填充表单，不能被误读为连接成功。
- 不让 `update_connection_preset_context()` 直接拥有 window/controller：保持 context surface 与业务 controller 的依赖方向。

## 验证

- 真实 `create_application()` + `create_main_window()` 的短时 Qt offscreen 组合根 vector：三套主题的内置 preset selection 均启动 shared
  activity 并产生 animated frames，清空 selection 保留 combo 语义；reduced-motion 与显式 paused 均不启动 activity timer。
- `.venv\Scripts\python.exe -m compileall -q src`：pass；项目静态门禁：pass。
- 独立质量复核代理 `019febb5-25aa-7640-9acd-9d66d3f0157c` 在窗口内超时并关闭；未将超时写成通过，父代理完成五轴审查。
- 未启动完整 GUI/EXE、未运行 unit tests、未创建测试/夹具/模拟器、未接入真实设备；嵌入式 C/C++ 适用性：N/A。
