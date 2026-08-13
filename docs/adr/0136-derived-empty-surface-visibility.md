# ADR-0136：派生数据空态与观察预览互斥可见性

日期：2026-08-12

状态：accepted

## 背景

协议页在没有收到数据时同时渲染组件空态卡与固定高度的组件帧预览；Dataset 也渲染固定高度的
空预览。两者都能表达“等待数据”，导致 980px 窗口中出现重复占位、空白表面和更长的滚动距离。

## 决策

- `protocol.py` 只负责创建观察控件并把两个预览初始为 hidden。
- `derived_data.py` 依据现有 `protocol_frames_latest` 与 Dataset samples 投影预览可见性。
- Component 没有协议帧时由 `ComponentEmptyStateSurface` 独占等待态；有帧后观察预览展开。
- Dataset 没有样本时保留状态标签、加载动作和曲线空态；有样本后 Dataset 观察预览展开。
- 不引入第二份业务状态、timer、scroll owner 或额外 callback；现有 objectName、accessibility、
  block count、内容上限、数据/导出路径和 close fence 保持。

## 取舍与验证

这个决策优先降低空态认知噪音和垂直占位，而不是让每个空表面持续占满父容器。真实组合根的
980×720 测量为 `horizontal_max=0`、`component_preview_visible=False`、
`component_empty_visible=True`、`dataset_preview_visible=False`、`vertical_max=778`；注入合法
`DecodedFrame` 后组件预览恢复可见。三主题/最终打包证据由本轮 handoff 追加。

共享动效仍由单一 `MotionController` fan-out；1 秒离屏采样在 `TARGET_HZ=120`、`PreciseTimer`、
8ms target 下得到 124 次 frame callback。Qt/桌面合成不等价于显示器 120Hz。
