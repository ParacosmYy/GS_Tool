# ADR 0051：连接方式配置面板一次性过渡

日期：2026-08-10

状态：accepted

## 背景

连接页切换 UART、TCP Client、TCP Server、UDP、BLE GATT 或 RTT 时，
`connection_runtime.py` 会立即切换对应配置区域的可见性。业务行为正确，但视觉上突然跳变，
与工作区 Tab 和主题切换已有的一次性过渡不一致。

## 决策

- 新增 `presentation/transport_panel_transition.py`，只对已完成显隐 projection 的当前 panel
  添加 160ms `QGraphicsOpacityEffect` fade。
- 新增 `presentation/motion_policy.py`，由一次性过渡和 workspace runtime 共享低动效/暂停偏好判定。
- `connection_runtime.py` 继续拥有 transport kind、panel visibility、字段显隐、默认值和连接 gate；
  transition helper 不读取 ViewModel、不修改配置、不进入焦点/无障碍树。
- `bootstrap.py` 只初始化 animation/effect 引用；`lifecycle.py` 在主题切换、低动效、暂停、隐藏、
  最小化和关闭时统一停止并清理。
- 如果目标 panel 已有其他 `graphicsEffect()`，transition 不接管；快速切换先清理上一轮，避免 effect 叠加。

## 被否决的方案

- 为每个 transport panel 建立独立 timer 或常驻动画：会增加后台开销并与共享 motion policy 分叉。
- 在 transition helper 内复制连接状态或 transport enum 分支：会把业务事实带入 presentation decoration。
- 动画 panel geometry 或焦点：布局 owner 可能在动画期间重新计算尺寸，且会破坏键盘路径。

## 后果与验证

切换连接方式时获得低干扰的主题化 fade，同时原生控件、Tab 顺序、无障碍和连接动作保持不变。
静态检查、compileall 和 presentation import 向量已通过；按当前项目授权边界，本轮未启动 GUI/EXE、
未运行 offscreen/HIDPI/读屏、真实设备或硬件验收，这些项目必须在用户授权后单独记录证据。
