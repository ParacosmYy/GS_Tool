# ADR 0042：发送状态 activity rail

日期：2026-08-10

## 状态

已接受，UI-1.55。

## 决策

发送控制带的 `sendState` 继续使用原生 `QLabel`，但改由 `SendStateSurface` 在原生文字之后绘制四节点状态 rail。
surface 只读取 `connection.py` 已经投影的 `state` property：`blocked`、`waiting`、`ready`、`busy`、`history`。

`busy` 在共享 `MotionController` frame 可用时显示 pulse；其它状态只显示静态节点，避免把等待、可发送或历史来源误解为进度。
文字、tooltip、AccessibleName/Description、objectName、发送 gate 和状态 owner 均保持原有行为。

## 边界

- `terminal.py` 只替换 QLabel 构造，不承载发送状态；
- `connection.py` 继续唯一负责 state/text/accessibility projection；
- `lifecycle.py` 只负责共享 frame/stop fan-out；
- surface 不读取 ViewModel、不创建 timer、不改变发送/批量/历史语义。

## 验证要求

覆盖五种 state、native text/accessibility/objectName、busy pulse、static/stop/reduced-motion、三主题、980×680 near-white
与最终 provenance 构建。
