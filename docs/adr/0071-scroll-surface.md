# ADR-0071：设置页横向滚动边界

日期：2026-08-10  
状态：Accepted for UI-1.84  
范围：`presentation/controllers/composition.py:scroll_page`

## 背景

连接、协议/遥测、命令管理三个页面共用 `scroll_page()`。横向滚动策略未被显式声明，长控件或布局测量可能把横向滚动槽绘成一条亮色横带，破坏工作区的连续视觉层级；项目约束同时要求这些页面不得出现横向滚动。

## 决策

共享入口让 content 使用横向 `QSizePolicy.Expanding`，并将 `QScrollArea` 横向策略设为 `ScrollBarAlwaysOff`。`widgetResizable=True` 保持不变，纵向策略保持 `ScrollBarAsNeeded`。不改页面布局、内容、焦点、accessibility、controller 或 theme token。

## 拒绝的方案

- 不在三个 feature controller 中分别设置滚动条：会复制布局策略并产生边界漂移。
- 不创建自定义横向滚动条或把亮色槽改成装饰：问题是错误的滚动行为，不应继续用视觉掩盖。
- 不通过裁切/固定宽度隐藏内容：content 使用 Expanding 以保留可用宽度和长文案布局。

## 验证

真实组合根 offscreen vector 在 980/1180/1440 宽度检查三个页面：横向 policy hidden、横向 scrollbar 不可见、纵向 policy as-needed、widgetResizable、content Expanding 和页面 objectName 保持；三主题刷新后再次检查通过。主窗口未 `.show()`，完整 Windows GUI/HIDPI/读屏、真实设备/网络、OTA、签名和正式发行未运行。

## 评审与适用性

六个职责角色在源码修改前按项目约束调用，均在窗口内超时并关闭；独立质量复核在源码修改后调用，同样超时并关闭。父代理完成 correctness、readability/simplicity、architecture、security、performance 五轴复核；嵌入式 C/C++ 适用性为 N/A。
