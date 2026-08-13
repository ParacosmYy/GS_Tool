# ADR 0048：Dataset 曲线空态画布与共享 signal rail

日期：2026-08-10  
状态：accepted  
范围：`presentation/dataset_curve.py`

## 背景

Dataset 曲线在未选择 series、等待 sample 或没有可绘制数值时只显示单行文字。虽然数据契约正确，但视觉上像
失效控件，无法表达“当前仍可继续配置/等待数据”的状态，也没有继承工作台的动态 signal 语言。

## 决策

- 在现有 `DatasetCurveWidget` 内增加资源无关的空态画布：主题化 panel、protocol-to-curve glyph、标题/说明和
  节点 rail；不新增文件或状态机。
- `set_frame()`/`stop()` 接收生命周期共享帧，仅在空态或无 points 时刷新装饰；数据 points 的真实绘制路径保持原样。
- 保持 `CurveSnapshot` immutable projection、QTimer debounce、`flush()`、`set_suspended()`、`shutdown()`、焦点环和
  AccessibleDescription；不自动选择 series、不模拟进度、不新增 action。
- 将现有 `_dataset_curve` 加入 `lifecycle._motion_surfaces()`，统一 reduced-motion、暂停、隐藏、最小化和关闭回退。

## 被拒绝的替代

- 新建 Dataset 曲线状态 controller：空态事实已经由 `CurveSnapshot` 表达，会复制业务状态。
- 用独立 QTimer 做扫描动画：违反共享 MotionController 生命周期，隐藏/暂停时更容易泄漏。
- 在曲线空态中加入自动选 series/加载 Dataset 按钮：会改变用户配置和 action owner 边界。

## 后果与验证

曲线空态保持原有尺寸和键盘焦点行为，但具备三种可感知的等待/错误视觉层级；真实数据曲线不受影响。验证包括
源码质量门禁、empty/waiting snapshot、共享 frame/stop、三主题 1180×780 无近白像素和 onefile/provenance。
真实设备、读屏、硬件和正式发行验收仍不在本 ADR 范围内。
